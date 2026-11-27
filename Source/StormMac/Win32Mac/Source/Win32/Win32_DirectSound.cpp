// StormMac Win32 Mac/Source/Win32/Win32_DirectSound.cpp: the DirectDraw and DirectSound
// entry points Storm's sound layer calls, on the Sound Manager, 1.14d Intel Mac
// 00032b50..00033a24 (DirectDrawCreate .. IKsPropertySet_Set).
//
// DirectDraw is a four-byte token. IDirectSound owns two pools of Sound Manager channels
// (12 and 4); an IDirectSoundBuffer plays through one of them with a callback that queues
// the next chunk of its data, up to 0x2000 frames at a time when it loops. The original
// name of this file is not recorded; see Source/StormMac/README.md.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <math.h>
#include <new>
#include <string.h>

#define S_OK_ 0
#define E_FAIL_ ((int32_t)0x80004005)
#define E_NOINTERFACE_ ((int32_t)0x80004002)
#define E_NOTIMPL_ ((int32_t)0x80004001)
#define E_INVALIDARG_ ((int32_t)0x80070057)
#define E_OUTOFMEMORY_ ((int32_t)0x8007000e)
#define DSERR_INVALIDCALL_ ((int32_t)0x88780032)
#define DSERR_BUFFERLOST_ ((int32_t)0x88780096)

#define DSBCAPS_PRIMARYBUFFER_ 0x1
#define DSBCAPS_STATIC_ 0x2
#define DSBPLAY_LOOPING_ 0x1
#define DSBLOCK_FROMWRITECURSOR_ 0x1

// A memory file (Win32_memfile: handles into the layer's own locked blocks).
// 1.14d mac: 00036440
int GetMemFile2Ptr(int hMemFile);
// 1.14d mac: 0003645d
int AllocateMemFile2(int nSize, BYTE bStatic);
// 1.14d mac: 00036580
void FreeMemFile2(int hMemFile);
// 1.14d mac: 0003661b
int LockMemFile2(int hMemFile);
// 1.14d mac: 000367c0
void UnLockMemFile2(int hMemFile);

// One Sound Manager channel of a pool and the buffer playing on it.
struct SoundChannelSlot {
    SndChannelPtr pChannel;  // 0x00
    struct IDirectSoundBuffer* pOwner;  // 0x04  NULL: free
    BYTE bFlag;              // 0x08
    BYTE pad09[3];
};

struct IDirectSound {
    int nRefCount;                      // 0x00
    SoundChannelSlot aPool[12];         // 0x04
    SoundChannelSlot aPoolSmall[4];     // 0x94
    SndCallBackUPP pfnCallback;         // 0xC4
};
WIN32MAC_LAYOUT(IDirectSound_size, sizeof(IDirectSound) == 200);

// The sample data of a buffer, shared between buffers that duplicate each other.
struct SoundData {
    int nSize;       // 0x00
    int hMemFile;    // 0x04
    int nRefCount;   // 0x08
};

#pragma pack(push, 2)
// The Sound Manager's extended header; the standard header is its first 0x16 bytes.
struct SoundHeaderBlock {
    void* pSamples;           // 0x00
    uint32_t nChannelsOrLen;  // 0x04
    uint32_t nSampleRate;     // 0x08
    uint32_t nLoopStart;      // 0x0C
    uint32_t nLoopEnd;        // 0x10
    uint8_t nEncode;          // 0x14
    uint8_t nBaseFreq;        // 0x15
    uint32_t nFrames;         // 0x16
    uint8_t aSampleRate[10];  // 0x1A
    uint32_t nMarkerChunk;    // 0x24
    uint32_t nInstruments;    // 0x28
    uint32_t nAESRecording;   // 0x2C
    uint16_t nSampleSize;     // 0x30
    uint8_t aFuture[14];      // 0x32
};
#pragma pack(pop)

