// Storm's Win32-on-Mac layer: Win32_utils.cpp, named by its __FILE__ literal (0030ece4,
// used by PATHCACHE_Initialize). 1.14d Intel Mac band 0002c670..0002eae0: its functions
// from 0002c670 (the mutex destructor its static objects register) to 0002d3ef, the
// template instantiations it emitted (stpl.h, 0002d24c on, listed below), and its static
// initialiser (0002ea70). One file: FilepathToFSSpec (0002c935)
// inlines the token parsing, and calls PATHCACHE_ResolveFSSpec (0002d3ef) with its first
// two arguments in ecx and edx, clang's fastcc, which it gives only to a static function
// of the same file; the statics of all three parts are contiguous in __bss (003f22e0 token
// table .. 003f24e4 kernel list .. 003f24ec path cache).
//
// Hex formatting and the Mac path tokens: a path the layer hands out starts with
// "\{MacPathNN}\": NN (two hex digits) names an entry of the token table, a volume and
// directory; the rest is a name in it.
//
// The kernel-object list: events, modules and threads (AddObjectToKernelPool's callers)
// are also kept on a list of handles, in a Memory Manager handle, so they can be looked
// up by what they wrap: a pthread, a process serial number, a module name.
#include "Win32Mac.h"
#include "Source/StormMac/H/stpl.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include <Carbon/Carbon.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MACPATH_TOKEN_MAX 32
#define MAX_PATH 260

// 1.14d mac: 0002a1d0
DWORD GetCurrentDirectory(DWORD nBufferLength, char* szBuffer);

// A pthread mutex owned by a static object: the file's static initialiser (0002ea70)
// runs pthread_mutex_init with default attributes and registers the destructor with
// __cxa_atexit.
class Win32MacMutex {
public:
    pthread_mutex_t m_tMutex;

    Win32MacMutex() { pthread_mutex_init(&m_tMutex, NULL); }
    ~Win32MacMutex();
};

// Out of line: the static initialiser takes its address directly, not through a
// non-lazy pointer as it would for an inline (weak) one.
Win32MacMutex::~Win32MacMutex() {
    pthread_mutex_destroy(&m_tMutex);
}

// The template instantiations and compiler-made functions this file's object holds
// (stpl.h), by the qualified name the compare tool pairs them with:
// 1.14d mac: 0002c670 Win32MacMutex::~Win32MacMutex[D1]
// 1.14d mac: 0002dbc6 TSGrowableArray<PathCacheDir>::~TSGrowableArray[D2]
// 1.14d mac: 0002dc40 TSLink<PathCacheDir>::Unlink
// 1.14d mac: 0002dc80 TSLink<PathCacheDir>::NextLink
// 1.14d mac: 0002dcb6 TSGrowableArray<PathCacheDir>::New
// 1.14d mac: 0002dd06 TSHashTable<FileLocation,HASHKEY_STRI>::Ptr
// 1.14d mac: 0002ddc2 TSLink<FileLocation>::Unlink
// 1.14d mac: 0002de02 TSLink<FileLocation>::NextLink
// 1.14d mac: 0002de38 TSHashTable<FileLocation,HASHKEY_STRI>::InternalNewNode
// 1.14d mac: 0002debc HASHKEY_STRI::operator=
// 1.14d mac: 0002df10 TSHashTable<FileLocation,HASHKEY_STRI>::InitializeTableSlots
// 1.14d mac: 0002df7c TSHashTable<FileLocation,HASHKEY_STRI>::MonitorFullness
// 1.14d mac: 0002e050 TSExplicitList<FileLocation,-572662307>::LinkNode
// 1.14d mac: 0002e0d2 TSHashTable<FileLocation,HASHKEY_STRI>::GrowListArray
// 1.14d mac: 0002e296 TSGrowableArray<TSExplicitList<FileLocation,-572662307>>::SetCount
// 1.14d mac: 0002e342 TSExplicitList<FileLocation,-572662307>::UnlinkAll
// 1.14d mac: 0002e38e TSGrowableArray<TSExplicitList<FileLocation,-572662307>>::Reserve
// 1.14d mac: 0002e400 TSGrowableArray<TSExplicitList<FileLocation,-572662307>>::ReallocData
// 1.14d mac: 0002e548 TSHashTable<FileLocation,HASHKEY_STRI>::InternalDelete
// 1.14d mac: 0002e5c6 TSHashTable<FileLocation,HASHKEY_STRI>::InternalNew
// 1.14d mac: 0002e5f2 TSHashTable<FileLocation,HASHKEY_STRI>::~TSHashTable[D1]
// 1.14d mac: 0002e5fc TSHashTable<FileLocation,HASHKEY_STRI>::~TSHashTable[D0]
// 1.14d mac: 0002e616 TSHashTable<FileLocation,HASHKEY_STRI>::Destroy
// 1.14d mac: 0002e664 TSHashTable<FileLocation,HASHKEY_STRI>::InternalClear
// 1.14d mac: 0002e720 TSGrowableArray<TSExplicitList<FileLocation,-572662307>>::~TSGrowableArray[D2]
// 1.14d mac: 0002e7a0 TSHashTable<FileLocation,HASHKEY_STRI>::~TSHashTable[D2]
// 1.14d mac: 0002e818 TSExplicitList<FileLocation,-572662307>::NewNode
// 1.14d mac: 0002e8a4 TSGrowableArray<PathCacheDir>::Reserve
// 1.14d mac: 0002e916 TSGrowableArray<PathCacheDir>::ReallocData
// 1.14d mac: 0002ea70 _GLOBAL__I_a

