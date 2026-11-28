// Win32Posix: the message queue and the pump on SDL3 (Win32Mac: Win32_Messages.cpp).
//
// One queue for the process, owned by the thread that brought SDL up (the game's main thread).
// PeekMessageA and GetMessageA first drain SDL's events into it as WM_* messages, then hand
// them out in Windows' order of precedence: messages sent from other threads (delivered, not
// returned), posted and input messages in arrival order, WM_QUIT, WM_PAINT, WM_TIMER. Any
// thread may post; a SendMessageA from another thread waits for the owner to deliver it.
#include "Source/StormMac/Win32Posix/UserInternal.h"

#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <stdio.h>
#include <string.h>

namespace Win32Posix {

static bool OnPumpThread() {
    State& g = G();
    return !g.bSdlReady || g.nPumpThread == SDL_GetCurrentThreadID();
}

static void Wake() {
    State& g = G();
    if (g.bSdlReady && g.nWakeEvent && !OnPumpThread()) {
        SDL_Event tEvent;
        SDL_zero(tEvent);
        tEvent.type = g.nWakeEvent;
        SDL_PushEvent(&tEvent);
    }
}

static void Stamp(MSG* pMsg) {
    pMsg->time = GetTickCount();
    pMsg->pt = G().ptCursor;
}

static void QueueFull(const MSG& tMsg) {
    static bool s_bReported;
    if (!s_bReported) {
        s_bReported = true;
        fprintf(stderr, "Win32Posix: the message queue is full; dropping 0x%04x and later messages\n", tMsg.message);
    }
}

static void RemoveAt(uint32_t nIndex) {
    State& g = G();
    memmove(&g.aQueue[nIndex], &g.aQueue[nIndex + 1], (g.nQueue - nIndex - 1) * sizeof(MSG));
    g.nQueue--;
}

void Enqueue(const MSG& tMsg) {
    State& g = G();
    {
        Lock tGuard;
        // Mouse moves coalesce, as Windows keeps one pending WM_MOUSEMOVE.
        if (tMsg.message == kWmMouseMove && g.nQueue > 0) {
            MSG& tLast = g.aQueue[g.nQueue - 1];
            if (tLast.message == kWmMouseMove && tLast.hwnd == tMsg.hwnd && tLast.wParam == tMsg.wParam) {
                tLast = tMsg;
                return;
            }
        }
        if (g.nQueue >= kQueueSize) {
            QueueFull(tMsg);
            return;
        }
        g.aQueue[g.nQueue++] = tMsg;
    }
    Wake();
}

void EnqueueFront(const MSG& tMsg) {
    State& g = G();
    Lock tGuard;
    if (g.nQueue >= kQueueSize) {
        QueueFull(tMsg);
        return;
    }
    memmove(&g.aQueue[1], &g.aQueue[0], g.nQueue * sizeof(MSG));
    g.aQueue[0] = tMsg;
    g.nQueue++;
}

void Post(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MSG tMsg;
    tMsg.hwnd = hWnd;
    tMsg.message = uMsg;
    tMsg.wParam = wParam;
    tMsg.lParam = lParam;
    Stamp(&tMsg);
    Enqueue(tMsg);
}

void RemoveWindowMessages(HWND hWnd) {
    State& g = G();
    Lock tGuard;
    for (uint32_t i = 0; i < g.nQueue;) {
        if (g.aQueue[i].hwnd == hWnd) {
            RemoveAt(i);
        } else {
            i++;
        }
    }
}

// ---- SDL events to messages ----

void TranslateSdlEvent(const SDL_Event& tEvent) {
    State& g = G();
    switch (tEvent.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        OnSdlKey(tEvent.key);
        return;
    case SDL_EVENT_TEXT_INPUT:
        OnSdlText(tEvent.text);
        return;
    case SDL_EVENT_MOUSE_MOTION:
        OnSdlMouseMotion(tEvent.motion);
        return;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        OnSdlMouseButton(tEvent.button);
        return;
    case SDL_EVENT_MOUSE_WHEEL:
        OnSdlMouseWheel(tEvent.wheel);
        return;
    case SDL_EVENT_QUIT: {
        // Cmd-Q, the Dock's Quit, a logout: the main window is asked to close.
        HWND hMain = MainWindow();
        if (hMain) {
            Post(hMain, kWmClose, 0, 0);
        } else {
            PostQuitMessage(0);
        }
        return;
    }
    default:
        break;
    }
    if (tEvent.type < SDL_EVENT_WINDOW_FIRST || tEvent.type > SDL_EVENT_WINDOW_LAST) {
        return;
    }
    HWND hWnd = WndFromSdlId(tEvent.window.windowID);
    if (!hWnd) {
        return;
    }
    switch (tEvent.type) {
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        Post(hWnd, kWmClose, 0, 0);
        break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        if (!g.bAppActive) {
            g.bAppActive = true;
            Post(hWnd, kWmActivateApp, TRUE, 0);
        }
        if (g.hActive != hWnd) {
            HWND hOld = g.hActive;
            g.hActive = hWnd;
            Post(hWnd, kWmNcActivate, TRUE, 0);
            Post(hWnd, kWmActivate, 1, (LPARAM)hOld);
        }
        if (g.hFocus != hWnd) {
            HWND hOld = g.hFocus;
            g.hFocus = hWnd;
            Post(hWnd, kWmSetFocus, (WPARAM)hOld, 0);
        }
        if (Wnd* pWnd = WndFromHandle(hWnd)) {
            SDL_StartTextInput(pWnd->pSdlWindow);
        }
        ApplyCursor();
        break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        if (g.hFocus == hWnd) {
            g.hFocus = nullptr;
            Post(hWnd, kWmKillFocus, 0, 0);
        }
        if (g.hActive == hWnd) {
            Post(hWnd, kWmNcActivate, FALSE, 0);
            Post(hWnd, kWmActivate, 0, 0);
        }
        if (g.bAppActive) {
            g.bAppActive = false;
            Post(hWnd, kWmActivateApp, FALSE, 0);
        }
        ResetKeys();
        break;
    case SDL_EVENT_WINDOW_RESIZED:
        OnSdlResized(hWnd, tEvent.window.data1, tEvent.window.data2);
        break;
    case SDL_EVENT_WINDOW_MOVED:
        OnSdlMoved(hWnd);
        break;
    case SDL_EVENT_WINDOW_MINIMIZED:
        Post(hWnd, kWmSize, kSizeMinimized, 0);
        break;
    case SDL_EVENT_WINDOW_MAXIMIZED:
    case SDL_EVENT_WINDOW_RESTORED:
        if (Wnd* pWnd = WndFromHandle(hWnd)) {
            Post(hWnd, kWmSize, tEvent.type == SDL_EVENT_WINDOW_MAXIMIZED ? kSizeMaximized : kSizeRestored,
                 MAKELPARAM(pWnd->nClientWidth, pWnd->nClientHeight));
        }
        InvalidateRect(hWnd, nullptr, TRUE);
        break;
    case SDL_EVENT_WINDOW_EXPOSED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        InvalidateRect(hWnd, nullptr, TRUE);
        break;
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        OnSdlMouseLeave(hWnd);
        break;
    default:
        break;
    }
}

void PumpSdl() {
    State& g = G();
    if (!g.bSdlReady || !OnPumpThread()) {
        return;
    }
    SDL_Event tEvent;
    while (SDL_PollEvent(&tEvent)) {
        TranslateSdlEvent(tEvent);
    }
    RunScript();
}

// ---- taking a message ----

static bool InRange(UINT uMsg, UINT nMin, UINT nMax) {
    return (nMin == 0 && nMax == 0) || (uMsg >= nMin && uMsg <= nMax);
}

// hWnd NULL takes any message; (HWND)-1 only those posted to no window; a window its own and
// its children's.
static bool ForWindow(HWND hMsgWnd, HWND hFilter) {
    if (!hFilter) {
        return true;
    }
    if (hFilter == (HWND)(intptr_t)-1) {
        return hMsgWnd == nullptr;
    }
    for (HWND h = hMsgWnd; h; h = GetParent(h)) {
        if (h == hFilter) {
            return true;
        }
    }
    return false;
}

static void DeliverSent() {
    State& g = G();
    for (;;) {
        State::Sent* pSent;
        {
            Lock tGuard;
            if (g.nSent == 0) {
                return;
            }
            pSent = g.apSent[0];
            memmove(&g.apSent[0], &g.apSent[1], (g.nSent - 1) * sizeof(g.apSent[0]));
            g.nSent--;
        }
        LRESULT lResult = CallProc(pSent->hWnd, pSent->uMsg, pSent->wParam, pSent->lParam);
        {
            Lock tGuard;
            pSent->lResult = lResult;
            pSent->bDone = true;
        }
        SDL_BroadcastCondition(g.pSentDone);
    }
}

static bool TakeTimer(MSG* pMsg, HWND hFilter, UINT nMin, UINT nMax, bool bRemove) {
    State& g = G();
    if (!InRange(kWmTimer, nMin, nMax)) {
        return false;
    }
    uint64_t nNow = SDL_GetTicks();
    for (uint32_t i = 0; i < g.nTimers; i++) {
        State::Timer& t = g.aTimers[i];
        if (t.nDue <= nNow && ForWindow(t.hWnd, hFilter)) {
            pMsg->hwnd = t.hWnd;
            pMsg->message = kWmTimer;
            pMsg->wParam = t.nId;
            pMsg->lParam = (LPARAM)t.pfnProc;
            Stamp(pMsg);
            if (bRemove) {
                t.nDue = nNow + t.nElapse;  // no backlog, as Windows
            }
            return true;
        }
    }
    return false;
}

static bool TakePaint(MSG* pMsg, HWND hFilter, UINT nMin, UINT nMax) {
    if (!InRange(kWmPaint, nMin, nMax)) {
        return false;
    }
    for (uint32_t i = kDesktopSlot + 1; i < kMaxWindows; i++) {
        Wnd& w = G().aWindows[i];
        HWND hWnd = (HWND)MakeHandle(kHwndTag, i);
        if (w.bUsed && w.bNeedsPaint && (w.dwStyle & kWsVisible) && ForWindow(hWnd, hFilter)) {
            pMsg->hwnd = hWnd;
            pMsg->message = kWmPaint;
            pMsg->wParam = 0;
            pMsg->lParam = 0;
            Stamp(pMsg);
            return true;  // stays until the window is validated
        }
    }
    return false;
}

static bool TakeMessage(MSG* pMsg, HWND hFilter, UINT nMin, UINT nMax, bool bRemove) {
    State& g = G();
    DeliverSent();
    {
        Lock tGuard;
        for (uint32_t i = 0; i < g.nQueue; i++) {
            if (ForWindow(g.aQueue[i].hwnd, hFilter) && InRange(g.aQueue[i].message, nMin, nMax)) {
                *pMsg = g.aQueue[i];
                if (bRemove) {
                    RemoveAt(i);
                }
                goto taken;
            }
        }
        if (g.bQuitPosted) {
            pMsg->hwnd = nullptr;
            pMsg->message = kWmQuit;
            pMsg->wParam = (WPARAM)(intptr_t)g.nQuitCode;
            pMsg->lParam = 0;
            Stamp(pMsg);
            if (bRemove) {
                g.bQuitPosted = false;
            }
            return true;
        }
    }
    return TakePaint(pMsg, hFilter, nMin, nMax) || TakeTimer(pMsg, hFilter, nMin, nMax, bRemove);
taken:
    if (bRemove) {
        ApplyKeyMessage(*pMsg);
    }
    return true;
}

// The milliseconds until the next timer is due, or -1.
static Sint32 NextTimerWait() {
    State& g = G();
    if (g.nTimers == 0) {
        return -1;
    }
    uint64_t nNow = SDL_GetTicks();
    uint64_t nNext = UINT64_MAX;
    for (uint32_t i = 0; i < g.nTimers; i++) {
        nNext = g.aTimers[i].nDue < nNext ? g.aTimers[i].nDue : nNext;
    }
    return nNext <= nNow ? 0 : (Sint32)(nNext - nNow > 0x7FFFFFFF ? 0x7FFFFFFF : nNext - nNow);
}

}  // namespace Win32Posix

