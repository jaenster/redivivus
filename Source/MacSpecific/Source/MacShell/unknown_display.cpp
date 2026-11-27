// unknown_display.cpp - the Mac shell's display set-up: the video-mode switch between windowed
// and full screen, the display module's options, the display hook set-up, the cursor callbacks
// and the menu-bar enable/disable.
//
// 1.14d Intel Mac (DiabloII_macho) 0019c7e4..0019cf88. No binary records this unit's name.
// Bounded by padding on both sides (one byte to 0019c7e4, three to 0019cf88). It defines the
// video mode 003b8d7c and the statics 00536c50/00536c54, and takes the addresses of its cursor
// and display callbacks with direct references, while it reaches 005c8a38 (unknown_app.cpp's)
// and the configuration 005c8660 through non-lazy pointers. See Source/MacSpecific/README.md.
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
// The application's procedure table (the slots after the shell's own: 0x6c, 0x70 and 0x74 bracket
// a dialog the way the shell wants the app told).
struct MacAppProcs {
    void (*apfn[64])(void);
};
// 1.14d mac: 00397abc
// The pointer to the display driver table (g_pDisplayDriver, StormMac/MacDisplayDriver.cpp, 00397abc).
extern MacAppProcs* g_pMacAppProcs __asm__("_g_pDisplayDriver");
// 1.14d mac: 0019be00
void MAC_GetDefaultDisplayMode(int* pnWidth, int* pnHeight, int* pnBitDepth);
// 1.14d mac: 00036d19
bool MACDISPLAY_SelectDriver(int nDriver);
// 1.14d mac: 00036d8b
void MACCURSOR_RegisterGuardProc(bool (*pfnGuard)(bool));
// 1.14d mac: 00036d9f
void MACCURSOR_RegisterSetCursorPosProc(bool (*pfnSetCursorPos)(int16_t*, int16_t*));
// 1.14d mac: 00036db3
void MACCURSOR_RegisterGetCursorPosProc(bool (*pfnGetCursorPos)(int*, int*));
// 1.14d mac: 00036830
void MACCURSOR_ShowDisplayCursor(void);
// 1.14d mac: 00036847
void MACCURSOR_HideDisplayCursor(void);
// 1.14d mac: 00036ff5
void MACCURSOR_InitCursor(void);
// 1.14d mac: 0001e550
void FPSetMessageBoxCallback(void (*pfnCallback)(void));
// 1.14d mac: 0001e9bb
void MAC_GetGlobalMouse(Point* pPoint);
// 1.14d mac: 0001e9c4
void MAC_WarpMouseCursorPosition(Point ptCursor);
// 1.14d mac: 002b5de2
// Converts a game point to global coordinates; 0 without a renderer.
int FUN_002b5de2(Point* pPoint);
bool MAC_SetCursorPos(int16_t* pnX, int16_t* pnY);
bool MAC_GetCursorPos(int* pnX, int* pnY);
// 1.14d mac: 0019ced5
void MAC_DisableMenus(void);
void MAC_DSpShutdown(void);
// 1.14d mac: 002b5dc3
// Converts a point to the renderer's coordinates; 0 without a renderer.
int FUN_002b5dc3(Point* pPoint);
extern "C" OSStatus DSpShutdown(void);
// 1.14d mac: 0003ad51
int SRegDeleteValue(const char* szKeyName, const char* szValueName, uint32_t dwFlags);
// 1.14d mac: 00034519
void fp_fatal(const char* szFormat, ...);
// 1.14d mac: 002b4d4f
int D2GFX_GetRendererMode(void);
// 1.14d mac: 002b4b3c
// Option 10 reads back whether the renderer is busy; 2 and 3 take a pointer on the Mac.
void D2GFX_SetOption(int nOption, intptr_t nValue);
// 1.14d mac: 002b4b72
int D2GFX_SwitchModule(int nVideoMode, int bFullScreen, int nStage);
// 1.14d mac: 002b4d60
// The display state D2gfx is in: 1 full screen, 0 windowed.
int FUN_002b4d60(void);
// 1.14d mac: 002b5c1c
Boolean FUN_002b5c1c(void);
// Options 2 and 3 are eight-byte values D2gfx takes by address; their fields are unknown.
struct MacDisplayOption {
    int32_t n00;
    int32_t n04;
};
// 1.14d mac: 002b603a
void MACGFX_EnumerateDisplayModes(int* pnDisplayModule, int* pnOption4, int* pnOption5, int* pnOption0,
                                  MacDisplayOption* pOption2, MacDisplayOption* pOption3);