// 1.14d mac: 003f2288
// Guards the kernel-object list.
Win32MacMutex g_tKernelPoolMutex;
// 1.14d mac: 003f22b4
// Guards the token table.
Win32MacMutex g_tMacPathTokenMutex;
// 1.14d mac: 003f22e0
static Win32MacPathToken s_aMacPathTokens[MACPATH_TOKEN_MAX];
// 1.14d mac: 003f24e0
static int s_nMacPathTokens;

// One byte as two uppercase hex digits.
static inline void HEX_FormatByte(BYTE byValue, char* pOut) {
    BYTE byHigh = byValue >> 4;
    pOut[0] = byValue < 0xA0 ? byHigh | '0' : byHigh + 'A' - 10;
    BYTE byLow = byValue & 0xF;
    pOut[1] = byLow < 10 ? byLow | '0' : byLow + 'A' - 10;
}

// 1.14d mac: 0002c68b
// 4 uppercase hex digits, not terminated.
void HEX_FormatWord(WORD wValue, char* pOut) {
    HEX_FormatByte(wValue >> 8, pOut);
    HEX_FormatByte(wValue & 0xFF, pOut + 2);
}

// 1.14d mac: 0002c6e3
void HEX_FormatDword(DWORD dwValue, char* pOut) {
    HEX_FormatWord(dwValue >> 16, pOut);
    HEX_FormatWord(dwValue & 0xFFFF, pOut + 4);
}

bool PATH_IsSeparator(char cChar);

// 1.14d mac: 0002c71b
// "\{MacPath??}\" with either slash, the digits not checked. The middle compares a
// word at a time (little-endian): "{Ma", "cPat", and 'h' and '}' around the digits.
BOOL PATH_IsMacPathToken(LPCSTR szPath) {
    if (!PATH_IsSeparator(szPath[0]) || !PATH_IsSeparator(szPath[12])) {
        return 0;
    }
    if ((*(const DWORD*)szPath & 0xFFFFFF00) != 0x614D7B00 || *(const DWORD*)(szPath + 4) != 0x74615063) {
        return 0;
    }
    return (*(const DWORD*)(szPath + 8) & 0xFF0000FF) == 0x7D000068;
}

// 1.14d mac: 0002c76c
bool PATH_IsSeparator(char cChar) {
    return cChar == '/' || cChar == '\\';
}

// Uppercase hex digits back to a number. Inlined everywhere: no copy of its own.
static inline int HEX_Parse(const char* pIn, int nDigits) {
    int nValue = 0;
    for (int i = 0; i != nDigits; i++) {
        int nDigit = pIn[i] - '0';
        if (nDigit > 9) {
            nDigit = pIn[i] - 'A' + 10;
        }
        nValue = nValue * 16 + nDigit;
    }
    return nValue;
}

// The first '/' or '\\', or NULL.
static inline char* PATH_FindSeparator(char* pPath) {
    for (; *pPath != '\0'; pPath++) {
        if (PATH_IsSeparator(*pPath)) {
            return pPath;
        }
    }
    return NULL;
}

// 1.14d mac: 0002c785
Win32MacPathToken* FilepathWorkingDirectory(LPCSTR szPath) {
    return &s_aMacPathTokens[HEX_Parse(szPath + 9, 2) - 1];
}

