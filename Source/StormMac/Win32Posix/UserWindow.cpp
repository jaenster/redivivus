// Win32Posix: window classes and windows on SDL3 (Win32Mac: Win32_Window.cpp, Win32_WndProcs.cpp).
//
// A top-level window is an SDL window; a child window is a table entry only. A window keeps the
// client size the game gave it (CreateWindowExA, SetWindowPos) whatever size the SDL window
// has: a borderless desktop-fullscreen window, or one the player resized, shows the client
// scaled with its aspect kept. Only a WS_THICKFRAME window follows the player's resize with
// WM_SIZE. There is no non-client area: AdjustWindowRectEx leaves the rectangle as it is, so
// the size the game asks for is the size its client gets.
#include "Source/StormMac/Win32Posix/UserInternal.h"

#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <stdio.h>
#include <string.h>

namespace Win32Posix {

State& G() {
    static State* pState = [] {
        State* p = (State*)SDL_calloc(1, sizeof(State));
        p->pLock = SDL_CreateMutex();
        p->pSentDone = SDL_CreateCondition();
        return p;
    }();
    return *pState;
}

static void LogSdlError(const char* szWhat) {
    fprintf(stderr, "Win32Posix: %s: %s\n", szWhat, SDL_GetError());
}

// SDL is brought up on first use, on the thread that uses it first: that thread owns the
// windows and pumps the events, as the thread that creates a window owns its queue on Windows.
bool EnsureSdl() {
    State& g = G();
    if (g.bSdlReady) {
        return true;
    }
    SDL_SetHint(SDL_HINT_QUIT_ON_LAST_WINDOW_CLOSE, "0");
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
    // Full screen is a borderless window over the desktop in place, not a macOS fullscreen Space:
    // moving to a Space takes the window's focus away, which a full-screen game reads as being
    // switched away from (WM_ACTIVATEAPP) and answers by minimising.
    SDL_SetHint(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES, "0");
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        LogSdlError("SDL_InitSubSystem");
        return false;
    }
    g.bSdlReady = true;
    g.nPumpThread = SDL_GetCurrentThreadID();
    g.nWakeEvent = SDL_RegisterEvents(1);
    if (!g.bCreateFlagsSet) {
        g.nCreateFlags = SDL_WINDOW_OPENGL;
    }
    if (!g.hCursor) {
        g.hCursor = LoadCursorA(nullptr, MAKEINTRESOURCEA(32512));  // IDC_ARROW
    }
    return true;
}

static void EnsureDesktop() {
    State& g = G();
    Wnd& tDesktop = g.aWindows[kDesktopSlot];
    if (tDesktop.bUsed) {
        return;
    }
    tDesktop.bUsed = true;
    tDesktop.dwStyle = kWsVisible | kWsPopup;
    int nWidth, nHeight;
    ScreenSize(&nWidth, &nHeight);
    tDesktop.rcWindow = {0, 0, nWidth, nHeight};
    tDesktop.nClientWidth = nWidth;
    tDesktop.nClientHeight = nHeight;
    strncpy(tDesktop.szTitle, "Desktop", sizeof(tDesktop.szTitle) - 1);
}

Wnd* WndFromHandle(HWND hWnd) {
    if (!HandleHasTag(hWnd, kHwndTag)) {
        return nullptr;
    }
    uint32_t nSlot = HandleSlot(hWnd);
    if (nSlot == kDesktopSlot) {
        EnsureDesktop();
    }
    if (nSlot == 0 || nSlot >= kMaxWindows || !G().aWindows[nSlot].bUsed) {
        return nullptr;
    }
    return &G().aWindows[nSlot];
}

HWND HandleFromWnd(const Wnd* pWnd) {
    if (!pWnd) {
        return nullptr;
    }
    return (HWND)MakeHandle(kHwndTag, (uint32_t)(pWnd - G().aWindows));
}

HWND WndFromSdlId(SDL_WindowID nId) {
    if (nId == 0) {
        return nullptr;
    }
    for (uint32_t i = kDesktopSlot + 1; i < kMaxWindows; i++) {
        const Wnd& w = G().aWindows[i];
        if (w.bUsed && w.pSdlWindow && w.nSdlId == nId) {
            return (HWND)MakeHandle(kHwndTag, i);
        }
    }
    return nullptr;
}

// The window the game draws into: the first visible top-level window.
HWND MainWindow() {
    HWND hFirst = nullptr;
    for (uint32_t i = kDesktopSlot + 1; i < kMaxWindows; i++) {
        const Wnd& w = G().aWindows[i];
        if (w.bUsed && !(w.dwStyle & kWsChild)) {
            if (w.dwStyle & kWsVisible) {
                return (HWND)MakeHandle(kHwndTag, i);
            }
            if (!hFirst) {
                hFirst = (HWND)MakeHandle(kHwndTag, i);
            }
        }
    }
    return hFirst;
}

LRESULT CallProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd || !pWnd->pfnWndProc) {
        return 0;
    }
    return pWnd->pfnWndProc(hWnd, uMsg, wParam, lParam);
}

// ---- the client area and the SDL window ----

static void SdlPointSize(const Wnd* pWnd, int* pnWidth, int* pnHeight) {
    *pnWidth = pWnd->nClientWidth;
    *pnHeight = pWnd->nClientHeight;
    if (pWnd->pSdlWindow) {
        SDL_GetWindowSize(pWnd->pSdlWindow, pnWidth, pnHeight);
    }
}

