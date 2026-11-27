// Storm's Win32-on-Mac layer: the window manager's types, 1.14d Intel Mac
// (DiabloII_macho) band 0002eae0..00031b0c. The layer emulates HWNDs as heap
// objects linked into a tree under one desktop window; every window keeps the
// region of itself that needs painting.
//
// The original names of these structs are unknown; the layouts are read off the
// binary. Mac-only: see Win32Mac.h.
#pragma once

#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

typedef uint32_t WPARAM;
typedef int32_t LPARAM;
typedef int32_t LRESULT;
typedef uint16_t ATOM;
struct Win32MacWindow;
struct Win32MacWindowProp;
typedef Win32MacWindow* HWND;
typedef LRESULT (*WNDPROC)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct RECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};

struct POINT {
    LONG x;
    LONG y;
};

// A region: a handle to a strip region object (Win32_Region.cpp); opaque here.
typedef void** HRGN;
typedef void** HDC;

struct WNDCLASS {
    UINT style;           // 0x00
    WNDPROC lpfnWndProc;  // 0x04
    int cbClsExtra;       // 0x08
    int cbWndExtra;       // 0x0C
    int hInstance;        // 0x10
    int hIcon;            // 0x14
    int hCursor;          // 0x18
    int hbrBackground;    // 0x1C
    LPCSTR lpszMenuName;  // 0x20
    LPCSTR lpszClassName; // 0x24
};

struct WNDCLASSEX {
    UINT cbSize;          // 0x00  0x30
    UINT style;           // 0x04
    WNDPROC lpfnWndProc;  // 0x08
    int cbClsExtra;       // 0x0C
    int cbWndExtra;       // 0x10
    int hInstance;        // 0x14
    int hIcon;            // 0x18
    int hCursor;          // 0x1C
    int hbrBackground;    // 0x20
    LPCSTR lpszMenuName;  // 0x24
    LPCSTR lpszClassName; // 0x28
    int hIconSm;          // 0x2C
};

struct PAINTSTRUCT {
    HDC hdc;              // 0x00
    BOOL fErase;          // 0x04
    RECT rcPaint;         // 0x08
    BYTE rgbReserved[0x28];
};

struct DRAWITEMSTRUCT {
    UINT CtlType;         // 0x00
    UINT CtlID;           // 0x04
    UINT itemID;          // 0x08
    UINT itemAction;      // 0x0C
    UINT itemState;       // 0x10
    HWND hwndItem;        // 0x14
    HDC hDC;              // 0x18
    RECT rcItem;          // 0x1C
    DWORD itemData;       // 0x2C
};

// A registered class: 16 slots of 0x230 bytes, a slot is free while nUsed is 0.
struct Win32MacWindowClass {
    int16_t nUsed;               // 0x00  the class atom; 0 = free slot
    BYTE pad02[0x02];
    UINT cbSize;                 // 0x04
    UINT style;                  // 0x08
    WNDPROC pfnWndProc;          // 0x0C
    int cbClsExtra;              // 0x10
    int cbWndExtra;              // 0x14
    int hInstance;               // 0x18
    int hIcon;                   // 0x1C
    int hCursor;                 // 0x20
    int hbrBackground;           // 0x24
    char szMenuName[0x100];      // 0x28
    char szClassName[0x100];     // 0x128
    BYTE pad228[0x22C - 0x228];
    void* pClsExtra;             // 0x22C
};
WIN32MAC_LAYOUT(Win32MacWindowClass_size, sizeof(Win32MacWindowClass) == 0x230);

// One SetProp entry: the value, the next entry and the name, in one allocation.
struct Win32MacWindowProp {
    int nValue;                   // 0x00
    Win32MacWindowProp* pNext;    // 0x04
    char szName[1];               // 0x08
};

// A window, 0x90 bytes and then cbWndExtra.
struct Win32MacWindow {
    Win32MacWindowClass* pClass;  // 0x00
    int n04;                      // 0x04
    int n08;                      // 0x08
    WNDPROC pfnWndProc;           // 0x0C
    int n10;                      // 0x10
    int hInstance;                // 0x14
    int nID;                      // 0x18  the child id, WM_COMMAND's low word
    int nX;                       // 0x1C
    int nY;                       // 0x20
    int nWidth;                   // 0x24
    int nHeight;                  // 0x28
    int n2C;                      // 0x2C
    DWORD dwStyle;                // 0x30
    char* szText;                 // 0x34
    char* szClassName;            // 0x38
    DWORD dwExStyle;              // 0x3C
    BYTE n40;                     // 0x40
    BYTE cMnemonic;               // 0x41  the char after '&' in the text
    BYTE pad42[0x48 - 0x42];
    Win32MacWindow* pOwner;       // 0x48
    Win32MacWindow* pParent;      // 0x4C
    Win32MacWindow* pFirstChild;  // 0x50
    Win32MacWindow* pLastChild;   // 0x54
    Win32MacWindow* pPrevSibling; // 0x58
    Win32MacWindow* pNextSibling; // 0x5C
    RECT rcWindow;                // 0x60  in the parent's client space
    HRGN hUpdateRgn;              // 0x70
    int nNeedsPaint;              // 0x74
    int n78;                      // 0x78
    int n7C;                      // 0x7C
    int hFont;                    // 0x80
    int n84;                      // 0x84
    Win32MacWindowProp* pProps;   // 0x88  SetProp's list, newest first
    int nExtra;                   // 0x8C  the class's window extra bytes start here (GetWindowLong)
};
WIN32MAC_LAYOUT(Win32MacWindow_size, sizeof(Win32MacWindow) == 0x90);