struct IDirectSoundBuffer {
    int nRefCount;                  // 0x00
    int n04;                        // 0x04
    IDirectSound* pDirectSound;     // 0x08
    SndChannelPtr pChannel;         // 0x0C
    SoundData* pData;               // 0x10
    int n14;                        // 0x14
    uint16_t wFormatTag;            // 0x18
    uint16_t nChannels;             // 0x1A
    uint32_t nSamplesPerSec;        // 0x1C
    uint32_t nAvgBytesPerSec;       // 0x20
    uint16_t nBlockAlign;           // 0x24
    uint16_t wBitsPerSample;        // 0x26
    uint16_t cbSize;                // 0x28
    uint8_t pad2A[2];
    int nVolume;                    // 0x2C  hundredths of a decibel, -10000..0
    int nPan;                       // 0x30  hundredths of a decibel, -10000..10000
    uint32_t nLevelLeft;            // 0x34
    uint32_t nLevelRight;           // 0x38
    uint32_t nAppliedLeft;          // 0x3C
    uint32_t nAppliedRight;         // 0x40
    float fGain;                    // 0x44
    uint32_t dwFlags;               // 0x48
    int nPlayCursor;                // 0x4C
    int nWriteCursor;               // 0x50
    int nChunkBytes;                // 0x54
    int bLooping;                   // 0x58
    int bStopping;                  // 0x5C
    int bStarted;                   // 0x60  set by the 'MH' callback
    SoundHeaderBlock tHeader;       // 0x64
    int nA4;                        // 0xA4
    int nA5;                        // 0xA8
};
WIN32MAC_LAYOUT(IDirectSoundBuffer_size, sizeof(IDirectSoundBuffer) == 0xAC);
WIN32MAC_LAYOUT(IDirectSoundBuffer_header, offsetof(IDirectSoundBuffer, tHeader) == 0x64);
WIN32MAC_LAYOUT(IDirectSoundBuffer_frames, offsetof(IDirectSoundBuffer, tHeader.nFrames) == 0x7A);
// WAVEFORMATEX as the buffer stores it: wFormatTag through cbSize.
static const size_t kWaveFormatBytes = offsetof(IDirectSoundBuffer, pad2A) - offsetof(IDirectSoundBuffer, wFormatTag);

// 1.14d mac: 003f481c
// The tick count when the minimum-volume time was set.
static DWORD s_dwMinVolumeStart;
// 1.14d mac: 003f4820
// How long, in milliseconds, channels start at minimum volume.
static DWORD s_dwMinVolumeDelay;
// 1.14d mac: 003f4824
// Set: channels start at minimum volume and SetVolume leaves the levels alone.
static int s_bMinVolume;
// 1.14d mac: 003f4828
// The primary buffer, which owns no memory.
static IDirectSoundBuffer* s_pPrimaryBuffer;

void IDirectSound_Construct(IDirectSound* pDirectSound);
void IDirectSound_Destroy(IDirectSound* pDirectSound);
static void IDirectSound_SndChannelCallback(SndChannelPtr pChannel, SndCommand* pCommand);
SndChannelPtr IDirectSound_AcquireSoundChannel(IDirectSound* pDirectSound, IDirectSoundBuffer* pBuffer);
static int IDirectSound_GetFreeChannelFromPool(IDirectSound* pDirectSound, SoundChannelSlot* pPool, int nCount);
static void IDirectSound_ReleaseSoundChannel(IDirectSound* pDirectSound, SndChannelPtr pChannel);
int32_t IDirectSoundBuffer_Stop(IDirectSoundBuffer* pBuffer);
static short IDirectSoundBuffer_UpdatePanAndVolume(IDirectSoundBuffer* pBuffer);

// 1.14d mac: 00032b50
// The DirectDraw object is a token: nothing draws through it.
int DirectDrawCreate(void* lpGuid, int** ppObject, void* pUnkOuter) {
    if (lpGuid == 0 && pUnkOuter == 0) {
        int* pObject = (int*)operator new(4);
        *pObject = 1;
        *ppObject = pObject;
    }
    return 0;
}

// 1.14d mac: 00032b83
int32_t DirectSoundCreate(void* lpGuid, IDirectSound** ppDS, void* pUnkOuter) {
    int32_t hResult = E_INVALIDARG_;
    if (lpGuid == 0 && pUnkOuter == 0) {
        IDirectSound* pDirectSound = (IDirectSound*)operator new(200);
        IDirectSound_Construct(pDirectSound);
        *ppDS = pDirectSound;
        hResult = E_OUTOFMEMORY_;
        if (pDirectSound != 0) {
            hResult = S_OK_;
        }
    }
    return hResult;
}

// 1.14d mac: 00032bcd
// Makes the channels start at minimum volume from the next Play on, and keeps them there
// once dwDelay milliseconds have passed (zero: for good, when the top bit is set).
void DirectSound_SetMinVolumeDelay(DWORD dwUnused, DWORD dwDelay) {
    s_dwMinVolumeStart = GetTickCount();
    s_dwMinVolumeDelay = dwDelay;
    s_bMinVolume = dwDelay >> 31;
}

