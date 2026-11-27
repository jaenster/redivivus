// The unit linked after SRegMac.mm (0003e21d..0003e30a): an FSSpec-to-path helper, a counter
// bump, and the static object that brings the Mac Storm layer up and tears it down.
// No binary records this file's name. SRegMac.mm ends with its own static initialiser (0003e0a0),
// which registers only the registry tables, so what follows is another unit.
#include "Source/StormMac/H/stpl.h"

#if !defined(_WIN32) && D2_OS_MAC

#include "Source/StormMac/H/StormApi.h"

#include <CoreServices/CoreServices.h>

// 1.14d mac: 000354bd
void StormMacInit();
// 1.14d mac: 000354f4
void MyRuntimeTerminate();

// 1.14d mac: 0003e21d
// An FSSpec as a POSIX path; paramErr for a missing argument.
OSStatus MACFILE_FSSpecToPath(FSSpec* pSpec, char* szPath, int nMaxPathSize) {
    if (pSpec == NULL || szPath == NULL || nMaxPathSize == 0) {
        return paramErr;
    }
    FSRef tRef;
    OSErr nErr = FSpMakeFSRef(pSpec, &tRef);
    if (nErr != noErr) {
        return nErr;
    }
    return FSRefMakePath(&tRef, (UInt8*)szPath, nMaxPathSize);
}

// Starts Storm's Mac layer before main and ends it at exit.
class StormMacRuntime {
public:
    StormMacRuntime() { StormMacInit(); }
    ~StormMacRuntime();
};

// Out of line: the static initialiser takes its address directly, not through a
// non-lazy pointer as it would for an inline (weak) one.
StormMacRuntime::~StormMacRuntime() {
    MyRuntimeTerminate();
}

// 1.14d mac: 003f54c0
static StormMacRuntime s_tStormMacRuntime;
// 1.14d mac: 003f54c4
int g_nStormMacCounter;

// 1.14d mac: 0003e2ab
int FUN_0003e2ab() {
    return g_nStormMacCounter++;
}

// The compiler-made functions of this object:
// 1.14d mac: 0003e294 StormMacRuntime::~StormMacRuntime[D1]
// 1.14d mac: 0003e2d0 _GLOBAL__I_a

#endif  // D2_OS_MAC
