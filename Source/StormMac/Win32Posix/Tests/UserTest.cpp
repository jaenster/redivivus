// Win32Posix: the user layer's test. It opens a window, feeds SDL events into the pump and
// checks the WM_* messages that come out against Windows' packing, then the window procedures,
// timers, quit, the cross-thread SendMessageA, the display, the clipboard and the rectangles.
// Every message the window procedure sees is logged.
//
// Headless: SDL_VIDEODRIVER=dummy ./user-test (offscreen needs EGL for any window)
// Live:     ./user-test --live   (a real window; type, click, resize, close it)
//
// Built by build-user-test.sh. With no Kernel side linked it supplies the few Kernel entry
// points the user layer calls (WIN32POSIX_TEST_KERNEL_STUBS).
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if WIN32POSIX_TEST_KERNEL_STUBS
static DWORD s_dwLastError;
extern "C" DWORD GetTickCount() {
    return (DWORD)SDL_GetTicks();
}
extern "C" DWORD GetLastError() {
    return s_dwLastError;
}
extern "C" void SetLastError(DWORD dwErrCode) {
    s_dwLastError = dwErrCode;
}
HGLOBAL GlobalAlloc(UINT uFlags, SIZE_T dwBytes) {
    (void)uFlags;
    return SDL_calloc(1, dwBytes ? dwBytes : 1);
}
LPVOID GlobalLock(HGLOBAL hMem) {
    return hMem;
}
BOOL GlobalUnlock(HGLOBAL hMem) {
    (void)hMem;
    return TRUE;
}
HGLOBAL GlobalFree(HGLOBAL hMem) {
    SDL_free(hMem);
    return nullptr;
}
#endif

// ---- the log ----

struct Logged {
    HWND hWnd;
    UINT uMsg;
    WPARAM wParam;
    LPARAM lParam;
};
static Logged s_aLog[1024];
static int s_nLog;
static bool s_bQuiet;
static int s_nFailures;
static int s_nChecks;

static const char* MessageName(UINT uMsg) {
    switch (uMsg) {
    case 0x0001: return "WM_CREATE";
    case 0x0002: return "WM_DESTROY";
    case 0x0003: return "WM_MOVE";
    case 0x0005: return "WM_SIZE";
    case 0x0006: return "WM_ACTIVATE";
    case 0x0007: return "WM_SETFOCUS";
    case 0x0008: return "WM_KILLFOCUS";
    case 0x000F: return "WM_PAINT";
    case 0x0010: return "WM_CLOSE";
    case 0x0012: return "WM_QUIT";
    case 0x0018: return "WM_SHOWWINDOW";
    case 0x001C: return "WM_ACTIVATEAPP";
    case 0x0081: return "WM_NCCREATE";
    case 0x0082: return "WM_NCDESTROY";
    case 0x0086: return "WM_NCACTIVATE";
    case 0x0100: return "WM_KEYDOWN";
    case 0x0101: return "WM_KEYUP";
    case 0x0102: return "WM_CHAR";
    case 0x0104: return "WM_SYSKEYDOWN";
    case 0x0105: return "WM_SYSKEYUP";
    case 0x0106: return "WM_SYSCHAR";
    case 0x0112: return "WM_SYSCOMMAND";
    case 0x0113: return "WM_TIMER";
    case 0x0200: return "WM_MOUSEMOVE";
    case 0x0201: return "WM_LBUTTONDOWN";
    case 0x0202: return "WM_LBUTTONUP";
    case 0x0203: return "WM_LBUTTONDBLCLK";
    case 0x0204: return "WM_RBUTTONDOWN";
    case 0x0205: return "WM_RBUTTONUP";
    case 0x0207: return "WM_MBUTTONDOWN";
    case 0x0208: return "WM_MBUTTONUP";
    case 0x020A: return "WM_MOUSEWHEEL";
    case 0x020B: return "WM_XBUTTONDOWN";
    case 0x020C: return "WM_XBUTTONUP";
    case 0x020E: return "WM_MOUSEHWHEEL";
    case 0x0215: return "WM_CAPTURECHANGED";
    case 0x02A3: return "WM_MOUSELEAVE";
    case 0x0401: return "WM_USER+1";
    default: return "WM_?";
    }
}

static void Check(bool bOk, const char* szWhat) {
    s_nChecks++;
    if (!bOk) {
        s_nFailures++;
        printf("FAIL %s\n", szWhat);
    }
}

static bool s_bCloseDestroys;

