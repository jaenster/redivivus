// StormMac Storm Mac/SOURCE/SCMD.CPP: the command-line switch registry and parser,
// 1.14d Intel Mac 0000d43e..0000e5cf.
//
// Two lists of switch definitions (CMDDEF, 0x40 bytes) are kept: the ones that take a
// leading '-' or '/' (0003f0254) and the bare ones (0003f0260). SCmdProcess walks a
// command line over them. The intrusive list helpers after the parser are the TSList
// instantiations this file's object holds; the file's static initialiser closes the run.
//
// __FILE__ is ".../StormMac/Storm Mac/SOURCE/SCMD.CPP".
#include "../Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void* SMemAlloc(uint32_t nBytes, const char* szFile, int nLine, uint32_t dwFlags);
int SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags);
void SErrSetLastError(DWORD dwErrCode);
uint32_t SStrLen(const char* szString);
char* SStrChr(const char* szString, char cChar);
uint32_t SStrCopy(char* szDest, const char* szSource, uint32_t nMaxChars);
uint32_t SStrPack(char* szDest, const char* szSource, uint32_t nMaxChars);
BOOL SStrTokenize(char** ppszCursor, char* szToken, uint32_t nTokenSize, const char* szDelimiters, int* pbQuoted);
int _strnicmp(const char* szA, const char* szB, size_t nCount);
HANDLE CreateFile(const char* szName, DWORD dwAccess, DWORD dwShare, void* pSecurity, DWORD dwDisposition, DWORD dwFlags,
                  HANDLE hTemplate);
DWORD GetFileSize(HANDLE hFile, DWORD* pdwHigh);
BOOL ReadFile(HANDLE hFile, void* pBuffer, DWORD nBytes, DWORD* pnRead, void* pOverlapped);

#define SCMD_FILE "/Users/bclemetson/dev/diablo2/Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/SOURCE/SCMD.CPP"

// 1.14d mac: gaSCMD_FallbackMessages
// The messages for the errors 0x85100065, 0x8510006d and 0x6e, in that order.
extern const char* gaSCMD_FallbackMessages[3];

struct CMDDEF;
typedef int (*CMDCALLBACK)(void* pParams, const char* szText);
typedef int (*CMDUNKNOWNCALLBACK)(const char* szArg);
typedef int (*CMDERRORCALLBACK)(void* pErrorInfo);

// The CMDPARAMS a switch callback is given.
struct CMDPARAMS {
    uint32_t dwFlags;
    int32_t nGroupId;
    uintptr_t* pValue;
    void* pVariable;
    uint32_t dwSetValue;
    uint32_t dwSetMask;
    uintptr_t uValue;
};

// A switch definition, one TSList node: the list link first.
struct CMDDEF {
    CMDDEF* pPrevLink;     // 0x00
    CMDDEF* pNextElement;  // 0x04: next element; at the end the terminator link | 1
    uint32_t dwFlags;      // 0x08: 0x30000 value type (0 flag, 0x10000 number, 0x20000 string), 0x3000000 kind
    int32_t nGroupId;      // 0x0C
    char szName[16];       // 0x10
    int32_t nNameLen;      // 0x20
    uint32_t dwSetValue;   // 0x24
    uint32_t dwSetMask;    // 0x28
    void* pVariable;       // 0x2C
    int32_t nVariableSize; // 0x30
    CMDCALLBACK pfnCallback;  // 0x34
    int32_t bSupplied;     // 0x38
    uintptr_t uValue;      // 0x3C
};

// The cursor of a parse: the definition a switch matched, its text and length.
struct SCMD_PROCESSING {
    CMDDEF* pCurrent;  // 0x00
    char szName[16];   // 0x04
    int32_t nNameLen;  // 0x14
};

// The list header, 0x0C bytes: the link offset, then the list's own link.
struct CMDLIST {
    int32_t nLinkOffset;  // 0x00
    CMDDEF* pLastLink;    // 0x04
    CMDDEF* pFirst;       // 0x08: first element; empty: the terminator link | 1
};

