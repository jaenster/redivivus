// Win32Posix: a test of the DirectSound layer (DirectSound.cpp) through the COM vtables, as
// D2Sound calls them: the primary buffer and its format, secondary buffers, Lock/Unlock with a
// wrap, volume and pan in hundredths of a decibel, frequency, cursors, looping and one-shot
// ends. The mixed samples are checked with the device paused (DirectSoundMixer.h), then a
// generated tone plays through the real device stream for the cursor to move in real time.
//
// The d2_directsound_test target (cmake/mac-modern.cmake). Run with SDL's dummy audio driver:
// SDL_AUDIO_DRIVER=dummy ./d2_directsound_test (it sets it when the variable is unset). Exits 0
// when every check passes.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include "Source/StormMac/Win32Posix/DirectSoundMixer.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

namespace {

// The COM records as the shared headers declare them (D2Sound.h, Storm.h, Sound.h).
struct DSVtbl {
    HRESULT (*QueryInterface)(void*, GUID*, void**);
    ULONG (*AddRef)(void*);
    ULONG (*Release)(void*);
    HRESULT (*CreateSoundBuffer)(void*, void*, void**, void*);
    HRESULT (*GetCaps)(void*, void*);
    void* DuplicateSoundBuffer;
    HRESULT (*SetCooperativeLevel)(void*, HWND, DWORD);
    void* Compact;
    void* GetSpeakerConfig;
    void* SetSpeakerConfig;
    void* Initialize;
};
struct DS {
    DSVtbl* lpVtbl;
};
struct DSBVtbl {
    HRESULT (*QueryInterface)(void*, GUID*, void**);
    ULONG (*AddRef)(void*);
    ULONG (*Release)(void*);
    HRESULT (*GetCaps)(void*, void*);
    HRESULT (*GetCurrentPosition)(void*, DWORD*, DWORD*);
    void* GetFormat;
    HRESULT (*GetVolume)(void*, LONG*);
    HRESULT (*GetPan)(void*, LONG*);
    void* GetFrequency;
    HRESULT (*GetStatus)(void*, DWORD*);
    void* Initialize;
    HRESULT (*Lock)(void*, DWORD, DWORD, void**, DWORD*, void**, DWORD*, DWORD);
    HRESULT (*Play)(void*, DWORD, DWORD, DWORD);
    HRESULT (*SetCurrentPosition)(void*, DWORD);
    HRESULT (*SetFormat)(void*, void*);
    HRESULT (*SetVolume)(void*, LONG);
    HRESULT (*SetPan)(void*, LONG);
    // Not in the shared header's typed slots (void* there); the layer implements it.
    HRESULT (*SetFrequency)(void*, DWORD);
    HRESULT (*Stop)(void*);
    HRESULT (*Unlock)(void*, void*, DWORD, void*, DWORD);
    HRESULT (*Restore)(void*);
};
struct DSB {
    DSBVtbl* lpVtbl;
};
struct DS3DVtbl {
    HRESULT (*QueryInterface)(void*, GUID*, void**);
    ULONG (*AddRef)(void*);
    ULONG (*Release)(void*);
    void* aGet[9];
    void* aSet[4];
    HRESULT (*SetMaxDistance)(void*, float, DWORD);
    HRESULT (*SetMinDistance)(void*, float, DWORD);
    void* SetMode;
    HRESULT (*SetPosition)(void*, float, float, float, DWORD);
    void* SetVelocity;
};
struct DS3D {
    DS3DVtbl* lpVtbl;
};
struct BufferDesc {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwReserved;
    WAVEFORMATEX* lpwfxFormat;
};
struct BufferCaps {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwUnlockTransferRate;
    DWORD dwPlayCpuOverhead;
};
struct DeviceCaps {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwMinSecondarySampleRate;
    DWORD dwMaxSecondarySampleRate;
    DWORD dwPrimaryBuffers;
    DWORD aRest[19];
};

constexpr HRESULT kInvalidParam = (HRESULT)0x80070057;
constexpr HRESULT kControlUnavail = (HRESULT)0x8878001E;
constexpr HRESULT kNoInterface = (HRESULT)0x80004002;
GUID gIID_3D = {0x279AFA86, 0x4981, 0x11CE, {0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60}};
GUID gIID_KsPropertySet = {0x31EFAC30, 0x515C, 0x11D0, {0xA9, 0xAA, 0x00, 0xAA, 0x00, 0x61, 0xBE, 0x93}};

int gnChecks = 0;
int gnFailures = 0;

void Check(bool bOk, const char* szWhat, double fGot = 0, double fWant = 0) {
    gnChecks++;
    if (!bOk) {
        gnFailures++;
        printf("FAIL %s (got %g, want %g)\n", szWhat, fGot, fWant);
    }
}
void CheckEq(long nGot, long nWant, const char* szWhat) {
    Check(nGot == nWant, szWhat, (double)nGot, (double)nWant);
}
void CheckNear(double fGot, double fWant, double fTolerance, const char* szWhat) {
    Check(fabs(fGot - fWant) <= fTolerance, szWhat, fGot, fWant);
}

void SleepMs(int nMs) {
    struct timespec tSleep = {nMs / 1000, (long)(nMs % 1000) * 1000000};
    nanosleep(&tSleep, nullptr);
}

WAVEFORMATEX Format(WORD nChannels, WORD nBits, DWORD nRate) {
    WAVEFORMATEX tFormat;
    memset(&tFormat, 0, sizeof(tFormat));
    tFormat.wFormatTag = 1;
    tFormat.nChannels = nChannels;
    tFormat.nSamplesPerSec = nRate;
    tFormat.wBitsPerSample = nBits;
    tFormat.nBlockAlign = (WORD)(nChannels * nBits / 8);
    tFormat.nAvgBytesPerSec = nRate * tFormat.nBlockAlign;
    return tFormat;
}

DSB* CreateBuffer(DS* pDS, DWORD dwFlags, WAVEFORMATEX* pFormat, DWORD dwBytes, HRESULT* phResult = nullptr) {
    BufferDesc tDesc;
    memset(&tDesc, 0, sizeof(tDesc));
    tDesc.dwSize = sizeof(tDesc);
    tDesc.dwFlags = dwFlags;
    tDesc.dwBufferBytes = dwBytes;
    tDesc.lpwfxFormat = pFormat;
    DSB* pBuffer = nullptr;
    HRESULT hResult = pDS->lpVtbl->CreateSoundBuffer(pDS, &tDesc, (void**)&pBuffer, nullptr);
    if (phResult) {
        *phResult = hResult;
    }
    return hResult >= 0 ? pBuffer : nullptr;
}

// Fills a 16-bit mono buffer with one value through Lock/Unlock.
void Fill16(DSB* pBuffer, DWORD dwBytes, int16_t nValue) {
    void* p1;
    DWORD n1;
    CheckEq(pBuffer->lpVtbl->Lock(pBuffer, 0, dwBytes, &p1, &n1, nullptr, nullptr, 0), 0, "lock to fill");
    for (DWORD i = 0; i < n1 / 2; i++) {
        ((int16_t*)p1)[i] = nValue;
    }
    pBuffer->lpVtbl->Unlock(pBuffer, p1, n1, nullptr, 0);
}

DWORD PlayCursor(DSB* pBuffer) {
    DWORD dwPlay = 0xdeadbeef, dwWrite = 0xdeadbeef;
    pBuffer->lpVtbl->GetCurrentPosition(pBuffer, &dwPlay, &dwWrite);
    return dwPlay;
}

DWORD Status(DSB* pBuffer) {
    DWORD dwStatus = 0xdeadbeef;
    pBuffer->lpVtbl->GetStatus(pBuffer, &dwStatus);
    return dwStatus;
}

float gaMix[2 * 4096];

void TestMixer(DS* pDS, IDirectSound* pDevice) {
    Win32PosixDirectSound::PauseDevice(pDevice, true);
    // Let a callback that was running finish before the buffers are touched.
    SleepMs(50);

    // A DC buffer of 0.5: the mixer at the buffer's own rate copies it to both sides.
    WAVEFORMATEX tMono16 = Format(1, 16, 22050);
    DSB* pBuffer = CreateBuffer(pDS, 0xe0, &tMono16, 200);
    Check(pBuffer != nullptr, "create a 16-bit mono buffer");
    Fill16(pBuffer, 200, 16384);
    CheckEq(pBuffer->lpVtbl->Play(pBuffer, 0, 0, 0), 0, "play one-shot");
    CheckEq(Status(pBuffer), 1, "status playing");
    Win32PosixDirectSound::Mix(pDevice, gaMix, 40);
    CheckNear(gaMix[0], 0.5, 1e-6, "left of a DC buffer");
    CheckNear(gaMix[79], 0.5, 1e-6, "right of a DC buffer");
    CheckEq(PlayCursor(pBuffer), 80, "cursor after 40 frames");

    // -6 dB: 10^(-600/2000).
    CheckEq(pBuffer->lpVtbl->SetVolume(pBuffer, -600), 0, "SetVolume -600");
    LONG lVolume = 0;
    pBuffer->lpVtbl->GetVolume(pBuffer, &lVolume);
    CheckEq(lVolume, -600, "GetVolume");
    Win32PosixDirectSound::Mix(pDevice, gaMix, 10);
    CheckNear(gaMix[0], 0.5 * pow(10.0, -0.3), 1e-5, "volume -600 level");
    CheckEq(pBuffer->lpVtbl->SetVolume(pBuffer, -10001), kInvalidParam, "SetVolume below the minimum");
    CheckEq(pBuffer->lpVtbl->SetVolume(pBuffer, 1), kInvalidParam, "SetVolume above 0");

    // Pan: right attenuates the left side, left the right side; full is silence.
    pBuffer->lpVtbl->SetVolume(pBuffer, 0);
    CheckEq(pBuffer->lpVtbl->SetPan(pBuffer, 2000), 0, "SetPan 2000");
    Win32PosixDirectSound::Mix(pDevice, gaMix, 10);
    CheckNear(gaMix[0], 0.5 * 0.1, 1e-5, "pan right: left level");
    CheckNear(gaMix[1], 0.5, 1e-6, "pan right: right level");
    pBuffer->lpVtbl->SetPan(pBuffer, -10000);
    Win32PosixDirectSound::Mix(pDevice, gaMix, 10);
    CheckNear(gaMix[0], 0.5, 1e-6, "full left: left level");
    CheckNear(gaMix[1], 0.0, 1e-9, "full left: right silent");
    CheckEq(pBuffer->lpVtbl->SetPan(pBuffer, 10001), kInvalidParam, "SetPan out of range");
    LONG lPan = 0;
    pBuffer->lpVtbl->GetPan(pBuffer, &lPan);
    CheckEq(lPan, -10000, "GetPan");
    pBuffer->lpVtbl->SetPan(pBuffer, 0);

    // The one-shot end: 100 frames, 70 consumed; 30 more play, the rest is silence, the
    // buffer stops and its cursor goes back to 0.
    CheckEq(PlayCursor(pBuffer), 140, "cursor after 70 frames");
    Win32PosixDirectSound::Mix(pDevice, gaMix, 100);
    CheckNear(gaMix[2 * 29], 0.5, 1e-6, "last frame of a one-shot");
    CheckNear(gaMix[2 * 30], 0.0, 1e-9, "silence after a one-shot");
    CheckEq(Status(pBuffer), 0, "one-shot stopped at its end");
    CheckEq(PlayCursor(pBuffer), 0, "one-shot cursor back at 0");

    // Looping: 250 frames of a 100-frame buffer wrap twice and end at frame 50.
    CheckEq(pBuffer->lpVtbl->Play(pBuffer, 0, 0, 1), 0, "play looping");
    CheckEq(Status(pBuffer), 5, "status playing and looping");
    Win32PosixDirectSound::Mix(pDevice, gaMix, 250);
    CheckEq(PlayCursor(pBuffer), 100, "looping cursor wraps");
    CheckNear(gaMix[2 * 249], 0.5, 1e-6, "looping keeps playing");
    CheckEq(Status(pBuffer), 5, "still looping");

    // Stop pauses; SetCurrentPosition moves the cursor (frame-aligned).
    CheckEq(pBuffer->lpVtbl->Stop(pBuffer), 0, "Stop");
    CheckEq(Status(pBuffer), 0, "stopped");
    CheckEq(PlayCursor(pBuffer), 100, "Stop keeps the cursor");
    Win32PosixDirectSound::Mix(pDevice, gaMix, 10);
    CheckNear(gaMix[0], 0.0, 1e-9, "a stopped buffer is silent");
    CheckEq(pBuffer->lpVtbl->SetCurrentPosition(pBuffer, 31), 0, "SetCurrentPosition");
    CheckEq(PlayCursor(pBuffer), 30, "cursor aligned to a frame");
    CheckEq(pBuffer->lpVtbl->SetCurrentPosition(pBuffer, 200), kInvalidParam, "SetCurrentPosition past the end");

    // Frequency: twice the rate moves twice as fast.
    pBuffer->lpVtbl->SetCurrentPosition(pBuffer, 0);
    CheckEq(pBuffer->lpVtbl->SetFrequency(pBuffer, 44100), 0, "SetFrequency");
    pBuffer->lpVtbl->Play(pBuffer, 0, 0, 1);
    Win32PosixDirectSound::Mix(pDevice, gaMix, 25);
    CheckEq(PlayCursor(pBuffer), 100, "cursor at twice the rate");
    CheckEq(pBuffer->lpVtbl->SetFrequency(pBuffer, 50), kInvalidParam, "SetFrequency below the minimum");
    pBuffer->lpVtbl->Stop(pBuffer);

    // Lock across the end: the first part to the end, the second from the start.
    void *p1, *p2;
    DWORD n1, n2;
    CheckEq(pBuffer->lpVtbl->Lock(pBuffer, 150, 100, &p1, &n1, &p2, &n2, 0), 0, "Lock across the end");
    CheckEq(n1, 50, "first part to the end");
    CheckEq(n2, 50, "second part from the start");
    Check(p2 != nullptr && (uint8_t*)p1 - (uint8_t*)p2 == 150, "the parts are the buffer's own memory");
    pBuffer->lpVtbl->Unlock(pBuffer, p1, n1, p2, n2);
    CheckEq(pBuffer->lpVtbl->Lock(pBuffer, 150, 100, &p1, &n1, nullptr, nullptr, 0), 0, "Lock without a second part");
    CheckEq(n1, 50, "only the first part");
    CheckEq(pBuffer->lpVtbl->Lock(pBuffer, 0, 201, &p1, &n1, nullptr, nullptr, 0), kInvalidParam, "Lock more than the buffer");
    CheckEq(pBuffer->lpVtbl->Lock(pBuffer, 0, 0, &p1, &n1, nullptr, nullptr, 2), 0, "Lock the entire buffer");
    CheckEq(n1, 200, "entire buffer size");

    // No 3D interface on a buffer made without DSBCAPS_CTRL3D.
    void* p3D = nullptr;
    CheckEq(pBuffer->lpVtbl->QueryInterface(pBuffer, &gIID_3D, &p3D), kNoInterface, "no 3D without CTRL3D");
    BufferCaps tCaps;
    memset(&tCaps, 0, sizeof(tCaps));
    tCaps.dwSize = sizeof(tCaps);
    CheckEq(pBuffer->lpVtbl->GetCaps(pBuffer, &tCaps), 0, "buffer GetCaps");
    CheckEq(tCaps.dwBufferBytes, 200, "buffer caps size");
    CheckEq(tCaps.dwFlags & 0xe0, 0xe0, "buffer caps flags");
    CheckEq(pBuffer->lpVtbl->Release(pBuffer), 0, "release the buffer");

    // 8-bit stereo: 0xC0 is +0.5, 0x40 is -0.5.
    WAVEFORMATEX tStereo8 = Format(2, 8, 22050);
    DSB* pStereo = CreateBuffer(pDS, 0x80, &tStereo8, 64);
    Check(pStereo != nullptr, "create an 8-bit stereo buffer");
    CheckEq(pStereo->lpVtbl->Lock(pStereo, 0, 64, &p1, &n1, nullptr, nullptr, 0), 0, "lock 8-bit");
    for (DWORD i = 0; i < n1; i += 2) {
        ((uint8_t*)p1)[i] = 0xC0;
        ((uint8_t*)p1)[i + 1] = 0x40;
    }
    pStereo->lpVtbl->Unlock(pStereo, p1, n1, nullptr, 0);
    CheckEq(pStereo->lpVtbl->SetPan(pStereo, 0), kControlUnavail, "SetPan without CTRLPAN");
    pStereo->lpVtbl->Play(pStereo, 0, 0, 1);
    Win32PosixDirectSound::Mix(pDevice, gaMix, 4);
    CheckNear(gaMix[0], 0.5, 1e-6, "8-bit left");
    CheckNear(gaMix[1], -0.5, 1e-6, "8-bit right");
    pStereo->lpVtbl->Release(pStereo);

    // A 3D buffer (mono, no pan): its 3D interface takes the game's calls.
    DSB* p3DBuffer = CreateBuffer(pDS, 0x80b4 & ~0x40u, &tMono16, 0x400);
    Check(p3DBuffer != nullptr, "create a 3D buffer");
    if (p3DBuffer) {
        DS3D* pInterface = nullptr;
        CheckEq(p3DBuffer->lpVtbl->QueryInterface(p3DBuffer, &gIID_3D, (void**)&pInterface), 0, "QueryInterface 3D");
        if (pInterface) {
            CheckEq(pInterface->lpVtbl->SetMaxDistance(pInterface, 100.0f, 0), 0, "SetMaxDistance");
            CheckEq(pInterface->lpVtbl->SetMinDistance(pInterface, 2.0f, 0), 0, "SetMinDistance");
            CheckEq(pInterface->lpVtbl->SetPosition(pInterface, 1.0f, 2.0f, 3.0f, 0), 0, "SetPosition");
            void* pEax = nullptr;
            CheckEq(pInterface->lpVtbl->QueryInterface(pInterface, &gIID_KsPropertySet, &pEax), kNoInterface,
                    "no EAX property set");
            CheckEq(pInterface->lpVtbl->Release(pInterface), 1, "release the 3D interface");
        }
        p3DBuffer->lpVtbl->Release(p3DBuffer);
    }
    HRESULT hResult;
    WAVEFORMATEX tStereo16 = Format(2, 16, 22050);
    Check(CreateBuffer(pDS, 0x10, &tStereo16, 0x400, &hResult) == nullptr, "a stereo 3D buffer is refused");
    CheckEq(hResult, kInvalidParam, "stereo 3D buffer error");
    WAVEFORMATEX tBad = Format(1, 12, 22050);
    Check(CreateBuffer(pDS, 0x80, &tBad, 0x400, &hResult) == nullptr, "a 12-bit buffer is refused");

    Win32PosixDirectSound::PauseDevice(pDevice, false);
}

// A 441 Hz tone, looping, through the device stream in real time.
void TestTone(DS* pDS) {
    const DWORD nRate = 22050;
    const DWORD nFrames = nRate / 4;  // a quarter second
    WAVEFORMATEX tFormat = Format(1, 16, nRate);
    DSB* pTone = CreateBuffer(pDS, 0xe0, &tFormat, nFrames * 2);
    Check(pTone != nullptr, "create the tone buffer");
    if (!pTone) {
        return;
    }
    void* p1;
    DWORD n1;
    pTone->lpVtbl->Lock(pTone, 0, nFrames * 2, &p1, &n1, nullptr, nullptr, 0);
    for (DWORD i = 0; i < n1 / 2; i++) {
        ((int16_t*)p1)[i] = (int16_t)(12000.0 * sin(2.0 * M_PI * 441.0 * i / nRate));
    }
    pTone->lpVtbl->Unlock(pTone, p1, n1, nullptr, 0);
    pTone->lpVtbl->SetVolume(pTone, -1200);
    CheckEq(pTone->lpVtbl->Play(pTone, 0, 0, 1), 0, "play the tone");
    DWORD dwLast = PlayCursor(pTone);
    int nWraps = 0;
    DWORD dwAdvanced = 0;
    for (int i = 0; i < 40; i++) {
        SleepMs(25);
        DWORD dwNow = PlayCursor(pTone);
        Check(dwNow < nFrames * 2, "cursor inside the buffer", dwNow, nFrames * 2);
        Check(dwNow % 2 == 0, "cursor on a frame", dwNow, 0);
        if (dwNow < dwLast) {
            nWraps++;
            dwAdvanced += nFrames * 2 - dwLast + dwNow;
        } else {
            dwAdvanced += dwNow - dwLast;
        }
        dwLast = dwNow;
    }
    // A second of play: four times round a quarter-second buffer, give or take the device's
    // buffering.
    Check(nWraps >= 2, "the looping tone wrapped", nWraps, 4);
    CheckNear((double)dwAdvanced / (nRate * 2), 1.0, 0.5, "seconds played in a second");
    CheckEq(Status(pTone), 5, "tone still looping");
    pTone->lpVtbl->Stop(pTone);
    DWORD dwStopped = PlayCursor(pTone);
    SleepMs(100);
    CheckEq(PlayCursor(pTone), dwStopped, "a stopped cursor stays");
    pTone->lpVtbl->Release(pTone);
}

}  // namespace

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    setenv("SDL_AUDIO_DRIVER", "dummy", 0);

    DS* pDS = nullptr;
    HRESULT hResult = DirectSoundCreate(nullptr, (IDirectSound**)&pDS, nullptr);
    CheckEq(hResult, 0, "DirectSoundCreate");
    if (!pDS) {
        printf("FAIL: no device\n");
        return 1;
    }
    CheckEq(pDS->lpVtbl->SetCooperativeLevel(pDS, nullptr, 2), 0, "SetCooperativeLevel");

    // The primary buffer: no memory, takes the format, locks fail (as ClearBuffer expects).
    BufferDesc tPrimaryDesc;
    memset(&tPrimaryDesc, 0, sizeof(tPrimaryDesc));
    tPrimaryDesc.dwSize = sizeof(tPrimaryDesc);
    tPrimaryDesc.dwFlags = 1;
    DSB* pPrimary = nullptr;
    CheckEq(pDS->lpVtbl->CreateSoundBuffer(pDS, &tPrimaryDesc, (void**)&pPrimary, nullptr), 0, "create the primary");
    DeviceCaps tCaps;
    memset(&tCaps, 0, sizeof(tCaps));
    tCaps.dwSize = 0x60;
    CheckEq(pDS->lpVtbl->GetCaps(pDS, &tCaps), 0, "device GetCaps");
    CheckEq(tCaps.aRest[(0x2c - 0x14) / 4], 0, "no hardware 3D buffers");
    CheckEq(tCaps.dwPrimaryBuffers, 1, "one primary buffer");
    WAVEFORMATEX tPrimaryFormat = Format(2, 16, 0x5622);
    CheckEq(pPrimary->lpVtbl->SetFormat(pPrimary, &tPrimaryFormat), 0, "primary SetFormat");
    CheckEq(Win32PosixDirectSound::MixRate((IDirectSound*)pDS), 22050, "mix rate from the primary");
    void* p1;
    DWORD n1 = 0;
    Check(pPrimary->lpVtbl->Lock(pPrimary, 0, 0, &p1, &n1, nullptr, nullptr, 0) < 0, "primary Lock fails");
    CheckEq(pPrimary->lpVtbl->Play(pPrimary, 0, 0, 1), 0, "primary Play");

    TestMixer(pDS, (IDirectSound*)pDS);
    TestTone(pDS);

    CheckEq(pPrimary->lpVtbl->Release(pPrimary), 0, "release the primary");
    CheckEq(pDS->lpVtbl->Release(pDS), 0, "release the device");
    printf("%s: %d checks, %d failed\n", gnFailures ? "FAIL" : "OK", gnChecks, gnFailures);
    return gnFailures ? 1 : 0;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