static LRESULT TestWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (s_nLog < (int)SDL_arraysize(s_aLog)) {
        s_aLog[s_nLog++] = {hWnd, uMsg, wParam, lParam};
    }
    if (!s_bQuiet) {
        printf("  %-18s hwnd=%p wParam=0x%08llx lParam=0x%08llx\n", MessageName(uMsg), hWnd,
               (unsigned long long)wParam, (unsigned long long)(uint32_t)lParam);
    }
    switch (uMsg) {
    case 0x0010:  // WM_CLOSE: kept unless asked, so the test can go on
        if (s_bCloseDestroys) {
            DestroyWindow(hWnd);
        }
        return 0;
    case 0x0002:  // WM_DESTROY
        PostQuitMessage(0);
        return 0;
    case 0x0401:  // WM_USER+1
        return (LRESULT)(wParam * 2);
    }
    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

// Pumps until the queue is empty, dispatching as the game's loops do.
static void Drain() {
    MSG tMsg;
    while (PeekMessageA(&tMsg, nullptr, 0, 0, PM_REMOVE)) {
        if (tMsg.message == 0x000F) {  // WM_PAINT: validate it so it is not returned again
            ValidateRect(tMsg.hwnd, nullptr);
            continue;
        }
        TranslateMessage(&tMsg);
        DispatchMessageA(&tMsg);
    }
}

static const Logged* Find(UINT uMsg, int nFrom = 0) {
    for (int i = nFrom; i < s_nLog; i++) {
        if (s_aLog[i].uMsg == uMsg) {
            return &s_aLog[i];
        }
    }
    return nullptr;
}

static void ExpectMsg(UINT uMsg, WPARAM wParam, uint32_t lParam, const char* szWhat) {
    const Logged* p = Find(uMsg);
    for (const Logged* q = p; q; q = Find(uMsg, (int)(q - s_aLog) + 1)) {
        if (q->wParam == wParam) {
            p = q;
            break;
        }
    }
    char sz[256];
    snprintf(sz, sizeof(sz), "%s: %s wParam=0x%llx lParam=0x%08x (got %s wParam=0x%llx lParam=0x%08x)", szWhat,
             MessageName(uMsg), (unsigned long long)wParam, lParam, p ? "it" : "none",
             p ? (unsigned long long)p->wParam : 0ull, p ? (uint32_t)p->lParam : 0u);
    Check(p && p->wParam == wParam && (uint32_t)p->lParam == lParam, sz);
}

static void Begin(const char* szName) {
    s_nLog = 0;
    printf("-- %s\n", szName);
}

static SDL_WindowID s_nWindowId;

static void PushKey(SDL_Scancode nScan, SDL_Keycode nKey, bool bDown, bool bRepeat = false) {
    SDL_Event e;
    SDL_zero(e);
    e.type = bDown ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    e.key.windowID = s_nWindowId;
    e.key.scancode = nScan;
    e.key.key = nKey;
    e.key.down = bDown;
    e.key.repeat = bRepeat;
    SDL_PushEvent(&e);
}

static void PushText(const char* szUtf8) {
    SDL_Event e;
    SDL_zero(e);
    e.type = SDL_EVENT_TEXT_INPUT;
    e.text.windowID = s_nWindowId;
    e.text.text = szUtf8;
    SDL_PushEvent(&e);
}

static void PushMotion(float fX, float fY) {
    SDL_Event e;
    SDL_zero(e);
    e.type = SDL_EVENT_MOUSE_MOTION;
    e.motion.windowID = s_nWindowId;
    e.motion.x = fX;
    e.motion.y = fY;
    SDL_PushEvent(&e);
}

static void PushButton(Uint8 nButton, bool bDown, float fX, float fY, Uint8 nClicks = 1) {
    SDL_Event e;
    SDL_zero(e);
    e.type = bDown ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
    e.button.windowID = s_nWindowId;
    e.button.button = nButton;
    e.button.down = bDown;
    e.button.clicks = nClicks;
    e.button.x = fX;
    e.button.y = fY;
    SDL_PushEvent(&e);
}

static void PushWheel(float fY, float fX, float fMouseX, float fMouseY) {
    SDL_Event e;
    SDL_zero(e);
    e.type = SDL_EVENT_MOUSE_WHEEL;
    e.wheel.windowID = s_nWindowId;
    e.wheel.y = fY;
    e.wheel.x = fX;
    e.wheel.direction = SDL_MOUSEWHEEL_NORMAL;
    e.wheel.mouse_x = fMouseX;
    e.wheel.mouse_y = fMouseY;
    SDL_PushEvent(&e);
}