// 1.14d mac: 0002c7c3
void PATH_MakeMacPathToken(char* szOut, const unsigned char* pName, int16_t nVRefNum, int32_t nDirID, int32_t n08,
                           int32_t n0C) {
    char szName[256];
    pthread_mutex_lock(&g_tMacPathTokenMutex.m_tMutex);
    Win32MacPathToken* pToken = s_aMacPathTokens;
    int i;
    for (i = 0; i < s_nMacPathTokens; i++, pToken++) {
        if (pToken->nVRefNum == nVRefNum && pToken->nDirID == nDirID && pToken->n08 == n08 && pToken->n0C == n0C) {
            break;
        }
    }
    if (i == s_nMacPathTokens) {
        s_nMacPathTokens++;
        pToken->nToken = s_nMacPathTokens;
        pToken->nVRefNum = nVRefNum;
        pToken->nDirID = nDirID;
        pToken->n08 = n08;
        pToken->n0C = n0C;
    }
    pthread_mutex_unlock(&g_tMacPathTokenMutex.m_tMutex);
    CopyPascalStringToC(pName, szName);
    strcpy(szOut, "\\{MacPath..}\\");
    HEX_FormatByte(pToken->nToken, szOut + 9);
    strcpy(szOut + 13, szName);
}

// 1.14d mac: 0002c8f7
void FSSpecToFilepath(char* szOut, const FSSpec* pSpec) {
    PATH_MakeMacPathToken(szOut, pSpec->name, pSpec->vRefNum, pSpec->parID, 0, 0);
}

// 1.14d mac: 000346e7
OSErr GetDirectoryID(short nVRefNum, long nDirID, ConstStr255Param szName, long* pnDirID, Boolean* pbIsDirectory);
// 1.14d mac: 000344dc
OSErr MacResolveAliasFile(FSSpec* pSpec, Boolean bResolveChains, Boolean* pbTargetIsFolder, Boolean* pbWasAliased,
                          Boolean bNoUI);
// 1.14d mac: 0001eafe
unsigned char* c2pstr(char* szString);
// 1.14d mac: 0003e21d
int MACFILE_FSSpecToPath(FSSpec* pSpec, char* szPath, int nMaxPathSize);

// The path cache: for each volume and directory a path was resolved from, a hash
// table of the directories under it, three levels deep, by their path relative to
// it ("a\\b\\c"). It is flushed whenever a volume is mounted or unmounted.

// One directory under a cache root, keyed by its relative path.
class FileLocation : public TSHashObject<FileLocation, HASHKEY_STRI> {
public:
    long m_nDirID;      // 0x18
    short m_nVRefNum;   // 0x1C
};

typedef TSHashTable<FileLocation, HASHKEY_STRI> FileLocationTable;

// A cache root: the directory a path was relative to, and what lies under it.
class PathCacheDir : public TSLinkedNode<PathCacheDir> {
public:
    FileLocationTable* m_pTable;  // 0x08
    long m_nDirID;                // 0x0C
    short m_nVRefNum;             // 0x10
};

// 1.14d mac: 003f24ec
static TSGrowableArray<PathCacheDir>* s_pPathCache;
// 1.14d mac: 003224a8
static const EventTypeSpec s_aVolumeEvents[2] = {
    {kEventClassVolume, kEventVolumeMounted},
    {kEventClassVolume, kEventVolumeUnmounted},
};
// 1.14d mac: 003f2500
static EventHandlerRef s_hVolumeEventHandler;

// 1.14d mac: 0002d3ef
static int PATHCACHE_ResolveFSSpec(short nVRefNum, long nDirID, LPCSTR szPath, FSSpec* pSpec);

