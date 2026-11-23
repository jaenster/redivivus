// Storm's Win32-on-Mac layer: current and temporary directories, the process exit
// hook, machine and user names, 1.14d Intel Mac band 0002a1d0..0002ab8e. See
// Win32Mac.h for what is and is not known about the original file boundaries.
//
// Directories come back as Mac path tokens ("\{MacPathNN}\", PATH_MakeMacPathToken),
// not as paths. Temporary files go to the Temporary Items folder of whichever of
// the system volume and the application's volume has more free space.
//
// GetCommandLine (0002ab8e) follows this band; it is Objective-C, so its file was
// compiled as Objective-C++, and it is not here.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <stdio.h>
#include <string.h>

typedef void (*Win32MacExitHandler)(void);

// 1.14d mac: 0002a16f
HANDLE GetModuleHandle(LPCSTR szModuleName);
// 1.14d mac: 0001eafe
// In place; Storm's own, not the Toolbox's.
unsigned char* c2pstr(char* szString);
// 1.14d mac: 0001eb1c
char* p2cstr(unsigned char* pString);
// 1.14d mac: 000347e7
CFStringRef MacCopyMachineName(void);
// 1.14d mac: 0003477a
CFStringRef MacCopyUserName(Boolean bLongName);

// 1.14d mac: 003f1a6c
static Win32MacExitHandler s_pfnExitHandler;
// 1.14d mac: 003f1a70
// FindMostEmptyVolume has run.
static bool s_bTempVolumeChosen;
// 1.14d mac: 003f1a72
static int16_t s_nTempVRefNum;

// 1.14d mac: 0002a1d0
// The directory of the process module's file, or the application's directory when
// the module has no path.
DWORD GetCurrentDirectory(DWORD nBufferLength, char* szBuffer) {
    Win32HandleObject* pModule = GetWin32HandlePtr(GetModuleHandle(NULL));
    if (pModule != NULL && pModule->szModulePath != NULL) {
        if (nBufferLength < 40) {
            return 0;
        }
        Win32MacPathToken* pDir = FilepathWorkingDirectory(pModule->szModulePath);
        PATH_MakeMacPathToken(szBuffer, "\p", pDir->nVRefNum, pDir->nDirID, 0, 0);
    } else {
        FSSpec tAppSpec;
        GetApplicationInformation(NULL, &tAppSpec, NULL, NULL);
        PATH_MakeMacPathToken(szBuffer, "\p", tAppSpec.vRefNum, tAppSpec.parID, 0, 0);
    }
    return strlen(szBuffer);
}

// 1.14d mac: 0002a2a2
// szPath made absolute (a relative one goes under the current directory) and, if
// the file exists, canonical: a token path plus the file's own name. A Windows path
// with no token becomes a Mac path relative to the token's directory; if that does
// not resolve, the result is the token followed by the Mac path's name.
char* PATH_MakeFullPath(char* szFullPath, LPCSTR szPath, DWORD nBufferSize) {
    char szMacPath[256];
    FSSpec tSpec;
    *szFullPath = '\0';
    if (*szPath != '\\') {
        GetCurrentDirectory(nBufferSize, szFullPath);
    }
    strcat(szFullPath, szPath);
    if (PATH_IsMacPathToken(szFullPath)) {
        if (FilepathToFSSpec((struct FSSpec*)&tSpec, szFullPath, 0) == noErr) {
            FSSpecToFilepath(szFullPath, &tSpec);
        }
    } else {
        strcpy(szMacPath, szFullPath);
        if (strchr(szMacPath, '\\') != NULL) {
            if (szMacPath[0] != '\\') {
                memmove(szMacPath + 1, szMacPath, 255);
                szMacPath[0] = '\\';
            }
            char* pSep;
            while ((pSep = strchr(szMacPath, '\\')) != NULL) {
                *pSep = ':';
            }
            c2pstr(szMacPath);
            Win32MacPathToken* pDir = FilepathWorkingDirectory(szFullPath);
                if (FSMakeFSSpec(pDir->nVRefNum, pDir->nDirID, (const unsigned char*)szMacPath, &tSpec) == noErr) {
                FSSpecToFilepath(szFullPath, &tSpec);
            } else {
                szFullPath[13] = '\0';
                p2cstr((unsigned char*)szMacPath);
                strcat(szFullPath, szMacPath);
            }
        }
    }
    return szFullPath;
}

