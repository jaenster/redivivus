// Declarations shared by the Win32 Mac layer's file-search and GDI code (Win32_Files.cpp,
// Win32_GDI.cpp), 1.14d Intel Mac. The original name of this header is unknown.
#pragma once

#include "Win32Text.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>

// QuickDraw, which the 10.7 SDK headers no longer declare.
extern "C" {
void GetFNum(ConstStr255Param pFontName, short* pnFontNum);
void GetFontName(short nFontNum, Str255 szName);
void GetGWorld(CGrafPtr* ppPort, GDHandle* phDevice);
void SetGWorld(CGrafPtr pPort, GDHandle hDevice);
OSErr NewGWorld(CGrafPtr* ppWorld, short nPixelDepth, const Rect* pBounds, CTabHandle hColorTable,
                GDHandle hDevice, unsigned long nFlags);
PixMapHandle GetGWorldPixMap(CGrafPtr pWorld);
Boolean LockPixels(PixMapHandle hPixMap);
void DisposeGWorld(CGrafPtr pWorld);
void EraseRect(const Rect* pRect);
void MoveTo(short nH, short nV);
void Line(short nDH, short nDV);
void LineTo(short nH, short nV);
void DrawText(const void* pText, short nFirstByte, short nByteCount);
void SetRect(Rect* pRect, short nLeft, short nTop, short nRight, short nBottom);
short TextWidth(const void* pText, short nFirstByte, short nByteCount);
}
typedef CGrafPtr GWorldPtr;

// 1.14d mac: 0001e325
void* MemReAlloc(void* pMem, size_t nSize);
// 1.14d mac: 00033a6f
int _strnicmp(const char* szA, const char* szB, size_t nCount);
// 1.14d mac: 00033a24
int _stricmp(const char* szA, const char* szB);

// The 0x140-byte find data FindFirstFile / FindNextFile fill (WIN32_FIND_DATA).
struct Win32MacFindData {
    DWORD dwFileAttributes;       // 0x00  0x10 directory, 0x02 hidden (leading '.')
    FILETIME tCreationTime;       // 0x04
    FILETIME tLastAccessTime;     // 0x0C
    FILETIME tLastWriteTime;      // 0x14
    DWORD nFileSizeHigh;          // 0x1C
    DWORD nFileSizeLow;           // 0x20
    DWORD dwReserved0;            // 0x24
    DWORD dwReserved1;            // 0x28
    char cFileName[0x114];        // 0x2C
};
WIN32MAC_LAYOUT(Win32MacFindData_size, sizeof(Win32MacFindData) == 0x140);

// The list a find handle (type 3) owns: the FSSpecs FSpIterateDirectory collected.
struct Win32MacFindList {
    int32_t nNext;                // 0x00  next entry FindNextFile returns
    int32_t nCount;               // 0x04
    int32_t nCapacity;            // 0x08
    Win32MacFSSpec* pEntries;     // 0x0C
};

// GDI objects are Memory Manager handles (Mem_HandleAlloc) tagged with a four-character
// type; the data behind them starts with a stock-object flag a stock object keeps set so
// DeleteObject leaves it alone.
struct HandleHeader;
typedef HandleHeader** HGDIOBJ;
typedef HGDIOBJ HRGN;
typedef HGDIOBJ HPEN;
typedef HGDIOBJ HBRUSH;
typedef HGDIOBJ HFONT;
typedef HGDIOBJ HPALETTE;

#define GDI_TYPE_DC 0x44432020      // 'DC  '
#define GDI_TYPE_RGN 0x52474E20     // 'RGN '
#define GDI_TYPE_PEN 0x50454E20     // 'PEN '
#define GDI_TYPE_BRUSH 0x42525348   // 'BRSH'
#define GDI_TYPE_FONT 0x464F4E54    // 'FONT'
#define GDI_TYPE_PALETTE 0x50414C20 // 'PAL '
#define GDI_TYPE_BITMAP 0x424D4150  // 'BMAP'

// Win32 OBJ_* values GDI_GetObjectType returns.
#define OBJ_PEN 1
#define OBJ_BRUSH 2
#define OBJ_PAL 5
#define OBJ_FONT 6
#define OBJ_BITMAP 7
#define OBJ_REGION 8

struct Win32MacPen {              // 0x14
    LONG bStock;                  // 0x00
    BYTE nColor;                  // 0x04  palette index
    BYTE pad05[3];
    LONG bNull;                   // 0x08
    LONG n0C;                     // 0x0C  1
    LONG n10;                     // 0x10
};
typedef Win32MacPen Win32MacBrush;

struct Win32MacFont {             // 0x3C
    LONG bStock;                  // 0x00
    LONG nHeight;                 // 0x04  as asked for, negative
    char szFace[0x1F];            // 0x08
    char cTerminator;             // 0x27
    uint16_t nFontNum;            // 0x28  the Mac font number; the QuickDraw text state starts here
    uint16_t nStyle;              // 0x2A  bit 0 bold, 1 italic, 2 underline
    uint16_t nSize;               // 0x2C
    int16_t nSizeNeg;             // 0x2E  -nHeight
    LONG nAscent;                 // 0x30
    LONG nDescent;                // 0x34
    LONG nAveWidth;               // 0x38
};
WIN32MAC_LAYOUT(Win32MacFont_size, sizeof(Win32MacFont) == 0x3C);

