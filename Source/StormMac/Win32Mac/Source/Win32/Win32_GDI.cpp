// Storm's Win32-on-Mac layer: the GDI emulation, 1.14d Intel Mac 00022ca8..00025660
// (GDI_NewHDC .. GDI_AddDirtyRect, with the cursor entry points that sit inside it).
// The original file is unknown: no 1.14d string or initialiser names it.
//
// A device context, pen, brush, font, palette, bitmap or region is a Memory Manager
// handle (Mem_HandleAlloc) tagged with a four-character type; the data behind it
// starts with a flag a stock object keeps set so DeleteObject leaves it alone.
// Drawing goes to a byte surface (Win32MacDCData::pPixels); text is rendered with QuickDraw
// into a monochrome scratch GWorld and expanded into it. Every change to the surface is
// added to one global dirty region.
#include "Win32GDI.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <string.h>

// 1.14d mac: 003f0e80
// The device contexts in existence, newest first.
static HGDIOBJ g_hDCList;
// 1.14d mac: 003f0e84
HFONT g_hStockFont;
// 1.14d mac: 003f0e88
int g_nSurfaceWidth;
// 1.14d mac: 003f0e8c
int g_nSurfaceHeight;
// 1.14d mac: 003f0e90
uint16_t g_nFontArial;
// 1.14d mac: 003f0e92
uint16_t g_nFontVerdana;
// 1.14d mac: 003f0e94
uint16_t g_nFontTimes;
// 1.14d mac: 003f0e98
static int g_hEmptyStripRgn;
// 1.14d mac: 003f0e9c
HPEN g_hBlackPen;
// 1.14d mac: 003f0ea0
HPEN g_hWhitePen;
// 1.14d mac: 003f0ea4
HPEN g_hNullPen;
// 1.14d mac: 003f0ea8
HBRUSH g_hWhiteBrush;
// 1.14d mac: 003f0eac
HBRUSH g_hLtGrayBrush;
// 1.14d mac: 003f0eb0
HBRUSH g_hBlackBrush;
// 1.14d mac: 003f0eb4
HBRUSH g_hNullBrush;
// 1.14d mac: 003f0eb8
HPALETTE g_hStockPalette;
// 1.14d mac: 003f0ebc
GWorldPtr g_pScratchGWorld;
// 1.14d mac: 003f0ec0
// Swaps black and white in SetTextColor.
int g_bSwapBlackWhite;
// 1.14d mac: 003f0ec8
Rect g_rcScratchGWorld;
// 1.14d mac: 003f0ed0
PixMapHandle g_hScratchPixMap;
// 1.14d mac: 003f0ed4
// The area changed since the screen was last updated.
HRGN g_hDirtyRgn;
// 1.14d mac: 003f0ed8
// Device contexts GetDC has handed out and ReleaseDC has not taken back.
int g_nOutstandingDCs;

// The 20 system palette colours, 0x00BBGGRR; entries 0..9 are palette indices 0..9, the
// rest 246..255.
// 1.14d mac: 00321da0
static const DWORD g_adwSystemPalette[20] = {
    0x00000000, 0x00000080, 0x00008000, 0x00008080, 0x00800000, 0x00800080, 0x00808000,
    0x00C0C0C0, 0x00C0DCC0, 0x00F0CAA6, 0x00F0FBFF, 0x00A4A0A0, 0x00808080, 0x000000FF,
    0x0000FF00, 0x0000FFFF, 0x00FF0000, 0x00FF00FF, 0x00FFFF00, 0x00FFFFFF,
};

// The palette entry each COLOR_* index of GetSysColor uses.
// 1.14d mac: 0039727c
static const BYTE g_abSysColorEntry[28] = {
    0x13, 0x00, 0x13, 0x13, 0x13, 0x13, 0x00, 0x00, 0x00, 0x00, 0x13, 0x13, 0x13, 0x04,
    0x13, 0x13, 0x00, 0x0C, 0x00, 0x00, 0x13, 0x00, 0x13, 0x00, 0x13, 0x00, 0x00, 0x00,
};

// GDI_PickMacFont's tables, indexed by the font height + 21 (heights -21 .. -9).
// 1.14d mac: 00321df0
static const int g_anFontSize[13] = {19, 0, 0, 0, 15, 15, 0, 0, 13, 0, 0, 10, 9};
// 1.14d mac: 00321e30
static const int g_anFontAscent[13] = {5, 0, 0, 0, 4, 3, 0, 0, 3, 0, 0, 3, 3};
// 1.14d mac: 00321e70
static const int g_anFontAveWidth[13] = {1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0};

// Declared fastcall in the source (Storm's Windows convention), not made fastcc by the
// optimiser: GDI_PickMacFont takes its arguments in ecx/edx yet its one call site
// (GDI_InitStockObjects) does not inline it, which a static single-use callee would be.
#define D2_FASTCALL __attribute__((fastcall))

// What GetRegionData fills: the RGNDATA header, the bounds and the rectangles.
struct Win32MacRgnData {
    DWORD dwSize;                 // 0x00  0x20
    DWORD iType;                  // 0x04  1
    DWORD nCount;                 // 0x08
    DWORD nRgnSize;               // 0x0C
    RECT rcBound;                 // 0x10
    RECT aRects[1];               // 0x20
};

static inline HandleHeader** DCHandle(HDC hDC) {
    return (HandleHeader**)hDC;
}

static inline Win32MacDCData* LockDC(HDC hDC) {
    return (Win32MacDCData*)Mem_HandleLock(GDI_TYPE_DC, DCHandle(hDC));
}

static inline void UnlockDC(HDC hDC) {
    Mem_HandleUnLock(GDI_TYPE_DC, DCHandle(hDC));
}

// A region handle's data is the strip region itself.
static inline int* LockRgn(HRGN hRgn) {
    return (int*)Mem_HandleLock(GDI_TYPE_RGN, hRgn);
}

static inline void UnlockRgn(HRGN hRgn) {
    Mem_HandleUnLock(GDI_TYPE_RGN, hRgn);
}

HRGN CreateRectRgn(int nLeft, int nTop, int nRight, int nBottom);
BOOL DeleteObject(HGDIOBJ hObject);
BOOL SetRectRgn(HRGN hRgn, int nLeft, int nTop, int nRight, int nBottom);
int CombineRgn(HRGN hrgnDest, HRGN hrgnSrc1, HRGN hrgnSrc2, int nMode);
BOOL GDI_DeleteHDC(HDC hDC);
UINT GetRegionData(HRGN hRgn, UINT nBufSize, Win32MacRgnData* pRgnData);
int ExtSelectClipRgn(HDC hDC, HRGN hRgn, int nMode);
DWORD SetTextColor(HDC hDC, DWORD crColor);
DWORD SetBkColor(HDC hDC, DWORD crColor);
static void GDI_AddDirtyRect(const RECT* pRect);
int GetRgnBox(HRGN hRgn, RECT* pRect);
static BOOL GDI_FillRect(Win32MacDCData* pDC, const RECT* pRect, BYTE nColor);
static BOOL GDI_IsRgnEmpty(HRGN hRgn);
static UINT GDI_GetObjectType(HGDIOBJ hObject);
static BOOL GDI_DeletePalette(HPALETTE hPalette);
static BOOL GDI_DeleteFont(HFONT hFont);
static BOOL GDI_DeleteBrush(HBRUSH hBrush);
BOOL GDI_ColorToPaletteIndex(DWORD dwColor, BYTE* pIndexOut);

