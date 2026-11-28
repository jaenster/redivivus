// Win32Posix: DirectSound (Win32PosixDirectSound.h) on an SDL3 audio stream. The shared code
// (D2Sound, Storm's SFile streams) calls through the COM vtables, `p->lpVtbl->Play(p, ...)`, so
// each interface here is a record whose first member is a vtable laid out as the shared
// headers lay it out (Source/D2Sound/D2Sound.h, Source/Storm/Storm.h, Source/D2Client/Sound/Sound.h).
//
// The behaviour reference is Storm's Mac layer (Source/StormMac/Win32Mac/Source/Win32/
// Win32_DirectSound.cpp): volume and pan in hundredths of a decibel turned into a level per
// side as 10^(dB/2000), the primary buffer that owns no memory, Lock handing out the buffer's
// own memory, no EAX property set. Where the Mac layer approximates DirectSound with the Sound
// Manager's chunks - a cursor that moves a chunk at a time, Stop rewinding, Play restarting a
// one-shot from the top - this layer does what DirectSound does, because the modern build takes
// D2Sound's Windows path: the samples are ring buffers played looping and refilled from the
// play cursor (SOUNDSMP_WriteToBuffer, D2SOUNDSMP_SoundUpdateThreadProc), so the cursor moves
// frame by frame, wraps, and stays put across Stop.
//
// The mixer: every playing secondary buffer is resampled (linear) to the primary format's rate,
// scaled by its two levels and summed into stereo float, which the SDL stream converts to the
// device. The play cursor is the mix position: the bytes before it have been consumed, so the
// write cursor is the same point. The 3D interface is accepted and stored but not spatialised,
// and the device reports no hardware 3D buffers, as a software DirectSound does.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include "Source/D2Headers/d2_platform.h"
#include "Source/StormMac/Win32Posix/DirectSoundMixer.h"

#include <SDL3/SDL.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

// No <vector>: libc++'s reads <sys/types.h>, whose fd_set is not Winsock's.
#include <atomic>
#include <mutex>
#include <new>