// The client's scale and offset inside the SDL window (points).
static void ClientFit(const Wnd* pWnd, float* pfScale, float* pfX, float* pfY) {
    int nWidth, nHeight;
    SdlPointSize(pWnd, &nWidth, &nHeight);
    float fScale = 1.0f;
    if (pWnd->nClientWidth > 0 && pWnd->nClientHeight > 0 && nWidth > 0 && nHeight > 0) {
        float fScaleX = (float)nWidth / (float)pWnd->nClientWidth;
        float fScaleY = (float)nHeight / (float)pWnd->nClientHeight;
        fScale = fScaleX < fScaleY ? fScaleX : fScaleY;
    }
    *pfScale = fScale;
    *pfX = ((float)nWidth - (float)pWnd->nClientWidth * fScale) * 0.5f;
    *pfY = ((float)nHeight - (float)pWnd->nClientHeight * fScale) * 0.5f;
}

void SdlToClient(const Wnd* pWnd, float fX, float fY, int* pnX, int* pnY) {
    float fScale, fOffX, fOffY;
    ClientFit(pWnd, &fScale, &fOffX, &fOffY);
    float fCx = (fX - fOffX) / fScale;
    float fCy = (fY - fOffY) / fScale;
    *pnX = (int)(fCx < 0.0f ? fCx - 0.999f : fCx);
    *pnY = (int)(fCy < 0.0f ? fCy - 0.999f : fCy);
}

void ClientToSdl(const Wnd* pWnd, int nX, int nY, float* pfX, float* pfY) {
    float fScale, fOffX, fOffY;
    ClientFit(pWnd, &fScale, &fOffX, &fOffY);
    *pfX = fOffX + ((float)nX + 0.5f) * fScale;
    *pfY = fOffY + ((float)nY + 0.5f) * fScale;
}

// The screen position of the client's top-left corner. A fullscreen window is the whole
// (reported) screen, so its client starts at 0,0.
void ClientOrigin(const Wnd* pWnd, POINT* pt) {
    if (pWnd->pSdlWindow && !pWnd->bFullscreen) {
        int nX = pWnd->rcWindow.left, nY = pWnd->rcWindow.top;
        SDL_GetWindowPosition(pWnd->pSdlWindow, &nX, &nY);
        pt->x = nX;
        pt->y = nY;
        return;
    }
    pt->x = pWnd->rcWindow.left;
    pt->y = pWnd->rcWindow.top;
}

static bool CoversScreen(const Wnd* pWnd) {
    int nWidth, nHeight;
    ScreenSize(&nWidth, &nHeight);
    return pWnd->rcWindow.left <= 0 && pWnd->rcWindow.top <= 0 && pWnd->rcWindow.right >= nWidth &&
           pWnd->rcWindow.bottom >= nHeight;
}

// A top-level popup over the whole reported screen is the fullscreen window: SDL shows it as
// a borderless desktop-fullscreen window, never as a mode switch.
void UpdateFullscreen(Wnd* pWnd) {
    bool bWant = !(pWnd->dwStyle & kWsChild) && (pWnd->dwStyle & kWsPopup) && CoversScreen(pWnd);
    if (bWant == pWnd->bFullscreen) {
        return;
    }
    pWnd->bFullscreen = bWant;
    if (!pWnd->pSdlWindow) {
        return;
    }
    if (bWant) {
        SDL_SetWindowFullscreenMode(pWnd->pSdlWindow, nullptr);
        SDL_SetWindowFullscreen(pWnd->pSdlWindow, true);
    } else {
        SDL_SetWindowFullscreen(pWnd->pSdlWindow, false);
        SDL_SetWindowSize(pWnd->pSdlWindow, pWnd->nClientWidth, pWnd->nClientHeight);
    }
}

void UpdateAllFullscreen() {
    for (uint32_t i = kDesktopSlot + 1; i < kMaxWindows; i++) {
        if (G().aWindows[i].bUsed) {
            UpdateFullscreen(&G().aWindows[i]);
        }
    }
}

static void SendSizeAndMove(HWND hWnd, Wnd* pWnd, bool bSized, bool bMoved) {
    if (bSized) {
        WPARAM nKind = kSizeRestored;
        if (pWnd->pSdlWindow) {
            SDL_WindowFlags nFlags = SDL_GetWindowFlags(pWnd->pSdlWindow);
            nKind = (nFlags & SDL_WINDOW_MINIMIZED) ? kSizeMinimized
                  : (nFlags & SDL_WINDOW_MAXIMIZED) ? kSizeMaximized
                                                    : kSizeRestored;
        }
        CallProc(hWnd, kWmSize, nKind, MAKELPARAM(pWnd->nClientWidth, pWnd->nClientHeight));
    }
    if (bMoved) {
        POINT pt;
        ClientOrigin(pWnd, &pt);
        CallProc(hWnd, kWmMove, 0, MAKELPARAM(pt.x, pt.y));
    }
}

