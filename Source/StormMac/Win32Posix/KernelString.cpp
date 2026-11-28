// Win32Posix: wsprintf, the code page conversions, CharLowerBuff and the Microsoft CRT's string
// extras (Win32Mac's Win32_String.cpp and Win32_Text.cpp).
//
// wsprintf is Windows' own formatter, not printf: no floating point, at most 1024 characters
// out, and l is 32 bits - on an LP64 target a %ld argument is read as an int, which is what
// the shared code passes, and only I64 (or ll) reads 64 bits. %s is a string of the call's own
// width, %S (and %ls, %hs) of the other one. An unknown conversion is copied out as written.
//
// The ANSI code page is Windows-1252 (GetACP); CP_UTF8 is UTF-8. Conversions follow the SDK's
// counting: -1 counts the terminator in, a zero output size asks for the size.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

using namespace Win32Posix;

namespace {

// Windows-1252's 0x80..0x9F; the unassigned five map to themselves, as Windows maps them.
const uint16_t kCp1252High[32] = {
    0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160,
    0x2039, 0x0152, 0x008D, 0x017D, 0x008F, 0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022,
    0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178,
};

uint16_t Cp1252ToUnicode(unsigned char c) {
    return (c >= 0x80 && c < 0xA0) ? kCp1252High[c - 0x80] : c;
}

// The 1252 byte of a code point; -1 when it has none.
int UnicodeToCp1252(uint32_t u) {
    if (u < 0x80 || (u >= 0xA0 && u < 0x100)) {
        return (int)u;
    }
    for (int i = 0; i < 32; ++i) {
        if (kCp1252High[i] == u) {
            return 0x80 + i;
        }
    }
    return -1;
}

// Decodes multibyte text into UTF-16; false on an invalid UTF-8 sequence when bStrict.
bool DecodeToUtf16(UINT CodePage, const unsigned char* p, size_t n, std::u16string* pOut, bool bStrict) {
    if (CodePage != CP_UTF8) {
        for (size_t i = 0; i < n; ++i) {
            pOut->push_back((char16_t)Cp1252ToUnicode(p[i]));
        }
        return true;
    }
    for (size_t i = 0; i < n;) {
        uint32_t c = p[i];
        int nExtra = c < 0x80 ? 0 : (c & 0xE0) == 0xC0 ? 1 : (c & 0xF0) == 0xE0 ? 2 : (c & 0xF8) == 0xF0 ? 3 : -1;
        bool bValid = nExtra >= 0 && i + (size_t)nExtra < n;
        uint32_t u = nExtra > 0 ? c & (0x3Fu >> nExtra) : c;
        for (int k = 1; bValid && k <= nExtra; ++k) {
            bValid = (p[i + (size_t)k] & 0xC0) == 0x80;
            u = (u << 6) | (p[i + (size_t)k] & 0x3F);
        }
        if (bValid && ((nExtra == 1 && u < 0x80) || (nExtra == 2 && u < 0x800) || (nExtra == 3 && u < 0x10000) ||
                       u > 0x10FFFF || (u >= 0xD800 && u < 0xE000))) {
            bValid = false;  // overlong, out of range, or a surrogate
        }
        if (!bValid) {
            if (bStrict) {
                return false;
            }
            pOut->push_back(0xFFFD);
            ++i;
            continue;
        }
        if (u >= 0x10000) {
            u -= 0x10000;
            pOut->push_back((char16_t)(0xD800 + (u >> 10)));
            pOut->push_back((char16_t)(0xDC00 + (u & 0x3FF)));
        } else {
            pOut->push_back((char16_t)u);
        }
        i += (size_t)nExtra + 1;
    }
    return true;
}

void EncodeFromUtf16(UINT CodePage, const char16_t* p, size_t n, char cDefault, std::string* pOut, bool* pbUsedDefault) {
    for (size_t i = 0; i < n; ++i) {
        uint32_t u = p[i];
        if (u >= 0xD800 && u < 0xDC00 && i + 1 < n && p[i + 1] >= 0xDC00 && p[i + 1] < 0xE000) {
            u = 0x10000 + ((u - 0xD800) << 10) + (uint32_t)(p[++i] - 0xDC00);
        }
        if (CodePage != CP_UTF8) {
            int b = UnicodeToCp1252(u);
            if (b < 0) {
                b = (unsigned char)cDefault;
                *pbUsedDefault = true;
            }
            pOut->push_back((char)b);
        } else if (u < 0x80) {
            pOut->push_back((char)u);
        } else if (u < 0x800) {
            pOut->push_back((char)(0xC0 | (u >> 6)));
            pOut->push_back((char)(0x80 | (u & 0x3F)));
        } else if (u < 0x10000) {
            if (u >= 0xD800 && u < 0xE000) {
                u = 0xFFFD;  // a lone surrogate
            }
            pOut->push_back((char)(0xE0 | (u >> 12)));
            pOut->push_back((char)(0x80 | ((u >> 6) & 0x3F)));
            pOut->push_back((char)(0x80 | (u & 0x3F)));
        } else {
            pOut->push_back((char)(0xF0 | (u >> 18)));
            pOut->push_back((char)(0x80 | ((u >> 12) & 0x3F)));
            pOut->push_back((char)(0x80 | ((u >> 6) & 0x3F)));
            pOut->push_back((char)(0x80 | (u & 0x3F)));
        }
    }
}

size_t Utf16Length(const uint16_t* p) {
    size_t n = 0;
    while (p[n]) {
        ++n;
    }
    return n;
}

// ---- the wsprintf formatter, over either character width ----

template <class CharT> void AppendNarrow(std::basic_string<CharT>* pOut, const char* szText) {
    for (const char* p = szText; *p; ++p) {
        pOut->push_back((CharT)(unsigned char)*p);
    }
}

// A string argument: narrow (char) or wide (UTF-16), cut and padded as the spec says.
template <class CharT>
void PutString(std::basic_string<CharT>* pOut, const void* pArg, bool bWideArg, bool bLeft, int nWidth, int nPrecision) {
    std::basic_string<CharT> sArg;
    if (!pArg) {
        AppendNarrow(&sArg, "(null)");
    } else if (bWideArg) {
        const uint16_t* p = (const uint16_t*)pArg;
        if (sizeof(CharT) == 2) {
            for (; *p; ++p) {
                sArg.push_back((CharT)*p);
            }
        } else {
            std::string sNarrow;
            bool bUsed = false;
            EncodeFromUtf16(CP_ACP, (const char16_t*)p, Utf16Length(p), '?', &sNarrow, &bUsed);
            AppendNarrow(&sArg, sNarrow.c_str());
        }
    } else {
        for (const unsigned char* p = (const unsigned char*)pArg; *p; ++p) {
            sArg.push_back(sizeof(CharT) == 2 ? (CharT)Cp1252ToUnicode(*p) : (CharT)*p);
        }
    }
    if (nPrecision >= 0 && sArg.size() > (size_t)nPrecision) {
        sArg.resize((size_t)nPrecision);
    }
    size_t nPad = nWidth > 0 && (size_t)nWidth > sArg.size() ? (size_t)nWidth - sArg.size() : 0;
    if (!bLeft) {
        pOut->append(nPad, (CharT)' ');
    }
    *pOut += sArg;
    if (bLeft) {
        pOut->append(nPad, (CharT)' ');
    }
}

constexpr size_t kI64Length = sizeof("I64") - 1;

template <class CharT> int Format(CharT* pBuffer, const CharT* pFormat, va_list ap) {
    const bool bWideCall = sizeof(CharT) == 2;
    std::basic_string<CharT> sOut;
    const CharT* p = pFormat;
    while (*p) {
        if (*p != '%') {
            sOut.push_back(*p++);
            continue;
        }
        const CharT* pSpec = p++;
        if (*p == '%') {
            sOut.push_back(*p++);
            continue;
        }
        std::string sFlags;
        while (*p == '-' || *p == '#' || *p == '0' || *p == '+' || *p == ' ') {
            sFlags.push_back((char)*p++);
        }
        int nWidth = 0;
        while (*p >= '0' && *p <= '9') {
            nWidth = nWidth * 10 + (int)(*p++ - '0');
        }
        int nPrecision = -1;
        if (*p == '.') {
            ++p;
            nPrecision = 0;
            while (*p >= '0' && *p <= '9') {
                nPrecision = nPrecision * 10 + (int)(*p++ - '0');
            }
        }
        // the size: h (16 bits, or a narrow string), l (32 bits, or a wide string), I64 or ll
        int nSize = 0;  // 0 none, 1 h, 2 l, 3 64-bit
        if (*p == 'h') {
            nSize = 1;
            ++p;
        } else if (*p == 'l') {
            nSize = 2;
            ++p;
            if (*p == 'l') {
                nSize = 3;
                ++p;
            }
        } else if (*p == 'I' && p[1] == '6' && p[2] == '4') {
            nSize = 3;
            p += kI64Length;
        }
        CharT c = *p;
        if (c == 0) {
            break;
        }
        ++p;
        bool bLeft = sFlags.find('-') != std::string::npos;
        char szText[128];
        switch (c) {
            case 'd':
            case 'i':
            case 'u':
            case 'x':
            case 'X':
            case 'o': {
                char szSpec[48];
                char szPrecision[16] = "";
                if (nPrecision >= 0) {
                    snprintf(szPrecision, sizeof(szPrecision), ".%d", nPrecision);
                }
                snprintf(szSpec, sizeof(szSpec), "%%%s%d%s%s%c", sFlags.c_str(), nWidth, szPrecision,
                         nSize == 3 ? "ll" : "", (char)(c == 'i' ? 'd' : c));
                if (nSize == 3) {
                    snprintf(szText, sizeof(szText), szSpec, va_arg(ap, long long));
                } else {
                    int n = va_arg(ap, int);
                    if (nSize == 1) {
                        n = (c == 'd' || c == 'i') ? (int)(short)n : (int)(unsigned short)n;
                    }
                    snprintf(szText, sizeof(szText), szSpec, n);
                }
                AppendNarrow(&sOut, szText);
                break;
            }
            case 'p':
                snprintf(szText, sizeof(szText), "%0*llX", (int)(sizeof(void*) * 2),
                         (unsigned long long)(uintptr_t)va_arg(ap, void*));
                AppendNarrow(&sOut, szText);
                break;
            case 'c':
            case 'C': {
                bool bWideArg = c == 'C' ? !bWideCall : bWideCall;
                if (nSize == 1) {
                    bWideArg = false;
                } else if (nSize == 2) {
                    bWideArg = true;
                }
                unsigned nChar = (unsigned)va_arg(ap, int);
                uint16_t aWide[2] = {(uint16_t)nChar, 0};
                char aNarrow[2] = {(char)nChar, 0};
                PutString(&sOut, bWideArg ? (const void*)aWide : (const void*)aNarrow, bWideArg, bLeft, nWidth, -1);
                break;
            }
            case 's':
            case 'S': {
                bool bWideArg = c == 'S' ? !bWideCall : bWideCall;
                if (nSize == 1) {
                    bWideArg = false;
                } else if (nSize == 2) {
                    bWideArg = true;
                }
                PutString(&sOut, va_arg(ap, const void*), bWideArg, bLeft, nWidth, nPrecision);
                break;
            }
            default:  // not a wsprintf conversion: written out as it stands
                sOut.append(pSpec, (size_t)(p - pSpec));
                break;
        }
    }
    if (sOut.size() > 1023) {
        sOut.resize(1023);
    }
    memcpy(pBuffer, sOut.c_str(), (sOut.size() + 1) * sizeof(CharT));
    return (int)sOut.size();
}

}  // namespace

