// StormMac Win32 Mac/Source/Win32/Win32_Process.cpp: modules, the last-error code, directories, the process exit hook, machine
// and user names, temporary files, interlocked operations and critical sections, 1.14d
// Intel Mac 00029b68..0002b1fe (LoadLibrary .. WSOCK_CloseSocket).
//
// Pinned by the PPC Storm 1.10 Beta: the run its __sinit_Win32_Process_cpp closes holds,
// read last to first as MWCC emits a file, LoadLibrary, LoadLibraryEx, FreeLibrary,
// GetProcAddress, GetModuleFileName, GetModuleHandle, GetCurrentDirectory, GetFullPathName,
// InstallExitProcessCallback, ExitProcess, GetComputerName, GetUserName, the temp-path
// functions, GetCommandLine, the Interlocked functions, the critical sections, the locale
// queries, GetLastError, SetLastError and MessageBox: 1.14d's run from 00029b68 to
// 0002b1fe. The Priv_ allocator and clipboard linked just before it are another unit
// (Win32_Priv.cpp).
// SetLastError is inlined into GetModuleFileName in 1.14d, so they share a unit there too.
// The folder is that of Win32_utils.cpp, the one Win32_ file whose 1.14d path is in the
// binary. GetCommandLine (0002ab8e) is Objective-C (Win32_CommandLine.mm) and the locale functions
// (0002af55..0002b1c0), GetLastError and MessageBox are not reconstructed yet.
//
// ddraw.dll, dsound.dll, advapi32.dll and wsock32.dll are not loaded: LoadLibrary
// hands out the fixed pseudo-handles 0xC0000002..0xC0000005 for them, and
// GetProcAddress answers from the layer's own entry points. Every other module is
// a type-4 kernel object. Directories come back as Mac path tokens
// ("\{MacPathNN}\", PATH_MakeMacPathToken), not as paths. Temporary files go to the
// Temporary Items folder of whichever of the system volume and the application's volume
// has more free space. Win32 critical sections are re-entrant: recursive mutexes.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <dlfcn.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define MODULE_DDRAW ((HANDLE)0xC0000002)
#define MODULE_DSOUND ((HANDLE)0xC0000003)
#define MODULE_ADVAPI32 ((HANDLE)0xC0000004)
#define MODULE_WSOCK32 ((HANDLE)0xC0000005)
#define IS_BUILTIN_MODULE(h) ((DWORD)(h) - (DWORD)MODULE_DDRAW < 4)

#define DLL_PROCESS_DETACH 0
#define DLL_PROCESS_ATTACH 1
#define LOAD_LIBRARY_AS_DATAFILE 0x00000002

typedef BOOL (*Win32MacDllMain)(HANDLE hModule, DWORD dwReason, LPVOID pReserved);

// 1.14d mac: 0002b1d1
int MessageBox(void* hWnd, LPCSTR szText, LPCSTR szCaption, UINT uType);
// Win32_FP.cpp
int FPMessageBox(const char* szText, const char* szCaption, DWORD dwFlags);
int32_t GetLanguage(void);
int32_t GetUserDefaultLangID(void);
DWORD LOCALE_BuildLangIdFromMac(void);
// Defined below (0002a41f).
void ExitProcess(UINT uExitCode);
// 1.14d mac: 00033a24
int _stricmp(const char* szA, const char* szB);
// 1.14d mac: 000344dc
OSErr MacResolveAliasFile(FSSpec* pSpec, Boolean bResolveChains, Boolean* pbTargetIsFolder, Boolean* pbWasAliased,
                          Boolean bNoUI);
