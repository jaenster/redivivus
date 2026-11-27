// MacSupport.mm - two Mac shell helpers the game calls to find and start its own helper
// programs: spawning a process by path and the bundle's shared-support folder.
//
// 1.14d Intel Mac (DiabloII_macho) 0030a84a..0030a969. No __FILE__ names the unit (the
// bodies are Objective-C++: one sends NSBundle messages, the other copies the path into a
// std::string for argv).
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC`.
#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#import <Foundation/Foundation.h>
#include <crt_externs.h>
#include <spawn.h>
#include <stdint.h>
#include <string.h>
#include <string>

// 1.14d mac: 0030a84a
// posix_spawn of the path with argv {path} and the current environment.
bool MAC_SpawnProcess(const char* szPath) {
    std::string szArgv0(szPath);
    char* apszArgv[2] = {(char*)szArgv0.c_str(), 0};
    int nError = posix_spawn(0, szPath, 0, 0, apszArgv, *_NSGetEnviron());
    return nError == 0;
}

// 1.14d mac: 0030a904
// The bundle's shared-support folder as a file-system path.
bool MAC_GetSharedSupportPath(char* szBuffer, unsigned int nMaxLength) {
    return [[[NSBundle mainBundle] sharedSupportPath] getFileSystemRepresentation:szBuffer maxLength:nMaxLength];
}

#endif