extern "C" int wsprintfA(LPSTR lpOut, LPCSTR lpFmt, ...) {
    va_list ap;
    va_start(ap, lpFmt);
    int n = Format<char>(lpOut, lpFmt, ap);
    va_end(ap);
    return n;
}

int wvsprintfA(LPSTR lpOutput, LPCSTR lpFmt, va_list arglist) {
    return Format<char>(lpOutput, lpFmt, arglist);
}

extern "C" int wsprintfW(LPWSTR lpOut, LPCWSTR lpFmt, ...) {
    va_list ap;
    va_start(ap, lpFmt);
    int n = Format<uint16_t>(lpOut, lpFmt, ap);
    va_end(ap);
    return n;
}

UINT GetACP(void) {
    return 1252;
}

int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr,
                        int cchWideChar) {
    if (!lpMultiByteStr || cbMultiByte == 0 || cchWideChar < 0) {
        SetError(87);
        return 0;
    }
    size_t nIn = cbMultiByte < 0 ? strlen(lpMultiByteStr) + 1 : (size_t)cbMultiByte;
    std::u16string sOut;
    if (!DecodeToUtf16(CodePage, (const unsigned char*)lpMultiByteStr, nIn, &sOut, (dwFlags & 0x8) != 0)) {
        SetError(1113);  // ERROR_NO_UNICODE_TRANSLATION
        return 0;
    }
    if (cchWideChar == 0) {
        return (int)sOut.size();
    }
    if (sOut.size() > (size_t)cchWideChar) {
        SetError(122);
        return 0;
    }
    memcpy(lpWideCharStr, sOut.data(), sOut.size() * sizeof(WCHAR));
    return (int)sOut.size();
}

