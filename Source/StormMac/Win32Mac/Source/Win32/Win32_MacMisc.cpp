// Storm's Win32-on-Mac layer: Carbon helpers and the layer's start-up, 1.14d Intel Mac
// 0001e94c..0001ed6c: newline conversion for alerts, the mouse, debug output, the
// microsecond/millisecond clock, c2pstr/p2cstr, WIN32MAC_Initialize and WinDestroy.
// The original file is unknown: no 1.14d string names it (see Source/StormMac/README.md).
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// 1.14d mac: 0001ec2e
void MAC_GetMicrosecondsThunk(DWORD* pHigh, DWORD* pLow) __attribute__((noinline));
// 1.14d mac: 0001ec37
void MAC_GetMillisecondsThunk(DWORD* pHigh, DWORD* pLow) __attribute__((noinline));
// 1.14d mac: 0001eb3a
void MAC_GetMicroseconds(DWORD* pdwHigh, DWORD* pdwLow);

void PATHCACHE_Initialize(void);
void PATHCACHE_Destroy(void);
void GDI_InitStockObjects(void);
void Wnd_InitWnd(int nFlags);
void ShutdownWin32GDI(void);
void Rgn_DestroyAllRegions(void);
void Mem_DestroyMasterPointer(void);
HANDLE AllocModuleHandle(void);

// 1.14d mac: 003f0e74
// 0 before the first clock read, 1 once Microseconds() is the source.
static char s_nClockState;
// 1.14d mac: 003f0e78
static HANDLE s_hModule;

// 1.14d mac: 0001e94c
// Replaces *pString with it split on "\n" and rejoined with "\r".
void MAC_ConvertNewlinesToReturns(CFStringRef* pString) {
    CFArrayRef pParts = CFStringCreateArrayBySeparatingStrings(NULL, *pString, CFSTR("\n"));
    if (pParts != NULL) {
        CFStringRef pOldString = *pString;
        *pString = CFStringCreateByCombiningStrings(NULL, pParts, CFSTR("\r"));
        CFRelease(pParts);
        CFRelease(pOldString);
    }
}

// 1.14d mac: 0001e9bb
void MAC_GetGlobalMouse(Point* pPoint) {
    GetGlobalMouse(pPoint);
}

// 1.14d mac: 0001e9c4
void MAC_WarpMouseCursorPosition(Point ptPos) {
    CGPoint tPoint;
    tPoint.x = (float)ptPos.h;
    tPoint.y = (float)ptPos.v;
    CGWarpMouseCursorPosition(tPoint);
}

// 1.14d mac: 0001e9fb
void MAC_OutputDebugStringF(const char* szFormat, ...) {
    char szBuffer[1024];
    va_list tArgs;
    va_start(tArgs, szFormat);
    vsprintf(szBuffer, szFormat, tArgs);
    va_end(tArgs);
    CFStringRef cfString = CFStringCreateWithCString(NULL, szBuffer, kCFStringEncodingUTF8);
    if (cfString == NULL) {
        cfString = CFStringCreateWithCString(NULL, szBuffer, 0);
        if (cfString == NULL) {
            CFShow(CFSTR("*** OutputDebugString: Couldn't encode string!"));
            return;
        }
    }
    CFShow(cfString);
    CFRelease(cfString);
}

// 1.14d mac: 0001eabd
void MAC_GetMicroseconds64(DWORD* pCounterOut) {
    MAC_GetMicrosecondsThunk(pCounterOut + 1, pCounterOut);
}

// 1.14d mac: 0001eade
DWORD FPMilliseconds(void) {
    DWORD dwMillisecondsHigh;
    DWORD dwMilliseconds;
    MAC_GetMillisecondsThunk(&dwMillisecondsHigh, &dwMilliseconds);
    return dwMilliseconds;
}

// 1.14d mac: 0001eafe
unsigned char* c2pstr(char* szString) {
    CopyCStringToPascal(szString, (unsigned char*)szString);
    return (unsigned char*)szString;
}

// 1.14d mac: 0001eb1c
char* p2cstr(unsigned char* pPascalString) {
    CopyPascalStringToC(pPascalString, (char*)pPascalString);
    return (char*)pPascalString;
}

// 1.14d mac: 0001eb3a
// The clock as two dwords, high first in the out list.
void MAC_GetMicroseconds(DWORD* pdwHigh, DWORD* pdwLow) {
    UnsignedWide tTime;
    if (s_nClockState == 0) {
        if (true) {
            s_nClockState = 1;
        }
        MAC_GetMicroseconds(pdwHigh, pdwLow);
    } else if (s_nClockState == 1) {
        Microseconds(&tTime);
        *pdwHigh = tTime.hi;
        *pdwLow = tTime.lo;
    } else {
        *pdwHigh = 0;
        *pdwLow = 0;
    }
}

// 1.14d mac: 0001eba1
void MAC_GetMilliseconds(DWORD* pdwHigh, DWORD* pdwLow) {
    UnsignedWide tTime;
    if (s_nClockState == 0) {
        if (true) {
            s_nClockState = 1;
        }
        MAC_GetMicroseconds(pdwHigh, pdwLow);
        tTime.lo = *pdwLow;
    } else if (s_nClockState == 1) {
        Microseconds(&tTime);
        *pdwHigh = tTime.hi;
        *pdwLow = tTime.lo;
    } else {
        *pdwHigh = 0;
        *pdwLow = 0;
        tTime.lo = 0;
    }
    uint64_t ullMilliseconds = (((uint64_t)*pdwHigh << 32) | tTime.lo) / 1000;
    *pdwLow = (DWORD)ullMilliseconds;
    *pdwHigh = (DWORD)(ullMilliseconds >> 32);
}

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
