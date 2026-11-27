// Storm's Win32-on-Mac layer: the message queues and timers, 1.14d Intel Mac band
// 00028e09..00029968. See Win32Mac.h for what is and is not known about the
// original file boundaries.
//
// Two 40-slot rings: messages posted to windows, and input (keys, mouse) that the
// Carbon event handlers enqueue for no window. A mouse move is not queued; only the
// latest one is kept. Timers are a list polled by the message pump.
//
// The message pump (Msg_GetPendingMessage) also makes the messages nothing queues:
// WM_QUIT, the pending mouse move, WM_PAINT for a window that needs it, and WM_TIMER.
#include "Win32MacWindow.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <string.h>

#define WM_QUIT 0x0012
#define WM_ACTIVATEAPP 0x001C
#define WM_NCMOUSEMOVE 0x00A0
#define WM_KEYUP 0x0101
#define WM_SYSKEYUP 0x0105
#define WM_TIMER 0x0113
#define WM_MOUSEWHEEL 0x020A

#define PM_REMOVE 0x0001
#define CS_DBLCLKS 0x0008
#define VK_CAPITAL 0x14
#define VK_MENU 0x12

// 1.14d mac: 003f1134
// The last id SetTimer made up; they count down from -1.
static UINT s_nLastTimerID;
// 1.14d mac: 003f1138
static DWORD s_dwCmdPeriodFlag;

// 1.14d mac: 003f113c
static Win32MacMsgState s_tMsgState;
// 1.14d mac: 003972a0
// The queues and timers. Every user but the timer calls checks it for NULL.
Win32MacMsgState* g_pMsgState = &s_tMsgState;