// 1.14d mac: 0002c935
// "\{MacPathNN}\rest" is relative to token NN; any other path to hBase's module
// directory, or with no hBase to the current directory. Every component is checked
// first (at most 31 characters, no "." or ".."); then each directory is resolved
// through aliases, and a missing final file is not an error.
int16_t FilepathToFSSpec(FSSpec* pSpec, LPCSTR szPath, int hBase) {
    char szWorkPath[256];
    Str63 szPascalName;
    char szCurDir[MAX_PATH];
    long nDirID;
    FSSpec tSpec;
    Boolean bTargetIsFolder;
    Boolean bWasAliased;
    Boolean bIsDirectory;
    short nVRefNum;
    long nParID;
    int nResult;
    char* pName;
    char* pNext;
    OSErr nErr;
    if (szPath != NULL && PATH_IsMacPathToken(szPath)) {
        int nToken = HEX_Parse(szPath + 9, 2);
        szPath += 13;
        nParID = s_aMacPathTokens[nToken - 1].nDirID;
        nVRefNum = s_aMacPathTokens[nToken - 1].nVRefNum;
    } else if (hBase != 0) {
        Win32HandleObject* pModule = GetWin32HandlePtr(hBase);
        int nToken = HEX_Parse(pModule->szModulePath + 9, 2);
        nParID = s_aMacPathTokens[nToken - 1].nDirID;
        nVRefNum = s_aMacPathTokens[nToken - 1].nVRefNum;
        if (szPath == NULL || *szPath == '\0') {
            szPath = pModule->szModulePath + 13;
        }
    } else {
        GetCurrentDirectory(MAX_PATH, szCurDir);
        int nToken = HEX_Parse(szCurDir + 9, 2);
        nParID = s_aMacPathTokens[nToken - 1].nDirID;
        nVRefNum = s_aMacPathTokens[nToken - 1].nVRefNum;
    }
    pSpec->parID = nParID;
    pSpec->vRefNum = nVRefNum;
    pSpec->name[0] = 0;

    const char* pComponent = szPath;
    for (;;) {
        int nLength = 0;
        while (pComponent[nLength] != '\\' && pComponent[nLength] != '/') {
            if (pComponent[nLength] == '\0') {
                if ((nLength == 1 && pComponent[0] == '.') || (nLength == 2 && pComponent[0] == '.' && pComponent[1] == '.')) {
                    pSpec->vRefNum = nVRefNum;
                    pSpec->parID = nParID;
                    pSpec->name[0] = 0;
                    nResult = dirNFErr;
                    goto done;
                }
                goto resolve;
            }
            if (++nLength >= 32) {
                pSpec->vRefNum = nVRefNum;
                pSpec->parID = nParID;
                pSpec->name[0] = 0;
                nResult = fnfErr;
                goto done;
            }
        }
        const char* pNextComponent = pComponent + nLength + 1;
        if ((nLength == 1 && pComponent[0] == '.') || (nLength == 2 && pComponent[0] == '.' && pComponent[1] == '.')) {
            pSpec->vRefNum = nVRefNum;
            pSpec->parID = nParID;
            pSpec->name[0] = 0;
            nResult = dirNFErr;
            goto done;
        }
        pComponent = pNextComponent;
    }

resolve:
    if (s_pPathCache) {
        nResult = PATHCACHE_ResolveFSSpec(nVRefNum, nParID, szPath, pSpec);
        goto done;
    }
    nDirID = nParID;
    strcpy(szWorkPath, szPath);
    pName = szWorkPath;
    while (*pName != '\0') {
        char* pSep = PATH_FindSeparator(pName);
        if (pSep == NULL) {
            break;
        }
        *pSep = '\0';
        pNext = pSep + 1;
        memset(&tSpec, 0, sizeof(tSpec));
        tSpec.vRefNum = nVRefNum;
        tSpec.parID = nDirID;
        tSpec.name[0] = 0;
        strcpy((char*)tSpec.name, pName);
        c2pstr((char*)tSpec.name);
        nErr = MacResolveAliasFile(&tSpec, true, &bTargetIsFolder, &bWasAliased, false);
        nResult = dirNFErr;
        if (nErr == fnfErr) {
            if (*pNext != '\0') {
                goto done;
            }
            break;
        }
        if (nErr != noErr || !bTargetIsFolder) {
            goto done;
        }
        nVRefNum = tSpec.vRefNum;
        GetDirectoryID(tSpec.vRefNum, tSpec.parID, tSpec.name, &nDirID, &bIsDirectory);
        pName = pNext;
    }
    CopyCStringToPascal(pName, szPascalName);
    if (szPascalName[0] == 0) {
        pSpec->parID = nDirID;
        pSpec->vRefNum = nVRefNum;
        pSpec->name[0] = 0;
    } else {
        nErr = FSMakeFSSpec(nVRefNum, nDirID, szPascalName, pSpec);
        if (nErr != noErr && nErr != fnfErr) {
            nResult = nErr;
            goto done;
        }
    }
    nResult = noErr;
done:
    return nResult;
}