// The player resized the SDL window (points).
void OnSdlResized(HWND hWnd, int nWidth, int nHeight) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd || pWnd->bFullscreen || !(pWnd->dwStyle & kWsThickFrame)) {
        return;  // the client keeps its size and is scaled
    }
    if (nWidth == pWnd->nClientWidth && nHeight == pWnd->nClientHeight) {
        return;
    }
    pWnd->nClientWidth = nWidth;
    pWnd->nClientHeight = nHeight;
    pWnd->rcWindow.right = pWnd->rcWindow.left + nWidth;
    pWnd->rcWindow.bottom = pWnd->rcWindow.top + nHeight;
    Post(hWnd, kWmSize, kSizeRestored, MAKELPARAM(nWidth, nHeight));
}

void OnSdlMoved(HWND hWnd) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd || pWnd->bFullscreen || !pWnd->pSdlWindow) {
        return;
    }
    POINT pt;
    ClientOrigin(pWnd, &pt);
    if (pt.x == pWnd->rcWindow.left && pt.y == pWnd->rcWindow.top) {
        return;
    }
    pWnd->rcWindow = {pt.x, pt.y, pt.x + pWnd->nClientWidth, pt.y + pWnd->nClientHeight};
    Post(hWnd, kWmMove, 0, MAKELPARAM(pt.x, pt.y));
}

}  // namespace Win32Posix

using namespace Win32Posix;

// ---- classes ----

static uint32_t FindClass(LPCSTR szName) {
    if (!szName) {
        return 0;
    }
    if ((uintptr_t)szName < 0x10000) {  // an atom
        uint32_t nSlot = (uint32_t)(uintptr_t)szName - kAtomBase;
        return nSlot < kMaxClasses && G().aClasses[nSlot].bUsed ? nSlot : 0;
    }
    for (uint32_t i = 1; i < kMaxClasses; i++) {
        if (G().aClasses[i].bUsed && strcasecmp(G().aClasses[i].szName, szName) == 0) {
            return i;
        }
    }
    return 0;
}

ATOM RegisterClassA(const WNDCLASSA* lpWndClass) {
    if (!lpWndClass || !lpWndClass->lpszClassName || FindClass(lpWndClass->lpszClassName)) {
        SetLastError(1410);  // ERROR_CLASS_ALREADY_EXISTS
        return 0;
    }
    for (uint32_t i = 1; i < kMaxClasses; i++) {
        WndClass& c = G().aClasses[i];
        if (!c.bUsed) {
            c = WndClass();
            c.bUsed = true;
            c.tClass = *lpWndClass;
            strncpy(c.szName, lpWndClass->lpszClassName, sizeof(c.szName) - 1);
            c.tClass.lpszClassName = c.szName;
            c.tClass.lpszMenuName = nullptr;
            return (ATOM)(kAtomBase + i);
        }
    }
    SetLastError(8);  // ERROR_NOT_ENOUGH_MEMORY
    return 0;
}

BOOL UnregisterClassA(LPCSTR lpClassName, HINSTANCE hInstance) {
    (void)hInstance;
    uint32_t nSlot = FindClass(lpClassName);
    if (!nSlot) {
        SetLastError(1411);  // ERROR_CLASS_DOES_NOT_EXIST
        return FALSE;
    }
    if (G().aClasses[nSlot].nWindows > 0) {
        SetLastError(1412);  // ERROR_CLASS_HAS_WINDOWS
        return FALSE;
    }
    G().aClasses[nSlot].bUsed = false;
    return TRUE;
}

// ---- windows ----

void Win32Posix_SetWindowCreateFlags(uint64_t nFlags) {
    G().nCreateFlags = nFlags;
    G().bCreateFlagsSet = true;
}

SDL_Window* Win32Posix_GetSdlWindow(HWND hWnd) {
    Wnd* pWnd = WndFromHandle(hWnd);
    return pWnd ? pWnd->pSdlWindow : nullptr;
}

BOOL Win32Posix_GetViewport(HWND hWnd, int* pnX, int* pnY, int* pnWidth, int* pnHeight) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd) {
        return FALSE;
    }
    float fScale, fOffX, fOffY;
    ClientFit(pWnd, &fScale, &fOffX, &fOffY);
    float fDensity = pWnd->pSdlWindow ? SDL_GetWindowPixelDensity(pWnd->pSdlWindow) : 1.0f;
    if (fDensity <= 0.0f) {
        fDensity = 1.0f;
    }
    *pnX = (int)(fOffX * fDensity + 0.5f);
    *pnY = (int)(fOffY * fDensity + 0.5f);
    *pnWidth = (int)((float)pWnd->nClientWidth * fScale * fDensity + 0.5f);
    *pnHeight = (int)((float)pWnd->nClientHeight * fScale * fDensity + 0.5f);
    return TRUE;
}