static void PushWindowEvent(SDL_EventType nType, Sint32 nData1 = 0, Sint32 nData2 = 0) {
    SDL_Event e;
    SDL_zero(e);
    e.type = nType;
    e.window.windowID = s_nWindowId;
    e.window.data1 = nData1;
    e.window.data2 = nData2;
    SDL_PushEvent(&e);
}

static HWND s_hSendTarget;
static LRESULT s_lSendResult;
static int SendFromThread(void* pArg) {
    (void)pArg;
    s_lSendResult = SendMessageA(s_hSendTarget, 0x0401, 21, 0);
    PostMessageA(s_hSendTarget, 0x0401, 1, 0);
    return 0;
}

static const unsigned kClientWidth = 640, kClientHeight = 480;

static int RunTests() {
    WNDCLASSA tClass;
    memset(&tClass, 0, sizeof(tClass));
    tClass.style = 0x0008;  // CS_DBLCLKS
    tClass.lpfnWndProc = TestWndProc;
    tClass.lpszClassName = "Diablo II";
    tClass.hCursor = LoadCursorA(nullptr, MAKEINTRESOURCEA(0x7f00));
    Check(RegisterClassA(&tClass) != 0, "RegisterClassA");
    Check(RegisterClassA(&tClass) == 0, "RegisterClassA twice fails");

    Begin("CreateWindowExA (WS_OVERLAPPEDWINDOW 640x480) and ShowWindow");
    RECT rcAdjust = {0, 0, (LONG)kClientWidth, (LONG)kClientHeight};
    AdjustWindowRectEx(&rcAdjust, 0xCF0000, FALSE, 0x40000);
    HWND hWnd = CreateWindowExA(0x40000, "Diablo II", "Diablo II", 0xCF0000, 0, 0, rcAdjust.right - rcAdjust.left,
                                rcAdjust.bottom - rcAdjust.top, nullptr, nullptr, nullptr, nullptr);
    Check(hWnd != nullptr, "CreateWindowExA");
    Check(((uintptr_t)hWnd >> 32) == 0, "an HWND is a 32-bit table handle");
    if (!hWnd) {
        return 1;
    }
    Check(s_nLog >= 4 && s_aLog[0].uMsg == 0x0081 && s_aLog[1].uMsg == 0x0001, "WM_NCCREATE then WM_CREATE");
    ExpectMsg(0x0005, 0, 0x01E00280, "WM_SIZE after create");
    ShowWindow(hWnd, 1);
    UpdateWindow(hWnd);
    SetFocus(hWnd);
    Check(Find(0x0018) != nullptr, "ShowWindow sends WM_SHOWWINDOW");
    Check(Find(0x0006) != nullptr && Find(0x0007) != nullptr, "ShowWindow activates and focuses");
    Check(IsWindow(hWnd) && IsWindowVisible(hWnd), "IsWindow, IsWindowVisible");
    RECT rc;
    GetClientRect(hWnd, &rc);
    Check(rc.left == 0 && rc.top == 0 && rc.right == 640 && rc.bottom == 480, "GetClientRect 640x480");
    SDL_Window* pWindow = Win32Posix_GetSdlWindow(hWnd);
    Check(pWindow != nullptr, "an SDL window behind the HWND");
    s_nWindowId = pWindow ? SDL_GetWindowID(pWindow) : 0;
    Drain();

    Begin("keys: a, with text input");
    PushKey(SDL_SCANCODE_A, SDLK_A, true);
    PushText("a");
    PushKey(SDL_SCANCODE_A, SDLK_A, false);
    Drain();
    ExpectMsg(0x0100, 'A', 0x001E0001, "A down");
    ExpectMsg(0x0102, 'a', 0x001E0001, "the character from text input");
    ExpectMsg(0x0101, 'A', 0xC01E0001, "A up");
    Check(s_nLog == 3 && s_aLog[1].uMsg == 0x0102, "KEYDOWN, CHAR, KEYUP in order, no duplicate CHAR");

    Begin("keys: repeat");
    PushKey(SDL_SCANCODE_A, SDLK_A, true);
    PushKey(SDL_SCANCODE_A, SDLK_A, true, true);
    PushKey(SDL_SCANCODE_A, SDLK_A, false);
    Drain();
    Check(s_nLog == 3 && (uint32_t)s_aLog[1].lParam == 0x401E0001, "the repeat has the previous-state bit");

    Begin("keys: layout letter (AZERTY: the A key types q)");
    PushKey(SDL_SCANCODE_A, SDLK_Q, true);
    PushKey(SDL_SCANCODE_A, SDLK_Q, false);
    Drain();
    ExpectMsg(0x0100, 'Q', 0x001E0001, "the layout's letter, the key's scan code");

    Begin("keys: Shift, GetKeyState");
    PushKey(SDL_SCANCODE_LSHIFT, SDLK_LSHIFT, true);
    Drain();
    ExpectMsg(0x0100, 0x10, 0x002A0001, "VK_SHIFT down");
    Check(GetKeyState(0x10) < 0 && GetKeyState(0xA0) < 0 && GetKeyState(0xA1) >= 0, "VK_SHIFT and VK_LSHIFT down");
    Check(GetAsyncKeyState(0x10) & 0x8000, "GetAsyncKeyState VK_SHIFT");
    PushButton(SDL_BUTTON_RIGHT, true, 5, 6);
    PushButton(SDL_BUTTON_RIGHT, false, 5, 6);
    PushKey(SDL_SCANCODE_LSHIFT, SDLK_LSHIFT, false);
    Drain();
    ExpectMsg(0x0204, 0x0002 | 0x0004, 0x00060005, "WM_RBUTTONDOWN with MK_RBUTTON|MK_SHIFT");
    ExpectMsg(0x0101, 0x10, 0xC02A0001, "VK_SHIFT up");
    Check(GetKeyState(0x10) >= 0, "VK_SHIFT up after the message");

    Begin("keys: control characters (Enter, Backspace, Escape, Tab)");
    PushKey(SDL_SCANCODE_RETURN, SDLK_RETURN, true);
    PushKey(SDL_SCANCODE_BACKSPACE, SDLK_BACKSPACE, true);
    PushKey(SDL_SCANCODE_ESCAPE, SDLK_ESCAPE, true);
    PushKey(SDL_SCANCODE_TAB, SDLK_TAB, true);
    Drain();
    Check(s_nLog == 8 && s_aLog[1].uMsg == 0x0102 && s_aLog[1].wParam == 0x0D && s_aLog[3].wParam == 0x08 &&
              s_aLog[5].wParam == 0x1B && s_aLog[7].wParam == 0x09,
          "each KEYDOWN is followed by its WM_CHAR");
    PushKey(SDL_SCANCODE_RETURN, SDLK_RETURN, false);
    PushKey(SDL_SCANCODE_BACKSPACE, SDLK_BACKSPACE, false);
    PushKey(SDL_SCANCODE_ESCAPE, SDLK_ESCAPE, false);
    PushKey(SDL_SCANCODE_TAB, SDLK_TAB, false);
    Drain();

    Begin("keys: extended (arrows, keypad Enter, right Ctrl)");
    PushKey(SDL_SCANCODE_LEFT, SDLK_LEFT, true);
    PushKey(SDL_SCANCODE_LEFT, SDLK_LEFT, false);
    PushKey(SDL_SCANCODE_KP_ENTER, SDLK_KP_ENTER, true);
    PushKey(SDL_SCANCODE_KP_ENTER, SDLK_KP_ENTER, false);
    PushKey(SDL_SCANCODE_RCTRL, SDLK_RCTRL, true);
    Drain();
    ExpectMsg(0x0100, 0x25, 0x014B0001, "VK_LEFT, extended");
    Check(GetKeyState(0xA3) < 0 && GetKeyState(0xA2) >= 0 && GetKeyState(0x11) < 0, "VK_RCONTROL, not VK_LCONTROL");
    PushKey(SDL_SCANCODE_RCTRL, SDLK_RCTRL, false);
    Drain();

    Begin("keys: Ctrl+V is 0x16");
    PushKey(SDL_SCANCODE_LCTRL, SDLK_LCTRL, true);
    PushKey(SDL_SCANCODE_V, SDLK_V, true);
    PushKey(SDL_SCANCODE_V, SDLK_V, false);
    PushKey(SDL_SCANCODE_LCTRL, SDLK_LCTRL, false);
    Drain();
    ExpectMsg(0x0102, 0x16, 0x002F0001, "Ctrl+V");

    Begin("keys: Cmd is VK_LWIN; Cmd-X/C/V/A are the Edit menu's characters");
    PushKey(SDL_SCANCODE_LGUI, SDLK_LGUI, true);
    PushKey(SDL_SCANCODE_C, SDLK_C, true);
    PushText("c");  // a system that still sends text for a Cmd chord: dropped
    PushKey(SDL_SCANCODE_C, SDLK_C, false);
    PushKey(SDL_SCANCODE_V, SDLK_V, true);
    PushKey(SDL_SCANCODE_V, SDLK_V, false);
    PushKey(SDL_SCANCODE_LGUI, SDLK_LGUI, false);
    Drain();
    ExpectMsg(0x0100, 0x5B, 0x015B0001, "VK_LWIN, extended");
    Check(GetKeyState(0x11) >= 0, "Cmd is not Ctrl");
    {
        int nChars = 0;
        for (int i = 0; i < s_nLog; i++) {
            if (s_aLog[i].uMsg == 0x0102) {
                Check(nChars != 0 || s_aLog[i].wParam == 0x03, "Cmd-C is 0x03");
                Check(nChars != 1 || s_aLog[i].wParam == 0x16, "Cmd-V is 0x16");
                nChars++;
            }
        }
        Check(nChars == 2, "two characters, no text for the chord");
    }

    Begin("keys: Alt (Option) and Alt+F4");
    PushKey(SDL_SCANCODE_LALT, SDLK_LALT, true);
    PushKey(SDL_SCANCODE_F4, SDLK_F4, true);
    PushKey(SDL_SCANCODE_F4, SDLK_F4, false);
    PushKey(SDL_SCANCODE_LALT, SDLK_LALT, false);
    Drain();
    ExpectMsg(0x0104, 0x12, 0x20380001, "VK_MENU is WM_SYSKEYDOWN with the Alt bit");
    ExpectMsg(0x0105, 0x73, 0xE03E0001, "F4 up while Alt is down is WM_SYSKEYUP");
    ExpectMsg(0x0112, 0xF060, 0, "DefWindowProc turns Alt+F4 into SC_CLOSE");
    Check(Find(0x0010) != nullptr, "and SC_CLOSE into WM_CLOSE");
    ExpectMsg(0x0105, 0x12, 0xC0380001, "VK_MENU up");

    Begin("keys: F10 is a system key");
    PushKey(SDL_SCANCODE_F10, SDLK_F10, true);
    PushKey(SDL_SCANCODE_F10, SDLK_F10, false);
    Drain();
    ExpectMsg(0x0104, 0x79, 0x00440001, "F10 down");

    Begin("text: Windows-1252");
    PushText("\xC3\xA9\xE2\x82\xAC\xE4\xB8\xAD");  // e-acute, the euro sign, a CJK character
    Drain();
    Check(s_nLog == 3 && s_aLog[0].wParam == 0xE9 && s_aLog[1].wParam == 0x80 && s_aLog[2].wParam == '?',
          "0xE9, 0x80 and '?' for what 1252 lacks");

    Begin("mouse: move, buttons, double click, wheel, X buttons");
    PushMotion(100.6f, 50.2f);
    PushMotion(101.0f, 51.0f);  // coalesces with the one before
    Drain();
    Check(s_nLog == 1, "two moves coalesce into one WM_MOUSEMOVE");
    ExpectMsg(0x0200, 0, 0x00330065, "WM_MOUSEMOVE (101,51)");
    PushButton(SDL_BUTTON_LEFT, true, 10, 20);
    PushButton(SDL_BUTTON_LEFT, false, 10, 20);
    PushButton(SDL_BUTTON_LEFT, true, 10, 20, 2);
    PushButton(SDL_BUTTON_LEFT, false, 10, 20, 2);
    Drain();
    ExpectMsg(0x0201, 0x0001, 0x0014000A, "WM_LBUTTONDOWN with MK_LBUTTON");
    ExpectMsg(0x0202, 0, 0x0014000A, "WM_LBUTTONUP");
    ExpectMsg(0x0203, 0x0001, 0x0014000A, "WM_LBUTTONDBLCLK (CS_DBLCLKS)");
    POINT pt;
    GetCursorPos(&pt);
    POINT ptClient = pt;
    ScreenToClient(hWnd, &ptClient);
    Check(ptClient.x == 10 && ptClient.y == 20, "GetCursorPos follows the events");
    Begin("mouse: wheel");
    PushWheel(1.0f, 0.0f, 10, 20);
    PushWheel(0.5f, 0.0f, 10, 20);
    PushWheel(0.5f, 0.0f, 10, 20);
    PushWheel(-1.0f, 0.0f, 10, 20);
    Drain();
    Check(s_nLog == 3, "1, 0.5+0.5, -1: three notches");
    Check(s_nLog >= 1 && s_aLog[0].uMsg == 0x020A && (short)HIWORD(s_aLog[0].wParam) == 120 &&
              (uint32_t)s_aLog[0].lParam == (uint32_t)MAKELPARAM(pt.x, pt.y),
          "WM_MOUSEWHEEL +120 at the screen point");
    Check(s_nLog >= 3 && (short)HIWORD(s_aLog[2].wParam) == -120, "WM_MOUSEWHEEL -120");
    Begin("mouse: X buttons");
    PushButton(SDL_BUTTON_X1, true, 10, 20);
    PushButton(SDL_BUTTON_X1, false, 10, 20);
    Drain();
    ExpectMsg(0x020B, 0x00010020, 0x0014000A, "WM_XBUTTONDOWN XBUTTON1, MK_XBUTTON1");

    Begin("mouse: capture and TrackMouseEvent");
    SetCapture(hWnd);
    Check(GetCapture() == hWnd, "GetCapture");
    PushMotion(-5.0f, 700.0f);
    Drain();
    ExpectMsg(0x0200, 0, (uint32_t)MAKELPARAM(-5, 700), "captured moves run outside the client");
    ReleaseCapture();
    struct {
        DWORD cbSize;
        DWORD dwFlags;
        HWND hwndTrack;
        DWORD dwHoverTime;
    } tTrack = {sizeof(tTrack), 2, hWnd, 0};
    TrackMouseEvent((struct tagTRACKMOUSEEVENT*)&tTrack);
    PushWindowEvent(SDL_EVENT_WINDOW_MOUSE_LEAVE);
    PushWindowEvent(SDL_EVENT_WINDOW_MOUSE_LEAVE);
    Drain();
    Check(Find(0x02A3) && !Find(0x02A3, (int)(Find(0x02A3) - s_aLog) + 1), "WM_MOUSELEAVE once");

    Begin("resize: the player resizes a WS_THICKFRAME window");
    SDL_SetWindowSize(pWindow, 800, 600);
    SDL_SyncWindow(pWindow);
    Drain();
    if (!Find(0x0005)) {
        PushWindowEvent(SDL_EVENT_WINDOW_RESIZED, 800, 600);  // a driver that does not report it
        Drain();
    }
    ExpectMsg(0x0005, 0, 0x02580320, "WM_SIZE 800x600");
    GetClientRect(hWnd, &rc);
    Check(rc.right == 800 && rc.bottom == 600, "the client follows");

    Begin("resize: SetWindowPos");
    SetWindowPos(hWnd, (HWND)(intptr_t)-2, 0, 0, 640, 480, 0x16);
    ExpectMsg(0x0005, 0, 0x01E00280, "WM_SIZE 640x480, sent");
    Drain();

    Begin("scaling: a window without WS_THICKFRAME keeps its client");
    SetWindowLongA(hWnd, -16, (LONG)(GetWindowLongA(hWnd, -16) & ~0x40000));
    SDL_SetWindowSize(pWindow, 1280, 960);
    SDL_SyncWindow(pWindow);
    Drain();
    PushWindowEvent(SDL_EVENT_WINDOW_RESIZED, 1280, 960);
    PushMotion(640.0f, 480.0f);
    Drain();
    Check(!Find(0x0005), "no WM_SIZE");
    int nWinW = 0, nWinH = 0;
    SDL_GetWindowSize(pWindow, &nWinW, &nWinH);
    if (nWinW == 1280 && nWinH == 960) {
        ExpectMsg(0x0200, 0, 0x00F00140, "the window's centre is the client's centre (320,240)");
        int nX, nY, nW, nH;
        Win32Posix_GetViewport(hWnd, &nX, &nY, &nW, &nH);
        float fDensity = SDL_GetWindowPixelDensity(pWindow);
        Check(nX == 0 && nY == 0 && nW == (int)(1280 * fDensity) && nH == (int)(960 * fDensity), "the viewport fills it");
    } else {
        printf("   (the driver kept the window at %dx%d; the scale check is skipped)\n", nWinW, nWinH);
    }
    SetWindowPos(hWnd, nullptr, 0, 0, 640, 480, 0x16);
    SDL_SetWindowSize(pWindow, 640, 480);
    SDL_SyncWindow(pWindow);
    Drain();

    Begin("focus");
    PushWindowEvent(SDL_EVENT_WINDOW_FOCUS_LOST);
    PushWindowEvent(SDL_EVENT_WINDOW_FOCUS_GAINED);
    Drain();
    ExpectMsg(0x001C, 0, 0, "WM_ACTIVATEAPP FALSE");
    Check(Find(0x0008) && Find(0x001C, (int)(Find(0x001C) - s_aLog) + 1) &&
              Find(0x001C, (int)(Find(0x001C) - s_aLog) + 1)->wParam == 1,
          "WM_KILLFOCUS, then WM_ACTIVATEAPP TRUE");
    Check(Find(0x0007) != nullptr, "WM_SETFOCUS");
    Check(GetFocus() == hWnd && GetActiveWindow() == hWnd, "focus and activation are back");

    Begin("timers");
    SetTimer(hWnd, 7, 20, nullptr);
    MSG tMsg;
    Uint64 nStart = SDL_GetTicks();
    bool bTimer = false;
    while (!bTimer && SDL_GetTicks() - nStart < 2000) {
        if (GetMessageA(&tMsg, nullptr, 0, 0)) {
            bTimer = tMsg.message == 0x0113 && tMsg.wParam == 7;
            if (tMsg.message == 0x000F) {
                ValidateRect(tMsg.hwnd, nullptr);
            }
            DispatchMessageA(&tMsg);
        }
    }
    Check(bTimer, "WM_TIMER 7 through GetMessageA");
    KillTimer(hWnd, 7);

    Begin("SendMessageA from another thread");
    s_hSendTarget = hWnd;
    SDL_Thread* pThread = SDL_CreateThread(SendFromThread, "sender", nullptr);
    nStart = SDL_GetTicks();
    bool bPosted = false;
    while (!bPosted && SDL_GetTicks() - nStart < 2000) {
        if (GetMessageA(&tMsg, nullptr, 0, 0)) {
            bPosted = tMsg.message == 0x0401;
            DispatchMessageA(&tMsg);
        }
    }
    SDL_WaitThread(pThread, nullptr);
    Check(s_lSendResult == 42, "the owner thread delivered it and the result came back");
    Check(bPosted, "the posted message woke GetMessageA");

    Begin("window close: the close button");
    PushWindowEvent(SDL_EVENT_WINDOW_CLOSE_REQUESTED);
    Drain();
    Check(Find(0x0010) != nullptr, "WM_CLOSE");

    Begin("display");
    int nDesktopWidth = GetSystemMetrics(0), nDesktopHeight = GetSystemMetrics(1);
    printf("   screen %dx%d, caption %d, monitors %d\n", nDesktopWidth, nDesktopHeight, GetSystemMetrics(4),
           GetSystemMetrics(80));
    Check(nDesktopWidth > 0 && nDesktopHeight > 0 && GetSystemMetrics(4) == 0, "GetSystemMetrics");
    DEVMODEA tMode;
    memset(&tMode, 0, sizeof(tMode));
    int nModes = 0;
    bool b800 = false;
    while (EnumDisplaySettingsA(nullptr, (DWORD)nModes, &tMode)) {
        b800 |= tMode.dmPelsWidth == 800 && tMode.dmPelsHeight == 600;
        nModes++;
    }
    Check(nModes >= 3 && b800, "EnumDisplaySettingsA lists 800x600");
    memset(&tMode, 0, sizeof(tMode));
    tMode.dmSize = sizeof(tMode);
    tMode.dmFields = 0x40000 | 0x80000 | 0x100000;
    tMode.dmBitsPerPel = 32;
    tMode.dmPelsWidth = 800;
    tMode.dmPelsHeight = 600;
    Check(ChangeDisplaySettingsA(&tMode, 4) == 0, "ChangeDisplaySettingsA 800x600");
    Check(GetSystemMetrics(0) == 800 && GetSystemMetrics(1) == 600, "the screen is 800x600 now");
    HWND hFull = CreateWindowExA(0x40008, "Diablo II", "fullscreen", 0x80080000, 0, 0, 800, 600, nullptr, nullptr,
                                 nullptr, nullptr);
    ShowWindow(hFull, 1);
    SDL_Window* pFull = Win32Posix_GetSdlWindow(hFull);
    SDL_SyncWindow(pFull);
    Check(pFull && (SDL_GetWindowFlags(pFull) & SDL_WINDOW_FULLSCREEN), "a popup over the screen is fullscreen");
    Check(SDL_GetWindowFullscreenMode(pFull) == nullptr, "desktop fullscreen, no mode switch");
    GetClientRect(hFull, &rc);
    Check(rc.right == 800 && rc.bottom == 600, "its client stays 800x600");
    DestroyWindow(hFull);
    ChangeDisplaySettingsA(nullptr, 0);
    Check(GetSystemMetrics(0) == nDesktopWidth, "ChangeDisplaySettingsA(NULL) restores");
    Check(MessageBoxA(hWnd, "a test message", "Win32Posix", 0x24) == 6, "MessageBoxA's default button, headless");
    Drain();

    Begin("clipboard");
    if (OpenClipboard(hWnd)) {
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(0x2000, 8);
        strcpy((char*)GlobalLock(hMem), "caf\xE9");
        GlobalUnlock(hMem);
        Check(SetClipboardData(CF_TEXT, hMem) != nullptr, "SetClipboardData");
        CloseClipboard();
        GlobalFree(hMem);  // as D2WINEDITBOX_SetClipboardText does, later
        char* szUtf8 = SDL_GetClipboardText();
        Check(szUtf8 && strcmp(szUtf8, "caf\xC3\xA9") == 0, "SDL holds it as UTF-8");
        SDL_free(szUtf8);
        Check(IsClipboardFormatAvailable(CF_TEXT), "IsClipboardFormatAvailable");
        OpenClipboard(hWnd);
        HANDLE hData = GetClipboardData(CF_TEXT);
        Check(hData && strcmp((char*)GlobalLock(hData), "caf\xE9") == 0, "GetClipboardData gives 1252 back");
        CloseClipboard();
    } else {
        Check(false, "OpenClipboard");
    }

    Begin("rectangles");
    RECT a = {0, 0, 10, 10}, b = {5, 5, 20, 20}, c;
    Check(IntersectRect(&c, &a, &b) && c.left == 5 && c.right == 10, "IntersectRect");
    Check(UnionRect(&c, &a, &b) && c.left == 0 && c.bottom == 20, "UnionRect");
    Check(PtInRect(&a, POINT{0, 0}) && !PtInRect(&a, POINT{10, 5}), "PtInRect: right edge outside");
    SetRect(&c, 1, 2, 3, 4);
    Check(c.left == 1 && c.bottom == 4, "SetRect");

    Begin("quit: DestroyWindow and GetMessageA");
    s_bCloseDestroys = true;
    PostMessageA(hWnd, 0x0010, 0, 0);
    int nGot = 0;
    while (GetMessageA(&tMsg, nullptr, 0, 0)) {
        TranslateMessage(&tMsg);
        DispatchMessageA(&tMsg);
        if (++nGot > 1000) {
            break;
        }
    }
    Check(tMsg.message == 0x0012, "GetMessageA returns FALSE on WM_QUIT");
    Check(Find(0x0002) && Find(0x0082), "WM_DESTROY, WM_NCDESTROY");
    Check(!IsWindow(hWnd), "the handle is gone");
    Check(UnregisterClassA("Diablo II", nullptr), "UnregisterClassA");

    printf("%d checks, %d failed\n", s_nChecks, s_nFailures);
    return s_nFailures ? 1 : 0;
}

