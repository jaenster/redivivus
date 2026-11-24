// StormMac Win32 Mac/Source/Win32/Win32_Files.cpp: file handles on the Carbon File
// Manager, 1.14d Intel Mac 00021a94..00022ca8 (HANDLE_GetFileRefNum .. FindClose).
//
// Pinned by the PPC Storm 1.10 Beta: its __sinit_Win32_Files_cpp closes the run
// FileRefFromHANDLE, CreateFile, ReadFile, WriteFile, FlushFileBuffers, SetFilePointer,
// GetFileSize, DeleteFile, GetFileAttributes, GetFileTime, SetFileTime, SetFileTimeAtClose,
// CreateDirectory, FindFirstFile, FindNextFile, FindClose (MWCC emits a file's functions
// last to first), the same functions in the same order as 1.14d's run. The folder is that
// of Win32_utils.cpp, the one Win32_ file whose 1.14d path is in the binary.
//
// A file handle (type 2) holds the refnum of one open fork, data or resource, and
// the FSSpec it was opened through. Positions and sizes are 32-bit: a high
// distance is refused.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <CoreServices/CoreServices.h>
#include <string.h>
#include <time.h>

// 1.14d mac: 00021a94
// The File Manager refnum behind a file handle.
BOOL HANDLE_GetFileRefNum(HANDLE hFile, int16_t* pnRefNum) {
    *pnRefNum = 0;
    Win32HandleObject* pFile = GetWin32HandlePtr(hFile);
    if (pFile == NULL || pFile->dwType != WIN32MAC_HANDLE_FILE) {
        SetLastError(6);
        return 0;
    }
    *pnRefNum = pFile->nRefNum;
    return 1;
}

#define GENERIC_READ 0x80000000
#define GENERIC_WRITE 0x40000000
#define CREATE_NEW 1
#define OPEN_EXISTING 3
#define TRUNCATE_EXISTING 5
#define FILE_FLAG_OVERLAPPED 0x40000000
// Not a Win32 flag: the layer's own request for the resource fork.
#define FILE_FLAG_MAC_RESOURCE_FORK 0x00800000

// 1.14d mac: 000344dc
OSErr MacResolveAliasFile(FSSpec* pSpec, Boolean bResolveChains, Boolean* pbTargetIsFolder, Boolean* pbWasAliased,
                          Boolean bNoUI);

// 1.14d mac: 0002c143
BOOL FileTimeToMacDateTime(const FILETIME* pFileTime, UInt32* pnMacDate, void* pUnused);
// 1.14d mac: 0002c199
BOOL MacDateTimeToFileTime(const UInt32* pnMacDate, FILETIME* pFileTime);

// 1.14d mac: 00021ad7
HANDLE CreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPVOID lpSecurityAttributes,
                  DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    int16_t nRefNum = 0;
    DWORD dwErrCode;
    if (hTemplateFile != 0 || lpSecurityAttributes != NULL || (dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED)) {
        dwErrCode = 0x57;
        goto fail;
    }
    {
        DWORD dwResourceFork = dwFlagsAndAttributes & FILE_FLAG_MAC_RESOURCE_FORK;
        FSSpec tSpec;
        FilepathToFSSpec(&tSpec, lpFileName, 0);
        if (dwCreationDisposition != OPEN_EXISTING && dwCreationDisposition != TRUNCATE_EXISTING) {
            DWORD dwCreator = g_dwNewFileCreator;
            DWORD dwType = g_dwNewFileType;
            if (dwCreator == 0) {
                GetApplicationInformation(NULL, NULL, &dwCreator, NULL);
            }
            if (dwType == 0) {
                dwType = GetFileTypeAndCreatorForPath(lpFileName, &dwCreator);
            }
            OSErr nErr = FSpCreate(&tSpec, dwCreator, dwType, smSystemScript);
            if (dwCreationDisposition == CREATE_NEW && nErr == dupFNErr) {
                dwErrCode = 0xB7;
                goto fail;
            }
            if (dwResourceFork) {
                FSpCreateResFile(&tSpec, dwCreator, dwType, smSystemScript);
                ResError();
            }
        }
        SInt8 nPermission;
        if (dwDesiredAccess == (GENERIC_READ | GENERIC_WRITE)) {
            nPermission = fsRdWrPerm;
        } else if (dwDesiredAccess == GENERIC_READ) {
            nPermission = fsRdPerm;
        } else if (dwDesiredAccess == GENERIC_WRITE) {
            nPermission = fsWrPerm;
        } else {
            dwErrCode = 0x57;
            goto fail;
        }
        Boolean bTargetIsFolder = 0;
        Boolean bWasAliased = 0;
        FSSpec tResolved;
        memcpy(&tResolved, &tSpec, sizeof(FSSpec));
        if (dwShareMode == 3) {
            nPermission = fsRdWrShPerm;
        }
        OSErr nErr = MacResolveAliasFile(&tResolved, 1, &bTargetIsFolder, &bWasAliased, 0);
        if (bTargetIsFolder) {
            nErr = fnfErr;
        }
        if (nErr == noErr) {
            if (!dwResourceFork) {
                nErr = FSpOpenDF(&tResolved, nPermission, &nRefNum);
            } else {
                nRefNum = FSpOpenResFile(&tResolved, nPermission);
                if (nRefNum == -1) {
                    nErr = ResError();
                }
            }
            if (nErr == noErr) {
                if (dwCreationDisposition - 1 < 2) {
                    SetEOF(nRefNum, 0);
                }
                HANDLE hFile = AllocWin32Handle(WIN32MAC_HANDLE_FILE);
                Win32HandleObject* pFile = GetWin32HandlePtr(hFile);
                pFile->nRefNum = nRefNum;
                memcpy(&pFile->tSpec, &tResolved, sizeof(FSSpec));
                pFile->bWritable = (dwDesiredAccess >> 30) & 1;
                pFile->bResourceFork = dwResourceFork >> 23;
                return hFile;
            }
        }
        switch (nErr) {
            case afpAccessDenied:
            case permErr:
            case opWrErr:
            case vLckdErr:
            case wPrErr:
                dwErrCode = 5;
                break;
            case -5025:
            case dirNFErr:
            case bdNamErr:
                dwErrCode = 0xA1;
                break;
            case memFullErr:
            case dskFulErr:
                dwErrCode = 8;
                break;
            case dupFNErr:
                dwErrCode = 0xB7;
                break;
            case fnfErr:
                dwErrCode = 2;
                break;
            case eofErr:
                dwErrCode = 0x26;
                break;
            case ioErr:
                dwErrCode = 0x15;
                break;
            case nsvErr:
                dwErrCode = 0xF;
                break;
            default:
                dwErrCode = 0x6E;
                break;
        }
    }
