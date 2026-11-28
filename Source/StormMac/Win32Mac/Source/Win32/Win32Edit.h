// Storm's Win32-on-Mac layer: the edit control (Win32_Edit.cpp), 1.14d Intel Mac
// 0001ed6c..00021690. The original name of this header is unknown.
#pragma once

#include "Win32Text.h"

#if !defined(_WIN32) && D2_MAC_CARBON

struct Win32MacWindow;
typedef Win32MacWindow* HWND;
typedef uint32_t WPARAM;
typedef int32_t LPARAM;

#define ES_MULTILINE 0x0004
#define ES_PASSWORD 0x0020
#define ES_AUTOVSCROLL 0x0040
#define ES_AUTOHSCROLL 0x0080
#define ES_READONLY 0x0800
#define ES_WANTRETURN 0x1000

// What the control keeps under the window property "__EditData": 0x4C bytes. A backup
// (Edit_CreateEditDataBackup) is the same header followed by a copy of the text.
struct Win32MacEditData {
    DWORD dwStyle;                // 0x00
    int nLimit;                   // 0x04  characters, 30000 to start with
    int bCaretOn;                 // 0x08  the blink phase
    int nSelAnchor;               // 0x0C
    int nSelCaret;                // 0x10
    int nLength;                  // 0x14  characters in pText
    int bChanged;                 // 0x18  EN_CHANGE is due after the next paint
    int nScrollX;                 // 0x1C  single-line horizontal scroll, pixels
    int n20;                      // 0x20
    int nDragX;                   // 0x24  -1/1 while a drag is outside the client rect
    int nDragY;                   // 0x28
    int n2C;                      // 0x2C
    int n30;                      // 0x30
    int nLines;                   // 0x34
    int nTopLine;                 // 0x38
    Win32MacTextLine* pLines;     // 0x3C
    int nLineHeight;              // 0x40
    int nCapacity;                // 0x44  bytes in pText
    char* pText;                  // 0x48
};
WIN32MAC_LAYOUT(Win32MacEditData_size, sizeof(Win32MacEditData) == 0x4C);

struct PAINTSTRUCT {
    DWORD adw[16];
};

// 1.14d mac: 0001ed6c
// The edit control's window procedure.
LONG Edit_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif  // D2_OS_MAC