static SDL_Window* CreateSdlWindow(Wnd* pWnd) {
    if (!EnsureSdl()) {
        return nullptr;
    }
    char szTitle[sizeof(pWnd->szTitle) * 2];
    AnsiToUtf8(pWnd->szTitle, szTitle, sizeof(szTitle));
    SDL_WindowFlags nFlags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    int nWidth = pWnd->nClientWidth > 0 ? pWnd->nClientWidth : 1;
    int nHeight = pWnd->nClientHeight > 0 ? pWnd->nClientHeight : 1;
    SDL_Window* pWindow = SDL_CreateWindow(szTitle, nWidth, nHeight, nFlags | G().nCreateFlags);
    if (!pWindow && G().nCreateFlags) {
        LogSdlError("SDL_CreateWindow with the renderer's flags");
        pWindow = SDL_CreateWindow(szTitle, nWidth, nHeight, nFlags);
    }
    if (!pWindow) {
        LogSdlError("SDL_CreateWindow");
        return nullptr;
    }
    if (pWnd->rcWindow.left != 0 || pWnd->rcWindow.top != 0) {
        SDL_SetWindowPosition(pWindow, pWnd->rcWindow.left, pWnd->rcWindow.top);
    } else {
        SDL_SetWindowPosition(pWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
    return pWindow;
}

HWND CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y,
                     int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    uint32_t nClass = FindClass(lpClassName);
    if (!nClass) {
        SetLastError(1407);  // ERROR_CANNOT_FIND_WND_CLASS
        return nullptr;
    }
    Wnd* pParent = hWndParent ? WndFromHandle(hWndParent) : nullptr;
    if ((dwStyle & kWsChild) && !pParent) {
        SetLastError(1406);  // ERROR_TLW_WITH_WSCHILD
        return nullptr;
    }
    uint32_t nSlot = 0;
    for (uint32_t i = kDesktopSlot + 1; i < kMaxWindows; i++) {
        if (!G().aWindows[i].bUsed) {
            nSlot = i;
            break;
        }
    }
    if (!nSlot) {
        SetLastError(1158);  // ERROR_NO_MORE_USER_HANDLES
        return nullptr;
    }
    WndClass& c = G().aClasses[nClass];
    Wnd& w = G().aWindows[nSlot];
    w = Wnd();
    w.bUsed = true;
    w.nClass = nClass;
    w.pfnWndProc = c.tClass.lpfnWndProc;
    w.dwStyle = dwStyle & ~kWsVisible;
    w.dwExStyle = dwExStyle;
    w.hParent = (dwStyle & kWsChild) ? hWndParent : nullptr;
    w.nId = (LONG_PTR)hMenu;
    w.hInstance = hInstance;
    w.nExtra = c.tClass.cbWndExtra > 0 ? c.tClass.cbWndExtra : 0;
    if (w.nExtra > (int)sizeof(w.abExtra)) {
        w.nExtra = (int)sizeof(w.abExtra);  // more than any of the game's classes asks for
    }
    strncpy(w.szTitle, lpWindowName ? lpWindowName : "", sizeof(w.szTitle) - 1);
    // CW_USEDEFAULT (0x80000000) places and sizes it as the system likes.
    const int nUseDefault = (int)0x80000000u;
    if (X == nUseDefault) {
        X = Y = 0;
    }
    if (nWidth == nUseDefault) {
        nWidth = 640;
        nHeight = 480;
    }
    POINT ptParent = {0, 0};
    if (pParent && (dwStyle & kWsChild)) {
        ClientOrigin(pParent, &ptParent);
    }
    w.rcWindow = {ptParent.x + X, ptParent.y + Y, ptParent.x + X + nWidth, ptParent.y + Y + nHeight};
    w.nClientWidth = nWidth;
    w.nClientHeight = nHeight;
    c.nWindows++;
    HWND hWnd = (HWND)MakeHandle(kHwndTag, nSlot);
    if (!(dwStyle & kWsChild)) {
        w.pSdlWindow = CreateSdlWindow(&w);
        w.nSdlId = w.pSdlWindow ? SDL_GetWindowID(w.pSdlWindow) : 0;
        UpdateFullscreen(&w);
    }

    CREATESTRUCTA tCreate;
    tCreate.lpCreateParams = lpParam;
    tCreate.hInstance = hInstance;
    tCreate.hMenu = hMenu;
    tCreate.hwndParent = hWndParent;
    tCreate.cy = nHeight;
    tCreate.cx = nWidth;
    tCreate.y = Y;
    tCreate.x = X;
    tCreate.style = (LONG)dwStyle;
    tCreate.lpszName = w.szTitle;
    tCreate.lpszClass = c.szName;
    tCreate.dwExStyle = dwExStyle;
    if (!CallProc(hWnd, kWmNcCreate, 0, (LPARAM)&tCreate) || CallProc(hWnd, kWmCreate, 0, (LPARAM)&tCreate) == -1) {
        DestroyWindow(hWnd);
        return nullptr;
    }
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd) {
        return nullptr;  // destroyed while it was being created
    }
    SendSizeAndMove(hWnd, pWnd, true, true);
    if (dwStyle & kWsVisible) {
        ShowWindow(hWnd, (dwStyle & kWsMaximize) ? 3 : (dwStyle & kWsMinimize) ? 6 : 1);
    }
    return hWnd;
}

BOOL DestroyWindow(HWND hWnd) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd || HandleSlot(hWnd) == kDesktopSlot) {
        SetLastError(1400);  // ERROR_INVALID_WINDOW_HANDLE
        return FALSE;
    }
    State& g = G();
    if (g.hCapture == hWnd) {
        ReleaseCapture();
    }
    CallProc(hWnd, kWmDestroy, 0, 0);
    for (uint32_t i = kDesktopSlot + 1; i < kMaxWindows; i++) {
        if (g.aWindows[i].bUsed && g.aWindows[i].hParent == hWnd) {
            DestroyWindow((HWND)MakeHandle(kHwndTag, i));
        }
    }
    CallProc(hWnd, kWmNcDestroy, 0, 0);
    pWnd = WndFromHandle(hWnd);
    if (!pWnd) {
        return TRUE;
    }
    RemoveWindowMessages(hWnd);
    for (uint32_t i = 0; i < g.nTimers;) {
        if (g.aTimers[i].hWnd == hWnd) {
            g.aTimers[i] = g.aTimers[--g.nTimers];
        } else {
            i++;
        }
    }
    if (g.hFocus == hWnd) {
        g.hFocus = nullptr;
    }
    if (g.hActive == hWnd) {
        g.hActive = nullptr;
    }
    if (pWnd->pSdlWindow) {
        SDL_DestroyWindow(pWnd->pSdlWindow);
    }
    g.aClasses[pWnd->nClass].nWindows--;
    *pWnd = Wnd();
    return TRUE;
}

