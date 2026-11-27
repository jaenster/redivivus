// unknown_app.cpp - the Mac shell's application glue: the application instance, the
// Win32-style menu commands posted to the game window, the default display mode, the update
// redraw, and the run and exit of the game.
//
// 1.14d Intel Mac (DiabloII_macho) 0019bbe8..0019bed3. No binary records this unit's name.
// It starts at the padding to 0019bbe8 (its jump table gives it 4-byte alignment) and defines
// the statics 00536c40/00536c48. The units around it call its one-line functions out of line:
// MAC_HandleUpdateEvent FUN_0019be2c, FUN_0019bed3 (GameSetup.cpp) and FUN_0019cc81
// MAC_GetDefaultDisplayMode; FUN_0019be53/be79 call the input unit's FUN_0019b290/b2a2/b2b4
// out of line and reach 005c865c through a non-lazy pointer. See Source/MacSpecific/README.md.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC` (see
// Source/MacSpecific/README.md).
#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

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
void FUN_0019be53(int nExitCode);
// 1.14d mac: 0019b290
void FUN_0019b290(void);
// 1.14d mac: 0019b2a2
void FUN_0019b2a2(void);
// 1.14d mac: 0019b2b4
void FUN_0019b2b4(int nExitCode);
void MAC_GetDefaultDisplayMode(int* pnWidth, int* pnHeight, int* pnBitDepth);
void* FUN_0019bbe8(void);
// 1.14d mac: 005c865c
extern int g_nMacAppInstance;
// 1.14d mac: 00030cb2
struct Win32MacWindow;
struct RECT;
Win32MacWindow* GetActiveWindow(void);
// 1.14d mac: 000305a7
Win32MacWindow* GetFocus(void);
// 1.14d mac: 0002956e
BOOL PostMessage(Win32MacWindow* hWnd, uint32_t nMessage, uint32_t nWParam, int32_t nLParam);
// 1.14d mac: 000298e3
void MSG_PostAltF4Keys(void);
// 1.14d mac: 0002a16f
void* GetModuleHandle(const char* szModuleName);
// 1.14d mac: 000312bf
BOOL InvalidateRect(Win32MacWindow* hWnd, const RECT* pRect, BOOL bErase);
// 1.14d mac: 005c8a38
// Set when the display did not end up in the state asked for. Defined here: MAC_HandleMenuCommand
// stores to it directly, the display unit reaches it through a non-lazy pointer.
int g_bMacDisplayMismatch;
// 1.14d mac: 0019ae9c
int MAC_ParseCommandLineAndRun(int nInstance, int nUnused1, int nUnused2, int nUnused3);

// 1.14d mac: 0019bbe8
// The application's module handle, asked for once.
void* FUN_0019bbe8(void) {
    static void* s_hModule = GetModuleHandle(NULL);
    return s_hModule;
}

// 1.14d mac: 0019bc4b
// A menu item: the edit commands go to the focused window as WM_CUT/COPY/PASTE/CLEAR (and the
// two Storm-specific ones), quit sets the display flag, Alt-F4 is posted for the close item.
uint8_t MAC_HandleMenuCommand(uint32_t nCommand) {
    Win32MacWindow* hWnd;
    uint8_t bHandled = 0;
    if (nCommand < 0x81) {
        switch (nCommand) {
        case 4:
            hWnd = GetActiveWindow();
            if (hWnd == NULL) {
                break;
            }
            PostMessage(hWnd, 0x3A2, 0, 0);
            bHandled = 1;
            break;
        case 7:
            MSG_PostAltF4Keys();
            bHandled = 1;
            break;
        case 9:
            hWnd = GetFocus();
            if (hWnd == NULL && (hWnd = GetActiveWindow()) == NULL) {
                break;
            }
            PostMessage(hWnd, 0x300, 0, 0);
            bHandled = 1;
            break;
        case 10:
            hWnd = GetFocus();
            if (hWnd == NULL && (hWnd = GetActiveWindow()) == NULL) {
                break;
            }
            PostMessage(hWnd, 0x301, 0, 0);
            bHandled = 1;
            break;
        case 11:
            hWnd = GetFocus();
            if (hWnd == NULL && (hWnd = GetActiveWindow()) == NULL) {
                break;
            }
            PostMessage(hWnd, 0x302, 0, 0);
            bHandled = 1;
            break;
        case 12:
            hWnd = GetFocus();
            if (hWnd == NULL && (hWnd = GetActiveWindow()) == NULL) {
                break;
            }
            PostMessage(hWnd, 0x303, 0, 0);
            bHandled = 1;
            break;
        case 13:
            hWnd = GetFocus();
            if (hWnd == NULL && (hWnd = GetActiveWindow()) == NULL) {
                break;
            }
            PostMessage(hWnd, 0x3A6, 0, 0);
            bHandled = 1;
            break;
        default:
            break;
        }
    } else {
        if (nCommand != 0x81) {
            return bHandled;
        }
        g_bMacDisplayMismatch = 1;
        bHandled = 1;
    }
    return bHandled;
}

// 1.14d mac: 0019be00
// The display mode the set-up assumes before the game has chosen one.
void MAC_GetDefaultDisplayMode(int* pnWidth, int* pnHeight, int* pnBitDepth) {
    if (pnWidth != NULL) {
        *pnWidth = 800;
    }
    if (pnHeight != NULL) {
        *pnHeight = 600;
    }
    if (pnBitDepth != NULL) {
        *pnBitDepth = 8;
    }
}

// 1.14d mac: 0019be2c
void FUN_0019be2c(void) {
    InvalidateRect(NULL, NULL, 1);
}


// 1.14d mac: 0019be53
void FUN_0019be53(int nExitCode) {
    FUN_0019b2a2();
    g_nMacAppInstance = 0;
    FUN_0019b2b4(nExitCode);
}

// 1.14d mac: 0019be79
// Runs the game: the handlers on, the command line parsed and the application run, then the
// handlers off again.
void FUN_0019be79(void) {
    FUN_0019b290();
    int nExitCode = MAC_ParseCommandLineAndRun((int)FUN_0019bbe8(), 0, 0, 0);
    FUN_0019b2a2();
    g_nMacAppInstance = 0;
    FUN_0019b2b4(nExitCode);
}

#endif  // D2_OS_MAC
