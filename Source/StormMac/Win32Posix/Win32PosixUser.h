// Win32Posix: the user entry points (windows, messages, rectangles, the keyboard, the cursor,
// the clipboard) the shared game code calls, as the SDK declares them. Read through
// Win32Posix.h.
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

// ---- windows, messages and the device context ----
extern "C" int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
void PostQuitMessage(int nExitCode);
BOOL PostMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT SendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
int GetSystemMetrics(int nIndex);
BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
BOOL ShowWindow(HWND hWnd, int nCmdShow);
BOOL DestroyWindow(HWND hWnd);
HWND CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y,
                     int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
BOOL GetClientRect(HWND hWnd, LPRECT lpRect);
HDC GetDC(HWND hWnd);
int ReleaseDC(HWND hWnd, HDC hDC);

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

BOOL IsIconic(HWND hWnd);
BOOL SetForegroundWindow(HWND hWnd);
HWND FindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName);
BOOL GetWindowPlacement(HWND hWnd, struct tagWINDOWPLACEMENT* lpwndpl);
BOOL TrackMouseEvent(struct tagTRACKMOUSEEVENT* lpEventTrack);
BOOL SystemParametersInfoA(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);

// ---- monitors and the display mode ----
HMONITOR MonitorFromWindow(HWND hwnd, DWORD dwFlags);
BOOL GetMonitorInfoA(HMONITOR hMonitor, struct tagMONITORINFO* lpmi);
LONG ChangeDisplaySettingsA(DEVMODEA* lpDevMode, DWORD dwFlags);

// ---- GDI ----
HGDIOBJ GetStockObject(int i);
DWORD GdiSetBatchLimit(DWORD dw);

// ---- resources ----
#define MAKEINTRESOURCEA(i) ((LPSTR)(ULONG_PTR)((WORD)(i)))
HANDLE LoadImageA(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad);
HICON LoadIconA(HINSTANCE hInstance, LPCSTR lpIconName);

// ---- the clipboard ----
#define CF_TEXT 1
BOOL IsClipboardFormatAvailable(UINT format);
BOOL OpenClipboard(HWND hWndNewOwner);
BOOL CloseClipboard(void);
BOOL EmptyClipboard(void);
HANDLE GetClipboardData(UINT uFormat);
HANDLE SetClipboardData(UINT uFormat, HANDLE hMem);

// ---- the input method (imm.h): no POSIX IME, so no context and no candidates ----
HKL GetKeyboardLayout(DWORD idThread);
HIMC ImmGetContext(HWND hWnd);
BOOL ImmReleaseContext(HWND hWnd, HIMC hIMC);
LONG ImmGetCompositionStringA(HIMC hIMC, DWORD dwIndex, LPVOID lpBuf, DWORD dwBufLen);
DWORD ImmGetCandidateListA(HIMC hIMC, DWORD deIndex, LPCANDIDATELIST lpCandList, DWORD dwBufLen);
DWORD ImmGetCandidateListCountA(HIMC hIMC, LPDWORD lpdwListCount);
BOOL ImmGetOpenStatus(HIMC hIMC);
BOOL ImmSetOpenStatus(HIMC hIMC, BOOL fOpen);
BOOL ImmGetConversionStatus(HIMC hIMC, LPDWORD lpfdwConversion, LPDWORD lpfdwSentence);
BOOL ImmSetConversionStatus(HIMC hIMC, DWORD fdwConversion, DWORD fdwSentence);
BOOL ImmSimulateHotKey(HWND hWnd, DWORD dwHotKeyID);
BOOL ImmIsIME(HKL hKL);

// ---- messages ----
#define PM_NOREMOVE 0x0000
#define PM_REMOVE 0x0001
BOOL PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
BOOL GetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
BOOL TranslateMessage(const MSG* lpMsg);
LRESULT DispatchMessageA(const MSG* lpMsg);
#define PostMessage PostMessageA

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
