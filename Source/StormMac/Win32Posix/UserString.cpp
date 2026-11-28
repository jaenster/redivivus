// Win32Posix: Windows-1252 to UTF-8 and back, for the text that crosses into SDL (window
// titles, message boxes, text input, the clipboard). A character 1252 lacks becomes '?'.
#include "Source/StormMac/Win32Posix/UserInternal.h"

#if !defined(_WIN32) && D2_BACKEND_POSIX

namespace Win32Posix {

// 0x80..0x9F of Windows-1252; 0 where the code page leaves the byte undefined.
static const uint16_t s_awCp1252High[32] = {
    0x20AC, 0,      0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160,
    0x2039, 0x0152, 0,      0x017D, 0,      0,      0x2018, 0x2019, 0x201C, 0x201D, 0x2022,
    0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0,      0x017E, 0x0178,
};

size_t AnsiToUtf8(const char* szAnsi, char* szOut, size_t nOut) {
    size_t n = 0;
    if (!nOut) {
        return 0;
    }
    for (const unsigned char* p = (const unsigned char*)szAnsi; *p; p++) {
        uint32_t c = *p;
        if (c >= 0x80 && c < 0xA0) {
            c = s_awCp1252High[c - 0x80] ? s_awCp1252High[c - 0x80] : '?';
        }
        char ab[3];
        size_t nLen;
        if (c < 0x80) {
            ab[0] = (char)c;
            nLen = 1;
        } else if (c < 0x800) {
            ab[0] = (char)(0xC0 | (c >> 6));
            ab[1] = (char)(0x80 | (c & 0x3F));
            nLen = 2;
        } else {
            ab[0] = (char)(0xE0 | (c >> 12));
            ab[1] = (char)(0x80 | ((c >> 6) & 0x3F));
            ab[2] = (char)(0x80 | (c & 0x3F));
            nLen = 3;
        }
        if (n + nLen >= nOut) {
            break;
        }
        for (size_t i = 0; i < nLen; i++) {
            szOut[n++] = ab[i];
        }
    }
    szOut[n] = 0;
    return n;
}

size_t Utf8ToAnsi(const char* szUtf8, char* szOut, size_t nOut) {
    size_t n = 0;
    if (!nOut) {
        return 0;
    }
    const unsigned char* p = (const unsigned char*)szUtf8;
    while (*p && n + 1 < nOut) {
        uint32_t c = *p++;
        int nMore = c >= 0xF0 ? 3 : c >= 0xE0 ? 2 : c >= 0xC0 ? 1 : 0;
        if (c >= 0x80 && nMore == 0) {
            szOut[n++] = '?';  // a stray continuation byte
            continue;
        }
        c &= nMore == 3 ? 0x07 : nMore == 2 ? 0x0F : nMore == 1 ? 0x1F : 0x7F;
        for (; nMore > 0 && (*p & 0xC0) == 0x80; nMore--) {
            c = (c << 6) | (*p++ & 0x3F);
        }
        BYTE b = '?';
        if (nMore == 0) {
            if (c < 0x80 || (c >= 0xA0 && c <= 0xFF)) {
                b = (BYTE)c;
            } else {
                for (int i = 0; i < 32; i++) {
                    if (s_awCp1252High[i] && s_awCp1252High[i] == c) {
                        b = (BYTE)(0x80 + i);
                        break;
                    }
                }
            }
        }
        szOut[n++] = (char)b;
    }
    szOut[n] = 0;
    return n;
}

}  // namespace Win32Posix

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
