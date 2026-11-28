// unknown_input.cpp - the Mac shell's input side: the Carbon mouse handler and the mouse and
// key messages it posts to the game window, activation and update events, the exit handler and
// the start-up hooks (main thread, Carbon handler, system check).
//
// 1.14d Intel Mac (DiabloII_macho) 0019b290..0019bbe8. No binary records this unit's name.
// Bounded by padding on both sides (to 0019b290, a 16-byte boundary, and to 0019bbe8); its
// file-local state 00536c28..00536c3d is addressed directly by every function here and by
// none outside, and the globals 005c8a34..36 and 003b8d78 are defined here. The other shell
// units call its one-line handlers out of line. See Source/MacSpecific/README.md.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_MAC_CARBON` (see
// Source/MacSpecific/README.md).
#include "D2Arch.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include <Carbon/Carbon.h>
#include <pthread.h>
#include <stdlib.h>

#include <QuickTime/QuickTime.h>
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <string.h>

#include "Source/MacSpecific/Source/MacShell/QuickDrawMac.h"

#include "Source/StormMac/H/stpl.h"

typedef int BOOL;
// 1.14d mac: 00032bcd
void DirectSound_SetMinVolumeDelay(uint32_t nMilliseconds);
// The application's procedure table (the slots after the shell's own: 0x6c, 0x70 and 0x74 bracket
// a dialog the way the shell wants the app told).
struct MacAppProcs {
    void (*apfn[64])(void);
};
// 1.14d mac: 00397abc
// The pointer to the display driver table (g_pDisplayDriver, StormMac/MacDisplayDriver.cpp, 00397abc).
extern MacAppProcs* g_pMacAppProcs __asm__("_g_pDisplayDriver");
void FUN_0019b290(void);
void FUN_0019b2a2(void);
void FUN_0019b2b4(int nExitCode);
bool MAC_PostKeyDown(EventRecord* pEvent);
uint8_t MAC_HandleMouseDownEvent(EventRecord* pEvent);
uint8_t MAC_HandleMouseMoved(EventRecord* pEvent);
uint8_t MAC_HandleMouseUpEvent(EventRecord* pEvent);
bool MAC_HandleUpdateEvent(EventRecord* pEvent);
bool MAC_PostKeyUp(EventRecord* pEvent);
uint8_t FUN_0019bac5(EventRecord* pEvent);
uint8_t FUN_0019bbcd(uint32_t nCommand);
bool MAC_OnApplicationDeactivate(EventRecord* pEvent);
bool MAC_OnApplicationActivate(EventRecord* pEvent);
// 1.14d mac: 000298b2
void MSG_PostKeyMessage(int bKeyDown, uint8_t byKeyCode, uint8_t byChar);
// 1.14d mac: 0019cc78
void MAC_DSpShutdown(void);
// 1.14d mac: 0019bed3
// 1.14d mac: 0019bed3
Boolean FUN_0019bed3(void);
static void FUN_0019b732(void);
// 1.14d mac: 0002a40b
void PROCESS_SetExitHandler(void (*pfnExitHandler)(void));
// 1.14d mac: 0002c02c
uint32_t GetCurrentThreadId(void);
// 1.14d mac: 00031060
struct Win32MacWindow;
struct RECT;
BOOL GetWindowRect(Win32MacWindow* hWnd, RECT* pRect);
// 1.14d mac: 000297c3
void FUN_000297c3(uint32_t nMessage, uint32_t nWParam, int32_t nLParam);
namespace D2gfx::D2GFX {
// 1.14d mac: 002b5b6d
void* GetHwnd(void);
}
using D2gfx::D2GFX::GetHwnd;
namespace D2gfx::D2GFX {
// 1.14d mac: 002b5b81
int D2GFX_GetScreenSize(int* pnWidth, int* pnHeight);
}
using D2gfx::D2GFX::D2GFX_GetScreenSize;
// 1.14d mac: 002b5da4
// Hands a Carbon event to the renderer; 0 without one.
uint8_t FUN_002b5da4(EventRecord* pEvent);
// 1.14d mac: 002b5dc3
// Converts a point to the renderer's coordinates; 0 without a renderer.
int FUN_002b5dc3(Point* pPoint);
// 1.14d mac: 003b8d78
uint32_t g_nMacMainThread = (uint32_t)-1;


