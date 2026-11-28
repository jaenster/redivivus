// Win32Posix: what the User*.cpp units share and nothing else reads - the window and class
// tables, the message queue, the key state and the SDL3 side of each window. It stands where
// Win32MacWindow.h and Win32Priv.h stand for Storm's Win32-on-Mac layer.
//
// Handles are indices into this layer's tables with a tag in the upper half, never pointers:
// an HWND is kHwndTag | slot, its HDC kHdcTag | slot, and so on. A 64-bit build hands the
// game the same 32-bit values a 32-bit one does.
#pragma once

#if !defined(_WIN32) && D2_BACKEND_POSIX

// SDL only: the C++ library's containers and threads read <sys/types.h>, whose u_long and
// fd_set are not Winsock's (Win32PosixSockets.h).
#include <SDL3/SDL.h>

namespace Win32Posix {

// ---- the window messages and the values this layer packs into them (winuser.h) ----
enum : UINT {
    kWmNull = 0x0000,
    kWmCreate = 0x0001,
    kWmDestroy = 0x0002,
    kWmMove = 0x0003,
    kWmSize = 0x0005,
    kWmActivate = 0x0006,
    kWmSetFocus = 0x0007,
    kWmKillFocus = 0x0008,
    kWmSetText = 0x000C,
    kWmPaint = 0x000F,
    kWmClose = 0x0010,
    kWmQuit = 0x0012,
    kWmEraseBkgnd = 0x0014,
    kWmShowWindow = 0x0018,
    kWmActivateApp = 0x001C,
    kWmSetCursor = 0x0020,
    kWmGetMinMaxInfo = 0x0024,
    kWmWindowPosChanged = 0x0047,
    kWmDisplayChange = 0x007E,
    kWmNcCreate = 0x0081,
    kWmNcDestroy = 0x0082,
    kWmNcHitTest = 0x0084,
    kWmNcActivate = 0x0086,
    kWmNcMouseMove = 0x00A0,
    kWmKeyDown = 0x0100,
    kWmKeyUp = 0x0101,
    kWmChar = 0x0102,
    kWmSysKeyDown = 0x0104,
    kWmSysKeyUp = 0x0105,
    kWmSysChar = 0x0106,
    kWmCommand = 0x0111,
    kWmSysCommand = 0x0112,
    kWmTimer = 0x0113,
    kWmMouseMove = 0x0200,
    kWmLButtonDown = 0x0201,
    kWmLButtonUp = 0x0202,
    kWmLButtonDblClk = 0x0203,
    kWmRButtonDown = 0x0204,
    kWmRButtonUp = 0x0205,
    kWmRButtonDblClk = 0x0206,
    kWmMButtonDown = 0x0207,
    kWmMButtonUp = 0x0208,
    kWmMButtonDblClk = 0x0209,
    kWmMouseWheel = 0x020A,
    kWmXButtonDown = 0x020B,
    kWmXButtonUp = 0x020C,
    kWmXButtonDblClk = 0x020D,
    kWmMouseHWheel = 0x020E,
    kWmCaptureChanged = 0x0215,
    kWmMouseLeave = 0x02A3,
};

enum : WPARAM {
    kMkLButton = 0x0001,
    kMkRButton = 0x0002,
    kMkShift = 0x0004,
    kMkControl = 0x0008,
    kMkMButton = 0x0010,
    kMkXButton1 = 0x0020,
    kMkXButton2 = 0x0040,
};

enum : WPARAM {
    kSizeRestored = 0,
    kSizeMinimized = 1,
    kSizeMaximized = 2,
};

enum : WPARAM {
    kScClose = 0xF060,
    kScMinimize = 0xF020,
    kScMaximize = 0xF030,
    kScRestore = 0xF120,
};

enum : int {
    kWheelDelta = 120,
};

// The virtual keys this layer names (winuser.h).
enum : BYTE {
    kVkLButton = 0x01,
    kVkRButton = 0x02,
    kVkMButton = 0x04,
    kVkXButton1 = 0x05,
    kVkXButton2 = 0x06,
    kVkBack = 0x08,
    kVkTab = 0x09,
    kVkReturn = 0x0D,
    kVkShift = 0x10,
    kVkControl = 0x11,
    kVkMenu = 0x12,
    kVkPause = 0x13,
    kVkCapital = 0x14,
    kVkEscape = 0x1B,
    kVkSpace = 0x20,
    kVkLWin = 0x5B,
    kVkRWin = 0x5C,
    kVkF4 = 0x73,
    kVkF10 = 0x79,
    kVkNumLock = 0x90,
    kVkScroll = 0x91,
    kVkLShift = 0xA0,
    kVkRShift = 0xA1,
    kVkLControl = 0xA2,
    kVkRControl = 0xA3,
    kVkLMenu = 0xA4,
    kVkRMenu = 0xA5,
};

// The window styles this layer reads (winuser.h).
enum : DWORD {
    kWsPopup = 0x80000000u,
    kWsChild = 0x40000000u,
    kWsMinimize = 0x20000000u,
    kWsVisible = 0x10000000u,
    kWsMaximize = 0x01000000u,
    kWsCaption = 0x00C00000u,
    kWsThickFrame = 0x00040000u,
};

enum : UINT {
    kCsDblClks = 0x0008,
};

// ---- handles ----
enum : uint32_t {
    kHandleTagMask = 0xFFFF0000u,
    kHandleSlotMask = 0x0000FFFFu,
    kHwndTag = 0x00570000u,
    kHdcTag = 0x00DC0000u,
    kHcursorTag = 0x00C50000u,
    kHiconTag = 0x00C10000u,
    kHimageTag = 0x00B10000u,
    kHstockTag = 0x00510000u,
    kHmonitorTag = 0x004D0000u,
    kAtomBase = 0xC000u,
};

static inline HANDLE MakeHandle(uint32_t nTag, uint32_t nSlot) {
    return (HANDLE)(uintptr_t)(nTag | (nSlot & kHandleSlotMask));
}
static inline bool HandleHasTag(const void* h, uint32_t nTag) {
    uintptr_t n = (uintptr_t)h;
    return n <= 0xFFFFFFFFu && (n & kHandleTagMask) == nTag;
}
static inline uint32_t HandleSlot(const void* h) {
    return (uint32_t)((uintptr_t)h & kHandleSlotMask);
}

// ---- classes and windows ----
struct WndClass {
    bool bUsed;
    char szName[64];
    WNDCLASSA tClass;
    int nWindows;
};

struct Wnd {
    bool bUsed;
    uint32_t nClass;  // slot in the class table
    WNDPROC pfnWndProc;
    DWORD dwStyle;
    DWORD dwExStyle;
    HWND hParent;
    LONG_PTR nId;  // the hMenu argument of a child window
    HINSTANCE hInstance;
    LONG_PTR nUserData;
    uint8_t abExtra[64];  // cbWndExtra
    int nExtra;
    char szTitle[256];
    RECT rcWindow;  // screen coordinates, the layer's units
    int nClientWidth;
    int nClientHeight;
    SDL_Window* pSdlWindow;  // top-level windows only
    SDL_WindowID nSdlId;
    bool bFullscreen;  // a popup over the whole display: borderless desktop fullscreen
    bool bNeedsPaint;
    bool bTrackLeave;  // TrackMouseEvent(TME_LEAVE)
};

enum : uint32_t {
    kMaxWindows = 256,
    kMaxClasses = 64,
    kDesktopSlot = 1,
    kQueueSize = 4096,
    kMaxSent = 64,
    kMaxTimers = 64,
};

// The layer's state. One process-wide instance, as Win32Mac's globals are; the message queue
// is the only part touched from more than one thread.
struct State {
    bool bSdlReady;
    SDL_ThreadID nPumpThread;
    Uint32 nWakeEvent;  // an SDL user event that wakes GetMessageA from another thread