using namespace Win32Posix;

// wRemoveMsg: PM_NOREMOVE 0, PM_REMOVE 1; PM_NOYIELD 2 is ignored.
BOOL PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
    if (!lpMsg) {
        return FALSE;
    }
    PumpSdl();
    return TakeMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, (wRemoveMsg & PM_REMOVE) != 0);
}

// Waits for a message: FALSE for WM_QUIT, TRUE for anything else.
BOOL GetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) {
    if (!lpMsg) {
        return -1;
    }
    State& g = G();
    for (;;) {
        PumpSdl();
        if (TakeMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, true)) {
            return lpMsg->message != kWmQuit;
        }
        Sint32 nWait = NextTimerWait();
        if (g.bSdlReady && OnPumpThread()) {
            SDL_WaitEventTimeout(nullptr, nWait < 0 || nWait > 100 ? 100 : nWait);
        } else {
            Lock tGuard;
            SDL_WaitConditionTimeout(g.pSentDone, g.pLock, nWait < 0 || nWait > 10 ? 10 : nWait);
        }
    }
}

LRESULT DispatchMessageA(const MSG* lpMsg) {
    if (!lpMsg) {
        return 0;
    }
    if (lpMsg->message == kWmTimer && lpMsg->lParam) {
        ((TIMERPROC)lpMsg->lParam)(lpMsg->hwnd, kWmTimer, (UINT_PTR)lpMsg->wParam, GetTickCount());
        return 0;
    }
    if (!lpMsg->hwnd) {
        return 0;
    }
    return CallProc(lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
}

BOOL PostMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    if (hWnd && !WndFromHandle(hWnd)) {
        SetLastError(1400);  // ERROR_INVALID_WINDOW_HANDLE
        return FALSE;
    }
    Post(hWnd, Msg, wParam, lParam);
    return TRUE;
}

