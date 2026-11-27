// Storm's Win32-on-Mac layer: the user entry points (windows, messages, rectangles, the
// keyboard and the cursor) as the shared game code calls them.
//
// The layer implements them under its own names, without the A suffix (Win32_Window.cpp,
// Win32_Messages.cpp, Win32_Rect.cpp, Win32_Keys.cpp, Win32_GDI.cpp), and the 1.14d Mac build
// calls them so. Declared with the shared code's Win32 types (d2_platform.h: HWND, RECT,
// POINT, WNDPROC), which the layer's own window headers spell differently, so it is read by
// d2_platform.h after them (and after Win32Mac.h) and not by the layer's units.
#pragma once

#if !defined(_WIN32) && D2_OS_MAC

#ifndef LOWORD
#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#endif
#ifndef HIWORD
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#endif
#ifndef MAKELONG
#define MAKELONG(a, b) ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#endif
#ifndef MAKELPARAM
#define MAKELPARAM(l, h) ((LPARAM)MAKELONG(l, h))
#endif
#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wParam) ((short)HIWORD(wParam))
#endif
#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) ((type*)((char*)(address) - (size_t)(&((type*)0)->field)))
#endif
#ifndef WM_CLOSE
#define WM_CLOSE 0x0010
#endif
#ifndef WM_MOUSEMOVE
#define WM_MOUSEMOVE 0x0200
#endif
#define HWND_NOTOPMOST ((HWND)-2)

// ---- Win32_Rect.cpp, Win32_Keys.cpp ----
BOOL CopyRect(RECT* pDest, RECT* pSrc);
BOOL IntersectRect(RECT* pDest, RECT* pRect1, RECT* pRect2);
BOOL PtInRect(RECT* pRect, POINT tPoint);
short GetAsyncKeyState(BYTE byVirtKey);

// ---- Win32_Window.cpp, Win32_GDI.cpp ----
struct WNDCLASS {
    UINT style;
    WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    int hInstance;
    int hIcon;
    int hCursor;
    int hbrBackground;
    LPCSTR lpszMenuName;
    LPCSTR lpszClassName;
};
ATOM RegisterClass(const WNDCLASS* pClass);
BOOL UnregisterClass(LPCSTR szClassName);
HWND CreateWindowEx(DWORD dwExStyle, LPCSTR szClassName, LPCSTR szWindowName, DWORD dwStyle, int nX, int nY, int nWidth,
                    int nHeight, HWND hWndParent, int hMenu, int hInstance, int lpParam);
LRESULT DefWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL GetCursorPos(POINT* pPoint);
BOOL SetCursorPos(int nX, int nY);
BOOL GetWindowRect(HWND hWnd, RECT* pRect);
HWND GetDesktopWindow(void);
BOOL AdjustWindowRectEx(void* pRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle);
BOOL UpdateWindow(HWND hWnd);
HWND SetFocus(HWND hWnd);
// 00024c76: the layer's LoadCursor, a stub that returns 0.
int LoadCursor(int hInstance, int nCursorId);
// The layer's ShowCursor (00024c7e) leaves the host's display count in eax, which the shared
// code's show/hide loops read.
int ShowCursor(BOOL bShow);
int Wnd_ReturnZero(void);
#define DefWindowProcA DefWindowProc

// ---- Win32_Messages.cpp ----
// The layer's message, 0x1C bytes.
struct tagMSG {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
};
BOOL PeekMessage(tagMSG* pMsg, HWND hWnd, UINT nMsgFilterMin, UINT nMsgFilterMax, UINT wRemoveMsg);
BOOL GetMessage(tagMSG* pMsg, HWND hWnd, UINT nMsgFilterMin, UINT nMsgFilterMax);
BOOL TranslateMessage(const tagMSG* pMsg);
LONG DispatchMessage(const tagMSG* pMsg);
BOOL PostMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#define PeekMessageA PeekMessage
#define GetMessageA GetMessage
#define DispatchMessageA DispatchMessage

// ---- StormMac/SOURCE/SVID.cpp ----
// Storm Mac's own: callers read LaunchURL's result, left in eax.
int STORM_LaunchURL(char* szURL);

#endif  // D2_OS_MAC
