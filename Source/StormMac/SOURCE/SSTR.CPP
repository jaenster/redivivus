// Storm Mac: SSTR.CPP, named by the __FILE__ literal of SStrDup's allocation (1.14d Intel Mac
// 0001c9bd, line 0x2b1). The string API: bounded copy and append, compare, search, length, hash,
// tokenizer and printf. 1.14d Mac band 0001c730..0001d2f9. Windows' Source/Storm/Source/SSTR.cpp
// is the same module; this file follows the Mac object's own code.
//
// SStrLen, SStrCopy and SStrPack are the C runtime's word-at-a-time scans (the 0x7efefeff
// zero-byte test); they are written here as their plain per-byte meaning, so they compare as
// different instructions.
#include "Source/StormMac/H/stpl.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include "Source/StormMac/H/StormApi.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// The Win32 layer's case-insensitive compare and case folds.
int _strnicmp(const char* szString1, const char* szString2, int nMaxChars);
char* _strupr(char* szString);
char* _strlwr(char* szString);

#define ERROR_INVALID_PARAMETER 0x57

static const uint32_t gnHashLowTable[16] = {
    0x486e26ee, 0xdcaa16b3, 0xe1918eef, 0x202dafdb, 0x341c7dc7, 0x1c365303, 0x40ef2d37, 0x65fd5e49,
    0xd6057177, 0x904ece93, 0x1c38024f, 0x98fd323b, 0xe3061ae7, 0xa39b0fa1, 0x9797f25f, 0xe4444563,
};

// 1.14d mac: 0001c730
char* Storm::Source::SSTR::SStrChr(const char* szText, char cTarget) {
    if (szText == NULL) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }
    for (; *szText != '\0'; szText++) {
        if (*szText == cTarget) {
            return (char*)szText;
        }
    }
    return NULL;
}

// 1.14d mac: 0001c766
char* Storm::Source::SSTR::SStrChrR(const char* szText, char cTarget) {
    char* pLastMatch;
    if (szText == NULL) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }
    pLastMatch = NULL;
    for (; *szText != '\0'; szText++) {
        if (*szText == cTarget) {
            pLastMatch = (char*)szText;
        }
    }
    return pLastMatch;
}

// 1.14d mac: 0001c7a0
int Storm::Source::SSTR::SStrCmp(const char* szString1, const char* szString2, uint32_t nMaxCount) {
    if (szString2 == NULL || szString1 == NULL) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
        return (int)szString1 - (int)szString2;
    }
    return strncmp(szString1, szString2, nMaxCount);
}

// 1.14d mac: 0001c7d3
int Storm::Source::SSTR::SStrCmpI(const char* szString1, const char* szString2, uint32_t nMaxCount) {
    if (szString2 == NULL || szString1 == NULL) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
        return (int)szString1 - (int)szString2;
    }
    return _strnicmp(szString1, szString2, nMaxCount);
}

// 1.14d mac: 0001c806
uint32_t Storm::Source::SSTR::SStrCopy(char* szBufferOutput, const char* szBufferInput, int nBufferOutputLength) {
    int n;
    if (szBufferInput == NULL || szBufferOutput == NULL) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    if (nBufferOutputLength == 0) {
        return 0;
    }
    for (n = 0; n < nBufferOutputLength - 1; n++) {
        szBufferOutput[n] = szBufferInput[n];
        if (szBufferInput[n] == '\0') {
            return n;
        }
    }
    szBufferOutput[n] = '\0';
    return n;
}

// 1.14d mac: 0001c9bd
char* Storm::Source::SSTR::SStrDup(const char* szText) {
    if (szText == NULL) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }
    size_t nBytes = SStrLen(szText) + 1;
    char* pCopy = (char*)SMemAlloc(nBytes, __FILE__, 0x2b1, 0);
    if ((int)nBytes > 0) {
        memmove(pCopy, szText, nBytes);
    }
    return pCopy;
}

// 1.14d mac: 0001ca30
uint32_t Storm::Source::SSTR::SStrLen(const char* szText) {
    if (szText == NULL) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    const char* p = szText;
    while (*p != '\0') {
        p++;
    }
    return p - szText;
}