LRESULT SendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    if (!WndFromHandle(hWnd)) {
        SetLastError(1400);  // ERROR_INVALID_WINDOW_HANDLE
        return 0;
    }
    if (OnPumpThread()) {
        return CallProc(hWnd, Msg, wParam, lParam);
    }
    State& g = G();
    State::Sent tSent = {hWnd, Msg, wParam, lParam, 0, false};
    {
        Lock tGuard;
        if (g.nSent >= kMaxSent) {
            return 0;
        }
        g.apSent[g.nSent++] = &tSent;
    }
    Wake();
    Lock tGuard;
    while (!tSent.bDone) {
        SDL_WaitCondition(g.pSentDone, g.pLock);
    }
    return tSent.lResult;
}

void PostQuitMessage(int nExitCode) {
    State& g = G();
    {
        Lock tGuard;
        g.bQuitPosted = true;
        g.nQuitCode = nExitCode;
    }
    Wake();
}

UINT_PTR SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc) {
    State& g = G();
    if (hWnd && !WndFromHandle(hWnd)) {
        SetLastError(1400);  // ERROR_INVALID_WINDOW_HANDLE
        return 0;
    }
    if (uElapse < 10) {
        uElapse = 10;  // USER_TIMER_MINIMUM
    }
    if (!hWnd) {
        nIDEvent = ++g.nNextTimerId;
    }
    uint64_t nDue = SDL_GetTicks() + uElapse;
    for (uint32_t i = 0; i < g.nTimers; i++) {
        State::Timer& t = g.aTimers[i];
        if (t.hWnd == hWnd && t.nId == nIDEvent) {
            t.nElapse = uElapse;
            t.pfnProc = (void*)lpTimerFunc;
            t.nDue = nDue;
            return hWnd ? 1 : nIDEvent;
        }
    }
    if (g.nTimers >= kMaxTimers) {
        SetLastError(1158);  // ERROR_NO_MORE_USER_HANDLES
        return 0;
    }
    g.aTimers[g.nTimers++] = {hWnd, nIDEvent, uElapse, (void*)lpTimerFunc, nDue};
    return hWnd ? 1 : nIDEvent;
}

BOOL KillTimer(HWND hWnd, UINT_PTR uIDEvent) {
    State& g = G();
    for (uint32_t i = 0; i < g.nTimers; i++) {
        if (g.aTimers[i].hWnd == hWnd && g.aTimers[i].nId == uIDEvent) {
            g.aTimers[i] = g.aTimers[--g.nTimers];
            Lock tGuard;
            for (uint32_t j = 0; j < g.nQueue;) {
                const MSG& m = g.aQueue[j];
                if (m.message == kWmTimer && m.hwnd == hWnd && m.wParam == uIDEvent) {
                    RemoveAt(j);
                } else {
                    j++;
                }
            }
            return TRUE;
        }
    }
    return FALSE;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
