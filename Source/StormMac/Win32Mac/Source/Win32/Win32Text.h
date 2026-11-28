// Storm's Win32-on-Mac layer: the rectangle, resource and text entry points that sit
// between the GDI band and the message code (1.14d Intel Mac 0002b1fe..0002bdea).
// The original name of this header is unknown.
#pragma once

#include "Win32Mac.h"

#if !defined(_WIN32) && D2_MAC_CARBON

struct RECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};

struct SIZE {
    LONG cx;
    LONG cy;
};

struct POINT {
    LONG x;
    LONG y;
};

// The GDI band (Win32_GDI.cpp): a device context is an opaque object here.
struct Win32MacDC;
typedef Win32MacDC* HDC;

// 15 longs; the first two are tmHeight and tmAscent.
struct Win32MacTextMetrics {
    LONG tmHeight;
    LONG tmAscent;
    LONG aOther[10];
    BYTE tmItalic;           // 0x30
    BYTE tmUnderlined;       // 0x31
    BYTE tmStruckOut;        // 0x32
    BYTE tmPitchAndFamily;   // 0x33
    LONG nTail;              // 0x34
};
static_assert(sizeof(void*) != 4 || sizeof(Win32MacTextMetrics) == 0x38, "Win32MacTextMetrics");

// The Mac port GDI_SetUpMacPort saves and GDI_RestoreMacPort puts back.
struct Win32MacSavedPort {
    DWORD adwState[4];
};

// One line of Text_CalculateLineBreaks's result, 0x14 bytes.
struct Win32MacTextLine {
    int nStart;
    int nVisibleLength;
    int nVisibleWidth;
    int nLength;
    int nWidth;
};

#define ETO_OPAQUE 0x0002
#define ETO_CLIPPED 0x0004
#define TA_UPDATECP 0x0001
#define TA_RIGHT 0x0002
#define TA_CENTER 0x0006
#define TA_BOTTOM 0x0008
#define TA_BASELINE 0x0018

void GDI_SetUpMacPort(HDC hDC, Win32MacSavedPort* pSaved);
void GDI_RestoreMacPort(Win32MacSavedPort* pSaved);
BOOL GetTextMetrics(HDC hDC, Win32MacTextMetrics* pMetrics);
UINT GetTextAlign(HDC hDC);
BOOL GetCurrentPositionEx(HDC hDC, POINT* pPoint);
BOOL MoveToEx(HDC hDC, int nX, int nY, POINT* pPrevious);
void GDI_FillRectWithBkColor(HDC hDC, const RECT* pRect);
BOOL GDI_DrawLine(HDC hDC, RECT* pBounds, RECT* pClip, const char* szText, int nLength, int n0, DWORD dwMask1,
                  DWORD dwMask2);

extern const unsigned char g_aAnsiToMacRoman[256];
extern const unsigned char g_aMacRomanToAnsi[256];

// 1.14d mac: 0002b500
void* FindResourceEx(HANDLE hModule, const char* szType, const char* szName, unsigned short wLanguage);
// 1.14d mac: 0002b180
WORD LOCALE_GetDefaultLangId(void);
// 1.14d mac: 0002a0c0
void* RESOURCE_GetModuleResourceMap(HANDLE hModule);

#endif  // D2_OS_MAC