// 1.14d mac: 0002c8f7
void FSSpecToFilepath(char* szPath, const FSSpec* pSpec);
// 1.14d mac: 0002cd41
int PATH_ToPosixPath(const char* szPath, char* szPosixPath, int nSize);
// 1.14d mac: 00025a77
HANDLE AllocModuleHandle(void);
// 1.14d mac: 00036fd3
struct AccelNode;
void Accel_DeleteAcceleratorList(AccelNode* pList);
// 1.14d mac: 0002d039
HANDLE HANDLE_FindModuleByPSN(ProcessSerialNumber* pPSN);
// 1.14d mac: 0002d0d2
HANDLE HANDLE_FindModuleByName(LPCSTR szModuleName);
// 1.14d mac: 0002b1f5
int WSOCK_CloseSocket(int nSocket);
// 1.14d mac: 00032b50
int DirectDrawCreate(void* pGuid, int** ppDirectDraw, void* pUnknownOuter);
// 1.14d mac: 00032b83
struct IDirectSound;
int DirectSoundCreate(void* pGuid, IDirectSound** ppDirectSound, void* pUnknownOuter);

FARPROC GetProcAddress(HANDLE hModule, LPCSTR szProcName);
uint16_t WSOCK_Htons(uint16_t wValue);
int WSOCK_WSAStartup(uint16_t wVersionRequested, void* pWsaData);
int WSOCK_WSACleanup(void);

// 1.14d mac: 003f227c
// GetLastError's value.
DWORD g_dwLastError;

HANDLE LoadLibraryEx(LPCSTR szFileName, HANDLE hFile, DWORD dwFlags);

// 1.14d mac: 00029b68
// hFile is passed on unset: LoadLibraryEx never reads it.
HANDLE LoadLibrary(LPCSTR szFileName) {
    HANDLE hFile;
    return LoadLibraryEx(szFileName, hFile, 0);
}

// 1.14d mac: 00029b86
// A built-in name gets its pseudo-handle. Anything else is resolved to a file and
// dlopened, or with LOAD_LIBRARY_AS_DATAFILE its resource fork's 'WRes' resource
// (named after the file) is kept instead; the module object records the path. A
// loaded library's DllMain runs, and a zero from it fails the load.
HANDLE LoadLibraryEx(LPCSTR szFileName, HANDLE hFile, DWORD dwFlags) {
    char szPath[256];
    char szPosixPath[1024];
    FSSpec tSpec;
    Boolean bTargetIsFolder;
    Boolean bWasAliased;

    if (_stricmp(szFileName, "ddraw.dll") == 0) {
        return MODULE_DDRAW;
    }
    if (_stricmp(szFileName, "dsound.dll") == 0) {
        return MODULE_DSOUND;
    }
    if (_stricmp(szFileName, "advapi32.dll") == 0) {
        return MODULE_ADVAPI32;
    }
    if (_stricmp(szFileName, "wsock32.dll") == 0) {
        return MODULE_WSOCK32;
    }

    FilepathToFSSpec(&tSpec, szFileName, 0);
    MacResolveAliasFile(&tSpec, true, &bTargetIsFolder, &bWasAliased, false);
    if (bTargetIsFolder) {
        return 0;
    }
    DWORD dwDataFile = dwFlags & LOAD_LIBRARY_AS_DATAFILE;
    FSSpecToFilepath(szPath, &tSpec);

    void* pLibrary = NULL;
    Handle hResource = NULL;
    if (dwDataFile) {
        int16_t nResFile = FSpOpenResFile(&tSpec, fsRdPerm);
        if (nResFile == -1) {
            return 0;
        }
        if (ResError() != noErr) {
            return 0;
        }
        hResource = Get1NamedResource('WRes', tSpec.name);
        if (hResource != NULL) {
            DetachResource(hResource);
            HLockHi(hResource);
        }
        CloseResFile(nResFile);
    } else {
        if (PATH_ToPosixPath(szPath, szPosixPath, sizeof(szPosixPath)) != 0) {
            return 0;
        }
        pLibrary = dlopen(szPosixPath, RTLD_LAZY | RTLD_LOCAL);
        if (pLibrary == NULL) {
            return 0;
        }
    }

    HANDLE hModule = AllocModuleHandle();
    Win32HandleObject* pModule = GetWin32HandlePtr(hModule);
    size_t nSize = strlen(szPath) + 1;
    pModule->szModulePath = (char*)MemAlloc(nSize, 0);
    pModule->szModulePath = (char*)memcpy(pModule->szModulePath, szPath, nSize);
    if (IS_BUILTIN_MODULE(hModule)) {
        fp_fatal("LoadLibrary builtin collision");
    }
    pModule = GetWin32HandlePtr(hModule);
    pModule->pLibrary = pLibrary;
    pModule->dwPSNHigh = 0;
    pModule->dwPSNLow = 0;
    pModule->pAccelList = NULL;
    pModule->hResource = hResource;

    if (!dwDataFile) {
        Win32MacDllMain pfnDllMain = (Win32MacDllMain)GetProcAddress(hModule, "DllMain");
        if (pfnDllMain != NULL) {
            return pfnDllMain(hModule, DLL_PROCESS_ATTACH, NULL) ? hModule : 0;
        }
    }
    return hModule;
}