// 1.14d mac: 00022ca8
// A new device context for the window, on the global list; freed again when its clip
// region cannot be made.
HDC GDI_NewHDC(void* hWnd) {
    HandleHeader** hHandle = Mem_HandleAlloc(GDI_TYPE_DC, 0x48, NULL);
    HGDIOBJ hHead = g_hDCList;
    if (hHandle != NULL) {
        Win32MacDCData* pDC = (Win32MacDCData*)Mem_HandleLock(GDI_TYPE_DC, hHandle);
        pDC->hNext = g_hDCList;
        pDC->hPen = NULL;
        pDC->hBrush = NULL;
        pDC->hFont = g_hStockFont;
        pDC->hClipRgn = CreateRectRgn(0, 0, 0, 0);
        pDC->hBitmap = NULL;
        pDC->hPalette = NULL;
        pDC->nBkMode = 2;
        pDC->crBack = 0;
        pDC->crText = 0xFFFFFFFF;
        pDC->nTextAlign = 0;
        pDC->nCurrentX = 0;
        pDC->nCurrentY = 0;
        pDC->hWnd = hWnd;
        Mem_HandleUnLock(GDI_TYPE_DC, hHandle);
        hHead = hHandle;
        if (pDC->hClipRgn == NULL) {
            Mem_HandleFree(GDI_TYPE_DC, hHandle);
        }
    }
    g_hDCList = hHead;
    return (HDC)hHandle;
}

// 1.14d mac: 00022da9
HRGN CreateRectRgn(int nLeft, int nTop, int nRight, int nBottom) {
    HRGN hRgn = Mem_HandleAlloc(GDI_TYPE_RGN, 4, NULL);
    int* ppRgn = LockRgn(hRgn);
    if (ppRgn != NULL) {
        *ppRgn = NewEmptyStripRgn();
    }
    UnlockRgn(hRgn);
    if (hRgn != NULL) {
        SetRectRgn(hRgn, nLeft, nTop, nRight, nBottom);
    }
    return hRgn;
}

// 1.14d mac: 00022f19
static HGDIOBJ GDI_GetNextHDC(HDC hDC) {
    if (hDC == NULL) {
        return NULL;
    }
    HGDIOBJ hNext = LockDC(hDC)->hNext;
    UnlockDC(hDC);
    return hNext;
}

// 1.14d mac: 00022e2d
// Unlinks the context from the global list, drops what it has selected and frees it.
BOOL GDI_DeleteHDC(HDC hDC) {
    if (hDC == NULL) {
        return 0;
    }
    if ((HGDIOBJ)hDC == g_hDCList) {
        g_hDCList = GDI_GetNextHDC((HDC)g_hDCList == hDC ? hDC : hDC);
    } else {
        HGDIOBJ hCur = g_hDCList;
        HGDIOBJ hPrev = NULL;
        if (hCur != NULL) {
            do {
                hPrev = hCur;
                hCur = GDI_GetNextHDC((HDC)hPrev);
                if (hCur == (HGDIOBJ)hDC) {
                    break;
                }
            } while (hCur != NULL);
            if (hCur == (HGDIOBJ)hDC && hPrev != NULL) {
                Win32MacDCData* pPrev = LockDC((HDC)hPrev);
                pPrev->hNext = GDI_GetNextHDC(hDC);
                UnlockDC((HDC)hPrev);
            }
        }
    }
    Win32MacDCData* pDC = LockDC(hDC);
    HRGN hClipRgn = pDC->hClipRgn;
    pDC->hBrush = NULL;
    pDC->hPen = NULL;
    pDC->hClipRgn = NULL;
    pDC->hFont = NULL;
    pDC->hPalette = NULL;
    pDC->hBitmap = NULL;
    UnlockDC(hDC);
    if (hClipRgn != NULL) {
        DeleteObject(hClipRgn);
    }
    return Mem_HandleFree(GDI_TYPE_DC, DCHandle(hDC));
}

// 1.14d mac: 00022f56
// Refuses while a device context still has the object selected; stock objects are not
// freed.
BOOL DeleteObject(HGDIOBJ hObject) {
    BOOL bResult = 0;
    if (hObject != NULL) {
        UINT nType = GDI_GetObjectType(hObject);
        for (HGDIOBJ hDC = g_hDCList; hDC != NULL; hDC = GDI_GetNextHDC((HDC)hDC)) {
            Win32MacDCData* pDC = (Win32MacDCData*)Mem_HandleLock(GDI_TYPE_DC, hDC);
            bool bSelected;
            switch (nType) {
            case OBJ_PEN:
                bSelected = pDC->hPen == hObject;
                break;
            case OBJ_BRUSH:
                bSelected = pDC->hBrush == hObject;
                break;
            default:
                Mem_HandleUnLock(GDI_TYPE_DC, hDC);
                continue;
            case OBJ_PAL:
                bSelected = pDC->hPalette == hObject;
                break;
            case OBJ_FONT:
                bSelected = pDC->hFont == hObject;
                break;
            case OBJ_REGION:
                bSelected = pDC->hClipRgn == hObject;
            }
            Mem_HandleUnLock(GDI_TYPE_DC, hDC);
            if (bSelected) {
                return 0;
            }
        }
        switch (nType) {
        case OBJ_PEN: {
            LONG bStock = *(LONG*)Mem_HandleLock(GDI_TYPE_PEN, hObject);
            Mem_HandleUnLock(GDI_TYPE_PEN, hObject);
            if (bStock == 0) {
                return Mem_HandleFree(GDI_TYPE_PEN, hObject);
            }
            break;
        }
        case OBJ_BRUSH:
            return GDI_DeleteBrush(hObject);
        case OBJ_PAL:
            return GDI_DeletePalette(hObject);
        case OBJ_FONT:
            return GDI_DeleteFont(hObject);
        case OBJ_BITMAP: {
            LONG bStock = *(LONG*)Mem_HandleLock(GDI_TYPE_BITMAP, hObject);
            Mem_HandleUnLock(GDI_TYPE_BITMAP, hObject);
            if (bStock == 0) {
                return Mem_HandleFree(GDI_TYPE_BITMAP, hObject);
            }
            break;
        }
        case OBJ_REGION: {
            int pRgn = *LockRgn(hObject);
            UnlockRgn(hObject);
            bResult = Mem_HandleFree(GDI_TYPE_RGN, hObject);
            if (bResult != 0 && pRgn != NULL) {
                DisposeStripRgn(pRgn);
            }
        }
        }
    }
    return bResult;
}

// A stock object's handle: one object of the type, its data initialised with a template.
// The template is the first 0x14 bytes of a local buffer; Mem_HandleAlloc copies the whole
// object size from it, so a font and the palette start with whatever follows.
struct Win32MacStockInit {
    LONG bStock;
    BYTE nColor;
    BYTE pad05[3];
    LONG bNull;
    LONG n0C;
    LONG n10;
};

// 1.14d mac: 00023785
// GetFNum by name; the number is 0 when the name is not a font on this system.
static void GDI_GetFontNumByName(const unsigned char* pFontName, uint16_t* pFontNumOut) {
    short nFontNum;
    Str255 szName;
    GetFNum(pFontName, &nFontNum);
    if (nFontNum == 0) {
        GetFontName(0, szName);
        EqualString(szName, pFontName, false, false);
    }
    *pFontNumOut = nFontNum;
}

// 1.14d mac: 00023821
// Fills a font record from a Windows face name and height: Times New Roman, Arial and
// Verdana map to the Mac font numbers found at start-up, with a size and style by height.
void D2_FASTCALL GDI_PickMacFont(Win32MacFont* pFont, const char* szFace, int nHeight) {
    uint16_t nFontNum;
    int nSize;
    int nAscent;
    int nAveWidth;
    if (_stricmp("Times New Roman", szFace) == 0) {
        nFontNum = g_nFontTimes;
        nSize = nFontNum;
        if (nHeight == -0x11 || nHeight == -0x10) {
            nAveWidth = 1;
            nAscent = 4;
            nSize = 0xF;
        } else if (nHeight == -10) {
            nAveWidth = 0;
            nAscent = 2;
            nSize = 10;
        }
    } else if (_stricmp("Arial", szFace) == 0 || _stricmp("Verdana", szFace) == 0) {
        nFontNum = g_nFontArial;
        nSize = nFontNum;
        if ((unsigned)(nHeight + 0x15) < 0xD) {
            nAveWidth = g_anFontAveWidth[nHeight + 0x15];
            nAscent = g_anFontAscent[nHeight + 0x15];
            nSize = g_anFontSize[nHeight + 0x15];
        }
    } else {
        nSize = 0;
        nFontNum = 0;
    }
    pFont->nSizeNeg = -nHeight;
    pFont->nFontNum = nFontNum;
    pFont->nAscent = nSize;
    pFont->nDescent = nAscent;
    pFont->nAveWidth = nAveWidth;
    pFont->nHeight = nHeight;
    strncpy(pFont->szFace, szFace, 0x1F);
    pFont->cTerminator = 0;
}

