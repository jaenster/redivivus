// Storm's Win32-on-Mac layer: the private allocator, the clipboard, modules
// (LoadLibrary on dlopen or a 'WRes' resource, GetProcAddress) and the last-error
// code, 1.14d Intel Mac band 00029968..0002a1d0. See Win32Mac.h for what is and is
// not known about the original file boundaries.
//
// ddraw.dll, dsound.dll, advapi32.dll and wsock32.dll are not loaded: LoadLibrary
// hands out the fixed pseudo-handles 0xC0000002..0xC0000005 for them, and
// GetProcAddress answers from the layer's own entry points. Every other module is
// a type-4 kernel object.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <dlfcn.h>
#include <netdb.h>
#include <string.h>
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
// 1.14d mac: 0002a41f
void ExitProcess(UINT uExitCode);
// 1.14d mac: 00033a24
int _stricmp(const char* szA, const char* szB);
// 1.14d mac: 000344dc
OSErr MacResolveAliasFile(FSSpec* pSpec, Boolean bResolveChains, Boolean* pbTargetIsFolder, Boolean* pbWasAliased,
                          Boolean bNoUI);
// 1.14d mac: 0002c8f7
void FSSpecToFilepath(char* szPath, const FSSpec* pSpec);
// 1.14d mac: 0002cd41
int PATH_ToPosixPath(const char* szPath, char* szPosixPath, size_t nSize);
// 1.14d mac: 00025a77
HANDLE AllocModuleHandle(void);
// 1.14d mac: 00036fd3
void Accel_DeleteAcceleratorList(void* pList);
// 1.14d mac: 0002d039
HANDLE HANDLE_FindModuleByPSN(ProcessSerialNumber* pPSN);
// 1.14d mac: 0002d0d2
HANDLE HANDLE_FindModuleByName(LPCSTR szModuleName);
// 1.14d mac: 0002b1f5
int WSOCK_CloseSocket(int nSocket);
// 1.14d mac: 00032b50
int DirectDrawCreate(void* pGuid, void** ppDirectDraw, void* pUnknownOuter);
// 1.14d mac: 00032b83
int DirectSoundCreate(void* pGuid, void** ppDirectSound, void* pUnknownOuter);

FARPROC GetProcAddress(HANDLE hModule, LPCSTR szProcName);
uint16_t WSOCK_Htons(uint16_t wValue);
int WSOCK_WSAStartup(uint16_t wVersionRequested, void* pWsaData);
int WSOCK_WSACleanup(void);

// 1.14d mac: 003f227c
// GetLastError's value.
DWORD g_dwLastError;

// 1.14d mac: 00029968
// Allocates or ends the process: an allocation the layer cannot do without.
void* Priv_Alloc(size_t nSize) {
    void* pMem = MemAlloc(nSize, 0);
    if (pMem == NULL) {
        MessageBox(NULL, "Out of Memory", "", 0x30);
        ExitProcess(0xFFFFFFFF);
    }
    return pMem;
}

// 1.14d mac: 000299cc
void* Priv_AllocClear(size_t nSize) {
    void* pMem = MemAlloc(nSize, 0x100);
    if (pMem == NULL) {
        MessageBox(NULL, "Out of Memory", "", 0x30);
        ExitProcess(0xFFFFFFFF);
    }
    return pMem;
}

// 1.14d mac: 00029a30
void Priv_Free(void* pMem) {
    MemFree(pMem, 0);
}

// 1.14d mac: 00029a4e
// Replaces the scrap with the text as flavor 'TEXT'.
BOOL CLIPBOARD_SetText(const char* szText, int nLength) {
    bool bResult = false;
    if (nLength > 0 && szText != NULL && *szText != '\0') {
        ClearCurrentScrap();
        ScrapRef hScrap;
        GetCurrentScrap(&hScrap);
        bResult = PutScrapFlavor(hScrap, 'TEXT', 0, nLength, szText) == noErr;
    }
    return bResult;
}

// 1.14d mac: 00029aad
// The 'TEXT' scrap's size plus its terminator, 0 when there is none.
int CLIPBOARD_GetTextSize(void) {
    ScrapRef hScrap;
    Size nSize = 0;
    GetCurrentScrap(&hScrap);
    GetScrapFlavorSize(hScrap, 'TEXT', &nSize);
    return nSize > 0 ? nSize + 1 : 0;
}

// 1.14d mac: 00029af1
// Copies the 'TEXT' scrap out with a terminator: its size plus one, 0 when there is
// none, -1 when the buffer is too small.
int CLIPBOARD_GetText(char* pBuffer, int nBufferSize) {
    ScrapRef hScrap;
    Size nSize = 0;
    GetCurrentScrap(&hScrap);
    GetScrapFlavorSize(hScrap, 'TEXT', &nSize);
    int nResult = 0;
    if (nSize != 0) {
        nResult = -1;
        if (nSize < nBufferSize) {
            GetScrapFlavorData(hScrap, 'TEXT', &nSize, pBuffer);
            pBuffer[nSize] = '\0';
            nResult = nSize + 1;
        }
    }
    return nResult;
}

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
        Accel_DeleteAcceleratorList(pModule->pAccelList);
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

#endif  // D2_OS_MAC