#define WS_VISIBLE 0x10000000
#define WS_DISABLED 0x08000000
#define WS_CHILD 0x40000000
#define WS_CLIPSIBLINGS 0x04000000
#define WS_CLIPCHILDREN 0x02000000

#define WM_CREATE 0x0001
#define WM_DESTROY 0x0002
#define WM_SETFOCUS 0x0007
#define WM_KILLFOCUS 0x0008
#define WM_SETTEXT 0x000C
#define WM_GETTEXT 0x000D
#define WM_GETTEXTLENGTH 0x000E
#define WM_PAINT 0x000F
#define WM_CLOSE 0x0010
#define WM_ERASEBKGND 0x0014
#define WM_SHOWWINDOW 0x0018
#define WM_SETFONT 0x0030
#define WM_GETFONT 0x0031
#define WM_NCCREATE 0x0081
#define WM_NCDESTROY 0x0082
#define WM_NCHITTEST 0x0084
#define WM_COMMAND 0x0111
#define WM_SYSCOMMAND 0x0112
#define WM_KEYDOWN 0x0100
#define WM_SYSKEYDOWN 0x0104
#define WM_MOUSEMOVE 0x0200
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_LBUTTONDBLCLK 0x0203
#define WM_PARENTNOTIFY 0x0210

// The message state (003972a0): the two queues and the timer list of Win32_Messages.cpp, and the
// focus, active and capture windows and the "some window needs WM_PAINT" count the window
// manager reads. Two 40-slot rings: messages posted to windows, and input (keys, mouse) that the
// Carbon event handlers enqueue for no window.
#define MSGRING_SIZE 40

typedef void (*TIMERPROC)(HWND hWnd, UINT uMsg, UINT nIDEvent, DWORD dwTime);

struct MSG {
    HWND hwnd;       // 0x00
    UINT message;    // 0x04
    WPARAM wParam;   // 0x08
    LPARAM lParam;   // 0x0C
    DWORD time;      // 0x10
    POINT pt;        // 0x14
};

struct MsgRing {
    int nHead;                  // 0x00  next to read
    int nTail;                  // 0x04  next to write; equal to nHead when empty
    MSG aMsgs[MSGRING_SIZE];    // 0x08
};

struct Win32MacTimer {
    HWND hWnd;                  // 0x00
    UINT nIDEvent;              // 0x04
    DWORD dwLastTime;           // 0x08
    UINT uElapse;               // 0x0C
    TIMERPROC pfnTimerFunc;     // 0x10
    Win32MacTimer* pNext;       // 0x14
};

struct Win32MacMsgState {
    MsgRing tPosted;                 // 0x000  PostMessage
    MsgRing tInput;                  // 0x468  key and mouse input
    int nExitCode;                   // 0x8D0  PostQuitMessage
    BOOL bQuit;                      // 0x8D4
    BYTE bPaintPending;              // 0x8D8  some window needs WM_PAINT
    BYTE pad8D9[3];                  // 0x8D9
    Win32MacTimer* pTimers;          // 0x8DC
    BOOL bMouseMovePending;          // 0x8E0
    WPARAM wMouseMoveWParam;         // 0x8E4
    LPARAM lMouseMoveLParam;         // 0x8E8
    BYTE pad8EC[0x900 - 0x8EC];
    UINT uLastClickMsg;              // 0x900  the last WM_LBUTTONDOWN, for double-clicks
    DWORD dwLastClickTime;           // 0x904
    LONG nLastClickX;                // 0x908  screen
    LONG nLastClickY;                // 0x90C
    HWND hWndLastClick;              // 0x910  NULL after a double-click
    HWND hWndActive;                 // 0x914  GetActiveWindow, SetActiveWindow
    HWND hWndFocus;                  // 0x918
    HWND hWndCapture;                // 0x91C
};
WIN32MAC_LAYOUT(Win32MacMsgState_input, offsetof(Win32MacMsgState, tInput) == 0x468);
WIN32MAC_LAYOUT(Win32MacMsgState_timers, offsetof(Win32MacMsgState, pTimers) == 0x8DC);
WIN32MAC_LAYOUT(Win32MacMsgState_capture, offsetof(Win32MacMsgState, hWndCapture) == 0x91C);
extern Win32MacMsgState* g_pMsgState;
// The two message rings are at 0x000 and 0x468 of the state.
void Msg_RemoveMsgsForHWnd(void* pRing, HWND hWnd);
#define MSGSTATE_INPUT_RING(pState) ((void*)((BYTE*)(pState) + 0x468))