// 1.14d mac: 00030049
BOOL IsWindow(HWND hWnd);
// 1.14d mac: 0002ffee
LRESULT CallWindowProc(WNDPROC pfnWndProc, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 1.14d mac: 0002ffdd
HWND GetDesktopWindow(void);
// 1.14d mac: 00030fce
HWND WindowFromPoint(POINT pt);
// 1.14d mac: 0002fef3
BOOL ScreenToClient(HWND hWnd, POINT* pPoint);
// 1.14d mac: 0002fe9b
// The ancestor whose parent is the desktop.
HWND Wnd_GetTopParent(HWND hWnd);
// 1.14d mac: 000305f6
HWND SetActiveWindow(HWND hWnd);
// 1.14d mac: 00030cb2
HWND GetActiveWindow(void);
// 1.14d mac: 00026f58
short GetKeyState(int nVirtKey);
// 1.14d mac: 00026d14
// Turns a Mac key event into the Win32 key message, in place.
void Keys_PrepareKeyMsg(MSG* pMsg, BOOL bUpdateState, bool bKeyDown, BYTE byMacKeyCode, BYTE byCharCode, BOOL bAltDown);

BOOL KillTimer(HWND hWnd, UINT nIDEvent);

static int MsgRing_NextHead(MsgRing* pRing) {
    if (pRing == NULL) {
        return 0;
    }
    int nNext = pRing->nHead + 1;
    if (nNext >= MSGRING_SIZE) {
        nNext = 0;
    }
    return nNext;
}

// 1.14d mac: 00028e09
// The ring's oldest message if it is in [nMsgMin, nMsgMax], taken off the ring
// with bRemove.
BOOL MSG_PeekQueuedMessage(MsgRing* pRing, MSG* pMsg, BOOL bRemove, UINT nMsgMin, UINT nMsgMax) {
    if (pRing->nHead == pRing->nTail) {
        return 0;
    }
    MSG* pHead = &pRing->aMsgs[pRing->nHead];
    if (pHead->message < nMsgMin || pHead->message > nMsgMax) {
        return 0;
    }
    *pMsg = *pHead;
    if (bRemove) {
        pRing->nHead = MsgRing_NextHead(pRing);
    }
    return 1;
}

// 1.14d mac: 00028e7c
// A window is going: its queued messages are blanked in place and its timers freed.
void Msg_RemoveMsgsForHWnd(MsgRing* pRing, HWND hWnd) {
    for (int i = pRing->nHead; i != pRing->nTail;) {
        if (pRing->aMsgs[i].hwnd == hWnd) {
            memset(&pRing->aMsgs[i], 0, 4 * sizeof(DWORD));
        }
        if (++i >= MSGRING_SIZE) {
            i = 0;
        }
    }
    Win32MacTimer** ppLink = &g_pMsgState->pTimers;
    Win32MacTimer* pTimer = *ppLink;
    while (pTimer != NULL) {
        if (pTimer->hWnd == hWnd) {
            *ppLink = pTimer->pNext;
            Priv_Free(pTimer);
            pTimer = *ppLink;
        } else {
            ppLink = &pTimer->pNext;
            pTimer = pTimer->pNext;
        }
    }
}

// 1.14d mac: 00028f1b
// Addresses a mouse message from its screen point (lParam as queued) and makes the
// lParam client coordinates. Non-client moves and the wheel go to the focus window
// (else the active one, else the desktop), the rest to the capture window or the
// window under the point. A second left press on the same window, of a class with
// CS_DBLCLKS, within 3 pixels and the double-click time, becomes WM_LBUTTONDBLCLK.
// A click activates the clicked window's top-level window.
void Msg_FillMouseMsgFromPoint(MSG* pMsg, LPARAM lScreenPos) {
    LONG nY = lScreenPos >> 16;
    LONG nX = (int16_t)lScreenPos;
    POINT ptClient;
    HWND hWnd;
    if (pMsg->message == WM_NCMOUSEMOVE || pMsg->message == WM_MOUSEWHEEL) {
        hWnd = g_pMsgState->hWndFocus;
        if (hWnd == NULL) {
            hWnd = g_pMsgState->hWndActive;
            if (hWnd == NULL) {
                hWnd = GetDesktopWindow();
            }
        }
    } else {
        hWnd = g_pMsgState->hWndCapture;
        if (hWnd == NULL) {
            POINT ptScreen;
            ptScreen.x = nX;
            ptScreen.y = nY;
            hWnd = WindowFromPoint(ptScreen);
        }
    }
    pMsg->hwnd = hWnd;
    if (hWnd != NULL) {
        ptClient.x = nX;
        ptClient.y = nY;
        ScreenToClient(hWnd, &ptClient);
        pMsg->lParam = (ptClient.y << 16) | (ptClient.x & 0xFFFF);
        pMsg->pt.x = nX;
        pMsg->pt.y = nY;
        if (pMsg->message == WM_LBUTTONDOWN) {
            LONG nLastY = g_pMsgState->nLastClickY;
            LONG nLastX = g_pMsgState->nLastClickX;
            HWND hWndLast = g_pMsgState->hWndLastClick;
            UINT uLastMsg = g_pMsgState->uLastClickMsg;
            DWORD dwLastTime = g_pMsgState->dwLastClickTime;
            HWND hWndClicked = pMsg->hwnd;
            g_pMsgState->hWndLastClick = hWndClicked;
            g_pMsgState->uLastClickMsg = WM_LBUTTONDOWN;
            g_pMsgState->dwLastClickTime = pMsg->time;
            g_pMsgState->nLastClickX = nX;
            g_pMsgState->nLastClickY = nY;
            if (uLastMsg == pMsg->message && hWndLast == hWndClicked && (hWndClicked->pClass->style & CS_DBLCLKS)) {
                UINT nDistX = (nLastX - nX) * (nLastX - nX);
                UINT nDistY = (nLastY - nY) * (nLastY - nY);
                UINT nDist = nDistY > nDistX ? nDistY : nDistX;
                if (nDist < 10 && pMsg->time - dwLastTime <= GetDblTime() * 16) {
                    if (pMsg->message == WM_LBUTTONDOWN) {
                        pMsg->message = WM_LBUTTONDBLCLK;
                    }
                    g_pMsgState->hWndLastClick = NULL;
                }
            }
            if (pMsg->hwnd != NULL && pMsg->hwnd != GetDesktopWindow()) {
                HWND hWndTop = Wnd_GetTopParent(hWndClicked);
                if (hWndTop != GetDesktopWindow()) {
                    SetActiveWindow(hWndTop);
                }
            }
        }
    }
}

static HWND Wnd_FindWindowNeedingPaint(HWND hWnd);

// 1.14d mac: 000290e9
// The next message for the pump, in [nMsgFilterMin, nMsgFilterMax] (both 0: any).
// Due timers with a callback are called first. Then, in order: a posted message,
// WM_QUIT, queued input, the pending mouse move, WM_PAINT, and WM_TIMER for the
// most overdue timer without a callback.
BOOL Msg_GetPendingMessage(MSG* pMsg, BOOL bRemove, UINT nMsgFilterMin, UINT nMsgFilterMax) {
    DWORD dwNow = GetTickCount();
    int nDueLate = 0;
    Win32MacTimer* pDue = NULL;
    for (Win32MacTimer* pTimer = g_pMsgState->pTimers; pTimer != NULL;) {
        Win32MacTimer* pNext = pTimer->pNext;
        int nLate = dwNow - pTimer->dwLastTime - pTimer->uElapse;
        if (nLate >= 0) {
            if (pTimer->pfnTimerFunc != NULL) {
                pTimer->dwLastTime = dwNow;
                pTimer->pfnTimerFunc(pTimer->hWnd, WM_TIMER, pTimer->nIDEvent, dwNow);
            } else if (pDue == NULL || nLate > nDueLate) {
                pDue = pTimer;
                nDueLate = nLate;
            }
        }
        pTimer = pNext;
    }
    UINT nMin = 0;
    UINT nMax = 0xFFFFFFFF;
    if (nMsgFilterMin != 0 || nMsgFilterMax != 0) {
        nMin = nMsgFilterMin;
        nMax = nMsgFilterMax;
    }
    if (MSG_PeekQueuedMessage(&g_pMsgState->tPosted, pMsg, bRemove, nMin, nMax)) {
        return 1;
    }
    if (g_pMsgState->bQuit && nMin <= WM_QUIT && nMax >= WM_QUIT) {
        pMsg->hwnd = g_pMsgState->hWndActive;
        pMsg->message = WM_QUIT;
        pMsg->wParam = g_pMsgState->nExitCode;
        pMsg->lParam = 0;
        if (bRemove) {
            g_pMsgState->bQuit = 0;
        }
        return 1;
    }
    if (MSG_PeekQueuedMessage(&g_pMsgState->tInput, pMsg, bRemove, nMin, nMax)) {
        return 1;
    }
    if (g_pMsgState->bMouseMovePending &&
        ((nMin <= WM_MOUSEMOVE && nMax >= WM_MOUSEMOVE) || (nMin <= WM_NCMOUSEMOVE && nMax >= WM_NCMOUSEMOVE))) {
        pMsg->message = WM_MOUSEMOVE;
        pMsg->wParam = g_pMsgState->wMouseMoveWParam;
        pMsg->lParam = g_pMsgState->lMouseMoveLParam;
        if (bRemove) {
            g_pMsgState->bMouseMovePending = 0;
        }
        return 1;
    }
    if (g_pMsgState->bPaintPending && nMin <= WM_PAINT && nMax >= WM_PAINT) {
        HWND hWnd = Wnd_FindWindowNeedingPaint(GetDesktopWindow());
        if (hWnd == NULL) {
            return 0;
        }
        pMsg->hwnd = hWnd;
        pMsg->message = WM_PAINT;
        pMsg->wParam = 0;
        pMsg->lParam = 0;
        return 1;
    }
    if (pDue == NULL || nMin > WM_TIMER || nMax < WM_TIMER) {
        return 0;
    }
    pMsg->hwnd = pDue->hWnd;
    pMsg->message = WM_TIMER;
    pMsg->wParam = pDue->nIDEvent;
    pMsg->lParam = 0;
    if (bRemove) {
        pDue->dwLastTime = GetTickCount();
    }
    return 1;
}

// 1.14d mac: 0002934e
// Depth first, the window itself before its children.
static HWND Wnd_FindWindowNeedingPaint(HWND hWnd) {
    if (hWnd->nNeedsPaint != 0) {
        return hWnd;
    }
    for (HWND hChild = hWnd->pFirstChild; hChild != NULL; hChild = hChild->pNextSibling) {
        HWND hFound = Wnd_FindWindowNeedingPaint(hChild);
        if (hFound != NULL) {
            return hFound;
        }
    }
    return NULL;
}

static BOOL IPeekMessage(MSG* pMsg, UINT nMsgFilterMin, UINT nMsgFilterMax, UINT wRemoveMsg);

// 1.14d mac: 0002937e
// Any window's messages: hWnd is not used.
BOOL PeekMessage(MSG* pMsg, HWND hWnd, UINT nMsgFilterMin, UINT nMsgFilterMax, UINT wRemoveMsg) {
    return IPeekMessage(pMsg, nMsgFilterMin, nMsgFilterMax, wRemoveMsg);
}

// 1.14d mac: 000293a1
// Finishes what the pump returns: WM_ACTIVATEAPP goes to the active window, a raw
// Mac key (wParam top bit set) becomes the Win32 key message, and mouse messages
// get their window and client point. Caps Lock comes back as a key-down on release.
static BOOL IPeekMessage(MSG* pMsg, UINT nMsgFilterMin, UINT nMsgFilterMax, UINT wRemoveMsg) {
    BOOL bRemove = wRemoveMsg & PM_REMOVE;
    BOOL bResult = 0;
    if (Msg_GetPendingMessage(pMsg, bRemove, nMsgFilterMin, nMsgFilterMax)) {
        UINT uMsg = pMsg->message;
        bResult = 1;
        switch (uMsg) {
        case WM_ACTIVATEAPP:
            pMsg->hwnd = GetActiveWindow();
            break;
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        {
            LONG nMacKey = pMsg->wParam;
            if (nMacKey < 0) {
                BYTE byCharCode = (BYTE)pMsg->lParam;
                BOOL bAltDown = (GetKeyState(VK_MENU) & 0x8000) != 0;
                Keys_PrepareKeyMsg(pMsg, bRemove, uMsg == WM_KEYDOWN, (BYTE)nMacKey, byCharCode, bAltDown);
                if (pMsg->wParam == VK_CAPITAL) {
                    if (pMsg->message == WM_KEYUP) {
                        pMsg->message = WM_KEYDOWN;
                        pMsg->lParam = (pMsg->lParam & 0x3FFF0000) | 1;
                    } else if (pMsg->message == WM_SYSKEYUP) {
                        pMsg->message = WM_SYSKEYDOWN;
                        pMsg->lParam = (pMsg->lParam & 0x3FFF0000) | 1;
                    }
                }
            }
            break;
        }
        case WM_NCMOUSEMOVE:
        case 0x200:
        case 0x201:
        case 0x202:
        case 0x204:
        case 0x205:
        case 0x207:
        case 0x208:
        case 0x20A:
        case 0x20B:
        case 0x20C:
            Msg_FillMouseMsgFromPoint(pMsg, pMsg->lParam);
            break;
        }
    }
    return bResult;
}

// 1.14d mac: 00029501
// Never waits: with nothing pending the MSG is cleared and the result is still TRUE.
BOOL GetMessage(MSG* pMsg, HWND hWnd, UINT nMsgFilterMin, UINT nMsgFilterMax) {
    if (IPeekMessage(pMsg, nMsgFilterMin, nMsgFilterMax, PM_REMOVE)) {
        return pMsg->message != WM_QUIT;
    }
    pMsg->message = 0;
    pMsg->hwnd = NULL;
    pMsg->lParam = 0;
    pMsg->wParam = 0;
    pMsg->pt.x = 0;
    pMsg->time = 0;
    pMsg->pt.y = 0;
    return 1;
}

// 1.14d mac: 0002956e
BOOL PostMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (hWnd == NULL || !IsWindow(hWnd) || g_pMsgState == NULL) {
        return 0;
    }
    MsgRing* pRing = &g_pMsgState->tPosted;
    DWORD dwTime = GetTickCount();
    int nNext = pRing->nTail + 1;
    if (nNext >= MSGRING_SIZE) {
        nNext = 0;
    }
    if (nNext == pRing->nHead) {
        return 0;
    }
    MSG* pMsg = &pRing->aMsgs[pRing->nTail];
    pMsg->hwnd = hWnd;
    pMsg->message = uMsg;
    pMsg->wParam = wParam;
    pMsg->lParam = lParam;
    pMsg->time = dwTime;
    pMsg->pt.x = 0;
    pMsg->pt.y = 0;
    pRing->nTail = nNext;
    return 1;
}