// A real window: every message logged until it is closed.
static int RunLive() {
    WNDCLASSA tClass;
    memset(&tClass, 0, sizeof(tClass));
    tClass.style = 0x0008;
    tClass.lpfnWndProc = TestWndProc;
    tClass.lpszClassName = "Diablo II";
    tClass.hCursor = LoadCursorA(nullptr, MAKEINTRESOURCEA(0x7f00));
    RegisterClassA(&tClass);
    s_bCloseDestroys = true;
    HWND hWnd = CreateWindowExA(0x40000, "Diablo II", "Win32Posix: type, click, resize, close", 0xCF0000, 0, 0, 640,
                                480, nullptr, nullptr, nullptr, nullptr);
    ShowWindow(hWnd, 1);
    MSG tMsg;
    while (GetMessageA(&tMsg, nullptr, 0, 0)) {
        if (tMsg.message == 0x000F) {
            ValidateRect(tMsg.hwnd, nullptr);
            continue;
        }
        TranslateMessage(&tMsg);
        DispatchMessageA(&tMsg);
        s_nLog = 0;
    }
    return 0;
}

int main(int argc, char** argv) {
    Win32Posix_SetWindowCreateFlags(0);  // no GL context is made here
    if (argc > 1 && strcmp(argv[1], "--live") == 0) {
        return RunLive();
    }
    s_bQuiet = argc > 1 && strcmp(argv[1], "--quiet") == 0;
    return RunTests();
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
