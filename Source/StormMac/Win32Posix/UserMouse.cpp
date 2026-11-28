// Win32Posix: the mouse and the cursor on SDL3 (Win32Mac: Win32_Messages.cpp's mouse half and
// Win32_Window.cpp's capture).
//
// SDL's window coordinates (points) become client coordinates of the window's own size (see
// UserWindow.cpp: a scaled client maps back through its scale and letterbox). WM_MOUSEMOVE and
// the button messages carry MAKELPARAM(client x, client y) and the MK_* flags after the event;
// WM_MOUSEWHEEL carries the screen point and the delta in 120ths in the high word of wParam,
// in whole notches: fractions of one (a trackpad) are kept until they add up. A class with CS_DBLCLKS gets the
// double-click messages on SDL's second click. With capture the coordinates run outside the
// client. TrackMouseEvent(TME_LEAVE) gets WM_MOUSELEAVE once when the pointer leaves.
#include "Source/StormMac/Win32Posix/UserInternal.h"

#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <math.h>

namespace Win32Posix {

// The window a mouse event goes to: the capture, else the window SDL saw it in.
static HWND MouseTarget(SDL_WindowID nId) {
    State& g = G();
    if (g.hCapture && WndFromHandle(g.hCapture)) {
        return g.hCapture;
    }
    return WndFromSdlId(nId);
}

// The event point in the target's client coordinates; the cursor's screen point is kept.
static bool MousePoint(HWND hTarget, SDL_WindowID nId, float fX, float fY, POINT* pClient) {
    State& g = G();
    HWND hSource = WndFromSdlId(nId);
    Wnd* pSource = WndFromHandle(hSource ? hSource : hTarget);
    Wnd* pTarget = WndFromHandle(hTarget);
    if (!pSource || !pTarget) {
        return false;
    }
    int nX, nY;
    SdlToClient(pSource, fX, fY, &nX, &nY);
    POINT ptOrigin;
    ClientOrigin(pSource, &ptOrigin);
    g.ptCursor = {ptOrigin.x + nX, ptOrigin.y + nY};
    if (g.bClip) {
        g.ptCursor.x = g.ptCursor.x < g.rcClip.left ? g.rcClip.left : g.ptCursor.x;
        g.ptCursor.y = g.ptCursor.y < g.rcClip.top ? g.rcClip.top : g.ptCursor.y;
        g.ptCursor.x = g.ptCursor.x >= g.rcClip.right ? g.rcClip.right - 1 : g.ptCursor.x;
        g.ptCursor.y = g.ptCursor.y >= g.rcClip.bottom ? g.rcClip.bottom - 1 : g.ptCursor.y;
    }
    ClientOrigin(pTarget, &ptOrigin);
    pClient->x = g.ptCursor.x - ptOrigin.x;
    pClient->y = g.ptCursor.y - ptOrigin.y;
    return true;
}

void OnSdlMouseMotion(const SDL_MouseMotionEvent& tMotion) {
    State& g = G();
    HWND hTarget = MouseTarget(tMotion.windowID);
    if (!hTarget) {
        return;
    }
    POINT pt;
    if (g.bRelative) {
        // The layer's own cursor, moved by the relative motion and held inside the client.
        Wnd* pWnd = WndFromHandle(hTarget);
        POINT ptOrigin;
        ClientOrigin(pWnd, &ptOrigin);
        float fScale = 1.0f;
        int nWidth = pWnd->nClientWidth, nHeight = pWnd->nClientHeight;
        if (pWnd->pSdlWindow) {
            int nSdlWidth, nSdlHeight;
            SDL_GetWindowSize(pWnd->pSdlWindow, &nSdlWidth, &nSdlHeight);
            float fX = nWidth > 0 ? (float)nSdlWidth / (float)nWidth : 1.0f;
            float fY = nHeight > 0 ? (float)nSdlHeight / (float)nHeight : 1.0f;
            fScale = fX < fY ? fX : fY;
        }
        pt.x = g.ptCursor.x - ptOrigin.x + (LONG)lroundf(tMotion.xrel / fScale);
        pt.y = g.ptCursor.y - ptOrigin.y + (LONG)lroundf(tMotion.yrel / fScale);
        pt.x = pt.x < 0 ? 0 : pt.x >= nWidth ? nWidth - 1 : pt.x;
        pt.y = pt.y < 0 ? 0 : pt.y >= nHeight ? nHeight - 1 : pt.y;
        g.ptCursor = {ptOrigin.x + pt.x, ptOrigin.y + pt.y};
    } else if (!MousePoint(hTarget, tMotion.windowID, tMotion.x, tMotion.y, &pt)) {
        return;
    }
    Post(hTarget, kWmMouseMove, MouseKeyFlags(), MAKELPARAM(pt.x, pt.y));
}

void OnSdlMouseButton(const SDL_MouseButtonEvent& tButton) {
    State& g = G();
    UINT uDown, uUp, uDouble;
    BYTE nVk;
    WPARAM nXButton = 0;
    switch (tButton.button) {
    case SDL_BUTTON_LEFT:
        uDown = kWmLButtonDown, uUp = kWmLButtonUp, uDouble = kWmLButtonDblClk, nVk = kVkLButton;
        break;
    case SDL_BUTTON_RIGHT:
        uDown = kWmRButtonDown, uUp = kWmRButtonUp, uDouble = kWmRButtonDblClk, nVk = kVkRButton;
        break;
    case SDL_BUTTON_MIDDLE:
        uDown = kWmMButtonDown, uUp = kWmMButtonUp, uDouble = kWmMButtonDblClk, nVk = kVkMButton;
        break;
    case SDL_BUTTON_X1:
        uDown = kWmXButtonDown, uUp = kWmXButtonUp, uDouble = kWmXButtonDblClk, nVk = kVkXButton1, nXButton = 1;
        break;
    case SDL_BUTTON_X2:
        uDown = kWmXButtonDown, uUp = kWmXButtonUp, uDouble = kWmXButtonDblClk, nVk = kVkXButton2, nXButton = 2;
        break;
    default:
        return;
    }
    SetAsyncKey(nVk, tButton.down);
    if (tButton.down) {
        G().abKeyState[nVk] |= 0x80;
    } else {
        G().abKeyState[nVk] &= (BYTE)~0x80;
    }
    HWND hTarget = MouseTarget(tButton.windowID);
    POINT pt;
    if (!hTarget) {
        return;
    }
    if (g.bRelative) {
        Wnd* pWnd = WndFromHandle(hTarget);
        POINT ptOrigin;
        ClientOrigin(pWnd, &ptOrigin);
        pt = {g.ptCursor.x - ptOrigin.x, g.ptCursor.y - ptOrigin.y};
    } else if (!MousePoint(hTarget, tButton.windowID, tButton.x, tButton.y, &pt)) {
        return;
    }
    UINT uMsg = tButton.down ? uDown : uUp;
    Wnd* pWnd = WndFromHandle(hTarget);
    if (tButton.down && tButton.clicks >= 2 && (G().aClasses[pWnd->nClass].tClass.style & kCsDblClks)) {
        uMsg = uDouble;
    }
    Post(hTarget, uMsg, MouseKeyFlags() | (nXButton << 16), MAKELPARAM(pt.x, pt.y));
}

// Whole notches only: the game steps once per message whatever its delta, so a trackpad's
// fractions are summed until they make a notch (WHEEL_DELTA).
static int TakeNotches(float* pfRemainder, float fDelta) {
    *pfRemainder += fDelta;
    int nNotches = (int)*pfRemainder;
    *pfRemainder -= (float)nNotches;
    return nNotches * kWheelDelta;
}

void OnSdlMouseWheel(const SDL_MouseWheelEvent& tWheel) {
    State& g = G();
    HWND hTarget = g.hFocus ? g.hFocus : MouseTarget(tWheel.windowID);
    if (!hTarget) {
        return;
    }
    POINT pt;
    if (!g.bRelative && !MousePoint(hTarget, tWheel.windowID, tWheel.mouse_x, tWheel.mouse_y, &pt)) {
        return;
    }
    float fSign = tWheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0f : 1.0f;
    LPARAM nScreen = MAKELPARAM(g.ptCursor.x, g.ptCursor.y);
    int nDelta = TakeNotches(&g.fWheelRemainder, tWheel.y * fSign);
    if (nDelta) {
        Post(hTarget, kWmMouseWheel, MouseKeyFlags() | ((WPARAM)(WORD)(short)nDelta << 16), nScreen);
    }
    int nHDelta = TakeNotches(&g.fHWheelRemainder, tWheel.x * fSign);
    if (nHDelta) {
        Post(hTarget, kWmMouseHWheel, MouseKeyFlags() | ((WPARAM)(WORD)(short)nHDelta << 16), nScreen);
    }
}

void OnSdlMouseLeave(HWND hWnd) {
    Wnd* pWnd = WndFromHandle(hWnd);
    if (pWnd && pWnd->bTrackLeave && G().hCapture != hWnd) {
        pWnd->bTrackLeave = false;
        Post(hWnd, kWmMouseLeave, 0, 0);
    }
}

// IDC_ARROW 32512 ... IDC_HAND 32649, to SDL's system cursors.
static SDL_SystemCursor SystemCursor(uint32_t nId) {
    switch (nId) {
    case 32513:
        return SDL_SYSTEM_CURSOR_TEXT;
    case 32514:
        return SDL_SYSTEM_CURSOR_WAIT;
    case 32515:
        return SDL_SYSTEM_CURSOR_CROSSHAIR;
    case 32642:
        return SDL_SYSTEM_CURSOR_NWSE_RESIZE;
    case 32643:
        return SDL_SYSTEM_CURSOR_NESW_RESIZE;
    case 32644:
        return SDL_SYSTEM_CURSOR_EW_RESIZE;
    case 32645:
        return SDL_SYSTEM_CURSOR_NS_RESIZE;
    case 32646:
        return SDL_SYSTEM_CURSOR_MOVE;
    case 32648:
        return SDL_SYSTEM_CURSOR_NOT_ALLOWED;
    case 32649:
        return SDL_SYSTEM_CURSOR_POINTER;
    case 32650:
        return SDL_SYSTEM_CURSOR_PROGRESS;
    default:
        return SDL_SYSTEM_CURSOR_DEFAULT;
    }
}

// The display count and the cursor, onto SDL.
void ApplyCursor() {
    State& g = G();
    if (!g.bSdlReady) {
        return;
    }
    static SDL_Cursor* s_apCursors[SDL_SYSTEM_CURSOR_COUNT];
    if (g.hCursor && HandleHasTag(g.hCursor, kHcursorTag)) {
        SDL_SystemCursor nCursor = SystemCursor(HandleSlot(g.hCursor));
        if (!s_apCursors[nCursor]) {
            s_apCursors[nCursor] = SDL_CreateSystemCursor(nCursor);
        }
        if (s_apCursors[nCursor]) {
            SDL_SetCursor(s_apCursors[nCursor]);
        }
    }
    if (g.nCursorCount >= 0 && g.hCursor && !g.bRelative) {
        SDL_ShowCursor();
    } else {
        SDL_HideCursor();
    }
}

}  // namespace Win32Posix

