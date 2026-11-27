// Storm's Win32-on-Mac layer: the window, message and key types that Win32_Keys.cpp and
// Win32_ListBox.cpp share. Win32_Messages.cpp declares the same ones for itself (its
// Win32MacWindow lists only the fields it reads); the layouts here must agree with it.
#pragma once

#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

struct Win32MacWindow;
typedef Win32MacWindow* HWND;
typedef uint32_t WPARAM;
typedef int32_t LPARAM;

struct POINT {
    LONG x;
    LONG y;
};

struct tagMSG {
    HWND hwnd;     // 0x00
    UINT message;  // 0x04
    WPARAM wParam; // 0x08
    LPARAM lParam; // 0x0C
    DWORD time;    // 0x10
    POINT pt;      // 0x14
};
typedef tagMSG MSG;

struct RECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};

typedef HandleHeader** HGDIOBJ;
typedef uint32_t COLORREF;
typedef LONG (*WNDPROC)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct PAINTSTRUCT {
    HDC hdc;
    BOOL fErase;
    RECT rcPaint;
    BOOL fRestore;
    BOOL fIncUpdate;
    BYTE rgbReserved[32];
};

#define WM_CHAR 0x0102
#define WM_SYSCHAR 0x0106
#define WM_KEYDOWN 0x0100
#define WM_SYSKEYDOWN 0x0104
#define WM_KEYUP 0x0101
#define WM_SYSKEYUP 0x0105
#define VK_CONTROL 0x11
#define VK_CAPITAL 0x14
#define VK_LWIN 0x5B

// 1.14d mac: 000305a7
HWND GetFocus(void);
// 1.14d mac: 0002ffdd
HWND GetDesktopWindow(void);
// 1.14d mac: 0002956e
BOOL PostMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 1.14d mac: 00031a84
// The virtual key a Mac key code stands for.
UINT Win32VirtualKey_From_MacScanCode(BYTE byMacKeyCode);
// 1.14d mac: 00031af6
BYTE Key_VirtualKeyToMacKeyCode(BYTE byVirtKey);

#endif  // D2_OS_MAC