fail:
    SetLastError(dwErrCode);
    return 0;
}

// 1.14d mac: 00021ea4
// Reads at the fork's mark. A failed read rewinds to where it started and tries
// again, at most ten times and for at most five seconds; giving up still returns 1.
BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead,
              LPVOID lpOverlapped) {
    int16_t nRefNum = GetWin32HandlePtr(hFile)->nRefNum;
    if (lpNumberOfBytesRead != NULL) {
        *lpNumberOfBytesRead = 0;
    }
    if (lpOverlapped != NULL) {
        SetLastError(0x15);
        return 0;
    }
    SInt64 nStart;
    if (FSGetForkPosition(nRefNum, &nStart) != noErr) {
        SetLastError(6);
        return 0;
    }
    DWORD dwStartTime = FPMilliseconds();
    int nRetries = 0;
    do {
        ParamBlockRec tPB = {};
        tPB.ioParam.ioRefNum = nRefNum;
        tPB.ioParam.ioBuffer = (Ptr)lpBuffer;
        tPB.ioParam.ioReqCount = nNumberOfBytesToRead;
        tPB.ioParam.ioPosMode = fsAtMark;
        tPB.ioParam.ioPosOffset = 0;
        PBReadSync(&tPB);
        OSErr nErr = tPB.ioParam.ioResult;
        if (nErr == noErr || nErr == eofErr) {
            *lpNumberOfBytesRead = tPB.ioParam.ioActCount;
            return 1;
        }
        if (FSSetForkPosition(nRefNum, fsFromStart, nStart) != noErr) {
            SetLastError(6);
            return 0;
        }
        if (++nRetries > 9) {
            return 1;
        }
    } while ((int)(FPMilliseconds() - dwStartTime) < 5000);
    return 1;
}

// 1.14d mac: 00021fc6
// Writes in 256 KB pieces.
BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten,
               LPVOID lpOverlapped) {
    Win32HandleObject* pFile = GetWin32HandlePtr(hFile);
    if (lpOverlapped == NULL) {
        int16_t nRefNum = pFile->nRefNum;
        DWORD nLast = nNumberOfBytesToWrite & 0x3FFFF;
        DWORD nPieces = nNumberOfBytesToWrite >> 18;
        const char* pData = (const char*)lpBuffer;
        DWORD nWritten = 0;
        long nCount;
        OSErr nErr;
        do {
            if (nPieces-- == 0) {
                nCount = nLast;
                nErr = FSWrite(nRefNum, &nCount, pData);
                *lpNumberOfBytesWritten = nWritten + nCount;
                if (nErr == noErr) {
                    return 1;
                }
                goto fail;
            }
            nCount = 0x40000;
            nErr = FSWrite(nRefNum, &nCount, pData);
            pData += nCount;
            nWritten += nCount;
        } while (nErr == noErr);
        *lpNumberOfBytesWritten = nWritten;
    }
fail:
    SetLastError(0x15);
    return 0;
}

