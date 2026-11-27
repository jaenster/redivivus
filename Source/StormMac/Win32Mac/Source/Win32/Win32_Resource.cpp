// Storm's Win32-on-Mac layer: resource lookup in a data-file module's 'WRes' map,
// 1.14d Intel Mac 0002b500..0002b88c (FindResourceEx, RESOURCE_FindEntry, LoadString).
// The original file is unknown: no binary string names it. RESOURCE_GetModuleResourceMap
// (0002a0c0) is in Win32_Process.cpp, this run follows the RECT functions.
//
// A map entry is variable length and padded to 4 bytes: two size words, the type (a WORD
// id, or -1 then a UTF-16 name) and the name the same way, then the language word.
#include "Win32Text.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <stdlib.h>

// 1.14d mac: 00033a24
int _stricmp(const char* szA, const char* szB);

static void* RESOURCE_FindEntry(unsigned int nTypeOrName, const char* szType, HANDLE hModule, unsigned int nName,
                                short nLanguage);

// 1.14d mac: 0002b500
// The language falls back to US English, then to any.
void* FindResourceEx(HANDLE hModule, const char* szType, const char* szName, short wLanguage) {
    void* pEntry = RESOURCE_FindEntry((unsigned int)szName, szType, hModule, (unsigned int)szName, wLanguage);
    if (pEntry == NULL && wLanguage == 0) {
        pEntry = RESOURCE_FindEntry(0, szType, hModule, (unsigned int)szName, 0x409);
        if (pEntry == NULL) {
            pEntry = RESOURCE_FindEntry(0, szType, hModule, (unsigned int)szName, -1);
        }
    }
    return pEntry;
}

// 1.14d mac: 0002b566
// Type and name are small integers (MAKEINTRESOURCE), "#nnn" strings or names.
static void* RESOURCE_FindEntry(unsigned int nTypeOrName, const char* szTypeArg, HANDLE hModule, unsigned int nNameArg,
                                short nLanguage) {
    char szEntryName[0x100];
    char szEntryType[0x100];

    if (nLanguage == 0) {
        nLanguage = LOCALE_GetDefaultLangId();
    }
    const char* szName = NULL;
    const char* szType = NULL;
    const char* pcType = szTypeArg;
    if ((unsigned int)szTypeArg > 0xFFFF) {
        szType = szTypeArg;
        if (*szTypeArg == '#') {
            pcType = (const char*)(intptr_t)atoi(szTypeArg + 1);
            szType = NULL;
        }
    }
    short nNameId = (short)nNameArg;
    if (nNameArg > 0xFFFF) {
        if (*szTypeArg == '#') {
            nNameId = (short)atoi((const char*)(nNameArg + 1));
        } else {
            szName = (const char*)nNameArg;
        }
    }
    Handle hMap = (Handle)RESOURCE_GetModuleResourceMap(hModule);
    if (hMap == NULL) {
        return NULL;
    }
    long nSize = GetHandleSize(hMap);
    if (nSize > 0) {
        int* pEntry = (int*)*hMap;
        int* pEnd = (int*)((char*)pEntry + nSize);
        do {
            short nEntryTypeId;
            int nOffset;
            if ((short)pEntry[2] == -1) {
                szEntryType[0] = '\0';
                nEntryTypeId = *(short*)((char*)pEntry + 10);
                nOffset = 4;
            } else {
                szEntryType[0] = (char)(short)pEntry[2];
                char* pDest = szEntryType + 1;
                int nPos = 10;
                short nChar;
                do {
                    nOffset = nPos;
                    nChar = *(short*)((char*)pEntry + nOffset);
                    *pDest++ = (char)nChar;
                    nPos = nOffset + 2;
                } while (nChar != 0);
                nOffset = nOffset - 6;
                nEntryTypeId = 0;
            }
            short nEntryNameId;
            int nNameOffset;
            short nFirst = *(short*)(nOffset + 8 + (char*)pEntry);
            if (nFirst == -1) {
                szEntryName[0] = '\0';
                nEntryNameId = *(short*)(nOffset + 10 + (char*)pEntry);
                nNameOffset = 4;
            } else {
                szEntryName[0] = (char)nFirst;
                char* pDest = szEntryName + 1;
                int nPos = 0;
                short nChar;
                do {
                    nNameOffset = nPos;
                    nChar = *(short*)((char*)pEntry + nNameOffset + nOffset + 10);
                    *pDest++ = (char)nChar;
                    nPos = nNameOffset + 2;
                } while (nChar != 0);
                nNameOffset = nNameOffset + 4;
                nEntryNameId = 0;
            }
            bool bLangMismatch = false;
            if (nLanguage != (short)0xFFFF) {
                bLangMismatch = *(uint16_t*)((((nOffset + 0xB + nNameOffset) & 0xFFFFFFFCU)) + 6 + (char*)pEntry) !=
                                (uint16_t)nLanguage;
            }
            bool bTypeMatch;
            if (szType == NULL) {
                bTypeMatch = false;
                if (szEntryType[0] == '\0') {
                    bTypeMatch = nEntryTypeId == (short)(intptr_t)pcType;
                }
            } else {
                bTypeMatch = false;
                if (szEntryType[0] != '\0') {
                    bTypeMatch = _stricmp(szEntryType, szType) == 0;
                }
            }
            bool bNameMatch;
            bool bNameKnown = true;
            if (szName == NULL) {
                if (szEntryName[0] == '\0') {
                    bNameMatch = nEntryNameId == nNameId;
                } else {
                    bNameKnown = false;
                }
            } else if (szEntryName[0] != '\0') {
                bNameMatch = _stricmp(szEntryName, szName) == 0;
            } else {
                bNameKnown = false;
            }
            if (bNameKnown && bTypeMatch && bNameMatch && !bLangMismatch) {
                return pEntry;
            }
            pEntry = (int*)((char*)pEntry + ((pEntry[1] + 3U) & 0xFFFFFFFC) + ((*pEntry + 3U) & 0xFFFFFFFC));
        } while (pEntry < pEnd);
    }
    return NULL;
}

// 1.14d mac: 0002b7d2
// RT_STRING (6) block (id >> 4) + 1, entry id & 15, narrowed to bytes.
UINT LoadString(HANDLE hInstance, UINT uID, char* szBuffer, int nBufferMax) {
    UINT nCopied = 0;
    if (nBufferMax > 0) {
        char* pResource = (char*)FindResourceEx(hInstance, (const char*)6, (const char*)((uID >> 4) + 1), 0);
        if (pResource != NULL) {
            int nSkip = (*(int*)(pResource + 4) + 3) & ~3;
            char* pString = pResource + nSkip;
            short nLength = *(short*)pString;
            for (UINT nEntry = uID & 0xF; nEntry != 0; nEntry--) {
                pString += nLength * 2 + 2;
                nLength = *(short*)pString;
            }
            nCopied = nBufferMax - 1;
            if (nLength <= (int)nCopied) {
                nCopied = nLength;
            }
            if ((int)nCopied > 0) {
                for (UINT n = nCopied; n != 0; n--) {
                    pString += 2;
                    *szBuffer++ = *pString;
                }
            }
            *szBuffer = 0;
        }
    }
    return nCopied;
}

#endif  // D2_OS_MAC