// 1.14d mac: 0002cd41
// The first component resolved to an FSSpec and made a POSIX path, the rest
// appended with '\' turned into '/'. Returns an OSErr, or 100055 when the
// out buffer is full.
int PATH_ToPosixPath(LPCSTR szPath, char* szOut, int nOutSize) {
    char szWorkPath[MAX_PATH];
    FSSpec tSpec;
    strlcpy(szWorkPath, szPath, MAX_PATH);
    char* pSep = NULL;
    for (char* p = &szWorkPath[1]; *p != '\0'; p++) {
        if (PATH_IsSeparator(*p)) {
            pSep = p;
            break;
        }
    }
    char cRest = pSep[1];
    pSep[1] = '\0';
    OSErr nErr = FilepathToFSSpec(&tSpec, szWorkPath, 0);
    if (nErr != noErr) {
        return nErr;
    }
    int nResult = MACFILE_FSSpecToPath(&tSpec, szOut, nOutSize);
    if (nResult != 0 || cRest == '\0') {
        return nResult;
    }
    pSep[1] = cRest;
    int nLength = strlen(szOut);
    if (nLength == nOutSize - 1) {
        return 100055;
    }
    szOut[nLength] = '/';
    char* pOut = &szOut[nLength + 1];
    char cChar = pSep[1];
    if (cChar != '\0' && nLength + 1 < nOutSize - 1) {
        const char* pIn = pSep + 2;
        do {
            *pOut++ = cChar == '\\' ? '/' : cChar;
            cChar = *pIn++;
        } while (cChar != '\0');
    }
    *pOut = '\0';
    return 0;
}

#define FORMAT_MESSAGE_FROM_SYSTEM 0x00001000

// 1.14d mac: 0001e45c
Handle MemAllocHandle(Size nSize);
LONG InterlockedIncrement(LPLONG pnValue);

// 1.14d mac: 003f24e4
static Handle s_hKernelObjects;
// 1.14d mac: 003f24e8
static int16_t s_nKernelObjects;
// 1.14d mac: 003f24ea
static int16_t s_nKernelObjectsMax;

// 1.14d mac: 0002ce67
// The list starts at 32 handles and grows by 16.
BOOL AddObjectToKernelPool(HANDLE hObject) {
    BOOL bResult;
    pthread_mutex_lock(&g_tKernelPoolMutex.m_tMutex);
    if (s_hKernelObjects == NULL) {
        s_nKernelObjects = 0;
        s_nKernelObjectsMax = 32;
        s_hKernelObjects = MemAllocHandle(32 * sizeof(HANDLE));
    }
    if (s_nKernelObjects == s_nKernelObjectsMax) {
        s_nKernelObjectsMax += 16;
        SetHandleSize(s_hKernelObjects, s_nKernelObjectsMax * sizeof(HANDLE));
        bResult = 0;
        if (MemError() != noErr) {
            goto done;
        }
    }
    ((HANDLE*)*s_hKernelObjects)[s_nKernelObjects++] = hObject;
    bResult = 1;
done:
    pthread_mutex_unlock(&g_tKernelPoolMutex.m_tMutex);
    return bResult;
}

// 1.14d mac: 0002cf23
BOOL HANDLE_RemoveFromList(HANDLE hObject) {
    pthread_mutex_lock(&g_tKernelPoolMutex.m_tMutex);
    // nLeft: the bytes after pEntry, which close up over a removed entry
    HANDLE* pEntry = (HANDLE*)*s_hKernelObjects;
    int nLeft = s_nKernelObjects * sizeof(HANDLE);
    BOOL bResult = 0;
    while (nLeft != 0) {
        nLeft -= sizeof(HANDLE);
        if (*pEntry++ == hObject) {
            if (nLeft > 0) {
                memmove(pEntry - 1, pEntry, nLeft);
            }
            s_nKernelObjects--;
            bResult = 1;
            break;
        }
    }
    pthread_mutex_unlock(&g_tKernelPoolMutex.m_tMutex);
    return bResult;
}

// 1.14d mac: 0002cfbd
// The thread's handle, with a reference added; 0 if it is not one of ours.
HANDLE HANDLE_FindThreadByPthread(pthread_t pThread) {
    pthread_mutex_lock(&g_tKernelPoolMutex.m_tMutex);
    HANDLE hResult = 0;
    int nCount = s_nKernelObjects;
    if (nCount > 0) {
        HANDLE* pHandles = (HANDLE*)*s_hKernelObjects;
        for (int i = 0; i < nCount; i++) {
            hResult = pHandles[i];
            Win32HandleObject* pObject = GetWin32HandlePtr(hResult);
            if (pObject->dwType == WIN32MAC_HANDLE_THREAD && pObject->dwThread == (DWORD)pThread) {
                InterlockedIncrement(&pObject->nRefCount);
                break;
            }
            hResult = 0;
        }
    }
    pthread_mutex_unlock(&g_tKernelPoolMutex.m_tMutex);
    return hResult;
}