// 1.14d mac: 000295f5
// Straight to the window's procedure, or its class's.
LRESULT SendMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (hWnd == NULL) {
        return 0;
    }
    WNDPROC pfnWndProc = hWnd->pfnWndProc;
    if (pfnWndProc == NULL) {
        pfnWndProc = hWnd->pClass->pfnWndProc;
    }
    return CallWindowProc(pfnWndProc, hWnd, uMsg, wParam, lParam);
}

// 1.14d mac: 0002963c
// A WM_TIMER that carries a callback calls it; anything else is sent.
LONG DispatchMessage(const MSG* pMsg) {
    if (pMsg->hwnd == NULL) {
        return 0;
    }
    if (pMsg->message == WM_TIMER && pMsg->lParam != 0) {
        TIMERPROC pfnTimerFunc = (TIMERPROC)pMsg->lParam;
        UINT nIDEvent = pMsg->wParam;
        DWORD dwTime = GetTickCount();
        pfnTimerFunc(pMsg->hwnd, WM_TIMER, nIDEvent, dwTime);
        return 0;
    }
    return SendMessage(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
}

// 1.14d mac: 000296a6
void PostQuitMessage(int nExitCode) {
    if (g_pMsgState != NULL) {
        g_pMsgState->bQuit = 1;
        g_pMsgState->nExitCode = nExitCode;
    }
}

// 1.14d mac: 000296ce
// Replaces the window's timer of that id. Id 0 asks for a new one.
UINT SetTimer(HWND hWnd, UINT nIDEvent, UINT uElapse, TIMERPROC pfnTimerFunc) {
    if (nIDEvent == 0) {
        nIDEvent = --s_nLastTimerID;
    }
    KillTimer(hWnd, nIDEvent);
    Win32MacTimer* pTimer = (Win32MacTimer*)Priv_Alloc(sizeof(Win32MacTimer));
    if (pTimer == NULL) {
        return 0;
    }
    pTimer->hWnd = hWnd;
    pTimer->nIDEvent = nIDEvent;
    pTimer->uElapse = uElapse;
    pTimer->dwLastTime = GetTickCount();
    pTimer->pfnTimerFunc = pfnTimerFunc;
    pTimer->pNext = g_pMsgState->pTimers;
    g_pMsgState->pTimers = pTimer;
    return nIDEvent;
}

// 1.14d mac: 00029755
BOOL KillTimer(HWND hWnd, UINT nIDEvent) {
    Win32MacTimer* pPrev = NULL;
    for (Win32MacTimer* pTimer = g_pMsgState->pTimers; pTimer != NULL; pTimer = pTimer->pNext) {
        if (pTimer->hWnd == hWnd && pTimer->nIDEvent == nIDEvent) {
            if (pPrev != NULL) {
                pPrev->pNext = pTimer->pNext;
            } else {
                g_pMsgState->pTimers = pTimer->pNext;
            }
            Priv_Free(pTimer);
            return 1;
        }
        pPrev = pTimer;
    }
    return 0;
}

static BOOL MSG_EnqueueMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

// 1.14d mac: 000297c3
void FUN_000297c3(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MSG_EnqueueMessage(uMsg, wParam, lParam);
}

// 1.14d mac: 000297df
// Input for no window. A mouse move replaces the pending one instead of queueing.
static BOOL MSG_EnqueueMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    BOOL bResult = 0;
    MsgRing* pRing = &g_pMsgState->tInput;
    if (pRing != NULL) {
        DWORD dwTime = GetTickCount();
        if (uMsg == WM_MOUSEMOVE) {
            g_pMsgState->wMouseMoveWParam = wParam;
            g_pMsgState->lMouseMoveLParam = lParam;
            g_pMsgState->bMouseMovePending = 1;
            bResult = 1;
        } else {
            int nNext = pRing->nTail + 1;
            if (nNext >= MSGRING_SIZE) {
                nNext = 0;
            }
            if (nNext != pRing->nHead) {
                MSG* pMsg = &pRing->aMsgs[pRing->nTail];
                pMsg->hwnd = NULL;
                pMsg->message = uMsg;
                pMsg->wParam = wParam;
                pMsg->lParam = lParam;
                pMsg->time = dwTime;
                pMsg->pt.x = 0;
                pMsg->pt.y = 0;
                pRing->nTail = nNext;
                bResult = 1;
            }
        }
    }
    return bResult;
}

// 1.14d mac: 000298b2
// wParam is the Mac key code with the top bit set, lParam the character.
void MSG_PostKeyMessage(BOOL bKeyDown, BYTE byKeyCode, BYTE byCharCode) {
    MSG_EnqueueMessage(bKeyDown ? WM_KEYDOWN : WM_KEYUP, byKeyCode | 0x80000000, byCharCode);
}

// 1.14d mac: 000298e3
// Option down, F4 down, F4 up, Option up: Alt+F4 for the game.
void MSG_PostAltF4Keys(void) {
    MSG_EnqueueMessage(WM_KEYDOWN, 0x8000003A, 0);
    MSG_EnqueueMessage(WM_KEYDOWN, 0x80000076, 0);
    MSG_EnqueueMessage(WM_KEYUP, 0x80000076, 0);
    MSG_EnqueueMessage(WM_KEYUP, 0x8000003A, 0);
}

// 1.14d mac: 00029946
DWORD MSG_ExchangeCmdPeriodFlag(const DWORD* pdwNewValue) {
    if (pdwNewValue != NULL) {
        s_dwCmdPeriodFlag = *pdwNewValue;
    }
    return s_dwCmdPeriodFlag;
}

#endif  // D2_OS_MAC