    WndClass aClasses[kMaxClasses];
    Wnd aWindows[kMaxWindows];

    HWND hFocus;
    HWND hActive;
    HWND hCapture;
    bool bAppActive;

    // Keys: per virtual key, 0x80 down and 0x01 toggled, as GetKeyState reports them. The
    // async table is updated when the event arrives, the synchronous one when its message is
    // taken off the queue (as Win32Mac's IPeekMessage does).
    BYTE abKeyState[256];
    BYTE abAsyncState[256];
    BYTE abAsyncPressed[256];  // pressed since the last GetAsyncKeyState

    // The mouse.
    POINT ptCursor;  // screen
    int nCursorCount;  // ShowCursor's display count
    HCURSOR hCursor;
    bool bClip;
    RECT rcClip;
    bool bRelative;
    float fWheelRemainder;
    float fHWheelRemainder;
    LPARAM nLastKeyLParam;  // for WM_CHAR from text input

    // The display: a ChangeDisplaySettingsA mode is never set on the hardware, it is the size
    // the layer reports and scales to.
    bool bModeChanged;
    int nModeWidth;
    int nModeHeight;
    int nModeRefresh;

    // Timers.
    struct Timer {
        HWND hWnd;
        UINT_PTR nId;
        UINT nElapse;
        void* pfnProc;
        uint64_t nDue;
    };
    Timer aTimers[kMaxTimers];
    uint32_t nTimers;
    UINT_PTR nNextTimerId;