// The shell's input state: the Carbon handlers and the Storm callbacks work on these.
// 1.14d mac: 005c8a34
uint8_t g_bMacQuitRequested;
// 1.14d mac: 005c8a35
uint8_t g_bMacCapsLock;
// 1.14d mac: 005c8a36
uint16_t g_wMacModifiers;
// 1.14d mac: 00536c28
// Set while the Carbon handlers may post: mouse and key messages, activation, updates.
static uint8_t s_bEventsEnabled;
// 1.14d mac: 00536c29
bool g_bMacExiting;
// 1.14d mac: 00536c37
// Pressed state of the mouse buttons 0..5 (1 left, 2 right, 3 middle, 4 and 5 the extra ones).
static uint8_t s_abMouseButtons[6];
// 1.14d mac: 00536c3d
// Set while the right button is emulated by a command-click.
bool g_bMacCommandClick;

// 1.14d mac: 0019b290
// Lets the handlers below post.
void FUN_0019b290(void) {
    s_bEventsEnabled = 1;
}

// 1.14d mac: 0019b2a2
void FUN_0019b2a2(void) {
    s_bEventsEnabled = 0;
}

// 1.14d mac: 0019b2b4
void FUN_0019b2b4(int nExitCode) {
    g_bMacQuitRequested = 1;
    g_nMacMainThread = (uint32_t)-1;
}

// 1.14d mac: 0019b2d1
void MAC_ApplyMouseSensitivity(int nMouseSensitivity) {
}

static uint8_t MAC_PostMouseMessage(uint32_t nMessage, uint32_t nWParamHigh, uint16_t nModifiers, Point ptMouse);

// 1.14d mac: 0019b2d9
// Whether a point (in window coordinates) is on the game's screen.
bool MAC_IsMousePointInGameScreen(Point ptMouse) {
    if (s_bEventsEnabled == 1) {
        Point ptLocal = ptMouse;
        if (FUN_002b5dc3(&ptLocal) != 0) {
            int nScreenWidth;
            int nScreenHeight;
            D2GFX_GetScreenSize(&nScreenWidth, &nScreenHeight);
            if (-1 < ptLocal.h && ptLocal.h <= nScreenWidth) {
                if (-1 < ptLocal.v) {
                    return ptLocal.v <= nScreenHeight;
                }
            }
        }
    }
    return false;
}

static uint8_t FUN_0019b51a(uint32_t nButton, uint8_t bPressed, uint16_t nModifiers, Point ptMouse);

// 1.14d mac: 0019b33e
// The Carbon 'mous' handler: the wheel is posted as WM_MOUSEWHEEL, and the buttons from the
// third on as button presses; the first two come with the classic events.
OSStatus MAC_HandleMouseCarbonEvent(EventHandlerCallRef hCallRef, EventRef hEvent, void* pUserData) {
    OSStatus nStatus = eventNotHandledErr;
    if (GetEventClass(hEvent) == kEventClassMouse) {
        Point ptMouse;
        GetEventParameter(hEvent, kEventParamMouseLocation, typeQDPoint, NULL, 4, NULL, &ptMouse);
        if (MAC_IsMousePointInGameScreen(ptMouse)) {
            UInt32 nKind = GetEventKind(hEvent);
            if (nKind == kEventMouseWheelMoved) {
                int32_t nWheelDelta;
                UInt32 nModifiers;
                GetEventParameter(hEvent, kEventParamMouseWheelDelta, typeLongInteger, NULL, 4, NULL, &nWheelDelta);
                GetEventParameter(hEvent, kEventParamKeyModifiers, typeUInt32, NULL, 4, NULL, &nModifiers);
                uint32_t nDelta = (uint32_t)(nWheelDelta * 0x78000) >> 12;
                MAC_PostMouseMessage(0x20A, nDelta & 0xFFFF, (uint16_t)nModifiers, ptMouse);
            } else {
                if (1 < nKind - 1) {
                    return eventNotHandledErr;
                }
                EventMouseButton nButton;
                UInt32 nModifiers;
                GetEventParameter(hEvent, kEventParamMouseButton, typeMouseButton, NULL, 2, NULL, &nButton);
                if (nButton < 2) {
                    return eventNotHandledErr;
                }
                GetEventParameter(hEvent, kEventParamKeyModifiers, typeUInt32, NULL, 4, NULL, &nModifiers);
                FUN_0019b51a(nButton, nKind == 1, (uint16_t)nModifiers, ptMouse);
            }
            nStatus = noErr;
        }
    }
    return nStatus;
}

