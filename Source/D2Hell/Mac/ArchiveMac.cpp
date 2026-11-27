// ArchiveMac.cpp - the Mac-only walk over the mounted volumes that D2Hell's archive code runs
// before it opens the game archives.
//
// 1.14d Intel Mac (DiabloII_macho) 002b692a. The binary's file-map names it with
// D2Hell/SRC/Archive.cpp (its neighbours ARCHIVE_LoadMPQ and D2FILE_AllocAndLoadD2File carry that
// __FILE__); the Windows build compiles Archive.cpp, so the Mac body sits in a file of its own.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC`.
#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>

// 1.14d mac: 00035351
bool STORM_IsCDVolume(short nVRefNum);
// 1.14d mac: 000346a3
short UnmountVolume(ConstStr255Param szVolumeName, short nVRefNum);

// 1.14d mac: 002b692a
// Walks the mounted volumes by index and unmounts each one the CD check accepts, stopping
// when an unmount fails or there are no more volumes. Always returns 1.
int FUN_002b692a(void) {
    HParamBlockRec tParam;
    Str255 szVolumeName;
    tParam.volumeParam.ioVolIndex = 1;
    tParam.volumeParam.ioVRefNum = 0;
    tParam.volumeParam.ioNamePtr = szVolumeName;
    short nVolIndex = PBHGetVInfoSync(&tParam);
    if (nVolIndex == 0) {
        nVolIndex = 2;
        do {
            if (STORM_IsCDVolume(tParam.volumeParam.ioVRefNum)) {
                if (UnmountVolume(0, tParam.volumeParam.ioVRefNum) != 0) {
                    break;
                }
            }
            tParam.volumeParam.ioVRefNum = 0;
            tParam.volumeParam.ioNamePtr = szVolumeName;
            tParam.volumeParam.ioVolIndex = nVolIndex;
            short nResult = PBHGetVInfoSync(&tParam);
            nVolIndex = nVolIndex + 1;
            if (nResult != 0) {
                break;
            }
        } while (true);
    }
    return 1;
}

#endif