    // The queue, under pLock.
    SDL_Mutex* pLock;
    MSG aQueue[kQueueSize];
    uint32_t nQueue;
    bool bQuitPosted;
    int nQuitCode;
    struct Sent {
        HWND hWnd;
        UINT uMsg;
        WPARAM wParam;
        LPARAM lParam;
        LRESULT lResult;
        bool bDone;
    };
    Sent* apSent[kMaxSent];
    uint32_t nSent;
    SDL_Condition* pSentDone;

    // The clipboard.
    bool bClipboardOpen;
    HGLOBAL hClipboardData;

    uint64_t nCreateFlags;
    bool bCreateFlagsSet;
};

State& G();

// The queue lock, held for a scope.
struct Lock {
    Lock() { SDL_LockMutex(G().pLock); }
    ~Lock() { SDL_UnlockMutex(G().pLock); }
    Lock(const Lock&) = delete;
    Lock& operator=(const Lock&) = delete;
};

// ---- UserWindow.cpp ----
bool EnsureSdl();
Wnd* WndFromHandle(HWND hWnd);
HWND HandleFromWnd(const Wnd* pWnd);
HWND WndFromSdlId(SDL_WindowID nId);
HWND MainWindow();
LRESULT CallProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ClientOrigin(const Wnd* pWnd, POINT* pt);
void SdlToClient(const Wnd* pWnd, float fX, float fY, int* pnX, int* pnY);
void ClientToSdl(const Wnd* pWnd, int nX, int nY, float* pfX, float* pfY);
void UpdateFullscreen(Wnd* pWnd);
void UpdateAllFullscreen();
void OnSdlResized(HWND hWnd, int nWidth, int nHeight);
void OnSdlMoved(HWND hWnd);

// ---- UserMessages.cpp ----
void Enqueue(const MSG& tMsg);
void EnqueueFront(const MSG& tMsg);
void Post(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void RemoveWindowMessages(HWND hWnd);
void TranslateSdlEvent(const SDL_Event& tEvent);
void PumpSdl();

// ---- UserScript.cpp ----
void RunScript();  // the due steps of D2_POSIX_SCRIPT, from each pump

// ---- UserKeys.cpp ----
BYTE VirtualKeyFromSdl(SDL_Scancode nScancode, SDL_Keycode nKey);
BYTE ScanCodeFromSdl(SDL_Scancode nScancode, bool* pbExtended);
void OnSdlKey(const SDL_KeyboardEvent& tKey);
void OnSdlText(const SDL_TextInputEvent& tText);
void ApplyKeyMessage(const MSG& tMsg);
void SetAsyncKey(BYTE nVk, bool bDown);
WPARAM MouseKeyFlags();
void ResetKeys();

// ---- UserMouse.cpp ----
void OnSdlMouseMotion(const SDL_MouseMotionEvent& tMotion);
void OnSdlMouseButton(const SDL_MouseButtonEvent& tButton);
void OnSdlMouseWheel(const SDL_MouseWheelEvent& tWheel);
void OnSdlMouseLeave(HWND hWnd);
void ApplyCursor();

// ---- UserDisplay.cpp ----
void ScreenSize(int* pnWidth, int* pnHeight);
void WorkArea(RECT* pRect);

// ---- UserString.cpp ----
// The game's text is Windows-1252; SDL's is UTF-8.
size_t AnsiToUtf8(const char* szAnsi, char* szOut, size_t nOut);
size_t Utf8ToAnsi(const char* szUtf8, char* szOut, size_t nOut);

}  // namespace Win32Posix

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