// 1.14d mac: 00023150
// The stock pens, brushes, font and palette and the scratch GWorld.
void GDI_InitStockObjects(void) {
    DWORD aInit[0x102];
    GWorldPtr pSavedPort;
    GDHandle hSavedDevice;
    g_nSurfaceWidth = 0x7FFF;
    g_nSurfaceHeight = 0x7FFF;
    GDI_GetFontNumByName((const unsigned char*)"\x06" "Geneva", &g_nFontArial);
    GDI_GetFontNumByName((const unsigned char*)"\x06" "Geneva", &g_nFontVerdana);
    GDI_GetFontNumByName((const unsigned char*)"\x05" "Times", &g_nFontTimes);
    g_hEmptyStripRgn = NewEmptyStripRgn();
    Win32MacStockInit* pInit = (Win32MacStockInit*)aInit;

    pInit->bStock = 0;
    pInit->nColor = 0;
    pInit->bNull = 0;
    pInit->n0C = 1;
    pInit->n10 = 0;
    g_hBlackPen = Mem_HandleAlloc(GDI_TYPE_PEN, 0x14, pInit);
    if (g_hBlackPen != NULL) {
        Win32MacPen* pPen = (Win32MacPen*)Mem_HandleLock(GDI_TYPE_PEN, g_hBlackPen);
        pPen->bStock = 1;
        pPen->nColor = 0;
        Mem_HandleUnLock(GDI_TYPE_PEN, g_hBlackPen);
    }
    pInit->bStock = 0;
    pInit->nColor = 0;
    pInit->bNull = 0;
    pInit->n0C = 1;
    pInit->n10 = 0;
    g_hWhitePen = Mem_HandleAlloc(GDI_TYPE_PEN, 0x14, pInit);
    if (g_hWhitePen != NULL) {
        Win32MacPen* pPen = (Win32MacPen*)Mem_HandleLock(GDI_TYPE_PEN, g_hWhitePen);
        pPen->bStock = 1;
        pPen->nColor = 0xFF;
        Mem_HandleUnLock(GDI_TYPE_PEN, g_hWhitePen);
    }
    pInit->bStock = 0;
    pInit->nColor = 0;
    pInit->bNull = 0;
    pInit->n0C = 1;
    pInit->n10 = 0;
    g_hNullPen = Mem_HandleAlloc(GDI_TYPE_PEN, 0x14, pInit);
    if (g_hNullPen != NULL) {
        Win32MacPen* pPen = (Win32MacPen*)Mem_HandleLock(GDI_TYPE_PEN, g_hNullPen);
        pPen->bStock = 1;
        pPen->bNull = 1;
        Mem_HandleUnLock(GDI_TYPE_PEN, g_hNullPen);
    }

    pInit->bStock = 0;
    pInit->nColor = 0xFF;
    pInit->bNull = 0;
    pInit->n0C = 1;
    pInit->n10 = 0;
    g_hWhiteBrush = Mem_HandleAlloc(GDI_TYPE_BRUSH, 0x14, pInit);
    if (g_hWhiteBrush != NULL) {
        Win32MacBrush* pBrush = (Win32MacBrush*)Mem_HandleLock(GDI_TYPE_BRUSH, g_hWhiteBrush);
        pBrush->bStock = 1;
        pBrush->nColor = 0xFF;
        Mem_HandleUnLock(GDI_TYPE_BRUSH, g_hWhiteBrush);
    }
    pInit->bStock = 0;
    pInit->nColor = 0xFF;
    pInit->bNull = 0;
    pInit->n0C = 1;
    pInit->n10 = 0;
    g_hLtGrayBrush = Mem_HandleAlloc(GDI_TYPE_BRUSH, 0x14, pInit);
    if (g_hLtGrayBrush != NULL) {
        Win32MacBrush* pBrush = (Win32MacBrush*)Mem_HandleLock(GDI_TYPE_BRUSH, g_hLtGrayBrush);
        pBrush->bStock = 1;
        pBrush->nColor = 1;
        Mem_HandleUnLock(GDI_TYPE_BRUSH, g_hLtGrayBrush);
    }
    pInit->bStock = 0;
    pInit->nColor = 0xFF;
    pInit->bNull = 0;
    pInit->n0C = 1;
    pInit->n10 = 0;
    g_hBlackBrush = Mem_HandleAlloc(GDI_TYPE_BRUSH, 0x14, pInit);
    if (g_hBlackBrush != NULL) {
        Win32MacBrush* pBrush = (Win32MacBrush*)Mem_HandleLock(GDI_TYPE_BRUSH, g_hBlackBrush);
        pBrush->bStock = 1;
        pBrush->nColor = 0;
        Mem_HandleUnLock(GDI_TYPE_BRUSH, g_hBlackBrush);
    }
    pInit->bStock = 0;
    pInit->nColor = 0xFF;
    pInit->bNull = 0;
    pInit->n0C = 1;
    pInit->n10 = 0;
    g_hNullBrush = Mem_HandleAlloc(GDI_TYPE_BRUSH, 0x14, pInit);
    if (g_hNullBrush != NULL) {
        Win32MacBrush* pBrush = (Win32MacBrush*)Mem_HandleLock(GDI_TYPE_BRUSH, g_hNullBrush);
        pBrush->bStock = 1;
        pBrush->bNull = 1;
        Mem_HandleUnLock(GDI_TYPE_BRUSH, g_hNullBrush);
    }

    pInit->bStock = 0;
    g_hStockFont = Mem_HandleAlloc(GDI_TYPE_FONT, 0x3C, pInit);
    if (g_hStockFont != NULL) {
        Win32MacFont* pFont = (Win32MacFont*)Mem_HandleLock(GDI_TYPE_FONT, g_hStockFont);
        GDI_PickMacFont(pFont, "Arial", -0x11);
        pFont->nSize = 1;
        pFont->nStyle = 0;
        pFont->bStock = 1;
        Mem_HandleUnLock(GDI_TYPE_FONT, g_hStockFont);
    }

    pInit->bStock = 0;
    g_hStockPalette = Mem_HandleAlloc(GDI_TYPE_PALETTE, 0x408, pInit);
    if (g_hStockPalette != NULL) {
        Win32MacPalette* pPalette = (Win32MacPalette*)Mem_HandleLock(GDI_TYPE_PALETTE, g_hStockPalette);
        pPalette->bStock = 1;
        pPalette->nEntries = 0x14;
        for (DWORD i = 0; i < (DWORD)pPalette->nEntries; i++) {
            pPalette->aEntries[i] = g_adwSystemPalette[i];
        }
        Mem_HandleUnLock(GDI_TYPE_PALETTE, g_hStockPalette);
    }

    GetGWorld(&pSavedPort, &hSavedDevice);
    g_rcScratchGWorld.left = 0;
    g_rcScratchGWorld.top = 0;
    g_rcScratchGWorld.right = 0x40;
    g_rcScratchGWorld.bottom = (short)g_nSurfaceWidth;
    g_pScratchGWorld = NULL;
    if (NewGWorld(&g_pScratchGWorld, 1, &g_rcScratchGWorld, NULL, NULL, 8) == noErr) {
        g_hScratchPixMap = GetGWorldPixMap(g_pScratchGWorld);
        if (LockPixels(g_hScratchPixMap)) {
            SetGWorld(g_pScratchGWorld, NULL);
            EraseRect(&g_rcScratchGWorld);
            goto done;
        }
    }
    if (g_pScratchGWorld != NULL) {
        DisposeGWorld(g_pScratchGWorld);
        g_pScratchGWorld = NULL;
        g_hScratchPixMap = NULL;
        SetRect(&g_rcScratchGWorld, 0, 0, 0, 0);
    }
done:
    SetGWorld(pSavedPort, hSavedDevice);
}