int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr,
                        int cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar) {
    (void)dwFlags;
    if (!lpWideCharStr || cchWideChar == 0 || cbMultiByte < 0) {
        SetError(87);
        return 0;
    }
    size_t nIn = cchWideChar < 0 ? Utf16Length(lpWideCharStr) + 1 : (size_t)cchWideChar;
    std::string sOut;
    bool bUsedDefault = false;
    EncodeFromUtf16(CodePage, (const char16_t*)lpWideCharStr, nIn, lpDefaultChar ? *lpDefaultChar : '?', &sOut,
                    &bUsedDefault);
    if (lpUsedDefaultChar) {
        *lpUsedDefaultChar = bUsedDefault;
    }
    if (cbMultiByte == 0) {
        return (int)sOut.size();
    }
    if (sOut.size() > (size_t)cbMultiByte) {
        SetError(122);
        return 0;
    }
    memcpy(lpMultiByteStr, sOut.data(), sOut.size());
    return (int)sOut.size();
}

// The 1252 lower case: ASCII, the Latin-1 capitals, and the four capitals 1252 adds at 0x8A,
// 0x8C, 0x8E and 0x9F.
DWORD CharLowerBuffA(LPSTR lpsz, DWORD cchLength) {
    for (DWORD i = 0; i < cchLength; ++i) {
        unsigned char c = (unsigned char)lpsz[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 0xC0 && c <= 0xDE && c != 0xD7)) {
            c = (unsigned char)(c + 0x20);
        } else if (c == 0x8A || c == 0x8C || c == 0x8E) {
            c = (unsigned char)(c + 0x10);
        } else if (c == 0x9F) {
            c = 0xFF;
        }
        lpsz[i] = (char)c;
    }
    return cchLength;
}

