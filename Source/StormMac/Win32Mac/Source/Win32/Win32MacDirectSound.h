// Storm's Win32-on-Mac layer: the DirectDraw and DirectSound shim (Win32_DirectSound.cpp).
//
// The Mac build has no COM: IDirectSound, IDirectSoundBuffer and IKsPropertySet are the
// layer's own records, with no vtable, and each method is a function the caller calls
// directly with the object as its first argument. D2Sound's SOUNDSMP_Play calls
// IDirectSoundBuffer_Play at 002e92c2 (call 00033344), SOUNDSMP_SetVolume calls
// IDirectSoundBuffer_SetVolume at 002e9156, SOUNDSMP_ClearBuffer IDirectSoundBuffer_Unlock at
// 002e8bfb; no caller loads a method from the object. The callers pass the full Win32 argument
// lists (Unlock five, CreateSoundBuffer four), which the methods declare here whether or not
// they read them.
//
// The shared code spells each call as the Windows build makes it, `p->lpVtbl->Play(p, ...)`.
// On the Mac the method tables below stand in for the shared headers' vtable types
// (D2_DEFINED_*Vtbl): each entry is a static member forwarding to the layer's function, so
// the call compiles to the direct call the 1.14d Mac build makes (the unused lpVtbl load is
// dead code at -Os). The shared code's
// IDirectSound3DBuffer has no Mac counterpart: the layer's QueryInterface answers
// E_NOINTERFACE, and the Mac D2Sound has no 3D buffer.
#pragma once

#include "Win32Mac.h"

#if !defined(_WIN32) && D2_MAC_CARBON

struct IDirectSound;
struct IDirectSoundBuffer;

// 1.14d mac: 00032b50
int DirectDrawCreate(void* lpGuid, int** ppObject, void* pUnkOuter);
// 1.14d mac: 00032b83
int32_t DirectSoundCreate(void* lpGuid, IDirectSound** ppDS, void* pUnkOuter);

// ---- IDirectSound ----
int32_t IDirectSound_Release(IDirectSound* pDirectSound);
int32_t IDirectSound_CreateSoundBuffer(IDirectSound* pDirectSound, const uint32_t* pBufferDesc,
                                       IDirectSoundBuffer** ppBuffer, void* pUnkOuter);
int32_t IDirectSound_GetCaps(IDirectSound* pDirectSound, uint32_t* pCaps);
int32_t IDirectSound_SetCooperativeLevel(void* pDirectSound, void* pWindow, uint32_t dwLevel);

// ---- IDirectSoundBuffer ----
int32_t IDirectSoundBuffer_QueryInterface(void* pBuffer, void* pIid, void** ppInterface);
int32_t IDirectSoundBuffer_Release(IDirectSoundBuffer* pBuffer);
int32_t IDirectSoundBuffer_GetCaps(IDirectSoundBuffer* pBuffer, uint32_t* pCaps);
int32_t IDirectSoundBuffer_GetCurrentPosition(IDirectSoundBuffer* pBuffer, DWORD* pdwPlayCursor,
                                              DWORD* pdwWriteCursor);
int32_t IDirectSoundBuffer_GetStatus(IDirectSoundBuffer* pBuffer, DWORD* pdwStatus);
int32_t IDirectSoundBuffer_Lock(IDirectSoundBuffer* pBuffer, int dwOffset, int dwBytes, int* ppvAudioPtr1,
                                int* pdwAudioBytes1, int* ppvAudioPtr2, int* pdwAudioBytes2, uint32_t dwFlags);
int32_t IDirectSoundBuffer_Play(IDirectSoundBuffer* pBuffer, DWORD dwReserved1, DWORD dwReserved2, uint32_t dwFlags);
int32_t IDirectSoundBuffer_SetCurrentPosition(IDirectSoundBuffer* pBuffer, DWORD dwNewPosition);
int32_t IDirectSoundBuffer_SetFormat(IDirectSoundBuffer* pBuffer, const void* pFormat);
int32_t IDirectSoundBuffer_SetVolume(IDirectSoundBuffer* pBuffer, int nVolume);
int32_t IDirectSoundBuffer_SetPan(IDirectSoundBuffer* pBuffer, int nPan);
int32_t IDirectSoundBuffer_Stop(IDirectSoundBuffer* pBuffer);
int32_t IDirectSoundBuffer_Unlock(IDirectSoundBuffer* pBuffer, void* pvAudioPtr1, DWORD dwAudioBytes1,
                                  void* pvAudioPtr2, DWORD dwAudioBytes2);
int32_t IDirectSoundBuffer_Restore(void* pBuffer);

// ---- IKsPropertySet ----
int32_t IKsPropertySet_Release(void* pObject);
int32_t IKsPropertySet_Set(void* pPropSet, void* pGuid, uint32_t dwPropId, void* pInstanceData,
                           uint32_t nInstanceLength, void* pPropertyData, uint32_t nDataLength);
int32_t IKsPropertySet_QuerySupport(void* pPropSet, void* pGuid, uint32_t dwPropId, uint32_t* pTypeSupport);