// 1.14d mac: 0005e6ac
void CLIENT_SetRendererMode(int nVideoMode, int bFullScreen);
// 1.14d mac: 0004d278
namespace D2Win {
namespace Src {
namespace D2WinSmack {
void D2WINSMACK_SetKeepTempFile(void);
}
}
}
// The configuration Game/Main.cpp parses the command line into (its D2IniConfigStrc, 0x3CD
// bytes); only the bytes the shell uses are laid out.
struct MacDisplayState {
    uint8_t unk00[8];
    bool bFullScreen;
    uint8_t unk09;
    uint8_t b0A;
    uint8_t b0B;
    uint8_t b0C;
    uint8_t b0D;
    uint8_t unk0E[0x211 - 0x0E];
    void* pfnHook __attribute__((packed));
    uint8_t unk215[0x3CD - 0x215];
};
static_assert(sizeof(void*) != 4 || offsetof(MacDisplayState, pfnHook) == 0x211, "MacDisplayState::pfnHook");
static_assert(sizeof(void*) != 4 || sizeof(MacDisplayState) == 0x3CD, "MacDisplayState");
// 1.14d mac: 005c8660
extern MacDisplayState g_tMacDisplayState;
// 1.14d mac: 005c8a38
// Set when the display did not end up in the state asked for.
extern int g_bMacDisplayMismatch;
// 1.14d mac: 003b8d7c
// The D2gfx video mode (renderer module) the Mac shell runs.
int g_nMacVideoMode = 2;
// 1.14d mac: 00536c50
// Set while FUN_0019cf50 runs: the switch then does not wait for the renderer to go idle.
static bool s_bSkipRendererIdleWait;

// 1.14d mac: 0019c7e4
// A display switch failed: forget every video setting, tell the user (STR# 200, 15 and 16)
// and quit.
void MAC_ResetVideoSettingsAndExit(void) {
    Str255 szError;
    Str255 szExplanation;
    DialogItemIndex nItemHit;
    SRegDeleteValue("Diablo II Mac", "Video Mode", 0);
    SRegDeleteValue("Diablo II Mac", "Video Engine", 0);
    SRegDeleteValue("Diablo II Mac", "Display ID", 0);
    SRegDeleteValue("Diablo II Mac", "Video Detect", 0);
    SRegDeleteValue("Diablo II Mac", "Engine Names", 0);
    SRegDeleteValue("Diablo II Mac", "32 Bit Video", 0);
    SRegDeleteValue("Diablo II Mac", "1-Resolution", 0);
    SRegDeleteValue("Diablo II Mac", "1-Expert", 0);
    SRegDeleteValue("Diablo II Mac", "Dont Show Again", 0);
    GetIndString(szError, 200, 15);
    GetIndString(szExplanation, 200, 16);
    StandardAlert(kAlertStopAlert, szError, szExplanation, 0, &nItemHit);
    exit(1);
}

int FUN_0019c96e(Boolean bFullScreen);
void MAC_ApplyDisplayModuleSettings(void);

// 1.14d mac: 0019c953
void FUN_0019c953(void) {
    FUN_0019c96e(FUN_002b5c1c());
}