// 1.14d mac: 00032bfc
void IDirectSound_Construct(IDirectSound* pDirectSound) {
    pDirectSound->nRefCount = 1;
    for (int i = 0; i < 12; i++) {
        pDirectSound->aPool[i].pChannel = 0;
        pDirectSound->aPool[i].pOwner = 0;
        pDirectSound->aPool[i].bFlag = 0;
    }
    for (int i = 0; i < 4; i++) {
        pDirectSound->aPoolSmall[i].pChannel = 0;
        pDirectSound->aPoolSmall[i].pOwner = 0;
        pDirectSound->aPoolSmall[i].bFlag = 0;
    }
    pDirectSound->pfnCallback = (SndCallBackUPP)IDirectSound_SndChannelCallback;
}

// 1.14d mac: 00032c68
// The channel's callback. 'SH' marks the end of a chunk: advance the play cursor and queue
// the next one, or give the channel back. 'MH' says playback has started.
static void IDirectSound_SndChannelCallback(SndChannelPtr pChannel, SndCommand* pCommand) {
    IDirectSoundBuffer* pBuffer = (IDirectSoundBuffer*)pCommand->param2;
    if (pCommand->param1 == 0x5348) {
        if (pBuffer->pChannel != 0 && pBuffer->bStopping == 0) {
            pBuffer->nPlayCursor += pBuffer->nChunkBytes;
            if (pBuffer->bLooping != 0) {
                int nData = GetMemFile2Ptr(pBuffer->pData->hMemFile);
                int nPos = pBuffer->nPlayCursor;
                int nLeft = pBuffer->pData->nSize - nPos;
                if (nLeft < 1) {
                    pBuffer->nPlayCursor = 0;
                    nLeft = pBuffer->pData->nSize;
                    nPos = 0;
                }
                int nChunk;
                if (pBuffer->wBitsPerSample == 8 && pBuffer->nChannels == 1) {
                    pBuffer->tHeader.pSamples = (void*)(nData + nPos);
                    nChunk = 0x2000;
                    if (nLeft < 0x2001) {
                        nChunk = nLeft;
                    }
                    pBuffer->tHeader.nChannelsOrLen = nChunk;
                } else {
                    pBuffer->tHeader.pSamples = (void*)(nData + nPos);
                    int nFrameBytes = (pBuffer->tHeader.nSampleSize >> 3) * pBuffer->tHeader.nChannelsOrLen;
                    int nFrames = nLeft / nFrameBytes;
                    nChunk = 0x2000;
                    if (nFrames < 0x2001) {
                        nChunk = nFrames;
                    }
                    pBuffer->tHeader.nFrames = nChunk;
                    nChunk = nChunk * nFrameBytes;
                }
                pBuffer->nChunkBytes = nChunk;
                uint32_t nLeftLevel;
                int nRightLevel;
                if (s_bMinVolume == 0 && s_dwMinVolumeDelay != 0 &&
                    s_dwMinVolumeDelay <= FPMilliseconds() - s_dwMinVolumeStart) {
                    s_bMinVolume = 1;
                    nLeftLevel = 1;
                    nRightLevel = 1;
                } else {
                    nLeftLevel = 1;
                    nRightLevel = 1;
                    if (s_bMinVolume == 0) {
                        nLeftLevel = pBuffer->nLevelLeft;
                        nRightLevel = pBuffer->nLevelRight;
                    }
                }
                SndCommand tCmd;
                if (nLeftLevel != pBuffer->nAppliedLeft || nRightLevel != pBuffer->nAppliedRight) {
                    tCmd.cmd = 0x2e;
                    tCmd.param1 = 0;
                    tCmd.param2 = nRightLevel << 16 | nLeftLevel;
                    if (SndDoCommand(pBuffer->pChannel, &tCmd, 1) == 0) {
                        pBuffer->nAppliedLeft = nLeftLevel;
                        pBuffer->nAppliedRight = nRightLevel;
                    }
                }
                tCmd.cmd = 0x51;
                tCmd.param1 = 0;
                tCmd.param2 = (long)&pBuffer->tHeader;
                SndDoCommand(pBuffer->pChannel, &tCmd, 1);
                tCmd.cmd = 0xd;
                tCmd.param1 = 0x5348;
                tCmd.param2 = (long)pBuffer;
                SndDoCommand(pBuffer->pChannel, &tCmd, 1);
                if (pBuffer->bLooping != 0) {
                    return;
                }
                if (pBuffer->bStopping != 0) {
                    return;
                }
            }
            IDirectSound_ReleaseSoundChannel(pBuffer->pDirectSound, pChannel);
            pBuffer->pChannel = 0;
            UnLockMemFile2(pBuffer->pData->hMemFile);
        }
    } else if (pCommand->param1 == 0x4d48) {
        pBuffer->bStarted = 1;
    }
}

