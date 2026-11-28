// Win32Posix: the user entry points (windows, messages, rectangles, the keyboard, the cursor,
// the clipboard) the shared game code calls, as the SDK declares them. Read through
// Win32Posix.h, after d2_platform.h's stand-ins (which declare CreateWindowExA, ShowWindow,
// SetWindowPos, GetClientRect, GetDC, PostMessageA ...).
#pragma once

#if !defined(_WIN32) && D2_BACKEND_POSIX

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
#define MAKELPARAM(l, h) ((LPARAM)(DWORD)MAKELONG(l, h))
#endif
#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wParam) ((short)HIWORD(wParam))
#endif
#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) ((type*)((char*)(address) - offsetof(type, field)))
#endif
#define WM_CLOSE 0x0010
#define WM_MOUSEMOVE 0x0200
#define HWND_NOTOPMOST ((HWND)(intptr_t)-2)

struct tagWNDCLASSA {
    UINT style;
    WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    LPCSTR lpszMenuName;
    LPCSTR lpszClassName;
};
typedef tagWNDCLASSA WNDCLASSA;
typedef tagWNDCLASSA WNDCLASS;

// ---- rectangles ----
BOOL CopyRect(LPRECT lprcDst, const RECT* lprcSrc);
BOOL IntersectRect(LPRECT lprcDst, const RECT* lprcSrc1, const RECT* lprcSrc2);
BOOL PtInRect(const RECT* lprc, POINT pt);
BOOL AdjustWindowRectEx(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle);

// ---- the keyboard and the cursor ----
SHORT GetAsyncKeyState(int vKey);
BOOL GetCursorPos(LPPOINT lpPoint);
BOOL SetCursorPos(int X, int Y);
int ShowCursor(BOOL bShow);
HCURSOR LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName);
#define LoadCursor LoadCursorA

// ---- windows ----
ATOM RegisterClassA(const WNDCLASSA* lpWndClass);
BOOL UnregisterClassA(LPCSTR lpClassName, HINSTANCE hInstance);
#define RegisterClass RegisterClassA
#define UnregisterClass UnregisterClassA
#define CreateWindowEx CreateWindowExA
LRESULT DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL GetWindowRect(HWND hWnd, LPRECT lpRect);
HWND GetDesktopWindow(void);
BOOL UpdateWindow(HWND hWnd);
HWND SetFocus(HWND hWnd);
BOOL IsWindow(HWND hWnd);
BOOL IsWindowVisible(HWND hWnd);

// ---- messages ----
#define PostMessage PostMessageA

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