// 1.14d mac: 0002a40b
// What ExitProcess calls instead of leaving to the shell.
void PROCESS_SetExitHandler(Win32MacExitHandler pfnExitHandler) {
    s_pfnExitHandler = pfnExitHandler;
}

// 1.14d mac: 0002a41f
void ExitProcess(UINT uExitCode) {
    if (s_pfnExitHandler != NULL) {
        s_pfnExitHandler();
        return;
    }
    ExitToShell();
}

// 1.14d mac: 0002a441
BOOL GetComputerName(char* szBuffer, DWORD* pnSize) {
    char szName[256] = {0};
    BOOL bResult = 0;
    if (szBuffer != NULL && pnSize != NULL && *pnSize != 0) {
        CFStringRef sName = MacCopyMachineName();
        CFStringGetCString(sName, szName, sizeof(szName), kCFStringEncodingMacRoman);
        CFRelease(sName);
        snprintf(szBuffer, *pnSize, "%s", szName);
        szBuffer[*pnSize - 1] = '\0';
        *pnSize = strlen(szBuffer);
        bResult = *pnSize != 0;
    }
    return bResult;
}

// 1.14d mac: 0002a56b
BOOL GetUserName(char* szBuffer, DWORD* pnSize) {
    char szName[256] = {0};
    BOOL bResult = 0;
    if (szBuffer != NULL && pnSize != NULL && *pnSize != 0) {
        CFStringRef sName = MacCopyUserName(1);
        CFStringGetCString(sName, szName, sizeof(szName), kCFStringEncodingMacRoman);
        CFRelease(sName);
        snprintf(szBuffer, *pnSize, "%s", szName);
        szBuffer[*pnSize - 1] = '\0';
        *pnSize = strlen(szBuffer);
        bResult = *pnSize != 0;
    }
    return bResult;
}

void FindMostEmptyVolume(void);

// 1.14d mac: 0002a6a3
// Free bytes on the temporary-files volume (32 bits: wraps above 4 GB).
DWORD GetTempPathFreeSpace(void) {
    HParamBlockRec tPB;
    if (!s_bTempVolumeChosen) {
        FindMostEmptyVolume();
    }
    SMemZero(&tPB, sizeof(tPB));
    tPB.volumeParam.ioVRefNum = s_nTempVRefNum;
    DWORD dwFree = 0;
    if (PBHGetVInfoSync(&tPB) == noErr) {
        dwFree = tPB.volumeParam.ioVFrBlk * tPB.volumeParam.ioVAlBlkSiz;
    }
    return dwFree;
}

// 1.14d mac: 0002a708
// The system volume's Temporary Items, or the application volume's if it has more
// free space.
void FindMostEmptyVolume(void) {
    HParamBlockRec tPB;
    int16_t nVRefNum;
    SInt32 nDirID;
    s_nTempVRefNum = kOnSystemDisk;
    if (FindFolder(kOnSystemDisk, kTemporaryFolderType, kCreateFolder, &nVRefNum, &nDirID) == noErr) {
        s_nTempVRefNum = nVRefNum;
        SMemZero(&tPB, sizeof(tPB));
        tPB.volumeParam.ioVRefNum = nVRefNum;
        if (PBHGetVInfoSync(&tPB) == noErr) {
            DWORD dwSystemFree = tPB.volumeParam.ioVFrBlk * tPB.volumeParam.ioVAlBlkSiz;
            if (FindFolder(0, kTemporaryFolderType, kCreateFolder, &nVRefNum, &nDirID) == noErr &&
                nVRefNum != s_nTempVRefNum) {
                SMemZero(&tPB, sizeof(tPB));
                tPB.volumeParam.ioVRefNum = nVRefNum;
                if (PBHGetVInfoSync(&tPB) == noErr &&
                    dwSystemFree < (DWORD)(tPB.volumeParam.ioVFrBlk * tPB.volumeParam.ioVAlBlkSiz)) {
                    s_nTempVRefNum = nVRefNum;
                }
            }
        }
    }
    s_bTempVolumeChosen = true;
}