// 1.14d mac: 00032e78
void IDirectSound_Destroy(IDirectSound* pDirectSound) {
    for (int i = 0; i < 12; i++) {
        if (pDirectSound->aPool[i].pOwner != 0) {
            operator delete(pDirectSound->aPool[i].pOwner);
            pDirectSound->aPool[i].pOwner = 0;
        }
    }
    for (int i = 0; i < 4; i++) {
        if (pDirectSound->aPoolSmall[i].pOwner != 0) {
            operator delete(pDirectSound->aPoolSmall[i].pOwner);
            pDirectSound->aPoolSmall[i].pOwner = 0;
        }
    }
    for (int i = 0; i < 12; i++) {
        if (pDirectSound->aPool[i].pChannel != 0) {
            SndDisposeChannel(pDirectSound->aPool[i].pChannel, true);
            pDirectSound->aPool[i].pChannel = 0;
        }
    }
    for (int i = 0; i < 4; i++) {
        if (pDirectSound->aPoolSmall[i].pChannel != 0) {
            SndDisposeChannel(pDirectSound->aPoolSmall[i].pChannel, true);
            pDirectSound->aPoolSmall[i].pChannel = 0;
        }
    }
}

// 1.14d mac: 00032f3c
int32_t IDirectSound_Release(IDirectSound* pDirectSound) {
    pDirectSound->nRefCount--;
    if (pDirectSound->nRefCount == 0 && pDirectSound != 0) {
        IDirectSound_Destroy(pDirectSound);
        operator delete(pDirectSound);
    }
    return 0;
}

// 1.14d mac: 00032f64
// A buffer with refcount 1. The primary buffer owns no memory; any other gets a data block
// and a memory file 0x100 bytes larger than asked, locked unless it is static.
int32_t IDirectSound_CreateSoundBuffer(IDirectSound* pDirectSound, const uint32_t* pBufferDesc,
                                       IDirectSoundBuffer** ppBuffer) {
    IDirectSoundBuffer* pBuffer = (IDirectSoundBuffer*)operator new(0xac);
    pBuffer->pChannel = 0;
    pBuffer->n14 = 0;
    pBuffer->nRefCount = 1;
    pBuffer->nVolume = 0;
    pBuffer->fGain = 1.0f;
    pBuffer->nPan = 0;
    pBuffer->n04 = 0;
    int32_t hResult = E_OUTOFMEMORY_;
    if (pBuffer != 0) {
        uint32_t dwFlags = pBufferDesc[1];
        pBuffer->dwFlags = dwFlags;
        pBuffer->nWriteCursor = 0;
        pBuffer->nPlayCursor = 0;
        pBuffer->pDirectSound = pDirectSound;
        if ((dwFlags & DSBCAPS_PRIMARYBUFFER_) == 0) {
            const WORD* pFormat = (const WORD*)pBufferDesc[4];
            memcpy(&pBuffer->wFormatTag, pFormat, kWaveFormatBytes);
            SoundData* pData = (SoundData*)operator new(12);
            int nSize = pBufferDesc[2];
            pData->nSize = nSize;
            int hMemFile = AllocateMemFile2(nSize + 0x100, (BYTE)((dwFlags & DSBCAPS_STATIC_) >> 1));
            pData->hMemFile = hMemFile;
            pData->nRefCount = 1;
            pBuffer->pData = pData;
            if (pData == 0) {
                operator delete(pBuffer);
                pBuffer = 0;
                hResult = E_OUTOFMEMORY_;
            } else {
                hResult = E_OUTOFMEMORY_;
                if (hMemFile == 0) {
                    operator delete(pData);
                    operator delete(pBuffer);
                    pBuffer = 0;
                } else {
                    hResult = S_OK_;
                    if ((pBuffer->dwFlags & DSBCAPS_STATIC_) == 0) {
                        LockMemFile2(hMemFile);
                    }
                }
            }
        } else {
            pBuffer->pData = 0;
            hResult = S_OK_;
            s_pPrimaryBuffer = pBuffer;
        }
    }
    *ppBuffer = pBuffer;
    return hResult;
}

// 1.14d mac: 000330ae
int32_t IDirectSound_GetCaps(IDirectSound* pDirectSound, uint32_t* pCaps) {
    pCaps[1] = 0x20;
    return 0;
}

// 1.14d mac: 000330c0
int32_t IDirectSound_SetCooperativeLevel(void* pDirectSound, void* pWindow, uint32_t dwLevel) {
    return 0;
}

