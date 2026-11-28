// Storm's Win32-on-Mac layer: text measuring and drawing on a device context and the
// Mac Roman / ANSI conversions, 1.14d Intel Mac 0002b88c..0002bdea.
// The original file is unknown: no binary string names it. The run follows the resource
// lookup and precedes the thread functions (Win32_Threads.cpp).
#include "Win32Text.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include <Carbon/Carbon.h>

// QuickDraw text routines the 10.7 SDK headers no longer declare.
extern "C" {
short TextWidth(const void* pText, short nFirstByte, short nByteCount);
short VisibleLength(Ptr pText, long nLength);
void MeasureText(short nCount, const void* pText, void* pArray);
int StyledLineBreak(Ptr pText, long nLength, long nStart, long nEnd, long nFlags, Fixed* pTextWidth,
                    long* pTextOffset);
}

void* Priv_Alloc(size_t nSize);
void Priv_Free(void* pMem);

// Windows ANSI to Mac Roman, lossy ('?' or '.' where there is no counterpart).
// 1.14d mac: 003222a8
const unsigned char g_aAnsiToMacRoman[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0xDB, 0x3F, 0xE2, 0xC4, 0xE3, 0xC9, 0xA0, 0xE0, 0xF6, 0xE4, 0x2E, 0xDC, 0xCE, 0x3F, 0x2E, 0x3F,
    0x3F, 0xD4, 0xD5, 0xD2, 0xD3, 0xA5, 0xD0, 0xD1, 0xF7, 0xAA, 0x2E, 0xDD, 0xCF, 0x3F, 0x2E, 0xD9,
    0xCA, 0xC1, 0xA2, 0xA3, 0x3F, 0xB4, 0x7C, 0xA4, 0xAC, 0xA9, 0xBB, 0xC7, 0xC2, 0x2D, 0xA8, 0xF8,
    0xA1, 0xB1, 0x32, 0x33, 0xAB, 0xB5, 0xA6, 0xE1, 0xFC, 0x31, 0xBC, 0xC8, 0x2E, 0x2E, 0x2E, 0xC0,
    0xCB, 0xE7, 0xE5, 0xCC, 0x80, 0x81, 0xAE, 0x82, 0xE9, 0x83, 0xE6, 0xE8, 0xED, 0xEA, 0xEB, 0xEC,
    0x2E, 0x84, 0xF1, 0xEE, 0xEF, 0xCD, 0x85, 0x2A, 0xAF, 0xF4, 0xF2, 0xF3, 0x86, 0x2E, 0x2E, 0xA7,
    0x88, 0x87, 0x89, 0x8B, 0x8A, 0x8C, 0xBE, 0x8D, 0x8F, 0x8E, 0x90, 0x91, 0x93, 0x92, 0x94, 0x95,
    0x2E, 0x96, 0x98, 0x97, 0x99, 0x9B, 0x9A, 0xD6, 0xBF, 0x9D, 0x9C, 0x9E, 0x9F, 0x2E, 0x2E, 0xD8,
};

// 1.14d mac: 003223a8
const unsigned char g_aMacRomanToAnsi[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0xC4, 0xC5, 0xC7, 0xC9, 0xD1, 0xD6, 0xDC, 0xE1, 0xE0, 0xE2, 0xE4, 0xE3, 0xE5, 0xE7, 0xE9, 0xE8,
    0xEA, 0xEB, 0xED, 0xEC, 0xEE, 0xEF, 0xF1, 0xF3, 0xF2, 0xF4, 0xF6, 0xF5, 0xFA, 0xF9, 0xFB, 0xFC,
    0x86, 0xB0, 0xA2, 0xA3, 0xA7, 0x95, 0xB6, 0xDF, 0xAE, 0xA9, 0x99, 0xB4, 0xA8, 0x2E, 0xC6, 0xD8,
    0x3F, 0xB1, 0x2E, 0x2E, 0xA5, 0xB5, 0x3F, 0x2E, 0x3F, 0x2E, 0x3F, 0xAA, 0xBA, 0x3F, 0xE6, 0xF8,
    0xBF, 0xA1, 0xAC, 0x3F, 0x83, 0x3F, 0x3F, 0xAB, 0xBB, 0x85, 0xA0, 0xC0, 0xC3, 0xD5, 0x8C, 0x9C,
    0x96, 0x97, 0x93, 0x94, 0x91, 0x92, 0xF7, 0x3F, 0xFF, 0x9F, 0x2F, 0x80, 0x8B, 0x9B, 0x2E, 0x2E,
    0x87, 0xB7, 0x82, 0x84, 0x89, 0xC2, 0xCA, 0xC1, 0xCB, 0xC8, 0xCD, 0xCE, 0xCF, 0xCC, 0xD3, 0xD4,
    0x3F, 0xD2, 0xDA, 0xDB, 0xD9, 0x3F, 0x88, 0x98, 0xAF, 0x2E, 0x3F, 0x3F, 0xB8, 0x2E, 0x3F, 0x2E,
};