// 1.14d mac: 00022082
BOOL FlushFileBuffers(HANDLE hFile) {
    Win32HandleObject* pFile = GetWin32HandlePtr(hFile);
    ParamBlockRec tPB;
    SMemZero(&tPB, sizeof(tPB));
    tPB.ioParam.ioRefNum = pFile->nRefNum;
    return PBFlushFileSync(&tPB) == noErr;
}

// 1.14d mac: 000220ca
// FILE_END reads the end of file into the current position before using the end
// it already has; the asm does it, it changes nothing.
DWORD SetFilePointer(HANDLE hFile, LONG lDistanceToMove, LPLONG lpDistanceToMoveHigh, DWORD dwMoveMethod) {
    int16_t nRefNum = GetWin32HandlePtr(hFile)->nRefNum;
    if (lpDistanceToMoveHigh != NULL && *lpDistanceToMoveHigh != 0) {
        SetLastError(0x57);
        return (DWORD)-1;
    }
    long nNewPos;
    long nPos;
    long nEOF;
    if (GetFPos(nRefNum, &nPos) != noErr || GetEOF(nRefNum, &nEOF) != noErr) {
        SetLastError(6);
        return (DWORD)-1;
    }
    short nPosMode = fsFromStart;
    long nTarget;
    if (dwMoveMethod == 0) {
        nPosMode = fsFromStart;
        nTarget = lDistanceToMove;
    } else if (dwMoveMethod == 2) {
        GetEOF(nRefNum, &nPos);
        nTarget = nEOF + lDistanceToMove;
        nPosMode = fsFromLEOF;
    } else if (dwMoveMethod == 1) {
        nTarget = nPos + lDistanceToMove;
        nPosMode = fsFromMark;
    } else {
        goto set_position;
    }
    if (nTarget < 0) {
        SetLastError(0xD);
        return (DWORD)-1;
    }
    if (nTarget > nEOF && SetEOF(nRefNum, nTarget) != noErr) {
        SetLastError(6);
        return (DWORD)-1;
    }
set_position:
    if (SetFPos(nRefNum, nPosMode, lDistanceToMove) != noErr) {
        SetLastError(6);
        return (DWORD)-1;
    }
    GetFPos(nRefNum, &nNewPos);
    return nNewPos;
}

// 1.14d mac: 000221d1
DWORD GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh) {
    int16_t nRefNum = GetWin32HandlePtr(hFile)->nRefNum;
    if (lpFileSizeHigh != NULL) {
        *lpFileSizeHigh = 0;
    }
    long nSize;
    if (GetEOF(nRefNum, &nSize) != noErr) {
        SetLastError(6);
        return 0;
    }
    return nSize;
}

// 1.14d mac: 00022222
// A locked file is unlocked first.
BOOL DeleteFile(LPCSTR lpFileName) {
    FSSpec tSpec;
    DWORD dwErrCode;
    if (FilepathToFSSpec(&tSpec, lpFileName, 0) == noErr) {
        CInfoPBRec tPB = {};
        tPB.hFileInfo.ioNamePtr = tSpec.name;
        tPB.hFileInfo.ioVRefNum = tSpec.vRefNum;
        tPB.hFileInfo.ioDirID = tSpec.parID;
        if (PBGetCatInfoSync(&tPB) == noErr) {
            if (!(tPB.hFileInfo.ioFlAttrib & kioFlAttribLockedMask) || FSpRstFLock(&tSpec) == noErr) {
                if (FSpDelete(&tSpec) == noErr) {
                    return 1;
                }
            }
        }
        dwErrCode = 6;
    } else {
        dwErrCode = 2;
    }
    SetLastError(dwErrCode);
    return 0;
}

// 1.14d mac: 0002232a
// Read-only from the lock bit, directory from the directory bit, hidden for a name
// that starts with a dot. A path naming a directory itself (empty leaf) asks for the
// directory's own entry.
DWORD GetFileAttributes(LPCSTR lpFileName) {
    Str255 szDirName;
    CInfoPBRec tPB;
    FSSpec tSpec;
    DWORD dwErrCode;
    if (FilepathToFSSpec(&tSpec, lpFileName, 0) == noErr) {
        if (tSpec.name[0] == 0) {
            tPB.hFileInfo.ioNamePtr = szDirName;
            tPB.hFileInfo.ioVRefNum = tSpec.vRefNum;
            tPB.hFileInfo.ioDirID = tSpec.parID;
            tPB.hFileInfo.ioFDirIndex = -1;
        } else {
            tPB.hFileInfo.ioNamePtr = tSpec.name;
            tPB.hFileInfo.ioVRefNum = tSpec.vRefNum;
            tPB.hFileInfo.ioDirID = tSpec.parID;
            tPB.hFileInfo.ioFDirIndex = 0;
        }
        OSErr nErr = PBGetCatInfoSync(&tPB);
        if (nErr == noErr) {
            DWORD dwAttributes =
                (tPB.hFileInfo.ioFlAttrib & kioFlAttribDirMask) | (tPB.hFileInfo.ioFlAttrib & kioFlAttribLockedMask);
            if (tSpec.name[0] != 0 && tSpec.name[1] == '.') {
                dwAttributes |= 2;
            }
            return dwAttributes != 0 ? dwAttributes : 0x80;
        }
        if (nErr == fnfErr) {
            goto not_found;
        }
        dwErrCode = 0x3EE;
    } else {
    not_found:
        dwErrCode = 2;
    }
    SetLastError(dwErrCode);
    return (DWORD)-1;
}