// 1.14d mac: 0019c96e
// Switches D2gfx between windowed and full screen, unless it is already there; without a
// renderer only the flag is recorded. Waits for the renderer to go idle first.
int FUN_0019c96e(Boolean bFullScreen) {
    int nRendererMode = D2GFX_GetRendererMode();
    int nState = FUN_002b4d60();
    if (!nRendererMode) {
        if (bFullScreen) {
            g_tMacDisplayState.b0A = 0;
            g_tMacDisplayState.b0B = 0;
            g_tMacDisplayState.b0C = 0;
            g_tMacDisplayState.b0D = 0;
            g_tMacDisplayState.bFullScreen = true;
        } else {
            g_tMacDisplayState.bFullScreen = false;
        }
        return 1;
    }
    if (bFullScreen && nState == 1)
        return 1;
    if (!bFullScreen && nState == 0)
        return 1;
    if (!s_bSkipRendererIdleWait) {
        int bBusy = 0;
        do {
            D2GFX_SetOption(10, (intptr_t)&bBusy);
            if (!bBusy)
                break;
            pthread_yield_np();
        } while (bBusy);
    }
    if (bFullScreen) {
        int nVideoMode = g_nMacVideoMode;
        if (!D2GFX_SwitchModule(nVideoMode, 1, 1))
            MAC_ResetVideoSettingsAndExit();
        CLIENT_SetRendererMode(nVideoMode, 1);
        g_tMacDisplayState.b0A = 0;
        g_tMacDisplayState.b0B = 1;
        g_tMacDisplayState.b0C = 0;
        g_tMacDisplayState.b0D = 0;
        g_tMacDisplayState.bFullScreen = 1;
        MAC_ApplyDisplayModuleSettings();
        if (!D2GFX_SwitchModule(nVideoMode, 1, 2))
            MAC_ResetVideoSettingsAndExit();
    } else {
        if (!D2GFX_SwitchModule(g_nMacVideoMode, 0, 1)) {
            g_nMacVideoMode = 2;
            if (!D2GFX_SwitchModule(2, 0, 1))
                MAC_ResetVideoSettingsAndExit();
        }
        CLIENT_SetRendererMode(g_nMacVideoMode, 0);
        g_tMacDisplayState.b0A = 0;
        g_tMacDisplayState.b0B = 1;
        g_tMacDisplayState.b0C = 0;
        g_tMacDisplayState.b0D = 0;
        g_tMacDisplayState.bFullScreen = 0;
        MAC_ApplyDisplayModuleSettings();
        if (!D2GFX_SwitchModule(g_nMacVideoMode, 0, 2))
            MAC_ResetVideoSettingsAndExit();
    }
    D2Win::Src::D2WinSmack::D2WINSMACK_SetKeepTempFile();
    return 1;
}

// 1.14d mac: 0019cb40
// Flags g_bMacDisplayMismatch when D2gfx is not in the display state asked for.
Boolean FUN_0019cb40(Boolean bFullScreen) {
    int nState = FUN_002b4d60();
    if (bFullScreen) {
        if (nState == 1)
            return 1;
    } else if (nState == 0) {
        return 1;
    }
    g_bMacDisplayMismatch = 1;
    return 1;
}

// 1.14d mac: 0019cb78
// The display module the set-up chose, as a D2gfx video mode, and its options.
void MAC_ApplyDisplayModuleSettings(void) {
    int nDisplayModule;
    int nOption4;
    int nOption5;
    MacDisplayOption tOption2;
    MacDisplayOption tOption3;
    int nOption0;
    MACGFX_EnumerateDisplayModes(&nDisplayModule, &nOption4, &nOption5, &nOption0, &tOption2, &tOption3);
    switch (nDisplayModule) {
    case 1:
        g_nMacVideoMode = 2;
        break;
    case 2:
        g_nMacVideoMode = 4;
        break;
    case 3:
        g_nMacVideoMode = 5;
        break;
    case 4:
        g_nMacVideoMode = 7;
        break;
    default:
        fp_fatal("Unrecognized display module!");
    }
    D2GFX_SetOption(5, nOption5);
    D2GFX_SetOption(0, nOption0);
    D2GFX_SetOption(2, (intptr_t)&tOption2);
    D2GFX_SetOption(3, (intptr_t)&tOption3);
    D2GFX_SetOption(4, nOption4);
}

// 1.14d mac: 0019cc78
void MAC_DSpShutdown(void) {
    DSpShutdown();
}

// 1.14d mac: 0019cf50
// Checks that D2gfx is full screen, without waiting for the renderer.
void FUN_0019cf50(void) {
    s_bSkipRendererIdleWait = true;
    FUN_0019cb40(1);
    s_bSkipRendererIdleWait = false;
}

// The two display set-up records the application's display hook takes (their meaning is
// the renderer's; only the fields the shell fills are named).
struct MacDisplayRequest {
    int32_t nWidth;
    int32_t nHeight;
    int32_t nPitch;
    int32_t nBitsPerPixel;
    uint8_t byDepth;
    uint8_t byFlags;
};
struct MacDisplayLimits {
    int32_t nWidth;
    int32_t nHeight;
    int32_t nPitch;
    int32_t nMaxBitsPerPixel;
    int32_t nDepth;
    int32_t nFlags;
};

