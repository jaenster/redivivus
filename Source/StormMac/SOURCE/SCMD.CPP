// StormMac Storm Mac/SOURCE/SCMD.CPP: the command-line switch registry and parser,
// 1.14d Intel Mac 0000d43e..0000e5cf.
//
// Two lists of switch definitions (CMDDEF, 0x40 bytes) are kept: the positional
// arguments (0003f0254, kind 0x1000000 optional or 0x2000000 required) and the flags that
// take a leading '-' or '/' (0003f0260, kind 0). SCmdProcess walks a command line over them. The intrusive list helpers after the parser are the TSList
// instantiations this file's object holds; the file's static initialiser closes the run.
//
// __FILE__ is ".../StormMac/Storm Mac/SOURCE/SCMD.CPP".
#include "../Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include "Source/StormMac/H/StormApi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int _strnicmp(const char* szA, const char* szB, int nCount);
HANDLE CreateFile(const char* szName, DWORD dwAccess, DWORD dwShare, void* pSecurity, DWORD dwDisposition, DWORD dwFlags,
                  HANDLE hTemplate);
DWORD GetFileSize(HANDLE hFile, DWORD* pdwHigh);
BOOL ReadFile(HANDLE hFile, void* pBuffer, DWORD nBytes, DWORD* pnRead, OVERLAPPED* pOverlapped);

// The list templates are instantiations shared with other objects, so they keep the C calling
// convention. noinline is kept as evidence, not cause: the binary calls every one of them out of
// line (TSLINK_Unlink, 23 instructions, included) where clang-425 -Os inlines an inline function
// of that size; what kept the original's out of line is not recovered.
#define SCMD_TPL inline __attribute__((noinline))
// Storm's release-build parameter check: every condition is evaluated and and-ed into one
// mask (no short circuit), and a failure sets ERROR_INVALID_PARAMETER and returns 0.
#define SCMD_VALIDATE_BEGIN int nValidMask = -1
#define SCMD_VALIDATE(bCondition) if (!(bCondition)) nValidMask = 0
#define SCMD_VALIDATE_END                \
    if (nValidMask == 0) {               \
        SErrSetLastError(0x57);          \
        return 0;                        \
    }
#define SCMD_FILE "/Users/bclemetson/dev/diablo2/Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/SOURCE/SCMD.CPP"

// 1.14d mac: 003cf6e8
// The messages for the errors 0x85100065, 0x8510006d and 0x6e, in that order. The table is in
// __const and SCMD_ReportError, its only reader, indexes it off the PIC base: a file-local constant.
static const char* const gaSCMD_FallbackMessages[3] = {
    "Invalid argument: %s",
    "The syntax of the command is incorrect.",
    "Unable to open response file: %s",
};

struct CMDDEF;
typedef int (*CMDCALLBACK)(void* pParams, const char* szText);
typedef int (*CMDUNKNOWNCALLBACK)(const char* szArg);
typedef int (*CMDERRORCALLBACK)(void* pErrorInfo);