// 1.14d mac: 0001caaf
uint32_t Storm::Source::SSTR::SStrPack(char* szText, const char* szSource, uint32_t nBufferLength) {
    if (szSource == NULL || szText == NULL) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    if (nBufferLength == 0) {
        return 0;
    }
    char* pDstLast = szText + nBufferLength - 1;
    if (nBufferLength != 0x7fffffff) {
        *pDstLast = '\0';
    }
    char* pEnd = szText;
    while (*pEnd != '\0') {
        pEnd++;
    }
    int n = 0;
    while (pEnd + n < pDstLast) {
        pEnd[n] = szSource[n];
        if (szSource[n] == '\0') {
            return (pEnd + n) - szText;
        }
        n++;
    }
    *pDstLast = '\0';
    return pDstLast - szText;
}

// 1.14d mac: 0001ccd5
uint32_t Storm::Source::SSTR::SStrPrintf(char* szOutput, uint32_t nSize, const char* szFormat, ...) {
    size_t nWritten = 0;
    if (szFormat == NULL || szOutput == NULL) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    if (nSize != 0) {
        va_list vaArgs;
        va_start(vaArgs, szFormat);
        nWritten = vsnprintf(szOutput, nSize, szFormat, vaArgs);
        va_end(vaArgs);
        if ((int)nWritten < 0) {
            szOutput[nSize - 1] = '\0';
            nWritten = nSize - 1;
        }
    }
    return nWritten;
}

// 1.14d mac: 0001cd31
uint32_t Storm::Source::SSTR::SStrVPrintf(char* szOutput, uint32_t nSize, const char* szFormat, va_list vaArgs) {
    uint32_t nWritten = 0;
    if (szFormat == NULL || szOutput == NULL) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    if (nSize != 0) {
        nWritten = vsnprintf(szOutput, nSize, szFormat, vaArgs);
        if ((int)nWritten < 0) {
            szOutput[nSize - 1] = '\0';
            nWritten = nSize - 1;
        }
    }
    return nWritten;
}

// 1.14d mac: 0001cd8c
// Copies the next token of *ppszString into szBuffer. Characters of szWhitespace separate
// tokens; when the double quote is one of them, a quoted run is one token.
void Storm::Source::SSTR::SStrTokenize(char** ppszString, char* szBuffer, uint32_t nBufferChars, const char* szWhitespace, uint32_t* pbQuoted) {
    if (szWhitespace == NULL || *ppszString == NULL || ppszString == NULL || (nBufferChars != 0 && szBuffer == NULL)) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
        return;
    }
    char* pszCur = *ppszString;
    uint32_t nLength = 0;
    int bQuoted = 0;
    bool bInQuotes = false;
    char* pszQuoteSep = SStrChr(szWhitespace, '"');

    // skip the separators in front of the token
    for (;;) {
        char c = *pszCur;
        if (c == '\0' || SStrChr(szWhitespace, c) == NULL) {
            break;
        }
        pszCur++;
        if (pszQuoteSep != NULL && c == '"') {
            bInQuotes = true;
            bQuoted = 1;
            break;
        }
    }

    for (;;) {
        char c = *pszCur;
        if (c == '\0') {
            break;
        }
        if (pszQuoteSep != NULL && c == '"') {
            if (nLength != 0 && !bInQuotes) {
                break;
            }
            pszCur++;
            if (bInQuotes) {
                bQuoted = 1;
                break;
            }
            bInQuotes = true;
            bQuoted = 1;
            continue;
        }
        if (!bInQuotes && SStrChr(szWhitespace, c) != NULL) {
            pszCur++;
            break;
        }
        if (nLength + 1 < nBufferChars) {
            szBuffer[nLength++] = c;
        }
        pszCur++;
    }
    if (nLength < nBufferChars) {
        szBuffer[nLength] = '\0';
    }
    *ppszString = pszCur;
    if (pbQuoted != NULL) {
        *pbQuoted = bQuoted;
    }
}

