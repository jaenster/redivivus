// GameSetup.cpp - the Mac shell's game set-up: start-up checks, the CD-key and display
// dialogs, the display-mode switch between windowed and full screen.
//
// 1.14d Intel Mac (DiabloII_macho). Pinned by the 1.14d __FILE__
// ".../Diablo2/Source/DiabloAll/../MacSpecific/Source/Mac Shell/GameSetup.cpp" (0031a4da),
// passed to SMemAlloc/SMemFree by GAMESETUP_Alloc (line 463) and GAMESETUP_Free (line 486),
// and by the video-mode globals 003b8d7c and 00536c50, which the functions below address
// directly (a global another unit defines would go through a non-lazy pointer). Only the
// run 0019c760..0019cc81 and 0019cf50 are reconstructed so far; the file starts earlier (GAMESETUP_Alloc
// is line 463) and where it ends is not settled.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC` (see
// Source/MacSpecific/README.md).
#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <pthread.h>
#include <stdlib.h>

#include "StormMac/H/stpl.h"

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
void D2WINSMACK_SetKeepVideoFile(void);

// The display flags D2gfx and the shell share (defined in another unit; only the bytes
// used here are laid out).
struct MacDisplayState {
    uint8_t unk00[8];
    bool bFullScreen;  // 0x08
    uint8_t unk09;
    uint8_t b0A;  // 0x0A..0x0D reset on every switch
    uint8_t b0B;
    uint8_t b0C;
    uint8_t b0D;
};
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

// 1.14d mac: 0019c760
// The allocator handed to the set-up code: never NULL, -1 on failure.
void* GAMESETUP_Alloc(size_t nSize) {
    void* pMemory = SMemAlloc(nSize, __FILE__, 463, 0);
    return pMemory ? pMemory : (void*)-1;
}

// 1.14d mac: 0019c79f
void GAMESETUP_Free(void* pMemory) {
    if (pMemory)
        SMemFree(pMemory, __FILE__, 486, 0);
}

// 1.14d mac: 0019c7d9
int FUN_0019c7d9(void) {
    return 1;
}

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
    D2WINSMACK_SetKeepVideoFile();
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

#endif  // D2_OS_MAC