BOOL IsWindow(HWND hWnd) {
    return WndFromHandle(hWnd) != nullptr;
}

BOOL IsWindowVisible(HWND hWnd) {
    for (Wnd* pWnd = WndFromHandle(hWnd); pWnd; pWnd = pWnd->hParent ? WndFromHandle(pWnd->hParent) : nullptr) {
        if (!(pWnd->dwStyle & kWsVisible)) {
            return FALSE;
        }
    }
    return WndFromHandle(hWnd) != nullptr;
}

BOOL IsIconic(HWND hWnd) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd) {
        return FALSE;
    }
    if (pWnd->pSdlWindow) {
        return (SDL_GetWindowFlags(pWnd->pSdlWindow) & SDL_WINDOW_MINIMIZED) != 0;
    }
    return (pWnd->dwStyle & kWsMinimize) != 0;
}

HWND GetDesktopWindow(void) {
    EnsureDesktop();
    return (HWND)MakeHandle(kHwndTag, kDesktopSlot);
}

HWND GetParent(HWND hWnd) {
    Wnd* pWnd = WndFromHandle(hWnd);
    return pWnd ? pWnd->hParent : nullptr;
}

HWND FindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName) {
    uint32_t nClass = lpClassName ? FindClass(lpClassName) : 0;
    if (lpClassName && !nClass) {
        return nullptr;
    }
    for (uint32_t i = kDesktopSlot + 1; i < kMaxWindows; i++) {
        const Wnd& w = G().aWindows[i];
        if (!w.bUsed || (w.dwStyle & kWsChild)) {
            continue;
        }
        if ((!nClass || w.nClass == nClass) && (!lpWindowName || strcmp(w.szTitle, lpWindowName) == 0)) {
            return (HWND)MakeHandle(kHwndTag, i);
        }
    }
    return nullptr;
}

// nCmdShow: SW_HIDE 0, SW_SHOWNORMAL 1, SW_SHOWMINIMIZED 2, SW_SHOWMAXIMIZED 3,
// SW_SHOWNOACTIVATE 4, SW_SHOW 5, SW_MINIMIZE 6, SW_SHOWMINNOACTIVE 7, SW_SHOWNA 8,
// SW_RESTORE 9, SW_SHOWDEFAULT 10. Returns whether it was visible before.
BOOL ShowWindow(HWND hWnd, int nCmdShow) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd) {
        return FALSE;
    }
    BOOL bWasVisible = (pWnd->dwStyle & kWsVisible) != 0;
    bool bShow = nCmdShow != 0;
    if (bShow != (bWasVisible != 0)) {
        CallProc(hWnd, kWmShowWindow, bShow, 0);
        pWnd = WndFromHandle(hWnd);
        if (!pWnd) {
            return bWasVisible;
        }
    }
    if (bShow) {
        pWnd->dwStyle |= kWsVisible;
    } else {
        pWnd->dwStyle &= ~kWsVisible;
    }
    SDL_Window* pWindow = pWnd->pSdlWindow;
    if (pWindow) {
        switch (nCmdShow) {
        case 0:
            SDL_HideWindow(pWindow);
            break;
        case 2:
        case 6:
        case 7:
            SDL_ShowWindow(pWindow);
            SDL_MinimizeWindow(pWindow);
            break;
        case 3:
            SDL_ShowWindow(pWindow);
            SDL_MaximizeWindow(pWindow);
            break;
        case 9:
            SDL_ShowWindow(pWindow);
            SDL_RestoreWindow(pWindow);
            break;
        default:
            SDL_ShowWindow(pWindow);
            break;
        }
        if (bShow && nCmdShow != 4 && nCmdShow != 7 && nCmdShow != 8 && nCmdShow != 2 && nCmdShow != 6) {
            SDL_RaiseWindow(pWindow);
            SDL_StartTextInput(pWindow);
        }
    }
    if (bShow && !(pWnd->dwStyle & kWsChild) && nCmdShow != 4 && nCmdShow != 7 && nCmdShow != 8) {
        SetActiveWindow(hWnd);
    }
    if (bShow) {
        pWnd->bNeedsPaint = true;
    }
    return bWasVisible;
}

BOOL UpdateWindow(HWND hWnd) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd) {
        return FALSE;
    }
    if (pWnd->bNeedsPaint && (pWnd->dwStyle & kWsVisible)) {
        CallProc(hWnd, kWmPaint, 0, 0);
    }
    return TRUE;
}