using namespace Win32Posix;

BOOL GetCursorPos(LPPOINT lpPoint) {
    if (!lpPoint) {
        return FALSE;
    }
    PumpSdl();
    *lpPoint = G().ptCursor;
    return TRUE;
}

// Moves the pointer to a screen point over the main window (warping it in SDL); the next
// WM_MOUSEMOVE reports it, as on Windows.
BOOL SetCursorPos(int X, int Y) {
    State& g = G();
    g.ptCursor = {X, Y};
    HWND hWnd = g.hCapture ? g.hCapture : g.hActive ? g.hActive : MainWindow();
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd) {
        return TRUE;
    }
    POINT ptOrigin;
    ClientOrigin(pWnd, &ptOrigin);
    int nX = X - ptOrigin.x, nY = Y - ptOrigin.y;
    if (!g.bRelative && pWnd->pSdlWindow) {
        float fX, fY;
        ClientToSdl(pWnd, nX, nY, &fX, &fY);
        SDL_WarpMouseInWindow(pWnd->pSdlWindow, fX, fY);
    }
    Post(hWnd, kWmMouseMove, MouseKeyFlags(), MAKELPARAM(nX, nY));
    return TRUE;
}

int ShowCursor(BOOL bShow) {
    State& g = G();
    g.nCursorCount += bShow ? 1 : -1;
    ApplyCursor();
    return g.nCursorCount;
}