// 1.14d mac: 0001ceaa
uint32_t Storm::Source::SSTR::SStrHash(char* pszText, uint8_t bCaseSensitive, uint32_t nSeed) {
    if (pszText == NULL) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    if (nSeed == 0) {
        nSeed = 0x7fed7fed;
    }
    if ((bCaseSensitive & 1) == 0) {
        int nAccum = -0x11111112;
        for (uint32_t nChar = (uint8_t)*pszText; nChar != 0; nChar = (uint8_t)*pszText) {
            pszText++;
            if (nChar >= 'a' && nChar <= 'z') {
                nChar -= 0x20;
            }
            if (nChar == '/') {
                nChar = '\\';
            }
            nSeed = (nSeed + nAccum) ^ (gnHashLowTable[nChar >> 4] - gnHashLowTable[nChar & 0xf]);
            nAccum = nSeed + 3 + nAccum * 0x21 + nChar;
        }
    } else {
        int nAccum = -0x11111112;
        for (uint32_t nChar = (uint8_t)*pszText; nChar != 0; nChar = (uint8_t)*pszText) {
            pszText++;
            nSeed = (nSeed + nAccum) ^ (gnHashLowTable[nChar >> 4] - gnHashLowTable[nChar & 0xf]);
            nAccum = nSeed + 3 + nAccum * 0x21 + nChar;
        }
    }
    return nSeed + (nSeed == 0);
}

// Jenkins' mix: the table-key hash SStrHashHT runs over the folded key.
#define SSTR_JENKINS_MIX(a, b, c) \
    a = (a - b - c) ^ (c >> 13);  \
    b = (b - c - a) ^ (a << 8);   \
    c = (c - a - b) ^ (b >> 13);  \
    a = (a - b - c) ^ (c >> 12);  \
    b = (b - c - a) ^ (a << 16);  \
    c = (c - a - b) ^ (b >> 5);   \
    a = (a - b - c) ^ (c >> 3);   \
    b = (b - c - a) ^ (a << 10);  \
    c = (c - a - b) ^ (b >> 15)

// 1.14d mac: 0001cfaf
// The key is upper-cased and '/' becomes '\\' in a 1024-byte buffer first.
uint32_t Storm::Source::SSTR::SStrHashHT(uint8_t* szText) {
    uint8_t szUpper[1024];
    uint32_t nLen = 0;
    uint32_t a = 0x9e3779b9, b = 0x9e3779b9, c = 0;
    uint8_t cChar = *szText;
    while (cChar != 0 && nLen < 0x3ff) {
        if ((uint8_t)(cChar - 'a') < 26) {
            cChar -= 0x20;
        } else if (cChar == '/') {
            cChar = '\\';
        }
        szUpper[nLen++] = cChar;
        cChar = szText[nLen];
    }
    szUpper[nLen] = 0;

    uint32_t nLeft = nLen;
    const uint8_t* p = szUpper;
    const uint32_t kJenkinsBlock = 3 * sizeof(uint32_t);
    while (nLeft >= kJenkinsBlock) {
        c += (uint32_t)p[8] | (uint32_t)p[9] << 8 | (uint32_t)p[10] << 16 | (uint32_t)p[11] << 24;
        b += (uint32_t)p[4] | (uint32_t)p[5] << 8 | (uint32_t)p[6] << 16 | (uint32_t)p[7] << 24;
        a += (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
        SSTR_JENKINS_MIX(a, b, c);
        p += kJenkinsBlock;
        nLeft -= 12;
    }
    c += nLen;
    switch (nLeft) {
    case 11: c += (uint32_t)p[10] << 24;
    case 10: c += (uint32_t)p[9] << 16;
    case 9:  c += (uint32_t)p[8] << 8;
    case 8:  b += (uint32_t)p[7] << 24;
    case 7:  b += (uint32_t)p[6] << 16;
    case 6:  b += (uint32_t)p[5] << 8;
    case 5:  b += p[4];
    case 4:  a += (uint32_t)p[3] << 24;
    case 3:  a += (uint32_t)p[2] << 16;
    case 2:  a += (uint32_t)p[1] << 8;
    case 1:  a += p[0];
    }
    SSTR_JENKINS_MIX(a, b, c);
    return c;
}

// 1.14d mac: 0001d2f0
void Storm::Source::SSTR::SStrUpper(char* szText) {
    _strupr(szText);
}

// 1.14d mac: 0001d2f9
void Storm::Source::SSTR::SStrLower(char* szText) {
    _strlwr(szText);
}

#endif  // D2_OS_MAC