// 1.14d mac: 00029e86
// A built-in module answers from the layer: ddraw and dsound by name, wsock32 by
// ordinal, advapi32 with nothing. A loaded one goes to dlsym.
FARPROC GetProcAddress(HANDLE hModule, LPCSTR szProcName) {
    FARPROC pfn = NULL;
    switch (hModule) {
        case MODULE_DDRAW:
            if (_stricmp(szProcName, "DirectDrawCreate") == 0) {
                pfn = (FARPROC)DirectDrawCreate;
            }
            break;
        case MODULE_DSOUND:
            if (_stricmp(szProcName, "DirectSoundCreate") == 0) {
                pfn = (FARPROC)DirectSoundCreate;
            }
            break;
        case MODULE_ADVAPI32:
            break;
        case MODULE_WSOCK32:
            switch ((DWORD)szProcName) {
                case 2:
                    pfn = (FARPROC)bind;
                    break;
                case 3:
                    pfn = (FARPROC)WSOCK_CloseSocket;
                    break;
                case 7:
                    pfn = (FARPROC)gethostbyname;
                    break;
                case 9:
                    pfn = (FARPROC)WSOCK_Htons;
                    break;
                case 17:
                    pfn = (FARPROC)recvfrom;
                    break;
                case 20:
                    pfn = (FARPROC)sendto;
                    break;
                case 21:
                    pfn = (FARPROC)setsockopt;
                    break;
                case 23:
                    pfn = (FARPROC)socket;
                    break;
                case 115:
                    pfn = (FARPROC)WSOCK_WSAStartup;
                    break;
                case 116:
                    pfn = (FARPROC)WSOCK_WSACleanup;
                    break;
            }
            break;
        default:
            pfn = (FARPROC)dlsym(GetWin32HandlePtr(hModule)->pLibrary, szProcName);
            break;
    }
    return pfn;
}

// 1.14d mac: 00029fec
// Detaches and unloads a loaded library and releases what the module holds. A
// built-in module is left alone. Always succeeds.
BOOL FreeLibrary(HANDLE hModule) {
    if (IS_BUILTIN_MODULE(hModule)) {
        return 1;
    }
    Win32HandleObject* pModule = GetWin32HandlePtr(hModule);
    void* pLibrary = pModule->pLibrary;
    if (pLibrary != NULL) {
        Win32MacDllMain pfnDllMain = (Win32MacDllMain)GetProcAddress(hModule, "DllMain");
        if (pfnDllMain != NULL) {
            pfnDllMain(hModule, DLL_PROCESS_DETACH, NULL);
        }
        dlclose(pLibrary);
    }
    if (pModule->pAccelList != NULL) {
        Accel_DeleteAcceleratorList((AccelNode*)pModule->pAccelList);
        pModule->pAccelList = NULL;
    }
    pModule = GetWin32HandlePtr(hModule);
    if (pModule->hResource != NULL) {
        DisposeHandle((Handle)pModule->hResource);
        pModule->hResource = NULL;
    }
    CloseHandle(hModule);
    return 1;
}

// 1.14d mac: 0002a0a2
// wsock32 htons (and ntohs): the Mac is big-endian only on PPC, so this swaps.
uint16_t WSOCK_Htons(uint16_t wValue) {
    return (uint16_t)((wValue << 8) | (wValue >> 8));
}