// The CMDPARAMS a switch callback is given.
struct CMDPARAMS {
    uint32_t dwFlags;
    int32_t nGroupId;
    const char* szName;
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
static CMDLIST s_tArgList;
// 1.14d mac: 003f0260
static CMDLIST s_tFlagList;
// 1.14d mac: 003f026c
// An optional positional argument (kind 0x1000000) has been registered: a required one
// may no longer follow.
static int32_t s_bAddedOptional;

// The next element of a list, NULL at the terminator.
// Both tests are evaluated: the end test is a bitwise or.
static inline bool ListIsTerminator(CMDDEF* pNext) {
    return ((intptr_t)pNext == 0) | ((intptr_t)pNext & 1);
}

static inline CMDDEF* ListElement(CMDDEF* pNext) {
    return ListIsTerminator(pNext) ? NULL : pNext;
}

// The raw next pointer after pElement (NULL starts from the list's own link): an element,
// or the terminator mark the caller tests for.
static inline CMDDEF* ListRawNext(CMDLIST* pList, CMDDEF* pElement) {
    CMDDEF* pLink = pElement != NULL ? pElement : (CMDDEF*)&pList->pLastLink;
    return pLink->pNextElement;
}

void TSLIST_DeleteAll(CMDLIST* pList);
SCMD_TPL void TSLIST_UnlinkAll(CMDLIST* pList);
SCMD_TPL void TSLINK_Unlink(void* pLink);
SCMD_TPL void* TSLINK_NextLink(void* pLink, int nLinkOffset);
SCMD_TPL CMDDEF* TSList_NewNode(CMDLIST* pList, int nLinkType, int nExtraBytes, uint32_t dwAllocFlags);
SCMD_TPL void TSList_LinkNode(CMDLIST* pList, CMDDEF* pNode, int nLinkType, void* pExistingLink);
// The file-local routines are static (LLVM gives them the fast convention: first two arguments
// in ecx and edx) and stay out of line on their own.
static void SCMD_ReportError(CMDERRORCALLBACK pfnError, DWORD dwError, const char* szText);
static CMDDEF* SCMD_FindFlagDef(const char* szToken, CMDDEF* pFirstDef, int32_t nMinLength);
static int PerformConversion(CMDDEF* pDef, const char* szValue, uint32_t* pnConsumed);
static BOOL SCMD_ProcessCurrentFlag(const char* szArgLine, SCMD_PROCESSING* pProcessing, int* pnConsumed);
static BOOL SCMD_ProcessTokens(char** ppszCursor, SCMD_PROCESSING* pProcessing, CMDDEF** ppPositional,
                               CMDUNKNOWNCALLBACK pfnExtra, CMDERRORCALLBACK pfnError);

// 1.14d mac: 0000d43e
// Frees every string value, then both lists.
BOOL SCmdDestroy(void) {
    for (int nList = 0; nList != 2; nList++) {
        CMDLIST* pList = nList != 0 ? &s_tFlagList : &s_tArgList;
        for (CMDDEF* pFlag = ListElement(pList->pFirst); !ListIsTerminator(pFlag); pFlag = ListRawNext(pList, pFlag)) {
            if ((pFlag->dwFlags & 0x30000) == 0x20000 && pFlag->uValue != 0) {
                SMemFree((void*)pFlag->uValue, SCMD_FILE, 0x21a, 0);
                pFlag->uValue = 0;
            }
        }
    }
    TSLIST_DeleteAll(&s_tArgList);
    TSLIST_DeleteAll(&s_tFlagList);
    s_bAddedOptional = 0;
    return 1;
}

// 1.14d mac: 0000d532
// Runs a command line over the registered switches; the positional arguments are the
// first list's, in order, and a required one left over is an error.
int Storm::Source::SCmdProcess(char* szCmdLine, int bSkipProgName, int (*pfnExtraArg)(char*), int (*pfnErrorArg)(uint32_t*)) {
    CMDUNKNOWNCALLBACK pfnExtra = (CMDUNKNOWNCALLBACK)pfnExtraArg;
    CMDERRORCALLBACK pfnError = (CMDERRORCALLBACK)pfnErrorArg;
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
    CMDDEF* pPositional = ListElement(s_tArgList.pFirst);
    int bResult = 0;
    if (SCMD_ProcessTokens(&pszCursor, &tProcessing, &pPositional, pfnExtra, pfnError)) {
        bResult = 1;
        while (pPositional != NULL) {
            if ((pPositional->dwFlags & 0x3000000) == 0x2000000) {
                bResult = 0;
                if (pfnError != NULL) {
                    SCMD_ReportError(pfnError, 0x8510006d, "");
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
static BOOL SCMD_ProcessTokens(char** ppszCursor, SCMD_PROCESSING* pProcessing, CMDDEF** ppPositional,
                               CMDUNKNOWNCALLBACK pfnExtra, CMDERRORCALLBACK pfnError) {
    if (**ppszCursor != '\0') {
        char* pszNext = *ppszCursor;
        do {
            char szToken[0x100];
            char szName[0x100];
            uint32_t bQuoted = 0;
            int nConsumed;
            int bOk;
            SStrTokenize(&pszNext, szToken, 0x100, " ,;\"\t\n\r\x1a", &bQuoted);
            if (szToken[0] != '\0') {
                if (szToken[0] == '@' && bQuoted == 0) {
                    HANDLE hFile = CreateFile(szToken + 1, (DWORD)-0x80000000, 1, 0, 3, 0x8000000, 0);
                    if (hFile == 0) {
                        if (pfnError != NULL) {
                            SCMD_ReportError(pfnError, 0x6e, szToken + 1);
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
                    bOk = SCMD_ProcessTokens(&pFileCursor, pProcessing, ppPositional, pfnExtra, pfnError);
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
                                pFound = SCMD_FindFlagDef(szName, ListElement(s_tFlagList.pFirst), 0);
                                if (pFound != NULL) {
                                    break;
                                }
                            }
                            nTry--;
                        }
                        if (pFound == NULL) {
                            if (pfnError != NULL) {
                                SCMD_ReportError(pfnError, 0x85100065, pszText);
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
                            SCMD_ReportError(pfnError, 0x85100065, szToken);
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
static void SCMD_ReportError(CMDERRORCALLBACK pfnError, DWORD dwError, const char* szText) {
    int nMessage;
    switch (dwError) {
    case 0x85100065:
        nMessage = 0;
        break;
    case 0x8510006d:
        nMessage = 1;
        break;
    case 0x6e:
        nMessage = 2;
        break;
    default:
        return;
    }
    char szMessage[256];
    char szFormat[256];
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


// 1.14d mac: 0000dce8
BOOL Storm::Source::SCmdRegisterArgument(uint32_t dwFlags, uint32_t nGroupId, char* szText, void* pVariable, uint32_t nVariableSize,
                          uint32_t nSetValue, uint32_t nSetMask, int (*pfnCallbackArg)(CMDPARAMS*, char*)) {
    CMDCALLBACK pfnCallback = (CMDCALLBACK)pfnCallbackArg;
    const char* szName = "";
    if (szText != NULL) {
        szName = szText;
    }
    int nNameLen = SStrLen(szName);
    SCMD_VALIDATE_BEGIN;
    SCMD_VALIDATE(nVariableSize == 0 || pVariable != NULL);
    SCMD_VALIDATE((dwFlags & 0x3000000) != 0x2000000 || !s_bAddedOptional);
    SCMD_VALIDATE(nNameLen < 0x10);
    SCMD_VALIDATE((dwFlags & 0x3000000) != 0 || nNameLen > 0);
    SCMD_VALIDATE((dwFlags & 0x30000) != 0 || pVariable == NULL || nVariableSize == 4);
    SCMD_VALIDATE_END;
    CMDDEF* pDef;
    if ((dwFlags & 0x3000000) == 0) {
        pDef = TSList_NewNode(&s_tFlagList, 2, 0, 0);
    } else {
        pDef = TSList_NewNode(&s_tArgList, 2, 0, 0);
    }
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
    if ((dwFlags & 0x3000000) == 0x1000000) {
        s_bAddedOptional = 1;
    }
    return 1;
}

// One definition of the list SCmdRegisterArgList takes: the four words of an argument.
struct ARGLIST {
    uint32_t dwFlags;
    uint32_t nGroupId;
    char* szText;
    int (*pfnCallback)(CMDPARAMS*, char*);
};

// 1.14d mac: 0000dc63
BOOL Storm::Source::SCmdRegisterArgList(ARGLIST* pCmdDefArray, uint32_t nCount) {
    if (pCmdDefArray == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    for (uint32_t nIndex = 0; nIndex < nCount; nIndex++) {
        if (!SCmdRegisterArgument(pCmdDefArray->dwFlags, pCmdDefArray->nGroupId, pCmdDefArray->szText, NULL, 0, 1, -1,
                                  pCmdDefArray->pfnCallback)) {
            return 0;
        }
        pCmdDefArray++;
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
        *pnConsumed = *pnConsumed < (int)nArgConsumed ? (int)nArgConsumed : *pnConsumed;
        pDef = SCMD_FindFlagDef(pProcessing->szName, ListElement(pDef->pNextElement), pProcessing->nNameLen);
    }
    return 1;
}

// A flag: a leading '-' clears it, '+' sets it; otherwise the definition's default (bit 0
// set: clear) applies. Inlined into PerformConversion, as are the two below.
static void ConvertBool(CMDDEF* pDef, const char* szValue, uint32_t* pnConsumed) {
    bool bSet;
    if (*szValue == '-') {
        bSet = false;
        *pnConsumed = 1;
    } else if (*szValue == '+') {
        bSet = true;
        *pnConsumed = 1;
    } else if ((pDef->dwFlags & 1) != 0) {
        bSet = false;
    } else {
        bSet = true;
    }
    pDef->uValue &= ~pDef->dwSetMask;
    if (bSet) {
        pDef->uValue |= pDef->dwSetValue;
    }
    if (pDef->pVariable != NULL) {
        *(uint32_t*)pDef->pVariable &= ~pDef->dwSetMask;
        if (bSet) {
            *(uint32_t*)pDef->pVariable |= pDef->dwSetValue;
        }
    }
}

// A number, signed when bit 0 is set; the variable takes up to four bytes of it.
static void ConvertNumber(CMDDEF* pDef, const char* szValue, uint32_t* pnConsumed) {
    char* pszEnd = NULL;
    if ((pDef->dwFlags & 1) != 0) {
        pDef->uValue = strtol(szValue, &pszEnd, 0);
    } else {
        pDef->uValue = strtoul(szValue, &pszEnd, 0);
    }
    *pnConsumed = pszEnd != NULL ? (uint32_t)(pszEnd - szValue) : SStrLen(szValue);
    if (pDef->pVariable != NULL) {
        int nSize = (uint32_t)pDef->nVariableSize < sizeof(uint32_t) ? pDef->nVariableSize : (int)sizeof(uint32_t);
        if (nSize > 0) {
            memmove(pDef->pVariable, &pDef->uValue, nSize);
        }
    }
}

// A string: the definition keeps its own copy; the variable takes one of its size.
static void ConvertString(CMDDEF* pDef, const char* szValue, uint32_t* pnConsumed) {
    *pnConsumed = SStrLen(szValue);
    if (pDef->uValue != 0) {
        SMemFree((void*)pDef->uValue, SCMD_FILE, 0x8f, 0);
    }
    pDef->uValue = (uintptr_t)SMemAlloc(SStrLen(szValue) + 1, SCMD_FILE, 0x90, 0);
    SStrCopy((char*)pDef->uValue, szValue, 0x7fffffff);
    if (pDef->pVariable != NULL) {
        SStrCopy((char*)pDef->pVariable, szValue, pDef->nVariableSize);
    }
}

// 1.14d mac: 0000dec7
// Stores a value into a definition: a string (copied), a number, or a flag set or cleared by
// its mask (a leading '+' or '-' on the text chooses). Then the callback, then every other
// definition of the same group and type takes the value too.
static int PerformConversion(CMDDEF* pDef, const char* szValue, uint32_t* pnConsumed) {
    *pnConsumed = 0;
    switch (pDef->dwFlags & 0x30000) {
    case 0:
        ConvertBool(pDef, szValue, pnConsumed);
        break;
    case 0x10000:
        ConvertNumber(pDef, szValue, pnConsumed);
        break;
    case 0x20000:
        ConvertString(pDef, szValue, pnConsumed);
        break;
    default:
        return 0;
    }
    pDef->bSupplied = 1;
    if (pDef->pfnCallback != NULL) {
        CMDPARAMS tParams;
        tParams.dwFlags = pDef->dwFlags;
        tParams.nGroupId = pDef->nGroupId;
        tParams.szName = pDef->szName;
        tParams.pVariable = pDef->pVariable;
        tParams.dwSetValue = pDef->dwSetValue;
        tParams.dwSetMask = pDef->dwSetMask;
        tParams.uValue = pDef->uValue;
        if (pDef->pfnCallback(&tParams, szValue) == 0) {
            return 0;
        }
    }
    for (int nList = 0; nList != 2; nList++) {
        CMDLIST* pList = nList != 0 ? &s_tFlagList : &s_tArgList;
        for (CMDDEF* pOther = ListElement(pList->pFirst); !ListIsTerminator(pOther);
             pOther = ListRawNext(pList, pOther)) {
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
            if (nCmdLen > nBest && nCmdLen <= nTokenLen) {
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
// A list's destructor: unlink everything, then the list's own link. The initialiser takes its
// address directly (lea off the PIC base, no non-lazy pointer), so it is not a weak copy.
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
SCMD_TPL CMDDEF* TSList_NewNode(CMDLIST* pList, int nLinkType, int nExtraBytes, uint32_t dwAllocFlags) {
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
SCMD_TPL void TSList_LinkNode(CMDLIST* pList, CMDDEF* pNode, int nLinkType, void* pExistingLink) {
    LINKWORD* pLink = (LINKWORD*)pNode;
    LINKWORD* pListWords = (LINKWORD*)pList;
    if (pNode == NULL) {
        pLink = pListWords + 1;
    }
    if (*pLink != 0) {
        TSLINK_Unlink(pLink);
    }
    LINKWORD* pTarget = (LINKWORD*)pExistingLink;
    if (pExistingLink == NULL) {
        pTarget = pListWords + 1;
    }
    switch (nLinkType) {
    case 1: {
        *pLink = (LINKWORD)pTarget;
        pLink[1] = pTarget[1];
        LINKWORD* pNextLink = (LINKWORD*)TSLINK_NextLink(pTarget, (int)*pListWords);
        *pNextLink = (LINKWORD)pLink;
        pTarget[1] = (LINKWORD)pNode;
        break;
    }
    case 2: {
        LINKWORD* pPrevious = (LINKWORD*)*pTarget;
        *pLink = (LINKWORD)pPrevious;
        pLink[1] = pPrevious[1];
        pPrevious[1] = (LINKWORD)pNode;
        *pTarget = (LINKWORD)pLink;
        break;
    }
    }
}

// 1.14d mac: 0000e47e
SCMD_TPL void TSLINK_Unlink(void* pLinkArg) {
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
SCMD_TPL void* TSLINK_NextLink(void* pLinkArg, int nLinkOffset) {
    LINKWORD* pLink = (LINKWORD*)pLinkArg;
    LINKWORD nNext = pLink[1];
    if ((nNext == 0) | ((nNext & 1) != 0)) {
        return (void*)(nNext & 0xfffffffe);
    }
    if (nLinkOffset < 0) {
        nLinkOffset = (int)((LINKWORD)pLink - ((LINKWORD*)*pLink)[1]);
    }
    return (void*)(nNext + nLinkOffset);
}

// 1.14d mac: 0000e4f4
SCMD_TPL void TSLIST_UnlinkAll(CMDLIST* pList) {
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
    s_tArgList.nLinkOffset = 0;
    s_tArgList.pLastLink = (CMDDEF*)&s_tArgList.pLastLink;
    s_tArgList.pFirst = (CMDDEF*)((uintptr_t)&s_tArgList.pLastLink | 1);
    __cxa_atexit(SCMD_ListDestructor, &s_tArgList, &__dso_handle);
    s_tFlagList.nLinkOffset = 0;
    s_tFlagList.pLastLink = (CMDDEF*)&s_tFlagList.pLastLink;
    s_tFlagList.pFirst = (CMDDEF*)((uintptr_t)&s_tFlagList.pLastLink | 1);
    __cxa_atexit(SCMD_ListDestructor, &s_tFlagList, &__dso_handle);
}

#endif  // D2_OS_MAC
