// unknown_util.cpp - two Mac shell helpers between Game/Main.cpp and D2FlamingLogo.cpp: the
// short sleep the OpenGL window code waits with, and the working directory set to the
// application's folder.
//
// 1.14d Intel Mac (DiabloII_macho) 0019f18a..0019f2b6. No binary records this unit's name, and
// nothing in the binary decides whether these two close Game/Main.cpp, open D2FlamingLogo.cpp
// or are a unit of their own; they are kept apart from both. See Source/MacSpecific/README.md.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC` (see
// Source/MacSpecific/README.md).
#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <pthread.h>
#include <stdlib.h>

#include <QuickTime/QuickTime.h>
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <string.h>

#include "Source/MacSpecific/Source/MacShell/QuickDrawMac.h"

#include "Source/StormMac/H/stpl.h"

typedef int BOOL;
// 1.14d mac: 0002c0ee
uint32_t GetTickCount(void);
// 1.14d mac: 0002c035
void Sleep(uint32_t dwMilliseconds);
// 1.14d mac: 0001e9fb
void MAC_OutputDebugStringF(const char* szFormat, ...);
void MAC_SetWorkingDirectoryToAppFolder(void);
// 1.14d mac: 00033f9a
int GetApplicationInformation(void* pName, FSSpec* pFileSpec, uint32_t* pdwSignature, uint64_t* pPSN);

// 1.14d mac: 0019f18a
// Sleeps in 10 ms steps, at least ten of them and at least a quarter of a second.
void MAC_SettleWait(void) {
    uint32_t dwStartTick = GetTickCount();
    uint32_t nSleepCount = 0;
    do {
        if (9 < nSleepCount) {
            uint32_t dwTick = GetTickCount();
            if (-1 < (int)(dwTick + (-0xFA - dwStartTick))) {
                return;
            }
        }
        nSleepCount++;
        Sleep(10);
    } while (true);
}

// 1.14d mac: 0019f1c4
// Makes the folder the application lives in the working directory.
void MAC_SetWorkingDirectoryToAppFolder(void) {
    FSSpec tAppSpec;
    FSSpec tSpec;
    FSRef tRef;
    char szPath[1024];
    GetApplicationInformation(NULL, &tAppSpec, NULL, NULL);
    FSMakeFSSpec(tAppSpec.vRefNum, tAppSpec.parID, "\p", &tSpec);
    FSpMakeFSRef(&tSpec, &tRef);
    CFURLRef pUrl = CFURLCreateFromFSRef(NULL, &tRef);
    CFURLGetFileSystemRepresentation(pUrl, true, (UInt8*)szPath, 0x400);
    if (chdir(szPath) == -1) {
        MAC_OutputDebugStringF("Couldn't set the working directory!");
    }
}

#endif  // D2_OS_MAC