// uFlags: SWP_NOSIZE 1, SWP_NOMOVE 2, SWP_NOZORDER 4, SWP_NOREDRAW 8, SWP_NOACTIVATE 0x10,
// SWP_FRAMECHANGED 0x20, SWP_SHOWWINDOW 0x40, SWP_HIDEWINDOW 0x80. The Z order is the
// window manager's: HWND_TOP and the rest are taken as given.
BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) {
    (void)hWndInsertAfter;
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd || HandleSlot(hWnd) == kDesktopSlot) {
        SetLastError(1400);  // ERROR_INVALID_WINDOW_HANDLE
        return FALSE;
    }
    bool bSized = false, bMoved = false;
    if (!(uFlags & 2)) {
        POINT ptParent = {0, 0};
        if (pWnd->hParent) {
            Wnd* pParent = WndFromHandle(pWnd->hParent);
            if (pParent) {
                ClientOrigin(pParent, &ptParent);
            }
        }
        int nLeft = ptParent.x + X, nTop = ptParent.y + Y;
        bMoved = nLeft != pWnd->rcWindow.left || nTop != pWnd->rcWindow.top;
        pWnd->rcWindow.right += nLeft - pWnd->rcWindow.left;
        pWnd->rcWindow.bottom += nTop - pWnd->rcWindow.top;
        pWnd->rcWindow.left = nLeft;
        pWnd->rcWindow.top = nTop;
    }
    if (!(uFlags & 1)) {
        bSized = cx != pWnd->nClientWidth || cy != pWnd->nClientHeight;
        pWnd->nClientWidth = cx;
        pWnd->nClientHeight = cy;
        pWnd->rcWindow.right = pWnd->rcWindow.left + cx;
        pWnd->rcWindow.bottom = pWnd->rcWindow.top + cy;
    }
    bool bWasFullscreen = pWnd->bFullscreen;
    UpdateFullscreen(pWnd);
    if (pWnd->pSdlWindow && !pWnd->bFullscreen) {
        if (bSized || bWasFullscreen) {
            SDL_SetWindowSize(pWnd->pSdlWindow, pWnd->nClientWidth, pWnd->nClientHeight);
        }
        if (bMoved || bWasFullscreen) {
            SDL_SetWindowPosition(pWnd->pSdlWindow, pWnd->rcWindow.left, pWnd->rcWindow.top);
        }
    }
    if (bSized || bMoved) {
        SendSizeAndMove(hWnd, pWnd, bSized, bMoved);
    }
    if (uFlags & 0x40) {
        ShowWindow(hWnd, (uFlags & 0x10) ? 8 : 5);
    } else if (uFlags & 0x80) {
        ShowWindow(hWnd, 0);
    }
    if (!(uFlags & 8) && (bSized || bMoved)) {
        InvalidateRect(hWnd, nullptr, TRUE);
    }
    return TRUE;
}

BOOL GetClientRect(HWND hWnd, LPRECT lpRect) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd || !lpRect) {
        return FALSE;
    }
    *lpRect = {0, 0, pWnd->nClientWidth, pWnd->nClientHeight};
    return TRUE;
}

BOOL GetWindowRect(HWND hWnd, LPRECT lpRect) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd || !lpRect) {
        return FALSE;
    }
    POINT pt;
    ClientOrigin(pWnd, &pt);
    *lpRect = {pt.x, pt.y, pt.x + pWnd->nClientWidth, pt.y + pWnd->nClientHeight};
    return TRUE;
}

// No frame: the client is the whole window.
BOOL AdjustWindowRectEx(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle) {
    (void)dwStyle;
    (void)bMenu;
    (void)dwExStyle;
    return lpRect != nullptr;
}

BOOL ScreenToClient(HWND hWnd, LPPOINT lpPoint) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd || !lpPoint) {
        return FALSE;
    }
    POINT pt;
    ClientOrigin(pWnd, &pt);
    lpPoint->x -= pt.x;
    lpPoint->y -= pt.y;
    return TRUE;
}

BOOL ClientToScreen(HWND hWnd, LPPOINT lpPoint) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd || !lpPoint) {
        return FALSE;
    }
    POINT pt;
    ClientOrigin(pWnd, &pt);
    lpPoint->x += pt.x;
    lpPoint->y += pt.y;
    return TRUE;
}

// showCmd: SW_SHOWNORMAL 1, SW_SHOWMINIMIZED 2, SW_SHOWMAXIMIZED 3.
BOOL GetWindowPlacement(HWND hWnd, struct tagWINDOWPLACEMENT* lpwndpl) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd || !lpwndpl) {
        return FALSE;
    }
    lpwndpl->flags = 0;
    lpwndpl->showCmd = IsIconic(hWnd) ? 2 : 1;
    if (pWnd->pSdlWindow && (SDL_GetWindowFlags(pWnd->pSdlWindow) & SDL_WINDOW_MAXIMIZED)) {
        lpwndpl->showCmd = 3;
    }
    lpwndpl->ptMinPosition = {-1, -1};
    lpwndpl->ptMaxPosition = {-1, -1};
    GetWindowRect(hWnd, &lpwndpl->rcNormalPosition);
    return TRUE;
}

BOOL SetWindowTextA(HWND hWnd, LPCSTR lpString) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd) {
        return FALSE;
    }
    strncpy(pWnd->szTitle, lpString ? lpString : "", sizeof(pWnd->szTitle) - 1);
    if (pWnd->pSdlWindow) {
        char szTitle[sizeof(pWnd->szTitle) * 2];
        AnsiToUtf8(pWnd->szTitle, szTitle, sizeof(szTitle));
        SDL_SetWindowTitle(pWnd->pSdlWindow, szTitle);
    }
    return TRUE;
}