HCURSOR SetCursor(HCURSOR hCursor) {
    State& g = G();
    HCURSOR hOld = g.hCursor;
    g.hCursor = hCursor;
    ApplyCursor();
    return hOld;
}

// The system cursors only (hInstance NULL, an IDC_* id); a module's own cursor resource is the
// arrow, as the game draws its cursor itself.
HCURSOR LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName) {
    uint32_t nId = (uintptr_t)lpCursorName < 0x10000 && !hInstance ? (uint32_t)(uintptr_t)lpCursorName : 32512;
    return (HCURSOR)MakeHandle(kHcursorTag, nId);
}

HWND SetCapture(HWND hWnd) {
    State& g = G();
    HWND hOld = g.hCapture;
    if (!WndFromHandle(hWnd)) {
        return hOld;
    }
    g.hCapture = hWnd;
    if (g.bSdlReady) {
        SDL_CaptureMouse(true);
    }
    if (hOld && hOld != hWnd && WndFromHandle(hOld)) {
        CallProc(hOld, kWmCaptureChanged, 0, (LPARAM)hWnd);
    }
    return hOld;
}

BOOL ReleaseCapture(void) {
    State& g = G();
    HWND hOld = g.hCapture;
    g.hCapture = nullptr;
    if (g.bSdlReady) {
        SDL_CaptureMouse(false);
    }
    if (hOld && WndFromHandle(hOld)) {
        CallProc(hOld, kWmCaptureChanged, 0, 0);
    }
    return TRUE;
}