// 1.14d mac: 0002d039
HANDLE HANDLE_FindModuleByPSN(ProcessSerialNumber* pPSN) {
    pthread_mutex_lock(&g_tKernelPoolMutex.m_tMutex);
    int nCount = s_nKernelObjects;
    if (nCount > 0) {
        HANDLE* pHandles = (HANDLE*)*s_hKernelObjects;
        for (int i = 0; i < nCount; i++) {
            HANDLE hObject = pHandles[i];
            Win32HandleObject* pObject = GetWin32HandlePtr(hObject);
            if (pObject->dwType == WIN32MAC_HANDLE_MODULE && pObject->dwPSNHigh == pPSN->highLongOfPSN &&
                pObject->dwPSNLow == pPSN->lowLongOfPSN) {
                InterlockedIncrement(&pObject->nRefCount);
                pthread_mutex_unlock(&g_tKernelPoolMutex.m_tMutex);
                return hObject;
            }
        }
    }
    pthread_mutex_unlock(&g_tKernelPoolMutex.m_tMutex);
    return 0;
}

// 1.14d mac: 0002d0d2
HANDLE HANDLE_FindModuleByName(LPCSTR szModuleName) {
    pthread_mutex_lock(&g_tKernelPoolMutex.m_tMutex);
    int nCount = s_nKernelObjects;
    if (nCount > 0) {
        HANDLE* pHandles = (HANDLE*)*s_hKernelObjects;
        for (int i = 0; i < nCount; i++) {
            HANDLE hObject = pHandles[i];
            Win32HandleObject* pObject = GetWin32HandlePtr(hObject);
            if (pObject->dwType == WIN32MAC_HANDLE_MODULE && strcmp(pObject->szModulePath, szModuleName) == 0) {
                InterlockedIncrement(&pObject->nRefCount);
                pthread_mutex_unlock(&g_tKernelPoolMutex.m_tMutex);
                return hObject;
            }
        }
    }
    pthread_mutex_unlock(&g_tKernelPoolMutex.m_tMutex);
    return 0;
}

// 1.14d mac: 0002d16d
// Always: the game takes the debugger's paths on the Mac.
BOOL IsDebuggerPresent(void) {
    return 1;
}

// 1.14d mac: 0002d178
void OutputDebugString(LPCSTR szOutputString) {
}

// 1.14d mac: 0002d17e
// No message table: the text is the number, "Error: 5" or "Application Error: 3:17".
DWORD FormatMessage(DWORD dwFlags, LPCVOID pSource, DWORD dwMessageId, DWORD dwLanguageId, char* szBuffer) {
    const char* szPrefix = "Error:";
    if (!(dwFlags & FORMAT_MESSAGE_FROM_SYSTEM)) {
        szPrefix = "Application Error:";
    }
    if (dwMessageId >> 16) {
        sprintf(szBuffer, "%s %d:%d", szPrefix, dwMessageId >> 16, dwMessageId & 0xFFFF);
    } else {
        sprintf(szBuffer, "%s %d", szPrefix, dwMessageId & 0xFFFF);
    }
    return strlen(szBuffer);
}

// 1.14d mac: 0002d1ec
// Every sound is the system beep. The compares on uType survive in the asm with no
// code behind them: empty or identical cases.
BOOL MessageBeep(UINT uType) {
    switch (uType) {
    case 0:  // MB_OK
        SysBeep(15);
        break;
    case 0x10:  // MB_ICONHAND
        SysBeep(15);
        break;
    default:
        SysBeep(15);
        break;
    }
    return 1;
}

// 1.14d mac: 0002d212
DWORD CharLowerBuff(char* lpsz, DWORD cchLength) {
    DWORD i = 0;
    if (lpsz != NULL && cchLength != 0) {
        for (; i != cchLength; i++) {
            lpsz[i] = tolower(lpsz[i]);
        }
    }
    return i;
}

void FlushPartialPathCache(void);

// 1.14d mac: 0002d31b
// Volume mounted or unmounted: what the cache knows may be gone.
static OSStatus PATHCACHE_FlushEventHandler(EventHandlerCallRef hCallRef, EventRef hEvent, void* pUserData) {
    FlushPartialPathCache();
    return eventNotHandledErr;
}

// 1.14d mac: 0002d24c
void PATHCACHE_Initialize(void) {
    // the original's line
    s_pPathCache = new (SMemAlloc(sizeof(TSGrowableArray<PathCacheDir>), __FILE__, 1100, 0))
        TSGrowableArray<PathCacheDir>;
    // 1.14d mac: 003f24f0 PATHCACHE_Initialize()::s_pfnFlushHandler
    // 1.14d mac: 003f24f8 guardvariableforPATHCACHE_Initialize()::s_pfnFlushHandler
    static EventHandlerUPP s_pfnFlushHandler = NewEventHandlerUPP(PATHCACHE_FlushEventHandler);
    InstallApplicationEventHandler(s_pfnFlushHandler, 2, s_aVolumeEvents, NULL, &s_hVolumeEventHandler);
}