// 1.14d mac: 0002393c
// Releases the scratch GWorld and the stock objects (their stock flags cleared first so
// they can be freed), then shuts the window system down.
void ShutdownWin32GDI(void) {
    if (g_pScratchGWorld != NULL) {
        DisposeGWorld(g_pScratchGWorld);
        g_pScratchGWorld = NULL;
    }
    if (g_hStockPalette != NULL) {
        *(LONG*)Mem_HandleLock(GDI_TYPE_PALETTE, g_hStockPalette) = 0;
        Mem_HandleUnLock(GDI_TYPE_PALETTE, g_hStockPalette);
        GDI_DeletePalette(g_hStockPalette);
        g_hStockPalette = NULL;
    }
    if (g_hStockFont != NULL) {
        *(LONG*)Mem_HandleLock(GDI_TYPE_FONT, g_hStockFont) = 0;
        Mem_HandleUnLock(GDI_TYPE_FONT, g_hStockFont);
        GDI_DeleteFont(g_hStockFont);
        g_hStockFont = NULL;
    }
    if (g_hEmptyStripRgn != NULL) {
        DisposeStripRgn(g_hEmptyStripRgn);
        g_hEmptyStripRgn = NULL;
    }
    if (g_hBlackPen != NULL) {
        *(LONG*)Mem_HandleLock(GDI_TYPE_PEN, g_hBlackPen) = 0;
        Mem_HandleUnLock(GDI_TYPE_PEN, g_hBlackPen);
        HPEN hPen = g_hBlackPen;
        if (hPen != NULL) {
            LONG bStock = *(LONG*)Mem_HandleLock(GDI_TYPE_PEN, hPen);
            Mem_HandleUnLock(GDI_TYPE_PEN, hPen);
            if (bStock == 0) {
                Mem_HandleFree(GDI_TYPE_PEN, hPen);
            }
        }
        g_hBlackPen = NULL;
    }
    if (g_hWhitePen != NULL) {
        *(LONG*)Mem_HandleLock(GDI_TYPE_PEN, g_hWhitePen) = 0;
        Mem_HandleUnLock(GDI_TYPE_PEN, g_hWhitePen);
        HPEN hPen = g_hWhitePen;
        if (hPen != NULL) {
            LONG bStock = *(LONG*)Mem_HandleLock(GDI_TYPE_PEN, hPen);
            Mem_HandleUnLock(GDI_TYPE_PEN, hPen);
            if (bStock == 0) {
                Mem_HandleFree(GDI_TYPE_PEN, hPen);
            }
        }
        g_hWhitePen = NULL;
    }
    if (g_hNullPen != NULL) {
        *(LONG*)Mem_HandleLock(GDI_TYPE_PEN, g_hNullPen) = 0;
        Mem_HandleUnLock(GDI_TYPE_PEN, g_hNullPen);
        HPEN hPen = g_hNullPen;
        if (hPen != NULL) {
            LONG bStock = *(LONG*)Mem_HandleLock(GDI_TYPE_PEN, hPen);
            Mem_HandleUnLock(GDI_TYPE_PEN, hPen);
            if (bStock == 0) {
                Mem_HandleFree(GDI_TYPE_PEN, hPen);
            }
        }
        g_hNullPen = NULL;
    }
    if (g_hWhiteBrush != NULL) {
        *(LONG*)Mem_HandleLock(GDI_TYPE_BRUSH, g_hWhiteBrush) = 0;
        Mem_HandleUnLock(GDI_TYPE_BRUSH, g_hWhiteBrush);
        GDI_DeleteBrush(g_hWhiteBrush);
        g_hWhiteBrush = NULL;
    }
    if (g_hLtGrayBrush != NULL) {
        *(LONG*)Mem_HandleLock(GDI_TYPE_BRUSH, g_hLtGrayBrush) = 0;
        Mem_HandleUnLock(GDI_TYPE_BRUSH, g_hLtGrayBrush);
        GDI_DeleteBrush(g_hLtGrayBrush);
        g_hLtGrayBrush = NULL;
    }
    if (g_hBlackBrush != NULL) {
        *(LONG*)Mem_HandleLock(GDI_TYPE_BRUSH, g_hBlackBrush) = 0;
        Mem_HandleUnLock(GDI_TYPE_BRUSH, g_hBlackBrush);
        GDI_DeleteBrush(g_hBlackBrush);
        g_hBlackBrush = NULL;
    }
    if (g_hNullBrush != NULL) {
        *(LONG*)Mem_HandleLock(GDI_TYPE_BRUSH, g_hNullBrush) = 0;
        Mem_HandleUnLock(GDI_TYPE_BRUSH, g_hNullBrush);
        GDI_DeleteBrush(g_hNullBrush);
        g_hNullBrush = NULL;
    }
    Wnd_Shutdown();
}

// 1.14d mac: 00023cd4
// Frees the palette unless it is a stock object.
static BOOL GDI_DeletePalette(HPALETTE hPalette) {
    if (hPalette != NULL) {
        LONG bStockObject = *(LONG*)Mem_HandleLock(GDI_TYPE_PALETTE, hPalette);
        Mem_HandleUnLock(GDI_TYPE_PALETTE, hPalette);
        if (bStockObject == 0) {
            return Mem_HandleFree(GDI_TYPE_PALETTE, hPalette);
        }
    }
    return 0;
}

// 1.14d mac: 00023d24
static BOOL GDI_DeleteFont(HFONT hFont) {
    if (hFont != NULL) {
        LONG bStockObject = *(LONG*)Mem_HandleLock(GDI_TYPE_FONT, hFont);
        Mem_HandleUnLock(GDI_TYPE_FONT, hFont);
        if (bStockObject == 0) {
            return Mem_HandleFree(GDI_TYPE_FONT, hFont);
        }
    }
    return 0;
}

// 1.14d mac: 00023d74
static BOOL GDI_DeleteBrush(HBRUSH hBrush) {
    if (hBrush != NULL) {
        LONG bStockObject = *(LONG*)Mem_HandleLock(GDI_TYPE_BRUSH, hBrush);
        Mem_HandleUnLock(GDI_TYPE_BRUSH, hBrush);
        if (bStockObject == 0) {
            return Mem_HandleFree(GDI_TYPE_BRUSH, hBrush);
        }
    }
    return 0;
}

// 1.14d mac: 00023dc6
// The stock object of a GetStockObject index (the binary names this GetSystemMetrics;
// the real one is 00024c4f), 0 for an index it has none for.
HGDIOBJ GetStockObject(int nIndex) {
    HGDIOBJ hObject = NULL;
    switch (nIndex) {
    case 0:
        hObject = g_hWhiteBrush;
        break;
    case 1:
        hObject = g_hLtGrayBrush;
        break;
    case 4:
        hObject = g_hBlackBrush;
        break;
    case 5:
        hObject = g_hNullBrush;
        break;
    case 6:
        hObject = g_hWhitePen;
        break;
    case 7:
        hObject = g_hBlackPen;
        break;
    case 8:
        hObject = g_hNullPen;
        break;
    case 0xD:
        hObject = g_hStockFont;
        break;
    case 0xF:
        hObject = g_hStockPalette;
    }
    return hObject;
}

// 1.14d mac: 00023e6c
// Copies the DC's clip region into hRgn: 1 when it is not empty, 0 when it is (or the
// DC has none), -1 for bad arguments.
int GetClipRgn(HDC hDC, HRGN hRgn) {
    int nResult = -1;
    if (hDC != NULL && hRgn != NULL) {
        Win32MacDCData* pDC = LockDC(hDC);
        if (pDC->hClipRgn == NULL) {
            SetRectRgn(hRgn, 0, 0, 0, 0);
            nResult = 0;
        } else {
            CombineRgn(hRgn, pDC->hClipRgn, NULL, 5);
            nResult = !GDI_IsRgnEmpty(hRgn);
        }
        UnlockDC(hDC);
    }
    return nResult;
}

