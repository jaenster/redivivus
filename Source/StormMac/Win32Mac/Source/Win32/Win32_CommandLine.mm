// StormMac Win32 Mac/Source/Win32/Win32_CommandLine.mm: GetCommandLine, 1.14d Intel Mac 0002ab8e.
//
// The one Objective-C function of the Win32_Process run (00029b68..0002b1fe): the command line is
// read from NSProcessInfo, so it cannot share a translation unit with Win32_Process.cpp (a C++
// unit; Objective-C++ here means the .mm suffix, which cmake/mac.cmake builds with
// -x objective-c++). The text is built once, from the arguments minus the Finder's process
// serial number ones (-psn_..., -parent_psn_...), an argument with a space quoted.
// Win32Mac.h is not included: it defines BOOL as a 32-bit integer, which Objective-C refuses.
#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#import <Foundation/Foundation.h>
#include <string.h>

#define WIN32MAC_COMMANDLINE_SIZE 0x800

// 1.14d mac: 003f1a74
static char s_szCommandLine[WIN32MAC_COMMANDLINE_SIZE];
// 1.14d mac: 003f2274
static bool s_bCommandLineBuilt;
// 1.14d mac: 003f2278
static size_t s_nCommandLineLength;

// 1.14d mac: 0002ab8e
char* GetCommandLine(void) {
    if (s_szCommandLine[0] == '\0' && !s_bCommandLineBuilt) {
        s_bCommandLineBuilt = true;
        for (NSString* pArgument in [[NSProcessInfo processInfo] arguments]) {
            if ([pArgument hasPrefix:@"-psn_"] || [pArgument hasPrefix:@"-parent_psn_"]) {
                continue;
            }
            if (s_szCommandLine[0] != '\0') {
                strlcat(s_szCommandLine, " ", WIN32MAC_COMMANDLINE_SIZE);
            }
            if ([pArgument rangeOfString:@" "].location != NSNotFound) {
                pArgument = [NSString stringWithFormat:@"\"%@\"",
                             [pArgument stringByReplacingOccurrencesOfString:@"\"" withString:@"\\\""]];
            }
            strlcat(s_szCommandLine, [pArgument UTF8String], WIN32MAC_COMMANDLINE_SIZE);
            if (s_nCommandLineLength == 0) {
                s_nCommandLineLength = strlen(s_szCommandLine);
            }
        }
    }
    return s_szCommandLine;
}

#endif  // D2_OS_MAC