namespace {

// ---- DirectSound's numbers (dsound.h) ----
constexpr HRESULT kOk = 0;
constexpr HRESULT kNoInterface = (HRESULT)0x80004002;
constexpr HRESULT kNotImpl = (HRESULT)0x80004001;
constexpr HRESULT kOutOfMemory = (HRESULT)0x8007000E;
constexpr HRESULT kInvalidParam = (HRESULT)0x80070057;
constexpr HRESULT kNoAggregation = (HRESULT)0x80040110;
constexpr HRESULT kControlUnavail = (HRESULT)0x8878001E;
constexpr HRESULT kInvalidCall = (HRESULT)0x88780032;
constexpr HRESULT kPrioLevelNeeded = (HRESULT)0x88780046;
constexpr HRESULT kNoDriver = (HRESULT)0x88780078;
constexpr HRESULT kAlreadyInitialized = (HRESULT)0x88780082;

constexpr DWORD DSBCAPS_PRIMARYBUFFER = 0x1;
constexpr DWORD DSBCAPS_LOCSOFTWARE = 0x8;
constexpr DWORD DSBCAPS_CTRL3D = 0x10;
constexpr DWORD DSBCAPS_CTRLFREQUENCY = 0x20;
constexpr DWORD DSBCAPS_CTRLPAN = 0x40;
constexpr DWORD DSBCAPS_CTRLVOLUME = 0x80;
constexpr DWORD DSBPLAY_LOOPING = 0x1;
constexpr DWORD DSBSTATUS_PLAYING = 0x1;
constexpr DWORD DSBSTATUS_LOOPING = 0x4;
constexpr DWORD DSBLOCK_FROMWRITECURSOR = 0x1;
constexpr DWORD DSBLOCK_ENTIREBUFFER = 0x2;
constexpr LONG DSBVOLUME_MIN = -10000;
constexpr LONG DSBVOLUME_MAX = 0;
constexpr LONG DSBPAN_LEFT = -10000;
constexpr LONG DSBPAN_RIGHT = 10000;
constexpr DWORD DSBFREQUENCY_MIN = 100;
constexpr DWORD DSBFREQUENCY_MAX = 200000;
constexpr DWORD DSBSIZE_MIN = 4;
constexpr DWORD DSBSIZE_MAX = 0x0FFFFFFF;
constexpr DWORD DSSPEAKER_STEREO = 4;
// DSCAPS: primary and secondary mono/stereo/8/16-bit, a continuous rate, an emulated driver.
constexpr DWORD kDeviceCapsFlags = 0x1 | 0x2 | 0x4 | 0x8 | 0x10 | 0x20 | 0x100 | 0x200 | 0x400 | 0x800;
constexpr WORD WAVE_FORMAT_PCM = 1;
// The mixer's output: stereo float.
constexpr int kMixChannels = 2;
// DirectSound's default primary format: 22050 Hz.
constexpr int kDefaultMixRate = 22050;

constexpr GUID kIID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
constexpr GUID kIID_IDirectSound = {0x279AFA83, 0x4981, 0x11CE, {0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60}};
constexpr GUID kIID_IDirectSoundBuffer = {0x279AFA85, 0x4981, 0x11CE, {0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60}};
constexpr GUID kIID_IDirectSound3DBuffer = {0x279AFA86, 0x4981, 0x11CE, {0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60}};
// DSDEVID_DefaultPlayback and DSDEVID_DefaultVoicePlayback.
constexpr GUID kDefaultPlayback = {0xDEF00000, 0x9C6D, 0x47AD, {0xAB, 0x5D, 0x00, 0xB8, 0x28, 0x6D, 0x14, 0x48}};
constexpr GUID kDefaultVoicePlayback = {0xDEF00002, 0x9C6D, 0x47AD, {0xAB, 0x5D, 0x00, 0xB8, 0x28, 0x6D, 0x14, 0x48}};

bool SameGuid(const GUID* pA, const GUID& tB) {
    return pA && memcmp(pA, &tB, sizeof(GUID)) == 0;
}

// DSBUFFERDESC (DirectX 7): the pointer is at 0x10 on both widths.
struct BufferDesc {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwReserved;
    const WAVEFORMATEX* lpwfxFormat;
};
// DSBCAPS and the head of DSCAPS, the fields this layer fills.
struct BufferCaps {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwUnlockTransferRate;
    DWORD dwPlayCpuOverhead;
};
static_assert(sizeof(BufferCaps) == 0x14, "DSBCAPS");
constexpr DWORD kDeviceCapsSize = 0x60;
struct DeviceCaps {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwMinSecondarySampleRate;
    DWORD dwMaxSecondarySampleRate;
    DWORD dwPrimaryBuffers;
    DWORD aRest[(kDeviceCapsSize - 5 * sizeof(DWORD)) / sizeof(DWORD)];
};
static_assert(sizeof(DeviceCaps) == kDeviceCapsSize, "DSCAPS");

struct DirectSoundObject;
struct SoundBufferObject;
struct Sound3DObject;

// ---- the vtables, in the shared headers' slot order ----
struct DirectSoundVtbl {
    HRESULT (*QueryInterface)(void*, const GUID*, void**);
    ULONG (*AddRef)(void*);
    ULONG (*Release)(void*);
    HRESULT (*CreateSoundBuffer)(void*, const void*, void**, void*);
    HRESULT (*GetCaps)(void*, void*);
    HRESULT (*DuplicateSoundBuffer)(void*, void*, void**);
    HRESULT (*SetCooperativeLevel)(void*, HWND, DWORD);
    HRESULT (*Compact)(void*);
    HRESULT (*GetSpeakerConfig)(void*, DWORD*);
    HRESULT (*SetSpeakerConfig)(void*, DWORD);
    HRESULT (*Initialize)(void*, const GUID*);
};
static_assert(sizeof(DirectSoundVtbl) == 11 * sizeof(void*), "IDirectSoundVtbl: 11 slots");

struct SoundBufferVtbl {
    HRESULT (*QueryInterface)(void*, const GUID*, void**);
    ULONG (*AddRef)(void*);
    ULONG (*Release)(void*);
    HRESULT (*GetCaps)(void*, void*);
    HRESULT (*GetCurrentPosition)(void*, DWORD*, DWORD*);
    HRESULT (*GetFormat)(void*, WAVEFORMATEX*, DWORD, DWORD*);
    HRESULT (*GetVolume)(void*, LONG*);
    HRESULT (*GetPan)(void*, LONG*);
    HRESULT (*GetFrequency)(void*, DWORD*);
    HRESULT (*GetStatus)(void*, DWORD*);
    HRESULT (*Initialize)(void*, void*, const void*);
    HRESULT (*Lock)(void*, DWORD, DWORD, void**, DWORD*, void**, DWORD*, DWORD);
    HRESULT (*Play)(void*, DWORD, DWORD, DWORD);
    HRESULT (*SetCurrentPosition)(void*, DWORD);
    HRESULT (*SetFormat)(void*, const WAVEFORMATEX*);
    HRESULT (*SetVolume)(void*, LONG);
    HRESULT (*SetPan)(void*, LONG);
    HRESULT (*SetFrequency)(void*, DWORD);
    HRESULT (*Stop)(void*);
    HRESULT (*Unlock)(void*, void*, DWORD, void*, DWORD);
    HRESULT (*Restore)(void*);
};
static_assert(sizeof(SoundBufferVtbl) == 21 * sizeof(void*), "IDirectSoundBufferVtbl: 21 slots");

struct Sound3DVtbl {
    HRESULT (*QueryInterface)(void*, const GUID*, void**);
    ULONG (*AddRef)(void*);
    ULONG (*Release)(void*);
    HRESULT (*GetAllParameters)(void*, void*);
    HRESULT (*GetConeAngles)(void*, DWORD*, DWORD*);
    HRESULT (*GetConeOrientation)(void*, void*);
    HRESULT (*GetConeOutsideVolume)(void*, LONG*);
    HRESULT (*GetMaxDistance)(void*, float*);
    HRESULT (*GetMinDistance)(void*, float*);
    HRESULT (*GetMode)(void*, DWORD*);
    HRESULT (*GetPosition)(void*, void*);
    HRESULT (*GetVelocity)(void*, void*);
    HRESULT (*SetAllParameters)(void*, const void*, DWORD);
    HRESULT (*SetConeAngles)(void*, DWORD, DWORD, DWORD);
    HRESULT (*SetConeOrientation)(void*, float, float, float, DWORD);
    HRESULT (*SetConeOutsideVolume)(void*, LONG, DWORD);
    HRESULT (*SetMaxDistance)(void*, float, DWORD);
    HRESULT (*SetMinDistance)(void*, float, DWORD);
    HRESULT (*SetMode)(void*, DWORD, DWORD);
    HRESULT (*SetPosition)(void*, float, float, float, DWORD);
    HRESULT (*SetVelocity)(void*, float, float, float, DWORD);
};
static_assert(sizeof(Sound3DVtbl) == 21 * sizeof(void*), "IDirectSound3DBufferVtbl: 21 slots");

// The 3D interface of a buffer: its parameters, kept and read back. It shares the buffer's
// reference count, as a COM interface of the same object does.
struct Sound3DObject {
    const Sound3DVtbl* lpVtbl;
    SoundBufferObject* pBuffer;
    float aPosition[3];
    float aVelocity[3];
    float aConeOrientation[3];
    DWORD dwInsideConeAngle;
    DWORD dwOutsideConeAngle;
    LONG lConeOutsideVolume;
    float fMinDistance;
    float fMaxDistance;
    DWORD dwMode;
};

struct SoundBufferObject {
    const SoundBufferVtbl* lpVtbl;
    std::atomic<ULONG> nRefCount;
    DirectSoundObject* pDevice;
    DWORD dwFlags;
    bool bPrimary;
    WAVEFORMATEX tFormat;
    uint8_t* pData;
    DWORD nSize;
    DWORD nFrameBytes;
    DWORD nFrames;
    // Guarded by the device's lock: what the mixer reads.
    bool bPlaying;
    bool bLooping;
    uint64_t nPosition;  // frames, 32.32 fixed point
    LONG lVolume;
    LONG lPan;
    DWORD dwFrequency;  // 0: the format's rate
    float fLevelLeft;
    float fLevelRight;
    Sound3DObject t3D;
};

struct DirectSoundObject {
    const DirectSoundVtbl* lpVtbl;
    std::atomic<ULONG> nRefCount;
    SDL_AudioStream* pStream;
    std::mutex tLock;
    // The secondary buffers, guarded by tLock.
    SoundBufferObject** apBuffers;
    size_t nBuffers;
    size_t nBufferRoom;
    SoundBufferObject* pPrimary;
    int nMixRate;
    DWORD dwCooperativeLevel;
    // The device callback's mix buffer.
    float* pScratch;
    size_t nScratchFrames;
};

// ---- levels ----

// Hundredths of a decibel to a linear level; DSBVOLUME_MIN is silence.
float Level(LONG lMillibels) {
    if (lMillibels <= DSBVOLUME_MIN) {
        return 0.0f;
    }
    if (lMillibels >= 0) {
        return 1.0f;
    }
    return (float)pow(10.0, (double)lMillibels / 2000.0);
}

// The two levels of a volume and a pan, as Win32_DirectSound.cpp's UpdatePanAndVolume: a pan to
// the right attenuates the left side by the pan, a pan to the left the right side.
void UpdateLevels(SoundBufferObject* pBuffer) {
    LONG lLeft = pBuffer->lVolume - (pBuffer->lPan > 0 ? pBuffer->lPan : 0);
    LONG lRight = pBuffer->lVolume + (pBuffer->lPan < 0 ? pBuffer->lPan : 0);
    pBuffer->fLevelLeft = Level(lLeft);
    pBuffer->fLevelRight = Level(lRight);
}

// ---- the mixer ----

inline void ReadFrame(const SoundBufferObject* pBuffer, DWORD nFrame, float* pLeft, float* pRight) {
    const uint8_t* p = pBuffer->pData + (size_t)nFrame * pBuffer->nFrameBytes;
    if (pBuffer->tFormat.wBitsPerSample == 8) {
        *pLeft = ((int)p[0] - 128) * (1.0f / 128.0f);
        *pRight = pBuffer->tFormat.nChannels == 2 ? ((int)p[1] - 128) * (1.0f / 128.0f) : *pLeft;
    } else {
        int16_t aSamples[2];
        memcpy(aSamples, p, pBuffer->tFormat.nChannels * sizeof(aSamples[0]));
        *pLeft = aSamples[0] * (1.0f / 32768.0f);
        *pRight = pBuffer->tFormat.nChannels == 2 ? aSamples[1] * (1.0f / 32768.0f) : *pLeft;
    }
}

void MixBuffer(SoundBufferObject* pBuffer, float* pOut, int nFrames, int nMixRate) {
    if (pBuffer->nFrames == 0) {
        pBuffer->bPlaying = false;
        return;
    }
    DWORD dwRate = pBuffer->dwFrequency ? pBuffer->dwFrequency : pBuffer->tFormat.nSamplesPerSec;
    uint64_t nStep = ((uint64_t)dwRate << 32) / (uint64_t)nMixRate;
    uint64_t nEnd = (uint64_t)pBuffer->nFrames << 32;
    float fLeftLevel = pBuffer->fLevelLeft;
    float fRightLevel = pBuffer->fLevelRight;
    for (int i = 0; i < nFrames; i++) {
        DWORD nFrame = (DWORD)(pBuffer->nPosition >> 32);
        float fFrac = (float)(uint32_t)pBuffer->nPosition * (1.0f / 4294967296.0f);
        DWORD nNext = nFrame + 1;
        if (nNext >= pBuffer->nFrames) {
            nNext = pBuffer->bLooping ? 0 : nFrame;
        }
        float fL0, fR0, fL1, fR1;
        ReadFrame(pBuffer, nFrame, &fL0, &fR0);
        ReadFrame(pBuffer, nNext, &fL1, &fR1);
        pOut[2 * i] += (fL0 + (fL1 - fL0) * fFrac) * fLeftLevel;
        pOut[2 * i + 1] += (fR0 + (fR1 - fR0) * fFrac) * fRightLevel;
        pBuffer->nPosition += nStep;
        if (pBuffer->nPosition >= nEnd) {
            if (pBuffer->bLooping) {
                pBuffer->nPosition %= nEnd;
            } else {
                // A one-shot buffer stops at its end and its cursor goes back to the start.
                pBuffer->bPlaying = false;
                pBuffer->nPosition = 0;
                return;
            }
        }
    }
}

void MixLocked(DirectSoundObject* pDevice, float* pOut, int nFrames) {
    size_t nSamples = (size_t)nFrames * kMixChannels;
    memset(pOut, 0, sizeof(pOut[0]) * nSamples);
    for (size_t i = 0; i < pDevice->nBuffers; i++) {
        SoundBufferObject* pBuffer = pDevice->apBuffers[i];
        if (pBuffer->bPlaying) {
            MixBuffer(pBuffer, pOut, nFrames, pDevice->nMixRate);
        }
    }
    for (size_t i = 0; i < nSamples; i++) {
        pOut[i] = pOut[i] > 1.0f ? 1.0f : pOut[i] < -1.0f ? -1.0f : pOut[i];
    }
}

void SDLCALL DeviceCallback(void* pUser, SDL_AudioStream* pStream, int nAdditional, int nTotal) {
    (void)nTotal;
    DirectSoundObject* pDevice = (DirectSoundObject*)pUser;
    int nFrames = nAdditional / (int)(kMixChannels * sizeof(float));
    if (nFrames <= 0) {
        return;
    }
    if (pDevice->nScratchFrames < (size_t)nFrames) {
        float* pScratch = (float*)realloc(pDevice->pScratch, sizeof(float) * kMixChannels * (size_t)nFrames);
        if (!pScratch) {
            return;
        }
        pDevice->pScratch = pScratch;
        pDevice->nScratchFrames = (size_t)nFrames;
    }
    {
        std::lock_guard<std::mutex> tLock(pDevice->tLock);
        MixLocked(pDevice, pDevice->pScratch, nFrames);
    }
    SDL_PutAudioStreamData(pStream, pDevice->pScratch, nFrames * (int)(kMixChannels * sizeof(float)));
}

SDL_AudioSpec MixSpec(int nRate) {
    SDL_AudioSpec tSpec;
    tSpec.format = SDL_AUDIO_F32;
    tSpec.channels = kMixChannels;
    tSpec.freq = nRate;
    return tSpec;
}

// ---- IDirectSound3DBuffer ----

Sound3DObject* As3D(void* p) {
    return (Sound3DObject*)p;
}

ULONG Buffer_AddRef(void* pThis);
ULONG Buffer_Release(void* pThis);

HRESULT S3D_QueryInterface(void* pThis, const GUID* pIid, void** ppOut) {
    if (!ppOut) {
        return kInvalidParam;
    }
    *ppOut = nullptr;
    Sound3DObject* p3D = As3D(pThis);
    if (SameGuid(pIid, kIID_IDirectSound3DBuffer) || SameGuid(pIid, kIID_IUnknown)) {
        *ppOut = p3D;
        Buffer_AddRef(p3D->pBuffer);
        return kOk;
    }
    if (SameGuid(pIid, kIID_IDirectSoundBuffer)) {
        *ppOut = p3D->pBuffer;
        Buffer_AddRef(p3D->pBuffer);
        return kOk;
    }
    // No IKsPropertySet: the device has no EAX.
    return kNoInterface;
}
ULONG S3D_AddRef(void* pThis) {
    return Buffer_AddRef(As3D(pThis)->pBuffer);
}
ULONG S3D_Release(void* pThis) {
    return Buffer_Release(As3D(pThis)->pBuffer);
}
HRESULT S3D_GetAllParameters(void*, void*) {
    return kNotImpl;
}
HRESULT S3D_GetConeAngles(void* pThis, DWORD* pInside, DWORD* pOutside) {
    if (pInside) *pInside = As3D(pThis)->dwInsideConeAngle;
    if (pOutside) *pOutside = As3D(pThis)->dwOutsideConeAngle;
    return kOk;
}
HRESULT S3D_GetConeOrientation(void* pThis, void* pVector) {
    if (!pVector) return kInvalidParam;
    memcpy(pVector, As3D(pThis)->aConeOrientation, sizeof(As3D(pThis)->aConeOrientation));
    return kOk;
}
HRESULT S3D_GetConeOutsideVolume(void* pThis, LONG* pVolume) {
    if (!pVolume) return kInvalidParam;
    *pVolume = As3D(pThis)->lConeOutsideVolume;
    return kOk;
}
HRESULT S3D_GetMaxDistance(void* pThis, float* pValue) {
    if (!pValue) return kInvalidParam;
    *pValue = As3D(pThis)->fMaxDistance;
    return kOk;
}
HRESULT S3D_GetMinDistance(void* pThis, float* pValue) {
    if (!pValue) return kInvalidParam;
    *pValue = As3D(pThis)->fMinDistance;
    return kOk;
}
HRESULT S3D_GetMode(void* pThis, DWORD* pMode) {
    if (!pMode) return kInvalidParam;
    *pMode = As3D(pThis)->dwMode;
    return kOk;
}
HRESULT S3D_GetPosition(void* pThis, void* pVector) {
    if (!pVector) return kInvalidParam;
    memcpy(pVector, As3D(pThis)->aPosition, sizeof(As3D(pThis)->aPosition));
    return kOk;
}
HRESULT S3D_GetVelocity(void* pThis, void* pVector) {
    if (!pVector) return kInvalidParam;
    memcpy(pVector, As3D(pThis)->aVelocity, sizeof(As3D(pThis)->aVelocity));
    return kOk;
}
HRESULT S3D_SetAllParameters(void*, const void*, DWORD) {
    return kNotImpl;
}
HRESULT S3D_SetConeAngles(void* pThis, DWORD dwInside, DWORD dwOutside, DWORD) {
    As3D(pThis)->dwInsideConeAngle = dwInside;
    As3D(pThis)->dwOutsideConeAngle = dwOutside;
    return kOk;
}
HRESULT S3D_SetConeOrientation(void* pThis, float x, float y, float z, DWORD) {
    float* p = As3D(pThis)->aConeOrientation;
    p[0] = x, p[1] = y, p[2] = z;
    return kOk;
}
HRESULT S3D_SetConeOutsideVolume(void* pThis, LONG lVolume, DWORD) {
    As3D(pThis)->lConeOutsideVolume = lVolume;
    return kOk;
}
HRESULT S3D_SetMaxDistance(void* pThis, float fValue, DWORD) {
    As3D(pThis)->fMaxDistance = fValue;
    return kOk;
}
HRESULT S3D_SetMinDistance(void* pThis, float fValue, DWORD) {
    As3D(pThis)->fMinDistance = fValue;
    return kOk;
}
HRESULT S3D_SetMode(void* pThis, DWORD dwMode, DWORD) {
    As3D(pThis)->dwMode = dwMode;
    return kOk;
}
HRESULT S3D_SetPosition(void* pThis, float x, float y, float z, DWORD) {
    float* p = As3D(pThis)->aPosition;
    p[0] = x, p[1] = y, p[2] = z;
    return kOk;
}
HRESULT S3D_SetVelocity(void* pThis, float x, float y, float z, DWORD) {
    float* p = As3D(pThis)->aVelocity;
    p[0] = x, p[1] = y, p[2] = z;
    return kOk;
}

const Sound3DVtbl gSound3DVtbl = {
    S3D_QueryInterface,  S3D_AddRef,        S3D_Release,          S3D_GetAllParameters,  S3D_GetConeAngles,
    S3D_GetConeOrientation, S3D_GetConeOutsideVolume, S3D_GetMaxDistance, S3D_GetMinDistance, S3D_GetMode,
    S3D_GetPosition,     S3D_GetVelocity,   S3D_SetAllParameters, S3D_SetConeAngles,     S3D_SetConeOrientation,
    S3D_SetConeOutsideVolume, S3D_SetMaxDistance, S3D_SetMinDistance, S3D_SetMode,   S3D_SetPosition,
    S3D_SetVelocity,
};

// ---- IDirectSoundBuffer ----

SoundBufferObject* AsBuffer(void* p) {
    return (SoundBufferObject*)p;
}

ULONG Device_Release(void* pThis);

HRESULT Buffer_QueryInterface(void* pThis, const GUID* pIid, void** ppOut) {
    if (!ppOut) {
        return kInvalidParam;
    }
    *ppOut = nullptr;
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    if (SameGuid(pIid, kIID_IDirectSoundBuffer) || SameGuid(pIid, kIID_IUnknown)) {
        *ppOut = pBuffer;
        Buffer_AddRef(pBuffer);
        return kOk;
    }
    if (SameGuid(pIid, kIID_IDirectSound3DBuffer) && (pBuffer->dwFlags & DSBCAPS_CTRL3D) && !pBuffer->bPrimary) {
        *ppOut = &pBuffer->t3D;
        Buffer_AddRef(pBuffer);
        return kOk;
    }
    return kNoInterface;
}

ULONG Buffer_AddRef(void* pThis) {
    return ++AsBuffer(pThis)->nRefCount;
}

ULONG Buffer_Release(void* pThis) {
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    ULONG nLeft = --pBuffer->nRefCount;
    if (nLeft) {
        return nLeft;
    }
    DirectSoundObject* pDevice = pBuffer->pDevice;
    {
        std::lock_guard<std::mutex> tLock(pDevice->tLock);
        if (pDevice->pPrimary == pBuffer) {
            pDevice->pPrimary = nullptr;
        }
        for (size_t i = 0; i < pDevice->nBuffers; i++) {
            if (pDevice->apBuffers[i] == pBuffer) {
                memmove(&pDevice->apBuffers[i], &pDevice->apBuffers[i + 1],
                        (pDevice->nBuffers - i - 1) * sizeof(pDevice->apBuffers[0]));
                pDevice->nBuffers--;
                break;
            }
        }
    }
    free(pBuffer->pData);
    pBuffer->~SoundBufferObject();
    free(pBuffer);
    // Each buffer holds the device.
    Device_Release(pDevice);
    return 0;
}

HRESULT Buffer_GetCaps(void* pThis, void* pCaps) {
    BufferCaps* pOut = (BufferCaps*)pCaps;
    if (!pOut || pOut->dwSize < sizeof(BufferCaps)) {
        return kInvalidParam;
    }
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    pOut->dwFlags = pBuffer->dwFlags | DSBCAPS_LOCSOFTWARE;
    pOut->dwBufferBytes = pBuffer->nSize;
    pOut->dwUnlockTransferRate = 0;
    pOut->dwPlayCpuOverhead = 0;
    return kOk;
}

HRESULT Buffer_GetCurrentPosition(void* pThis, DWORD* pdwPlay, DWORD* pdwWrite) {
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    DWORD dwPosition = 0;
    if (!pBuffer->bPrimary) {
        std::lock_guard<std::mutex> tLock(pBuffer->pDevice->tLock);
        dwPosition = (DWORD)(pBuffer->nPosition >> 32) * pBuffer->nFrameBytes;
    }
    if (pdwPlay) *pdwPlay = dwPosition;
    if (pdwWrite) *pdwWrite = dwPosition;
    return kOk;
}

HRESULT Buffer_GetFormat(void* pThis, WAVEFORMATEX* pFormat, DWORD dwSize, DWORD* pdwWritten) {
    if (!pFormat && !pdwWritten) {
        return kInvalidParam;
    }
    if (pFormat) {
        if (dwSize < sizeof(WAVEFORMATEX)) {
            return kInvalidParam;
        }
        *pFormat = AsBuffer(pThis)->tFormat;
    }
    if (pdwWritten) {
        *pdwWritten = sizeof(WAVEFORMATEX);
    }
    return kOk;
}

HRESULT Buffer_GetVolume(void* pThis, LONG* plVolume) {
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    if (!plVolume) return kInvalidParam;
    if (!(pBuffer->dwFlags & DSBCAPS_CTRLVOLUME)) return kControlUnavail;
    std::lock_guard<std::mutex> tLock(pBuffer->pDevice->tLock);
    *plVolume = pBuffer->lVolume;
    return kOk;
}

HRESULT Buffer_GetPan(void* pThis, LONG* plPan) {
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    if (!plPan) return kInvalidParam;
    if (!(pBuffer->dwFlags & DSBCAPS_CTRLPAN)) return kControlUnavail;
    std::lock_guard<std::mutex> tLock(pBuffer->pDevice->tLock);
    *plPan = pBuffer->lPan;
    return kOk;
}

HRESULT Buffer_GetFrequency(void* pThis, DWORD* pdwFrequency) {
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    if (!pdwFrequency) return kInvalidParam;
    if (!(pBuffer->dwFlags & DSBCAPS_CTRLFREQUENCY)) return kControlUnavail;
    std::lock_guard<std::mutex> tLock(pBuffer->pDevice->tLock);
    *pdwFrequency = pBuffer->dwFrequency ? pBuffer->dwFrequency : pBuffer->tFormat.nSamplesPerSec;
    return kOk;
}

HRESULT Buffer_GetStatus(void* pThis, DWORD* pdwStatus) {
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    if (!pdwStatus) return kInvalidParam;
    if (pBuffer->bPrimary) {
        *pdwStatus = DSBSTATUS_PLAYING | DSBSTATUS_LOOPING;
        return kOk;
    }
    std::lock_guard<std::mutex> tLock(pBuffer->pDevice->tLock);
    *pdwStatus = pBuffer->bPlaying ? (DSBSTATUS_PLAYING | (pBuffer->bLooping ? DSBSTATUS_LOOPING : 0)) : 0;
    return kOk;
}

HRESULT Buffer_Initialize(void*, void*, const void*) {
    return kAlreadyInitialized;
}

// The buffer's own memory, from the offset to the end and what wraps to the start; without a
// second pointer, only the first part.
HRESULT Buffer_Lock(void* pThis, DWORD dwOffset, DWORD dwBytes, void** ppv1, DWORD* pdw1, void** ppv2, DWORD* pdw2,
                    DWORD dwFlags) {
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    if (pBuffer->bPrimary) {
        return kPrioLevelNeeded;
    }
    if (!ppv1 || !pdw1) {
        return kInvalidParam;
    }
    if (dwFlags & DSBLOCK_FROMWRITECURSOR) {
        Buffer_GetCurrentPosition(pThis, nullptr, &dwOffset);
    }
    if (dwFlags & DSBLOCK_ENTIREBUFFER) {
        dwBytes = pBuffer->nSize;
    }
    if (dwOffset >= pBuffer->nSize || dwBytes == 0 || dwBytes > pBuffer->nSize) {
        return kInvalidParam;
    }
    DWORD dwFirst = pBuffer->nSize - dwOffset < dwBytes ? pBuffer->nSize - dwOffset : dwBytes;
    *ppv1 = pBuffer->pData + dwOffset;
    *pdw1 = dwFirst;
    if (ppv2) {
        *ppv2 = dwFirst < dwBytes ? pBuffer->pData : nullptr;
    }
    if (pdw2) {
        *pdw2 = ppv2 ? dwBytes - dwFirst : 0;
    }
    return kOk;
}

HRESULT Buffer_Play(void* pThis, DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags) {
    (void)dwPriority;
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    if (dwReserved1 != 0) {
        return kInvalidParam;
    }
    if (pBuffer->bPrimary) {
        // The primary buffer is the mixer, which always runs.
        return kOk;
    }
    std::lock_guard<std::mutex> tLock(pBuffer->pDevice->tLock);
    pBuffer->bPlaying = true;
    pBuffer->bLooping = (dwFlags & DSBPLAY_LOOPING) != 0;
    return kOk;
}

HRESULT Buffer_SetCurrentPosition(void* pThis, DWORD dwPosition) {
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    if (pBuffer->bPrimary) {
        return kInvalidCall;
    }
    if (dwPosition >= pBuffer->nSize) {
        return kInvalidParam;
    }
    std::lock_guard<std::mutex> tLock(pBuffer->pDevice->tLock);
    pBuffer->nPosition = (uint64_t)(dwPosition / pBuffer->nFrameBytes) << 32;
    return kOk;
}

bool ValidFormat(const WAVEFORMATEX* pFormat) {
    return pFormat && pFormat->wFormatTag == WAVE_FORMAT_PCM && (pFormat->nChannels == 1 || pFormat->nChannels == 2) &&
           (pFormat->wBitsPerSample == 8 || pFormat->wBitsPerSample == 16) &&
           pFormat->nBlockAlign == pFormat->nChannels * pFormat->wBitsPerSample / 8 &&
           pFormat->nSamplesPerSec >= DSBFREQUENCY_MIN && pFormat->nSamplesPerSec <= DSBFREQUENCY_MAX;
}

// The primary buffer takes the format and the mixer runs at its rate; a secondary refuses it.
HRESULT Buffer_SetFormat(void* pThis, const WAVEFORMATEX* pFormat) {
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    if (!pBuffer->bPrimary) {
        return kInvalidCall;
    }
    if (!ValidFormat(pFormat)) {
        return kInvalidParam;
    }
    DirectSoundObject* pDevice = pBuffer->pDevice;
    // The callback holds the stream's lock and then the device's: the same order here.
    SDL_LockAudioStream(pDevice->pStream);
    {
        std::lock_guard<std::mutex> tLock(pDevice->tLock);
        pBuffer->tFormat = *pFormat;
        pBuffer->tFormat.cbSize = 0;
        pDevice->nMixRate = (int)pFormat->nSamplesPerSec;
    }
    SDL_AudioSpec tSpec = MixSpec((int)pFormat->nSamplesPerSec);
    SDL_SetAudioStreamFormat(pDevice->pStream, &tSpec, nullptr);
    SDL_UnlockAudioStream(pDevice->pStream);
    return kOk;
}

HRESULT Buffer_SetVolume(void* pThis, LONG lVolume) {
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    if (!(pBuffer->dwFlags & DSBCAPS_CTRLVOLUME)) return kControlUnavail;
    if (lVolume < DSBVOLUME_MIN || lVolume > DSBVOLUME_MAX) return kInvalidParam;
    std::lock_guard<std::mutex> tLock(pBuffer->pDevice->tLock);
    pBuffer->lVolume = lVolume;
    UpdateLevels(pBuffer);
    return kOk;
}

HRESULT Buffer_SetPan(void* pThis, LONG lPan) {
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    if (!(pBuffer->dwFlags & DSBCAPS_CTRLPAN)) return kControlUnavail;
    if (lPan < DSBPAN_LEFT || lPan > DSBPAN_RIGHT) return kInvalidParam;
    std::lock_guard<std::mutex> tLock(pBuffer->pDevice->tLock);
    pBuffer->lPan = lPan;
    UpdateLevels(pBuffer);
    return kOk;
}

HRESULT Buffer_SetFrequency(void* pThis, DWORD dwFrequency) {
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    if (!(pBuffer->dwFlags & DSBCAPS_CTRLFREQUENCY) || pBuffer->bPrimary) return kControlUnavail;
    if (dwFrequency != 0 && (dwFrequency < DSBFREQUENCY_MIN || dwFrequency > DSBFREQUENCY_MAX)) return kInvalidParam;
    std::lock_guard<std::mutex> tLock(pBuffer->pDevice->tLock);
    pBuffer->dwFrequency = dwFrequency;
    return kOk;
}

// Stop pauses: the cursor stays where it is (SetCurrentPosition rewinds).
HRESULT Buffer_Stop(void* pThis) {
    SoundBufferObject* pBuffer = AsBuffer(pThis);
    if (pBuffer->bPrimary) {
        return kOk;
    }
    std::lock_guard<std::mutex> tLock(pBuffer->pDevice->tLock);
    pBuffer->bPlaying = false;
    return kOk;
}

// The lock handed out the buffer's own memory: nothing to copy back.
HRESULT Buffer_Unlock(void* pThis, void*, DWORD, void*, DWORD) {
    return AsBuffer(pThis)->bPrimary ? kPrioLevelNeeded : kOk;
}

// The memory is never lost.
HRESULT Buffer_Restore(void*) {
    return kOk;
}

const SoundBufferVtbl gSoundBufferVtbl = {
    Buffer_QueryInterface, Buffer_AddRef,     Buffer_Release,   Buffer_GetCaps,      Buffer_GetCurrentPosition,
    Buffer_GetFormat,      Buffer_GetVolume,  Buffer_GetPan,    Buffer_GetFrequency, Buffer_GetStatus,
    Buffer_Initialize,     Buffer_Lock,       Buffer_Play,      Buffer_SetCurrentPosition, Buffer_SetFormat,
    Buffer_SetVolume,      Buffer_SetPan,     Buffer_SetFrequency, Buffer_Stop,      Buffer_Unlock,
    Buffer_Restore,
};

// ---- IDirectSound ----

DirectSoundObject* AsDevice(void* p) {
    return (DirectSoundObject*)p;
}

HRESULT Device_QueryInterface(void* pThis, const GUID* pIid, void** ppOut) {
    if (!ppOut) {
        return kInvalidParam;
    }
    *ppOut = nullptr;
    if (SameGuid(pIid, kIID_IDirectSound) || SameGuid(pIid, kIID_IUnknown)) {
        *ppOut = pThis;
        ++AsDevice(pThis)->nRefCount;
        return kOk;
    }
    return kNoInterface;
}

ULONG Device_AddRef(void* pThis) {
    return ++AsDevice(pThis)->nRefCount;
}

ULONG Device_Release(void* pThis) {
    DirectSoundObject* pDevice = AsDevice(pThis);
    ULONG nLeft = --pDevice->nRefCount;
    if (nLeft) {
        return nLeft;
    }
    // Destroying the stream closes the device and waits out its callback.
    SDL_DestroyAudioStream(pDevice->pStream);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    free(pDevice->apBuffers);
    free(pDevice->pScratch);
    pDevice->~DirectSoundObject();
    free(pDevice);
    return 0;
}

SoundBufferObject* NewBuffer(DirectSoundObject* pDevice, DWORD dwFlags) {
    void* pMemory = calloc(1, sizeof(SoundBufferObject));
    if (!pMemory) {
        return nullptr;
    }
    SoundBufferObject* pBuffer = new (pMemory) SoundBufferObject();
    pBuffer->lpVtbl = &gSoundBufferVtbl;
    pBuffer->nRefCount = 1;
    pBuffer->pDevice = pDevice;
    pBuffer->dwFlags = dwFlags;
    pBuffer->lVolume = 0;
    pBuffer->lPan = 0;
    UpdateLevels(pBuffer);
    pBuffer->t3D.lpVtbl = &gSound3DVtbl;
    pBuffer->t3D.pBuffer = pBuffer;
    // DS3D's defaults.
    pBuffer->t3D.aConeOrientation[2] = 1.0f;
    pBuffer->t3D.dwInsideConeAngle = 360;
    pBuffer->t3D.dwOutsideConeAngle = 360;
    pBuffer->t3D.fMinDistance = 1.0f;
    pBuffer->t3D.fMaxDistance = 1000000000.0f;
    ++pDevice->nRefCount;
    return pBuffer;
}

bool AddToMixer(DirectSoundObject* pDevice, SoundBufferObject* pBuffer) {
    std::lock_guard<std::mutex> tLock(pDevice->tLock);
    if (pDevice->nBuffers == pDevice->nBufferRoom) {
        size_t nRoom = pDevice->nBufferRoom ? pDevice->nBufferRoom * 2 : 32;
        SoundBufferObject** apBuffers =
            (SoundBufferObject**)realloc(pDevice->apBuffers, nRoom * sizeof(pDevice->apBuffers[0]));
        if (!apBuffers) {
            return false;
        }
        pDevice->apBuffers = apBuffers;
        pDevice->nBufferRoom = nRoom;
    }
    pDevice->apBuffers[pDevice->nBuffers++] = pBuffer;
    return true;
}

HRESULT Device_CreateSoundBuffer(void* pThis, const void* pDescIn, void** ppBuffer, void* pUnkOuter) {
    DirectSoundObject* pDevice = AsDevice(pThis);
    const BufferDesc* pDesc = (const BufferDesc*)pDescIn;
    if (!ppBuffer) {
        return kInvalidParam;
    }
    *ppBuffer = nullptr;
    if (pUnkOuter) {
        return kNoAggregation;
    }
    if (!pDesc || pDesc->dwSize < offsetof(BufferDesc, lpwfxFormat) + sizeof(void*)) {
        return kInvalidParam;
    }
    DWORD dwFlags = pDesc->dwFlags;
    if (dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (pDesc->dwBufferBytes != 0 || pDesc->lpwfxFormat) {
            return kInvalidParam;
        }
        std::lock_guard<std::mutex> tLock(pDevice->tLock);
        // One primary buffer per device: asking again hands out the same one.
        if (pDevice->pPrimary) {
            ++pDevice->pPrimary->nRefCount;
            *ppBuffer = pDevice->pPrimary;
            return kOk;
        }
        SoundBufferObject* pBuffer = NewBuffer(pDevice, dwFlags);
        if (!pBuffer) {
            return kOutOfMemory;
        }
        pBuffer->bPrimary = true;
        pBuffer->tFormat.wFormatTag = WAVE_FORMAT_PCM;
        pBuffer->tFormat.nChannels = 2;
        pBuffer->tFormat.nSamplesPerSec = (DWORD)pDevice->nMixRate;
        pBuffer->tFormat.wBitsPerSample = 8;
        pBuffer->tFormat.nBlockAlign = 2;
        pBuffer->tFormat.nAvgBytesPerSec = (DWORD)pDevice->nMixRate * 2;
        pDevice->pPrimary = pBuffer;
        *ppBuffer = pBuffer;
        return kOk;
    }
    const WAVEFORMATEX* pFormat = pDesc->lpwfxFormat;
    if (!ValidFormat(pFormat) || pDesc->dwBufferBytes < DSBSIZE_MIN || pDesc->dwBufferBytes > DSBSIZE_MAX) {
        return kInvalidParam;
    }
    // DirectSound positions a 3D buffer itself: it must be mono, and has no pan.
    if ((dwFlags & DSBCAPS_CTRL3D) && (pFormat->nChannels != 1 || (dwFlags & DSBCAPS_CTRLPAN))) {
        return kInvalidParam;
    }
    SoundBufferObject* pBuffer = NewBuffer(pDevice, dwFlags & ~DSBCAPS_PRIMARYBUFFER);
    if (!pBuffer) {
        return kOutOfMemory;
    }
    pBuffer->tFormat = *pFormat;
    pBuffer->tFormat.cbSize = 0;
    pBuffer->nFrameBytes = pFormat->nBlockAlign;
    pBuffer->nFrames = pDesc->dwBufferBytes / pFormat->nBlockAlign;
    pBuffer->nSize = pDesc->dwBufferBytes;
    pBuffer->pData = (uint8_t*)calloc(1, pDesc->dwBufferBytes);
    if (!pBuffer->pData) {
        Buffer_Release(pBuffer);
        return kOutOfMemory;
    }
    if (pFormat->wBitsPerSample == 8) {
        memset(pBuffer->pData, 0x80, pDesc->dwBufferBytes);
    }
    if (!AddToMixer(pDevice, pBuffer)) {
        Buffer_Release(pBuffer);
        return kOutOfMemory;
    }
    *ppBuffer = pBuffer;
    return kOk;
}

HRESULT Device_GetCaps(void* pThis, void* pCapsIn) {
    (void)pThis;
    DeviceCaps* pCaps = (DeviceCaps*)pCapsIn;
    if (!pCaps || pCaps->dwSize < sizeof(DeviceCaps)) {
        return kInvalidParam;
    }
    DWORD dwSize = pCaps->dwSize;
    memset(pCaps, 0, sizeof(DeviceCaps));
    pCaps->dwSize = dwSize;
    pCaps->dwFlags = kDeviceCapsFlags;
    pCaps->dwMinSecondarySampleRate = DSBFREQUENCY_MIN;
    pCaps->dwMaxSecondarySampleRate = DSBFREQUENCY_MAX;
    pCaps->dwPrimaryBuffers = 1;
    // Every count of hardware buffers, 3D among them, and of hardware memory is 0: the mixing
    // is software.
    return kOk;
}

HRESULT Device_DuplicateSoundBuffer(void*, void*, void** ppOut) {
    if (ppOut) {
        *ppOut = nullptr;
    }
    return kNotImpl;
}

HRESULT Device_SetCooperativeLevel(void* pThis, HWND hWnd, DWORD dwLevel) {
    (void)hWnd;
    if (dwLevel < 1 || dwLevel > 4) {
        return kInvalidParam;
    }
    AsDevice(pThis)->dwCooperativeLevel = dwLevel;
    return kOk;
}

HRESULT Device_Compact(void*) {
    return kOk;
}

HRESULT Device_GetSpeakerConfig(void*, DWORD* pdwConfig) {
    if (!pdwConfig) {
        return kInvalidParam;
    }
    *pdwConfig = DSSPEAKER_STEREO;
    return kOk;
}

HRESULT Device_SetSpeakerConfig(void*, DWORD) {
    return kOk;
}

HRESULT Device_Initialize(void*, const GUID*) {
    return kAlreadyInitialized;
}

const DirectSoundVtbl gDirectSoundVtbl = {
    Device_QueryInterface, Device_AddRef,  Device_Release,          Device_CreateSoundBuffer,
    Device_GetCaps,        Device_DuplicateSoundBuffer, Device_SetCooperativeLevel, Device_Compact,
    Device_GetSpeakerConfig, Device_SetSpeakerConfig, Device_Initialize,
};

}  // namespace