// 1.14d mac: 0002a0b2
int WSOCK_WSAStartup(uint16_t wVersionRequested, void* pWsaData) {
    return 0;
}

// 1.14d mac: 0002a0b9
int WSOCK_WSACleanup(void) {
    return 0;
}

// 1.14d mac: 0002a0c0
// The 'WRes' resource a data-file module holds; none for a built-in module.
void* RESOURCE_GetModuleResourceMap(HANDLE hModule) {
    if (IS_BUILTIN_MODULE(hModule)) {
        return NULL;
    }
    return GetWin32HandlePtr(hModule)->hResource;
}

HANDLE GetModuleHandle(LPCSTR szModuleName);
void SetLastError(DWORD dwErrCode);

// 1.14d mac: 0002a0ea
// The module's path (the process's when the handle is not a module); the length
// copied, nSize when it did not fit.
DWORD GetModuleFileName(HANDLE hModule, char* szFileName, DWORD nSize) {
    Win32HandleObject* pModule = GetWin32HandlePtr(hModule);
    if (pModule == NULL) {
        pModule = GetWin32HandlePtr(GetModuleHandle(NULL));
        if (pModule == NULL) {
            SetLastError(6);
            return 0;
        }
    }
    if (pModule->szModulePath == NULL) {
        SetLastError(0x649);
        return 0;
    }
    DWORD nLength = strlcpy(szFileName, pModule->szModulePath, nSize);
    if (nLength >= nSize) {
        SetLastError(0x7A);
        nLength = nSize;
    }
    return nLength;
}

// 1.14d mac: 0002a16f
// NULL is the running process. The lookup adds a reference, which is dropped: the
// handle is returned borrowed.
HANDLE GetModuleHandle(LPCSTR szModuleName) {
    HANDLE hModule;
    if (szModuleName != NULL) {
        hModule = HANDLE_FindModuleByName(szModuleName);
    } else {
        ProcessSerialNumber tPSN;
        if (GetCurrentProcess(&tPSN) != noErr) {
            return 0;
        }
        hModule = HANDLE_FindModuleByPSN(&tPSN);
    }
    if (hModule == 0) {
        return 0;
    }
    CloseHandle(hModule);
    return hModule;
}

// 1.14d mac: 0002a1bc
void SetLastError(DWORD dwErrCode) {
    g_dwLastError = dwErrCode;
}

typedef void (*Win32MacExitHandler)(void);

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

// 1.14d mac: 0002ae74
LONG InterlockedDecrement(LPLONG pnValue) {
    return __sync_sub_and_fetch(pnValue, 1);
}

// 1.14d mac: 0002ae86
LONG InterlockedIncrement(LPLONG pnValue) {
    return __sync_add_and_fetch(pnValue, 1);
}

// 1.14d mac: 0002ae98
LONG InterlockedExchange(LPLONG pnTarget, LONG nValue) {
    LONG nOld;
    do {
        nOld = *pnTarget;
    } while (__sync_val_compare_and_swap(pnTarget, nOld, nValue) != nOld);
    return nOld;
}

// 1.14d mac: 0002aeb3
LONG InterlockedCompareExchange(LPLONG pnDest, LONG nExchange, LONG nComparand) {
    return __sync_val_compare_and_swap(pnDest, nComparand, nExchange);
}

// 1.14d mac: 0002aec5
// The same body as InterlockedCompareExchange; the network code calls it on its
// pointer-sized state words. Name not recovered (Ghidra: FUN_0002aec5).
LONG FUN_0002aec5(LPLONG pnDest, LONG nExchange, LONG nComparand) {
    return __sync_val_compare_and_swap(pnDest, nComparand, nExchange);
}