// 1.14d mac: 000330c8
int32_t IDirectSoundBuffer_Release(IDirectSoundBuffer* pBuffer) {
    pBuffer->nRefCount--;
    if (pBuffer->nRefCount == 0) {
        IDirectSoundBuffer_Stop(pBuffer);
        SoundData* pData = pBuffer->pData;
        if (pData != 0) {
            if ((pBuffer->dwFlags & DSBCAPS_STATIC_) == 0) {
                UnLockMemFile2(pData->hMemFile);
                pData = pBuffer->pData;
            }
            pData->nRefCount--;
            if (pData->nRefCount == 0 && pData != 0) {
                if (pData->hMemFile != 0) {
                    FreeMemFile2(pData->hMemFile);
                }
                operator delete(pData);
            }
        }
        if (s_pPrimaryBuffer == pBuffer) {
            s_pPrimaryBuffer = 0;
        }
        if (pBuffer != 0) {
            operator delete(pBuffer);
        }
    }
    return 0;
}

// 1.14d mac: 0003314e
// Flushes and quiets the channel, gives it back and rewinds the cursors.
int32_t IDirectSoundBuffer_Stop(IDirectSoundBuffer* pBuffer) {
    pBuffer->bStopping = 1;
    pBuffer->bLooping = 0;
    if (pBuffer->pChannel != 0) {
        SndCommand tCmd;
        tCmd.cmd = 4;
        tCmd.param1 = 0;
        tCmd.param2 = 0;
        SndDoImmediate(pBuffer->pChannel, &tCmd);
        tCmd.cmd = 3;
        SndDoImmediate(pBuffer->pChannel, &tCmd);
        IDirectSound_ReleaseSoundChannel(pBuffer->pDirectSound, pBuffer->pChannel);
        pBuffer->pChannel = 0;
        UnLockMemFile2(pBuffer->pData->hMemFile);
    }
    pBuffer->nPlayCursor = 0;
    pBuffer->nWriteCursor = 0;
    pBuffer->bStopping = 0;
    return 0;
}

// 1.14d mac: 000331ea
int32_t IDirectSoundBuffer_QueryInterface(void* pBuffer, void* pIid, void** ppInterface) {
    return E_NOINTERFACE_;
}

// 1.14d mac: 000331f4
int32_t IDirectSoundBuffer_GetCaps(IDirectSoundBuffer* pBuffer, uint32_t* pCaps) {
    int32_t hResult = E_FAIL_;
    if (pCaps != 0 && *pCaps > 0x13) {
        if (s_pPrimaryBuffer == pBuffer || pBuffer->pData == 0) {
            pCaps[2] = 0;
        } else {
            pCaps[2] = pBuffer->pData->nSize;
        }
        pCaps[1] = 0xe8;
        pCaps[3] = 0xa000;
        pCaps[4] = 1;
        hResult = 0;
    }
    return hResult;
}

// 1.14d mac: 00033248
int32_t IDirectSoundBuffer_GetCurrentPosition(IDirectSoundBuffer* pBuffer, DWORD* pdwPlayCursor,
                                              DWORD* pdwWriteCursor) {
    if (pdwPlayCursor != 0) {
        *pdwPlayCursor = pBuffer->nPlayCursor;
        if (pBuffer->bStarted != 0 && pBuffer->nPlayCursor == 0) {
            *pdwPlayCursor = 1;
        }
    }
    if (pdwWriteCursor != 0) {
        *pdwWriteCursor = pBuffer->nWriteCursor;
    }
    return 0;
}

// 1.14d mac: 0003327c
int32_t IDirectSoundBuffer_GetStatus(IDirectSoundBuffer* pBuffer, DWORD* pdwStatus) {
    *pdwStatus = 0;
    if (pBuffer->pChannel != 0) {
        SCStatus tStatus;
        if (SndChannelStatus(pBuffer->pChannel, 0x18, &tStatus) == 0 && tStatus.scChannelBusy != 0) {
            *pdwStatus = 1;
        }
    }
    return 0;
}

