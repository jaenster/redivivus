// Storm's Win32-on-Mac layer: the clock thunks and the layer's start-up and shutdown,
// 1.14d Intel Mac 0001ec2e..0001ed6c (WIN32MAC_Initialize, WinDestroy). A unit of its own
// after Win32_MacMisc.cpp: MAC_GetMicroseconds64 and FPMilliseconds call the two one-jump
// thunks out of line, which clang -Os would inline within one unit. The original file is
// unknown: no 1.14d string names it (see Source/StormMac/README.md).
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <string.h>

// 1.14d mac: 0001eb3a
void MAC_GetMicroseconds(DWORD* pdwHigh, DWORD* pdwLow);
// 1.14d mac: 0001eba1
void MAC_GetMilliseconds(DWORD* pdwHigh, DWORD* pdwLow);

void PATHCACHE_Initialize(void);
void PATHCACHE_Destroy(void);
void GDI_InitStockObjects(void);
void Wnd_InitWnd(int nFlags);
void ShutdownWin32GDI(void);
void Rgn_DestroyAllRegions(void);
void Mem_DestroyMasterPointer(void);
HANDLE AllocModuleHandle(void);

// 1.14d mac: 003f0e78
static HANDLE s_hModule;

// 1.14d mac: 0001ec2e
void MAC_GetMicrosecondsThunk(DWORD* pHigh, DWORD* pLow) {
    MAC_GetMicroseconds(pHigh, pLow);
}

// 1.14d mac: 0001ec37
void MAC_GetMillisecondsThunk(DWORD* pHigh, DWORD* pLow) {
    MAC_GetMilliseconds(pHigh, pLow);
}

// 1.14d mac: 0001ec40
// Makes the process' module handle (its path and serial number), then starts the path
// cache, the GDI stock objects and the window subsystem.
void WIN32MAC_Initialize(void) {
    Win32MacFSSpec tSpec;
    char szAppPath[256];
    HANDLE hModule = AllocModuleHandle();
    Win32HandleObject* pModule = GetWin32HandlePtr(hModule);
    pModule->pLibrary = NULL;
    uint64_t nSize;
    GetApplicationInformation(NULL, (FSSpec*)&tSpec, NULL, &nSize);
    pModule->dwPSNHigh = (DWORD)nSize;
    pModule->dwPSNLow = (DWORD)(nSize >> 32);
    FSSpecToFilepath(szAppPath, (const FSSpec*)&tSpec);
    size_t nPathLen = strlen(szAppPath);
    char* szPathCopy = (char*)MemAlloc(nPathLen + 1, 0);
    memcpy(szPathCopy, szAppPath, nPathLen + 1);
    pModule->szModulePath = szPathCopy;
    s_hModule = hModule;
    PATHCACHE_Initialize();
    GDI_InitStockObjects();
    Wnd_InitWnd(0);
}

// 1.14d mac: 0001ed37
void WinDestroy(void) {
    ShutdownWin32GDI();
    PATHCACHE_Destroy();
    CloseHandle(s_hModule);
    Rgn_DestroyAllRegions();
    Mem_DestroyMasterPointer();
}

#endif  // D2_OS_MAC