// 1.14d mac: 0002aeda
// Win32 critical sections are re-entrant, so the mutex is a recursive one.
void InitializeCriticalSection(LPCRITICAL_SECTION pCriticalSection) {
    pthread_mutexattr_t tAttr;
    pthread_mutexattr_init(&tAttr);
    pthread_mutexattr_settype(&tAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(pCriticalSection, &tAttr);
    pthread_mutexattr_destroy(&tAttr);
}

// 1.14d mac: 0002af3a
void EnterCriticalSection(LPCRITICAL_SECTION pCriticalSection) {
    pthread_mutex_lock(pCriticalSection);
}

// 1.14d mac: 0002af43
void LeaveCriticalSection(LPCRITICAL_SECTION pCriticalSection) {
    pthread_mutex_unlock(pCriticalSection);
}

// 1.14d mac: 0002af4c
void DeleteCriticalSection(LPCRITICAL_SECTION pCriticalSection) {
    pthread_mutex_destroy(pCriticalSection);
}

// Mac region code (the 'itlc' word) to Win32 sublanguage.
// 1.14d mac: 00321ea8
const DWORD g_aRegionSubLangId[0x6B] = {
    1, 1, 2, 1, 1, 1, 2, 1, 1, 0, 2, 3, 1, 0, 0, 3,
    0, 2, 4, 2, 0, 0, 0, 0, 0, 0, 1, 5, 4, 4, 2, 1,
    0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 6, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 0, 0, 0,
    0, 0, 2, 0, 4, 2, 0, 0, 0, 0, 0,
};

// Mac script/language code (the 'itlb' word) to Win32 primary language.
// 1.14d mac: 00322054
const DWORD g_aMacLangPrimaryId[0x95] = {
    9, 12, 7, 16, 19, 29, 10, 6, 22, 20, 13, 17, 1, 11, 8, 15,
    0, 31, 26, 0, 0, 0, 30, 18, 39, 21, 14, 37, 38, 0, 56, 41,
    25, 4, 0, 0, 28, 24, 5, 27, 36, 0, 26, 0, 2, 34, 35, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    42, 33, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 45, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0,
    0, 0, 0, 0, 0,
};

// The LCIDs GetLocaleInfo knows by name: primary language, sublanguage, and the index into
// g_aLocaleStrings.
struct Win32MacLocaleMap {
    WORD wPrimaryLang;
    WORD wSubLang;
    DWORD dwStringsIndex;
};
// 1.14d mac: 003972a4
Win32MacLocaleMap g_aLocaleMap[8] = {
    {9, 1, 0},
    {12, 1, 1},
    {9, 2, 2},
    {7, 1, 3},
    {16, 1, 4},
    {19, 1, 5},
    {10, 1, 8},
    {22, 1, 71},
};

// Per region: language code, country code, abbreviated country name, country name.
struct Win32MacLocaleStrings {
    const char* szLanguageCode;   // LOCALE_SABBREVLANGNAME
    const char* szCountryCode;    // LOCALE_ICOUNTRY
    const char* szCountryAbbrev;  // LOCALE_SABBREVCTRYNAME
    const char* szCountryName;    // LOCALE_SENGCOUNTRY
};
// 1.14d mac: 003972e4
Win32MacLocaleStrings g_aLocaleStrings[0x6C] = {
    {"ENU", "1", "USA", "United States"},
    {"FRA", "33", "FRA", "France"},
    {"ENG", "44", "GBR", "United Kingdom"},
    {"DEU", "49", "DEU", "Germany"},
    {"ITA", "39", "ITA", "Italy"},
    {"NLD", "31", "NLD", "Netherlands"},
    {"NLB", "32", "BEL", "Belgium"},
    {"SVE", "46", "SWE", "Sweden"},
    {"ESP", "34", "ESP", "Spain"},
    {"DAN", "45", "DNK", "Denmark"},
    {"PTG", "351", "PRT", "Portugal"},
    {"FRC", "2", "CAN", "Canada"},
    {"NOR", "47", "NOR", "Norway"},
    {"HEB", "972", "ISR", "Israel"},
    {"", "", "", ""},
    {"ENA", "61", "AUS", "Australia"},
    {"", "", "", ""},
    {"FIN", "358", "FIN", "Finland"},
    {"FRS", "41", "CHE", "Switzerland"},
    {"DES", "41", "CHE", "Switzerland"},
    {"ELL", "30", "GRC", "Greece"},
    {"ISL", "354", "ISL", "Iceland"},
    {"", "", "", ""},
    {"", "", "", ""},
    {"TRK", "90", "TUR", "Turkey"},
    {"", "", "", ""},
    {"NLD", "31", "NLD", "Netherlands"},
    {"DEL", "352", "LUX", "Luxembourg"},
    {"ENC", "2", "CAN", "Canada"},
    {"ENC", "2", "CAN", "Canada"},
    {"PTG", "351", "PRT", "Portugal"},
    {"NOR", "47", "NOR", "Norway"},
    {"DAN", "45", "DNK", "Denmark"},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"ITS", "41", "CHE", "Switzerland"},
    {"", "", "", ""},
    {"", "", "", ""},
    {"ROM", "40", "ROM", "Romania"},
    {"", "", "", ""},
    {"LTH", "370", "LTU", "Lithuania"},
    {"PLK", "48", "POL", "Poland"},
    {"HUN", "36", "HUN", "Hungary"},
    {"ETI", "372", "EST", "Estonia"},
    {"LVI", "371", "LVA", "Latvia"},
    {"", "", "", ""},
    {"", "", "", ""},
    {"FAR", "981", "IRN", "Iran"},
    {"RUS", "7", "RUS", "Russia"},
    {"ENI", "353", "IRL", "Ireland"},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"CSY", "42", "CZE", "Czech Republic"},
    {"SKY", "42", "SVK", "Slovakia"},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"BEL", "375", "BLR", "Belarus"},
    {"UKR", "380", "UKR", "Ukraine"},
    {"", "", "", ""},
    {"", "", "", ""},
    {"SRL", "381", "SPB", "Serbia"},
    {"SLV", "386", "SVN", "Slovenia"},
    {"", "", "", ""},
    {"HRV", "385", "HRV", "Croatia"},
    {"", "", "", ""},
    {"", "", "", ""},
    {"PTB", "55", "BRA", "Brazil"},
    {"BGR", "359", "BGR", "Bulgaria"},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"ESM", "52", "MEX", "Mexico"},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"FRA", "33", "FRA", "France"},
    {"DEA", "43", "AUT", "Austria"},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"FRB", "32", "BEL", "Belgium"},
    {"", "", "", ""},
    {"", "", "", ""},
    {"NON", "47", "NOR", "Norway"},
    {"AFK", "27", "ZAF", "South Africa"},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {"", "", "", ""},
    {NULL, NULL, NULL, NULL},
};

