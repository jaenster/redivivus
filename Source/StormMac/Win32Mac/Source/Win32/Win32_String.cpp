// StormMac Win32 Mac/Source/Win32/Win32_String.cpp: the Microsoft CRT string and number
// helpers the layer supplies (_stricmp, _itoa, rand ...), 1.14d Intel Mac
// 00033a24..00033cef. Several have a Windows twin in Game.exe, which linked the VC CRT's own
// copy. The original name of this file is not recorded; see Source/StormMac/README.md.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// 1.14d mac: 00033a24
int _stricmp(const char* szFirst, const char* szSecond) {
    for (;;) {
        char cFirst = (char)tolower(*szFirst++);
        char cSecond = (char)tolower(*szSecond++);
        if (cFirst < cSecond) {
            return -1;
        }
        if (cSecond < cFirst) {
            return 1;
        }
        if (cFirst == '\0') {
            return 0;
        }
    }
}

// 1.14d mac: 00033a6f
// Case-insensitive compare of up to nCount characters, -1, 0 or 1.
int _strnicmp(const char* szString1, const char* szString2, int nCount) {
    int nIndex = 0;
    for (;;) {
        if (nCount <= nIndex) {
            return 0;
        }
        char cFirst = (char)tolower(szString1[nIndex]);
        char cSecond = (char)tolower(szString2[nIndex]);
        if (cFirst < cSecond) {
            return -1;
        }
        if (cSecond < cFirst) {
            return 1;
        }
        nIndex++;
        if (cFirst == '\0') {
            return 0;
        }
    }
}

// 1.14d mac: 00033ac4
char* _strrev(char* szString) {
    int nLast = (int)strlen(szString) - 1;
    for (int nFirst = 0; nFirst < nLast; nFirst++, nLast--) {
        char cTemp = szString[nFirst];
        szString[nFirst] = szString[nLast];
        szString[nLast] = cTemp;
    }
    return szString;
}

// 1.14d mac: 00033afc
// Upper-cases in place, past the "\\{MacPathNN}\\" prefix of a token path.
char* _strupr(char* szString) {
    char* pChar = PATH_IsMacPathToken(szString) ? szString + 13 : szString;
    char cChar = *pChar;
    while (cChar != '\0') {
        *pChar = (char)toupper(cChar);
        cChar = pChar[1];
        pChar++;
    }
    return szString;
}

// 1.14d mac: 00033b3c
char* _strlwr(char* szString) {
    char* pChar = PATH_IsMacPathToken(szString) ? szString + 13 : szString;
    char cChar = *pChar;
    while (cChar != '\0') {
        *pChar = (char)tolower(cChar);
        cChar = pChar[1];
        pChar++;
    }
    return szString;
}

// 1.14d mac: 00033b7c
char* _itoa(int nValue, char* szBuffer, int nRadix) {
    int nMagnitude = nValue > 0 ? nValue : -nValue;
    int nLength = 0;
    do {
        int nDigit = nMagnitude % nRadix;
        szBuffer[nLength] = nDigit < 10 ? (char)(nDigit + '0') : (char)(nDigit + '7');
        nMagnitude /= nRadix;
        nLength++;
    } while (nMagnitude != 0);
    if (nValue < 0) {
        szBuffer[nLength] = '-';
        nLength++;
    }
    szBuffer[nLength] = '\0';
    _strrev(szBuffer);
    return szBuffer;
}

// 1.14d mac: 00033bd9
char* _ui64toa(uint64_t nValue, char* szBuffer, int nRadix) {
    char* pOut = szBuffer;
    do {
        int nDigit = (int)(nValue % (uint64_t)(int64_t)nRadix);
        *pOut = nDigit < 10 ? (char)(nDigit + '0') : (char)(nDigit + '7');
        pOut++;
        nValue = nValue / (uint64_t)(int64_t)nRadix;
    } while (nValue != 0);
    *pOut = '\0';
    _strrev(szBuffer);
    return szBuffer;
}

// 1.14d mac: 00033c5f
char* _ultoa(DWORD dwValue, char* szBuffer, DWORD dwRadix) {
    char* pOut = szBuffer;
    do {
        int nDigit = (int)((uint64_t)dwValue % (uint64_t)dwRadix);
        *pOut = nDigit < 10 ? (char)(nDigit + '0') : (char)(nDigit + '7');
        dwValue = dwValue / dwRadix;
        pOut++;
    } while (dwValue != 0);
    *pOut = '\0';
    _strrev(szBuffer);
    return szBuffer;
}

// 1.14d mac: 00033caa
int _vsnprintf(char* szBuffer, size_t nCount, const char* szFormat, va_list pArgs) {
    return vsnprintf(szBuffer, nCount, szFormat, pArgs);
}

// 1.14d mac: 003979cc
// The linear congruential seed, the CRT's: 1 until srand.
static DWORD s_dwRandSeed = 1;

// 1.14d mac: 00033cb3
DWORD Storm_rand(void) {
    s_dwRandSeed = s_dwRandSeed * 0x343fd + 0x269ec3;
    return s_dwRandSeed >> 16 & 0x7fff;
}

// 1.14d mac: 00033cdb
void Storm_srand(DWORD dwSeed) {
    s_dwRandSeed = dwSeed;
}

#endif  // D2_OS_MAC