// ---- the CRT's extras (the "C" locale: ASCII case only) ----

namespace {

char* UnsignedToText(unsigned long long nValue, char* szBuffer, int nRadix, bool bNegative) {
    if (nRadix < 2 || nRadix > 36) {
        szBuffer[0] = '\0';
        return szBuffer;
    }
    char aDigits[72];
    int n = 0;
    do {
        int nDigit = (int)(nValue % (unsigned)nRadix);
        aDigits[n++] = (char)(nDigit < 10 ? '0' + nDigit : 'a' + nDigit - 10);
        nValue /= (unsigned)nRadix;
    } while (nValue);
    char* pOut = szBuffer;
    if (bNegative) {
        *pOut++ = '-';
    }
    while (n) {
        *pOut++ = aDigits[--n];
    }
    *pOut = '\0';
    return szBuffer;
}

}  // namespace

// A negative value is signed in radix 10 only; any other radix writes its two's complement.
char* _itoa(int nValue, char* szBuffer, int nRadix) {
    if (nRadix == 10 && nValue < 0) {
        return UnsignedToText(0ULL - (unsigned long long)(long long)nValue, szBuffer, nRadix, true);
    }
    return UnsignedToText((unsigned)nValue, szBuffer, nRadix, false);
}

// unsigned long is 64 bits here: the value is the 32-bit Windows one.
char* _ultoa(unsigned long nValue, char* szBuffer, int nRadix) {
    return UnsignedToText((uint32_t)nValue, szBuffer, nRadix, false);
}

char* _i64toa(long long nValue, char* szBuffer, int nRadix) {
    if (nRadix == 10 && nValue < 0) {
        return UnsignedToText(0ULL - (unsigned long long)nValue, szBuffer, nRadix, true);
    }
    return UnsignedToText((unsigned long long)nValue, szBuffer, nRadix, false);
}

char* _ui64toa(unsigned long long nValue, char* szBuffer, int nRadix) {
    return UnsignedToText(nValue, szBuffer, nRadix, false);
}

char* _strlwr(char* szString) {
    for (char* p = szString; *p; ++p) {
        if (*p >= 'A' && *p <= 'Z') {
            *p = (char)(*p + 0x20);
        }
    }
    return szString;
}

char* _strupr(char* szString) {
    for (char* p = szString; *p; ++p) {
        if (*p >= 'a' && *p <= 'z') {
            *p = (char)(*p - 0x20);
        }
    }
    return szString;
}

extern "C" char* _strrev(char* szString) {
    size_t n = strlen(szString);
    for (size_t i = 0; i < n / 2; ++i) {
        char c = szString[i];
        szString[i] = szString[n - 1 - i];
        szString[n - 1 - i] = c;
    }
    return szString;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