// 1.14d mac: 0002af55
// LOCALE_SABBREVLANGNAME (3), LOCALE_ICOUNTRY (5), LOCALE_SABBREVCTRYNAME (7) and
// LOCALE_SENGCOUNTRY (0x1002) of the region the LCID names (the user's when it names none);
// the length with the terminator, 0 for an unknown type.
//
// The map holds 8 entries and the loop stops there, but the test after it is against 0x95:
// the original takes the map's entry even when none matched (it reads the one past the end).
int GetLocaleInfo(DWORD dwLocale, DWORD dwLCType, char* szData, size_t nDataSize) {
    const Win32MacLocaleMap* pLocaleMap = g_aLocaleMap;
    int nRegion;
    if (dwLocale != 0 && dwLocale != 0x400 && dwLocale != 0x800) {
        DWORD dwSubLang = (dwLocale >> 10) & 0x3F;
        DWORD nIndex = 0;
        do {
            if (pLocaleMap[nIndex].wPrimaryLang == (dwLocale & 0x3FF) &&
                (dwSubLang == 0 || dwSubLang == pLocaleMap[nIndex].wSubLang)) {
                break;
            }
            nIndex++;
        } while (nIndex < 8);
        if (nIndex != 0x95) {
            nRegion = pLocaleMap[nIndex].dwStringsIndex;
            goto haveRegion;
        }
    }
    nRegion = GetUserDefaultLangID();
haveRegion:
    int nResult = 0;
    const char* szValue;
    if (dwLCType < 0x1002) {
        if (dwLCType == 3) {
            szValue = g_aLocaleStrings[nRegion].szLanguageCode;
        } else if (dwLCType == 5) {
            szValue = g_aLocaleStrings[nRegion].szCountryCode;
        } else {
            if (dwLCType != 7) {
                return 0;
            }
            szValue = g_aLocaleStrings[nRegion].szCountryAbbrev;
        }
    } else {
        if (dwLCType != 0x1002) {
            return 0;
        }
        szValue = g_aLocaleStrings[nRegion].szCountryName;
    }
    if (szValue != NULL) {
        size_t nLength = strlen(szValue);
        if ((int)nDataSize > 0) {
            strncpy(szData, szValue, nDataSize);
            szData[nDataSize - 1] = '\0';
            nLength = strlen(szData);
        }
        nResult = nLength + 1;
    }
    return nResult;
}