// ---- the method tables the shared code calls through ----
#define D2_DEFINED_IDirectSoundVtbl
struct IDirectSoundVtbl {
    static int32_t Release(void* pThis) { return IDirectSound_Release((IDirectSound*)pThis); }
    static int32_t CreateSoundBuffer(void* pThis, void* pDesc, void** ppBuffer, void* pUnkOuter) {
        return IDirectSound_CreateSoundBuffer((IDirectSound*)pThis, (const uint32_t*)pDesc, (IDirectSoundBuffer**)ppBuffer,
                                              pUnkOuter);
    }
    static int32_t GetCaps(void* pThis, void* pCaps) { return IDirectSound_GetCaps((IDirectSound*)pThis, (uint32_t*)pCaps); }
    static int32_t SetCooperativeLevel(void* pThis, void* pWindow, DWORD dwLevel) {
        return IDirectSound_SetCooperativeLevel(pThis, pWindow, dwLevel);
    }
};

#define D2_DEFINED_IDirectSoundBufferVtbl
struct IDirectSoundBufferVtbl {
    static int32_t QueryInterface(void* pThis, void* pIid, void** ppInterface) {
        return IDirectSoundBuffer_QueryInterface(pThis, pIid, ppInterface);
    }
    static int32_t Release(void* pThis) { return IDirectSoundBuffer_Release((IDirectSoundBuffer*)pThis); }
    static int32_t GetCaps(void* pThis, void* pCaps) {
        return IDirectSoundBuffer_GetCaps((IDirectSoundBuffer*)pThis, (uint32_t*)pCaps);
    }
    static int32_t GetCurrentPosition(void* pThis, void* pdwPlayCursor, void* pdwWriteCursor) {
        return IDirectSoundBuffer_GetCurrentPosition((IDirectSoundBuffer*)pThis, (DWORD*)pdwPlayCursor,
                                                     (DWORD*)pdwWriteCursor);
    }
    static int32_t GetStatus(void* pThis, void* pdwStatus) {
        return IDirectSoundBuffer_GetStatus((IDirectSoundBuffer*)pThis, (DWORD*)pdwStatus);
    }
    static int32_t Lock(void* pThis, DWORD dwOffset, DWORD dwBytes, void* ppvAudioPtr1, void* pdwAudioBytes1,
                        void* ppvAudioPtr2, void* pdwAudioBytes2, DWORD dwFlags) {
        return IDirectSoundBuffer_Lock((IDirectSoundBuffer*)pThis, dwOffset, dwBytes, (int*)ppvAudioPtr1,
                                       (int*)pdwAudioBytes1, (int*)ppvAudioPtr2, (int*)pdwAudioBytes2, dwFlags);
    }
    static int32_t Play(void* pThis, DWORD dwReserved1, DWORD dwReserved2, DWORD dwFlags) {
        return IDirectSoundBuffer_Play((IDirectSoundBuffer*)pThis, dwReserved1, dwReserved2, dwFlags);
    }
    static int32_t SetCurrentPosition(void* pThis, DWORD dwNewPosition) {
        return IDirectSoundBuffer_SetCurrentPosition((IDirectSoundBuffer*)pThis, dwNewPosition);
    }
    static int32_t SetFormat(void* pThis, void* pFormat) {
        return IDirectSoundBuffer_SetFormat((IDirectSoundBuffer*)pThis, pFormat);
    }
    static int32_t SetVolume(void* pThis, LONG nVolume) {
        return IDirectSoundBuffer_SetVolume((IDirectSoundBuffer*)pThis, nVolume);
    }
    static int32_t SetPan(void* pThis, LONG nPan) { return IDirectSoundBuffer_SetPan((IDirectSoundBuffer*)pThis, nPan); }
    static int32_t Stop(void* pThis) { return IDirectSoundBuffer_Stop((IDirectSoundBuffer*)pThis); }
    static int32_t Unlock(void* pThis, void* pvAudioPtr1, DWORD dwAudioBytes1, void* pvAudioPtr2, DWORD dwAudioBytes2) {
        return IDirectSoundBuffer_Unlock((IDirectSoundBuffer*)pThis, pvAudioPtr1, dwAudioBytes1, pvAudioPtr2,
                                         dwAudioBytes2);
    }
    static int32_t Restore(void* pThis) { return IDirectSoundBuffer_Restore(pThis); }
};

#define D2_DEFINED_IKsPropertySetVtbl
struct IKsPropertySetVtbl {
    static int32_t Release(void* pThis) { return IKsPropertySet_Release(pThis); }
    static int32_t Set(void* pThis, void* pGuid, uint32_t dwPropId, void* pInstanceData, uint32_t nInstanceLength,
                       void* pPropertyData, uint32_t nDataLength) {
        return IKsPropertySet_Set(pThis, pGuid, dwPropId, pInstanceData, nInstanceLength, pPropertyData, nDataLength);
    }
    static int32_t QuerySupport(void* pThis, void* pGuid, uint32_t dwPropId, void* pTypeSupport) {
        return IKsPropertySet_QuerySupport(pThis, pGuid, dwPropId, (uint32_t*)pTypeSupport);
    }
};

#endif  // D2_OS_MAC
