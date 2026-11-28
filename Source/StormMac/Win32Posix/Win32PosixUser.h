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

// ---- the rest of the layer's surface: not called by the game today, there for the shells,
// tools and renderers on top of it ----
typedef void (*TIMERPROC)(HWND, UINT, UINT_PTR, DWORD);
struct tagCREATESTRUCTA {
    LPVOID lpCreateParams;
    HINSTANCE hInstance;
    HMENU hMenu;
    HWND hwndParent;
    int cy;
    int cx;
    int y;
    int x;
    LONG style;
    LPCSTR lpszName;
    LPCSTR lpszClass;
    DWORD dwExStyle;
};
typedef tagCREATESTRUCTA CREATESTRUCTA;
WIN32POSIX_SIZE(CREATESTRUCTA, 0x30, 0x50);
struct tagPAINTSTRUCT {
    HDC hdc;
    BOOL fErase;
    RECT rcPaint;
    BOOL fRestore;
    BOOL fIncUpdate;
    BYTE rgbReserved[32];
};
typedef tagPAINTSTRUCT PAINTSTRUCT;
WIN32POSIX_SIZE(PAINTSTRUCT, 0x40, 0x48);

LRESULT CallWindowProcA(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LONG GetWindowLongA(HWND hWnd, int nIndex);
LONG SetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong);
LONG_PTR GetWindowLongPtrA(HWND hWnd, int nIndex);
LONG_PTR SetWindowLongPtrA(HWND hWnd, int nIndex, LONG_PTR dwNewLong);
BOOL SetWindowTextA(HWND hWnd, LPCSTR lpString);
int GetWindowTextA(HWND hWnd, LPSTR lpString, int nMaxCount);
HWND GetParent(HWND hWnd);
HWND GetFocus(void);
HWND GetActiveWindow(void);
HWND SetActiveWindow(HWND hWnd);
HWND GetForegroundWindow(void);
BOOL ScreenToClient(HWND hWnd, LPPOINT lpPoint);
BOOL ClientToScreen(HWND hWnd, LPPOINT lpPoint);
BOOL InvalidateRect(HWND hWnd, const RECT* lpRect, BOOL bErase);
BOOL ValidateRect(HWND hWnd, const RECT* lpRect);
HDC BeginPaint(HWND hWnd, PAINTSTRUCT* lpPaint);
BOOL EndPaint(HWND hWnd, const PAINTSTRUCT* lpPaint);
UINT_PTR SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc);
BOOL KillTimer(HWND hWnd, UINT_PTR uIDEvent);
HWND SetCapture(HWND hWnd);
BOOL ReleaseCapture(void);
HWND GetCapture(void);
BOOL ClipCursor(const RECT* lpRect);
BOOL GetClipCursor(LPRECT lpRect);
HCURSOR SetCursor(HCURSOR hCursor);
BOOL EnumDisplaySettingsA(LPCSTR lpszDeviceName, DWORD iModeNum, DEVMODEA* lpDevMode);
BOOL OffsetRect(LPRECT lprc, int dx, int dy);
BOOL InflateRect(LPRECT lprc, int dx, int dy);
BOOL SetRectEmpty(LPRECT lprc);
BOOL IsRectEmpty(const RECT* lprc);
BOOL EqualRect(const RECT* lprc1, const RECT* lprc2);
BOOL UnionRect(LPRECT lprcDst, const RECT* lprcSrc1, const RECT* lprcSrc2);

// ---- the SDL3 side, for the renderer and the shell ----
struct SDL_Window;
// The SDL window behind a top-level HWND, or NULL.
SDL_Window* Win32Posix_GetSdlWindow(HWND hWnd);
// Where the window's client area lands in its drawable, in pixels: the client keeps the
// size the game gave it and is scaled, aspect kept, into the window SDL has.
BOOL Win32Posix_GetViewport(HWND hWnd, int* pnX, int* pnY, int* pnWidth, int* pnHeight);
// The SDL_WindowFlags every later CreateWindowExA adds (SDL_WINDOW_OPENGL by default).
void Win32Posix_SetWindowCreateFlags(uint64_t nFlags);
// Relative mouse mode: the OS cursor is hidden and held, and WM_MOUSEMOVE carries a cursor
// the layer moves by the relative motion, kept inside the client area.
BOOL Win32Posix_SetRelativeMouse(HWND hWnd, BOOL bRelative);

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