// 1.14d mac: 0002b05d
// Win32 LANGID (sublanguage << 10 | primary language) of the Mac language and region;
// US English when either is out of range.
DWORD LOCALE_BuildLangIdFromMac(void) {
    DWORD nMacLanguage = GetLanguage();
    DWORD nMacRegion = GetUserDefaultLangID();
    DWORD wLangId = 0x409;
    if (nMacLanguage < 0x95 && nMacRegion < 0x6B) {
        wLangId = (g_aRegionSubLangId[nMacRegion] << 10) | g_aMacLangPrimaryId[nMacLanguage];
    }
    return (WORD)wLangId;
}

// 1.14d mac: 0002b0a0
// The language word of the 'itlb' resource the system 'itlc' names; 0 when either is missing.
int32_t GetLanguage(void) {
    int32_t nResult = 0;
    Handle hItlc = GetResource('itlc', 0);
    if (hItlc != NULL) {
        HLock(hItlc);
        nResult = 0;
        if (*(short**)hItlc != NULL) {
            Handle hItlb = GetResource('itlb', **(short**)hItlc);
            if (hItlb != NULL) {
                HLock(hItlb);
                nResult = *(short*)(*(char**)hItlb + 0xC);
                HUnlock(hItlb);
                ReleaseResource(hItlb);
            }
        }
        HUnlock(hItlc);
        ReleaseResource(hItlc);
    }
    return nResult;
}

// 1.14d mac: 0002b126
// The region word of the system 'itlc' resource; 0 when it is missing.
int32_t GetUserDefaultLangID(void) {
    int32_t nResult = 0;
    Handle hItlc = GetResource('itlc', 0);
    if (hItlc != NULL) {
        HLock(hItlc);
        nResult = 0;
        if (*hItlc != NULL) {
            nResult = *(short*)(*(char**)hItlc + 0xC);
        }
        HUnlock(hItlc);
        ReleaseResource(hItlc);
    }
    return nResult;
}

// 1.14d mac: 0002b177
DWORD LOCALE_GetUserLocaleId(void) {
    return LOCALE_BuildLangIdFromMac();
}

// 1.14d mac: 0002b180
WORD LOCALE_GetDefaultLangId(void) {
    DWORD nMacLanguage = GetLanguage();
    DWORD nMacRegion = GetUserDefaultLangID();
    DWORD wLangId = 0x409;
    if (nMacLanguage < 0x95 && nMacRegion < 0x6B) {
        wLangId = (g_aRegionSubLangId[nMacRegion] << 10) | g_aMacLangPrimaryId[nMacLanguage];
    }
    return (WORD)wLangId;
}

// 1.14d mac: 0002b1c0
DWORD GetLastError(void) {
    return g_dwLastError;
}

// 1.14d mac: 0002b1d1
int MessageBox(void* hWnd, LPCSTR szText, LPCSTR szCaption, UINT uType) {
    return FPMessageBox((char*)szText, (char*)szCaption, uType);
}

// 1.14d mac: 0002b1f5
int WSOCK_CloseSocket(int nSocket) {
    return close(nSocket);
}

#endif  // D2_OS_MAC