// 1.14d mac: 00022461
// The Mac keeps no access time: the last-access and last-write times both come from
// the modification date.
BOOL GetFileTime(HANDLE hFile, FILETIME* lpCreationTime, FILETIME* lpLastAccessTime, FILETIME* lpLastWriteTime) {
    Win32HandleObject* pFile = GetWin32HandlePtr(hFile);
    CInfoPBRec tPB;
    SMemZero(&tPB, sizeof(tPB));
    FSSpec tSpec;
    memcpy(&tSpec, &pFile->tSpec, sizeof(FSSpec));
    tPB.hFileInfo.ioVRefNum = tSpec.vRefNum;
    tPB.hFileInfo.ioDirID = tSpec.parID;
    tPB.hFileInfo.ioNamePtr = tSpec.name;
    tPB.hFileInfo.ioFDirIndex = 0;
    OSErr nErr = PBGetCatInfoSync(&tPB);
    if (nErr == noErr) {
        if (lpCreationTime != NULL && !MacDateTimeToFileTime(&tPB.hFileInfo.ioFlCrDat, lpCreationTime)) {
            nErr = paramErr;
        }
        if (lpLastAccessTime != NULL && !MacDateTimeToFileTime(&tPB.hFileInfo.ioFlMdDat, lpLastAccessTime)) {
            nErr = paramErr;
        }
        if (lpLastWriteTime != NULL && !MacDateTimeToFileTime(&tPB.hFileInfo.ioFlMdDat, lpLastWriteTime)) {
            nErr = paramErr;
        }
    }
    return nErr == noErr;
}

// 1.14d mac: 0002257e
// Only records the times on the handle; SetFileTimeAtClose writes them.
BOOL SetFileTime(HANDLE hFile, const FILETIME* lpCreationTime, const FILETIME* lpLastAccessTime,
                 const FILETIME* lpLastWriteTime) {
    Win32HandleObject* pFile = GetWin32HandlePtr(hFile);
    if (lpCreationTime != NULL) {
        pFile->tCreationTime = *lpCreationTime;
    }
    if (lpLastAccessTime != NULL) {
        pFile->tLastAccessTime = *lpLastAccessTime;
    }
    if (lpLastWriteTime != NULL) {
        pFile->tLastWriteTime = *lpLastWriteTime;
    }
    return 1;
}

// 1.14d mac: 000225d2
// A zero time is left as it is; nothing is written if a conversion fails.
void SetFileTimeAtClose(HANDLE hFile) {
    BYTE aUnused[4];
    Win32HandleObject* pFile = GetWin32HandlePtr(hFile);
    CInfoPBRec tPB;
    SMemZero(&tPB, sizeof(tPB));
    FSSpec tSpec;
    memcpy(&tSpec, &pFile->tSpec, sizeof(FSSpec));
    tPB.hFileInfo.ioVRefNum = tSpec.vRefNum;
    tPB.hFileInfo.ioDirID = tSpec.parID;
    tPB.hFileInfo.ioNamePtr = tSpec.name;
    tPB.hFileInfo.ioFDirIndex = 0;
    OSErr nErr = PBGetCatInfoSync(&tPB);
    tPB.hFileInfo.ioDirID = tSpec.parID;
    if (nErr == noErr) {
        if (pFile->tCreationTime.dwHighDateTime != 0 || pFile->tCreationTime.dwLowDateTime != 0) {
            if (!FileTimeToMacDateTime(&pFile->tCreationTime, &tPB.hFileInfo.ioFlCrDat, aUnused)) {
                nErr = paramErr;
            }
        }
        if (pFile->tLastWriteTime.dwHighDateTime != 0 || pFile->tLastWriteTime.dwLowDateTime != 0) {
            if (!FileTimeToMacDateTime(&pFile->tLastWriteTime, &tPB.hFileInfo.ioFlMdDat, aUnused)) {
                nErr = paramErr;
            }
        }
        if (nErr == noErr) {
            PBSetCatInfoSync(&tPB);
        }
    }
}

#endif  // D2_OS_MAC