int GetWindowTextA(HWND hWnd, LPSTR lpString, int nMaxCount) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd || !lpString || nMaxCount <= 0) {
        return 0;
    }
    strncpy(lpString, pWnd->szTitle, (size_t)nMaxCount - 1);
    lpString[nMaxCount - 1] = 0;
    return (int)strlen(lpString);
}

// ---- the window long ----
// GWL_WNDPROC -4, GWL_HINSTANCE -6, GWL_HWNDPARENT -8, GWL_ID -12, GWL_STYLE -16,
// GWL_EXSTYLE -20, GWL_USERDATA -21; 0 and up index cbWndExtra.

static bool WindowLongSlot(Wnd* pWnd, int nIndex, size_t nSize, uint8_t** ppbOut) {
    if (nIndex < 0 || (size_t)nIndex + nSize > (size_t)pWnd->nExtra) {
        SetLastError(1413);  // ERROR_INVALID_INDEX
        return false;
    }
    *ppbOut = pWnd->abExtra + nIndex;
    return true;
}

LONG_PTR GetWindowLongPtrA(HWND hWnd, int nIndex) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd) {
        SetLastError(1400);  // ERROR_INVALID_WINDOW_HANDLE
        return 0;
    }
    switch (nIndex) {
    case -4:
        return (LONG_PTR)pWnd->pfnWndProc;
    case -6:
        return (LONG_PTR)pWnd->hInstance;
    case -8:
        return (LONG_PTR)pWnd->hParent;
    case -12:
        return pWnd->nId;
    case -16:
        return (LONG)pWnd->dwStyle;
    case -20:
        return (LONG)pWnd->dwExStyle;
    case -21:
        return pWnd->nUserData;
    }
    uint8_t* pb;
    if (!WindowLongSlot(pWnd, nIndex, sizeof(LONG_PTR), &pb)) {
        return 0;
    }
    LONG_PTR nValue;
    memcpy(&nValue, pb, sizeof(nValue));
    return nValue;
}

LONG_PTR SetWindowLongPtrA(HWND hWnd, int nIndex, LONG_PTR dwNewLong) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd) {
        SetLastError(1400);  // ERROR_INVALID_WINDOW_HANDLE
        return 0;
    }
    LONG_PTR nOld = GetWindowLongPtrA(hWnd, nIndex);
    switch (nIndex) {
    case -4:
        pWnd->pfnWndProc = (WNDPROC)dwNewLong;
        return nOld;
    case -6:
        pWnd->hInstance = (HINSTANCE)dwNewLong;
        return nOld;
    case -8:
        return nOld;  // re-parenting is not supported
    case -12:
        pWnd->nId = dwNewLong;
        return nOld;
    case -16:
        pWnd->dwStyle = (DWORD)dwNewLong;
        UpdateFullscreen(pWnd);
        return nOld;
    case -20:
        pWnd->dwExStyle = (DWORD)dwNewLong;
        return nOld;
    case -21:
        pWnd->nUserData = dwNewLong;
        return nOld;
    }
    uint8_t* pb;
    if (!WindowLongSlot(pWnd, nIndex, sizeof(LONG_PTR), &pb)) {
        return 0;
    }
    memcpy(pb, &dwNewLong, sizeof(dwNewLong));
    return nOld;
}

// The 32-bit form: on a 64-bit target a pointer does not fit, and GWL_WNDPROC, GWL_HINSTANCE
// and GWL_HWNDPARENT fail as Win64 has them fail (Win64 does not define them at all).
static bool PointerIndex(int nIndex) {
    return sizeof(void*) > sizeof(LONG) && (nIndex == -4 || nIndex == -6 || nIndex == -8);
}

LONG GetWindowLongA(HWND hWnd, int nIndex) {
    if (PointerIndex(nIndex)) {
        SetLastError(1413);  // ERROR_INVALID_INDEX
        return 0;
    }
    if (nIndex >= 0) {
        Wnd* pWnd = WndFromHandle(hWnd);
        uint8_t* pb;
        if (!pWnd || !WindowLongSlot(pWnd, nIndex, sizeof(LONG), &pb)) {
            return 0;
        }
        LONG nValue;
        memcpy(&nValue, pb, sizeof(nValue));
        return nValue;
    }
    return (LONG)GetWindowLongPtrA(hWnd, nIndex);
}

LONG SetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong) {
    if (PointerIndex(nIndex)) {
        SetLastError(1413);  // ERROR_INVALID_INDEX
        return 0;
    }
    if (nIndex >= 0) {
        Wnd* pWnd = WndFromHandle(hWnd);
        uint8_t* pb;
        if (!pWnd || !WindowLongSlot(pWnd, nIndex, sizeof(LONG), &pb)) {
            return 0;
        }
        LONG nOld;
        memcpy(&nOld, pb, sizeof(nOld));
        memcpy(pb, &dwNewLong, sizeof(dwNewLong));
        return nOld;
    }
    return (LONG)SetWindowLongPtrA(hWnd, nIndex, dwNewLong);
}

LRESULT CallWindowProcA(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    return lpPrevWndFunc ? lpPrevWndFunc(hWnd, Msg, wParam, lParam) : 0;
}

// ---- focus and activation ----

HWND GetFocus(void) {
    return G().hFocus;
}