// The desktop window (003f2504) of Win32_Window.cpp.
extern HWND g_hDesktopWnd;

LRESULT DefWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT SendMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL PostMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int GetSystemMetrics(int nIndex);
void* GetStockObject(int nIndex);
void* Priv_AllocClear(size_t nSize);
BOOL PtInRect(RECT* pRect, POINT tPoint);
BOOL OffsetRect(RECT* pRect, int dx, int dy);
BOOL SubtractRect(RECT* pDest, const RECT* pSrc1, const RECT* pSrc2);
BOOL IntersectRect(RECT* pDest, const RECT* pSrc1, const RECT* pSrc2);
HRGN CreateRectRgn(int nLeft, int nTop, int nRight, int nBottom);
BOOL SetRectRgn(HRGN hRgn, int nLeft, int nTop, int nRight, int nBottom);
int CombineRgn(HRGN hDest, HRGN hSrc1, HRGN hSrc2, int nMode);
BOOL DeleteObject(void* hObject);
BOOL FillRect(HDC hDC, const RECT* pRect, int hBrush);
HDC GetDC(HWND hWnd);
int ReleaseDC(HWND hWnd, HDC hDC);
int ExtSelectClipRgn(HDC hDC, HRGN hRgn, int nMode);
int GetClipBox(HDC hDC, RECT* pRect);
int _strnicmp(const char* szA, const char* szB, size_t nCount);
LRESULT ListBox_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT Edit_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT Proc_DesktopWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT Proc_ButtonWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT Proc_StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Wnd_InitWnd(int hInstance);
Win32MacWindowClass* Wnd_FindClassByName(LPCSTR szClassName);
void Wnd_Shutdown(void);
// The ancestor whose parent is the desktop.
HWND Wnd_GetTopParent(HWND hWnd);
BOOL ClientToScreen(HWND hWnd, POINT* pPoint);
BOOL ScreenToClient(HWND hWnd, POINT* pPoint);
HWND Wnd_ChildFromPoint(HWND hWndParent, POINT pt, int bRecurse, int bVisibleOnly);
HWND GetDesktopWindow(void);
LRESULT CallWindowProc(WNDPROC pfnWndProc, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL IsWindow(HWND hWnd);
ATOM RegisterClassEx(const WNDCLASSEX* pClassEx);
ATOM RegisterClass(const WNDCLASS* pClass);
BOOL UnregisterClass(LPCSTR szClassName);
BOOL DestroyWindow(HWND hWnd);
HWND SetActiveWindow(HWND hWnd);
HWND CreateWindowEx(DWORD dwExStyle, LPCSTR szClassName, LPCSTR szWindowName, DWORD dwStyle, int nX, int nY, int nWidth,
                    int nHeight, HWND hWndParent, int hMenu, int hInstance, int lpParam);
HWND GetActiveWindow(void);
HWND WindowFromPoint(POINT pt);
BOOL AdjustWindowRectEx(void* pRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle);
BOOL UpdateWindow(HWND hWnd);
HDC BeginPaint(HWND hWnd, PAINTSTRUCT* pPaint);
BOOL IsWindowVisible(HWND hWnd);
BYTE Wnd_IsVisibleAndEnabled(HWND hWnd);
int Wnd_ReturnZero(void);
int GetWindowLong(HWND hWnd, int nIndex);
UINT Win32VirtualKey_From_MacScanCode(BYTE nKeyCode);
BYTE Key_VirtualKeyToMacKeyCode(BYTE nIndex);

BOOL GetClientRect(HWND hWnd, RECT* pRect);
BOOL GetWindowRect(HWND hWnd, RECT* pRect);
// 1 when hWndFind is hWndRoot or anywhere in its child tree.
BOOL Wnd_IsSelfOrDescendant(HWND hWndFind, HWND hWndRoot, int nDepth);
BOOL ShowWindow(HWND hWnd, UINT nCmdShow);
HWND SetFocus(HWND hWnd);
HWND GetFocus(void);
HWND GetParent(HWND hWnd);
HWND GetCapture(void);
HWND SetCapture(HWND hWnd);
BOOL ReleaseCapture(void);
BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int nX, int nY, int cx, int cy, BYTE uFlags);
int GetProp(HWND hWnd, LPCSTR szName);
BOOL SetProp(HWND hWnd, LPCSTR szName, int nValue);
int RemoveProp(HWND hWnd, LPCSTR szName);
BOOL InvalidateRect(HWND hWnd, const RECT* pRect, BOOL bErase);
BOOL EndPaint(HWND hWnd, const PAINTSTRUCT* pPaint);
BOOL Wnd_ClearUpdateRegionTree(HWND hWnd, int nReserved);

#endif  // D2_OS_MAC