// 1.14d mac: 0002b88c
// Splits the text into lines that fit nMaxWidth, at CR, LF and where StyledLineBreak
// chooses; fills up to nMaxLines records when pLines is given. Returns the line count.
int Text_CalculateLineBreaks(HDC hDC, const char* szText, int nLength, int nMaxWidth, Win32MacTextLine* pLines,
                             int nMaxLines) {
    Win32MacSavedPort tSaved;
    GDI_SetUpMacPort(hDC, &tSaved);
    int nLineEnd = 0;
    int nLineCount = 0;
    if (0 < nLength) {
        nLineEnd = 0;
        do {
            if (szText[nLineEnd] == '\n' || szText[nLineEnd] == '\r') {
                break;
            }
            nLineEnd++;
        } while (nLineEnd < nLength);
    }
    int nLineStart = 0;
    for (;;) {
        long nBreakOffset = -1;
        int nEnd = nLength;
        if (nLineEnd <= nLength) {
            nEnd = nLineEnd;
        }
        Fixed nWidthFixed = nMaxWidth << 16;
        StyledLineBreak((Ptr)(szText + nLineStart), nEnd - nLineStart, 0, nEnd - nLineStart, 0, &nWidthFixed,
                        &nBreakOffset);
        int nTextEnd = nBreakOffset + nLineStart;
        int nNextLineEnd = nLineEnd;
        int nNextStart = nTextEnd;
        if (nLineEnd <= nTextEnd) {
            nTextEnd = nLineEnd + 1;
            nNextStart = nTextEnd;
            if (nTextEnd < nLength && szText[nLineEnd] == '\r') {
                nNextStart = nLineEnd + 2;
                nNextLineEnd = nTextEnd;
                if (szText[nLineEnd + 1] != '\n') {
                    nNextLineEnd = nLineEnd;
                    nNextStart = nTextEnd;
                }
            }
            do {
                nNextLineEnd++;
                nTextEnd = nLineEnd;
                if (nLength <= nNextLineEnd || szText[nNextLineEnd] == '\n') {
                    break;
                }
            } while (szText[nNextLineEnd] != '\r');
        }
        if (pLines != NULL && nLineCount < nMaxLines) {
            short nStart = (short)nLineStart;
            int nLineLength = (short)((short)nTextEnd - nStart);
            Win32MacTextLine* pLine = pLines + nLineCount;
            short nVisible = VisibleLength((Ptr)(szText + nLineStart), nLineLength);
            pLine->nStart = nLineStart;
            pLine->nVisibleLength = nVisible;
            pLine->nVisibleWidth = TextWidth(szText, nStart, nVisible);
            pLine->nLength = nLineLength;
            pLine->nWidth = TextWidth(szText, nStart, nLineLength);
        }
        nLineCount++;
        nLineStart = nNextStart;
        nLineEnd = nNextLineEnd;
        if (nLength <= nNextStart) {
            GDI_RestoreMacPort(&tSaved);
            return nLineCount;
        }
    }
}

// 1.14d mac: 0002ba27
// Size of the text in the device's font; optionally the extent after each character
// (pDx) and how many characters fit in nMaxExtent (pnFit).
BOOL GetTextExtentExPointA(HDC hDC, const char* szText, int nLength, int nMaxExtent, int* pnFit, int* pDx,
                           SIZE* pSize) {
    Win32MacSavedPort tSaved;
    Win32MacTextMetrics tMetrics;
    if (hDC == NULL || szText == NULL || pSize == NULL) {
        return 0;
    }
    GetTextMetrics(hDC, &tMetrics);
    pSize->cx = 0;
    pSize->cy = tMetrics.tmHeight;
    if (nLength < 1) {
        pSize->cx = 0;
        if (pnFit != NULL) {
            *pnFit = 0;
        }
        return 1;
    }
    GDI_SetUpMacPort(hDC, &tSaved);
    BOOL bResult;
    if (pDx == NULL && pnFit == NULL) {
        pSize->cx = TextWidth(szText, 0, (short)nLength);
    } else {
        short* pMeasure = (short*)Priv_Alloc(nLength * 2 + 2);
        bResult = 0;
        if (pMeasure == NULL) {
            goto done;
        }
        MeasureText((short)nLength, szText, (Ptr)pMeasure);
        int nFit = 0;
        do {
            int nChar = nFit;
            if (pnFit != NULL && nMaxExtent < pMeasure[nChar + 1]) {
                pSize->cx = pMeasure[nChar];
                nFit = nChar;
                goto fitted;
            }
            if (pDx != NULL) {
                pDx[nChar] = pMeasure[nChar];
            }
            nFit = nChar + 1;
        } while (nFit < nLength);
        pSize->cx = pMeasure[nFit];
        if (pnFit != NULL) {
        fitted:
            *pnFit = nFit;
        }
        Priv_Free(pMeasure);
    }
    bResult = 1;
done:
    GDI_RestoreMacPort(&tSaved);
    return bResult;
}