HWND SetFocus(HWND hWnd) {
    State& g = G();
    HWND hOld = g.hFocus;
    if (hWnd && !WndFromHandle(hWnd)) {
        return nullptr;
    }
    if (hWnd == hOld) {
        return hOld;
    }
    g.hFocus = hWnd;
    if (hOld && WndFromHandle(hOld)) {
        CallProc(hOld, kWmKillFocus, (WPARAM)hWnd, 0);
    }
    if (hWnd) {
        CallProc(hWnd, kWmSetFocus, (WPARAM)hOld, 0);
    }
    return hOld;
}

HWND GetActiveWindow(void) {
    return G().hActive;
}

HWND GetForegroundWindow(void) {
    return G().bAppActive ? G().hActive : nullptr;
}

// WA_INACTIVE 0, WA_ACTIVE 1.
HWND SetActiveWindow(HWND hWnd) {
    State& g = G();
    HWND hOld = g.hActive;
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd || hWnd == hOld) {
        return hOld;
    }
    g.hActive = hWnd;
    if (hOld && WndFromHandle(hOld)) {
        CallProc(hOld, kWmNcActivate, FALSE, 0);
        CallProc(hOld, kWmActivate, 0, (LPARAM)hWnd);
    }
    CallProc(hWnd, kWmNcActivate, TRUE, 0);
    CallProc(hWnd, kWmActivate, 1, (LPARAM)hOld);
    if (g.hActive == hWnd && (!g.hFocus || g.hFocus == hOld)) {
        SetFocus(hWnd);
    }
    return hOld;
}

BOOL SetForegroundWindow(HWND hWnd) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd) {
        return FALSE;
    }
    if (pWnd->pSdlWindow) {
        SDL_RaiseWindow(pWnd->pSdlWindow);
    }
    SetActiveWindow(hWnd);
    return TRUE;
}

// ---- painting ----

BOOL InvalidateRect(HWND hWnd, const RECT* lpRect, BOOL bErase) {
    (void)lpRect;
    (void)bErase;
    if (!hWnd) {
        for (uint32_t i = kDesktopSlot + 1; i < kMaxWindows; i++) {
            if (G().aWindows[i].bUsed) {
                G().aWindows[i].bNeedsPaint = true;
            }
        }
        return TRUE;
    }
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd) {
        return FALSE;
    }
    pWnd->bNeedsPaint = true;
    return TRUE;
}

BOOL ValidateRect(HWND hWnd, const RECT* lpRect) {
    (void)lpRect;
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd) {
        return FALSE;
    }
    pWnd->bNeedsPaint = false;
    return TRUE;
}

HDC BeginPaint(HWND hWnd, PAINTSTRUCT* lpPaint) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd || !lpPaint) {
        return nullptr;
    }
    memset(lpPaint, 0, sizeof(*lpPaint));
    lpPaint->hdc = GetDC(hWnd);
    lpPaint->fErase = FALSE;
    lpPaint->rcPaint = {0, 0, pWnd->nClientWidth, pWnd->nClientHeight};
    pWnd->bNeedsPaint = false;
    return lpPaint->hdc;
}

BOOL EndPaint(HWND hWnd, const PAINTSTRUCT* lpPaint) {
    if (lpPaint) {
        ReleaseDC(hWnd, lpPaint->hdc);
    }
    return TRUE;
}

// ---- the default window procedure (Win32Mac: Win32_WndProcs.cpp) ----
// HTCLIENT 1. Alt+F4 is SC_CLOSE, SC_CLOSE is WM_CLOSE, WM_CLOSE destroys the window.

LRESULT DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    switch (Msg) {
    case kWmNcCreate:
        return TRUE;
    case kWmClose:
        DestroyWindow(hWnd);
        return 0;
    case kWmPaint: {
        PAINTSTRUCT tPaint;
        BeginPaint(hWnd, &tPaint);
        EndPaint(hWnd, &tPaint);
        return 0;
    }
    case kWmEraseBkgnd:
        return 1;
    case kWmNcHitTest:
        return 1;
    case kWmNcActivate:
        return TRUE;
    case kWmActivate:
        if (LOWORD(wParam) != 0 && !IsIconic(hWnd)) {
            SetFocus(hWnd);
        }
        return 0;
    case kWmSetCursor: {
        Wnd* pWnd = WndFromHandle(hWnd);
        if (pWnd && LOWORD(lParam) == 1) {
            HCURSOR hCursor = G().aClasses[pWnd->nClass].tClass.hCursor;
            if (hCursor) {
                SetCursor(hCursor);
                return TRUE;
            }
        }
        return FALSE;
    }
    case kWmSysKeyDown:
        if (wParam == kVkF4 && (lParam & (1 << 29))) {
            PostMessageA(hWnd, kWmSysCommand, kScClose, 0);
        }
        return 0;
    case kWmSysCommand:
        switch (wParam & 0xFFF0) {
        case kScClose:
            return CallProc(hWnd, kWmClose, 0, 0);
        case kScMinimize:
            ShowWindow(hWnd, 6);
            return 0;
        case kScMaximize:
            ShowWindow(hWnd, 3);
            return 0;
        case kScRestore:
            ShowWindow(hWnd, 9);
            return 0;
        }
        return 0;
    case kWmSetText:
        return SetWindowTextA(hWnd, (LPCSTR)lParam);
    }
    return 0;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