// 1.14d mac: 0002d330
void PATHCACHE_Destroy(void) {
    FlushPartialPathCache();
    delete s_pPathCache;
    s_pPathCache = NULL;
    RemoveEventHandler(s_hVolumeEventHandler);
    s_hVolumeEventHandler = NULL;
}

// 1.14d mac: 0002d386
// Drops every cache root and its table.
void FlushPartialPathCache(void) {
    int nCount = s_pPathCache->Count();
    for (int i = 0; i < nCount; i++) {
        delete (*s_pPathCache)[i].m_pTable;
    }
    s_pPathCache->Clear();
}

static void PATHCACHE_ScanDirectory(short nVRefNum, long nDirID, const char* szPrefix, FileLocationTable* pTable,
                                    int nDepth);

// 1.14d mac: 0002d3ef
// The path, relative to the directory nVRefNum:nDirID, as an FSSpec: the result of
// FSMakeFSSpec for it. Directories come from the cache root's table, built on first
// use; a path deeper than three directories resolves its first three and recurses.
static int PATHCACHE_ResolveFSSpec(short nVRefNum, long nDirID, LPCSTR szPath, FSSpec* pSpec) {
    char szPathCopy[256];
    char szHead[256];
    char szTail[256];
    FSSpec tSpec;
    if (*szPath == '/' || *szPath == '\\') {
        szPath++;
    }
    strcpy(szPathCopy, szPath);
    char* pLastSep = NULL;
    for (char* p = szPathCopy; *p != '\0'; p++) {
        if (*p == '/' || *p == '\\') {
            pLastSep = p;
        }
    }
    if (pLastSep == NULL) {
        if (pSpec != NULL) {
            pSpec->vRefNum = nVRefNum;
            pSpec->parID = nDirID;
            CopyCStringToPascal(szPathCopy, pSpec->name);
        }
        return FSMakeFSSpec(pSpec->vRefNum, pSpec->parID, pSpec->name, &tSpec);
    }
    for (char* p = szPathCopy; *p != '\0'; p++) {
        if (*p == '/' || *p == '\\') {
            *p = '\\';
        }
    }

    FileLocationTable* pTable;
    int i;
    for (i = 0; i < (int)s_pPathCache->Count(); i++) {
        PathCacheDir* pDir = &(*s_pPathCache)[i];
        if (pDir->m_nVRefNum == nVRefNum && pDir->m_nDirID == nDirID) {
            pTable = pDir->m_pTable;
            goto found;
        }
    }
    {
        PathCacheDir* pDir = s_pPathCache->New();
        pDir->m_nVRefNum = nVRefNum;
        pDir->m_nDirID = nDirID;
        // the original's line
        pTable = new (SMemAlloc(sizeof(FileLocationTable), __FILE__, 1498, 0)) FileLocationTable;
        pDir->m_pTable = pTable;
        PATHCACHE_ScanDirectory(nVRefNum, nDirID, "", pTable, 0);
    }
found:
    int nSeps = 0;
    int nLength = strlen(szPathCopy);
    for (int k = 0; k < nLength - 1; k++) {
        if (szPathCopy[k] == '/' || szPathCopy[k] == '\\') {
            nSeps++;
        }
    }
    if (nSeps < 4) {
        *pLastSep = '\0';
        FileLocation* pLocation = pTable->Ptr(szPathCopy);
        if (pLocation != NULL) {
            if (pSpec != NULL) {
                pSpec->vRefNum = pLocation->m_nVRefNum;
                pSpec->parID = pLocation->m_nDirID;
                CopyCStringToPascal(pLastSep + 1, pSpec->name);
            }
            return FSMakeFSSpec(pSpec->vRefNum, pSpec->parID, pSpec->name, &tSpec);
        }
        if (pLastSep[1] != '\0') {
            return dirNFErr;
        }
        return PATHCACHE_ResolveFSSpec(nVRefNum, nDirID, szPathCopy, pSpec);
    }

    // the first three directories, then the rest
    int nHeadSeps = 0;
    char* pIn = szPathCopy;
    char* pOut = szHead;
    for (;;) {
        char c = *pIn++;
        *pOut = c;
        if (c == '/' || c == '\\') {
            nHeadSeps++;
        }
        if (nHeadSeps > 2) {
            break;
        }
        pOut++;
    }
    *pOut = '\0';
    strcpy(szTail, pIn);
    FileLocation* pHead = pTable->Ptr(szHead);
    if (pHead == NULL) {
        return dirNFErr;
    }
    return PATHCACHE_ResolveFSSpec(pHead->m_nVRefNum, pHead->m_nDirID, szTail, pSpec);
}