// 1.14d mac: 0019b51a
// A new pressed state of a mouse button: posted as the matching WM_*BUTTON message when it
// changed. A command-click on the left button counts as the right one.
static uint8_t FUN_0019b51a(uint32_t nButton, uint8_t bPressed, uint16_t nModifiers, Point ptMouse) {
    uint32_t nMessage;
    uint32_t nWParamHigh;
    bool bCanPost;
    uint8_t bRightHeld;
    if (nButton > 5 || s_abMouseButtons[nButton] == bPressed) {
        return 0;
    }
    s_abMouseButtons[nButton] = bPressed;
    if (nButton == 1) {
        if (!bPressed) {
            if (g_bMacCommandClick == 1) {
                bRightHeld = s_abMouseButtons[2];
                g_bMacCommandClick = 0;
                goto emulate;
            }
        } else if (nModifiers & 0x100) {
            bRightHeld = s_abMouseButtons[2];
            g_bMacCommandClick = 1;
        emulate:
            bCanPost = bRightHeld == 0;
            nButton = 2;
            goto dispatch;
        }
        nMessage = 0x201 + (bPressed == 0);
        nWParamHigh = 0;
    } else {
        bCanPost = !((nButton == 2) & g_bMacCommandClick);
    dispatch:
        if (!bCanPost) {
            return 0;
        }
        switch (nButton) {
        case 1:
            nMessage = 0x201 + (bPressed == 0);
            nWParamHigh = 0;
            break;
        case 2:
            nMessage = (bPressed == 0) | 0x204;
            nWParamHigh = 0;
            break;
        case 3:
            nMessage = 0x207 + (bPressed == 0);
            nWParamHigh = 0;
            break;
        case 4:
            nMessage = (bPressed == 0) + 0x20B;
            nWParamHigh = 1;
            break;
        case 5:
            nMessage = (bPressed == 0) + 0x20B;
            nWParamHigh = 2;
            break;
        default:
            return 0;
        }
    }
    return MAC_PostMouseMessage(nMessage, nWParamHigh, nModifiers, ptMouse);
}

// 1.14d mac: 0019b636
// Starts the shell: Storm's exit handler, the Carbon mouse handler for the wheel and the
// extra buttons, the system and archive checks; on success the heap is purged and compacted.
bool FUN_0019b636(void) {
    PROCESS_SetExitHandler(FUN_0019b732);
    static EventHandlerUPP s_pfnMouseHandler = NewEventHandlerUPP(MAC_HandleMouseCarbonEvent);
    EventTypeSpec aEvents[3] = {{kEventClassMouse, kEventMouseDown}, {kEventClassMouse, kEventMouseUp}, {kEventClassMouse, kEventMouseWheelMoved}};
    InstallEventHandler(GetApplicationEventTarget(), s_pfnMouseHandler, 3, aEvents, NULL, NULL);
    bool bResult = false;
    if (FUN_0019bed3()) {
        PurgeMem(0x7FFFFFF0);
        CompactMem(0x7FFFFFF0);
        g_nMacMainThread = GetCurrentThreadId();
        bResult = true;
    }
    return bResult;
}

// 1.14d mac: 0019b732
// Storm's exit handler: the run is over.
static void FUN_0019b732(void) {
    g_bMacExiting = true;
}

// 1.14d mac: 0019b744
void FUN_0019b744(void) {
    MAC_DSpShutdown();
}

// 1.14d mac: 0019b74d
// Keeps the modifier keys of an event, and passes the event on while posting is on.
uint8_t FUN_0019b74d(EventRecord* pEvent) {
    g_wMacModifiers = pEvent->modifiers;
    g_bMacCapsLock = (pEvent->modifiers >> 10) & 1;
    if (!s_bEventsEnabled) {
        return 0;
    }
    return FUN_002b5da4(pEvent);
}

// 1.14d mac: 0019b795
int FUN_0019b795(void) {
    DirectSound_SetMinVolumeDelay(1500);
    return 0;
}

// 1.14d mac: 0019b7ae
uint8_t MAC_HandleMouseDownEvent(EventRecord* pEvent) {
    return FUN_0019b51a(1, true, pEvent->modifiers, pEvent->where);
}

// 1.14d mac: 0019b7e2
uint8_t MAC_HandleMouseMoved(EventRecord* pEvent) {
    return MAC_PostMouseMessage(0x200, 0, pEvent->modifiers, pEvent->where);
}

