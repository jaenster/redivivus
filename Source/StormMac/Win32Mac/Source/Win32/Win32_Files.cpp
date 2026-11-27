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
#include "Win32GDI.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <CoreServices/CoreServices.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

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

// MoreFiles (third party), 1.14d mac: 00036f91
extern "C" OSErr FSpIterateDirectory(const FSSpec* pSpec, short nMaxLevels,
                                     void (*pfnFilter)(const CInfoPBRec*, Boolean*, void*), void* pYourData);
bool PATH_IsSeparator(char cChar);
void FlushPartialPathCache(void);
void SetLastError(DWORD dwErrCode);

// 1.14d mac: 000226f8
// A path that is not a Mac path token succeeds without doing anything; a directory that
// exists sets ERROR_ALREADY_EXISTS and still succeeds.
BOOL CreateDirectory(char* szPathName, void* pSecurityAttributes) {
    BOOL bResult = 1;
    if (PATH_IsMacPathToken(szPathName) && szPathName[0xD] != '\0') {
        char szTrimmed[261];
        char* szPath = szPathName;
        size_t nLen = strlen(szPathName);
        if (PATH_IsSeparator(szPathName[nLen - 1])) {
            szPath = szTrimmed;
            strncpy(szPath, szPathName, nLen - 1);
            szTrimmed[nLen - 1] = '\0';
        }
        FSSpec tSpec;
        FilepathToFSSpec(&tSpec, szPath, 0);
        OSErr nErr = noErr;
        bool bExists = false;
        if (tSpec.name[0] != '\0') {
            SInt32 nCreatedDirId;
            nErr = FSpDirCreate(&tSpec, smSystemScript, &nCreatedDirId);
            if (nErr == dupFNErr) {
                SetLastError(0xB7);
                nErr = dupFNErr;
                bExists = true;
            }
        }
        bResult = (nErr == noErr) | bExists;
        FlushPartialPathCache();
    }
    return bResult;
}

struct FindMatchContext {
    Win32MacFindList* pList;      // 0x000
    unsigned char aMask[256];     // 0x004  Pascal string
    OSType nType;                 // 0x104  0 = any
    OSType nCreator;              // 0x108  0 = any
};

// 1.14d mac: 00022a33
// FSpIterateDirectory's callback: the entry is recorded at the end of the list and kept
// (the count moves on) when it matches the mask and the type and creator.
void FindFile_MatchCallback(const CInfoPBRec* pCatInfo, Boolean* pbQuit, void* pvContext) {
    FindMatchContext* pContext = (FindMatchContext*)pvContext;
    Win32MacFindList* pList = pContext->pList;
    Win32MacFSSpec* pEntry = &pList->pEntries[pList->nCount];
    pEntry->vRefNum = pCatInfo->hFileInfo.ioVRefNum;
    pEntry->parID = pCatInfo->hFileInfo.ioFlParID;
    memmove(pEntry->name, pCatInfo->hFileInfo.ioNamePtr, pCatInfo->hFileInfo.ioNamePtr[0] + 1);
    const unsigned char* aMask = pContext->aMask;
    unsigned nMaskLen = aMask[0];
    if (nMaskLen != 0) {
        const unsigned char* pName = pCatInfo->hFileInfo.ioNamePtr;
        if (aMask[1] == '*') {
            if (aMask[2] == '.' && aMask[3] != '*') {
                if (_strnicmp((const char*)aMask + 2, (const char*)pName + (2 - nMaskLen) + pName[0], nMaskLen - 1) !=
                    0) {
                    goto done;
                }
            } else if (nMaskLen != 1 && (nMaskLen != 3 || aMask[2] != '.' || aMask[3] != '*')) {
                goto done;
            }
        } else {
            if (aMask[nMaskLen] == '*') {
                if (_strnicmp((const char*)aMask + 1, (const char*)pName + 1, nMaskLen - 1) != 0) {
                    goto done;
                }
            } else if (!EqualString(aMask, pName, false, false)) {
                goto done;
            }
        }
    }
    if ((pContext->nCreator == 0 || pContext->nCreator == pCatInfo->hFileInfo.ioFlFndrInfo.fdCreator) &&
        (pContext->nType == 0 || pContext->nType == pCatInfo->hFileInfo.ioFlFndrInfo.fdType)) {
        pList->nCount++;
    }
done:
    *pbQuit = pList->nCapacity <= pList->nCount;
}

BOOL FindNextFile(HANDLE hFind, Win32MacFindData* pFindData);
BOOL FindClose(HANDLE hFind);