// 1.14d mac: 003f0254
static CMDLIST s_tSwitchList;
// 1.14d mac: 003f0260
static CMDLIST s_tBareList;
// 1.14d mac: 003f026c
// A required switch (kind 0x1000000) has been registered.
static int32_t s_bRequiredRegistered;

// The next element of a list, NULL at the terminator.
static inline CMDDEF* ListElement(CMDDEF* pNext) {
    if (pNext == NULL || ((uintptr_t)pNext & 1) != 0) {
        return NULL;
    }
    return pNext;
}

void TSLIST_DeleteAll(CMDLIST* pList);
static void TSLIST_UnlinkAll(CMDLIST* pList);
static void TSLINK_Unlink(void* pLink);
static void* TSLINK_NextLink(void* pLink, int nLinkOffset);
static CMDDEF* TSList_NewNode(CMDLIST* pList, int nLinkType, int nExtraBytes, uint32_t dwAllocFlags);
static void TSList_LinkNode(CMDLIST* pList, CMDDEF* pNode, int nLinkType, void* pExistingLink);
static void SCMD_ReportError(const char* szItem, DWORD dwError, CMDERRORCALLBACK pfnError, const char* szText);
static CMDDEF* SCMD_FindFlagDef(const char* szToken, CMDDEF* pFirstDef, int32_t nMinLength);
static int PerformConversion(CMDDEF* pDef, const char* szValue, uint32_t* pnConsumed);
static BOOL SCMD_ProcessCurrentFlag(const char* szArgLine, SCMD_PROCESSING* pProcessing, int* pnConsumed);
static BOOL SCMD_ProcessTokens(SCMD_PROCESSING* pProcessing, char** ppszCursor, CMDDEF** ppPositional,
                               CMDUNKNOWNCALLBACK pfnExtra, CMDERRORCALLBACK pfnError);

// 1.14d mac: 0000d43e
// Frees every string value, then both lists.
BOOL SCmdDestroy(void) {
    for (int nList = 0; nList != 2; nList++) {
        CMDLIST* pList = nList != 0 ? &s_tBareList : &s_tSwitchList;
        CMDDEF* pFlag = ListElement(pList->pFirst);
        while (pFlag != NULL) {
            if ((pFlag->dwFlags & 0x30000) == 0x20000 && pFlag->uValue != 0) {
                SMemFree((void*)pFlag->uValue, SCMD_FILE, 0x21a, 0);
                pFlag->uValue = 0;
            }
            pFlag = ListElement(pFlag->pNextElement);
        }
    }
    TSLIST_DeleteAll(&s_tSwitchList);
    TSLIST_DeleteAll(&s_tBareList);
    s_bRequiredRegistered = 0;
    return 1;
}