// 1.14d mac: 00023f14
// Intersects (1), unions (2), subtracts (4) or copies (5) strip regions into the
// destination. 1 if the result is empty, 2 if not, 0 on error.
int CombineRgn(HRGN hrgnDest, HRGN hrgnSrc1, HRGN hrgnSrc2, int nMode) {
    int nCombineMode = 5;
    if (nMode != 2 || hrgnSrc2 != NULL) {
        nCombineMode = nMode;
    }
    int* ppDest;
    int* ppSrc1;
    int* ppSrc2;
    switch (nCombineMode) {
    case 1:
        if (hrgnDest == NULL || hrgnSrc1 == NULL || hrgnSrc2 == NULL) {
            return 0;
        }
        ppDest = LockRgn(hrgnDest);
        ppSrc1 = LockRgn(hrgnSrc1);
        ppSrc2 = LockRgn(hrgnSrc2);
        Rgn_IntersectStripRgn(*ppDest, *ppSrc1, *ppSrc2);
        break;
    case 2:
        if (hrgnDest == NULL || hrgnSrc1 == NULL || hrgnSrc2 == NULL) {
            return 0;
        }
        ppDest = LockRgn(hrgnDest);
        ppSrc1 = LockRgn(hrgnSrc1);
        ppSrc2 = LockRgn(hrgnSrc2);
        Rgn_UnionStripRgn(*ppDest, *ppSrc1, *ppSrc2);
        break;
    case 4:
        if (hrgnDest == NULL || hrgnSrc1 == NULL || hrgnSrc2 == NULL) {
            return 0;
        }
        ppDest = LockRgn(hrgnDest);
        ppSrc1 = LockRgn(hrgnSrc1);
        ppSrc2 = LockRgn(hrgnSrc2);
        Rgn_SubtractStripRgn(*ppDest, *ppSrc1, *ppSrc2);
        break;
    case 5: {
        if (hrgnDest == NULL || hrgnSrc1 == NULL || hrgnDest == hrgnSrc1) {
            return 0;
        }
        ppDest = LockRgn(hrgnDest);
        ppSrc1 = LockRgn(hrgnSrc1);
        CopyStripRgn(*ppDest, *ppSrc1);
        bool bEmpty = IsStripRgnEmpty(*ppDest);
        UnlockRgn(hrgnSrc1);
        UnlockRgn(hrgnDest);
        return (!bEmpty) + 1;
    }
    default:
        return 0;
    }
    bool bEmpty = IsStripRgnEmpty(*ppDest);
    UnlockRgn(hrgnDest);
    UnlockRgn(hrgnSrc1);
    UnlockRgn(hrgnSrc2);
    return (!bEmpty) + 1;
}

// 1.14d mac: 0002415c
// True for a NULL handle.
static BOOL GDI_IsRgnEmpty(HRGN hRgn) {
    bool bEmpty = true;
    if (hRgn != NULL) {
        int* ppRgn = LockRgn(hRgn);
        bEmpty = IsStripRgnEmpty(*ppRgn);
        UnlockRgn(hRgn);
    }
    return bEmpty;
}

// 1.14d mac: 000241a5
// The rectangle, or an empty region when the rectangle is empty.
BOOL SetRectRgn(HRGN hRgn, int nLeft, int nTop, int nRight, int nBottom) {
    BOOL bResult = 0;
    if (hRgn != NULL) {
        int* ppRgn = LockRgn(hRgn);
        if (ppRgn != NULL) {
            RECT rcRect;
            rcRect.left = nLeft;
            rcRect.top = nTop;
            rcRect.right = nRight;
            rcRect.bottom = nBottom;
            if (!IsRectEmpty(&rcRect)) {
                Rgn_SetStripRgnRect(*ppRgn, (Win32MacRect*)&rcRect);
            } else {
                Rgn_SetEmptyStripRgn(*ppRgn);
            }
        }
        UnlockRgn(hRgn);
        bResult = 1;
    }
    return bResult;
}

// 1.14d mac: 00024229
// GetRgnBox of the DC's clip region, -1 on error.
int GetClipBox(HDC hDC, RECT* pRect) {
    int nResult = -1;
    if (hDC != NULL && pRect != NULL) {
        Win32MacDCData* pDC = LockDC(hDC);
        nResult = -1;
        if (pDC->hClipRgn != NULL) {
            nResult = GetRgnBox(pDC->hClipRgn, pRect);
        }
        UnlockDC(hDC);
    }
    return nResult;
}

// 1.14d mac: 00024289
// The bounding rectangle; NULLREGION (1), SIMPLEREGION (2) or COMPLEXREGION (3).
int GetRgnBox(HRGN hRgn, RECT* pRect) {
    int nRegionType = 0;
    if (hRgn != NULL) {
        RECT rcBounds;
        int* ppRgn = LockRgn(hRgn);
        if (!IsStripRgnEmpty(*ppRgn)) {
            GetStripRgnBoundingRect(*ppRgn, (Win32MacRect*)&rcBounds);
            int nRectCount = CountStripRgnRects(*ppRgn);
            nRegionType = (1 < nRectCount) | 2;
        } else {
            SetRectEmpty(&rcBounds);
            nRegionType = 1;
        }
        if (pRect != NULL) {
            *pRect = rcBounds;
        }
        UnlockRgn(hRgn);
    }
    return nRegionType;
}

// 1.14d mac: 0002432c
// Combines hRgn into the DC's clip region with the mode (NULL empties it): 1 if the
// result is empty, 2 if not, 0 for a NULL DC.
int ExtSelectClipRgn(HDC hDC, HRGN hRgn, int nMode) {
    int nResult = 0;
    if (hDC != NULL) {
        Win32MacDCData* pDC = LockDC(hDC);
        if (hRgn == NULL) {
            SetRectRgn(pDC->hClipRgn, 0, 0, 0, 0);
        } else {
            HRGN hClip = pDC->hClipRgn;
            if (nMode == 5) {
                CombineRgn(hClip, hRgn, NULL, 5);
            } else {
                CombineRgn(hClip, hClip, hRgn, nMode);
            }
        }
        nResult = (!GDI_IsRgnEmpty(pDC->hClipRgn)) + 1;
        UnlockDC(hDC);
    }
    return nResult;
}

// 1.14d mac: 000243f2
int SelectClipRgn(HDC hDC, HRGN hRgn) {
    return ExtSelectClipRgn(hDC, hRgn, 5);
}

// 1.14d mac: 00024419
// Stores a pen, brush, palette, font or bitmap in the DC and returns the one it had; a
// region becomes the clip region.
HGDIOBJ SelectObject(HDC hDC, HGDIOBJ hObject) {
    HGDIOBJ hPrevious = NULL;
    if (hDC != NULL && hObject != NULL) {
        Win32MacDCData* pDC = LockDC(hDC);
        UINT nType = GDI_GetObjectType(hObject);
        hPrevious = NULL;
        switch (nType) {
        case OBJ_PEN:
            hPrevious = pDC->hPen;
            pDC->hPen = hObject;
            break;
        case OBJ_BRUSH:
            hPrevious = pDC->hBrush;
            pDC->hBrush = hObject;
            break;
        case OBJ_PAL:
            hPrevious = pDC->hPalette;
            pDC->hPalette = hObject;
            break;
        case OBJ_FONT:
            hPrevious = pDC->hFont;
            pDC->hFont = hObject;
            break;
        case OBJ_BITMAP:
            hPrevious = pDC->hBitmap;
            pDC->hBitmap = hObject;
            break;
        case OBJ_REGION:
            hPrevious = (HGDIOBJ)(intptr_t)ExtSelectClipRgn(hDC, hObject, 5);
        }
        UnlockDC(hDC);
    }
    return hPrevious;
}