// 1.14d mac: 0002d7c7
// Adds the directories under nVRefNum:nDirID to the table, szPrefix before each
// name, down to three levels below the root. An alias to a folder on the same volume
// counts as that folder.
static void PATHCACHE_ScanDirectory(short nVRefNum, long nDirID, const char* szPrefix, FileLocationTable* pTable,
                                    int nDepth) {
    Str63 szItemName;
    char szName[64];
    char szPath[256];
    FSSpec tAliasSpec;
    Boolean bTargetIsFolder;
    Boolean bWasAliased;
    CInfoPBRec tAliasInfo;
    CInfoPBRec tInfo;
    short nTargetVRefNum;
    long nTargetDirID;
    if (nDepth >= 4) {
        return;
    }
    int nPrefixLength = strlen(szPrefix);
    tInfo.hFileInfo.ioCompletion = NULL;
    tInfo.hFileInfo.ioNamePtr = szItemName;
    tInfo.hFileInfo.ioVRefNum = nVRefNum;
    tInfo.hFileInfo.ioFVersNum = 0;
    tInfo.hFileInfo.ioFDirIndex = 1;
    tInfo.hFileInfo.ioDirID = nDirID;
    OSErr nErr = PBGetCatInfoSync(&tInfo);
    short nIndex = 2;
    while (nErr == noErr) {
        if (szItemName[0] + nPrefixLength < 255) {
            if (tInfo.hFileInfo.ioFlAttrib & kioFlAttribDirMask) {
                CopyPascalStringToC(szItemName, szName);
                strcpy(szPath, szPrefix);
                strcat(szPath, szName);
                FileLocation* pLocation = pTable->New(szPath, 0, 0);
                pLocation->m_nVRefNum = nVRefNum;
                pLocation->m_nDirID = tInfo.dirInfo.ioDrDirID;
                strcat(szPath, "\\");
                PATHCACHE_ScanDirectory(nVRefNum, tInfo.dirInfo.ioDrDirID, szPath, pTable, nDepth + 1);
            } else if (tInfo.hFileInfo.ioFlFndrInfo.fdFlags & kIsAlias) {
                bool bSkip = true;
                if (FSMakeFSSpec(nVRefNum, nDirID, szItemName, &tAliasSpec) == noErr &&
                    MacResolveAliasFile(&tAliasSpec, true, &bTargetIsFolder, &bWasAliased, false) == noErr &&
                    bTargetIsFolder) {
                    tAliasInfo.hFileInfo.ioCompletion = NULL;
                    tAliasInfo.hFileInfo.ioNamePtr = tAliasSpec.name;
                    tAliasInfo.hFileInfo.ioVRefNum = tAliasSpec.vRefNum;
                    tAliasInfo.hFileInfo.ioFVersNum = 0;
                    tAliasInfo.hFileInfo.ioFDirIndex = 0;
                    tAliasInfo.hFileInfo.ioDirID = tAliasSpec.parID;
                    if (PBGetCatInfoSync(&tAliasInfo) == noErr && (tAliasInfo.hFileInfo.ioFlAttrib & kioFlAttribDirMask)) {
                        nTargetVRefNum = tAliasInfo.hFileInfo.ioVRefNum;
                        nTargetDirID = tAliasInfo.dirInfo.ioDrDirID;
                        bSkip = false;
                    }
                }
                if (!bSkip) {
                    CopyPascalStringToC(szItemName, szName);
                    strcpy(szPath, szPrefix);
                    strcat(szPath, szName);
                    if (nTargetVRefNum == nVRefNum) {
                        FileLocation* pLocation = pTable->New(szPath, 0, 0);
                        pLocation->m_nVRefNum = nVRefNum;
                        pLocation->m_nDirID = nTargetDirID;
                        strcat(szPath, "\\");
                        PATHCACHE_ScanDirectory(nVRefNum, nTargetDirID, szPath, pTable, nDepth + 1);
                    }
                }
            }
        }
        tInfo.hFileInfo.ioCompletion = NULL;
        tInfo.hFileInfo.ioNamePtr = szItemName;
        tInfo.hFileInfo.ioVRefNum = nVRefNum;
        tInfo.hFileInfo.ioFVersNum = 0;
        tInfo.hFileInfo.ioFDirIndex = nIndex++;
        tInfo.hFileInfo.ioDirID = nDirID;
        nErr = PBGetCatInfoSync(&tInfo);
    }
}

#endif  // D2_OS_MAC