HWND GetCapture(void) {
    return G().hCapture;
}

// Confines the pointer to a screen rectangle: SDL's mouse rectangle of the main window, in its
// coordinates. NULL frees it.
BOOL ClipCursor(const RECT* lpRect) {
    State& g = G();
    Wnd* pWnd = WndFromHandle(g.hActive ? g.hActive : MainWindow());
    if (!lpRect) {
        g.bClip = false;
        if (pWnd && pWnd->pSdlWindow) {
            SDL_SetWindowMouseRect(pWnd->pSdlWindow, nullptr);
        }
        return TRUE;
    }
    g.bClip = true;
    g.rcClip = *lpRect;
    if (pWnd && pWnd->pSdlWindow) {
        POINT ptOrigin;
        ClientOrigin(pWnd, &ptOrigin);
        float fLeft, fTop, fRight, fBottom;
        ClientToSdl(pWnd, lpRect->left - ptOrigin.x, lpRect->top - ptOrigin.y, &fLeft, &fTop);
        ClientToSdl(pWnd, lpRect->right - ptOrigin.x, lpRect->bottom - ptOrigin.y, &fRight, &fBottom);
        SDL_Rect tRect = {(int)fLeft, (int)fTop, (int)(fRight - fLeft), (int)(fBottom - fTop)};
        SDL_SetWindowMouseRect(pWnd->pSdlWindow, tRect.w > 0 && tRect.h > 0 ? &tRect : nullptr);
    }
    return TRUE;
}

BOOL GetClipCursor(LPRECT lpRect) {
    if (!lpRect) {
        return FALSE;
    }
    State& g = G();
    if (g.bClip) {
        *lpRect = g.rcClip;
    } else {
        int nWidth, nHeight;
        ScreenSize(&nWidth, &nHeight);
        *lpRect = {0, 0, nWidth, nHeight};
    }
    return TRUE;
}

// TME_LEAVE 2, TME_CANCEL 0x80000000; hover tracking is not supported.
BOOL TrackMouseEvent(struct tagTRACKMOUSEEVENT* lpEventTrack) {
    if (!lpEventTrack) {
        return FALSE;
    }
    // The record's layout (winuser.h): cbSize, dwFlags, hwndTrack, dwHoverTime.
    struct TrackRecord {
        DWORD cbSize;
        DWORD dwFlags;
        HWND hwndTrack;
        DWORD dwHoverTime;
    };
    const TrackRecord* pTrack = (const TrackRecord*)lpEventTrack;
    Wnd* pWnd = WndFromHandle(pTrack->hwndTrack);
    if (!pWnd) {
        return FALSE;
    }
    if (pTrack->dwFlags & 2) {
        pWnd->bTrackLeave = !(pTrack->dwFlags & 0x80000000u);
    }
    return TRUE;
}

BOOL Win32Posix_SetRelativeMouse(HWND hWnd, BOOL bRelative) {
    State& g = G();
    Wnd* pWnd = WndFromHandle(hWnd);
    if (!pWnd || !pWnd->pSdlWindow) {
        return FALSE;
    }
    if (!SDL_SetWindowRelativeMouseMode(pWnd->pSdlWindow, bRelative != 0)) {
        return FALSE;
    }
    g.bRelative = bRelative != 0;
    ApplyCursor();
    return TRUE;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