// 1.14d mac: 00536c54
// The display module the set-up chose (the first output of MACGFX_EnumerateDisplayModes).
static int s_nDisplayModule;


// 1.14d mac: 0019cc81
// Brings the process to the front and sets the display up through the application's display
// hook; the cursor and display-state callbacks go to Storm and the cursor module. False (and
// DrawSprocket shut down again) when the display is not available.
Boolean FUN_0019cc81(int nDisplayModule, int nUnused) {
    ProcessSerialNumber tProcess;
    int nOptionWidth;
    int nOptionHeight;
    int nOptionDepth;
    tProcess.highLongOfPSN = 0;
    tProcess.lowLongOfPSN = kCurrentProcess;
    SetFrontProcess(&tProcess);
    MAC_GetDefaultDisplayMode(&nOptionWidth, &nOptionHeight, &nOptionDepth);
    s_nDisplayModule = nDisplayModule;
    if (MACDISPLAY_SelectDriver(0)) {
        ((void (*)(void*, void*, void*))g_pMacAppProcs->apfn[49])((void*)MACCURSOR_InitCursor, (void*)MACCURSOR_ShowDisplayCursor,
                                                                   (void*)MACCURSOR_HideDisplayCursor);
        int nWidth;
        int nHeight;
        int nBitDepth;
        MAC_GetDefaultDisplayMode(&nWidth, &nHeight, &nBitDepth);
        MacDisplayRequest tRequest;
        tRequest.nWidth = nWidth;
        tRequest.nHeight = nHeight;
        tRequest.nPitch = nWidth;
        tRequest.nBitsPerPixel = 8;
        tRequest.byDepth = (uint8_t)nBitDepth;
        tRequest.byFlags = 2;
        MacDisplayLimits tLimits;
        tLimits.nWidth = nWidth;
        tLimits.nHeight = nHeight;
        tLimits.nPitch = nWidth;
        tLimits.nMaxBitsPerPixel = 0x20;
        tLimits.nDepth = nBitDepth;
        tLimits.nFlags = 0;
        if (((char (*)(void*, void*, int, const char*))g_pMacAppProcs->apfn[13])(&tRequest, &tLimits, s_nDisplayModule,
                                                                                 "") == 1) {
            FPSetMessageBoxCallback(FUN_0019cf50);
            MACCURSOR_RegisterGuardProc((bool (*)(bool))FUN_0019cb40);
            MACCURSOR_RegisterSetCursorPosProc(MAC_SetCursorPos);
            MACCURSOR_RegisterGetCursorPosProc(MAC_GetCursorPos);
            return 1;
        }
    }
    DSpShutdown();
    return 0;
}

// 1.14d mac: 0019cdd3
// Warps the cursor to a game position; false without a renderer.
bool MAC_SetCursorPos(int16_t* pnX, int16_t* pnY) {
    Point ptCursor;
    ptCursor.h = *pnX;
    ptCursor.v = *pnY;
    if (FUN_002b5de2(&ptCursor) != 0) {
        MAC_WarpMouseCursorPosition(ptCursor);
        return true;
    }
    return false;
}

// 1.14d mac: 0019ce1b
// The cursor position in game coordinates; false without a renderer.
bool MAC_GetCursorPos(int* pnX, int* pnY) {
    Point ptCursor;
    MAC_GetGlobalMouse(&ptCursor);
    if (FUN_002b5dc3(&ptCursor) != 0) {
        *pnX = ptCursor.h;
        *pnY = ptCursor.v;
        return true;
    }
    return false;
}

// 1.14d mac: 0019ce5a
void MAC_EnableMenus(void) {
    EnableMenuItem(GetMenuHandle(0x81), 0);
    EnableMenuItem(GetMenuHandle(0x82), 0);
    EnableMenuItem(GetMenuHandle(0x83), 0);
    EnableMenuItem(GetMenuHandle(0x84), 0);
}

// 1.14d mac: 0019ced5
void MAC_DisableMenus(void) {
    DisableMenuItem(GetMenuHandle(0x81), 0);
    DisableMenuItem(GetMenuHandle(0x82), 0);
    DisableMenuItem(GetMenuHandle(0x83), 0);
    DisableMenuItem(GetMenuHandle(0x84), 0);
}

#endif  // D2_OS_MAC