// 1.14d mac: 0000d532
// Runs a command line over the registered switches; the positional arguments are the
// first list's, in order, and a required one left over is an error.
int SCmdProcess(char* szCmdLine, int bSkipProgName, CMDUNKNOWNCALLBACK pfnExtra, CMDERRORCALLBACK pfnError) {
    char* pszCursor = szCmdLine;
    if (szCmdLine == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    if (bSkipProgName != 0) {
        SStrTokenize(&pszCursor, NULL, 0, " ,;\"\t\n\r\x1a", NULL);
    }
    SCMD_PROCESSING tProcessing;
    memset(&tProcessing, 0, sizeof(tProcessing));
    CMDDEF* pPositional = ListElement(s_tSwitchList.pFirst);
    int bResult = 0;
    if (SCMD_ProcessTokens(&tProcessing, &pszCursor, &pPositional, pfnExtra, pfnError)) {
        bResult = 1;
        while (pPositional != NULL) {
            if ((pPositional->dwFlags & 0x3000000) == 0x2000000) {
                bResult = 0;
                if (pfnError != NULL) {
                    SCMD_ReportError("", 0x8510006d, pfnError, "");
                }
                return bResult;
            }
            pPositional = ListElement(pPositional->pNextElement);
        }
        bResult = 1;
    }
    return bResult;
}

// 1.14d mac: 0000d674
// One token at a time: "@file" is read and parsed recursively, "-name" and "/name" name
// switches (letters may be run together), anything else is the value of the switch
// before it or the next positional argument.
static BOOL SCMD_ProcessTokens(SCMD_PROCESSING* pProcessing, char** ppszCursor, CMDDEF** ppPositional,
                               CMDUNKNOWNCALLBACK pfnExtra, CMDERRORCALLBACK pfnError) {
    if (**ppszCursor != '\0') {
        char* pszNext = *ppszCursor;
        do {
            char szToken[0x100];
            char szName[0x100];
            int bQuoted = 0;
            int nConsumed;
            int bOk;
            SStrTokenize(&pszNext, szToken, 0x100, " ,;\"\t\n\r\x1a", &bQuoted);
            if (szToken[0] != '\0') {
                if (szToken[0] == '@' && bQuoted == 0) {
                    HANDLE hFile = CreateFile(szToken + 1, (DWORD)-0x80000000, 1, 0, 3, 0x8000000, 0);
                    if (hFile == 0) {
                        if (pfnError != NULL) {
                            SCMD_ReportError(szToken + 1, 0x6e, pfnError, szToken + 1);
                        }
                        break;
                    }
                    DWORD nBytes = GetFileSize(hFile, NULL);
                    char* pFileData = (char*)SMemAlloc(nBytes + 1, SCMD_FILE, 0x159, 0x10000000);
                    DWORD nBytesRead;
                    ReadFile(hFile, pFileData, nBytes, &nBytesRead, 0);
                    CloseHandle(hFile);
                    pFileData[nBytesRead] = 0;
                    char* pFileCursor = pFileData;
                    bOk = SCMD_ProcessTokens(pProcessing, &pFileCursor, ppPositional, pfnExtra, pfnError);
                    SMemFree(pFileData, SCMD_FILE, 0x16a, 0);
                } else if (SStrChr("-/", szToken[0]) != NULL && bQuoted == 0) {
                    pProcessing->pCurrent = NULL;
                    szName[0] = 0;
                    const char* pszText = szToken + 1;
                    bool bContinue = true;
                    if (*pszText == '\0') {
                        goto next_token;
                    }
                    while (true) {
                        // the longest registered name that the text, with the letters
                        // already matched, still spells
                        uint32_t nTextLen = SStrLen(pszText);
                        uint32_t nTry = SStrLen(szName);
                        if (nTry < 2) {
                            nTry = 1;
                        }
                        CMDDEF* pFound = NULL;
                        while (nTry != 0) {
                            if ((int)((nTextLen - 1) + nTry) < 0x100) {
                                SStrCopy(szName + nTry - 1, pszText, 0x100);
                                pFound = SCMD_FindFlagDef(szName, ListElement(s_tBareList.pFirst), 0);
                                if (pFound != NULL) {
                                    break;
                                }
                            }
                            nTry--;
                        }
                        if (pFound == NULL) {
                            if (pfnError != NULL) {
                                SCMD_ReportError(pszText, 0x85100065, pfnError, pszText);
                            }
                            bContinue = false;
                            break;
                        }
                        int nNameLen = pFound->nNameLen;
                        szName[nNameLen] = '\0';
                        const char* pszValue = pszText;
                        do {
                            pszValue++;
                        } while (SStrChr("=:", pszValue[nNameLen - 1]) != NULL);
                        pszValue = pszValue - 1;
                        pProcessing->pCurrent = pFound;
                        pProcessing->nNameLen = nNameLen;
                        SStrCopy(pProcessing->szName, szName, 0x7fffffff);
                        const char* pszArg = pszValue + nNameLen;
                        if (*pszArg == '\0' && (pFound->dwFlags & 0x30000) != 0) {
                            pFound->bSupplied = 1;
                            break;
                        }
                        int nUsed;
                        if (!SCMD_ProcessCurrentFlag(pszArg, pProcessing, &nUsed)) {
                            return *(*ppszCursor) == '\0';
                        }
                        pszText = pszArg + nUsed;
                        if (*pszText == '\0') {
                            break;
                        }
                    }
                    if (!bContinue) {
                        break;
                    }
                    goto next_token;
                } else if (pProcessing->pCurrent == NULL) {
                    if (*ppPositional != NULL) {
                        uint32_t nUsed;
                        if (!PerformConversion(*ppPositional, szToken, &nUsed)) {
                            break;
                        }
                        *ppPositional = ListElement((*ppPositional)->pNextElement);
                        goto next_token;
                    }
                    if (pfnExtra == NULL) {
                        if (pfnError != NULL) {
                            SCMD_ReportError(szToken, 0x85100065, pfnError, szToken);
                        }
                        break;
                    }
                    bOk = pfnExtra(szToken);
                } else {
                    bOk = SCMD_ProcessCurrentFlag(szToken, pProcessing, &nConsumed);
                }
                if (bOk == 0) {
                    break;
                }
            }
        next_token:
            *ppszCursor = pszNext;
        } while (*pszNext != '\0');
    }
    return **ppszCursor == '\0';
}

// 1.14d mac: 0000db26
// The error callback's text: the registered message with the item put in, or a fallback.
static void SCMD_ReportError(const char* szItem, DWORD dwError, CMDERRORCALLBACK pfnError, const char* szText) {
    int nMessage;
    if (dwError == 0x6e) {
        nMessage = 2;
    } else {
        nMessage = 0;
        if (dwError != 0x85100065) {
            if (dwError != 0x8510006d) {
                return;
            }
            nMessage = 1;
        }
    }
    char szFormat[256];
    char szMessage[256];
    szFormat[0] = '\0';
    SStrCopy(szFormat, gaSCMD_FallbackMessages[nMessage], 0x100);
    if (strstr(szFormat, "%s") == NULL) {
        SStrCopy(szMessage, szFormat, 0x100);
    } else {
        sprintf(szMessage, szFormat, szText);
    }
    if (szMessage[0] != '\0') {
        SStrPack(szMessage, "\n", 0x100);
    }
    SErrSetLastError(dwError);
    struct {
        DWORD dwErrorCode;
        const char* pszItem;
        const char* pszErrorText;
    } tError = {dwError, szText, szMessage};
    pfnError(&tError);
}

// 1.14d mac: 0000dc63
BOOL SCmdRegisterArgList(const uint32_t* pCmdDefArray, uint32_t nCount);

// 1.14d mac: 0000dce8
BOOL SCmdRegisterArgument(uint32_t dwFlags, int nGroupId, const char* szText, void* pVariable, int nVariableSize,
                          int nSetValue, int nSetMask, CMDCALLBACK pfnCallback) {
    const char* szName = "";
    if (szText != NULL) {
        szName = szText;
    }
    uint32_t dwKind = dwFlags & 0x3000000;
    int nNameLen = SStrLen(szName);
    if ((s_bRequiredRegistered & (dwKind == 0x2000000)) != 0 || (nVariableSize != 0 && pVariable == NULL) ||
        0xf < nNameLen || (nNameLen < 1 && dwKind == 0) ||
        ((dwFlags & 0x30000) == 0 && pVariable != NULL && nVariableSize != 4)) {
        SErrSetLastError(0x57);
        return 0;
    }
    CMDLIST* pList = dwKind == 0 ? &s_tBareList : &s_tSwitchList;
    CMDDEF* pDef = TSList_NewNode(pList, 2, 0, 0);
    SStrCopy(pDef->szName, szName, 0x10);
    pDef->nGroupId = nGroupId;
    pDef->nNameLen = nNameLen;
    pDef->dwFlags = dwFlags;
    pDef->pVariable = pVariable;
    pDef->nVariableSize = nVariableSize;
    pDef->dwSetValue = nSetValue;
    pDef->dwSetMask = nSetMask;
    pDef->pfnCallback = pfnCallback;
    int nInitial = 0;
    if ((dwFlags & 0x30001) == 1) {
        nInitial = nSetValue;
    }
    pDef->uValue = nInitial;
    if (dwKind == 0x1000000) {
        s_bRequiredRegistered = 1;
    }
    return 1;
}

// 1.14d mac: 0000dc63
BOOL SCmdRegisterArgList(const uint32_t* pCmdDefArray, uint32_t nCount) {
    if (pCmdDefArray == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    for (uint32_t nIndex = 0; nIndex < nCount; nIndex++) {
        if (!SCmdRegisterArgument(pCmdDefArray[0], pCmdDefArray[1], (const char*)pCmdDefArray[2], NULL, 0, 1, -1,
                                  (CMDCALLBACK)pCmdDefArray[3])) {
            return 0;
        }
        pCmdDefArray += 4;
    }
    return 1;
}

// 1.14d mac: 0000de3a
// Applies the pending switch, and every other definition its name still matches, to a value.
static BOOL SCMD_ProcessCurrentFlag(const char* szArgLine, SCMD_PROCESSING* pProcessing, int* pnConsumed) {
    *pnConsumed = 0;
    CMDDEF* pDef = pProcessing->pCurrent;
    pProcessing->pCurrent = NULL;
    while (pDef != NULL) {
        uint32_t nArgConsumed;
        if (!PerformConversion(pDef, szArgLine, &nArgConsumed)) {
            return 0;
        }
        if (*pnConsumed < (int)nArgConsumed) {
            *pnConsumed = nArgConsumed;
        }
        pDef = SCMD_FindFlagDef(pProcessing->szName, ListElement(pDef->pNextElement), pProcessing->nNameLen);
    }
    return 1;
}

// 1.14d mac: 0000dec7
// Stores a value into a definition: a string (copied), a number, or a flag set or cleared by
// its mask (a leading '+' or '-' on the text chooses). Then the callback, then every other
// definition of the same group and type takes the value too.
static int PerformConversion(CMDDEF* pDef, const char* szValue, uint32_t* pnConsumed) {
    *pnConsumed = 0;
    uint32_t dwFlags = pDef->dwFlags;
    uint32_t dwType = dwFlags & 0x30000;
    if (dwType == 0x20000) {
        *pnConsumed = SStrLen(szValue);
        if (pDef->uValue != 0) {
            SMemFree((void*)pDef->uValue, SCMD_FILE, 0x8f, 0);
        }
        char* szCopy = (char*)SMemAlloc(SStrLen(szValue) + 1, SCMD_FILE, 0x90, 0);
        pDef->uValue = (uintptr_t)szCopy;
        SStrCopy(szCopy, szValue, 0x7fffffff);
        if (pDef->pVariable != NULL) {
            SStrCopy((char*)pDef->pVariable, szValue, pDef->nVariableSize);
        }
    } else if (dwType == 0x10000) {
        char* pszEnd = NULL;
        if ((dwFlags & 1) == 0) {
            pDef->uValue = strtoul(szValue, &pszEnd, 0);
        } else {
            pDef->uValue = strtol(szValue, &pszEnd, 0);
        }
        *pnConsumed = pszEnd == NULL ? SStrLen(szValue) : (uint32_t)(pszEnd - szValue);
        if (pDef->pVariable != NULL) {
            size_t nSize = 4;
            if ((uint32_t)pDef->nVariableSize < 5) {
                nSize = pDef->nVariableSize;
            }
            if (0 < (int)nSize) {
                memmove(pDef->pVariable, &pDef->uValue, nSize);
            }
        }
    } else {
        if (dwType != 0) {
            return 0;
        }
        bool bSet;
        if (*szValue == '+') {
            *pnConsumed = 1;
            bSet = true;
        } else if (*szValue == '-') {
            *pnConsumed = 1;
            bSet = false;
        } else {
            bSet = (dwFlags & 1) == 0;
        }
        pDef->uValue = pDef->uValue & ~pDef->dwSetMask;
        if (bSet) {
            pDef->uValue = pDef->uValue | pDef->dwSetValue;
        }
        uint32_t* pVariable = (uint32_t*)pDef->pVariable;
        if (pVariable != NULL) {
            uint32_t dwNew = ~pDef->dwSetMask & *pVariable;
            *pVariable = dwNew;
            if (bSet) {
                *pVariable = dwNew | pDef->dwSetValue;
            }
        }
    }
    pDef->bSupplied = 1;
    if (pDef->pfnCallback != NULL) {
        CMDPARAMS tParams;
        tParams.dwFlags = pDef->dwFlags;
        tParams.nGroupId = pDef->nGroupId;
        tParams.pValue = &pDef->uValue;
        tParams.pVariable = pDef->pVariable;
        tParams.dwSetValue = pDef->dwSetValue;
        tParams.dwSetMask = pDef->dwSetMask;
        tParams.uValue = pDef->uValue;
        if (pDef->pfnCallback(&tParams, szValue) == 0) {
            return 0;
        }
    }
    for (int nList = 0; nList != 2; nList++) {
        CMDLIST* pList = nList != 0 ? &s_tBareList : &s_tSwitchList;
        CMDDEF* pOther = ListElement(pList->pFirst);
        while (pOther != NULL) {
            if (pOther->nGroupId == pDef->nGroupId && ((pDef->dwFlags ^ pOther->dwFlags) & 0x30000) == 0 &&
                pOther != pDef) {
                pOther->bSupplied = 1;
                if ((pOther->dwFlags & 0x30000) == 0x20000) {
                    if (pOther->uValue != 0) {
                        SMemFree((void*)pOther->uValue, SCMD_FILE, 0x126, 0);
                    }
                    char* szCopy = (char*)SMemAlloc(SStrLen((const char*)pDef->uValue) + 1, SCMD_FILE, 0x127, 0);
                    pOther->uValue = (uintptr_t)szCopy;
                    SStrCopy(szCopy, (const char*)pDef->uValue, 0x7fffffff);
                } else {
                    pOther->uValue = pDef->uValue;
                }
            }
            pOther = ListElement(pOther->pNextElement);
        }
    }
    return 1;
}

// 1.14d mac: 0000e251
// The longest definition (longer than nMinLength) whose name starts the token, from pFirstDef on.
static CMDDEF* SCMD_FindFlagDef(const char* szToken, CMDDEF* pFirstDef, int32_t nMinLength) {
    int nTokenLen = SStrLen(szToken);
    CMDDEF* pFound = NULL;
    if (pFirstDef != NULL) {
        int nBest = nMinLength - 1;
        do {
            int nCmdLen = pFirstDef->nNameLen;
            if (nBest < nCmdLen && nCmdLen <= nTokenLen) {
                int nCmp;
                if ((pFirstDef->dwFlags & 0x100) == 0) {
                    nCmp = _strnicmp(pFirstDef->szName, szToken, nCmdLen);
                } else {
                    nCmp = strncmp(pFirstDef->szName, szToken, nCmdLen);
                }
                if (nCmp == 0) {
                    nBest = pFirstDef->nNameLen;
                    pFound = pFirstDef;
                }
            }
            pFirstDef = ListElement(pFirstDef->pNextElement);
        } while (pFirstDef != NULL);
    }
    return pFound;
}

// The list helpers below work on the raw words: a link is {previous link, next element}.
typedef uintptr_t LINKWORD;

// 1.14d mac: 0000e2ea
// A list's destructor: unlink everything, then the list's own link.
static void SCMD_ListDestructor(void* pList) {
    TSLIST_UnlinkAll((CMDLIST*)pList);
    TSLINK_Unlink((LINKWORD*)pList + 1);
}

// 1.14d mac: 0000e30c
void TSLIST_DeleteAll(CMDLIST* pList) {
    LINKWORD* pLink = (LINKWORD*)ListElement(pList->pFirst);
    while (pLink != NULL) {
        TSLINK_Unlink(pLink);
        TSLINK_Unlink(pLink);
        SMemFree(pLink, "", -2, 0);
        pLink = (LINKWORD*)ListElement(pList->pFirst);
    }
}

// 1.14d mac: 0000e394
// TSList::NewNode: a zeroed node of 0x40 bytes and more, linked when a link type is given.
static CMDDEF* TSList_NewNode(CMDLIST* pList, int nLinkType, int nExtraBytes, uint32_t dwAllocFlags) {
    CMDDEF* pMem = (CMDDEF*)SMemAlloc(nExtraBytes + 0x40, "", -2, dwAllocFlags | 8);
    CMDDEF* pNode = NULL;
    if (pMem != NULL) {
        pMem->pPrevLink = NULL;
        pMem->pNextElement = NULL;
        pNode = pMem;
    }
    if (nLinkType != 0) {
        TSList_LinkNode(pList, pNode, nLinkType, NULL);
    }
    return pNode;
}

// 1.14d mac: 0000e40a
// Links a node after (1) or before (2) a link of the list, by default the list's own.
static void TSList_LinkNode(CMDLIST* pList, CMDDEF* pNode, int nLinkType, void* pExistingLink) {
    LINKWORD* pLink = (LINKWORD*)pNode;
    LINKWORD* pListWords = (LINKWORD*)pList;
    if (pNode == NULL) {
        pLink = pListWords + 1;
    }
    if (*pLink != 0) {
        TSLINK_Unlink(pLink);
    }
    LINKWORD* pTarget = pListWords + 1;
    if (pExistingLink != NULL) {
        pTarget = (LINKWORD*)pExistingLink;
    }
    if (nLinkType == 2) {
        LINKWORD* pPrevious = (LINKWORD*)*pTarget;
        *pLink = (LINKWORD)pPrevious;
        pLink[1] = pPrevious[1];
        pPrevious[1] = (LINKWORD)pNode;
        *pTarget = (LINKWORD)pLink;
    } else if (nLinkType == 1) {
        *pLink = (LINKWORD)pTarget;
        pLink[1] = pTarget[1];
        LINKWORD* pNextLink = (LINKWORD*)TSLINK_NextLink(pTarget, (int)*pListWords);
        *pNextLink = (LINKWORD)pLink;
        pTarget[1] = (LINKWORD)pNode;
    }
}

// 1.14d mac: 0000e47e
static void TSLINK_Unlink(void* pLinkArg) {
    LINKWORD* pLink = (LINKWORD*)pLinkArg;
    LINKWORD nPrevious = *pLink;
    if (nPrevious != 0) {
        LINKWORD* pNextLink = (LINKWORD*)TSLINK_NextLink(pLink, -1);
        *pNextLink = nPrevious;
        ((LINKWORD*)*pLink)[1] = pLink[1];
        *pLink = 0;
        pLink[1] = 0;
    }
}

// 1.14d mac: 0000e4be
// The link of the next element; at the end the terminator's address, its low bit cleared.
static void* TSLINK_NextLink(void* pLinkArg, int nLinkOffset) {
    LINKWORD* pLink = (LINKWORD*)pLinkArg;
    LINKWORD nNext = pLink[1];
    if (nNext == 0 || (nNext & 1) != 0) {
        return (void*)(nNext & 0xfffffffe);
    }
    if (nLinkOffset < 0) {
        nLinkOffset = (int)((LINKWORD)pLink - ((LINKWORD*)*pLink)[1]);
    }
    return (void*)(nNext + nLinkOffset);
}

// 1.14d mac: 0000e4f4
static void TSLIST_UnlinkAll(CMDLIST* pList) {
    LINKWORD* pNode;
    while ((pNode = (LINKWORD*)ListElement(pList->pFirst)) != NULL) {
        TSLINK_Unlink(pNode);
    }
}

extern "C" int __cxa_atexit(void (*pfnDestructor)(void*), void* pObject, void* pDso);
extern "C" void* __dso_handle;

// 1.14d mac: 0000e540
// The file's static initialiser: both lists empty, a destructor for each.
void SCMD_InitStaticLists(void) {
    s_tSwitchList.nLinkOffset = 0;
    s_tSwitchList.pLastLink = (CMDDEF*)&s_tSwitchList.pLastLink;
    s_tSwitchList.pFirst = (CMDDEF*)((uintptr_t)&s_tSwitchList.pLastLink + 1);
    __cxa_atexit(SCMD_ListDestructor, &s_tSwitchList, &__dso_handle);
    s_tBareList.nLinkOffset = 0;
    s_tBareList.pLastLink = (CMDDEF*)&s_tBareList.pLastLink;
    s_tBareList.pFirst = (CMDDEF*)((uintptr_t)&s_tBareList.pLastLink + 1);
    __cxa_atexit(SCMD_ListDestructor, &s_tBareList, &__dso_handle);
}

#endif  // D2_OS_MAC