// 1.14d mac: 000244f0
// The Win32 OBJ_* type of a handle by its tag, 0 if unknown.
static UINT GDI_GetObjectType(HGDIOBJ hObject) {
    UINT nType = 0;
    if (hObject != NULL) {
        switch (Mem_HandleType(hObject)) {
        case GDI_TYPE_BITMAP:
            nType = OBJ_BITMAP;
            break;
        case GDI_TYPE_BRUSH:
            nType = OBJ_BRUSH;
            break;
        case GDI_TYPE_FONT:
            nType = OBJ_FONT;
            break;
        case GDI_TYPE_PALETTE:
            nType = OBJ_PAL;
            break;
        case GDI_TYPE_PEN:
            nType = OBJ_PEN;
            break;
        case GDI_TYPE_RGN:
            nType = OBJ_REGION;
            break;
        }
    }
    return nType;
}

// 1.14d mac: 00024573
// A DC for the window, its visible region as the clip and the window's font. Only one may
// be out at a time: a second is deleted again and NULL returned.
HDC GetDC(void* hWnd) {
    HDC hResult = NULL;
    if (hWnd != NULL) {
        HDC hDC = GDI_NewHDC(hWnd);
        if (hDC != NULL) {
            Win32MacDCData* pDC = LockDC(hDC);
            Wnd_CalculateVisibleRgn(hWnd, pDC->hClipRgn, 1);
            HFONT hWindowFont = ((const Win32MacWindowFontView*)hWnd)->hFont;
            if (hWindowFont != NULL) {
                pDC->hFont = hWindowFont;
            }
            if (g_nOutstandingDCs == 0) {
                g_nOutstandingDCs = 1;
                POINT ptOrigin;
                ptOrigin.x = 0;
                ptOrigin.y = 0;
                ClientToScreen(pDC->hWnd, &ptOrigin);
                pDC->pPixels = NULL;
                pDC->nRowBytes = 1;
                pDC->nOriginX = ptOrigin.x;
                pDC->nOriginY = ptOrigin.y;
                UnlockDC(hDC);
                hResult = hDC;
            } else {
                UnlockDC(hDC);
                GDI_DeleteHDC(hDC);
                hResult = NULL;
            }
        }
    }
    return hResult;
}

// 1.14d mac: 00024665
BOOL ReleaseDC(void* hWnd, HDC hDC) {
    if (hDC != NULL) {
        g_nOutstandingDCs--;
        GDI_DeleteHDC(hDC);
        return 1;
    }
    return 0;
}

// 1.14d mac: 00024694
// Sets the DC's text colour, returning the old one (-1 on error). With the swap flag set
// black and white trade places first; a plain RGB colour must be one of the 20 system
// palette entries.
DWORD SetTextColor(HDC hDC, DWORD crColor) {
    DWORD crNew = crColor;
    if (g_bSwapBlackWhite != 0) {
        crNew = 0xFFFFFF;
        if (crColor != 0) {
            crNew = 0;
            if (crColor != 0xFFFFFF) {
                crNew = crColor;
            }
        }
    }
    DWORD crPrevious = 0xFFFFFFFF;
    if (hDC != NULL && crNew != 0xFFFFFFFF) {
        BYTE nIndex = (BYTE)(crNew >> 24);
        if ((crNew >> 24) != 0 || GDI_ColorToPaletteIndex(crNew & 0xFFFFFF, &nIndex)) {
            Win32MacDCData* pDC = LockDC(hDC);
            crPrevious = pDC->crText;
            pDC->crText = (DWORD)nIndex << 24 | (crNew & 0xFFFFFF);
            UnlockDC(hDC);
        }
    }
    return crPrevious;
}

// 1.14d mac: 00024733
// The palette index of one of the 20 system colours, 0 if the RGB is none of them.
BOOL GDI_ColorToPaletteIndex(DWORD dwColor, BYTE* pIndexOut) {
    if (pIndexOut == NULL) {
        return 0;
    }
    int i = 0;
    while ((dwColor & 0xFFFFFF) != (g_adwSystemPalette[i] & 0xFFFFFF)) {
        i++;
        if (i > 19) {
            return 0;
        }
    }
    BYTE nIndex = (BYTE)i;
    if ((i & 0xFE) > 9) {
        nIndex = (BYTE)(i + 0xEC);
    }
    *pIndexOut = nIndex;
    return 1;
}

// 1.14d mac: 000247a8
// The background colour counterpart of SetTextColor.
DWORD SetBkColor(HDC hDC, DWORD crColor) {
    DWORD crPrevious = 0xFFFFFFFF;
    if (hDC != NULL && crColor != 0xFFFFFFFF) {
        BYTE nIndex = (BYTE)(crColor >> 24);
        if ((crColor >> 24) == 0 && !GDI_ColorToPaletteIndex(crColor & 0xFFFFFF, &nIndex)) {
            return 0xFFFFFFFF;
        }
        Win32MacDCData* pDC = LockDC(hDC);
        crPrevious = pDC->crBack;
        pDC->crBack = (DWORD)nIndex << 24 | (crColor & 0xFFFFFF);
        UnlockDC(hDC);
    }
    return crPrevious;
}

// 1.14d mac: 00024820
// The size of the region's RGNDATA, or, when the buffer is big enough, the data itself
// (header, bounds, rectangles) and 1.
UINT GetRegionData(HRGN hRgn, UINT nBufSize, Win32MacRgnData* pRgnData) {
    UINT nResult = 0;
    if (hRgn != NULL) {
        int* ppRgn = LockRgn(hRgn);
        nResult = 0;
        if (ppRgn != NULL) {
            int nRectCount = CountStripRgnRects(*ppRgn);
            nResult = nRectCount * 0x10 + 0x20;
            if (pRgnData != NULL && nResult <= nBufSize) {
                pRgnData->dwSize = 0x20;
                pRgnData->iType = 1;
                pRgnData->nCount = nRectCount;
                pRgnData->nRgnSize = nResult;
                if (nRectCount < 1) {
                    SetRect(&pRgnData->rcBound, 0, 0, 0, 0);
                } else {
                    GetStripRgnBoundingRect(*ppRgn, (Win32MacRect*)&pRgnData->rcBound);
                    GetStripRgnRects(*ppRgn, (Win32MacRect*)pRgnData->aRects, nRectCount);
                }
                nResult = 1;
            }
        }
        UnlockRgn(hRgn);
    }
    return nResult;
}

// 1.14d mac: 00024904
BOOL MoveToEx(HDC hDC, int nX, int nY, POINT* pPrevious) {
    if (hDC != NULL) {
        Win32MacDCData* pDC = LockDC(hDC);
        if (pPrevious != NULL) {
            pPrevious->x = pDC->nCurrentX;
            pPrevious->y = pDC->nCurrentY;
        }
        pDC->nCurrentX = nX;
        pDC->nCurrentY = nY;
        UnlockDC(hDC);
    }
    return 0;
}

// 1.14d mac: 0002495c
BOOL GetCurrentPositionEx(HDC hDC, POINT* pPoint) {
    if (hDC != NULL && pPoint != NULL) {
        Win32MacDCData* pDC = LockDC(hDC);
        pPoint->x = pDC->nCurrentX;
        pPoint->y = pDC->nCurrentY;
        UnlockDC(hDC);
    }
    return 0;
}

// 1.14d mac: 000249a9
// The COLORREF of a COLOR_* index, from the system palette.
DWORD GetSysColor(int nIndex) {
    DWORD dwEntry = g_abSysColorEntry[nIndex];
    return g_adwSystemPalette[dwEntry] & 0xFFFFFF;
}

// 1.14d mac: 000249e6
// Fills the rectangle with the background colour's palette index when the DC has a
// surface.
void GDI_FillRectWithBkColor(HDC hDC, const RECT* pRect) {
    Win32MacDCData* pDC = LockDC(hDC);
    if (pDC->pPixels != NULL && pDC->nRowBytes > 0) {
        GDI_FillRect(pDC, pRect, (BYTE)(pDC->crBack >> 24));
    }
    UnlockDC(hDC);
}

