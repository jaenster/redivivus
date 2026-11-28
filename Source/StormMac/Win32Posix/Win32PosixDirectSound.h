// Win32Posix: DirectSound. The shared code (D2Sound) carries the COM interfaces it calls
// through, IDirectSound and IDirectSoundBuffer with their vtables (Source/D2Sound/D2Sound.h);
// this layer declares DirectSoundCreate (d2_platform.h does on Windows) and will implement
// them, a vtable per interface, so the shared code's `p->lpVtbl->Play(p, ...)` calls stay as
// Windows makes them.
// Read through Win32Posix.h.
#pragma once

#if !defined(_WIN32) && D2_BACKEND_POSIX

#define DS_OK 0
#define DSERR_NODRIVER ((HRESULT)0x88780078)

extern "C" int32_t DirectSoundCreate(const GUID* pcGuidDevice, struct IDirectSound** ppDS, struct IUnknown* pUnkOuter);
extern "C" int32_t DirectSoundEnumerateA(BOOL (*pCallback)(int, char*, char*, DWORD), void* pContext);

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