// 1.14d mac: 0002bb80
BOOL GetTextExtentPoint32(HDC hDC, const char* szText, int nLength, SIZE* pSize) {
    return GetTextExtentExPointA(hDC, szText, nLength, 0, NULL, NULL, pSize);
}

// 1.14d mac: 0002bbc3
// Draws the text at (nX, nY) placed by the device's text alignment; with TA_UPDATECP
// it starts at the current position and leaves it at the end of the text.
BOOL ExtTextOut(HDC hDC, int nX, int nY, UINT fuOptions, RECT* pRect, const char* szText, int nLength) {
    BOOL bResult = 0;
    if (hDC != NULL) {
        int nCount = nLength;
        if (szText == NULL || *szText == '\0') {
            nCount = 0;
        }
        if (pRect != NULL && (fuOptions & ETO_OPAQUE) != 0) {
            GDI_FillRectWithBkColor(hDC, pRect);
        }
        SIZE tExtent;
        GetTextExtentExPointA(hDC, szText, nCount, 0, NULL, NULL, &tExtent);
        UINT dwAlign = GetTextAlign(hDC);
        if ((dwAlign & TA_UPDATECP) != 0) {
            POINT tPosition;
            GetCurrentPositionEx(hDC, &tPosition);
            nX = tPosition.x;
            nY = tPosition.y;
        }
        RECT tBounds;
        if ((dwAlign & TA_RIGHT) == 0) {
            tBounds.left = nX;
            if ((dwAlign & TA_CENTER) != 0) {
                tBounds.left = nX - tExtent.cx / 2;
            }
        } else {
            tBounds.left = nX - tExtent.cx;
        }
        tBounds.right = tBounds.left + tExtent.cx;
        if ((dwAlign & TA_BOTTOM) == 0) {
            if ((dwAlign & TA_BASELINE) != 0) {
                Win32MacTextMetrics tMetrics;
                GetTextMetrics(hDC, &tMetrics);
                nY = nY - tMetrics.tmAscent;
            }
        } else {
            nY = nY - tExtent.cy;
        }
        tBounds.bottom = nY + tExtent.cy;
        tBounds.top = nY;
        bResult = GDI_DrawLine(hDC, &tBounds, (RECT*)((((int)(fuOptions << 29)) >> 31) & (intptr_t)pRect), szText,
                               nCount, 0, 0xFFFFFFFF, 0xFFFFFFFF);
        if ((dwAlign & TA_UPDATECP) != 0) {
            POINT tPosition;
            GetCurrentPositionEx(hDC, &tPosition);
            MoveToEx(hDC, tBounds.right, tPosition.y, NULL);
        }
    }
    return bResult;
}

// 1.14d mac: 0002bd6c
// Through the 256-entry table up to the terminator or nSize; the last byte is always 0.
void TEXT_MacRomanToAnsi(const unsigned char* szSrc, unsigned char* szDest, unsigned int nSize) {
    if (nSize != 0) {
        unsigned int nIndex = 0;
        do {
            unsigned char nChar = szSrc[nIndex];
            szDest[nIndex] = g_aMacRomanToAnsi[nChar];
            if (nChar == 0) {
                break;
            }
            nIndex++;
        } while (nIndex < nSize);
    }
    szDest[nSize - 1] = 0;
}

// 1.14d mac: 0002bdab
void TEXT_AnsiToMacRoman(const unsigned char* szSrc, unsigned char* szDest, unsigned int nSize) {
    if (nSize != 0) {
        unsigned int nIndex = 0;
        do {
            unsigned char nChar = szSrc[nIndex];
            szDest[nIndex] = g_aAnsiToMacRoman[nChar];
            if (nChar == 0) {
                break;
            }
            nIndex++;
        } while (nIndex < nSize);
    }
    szDest[nSize - 1] = 0;
}

#endif  // D2_OS_MAC