// 1.14d mac: 00024a33
// Fills the rectangle with one palette index, clipped to the client area and to each
// rectangle of the DC's clip region; each filled area goes on the dirty region.
static BOOL GDI_FillRect(Win32MacDCData* pDC, const RECT* pRect, BYTE nColor) {
    UINT nSize = GetRegionData(pDC->hClipRgn, 0, NULL);
    BOOL bResult = 0;
    Win32MacRgnData* pRgnData = (Win32MacRgnData*)Priv_Alloc(nSize);
    if (pRgnData != NULL) {
        GetRegionData(pDC->hClipRgn, nSize, pRgnData);
        int nRectCount = pRgnData->nCount;
        RECT rcClip;
        GetClientRect(pDC->hWnd, &rcClip);
        IntersectRect(&rcClip, &rcClip, (RECT*)pRect);
        for (int i = 0; i != nRectCount; i++) {
            RECT rcFill;
            if (IntersectRect(&rcFill, &pRgnData->aRects[i], &rcClip)) {
                OffsetRect(&rcFill, pDC->nOriginX, pDC->nOriginY);
                int nWidth = rcFill.right - rcFill.left;
                if (nWidth > 0 && rcFill.bottom != rcFill.top && rcFill.bottom - rcFill.top >= 0) {
                    BYTE* pRow = pDC->pPixels + rcFill.top * pDC->nRowBytes + rcFill.left;
                    for (int nRow = rcFill.top - rcFill.bottom; nRow != 0; nRow++) {
                        memset(pRow, nColor, nWidth);
                        pRow += pDC->nRowBytes;
                    }
                }
                GDI_AddDirtyRect(&rcFill);
            }
        }
        Priv_Free(pRgnData);
        bResult = 1;
    }
    return bResult;
}

// 1.14d mac: 00024b8e
// Fills with palette index 0 when the DC has a surface.
BOOL FillRect(HDC hDC, const RECT* pRect) {
    Win32MacDCData* pDC = LockDC(hDC);
    BOOL bResult = 0;
    if (pDC->pPixels != NULL && pDC->nRowBytes > 0) {
        bResult = GDI_FillRect(pDC, pRect, 0);
    }
    UnlockDC(hDC);
    return bResult;
}

// 1.14d mac: 00024be3
BOOL GetCursorPos(POINT* pPoint) {
    if (pPoint == NULL) {
        return 0;
    }
    return g_pHostHooks->pfnGetCursorPos(&pPoint->x, &pPoint->y);
}

// 1.14d mac: 00024c17
BOOL SetCursorPos(int nX, int nY) {
    LONG nXCopy = nX;
    LONG nYCopy = nY;
    return g_pHostHooks->pfnSetCursorPos(&nXCopy, &nYCopy);
}

// 1.14d mac: 00024c4f
// Index 0 and 1 only: the surface's width and height.
int GetSystemMetrics(int nIndex) {
    int nResult = g_nSurfaceHeight;
    if (nIndex != 1) {
        nResult = 0;
        if (nIndex == 0) {
            nResult = g_nSurfaceWidth;
        }
    }
    return nResult;
}

// 1.14d mac: 00024c76
int GDI_ReturnZero(void) {
    return 0;
}

// 1.14d mac: 00024c7e
void ShowCursor(BOOL bShow) {
    if (bShow != 0) {
        g_pHostHooks->pfnShowCursor();
        return;
    }
    g_pHostHooks->pfnHideCursor();
}

// 1.14d mac: 00024ca1
// Saves the current GWorld and text state, switches to the scratch GWorld and applies the
// text state of the DC's font.
void GDI_SetUpMacPort(HDC hDC, Win32MacSavedPort* pSaved) {
    GetGWorld((GWorldPtr*)&pSaved->adwState[0], (GDHandle*)&pSaved->adwState[1]);
    SetGWorld(g_pScratchGWorld, NULL);
    GetTextState((int16_t*)&pSaved->adwState[2]);
    Win32MacDCData* pDC = LockDC(hDC);
    HFONT hFont = pDC->hFont;
    if (hFont != NULL) {
        Win32MacFont* pFont = (Win32MacFont*)Mem_HandleLock(GDI_TYPE_FONT, hFont);
        SetTextState((const int16_t*)&pFont->nFontNum);
        Mem_HandleUnLock(GDI_TYPE_FONT, hFont);
    }
    UnlockDC(hDC);
}

// 1.14d mac: 00024d3e
void GDI_RestoreMacPort(Win32MacSavedPort* pSaved) {
    SetTextState((const int16_t*)&pSaved->adwState[2]);
    SetGWorld((GWorldPtr)pSaved->adwState[0], (GDHandle)pSaved->adwState[1]);
}

// 1.14d mac: 00024d6a
// The font's height: ascent plus descent.
void GDI_GetFontHeight(HFONT hFont, int* pnHeight) {
    Win32MacFont* pFont = (Win32MacFont*)Mem_HandleLock(GDI_TYPE_FONT, hFont);
    *pnHeight = pFont->nAscent + pFont->nDescent;
    Mem_HandleUnLock(GDI_TYPE_FONT, hFont);
}

// 1.14d mac: 00024da7
// The metrics of the DC's font: height, ascent, descent, average width, weight (bold or
// normal), italic and underline.
BOOL GetTextMetrics(HDC hDC, Win32MacTextMetrics* pMetrics) {
    Win32MacDCData* pDC = LockDC(hDC);
    Win32MacFont* pFont = (Win32MacFont*)Mem_HandleLock(GDI_TYPE_FONT, pDC->hFont);
    LONG* pnMetrics = (LONG*)pMetrics;
    memset(pnMetrics, 0, sizeof(*pMetrics));
    pnMetrics[0] = pFont->nAscent + pFont->nDescent;
    pnMetrics[1] = pFont->nAscent;
    pnMetrics[2] = pFont->nDescent;
    pnMetrics[3] = 0;
    pnMetrics[4] = pFont->nAveWidth;
    pnMetrics[7] = (pFont->nStyle & 1) == 0 ? 400 : 700;
    pMetrics->tmItalic = (pFont->nStyle >> 1) & 1;
    pMetrics->tmUnderlined = (pFont->nStyle >> 2) & 1;
    pMetrics->tmPitchAndFamily = 4;
    Mem_HandleUnLock(GDI_TYPE_FONT, pDC->hFont);
    UnlockDC(hDC);
    return 1;
}

// 1.14d mac: 00024ec1
// -1 for a NULL DC.
UINT GetTextAlign(HDC hDC) {
    UINT nAlign = 0xFFFFFFFF;
    if (hDC != NULL) {
        nAlign = LockDC(hDC)->nTextAlign;
        UnlockDC(hDC);
    }
    return nAlign;
}