extern "C" int32_t DirectSoundCreate(const GUID* pcGuidDevice, struct IDirectSound** ppDS, struct IUnknown* pUnkOuter) {
    if (!ppDS) {
        return kInvalidParam;
    }
    *ppDS = nullptr;
    if (pUnkOuter) {
        return kNoAggregation;
    }
    if (pcGuidDevice && !SameGuid(pcGuidDevice, kDefaultPlayback) && !SameGuid(pcGuidDevice, kDefaultVoicePlayback)) {
        return kNoDriver;
    }
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        return kNoDriver;
    }
    void* pMemory = calloc(1, sizeof(DirectSoundObject));
    if (!pMemory) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return kOutOfMemory;
    }
    DirectSoundObject* pDevice = new (pMemory) DirectSoundObject();
    pDevice->lpVtbl = &gDirectSoundVtbl;
    pDevice->nRefCount = 1;
    pDevice->nMixRate = kDefaultMixRate;
    SDL_AudioSpec tSpec = MixSpec(kDefaultMixRate);
    pDevice->pStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &tSpec, DeviceCallback, pDevice);
    if (!pDevice->pStream) {
        pDevice->~DirectSoundObject();
        free(pDevice);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return kNoDriver;
    }
    SDL_ResumeAudioStreamDevice(pDevice->pStream);
    *ppDS = (struct IDirectSound*)pDevice;
    return kOk;
}