// 1.14d mac: 0002a83d
DWORD GetTempPath(DWORD nBufferLength, char* szBuffer) {
    int16_t nVRefNum;
    SInt32 nDirID;
    if (!s_bTempVolumeChosen) {
        FindMostEmptyVolume();
    }
    DWORD nLength = 0;
    if (nBufferLength >= 40 &&
        FindFolder(s_nTempVRefNum, kTemporaryFolderType, kCreateFolder, &nVRefNum, &nDirID) == noErr) {
        PATH_MakeMacPathToken(szBuffer, "\p", nVRefNum, nDirID, 0, 0);
        nLength = strlen(szBuffer);
    }
    return nLength;
}

// 1.14d mac: 0002a8df
// Free space counts as available to the caller too. A path that does not exist
// still names its volume (fnfErr).
BOOL GetDiskFreeSpaceEx(LPCSTR szDirectory, uint64_t* pnFreeBytesAvailable, uint64_t* pnTotalBytes,
                        uint64_t* pnTotalFreeBytes) {
    HParamBlockRec tPB;
    FSSpec tSpec;
    BOOL bResult = 0;
    OSErr nErr = FilepathToFSSpec((struct FSSpec*)&tSpec, szDirectory, 0);
    if (nErr == noErr || nErr == fnfErr) {
        SMemZero(&tPB, sizeof(tPB));
        tPB.volumeParam.ioVRefNum = tSpec.vRefNum;
        if (PBHGetVInfoSync(&tPB) == noErr) {
            *pnTotalBytes = (DWORD)(tPB.volumeParam.ioVNmAlBlks * tPB.volumeParam.ioVAlBlkSiz);
            *pnFreeBytesAvailable = (DWORD)(tPB.volumeParam.ioVFrBlk * tPB.volumeParam.ioVAlBlkSiz);
            bResult = 1;
            if (pnTotalFreeBytes != NULL) {
                *pnTotalFreeBytes = (DWORD)(tPB.volumeParam.ioVFrBlk * tPB.volumeParam.ioVAlBlkSiz);
            }
        }
    }
    return bResult;
}

// 1.14d mac: 0002a9a9
// With uUnique 0, creates the first free "<path>\<prefix><hex>.TMP", counting up
// from the tick count, and returns its token path; otherwise only formats the name.
UINT GetTempFileName(LPCSTR szPathName, LPCSTR szPrefix, UINT uUnique, char* szTempFileName) {
    char szFileName[256];
    FSSpec tSpec;
    DWORD dwCreator;
    if (uUnique != 0) {
        strcpy(szTempFileName, szPathName);
        strcpy(szFileName, szPrefix);
        size_t nPrefixLength = strlen(szPrefix);
        HEX_FormatDword(uUnique, szFileName + strlen(szFileName));
        szFileName[nPrefixLength + 8] = '\0';
        strcat(szTempFileName, szFileName);
    } else {
        UINT uTick = TickCount();
        const char* szSeparator = szPathName[strlen(szPathName) - 1] == '\\' ? "" : "\\";
        OSErr nErr;
        do {
            sprintf(szFileName, "%s%s%s%X.TMP", szPathName, szSeparator, szPrefix, uTick);
            nErr = FilepathToFSSpec((struct FSSpec*)&tSpec, szFileName, 0);
            if (nErr == fnfErr) {
                dwCreator = g_dwNewFileCreator;
                DWORD dwType = g_dwNewFileType;
                if (dwCreator == 0) {
                    GetApplicationInformation(NULL, NULL, &dwCreator, NULL);
                }
                if (dwType == 0) {
                    dwType = GetFileTypeAndCreatorForPath(szFileName, &dwCreator);
                }
                FSpCreate(&tSpec, dwCreator, dwType, smSystemScript);
                break;
            }
            uTick += 0x19919;
        } while (nErr == noErr);
        FSSpecToFilepath(szTempFileName, &tSpec);
    }
    return strlen(szTempFileName);
}

#endif  // D2_OS_MAC