struct Win32MacPalette {          // 0x408
    LONG bStock;                  // 0x00
    LONG nEntries;                // 0x04
    DWORD aEntries[0x100];        // 0x08  0x00BBGGRR
};

// What a device context handle points at (0x48 bytes).
struct Win32MacDCData {
    HGDIOBJ hNext;                // 0x00  the global DC list
    HPEN hPen;                    // 0x04
    HBRUSH hBrush;                // 0x08
    HFONT hFont;                  // 0x0C
    HRGN hClipRgn;                // 0x10
    HGDIOBJ hBitmap;              // 0x14
    HPALETTE hPalette;            // 0x18
    LONG nBkMode;                 // 0x1C  2 = opaque
    DWORD crBack;                 // 0x20  palette index in the top byte
    DWORD crText;                 // 0x24
    DWORD nTextAlign;             // 0x28
    LONG nCurrentX;               // 0x2C
    LONG nCurrentY;               // 0x30
    void* hWnd;                   // 0x34
    LONG nOriginX;                // 0x38  the window's screen origin
    LONG nOriginY;                // 0x3C
    BYTE* pPixels;                // 0x40  the byte surface drawing goes to
    LONG nRowBytes;               // 0x44
};
WIN32MAC_LAYOUT(Win32MacDCData_size, sizeof(Win32MacDCData) == 0x48);

// The rectangle functions (Win32_Rect.cpp).
// 1.14d mac: 0002b25d
BOOL SetRect(RECT* pRect, int nLeft, int nTop, int nRight, int nBottom);
// 1.14d mac: 0002b28d
BOOL SetRectEmpty(RECT* pRect);
// 1.14d mac: 0002b2e1
BOOL IntersectRect(RECT* pDest, RECT* pRect1, RECT* pRect2);
// 1.14d mac: 0002b485
BOOL OffsetRect(RECT* pRect, int nDX, int nDY);
// 1.14d mac: 0002b4ab
BOOL IsRectEmpty(RECT* pRect);

// The strip-region engine (a region is a handle to a pointer to one).
struct StripRgn;
// 1.14d mac: 00031b0d
StripRgn* NewEmptyStripRgn(void);
// 1.14d mac: 00031ce9
void Rgn_SetStripRgnRect(StripRgn* pRgn, const RECT* pRect);
// 1.14d mac: 00031dca
void DisposeStripRgn(StripRgn* pRgn);
// 1.14d mac: 00031e16
int CountStripRgnRects(StripRgn* pRgn);
// 1.14d mac: 00031e3b
void GetStripRgnRects(StripRgn* pRgn, RECT* pRects, int nCount);
// 1.14d mac: 00031ed2
void GetStripRgnBoundingRect(StripRgn* pRgn, RECT* pRect);
// 1.14d mac: 00031f17
bool IsStripRgnEmpty(StripRgn* pRgn);
// 1.14d mac: 00032060
void Rgn_SetEmptyStripRgn(StripRgn* pRgn);
// 1.14d mac: 000320a6
void CopyStripRgn(StripRgn* pDest, StripRgn* pSrc);
// 1.14d mac: 0003215e
void Rgn_UnionStripRgn(StripRgn* pDest, StripRgn* pSrc1, StripRgn* pSrc2);
// 1.14d mac: 00032754
void Rgn_IntersectStripRgn(StripRgn* pDest, StripRgn* pSrc1, StripRgn* pSrc2);
// 1.14d mac: 00032887
void Rgn_SubtractStripRgn(StripRgn* pDest, StripRgn* pSrc1, StripRgn* pSrc2);

// 1.14d mac: 0002fbf8
void Wnd_CalculateVisibleRgn(void* hWnd, HRGN hRgn, int bClient);
// 1.14d mac: 0002fec2
BOOL ClientToScreen(void* hWnd, POINT* pPoint);
// 1.14d mac: 0002fdc5
BOOL GetClientRect(void* hWnd, RECT* pRect);
// 1.14d mac: 0002fa76
void Wnd_Shutdown(void);

// Mem_HandleAlloc and friends (Win32_Memory.cpp).
HandleHeader** Mem_HandleAlloc(DWORD dwType, size_t nSize, const void* pInitData);
BOOL Mem_HandleFree(DWORD dwType, HandleHeader** hHandle);
void* Mem_HandleLock(DWORD dwType, HandleHeader** hHandle);
BOOL Mem_HandleUnLock(DWORD dwType, HandleHeader** hHandle);
DWORD Mem_HandleType(HandleHeader** hHandle);

// The Mac shell's table of cursor hooks, pointed to by the global at 00397abc.
struct Win32MacHostHooks {
    void* apSlot00[19];           // 0x00
    BOOL (*pfnGetCursorPos)(LONG* pnX, LONG* pnY);   // 0x4C
    void* apSlot50[3];            // 0x50
    BOOL (*pfnSetCursorPos)(LONG* pnX, LONG* pnY);   // 0x5C
    void (*pfnHideCursor)(void);  // 0x60
    void (*pfnShowCursor)(void);  // 0x64
};
// 1.14d mac: 00397abc
extern Win32MacHostHooks* g_pHostHooks;

// QuickDraw text state, 8 bytes (0003 52af / 00035310).
// 1.14d mac: 000352af
void GetTextState(int16_t* pState);
// 1.14d mac: 00035310
void SetTextState(const int16_t* pState);

#endif  // D2_OS_MAC