// 1.14d mac: 000227f8
// Collects the matches of the directory into a find handle (type 3) and returns it with
// the first entry; the mask "*.*" (or "*" without type/creator) answers with "..".
HANDLE FindFirstFile(char* szPattern, Win32MacFindData* pFindData) {
    HANDLE hFind = 0;
    if (PATH_IsMacPathToken(szPattern)) {
        char szDir[260];
        char szMask[260];
        strncpy(szDir, szPattern, 0x104);
        char* pSlash = strrchr(szDir, '\\');
        strcpy(szMask, pSlash + 1);
        pSlash[1] = '\0';
        FSSpec tSpec;
        if (FilepathToFSSpec(&tSpec, szDir, 0) == 0) {
            Win32MacPathToken* pToken = FilepathWorkingDirectory(szDir);
            int32_t nCreator = pToken->n08;
            int32_t nType = pToken->n0C;
            Win32MacFindList* pList = (Win32MacFindList*)MemAlloc(sizeof(Win32MacFindList), 0x100);
            pList->pEntries = (Win32MacFSSpec*)MemAlloc(70000, 0x100);
            pList->nCapacity = 1000;
            FindMatchContext tContext;
            tContext.pList = pList;
            bzero(tContext.aMask, sizeof(tContext.aMask));
            tContext.nType = nType;
            tContext.nCreator = nCreator;
            CopyCStringToPascal(szMask, tContext.aMask);
            FSpIterateDirectory(&tSpec, 1, FindFile_MatchCallback, &tContext);
            pList->pEntries = (Win32MacFSSpec*)MemReAlloc(pList->pEntries, pList->nCount * sizeof(Win32MacFSSpec));
            pList->nCapacity = pList->nCount;
            hFind = AllocWin32Handle(3);
            GetWin32HandlePtr(hFind)->pType5Data = pList;
            if (strcmp("*.*", szMask) != 0 && (strcmp("*", szMask) != 0 || nType != 0 || nCreator != 0)) {
                if (!FindNextFile(hFind, pFindData)) {
                    FindClose(hFind);
                    hFind = 0;
                }
                return hFind;
            }
            bzero(pFindData, sizeof(Win32MacFindData));
            pFindData->dwFileAttributes = 0x10;
            pFindData->cFileName[2] = '\0';
            pFindData->cFileName[0] = '.';
            pFindData->cFileName[1] = '.';
        }
    }
    return hFind;
}

// 1.14d mac: 00022b6a
// Entries whose catalog lookup fails are skipped; 0 when none are left.
BOOL FindNextFile(HANDLE hFind, Win32MacFindData* pFindData) {
    Win32HandleObject* pHandle = GetWin32HandlePtr(hFind);
    Win32MacFindList* pList;
    Win32MacFSSpec* pEntry;
    CInfoPBRec tPB;
    do {
        pList = (Win32MacFindList*)pHandle->pType5Data;
        int nIndex = pList->nNext;
        if (nIndex >= pList->nCount) {
            return 0;
        }
        pList->nNext = nIndex + 1;
        pEntry = &pList->pEntries[nIndex];
        bzero(pFindData, sizeof(Win32MacFindData));
        tPB.hFileInfo.ioFDirIndex = 0;
        tPB.hFileInfo.ioVRefNum = pEntry->vRefNum;
        tPB.hFileInfo.ioDirID = pEntry->parID;
        tPB.hFileInfo.ioNamePtr = pEntry->name;
    } while (PBGetCatInfoSync(&tPB) != noErr);
    CopyPascalStringToC(pEntry->name, pFindData->cFileName);
    if (tPB.hFileInfo.ioFlAttrib & 0x10) {
        pFindData->dwFileAttributes |= 0x10;
    }
    if (pEntry->name[0] != 0 && pEntry->name[1] == '.') {
        pFindData->dwFileAttributes |= 2;
    }
    MacDateTimeToFileTime(&tPB.hFileInfo.ioFlCrDat, &pFindData->tCreationTime);
    MacDateTimeToFileTime(&tPB.hFileInfo.ioFlMdDat, &pFindData->tLastAccessTime);
    MacDateTimeToFileTime(&tPB.hFileInfo.ioFlMdDat, &pFindData->tLastWriteTime);
    pFindData->nFileSizeLow = tPB.hFileInfo.ioFlLgLen;
    return 1;
}

// 1.14d mac: 00022c53
BOOL FindClose(HANDLE hFind) {
    Win32HandleObject* pHandle = GetWin32HandlePtr(hFind);
    Win32MacFindList* pList = (Win32MacFindList*)pHandle->pType5Data;
    MemFree(pList->pEntries, 0);
    MemFree(pList, 0);
    FreeWin32Handle(hFind);
    return 1;
}

#endif  // D2_OS_MAC