// 1.14d mac: 0019b813
// Posts a mouse message to Storm: the position clamped to the screen, the buttons and
// modifier keys as the wParam, or a non-client move when the point is outside the window.
static uint8_t MAC_PostMouseMessage(uint32_t nMessage, uint32_t nWParamHigh, uint16_t nModifiers, Point ptMouse) {
    int aWindowRect[4];
    int nScreenWidth;
    int nScreenHeight;
    uint32_t nPoint = *(uint32_t*)&ptMouse;
    GetWindowRect((Win32MacWindow*)GetHwnd(), (RECT*)aWindowRect);
    Point ptOrigin = {0, 0};
    LocalToGlobal(&ptOrigin);
    aWindowRect[0] = ptOrigin.h;
    aWindowRect[2] = aWindowRect[2] + aWindowRect[0];
    aWindowRect[1] = ptOrigin.v;
    aWindowRect[3] = aWindowRect[3] + aWindowRect[1];
    int nX = (int)nPoint >> 16;
    uint8_t bPosted = 0;
    bool bInWindow = false;
    if (nX < aWindowRect[2] && aWindowRect[0] <= nX) {
        bInWindow = (short)nPoint < aWindowRect[3] && aWindowRect[1] <= (short)nPoint;
    }
    if (s_bEventsEnabled && (uint8_t)FUN_002b5dc3(&ptMouse)) {
        D2GFX_GetScreenSize(&nScreenWidth, &nScreenHeight);
        if (ptMouse.h < 0) {
            ptMouse.h = 0;
        }
        if (nScreenWidth <= ptMouse.h) {
            ptMouse.h = nScreenWidth - 1;
        }
        if (ptMouse.v < 0) {
            ptMouse.v = 0;
        }
        if (nScreenHeight <= ptMouse.v) {
            ptMouse.v = nScreenHeight - 1;
        }
        uint32_t nWParam;
        uint32_t nLParam;
        if (bInWindow) {
            nWParam = (s_abMouseButtons[1] != 0) | (nWParamHigh << 16);
            if (s_abMouseButtons[2]) nWParam += 2;
            if (s_abMouseButtons[3]) nWParam |= 0x10;
            if (s_abMouseButtons[4]) nWParam |= 0x20;
            if (s_abMouseButtons[5]) nWParam |= 0x40;
            if (nModifiers & 0x200) nWParam |= 4;
            if (!s_bEventsEnabled) {
                return 0;
            }
            nLParam = (uint32_t)ptMouse.h | ((uint32_t)(uint16_t)ptMouse.v << 16);
            if (nModifiers & 0x1000) nWParam |= 8;
        } else {
            if (!s_bEventsEnabled) {
                return 0;
            }
            nLParam = (nPoint << 16) | nX;
            nMessage = 0xA0;
            nWParam = 2;
        }
        FUN_000297c3(nMessage, nWParam, nLParam);
        bPosted = 1;
    }
    return bPosted;
}

// 1.14d mac: 0019ba0c
uint8_t MAC_HandleMouseUpEvent(EventRecord* pEvent) {
    return FUN_0019b51a(1, false, pEvent->modifiers, pEvent->where);
}

// 1.14d mac: 0019ba3d
// A key went down: posted to Storm while the handlers are on.
bool MAC_PostKeyDown(EventRecord* pEvent) {
    if (s_bEventsEnabled) {
        MSG_PostKeyMessage(1, (uint8_t)(pEvent->message >> 8), (uint8_t)pEvent->message);
        return true;
    }
    return false;
}

// 1.14d mac: 0019ba81
bool MAC_PostKeyUp(EventRecord* pEvent) {
    if (s_bEventsEnabled) {
        MSG_PostKeyMessage(0, (uint8_t)(pEvent->message >> 8), (uint8_t)pEvent->message);
        return true;
    }
    return false;
}

// 1.14d mac: 0019bac5
uint8_t FUN_0019bac5(EventRecord* pEvent) {
    return 0;
}

// 1.14d mac: 0019bacc
// The application was suspended: lets go of every held mouse button, posts WM_ACTIVATEAPP
// (inactive) and tells the application.
bool MAC_OnApplicationDeactivate(EventRecord* pEvent) {
    Point ptZero = {0, 0};
    for (uint32_t nButton = 1; nButton != 6; nButton++) {
        if (s_abMouseButtons[nButton]) {
            FUN_0019b51a(nButton, false, 0, ptZero);
        }
    }
    bool bPosted;
    if (s_bEventsEnabled) {
        FUN_000297c3(0x1C, 0, 0);
        bPosted = true;
    } else {
        bPosted = false;
    }
    g_pMacAppProcs->apfn[28]();
    return bPosted;
}

// 1.14d mac: 0019bb55
// The application was resumed.
bool MAC_OnApplicationActivate(EventRecord* pEvent) {
    bool bPosted;
    if (s_bEventsEnabled) {
        FUN_000297c3(0x1C, 1, 0);
        bPosted = true;
    } else {
        bPosted = false;
    }
    g_pMacAppProcs->apfn[29]();
    return bPosted;
}

void FUN_0019be2c(void);

// 1.14d mac: 0019bba4
// An update event: the whole window is redrawn.
bool MAC_HandleUpdateEvent(EventRecord* pEvent) {
    if (s_bEventsEnabled) {
        FUN_0019be2c();
        return true;
    }
    return false;
}

uint8_t MAC_HandleMenuCommand(uint32_t nCommand);

// 1.14d mac: 0019bbcd
uint8_t FUN_0019bbcd(uint32_t nCommand) {
    return MAC_HandleMenuCommand(nCommand);
}

#endif  // D2_OS_MAC