// 1.14d mac: 000332c6
// The two regions of a lock: the range clipped at the end of the buffer and what wraps.
int32_t IDirectSoundBuffer_Lock(IDirectSoundBuffer* pBuffer, int dwOffset, int dwBytes, int* ppvAudioPtr1,
                                int* pdwAudioBytes1, int* ppvAudioPtr2, int* pdwAudioBytes2, uint32_t dwFlags) {
    int32_t hResult = E_FAIL_;
    if (s_pPrimaryBuffer != pBuffer) {
        hResult = 0;
        if ((dwFlags & DSBLOCK_FROMWRITECURSOR_) == 0) {
            uint32_t dwBufferSize = pBuffer->pData->nSize;
            int nOver = (dwBytes + dwOffset) - dwBufferSize;
            int nWrapBytes = 0;
            if (dwBufferSize <= (uint32_t)(dwBytes + dwOffset) && nOver != 0) {
                dwBytes = dwBufferSize - dwOffset;
                nWrapBytes = nOver;
            }
            int nData = LockMemFile2(pBuffer->pData->hMemFile);
            if (ppvAudioPtr1 != 0) {
                *ppvAudioPtr1 = dwOffset + nData;
            }
            if (pdwAudioBytes1 != 0) {
                *pdwAudioBytes1 = dwBytes;
            }
            if (ppvAudioPtr2 != 0) {
                *ppvAudioPtr2 = nData;
            }
            hResult = 0;
            if (pdwAudioBytes2 != 0) {
                *pdwAudioBytes2 = nWrapBytes;
            }
        }
    }
    return hResult;
}

// 1.14d mac: 00033344
// Starts playing: takes a channel, locks the data, fills a standard (8 bit mono) or an
// extended sound header from the wave format and queues the header and the callbacks.
// Looping plays from the cursor in chunks of at most 0x2000 frames.
int32_t IDirectSoundBuffer_Play(IDirectSoundBuffer* pBuffer, DWORD dwReserved1, DWORD dwReserved2, uint32_t dwFlags) {
    int32_t hResult = E_FAIL_;
    if (s_pPrimaryBuffer != pBuffer) {
        pBuffer->pChannel = IDirectSound_AcquireSoundChannel(pBuffer->pDirectSound, pBuffer);
        hResult = DSERR_BUFFERLOST_;
        if (pBuffer->pChannel != 0) {
            pBuffer->bStopping = 0;
            int nData = LockMemFile2(pBuffer->pData->hMemFile);
            pBuffer->tHeader.pSamples = (void*)nData;
            if (pBuffer->wBitsPerSample == 8 && pBuffer->nChannels == 1) {
                pBuffer->tHeader.nChannelsOrLen = pBuffer->pData->nSize;
                pBuffer->tHeader.nSampleRate = FixRatio((short)pBuffer->nSamplesPerSec, 1);
                pBuffer->tHeader.nLoopStart = 0;
                pBuffer->tHeader.nLoopEnd = 0;
                pBuffer->tHeader.nEncode = 0;
                pBuffer->tHeader.nBaseFreq = 0x3c;
            } else {
                pBuffer->tHeader.nChannelsOrLen = pBuffer->nChannels;
                pBuffer->tHeader.nSampleRate = FixRatio((short)pBuffer->nSamplesPerSec, 1);
                pBuffer->tHeader.nLoopStart = 0;
                pBuffer->tHeader.nLoopEnd = 0;
                pBuffer->tHeader.nEncode = 0xff;
                pBuffer->tHeader.nBaseFreq = 0x3c;
                pBuffer->tHeader.nFrames = (pBuffer->pData->nSize / pBuffer->nChannels) / (pBuffer->wBitsPerSample >> 3);
                long double fRate = (double)pBuffer->nSamplesPerSec;
                memcpy(pBuffer->tHeader.aSampleRate, &fRate, sizeof(pBuffer->tHeader.aSampleRate));
                pBuffer->tHeader.nMarkerChunk = 0;
                pBuffer->tHeader.nInstruments = 0;
                pBuffer->tHeader.nSampleSize = pBuffer->wBitsPerSample;
            }
            pBuffer->nA5 = (int)SetCurrentA5();
            int nSize = pBuffer->pData->nSize;
            if ((dwFlags & DSBPLAY_LOOPING_) == 0) {
                pBuffer->nChunkBytes = nSize;
                pBuffer->bLooping = 0;
            } else {
                int nPos = pBuffer->nPlayCursor;
                nSize -= nPos;
                if (nSize < 1) {
                    pBuffer->nPlayCursor = 0;
                    nSize = pBuffer->pData->nSize;
                    nPos = 0;
                }
                if (pBuffer->wBitsPerSample == 8 && pBuffer->nChannels == 1) {
                    pBuffer->tHeader.pSamples = (void*)(nData + nPos);
                    int nChunk = 0x2000;
                    if (nSize < 0x2001) {
                        nChunk = nSize;
                    }
                    pBuffer->tHeader.nChannelsOrLen = nChunk;
                    pBuffer->nChunkBytes = nChunk;
                } else {
                    pBuffer->tHeader.pSamples = (void*)(nData + nPos);
                    int nFrameBytes = (pBuffer->tHeader.nSampleSize >> 3) * pBuffer->tHeader.nChannelsOrLen;
                    int nFrames = nSize / nFrameBytes;
                    int nChunk = 0x2000;
                    if (nFrames < 0x2001) {
                        nChunk = nFrames;
                    }
                    pBuffer->tHeader.nFrames = nChunk;
                    pBuffer->nChunkBytes = nChunk * nFrameBytes;
                }
                pBuffer->bLooping = 1;
            }
            IDirectSoundBuffer_UpdatePanAndVolume(pBuffer);
            SndCommand tCmd;
            if (s_bMinVolume != 0) {
                tCmd.cmd = 0x2e;
                tCmd.param1 = 0;
                tCmd.param2 = 0x10001;
                if (SndDoCommand(pBuffer->pChannel, &tCmd, 1) == 0) {
                    pBuffer->nAppliedLeft = 1;
                    pBuffer->nAppliedRight = 1;
                }
            }
            pBuffer->bStarted = 0;
            tCmd.cmd = 0xd;
            tCmd.param1 = 0x4d48;
            tCmd.param2 = (long)pBuffer;
            SndDoCommand(pBuffer->pChannel, &tCmd, 1);
            tCmd.cmd = 0x51;
            tCmd.param1 = 0;
            tCmd.param2 = (long)&pBuffer->tHeader;
            SndDoCommand(pBuffer->pChannel, &tCmd, 1);
            tCmd.cmd = 0xd;
            tCmd.param1 = 0x5348;
            tCmd.param2 = (long)pBuffer;
            SndDoCommand(pBuffer->pChannel, &tCmd, 1);
            hResult = 0;
        }
    }
    return hResult;
}