// One device, the default output, as "Primary Sound Driver".
extern "C" int32_t DirectSoundEnumerateA(BOOL (*pCallback)(int, char*, char*, DWORD), void* pContext) {
    if (!pCallback) {
        return kInvalidParam;
    }
    char szDescription[] = "Primary Sound Driver";
    char szModule[] = "";
    pCallback(0, szDescription, szModule, (DWORD)(uintptr_t)pContext);
    return kOk;
}

namespace Win32PosixDirectSound {

void PauseDevice(IDirectSound* pDirectSound, bool bPause) {
    DirectSoundObject* pDevice = (DirectSoundObject*)pDirectSound;
    if (bPause) {
        SDL_PauseAudioStreamDevice(pDevice->pStream);
    } else {
        SDL_ResumeAudioStreamDevice(pDevice->pStream);
    }
}

void Mix(IDirectSound* pDirectSound, float* pOut, int nFrames) {
    DirectSoundObject* pDevice = (DirectSoundObject*)pDirectSound;
    std::lock_guard<std::mutex> tLock(pDevice->tLock);
    MixLocked(pDevice, pOut, nFrames);
}

int MixRate(IDirectSound* pDirectSound) {
    DirectSoundObject* pDevice = (DirectSoundObject*)pDirectSound;
    std::lock_guard<std::mutex> tLock(pDevice->tLock);
    return pDevice->nMixRate;
}

}  // namespace Win32PosixDirectSound

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