// 1.14d mac: 00025356
// Expands the rows of the 1bpp scratch GWorld to bytes: the foreground index where a bit
// is set, and the background index where it is not when bOpaque.
void GDI_ExpandMonoGWorldToBytes(int nX, int nRowBytes, BYTE* pDest, const unsigned* pRect, BYTE nForeground,
                                        BYTE nBackground, int bOpaque) {
    BYTE* pBuffer = (BYTE*)Priv_Alloc(g_nSurfaceWidth);
    if (pBuffer == NULL) {
        return;
    }
    int nRows = pRect[3] - pRect[1];
    if (0 < nRows) {
        unsigned nLeft32 = pRect[0] & 0xFFFFFFE0;
        unsigned nStride = (*g_hScratchPixMap)->rowBytes & 0x3FFF;
        const BYTE* pSrcRow = (const BYTE*)(*g_hScratchPixMap)->baseAddr + pRect[1] * nStride;
        unsigned nWords = (((pRect[2] + 0x1F) & 0xFFFFFFE0) - nLeft32) >> 5;
        for (int nRow = 0; nRow != nRows; nRow++) {
            unsigned* pExpand = (unsigned*)(pBuffer + nLeft32);
            for (unsigned w = 0; w < nWords; w++) {
                unsigned dwBits = *(const unsigned*)(pSrcRow + (nLeft32 >> 3) + w * 4);
                pExpand[0] = dwBits >> 0x1C & 1 | dwBits >> 0x15 & 0x100 | dwBits >> 7 & 0x1000000 |
                             dwBits >> 0xE & 0x10000;
                pExpand[1] = dwBits >> 0x18 & 1 | dwBits >> 0x11 & 0x100 | dwBits >> 3 & 0x1000000 |
                             dwBits >> 10 & 0x10000;
                pExpand[2] = dwBits >> 0x14 & 1 | dwBits >> 0xD & 0x100 | dwBits * 2 & 0x1000000 |
                             dwBits >> 6 & 0x10000;
                pExpand[3] = dwBits >> 0x10 & 1 | dwBits >> 9 & 0x100 | (dwBits & 0x80000) << 5 |
                             dwBits >> 2 & 0x10000;
                pExpand[4] = dwBits >> 0xC & 1 | dwBits >> 5 & 0x100 | (dwBits & 0x8000) << 9 |
                             dwBits * 4 & 0x10000;
                pExpand[5] = dwBits >> 8 & 1 | dwBits >> 1 & 0x100 | (dwBits & 0x800) << 0xD |
                             (dwBits & 0x400) << 6;
                pExpand[6] = dwBits >> 4 & 1 | (dwBits & 0x20) << 3 | (dwBits & 0x80) << 0x11 |
                             (dwBits & 0x40) << 10;
                pExpand[7] = (dwBits & 2) << 7 | (dwBits & 8) << 0x15 | (dwBits & 4) << 0xE | dwBits & 1;
                pExpand += 8;
            }
            int nWidth = pRect[2] - pRect[0];
            if (nWidth != 0) {
                const char* pSrc = (const char*)(pBuffer + pRect[0]);
                BYTE* pDst = pDest;
                for (int n = nWidth; n != 0; n--) {
                    if (*pSrc == '\0') {
                        if (bOpaque != 0) {
                            *pDst = nBackground;
                        }
                    } else {
                        *pDst = nForeground;
                    }
                    pDst++;
                    pSrc++;
                }
                pDest += nWidth;
            }
            pSrcRow += nStride;
            pDest += nRowBytes - nWidth;
        }
    }
    Priv_Free(pBuffer);
}

// 1.14d mac: 000255ed
// Unions the rectangle into the dirty region.
static void GDI_AddDirtyRect(const RECT* pRect) {
    HRGN hOldDirty = g_hDirtyRgn;
    HRGN hRgn = CreateRectRgn(pRect->left, pRect->top, pRect->right, pRect->bottom);
    if (hOldDirty != NULL) {
        CombineRgn(g_hDirtyRgn, hRgn, g_hDirtyRgn, 2);
        DeleteObject(hRgn);
        hRgn = g_hDirtyRgn;
    }
    g_hDirtyRgn = hRgn;
}

// 1.14d mac: 00024f03
// Draws one line of text on the DC: QuickDraw renders it into the monochrome scratch
// GWorld, with a caret line at one character index and an underline under another, and it
// is expanded into the DC's surface for every visible rectangle (clipped to the client
// area and to pClipRect). bUseColors sets the DC's colours from two system palette
// entries for the duration.
BOOL GDI_DrawLine(HDC hDC, RECT* pRect, RECT* pClipRect, const char* szText, int nLength, int bUseColors,
                  DWORD nCaretPos, DWORD nUnderlinePos) {
    DWORD crOldBack = 0;
    DWORD crOldText = 0;
    if (bUseColors != 0) {
        crOldBack = SetBkColor(hDC, g_adwSystemPalette[g_abSysColorEntry[13]] & 0xFFFFFF);
        crOldText = SetTextColor(hDC, g_adwSystemPalette[g_abSysColorEntry[14]] & 0xFFFFFF);
    }
    Win32MacDCData* pDC = LockDC(hDC);
    if (pDC->pPixels != NULL && pDC->nRowBytes > 0) {
        Win32MacFont* pFont = (Win32MacFont*)Mem_HandleLock(GDI_TYPE_FONT, pDC->hFont);
        LONG nAscent = pFont->nAscent;
        Mem_HandleUnLock(GDI_TYPE_FONT, pDC->hFont);
        RECT rcClient;
        RECT rcClip;
        GetClientRect(pDC->hWnd, &rcClient);
        IntersectRect(&rcClip, &rcClient, pRect);
        if (pClipRect != NULL) {
            IntersectRect(&rcClip, &rcClip, pClipRect);
        }
        RECT rcText = *pRect;
        short nOffsetX = 0;
        if (rcText.left < rcClip.left) {
            rcText.left = rcClip.left;
            nOffsetX = (short)pRect->left - (short)rcClip.left;
        }
        Win32MacSavedPort tSaved;
        GDI_SetUpMacPort(hDC, &tSaved);
        Rect rcMono = g_rcScratchGWorld;
        EraseRect(&rcMono);
        MoveTo(nOffsetX, (short)nAscent);
        DrawText(szText, 0, (short)nLength);
        if ((int)nCaretPos >= 0 && nCaretPos <= (DWORD)nLength) {
            short nWidth = TextWidth(szText, 0, (short)nCaretPos);
            MoveTo((short)(nWidth + nOffsetX), 1);
            Line(0, (short)(((short)pRect->bottom - 2) - (short)pRect->top));
        }
        if ((int)nUnderlinePos >= 0 && nUnderlinePos < (DWORD)nLength) {
            short nWidth1 = TextWidth(szText, 0, (short)nUnderlinePos);
            short nWidth2 = TextWidth(szText, 0, (short)(nUnderlinePos + 1));
            int nY = (short)(((short)pRect->bottom - 2) - (short)pRect->top);
            MoveTo((short)(nWidth1 + nOffsetX), nY);
            LineTo((short)(nWidth2 + nOffsetX), nY);
        }
        SetTextState((const int16_t*)&tSaved.adwState[2]);
        SetGWorld((GWorldPtr)tSaved.adwState[0], (GDHandle)tSaved.adwState[1]);
        UINT nSize = GetRegionData(pDC->hClipRgn, 0, NULL);
        Win32MacRgnData* pRgnData = (Win32MacRgnData*)Priv_Alloc(nSize);
        if (pRgnData != NULL) {
            GetRegionData(pDC->hClipRgn, nSize, pRgnData);
            int nRectCount = pRgnData->nCount;
            if (nRectCount != 0) {
                RECT* pRegionRect = pRgnData->aRects;
                do {
                    RECT rcDirty;
                    if (IntersectRect(&rcDirty, &rcClip, pRegionRect)) {
                        if (IntersectRect(&rcDirty, &rcText, &rcDirty)) {
                            unsigned aSource[4];
                            aSource[0] = rcDirty.left - rcText.left;
                            aSource[1] = rcDirty.top - rcText.top;
                            aSource[2] = rcDirty.right - rcText.left;
                            aSource[3] = rcDirty.bottom - rcText.top;
                            int nDestX = rcDirty.left + pDC->nOriginX;
                            GDI_ExpandMonoGWorldToBytes(
                                nDestX, pDC->nRowBytes,
                                pDC->pPixels + (rcDirty.top + pDC->nOriginY) * pDC->nRowBytes + nDestX, aSource,
                                (BYTE)(pDC->crText >> 24), (BYTE)(pDC->crBack >> 24), pDC->nBkMode == 2);
                            OffsetRect(&rcDirty, pDC->nOriginX, pDC->nOriginY);
                            GDI_AddDirtyRect(&rcDirty);
                        }
                    }
                    pRegionRect++;
                    nRectCount--;
                } while (nRectCount != 0);
            }
            Priv_Free(pRgnData);
        }
    }
    UnlockDC(hDC);
    if (bUseColors != 0) {
        SetBkColor(hDC, crOldBack);
        SetTextColor(hDC, crOldText);
    }
    return 0;
}

#endif  // D2_OS_MAC