// 1.14d mac: 00033632
// A free channel of the 12 pool, else of the 4 pool.
SndChannelPtr IDirectSound_AcquireSoundChannel(IDirectSound* pDirectSound, IDirectSoundBuffer* pBuffer) {
    int nSlot = IDirectSound_GetFreeChannelFromPool(pDirectSound, pDirectSound->aPool, 12);
    SndChannelPtr pChannel;
    if (nSlot == -1) {
        nSlot = IDirectSound_GetFreeChannelFromPool(pDirectSound, pDirectSound->aPoolSmall, 4);
        pChannel = 0;
        if (nSlot != -1 && pDirectSound->aPoolSmall[nSlot].bFlag == 0) {
            pDirectSound->aPoolSmall[nSlot].pOwner = pBuffer;
            pChannel = pDirectSound->aPoolSmall[nSlot].pChannel;
        }
    } else {
        pDirectSound->aPool[nSlot].pOwner = pBuffer;
        pChannel = pDirectSound->aPool[nSlot].pChannel;
    }
    return pChannel;
}

// 1.14d mac: 000336ae
// Turns volume and pan (hundredths of a decibel) into left and right levels (256 at full)
// scaled by the buffer's gain, and sends them to the channel.
static short IDirectSoundBuffer_UpdatePanAndVolume(IDirectSoundBuffer* pBuffer) {
    SndChannelPtr pChannel = pBuffer->pChannel;
    short nErr = 0;
    if (pChannel != 0) {
        int nVolume = pBuffer->nVolume;
        int nPan = pBuffer->nPan;
        int nPanRight = 0;
        if (nPan >= 0) {
            nPanRight = nPan;
        }
        double fLeft = pow(10.0, (double)(nVolume - nPanRight) / 2000.0);
        pBuffer->nLevelLeft = (int)(fLeft * 256.0);
        float fGain = pBuffer->fGain;
        uint32_t nLeft = (uint32_t)((float)(int)(fLeft * 256.0) * fGain);
        pBuffer->nLevelLeft = nLeft;
        double fRight = pow(10.0, (double)((nPan & (nPan >> 31)) + nVolume) / 2000.0);
        int nRight = (int)((float)(int)(fRight * 256.0) * fGain);
        pBuffer->nLevelRight = nRight;
        pBuffer->nAppliedLeft = nLeft;
        pBuffer->nAppliedRight = nRight;
        SndCommand tCmd;
        tCmd.param2 = nRight << 16 | nLeft;
        tCmd.cmd = 0x2e;
        tCmd.param1 = 0;
        nErr = SndDoImmediate(pChannel, &tCmd);
    }
    return nErr;
}

// 1.14d mac: 000337da
int32_t IDirectSoundBuffer_Restore(void* pBuffer) {
    return E_FAIL_;
}

// 1.14d mac: 000337e4
// The first pool entry with a channel and no owner; else a new channel made in the first
// empty entry. -1 when there is neither.
static int IDirectSound_GetFreeChannelFromPool(IDirectSound* pDirectSound, SoundChannelSlot* pPool, int nCount) {
    if (nCount > 0) {
        for (int i = 0; i < nCount; i++) {
            if (pPool[i].pChannel != 0 && pPool[i].pOwner == 0) {
                return i;
            }
        }
        for (int i = 0; i < nCount; i++) {
            if (pPool[i].pChannel == 0 &&
                SndNewChannel(&pPool[i].pChannel, 5, 0x80, pDirectSound->pfnCallback) == 0) {
                return i;
            }
        }
    }
    return -1;
}

// 1.14d mac: 00033862
// Clears the owner of the pool entry that holds the channel; the small pool's owner also
// goes back to full gain.
static void IDirectSound_ReleaseSoundChannel(IDirectSound* pDirectSound, SndChannelPtr pChannel) {
    for (int i = 0; i < 12; i++) {
        if (pDirectSound->aPool[i].pChannel == pChannel) {
            pDirectSound->aPool[i].pOwner = 0;
            return;
        }
    }
    for (int i = 0; i < 4; i++) {
        if (pDirectSound->aPoolSmall[i].pChannel == pChannel) {
            pDirectSound->aPoolSmall[i].pOwner->fGain = 1.0f;
            pDirectSound->aPoolSmall[i].pOwner = 0;
            return;
        }
    }
}

// 1.14d mac: 000338bc
int32_t IDirectSoundBuffer_SetCurrentPosition(IDirectSoundBuffer* pBuffer, DWORD dwNewPosition) {
    pBuffer->nPlayCursor = dwNewPosition;
    return 0;
}

// 1.14d mac: 000338cc
// The primary buffer takes the format; any other buffer refuses it.
int32_t IDirectSoundBuffer_SetFormat(IDirectSoundBuffer* pBuffer, const void* pFormat) {
    int32_t hResult = E_INVALIDARG_;
    if (pFormat != 0) {
        hResult = DSERR_INVALIDCALL_;
        if (s_pPrimaryBuffer == pBuffer) {
            memcpy(&pBuffer->wFormatTag, pFormat, kWaveFormatBytes);
            hResult = 0;
        }
    }
    return hResult;
}

// 1.14d mac: 00033912
int32_t IDirectSoundBuffer_SetPan(IDirectSoundBuffer* pBuffer, int nPan) {
    int32_t hResult = E_INVALIDARG_;
    if ((uint32_t)(nPan + 10000) < 20001u) {
        hResult = E_FAIL_;
        if (pBuffer->nPan != nPan) {
            pBuffer->nPan = nPan;
            short nErr = IDirectSoundBuffer_UpdatePanAndVolume(pBuffer);
            hResult = E_FAIL_;
            if (nErr == 0) {
                hResult = 0;
            }
        }
    }
    return hResult;
}

// 1.14d mac: 0003395c
int32_t IDirectSoundBuffer_SetVolume(IDirectSoundBuffer* pBuffer, int nVolume) {
    int32_t hResult = E_INVALIDARG_;
    if ((uint32_t)(nVolume + 10000) < 10001u) {
        hResult = E_FAIL_;
        if (pBuffer->nVolume != nVolume) {
            pBuffer->nVolume = nVolume;
            hResult = 0;
            if (s_bMinVolume == 0) {
                short nErr = IDirectSoundBuffer_UpdatePanAndVolume(pBuffer);
                hResult = E_FAIL_;
                if (nErr == 0) {
                    hResult = 0;
                }
            }
        }
    }
    return hResult;
}

// 1.14d mac: 000339ba
int32_t IDirectSoundBuffer_Unlock(IDirectSoundBuffer* pBuffer) {
    int32_t hResult = E_FAIL_;
    if (s_pPrimaryBuffer != pBuffer) {
        UnLockMemFile2(pBuffer->pData->hMemFile);
        hResult = 0;
    }
    return hResult;
}

// 1.14d mac: 000339ec
int32_t IKsPropertySet_Release(void* pObject) {
    if (pObject != 0) {
        operator delete(pObject);
    }
    return 0;
}

// 1.14d mac: 00033a08
int32_t IKsPropertySet_QuerySupport(void* pPropSet, void* pGuid, uint32_t dwPropId, uint32_t* pTypeSupport) {
    *pTypeSupport = 0;
    return E_NOTIMPL_;
}

// 1.14d mac: 00033a1c
int32_t IKsPropertySet_Set(void* pPropSet, void* pGuid, uint32_t dwPropId, void* pInstanceData,
                           uint32_t nInstanceLength, void* pPropertyData, uint32_t nDataLength) {
    return 0;
}

#endif  // D2_OS_MAC
