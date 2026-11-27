// D2GFXMacRenderer.cpp - the renderer switch and the state it keeps.
//
// 1.14d Intel Mac (DiabloII_macho). No __FILE__ names the unit: the Mac renderer's helper
// table (D2GfxHelperStrc) slots and the tile code sit among D2gfx's functions, and which
// translation unit each belongs to follows from direct versus indirect (PIC pointer cell)
// access to the globals alone. See Source/D2gfx/Mac/README.md.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC`; cmake/mac.cmake globs
// this folder.
#include "D2GFXMac.h"

#if !defined(_WIN32) && D2_OS_MAC

void fp_fatal(const char* szFormat, ...);
// 1.14d mac: 002eff85
void LOGFILE_WriteStringToFile(const char* szFormat, ...);

// 1.14d mac: 002b19a2
void D2GFX_InitializeWindowSize(void);
// 1.14d mac: 002b5170
void D2GFX_SetPalette(void* pPalette);
void FUN_002b1983(void);

// 1.14d mac: 005cbb80
MacRendererFunctions* RENDERER_CurrentRenderedFunctions;
// 1.14d mac: 005cbb84
int g_nRendererInitOption;
// 1.14d mac: 005cbb8c
int eRendererMode;
// 1.14d mac: 005cbb90
int g_bMacFullScreen;
// 1.14d mac: 005cbb94
int nResolutionMode;
// 1.14d mac: 005cbb98
int g_nMacWindow;
// 1.14d mac: 003c8144
int D2GFXRenderedData[16];
// 1.14d mac: 003c8148
int g_bMacHardware;
// 1.14d mac: 003c815c
extern void* g_apMacHelperTable[];
// 1.14d mac: 003c817c
extern MacRendererFunctions* g_apMacRenderers[];

// 1.14d mac: 002b4b72
// Step 2 creates the window; step 1 releases the renderer and loads the one of the given mode.
// Either returns what the renderer's last call returned.
// The binary has it in D2GFX.cpp's unit: it lies between D2GFX_Initialize (002b4975) and
// D2GFX_Release (002b4d0c) and reads that unit's renderer selector (003c817c) and helper
// table (003c815c) directly, where this file can only reach them through pointer cells.
int D2GFX_SwitchModule(int eMode, int bFullScreen, int nStep) {
    const char* szFormat;
    BOOL bResult;
    if (nStep == 2) {
        bResult = RENDERER_CurrentRenderedFunctions->pfnCreateWindow(g_nMacWindow, nResolutionMode);
        if (bResult != 0) {
            D2GFX_InitializeWindowSize();
            D2GFX_SetPalette(0);
            FUN_002b1983();
            return bResult;
        }
        szFormat = "D2GfxSwitchModule: *** GFXCreateSurface failed.";
    } else if (nStep == 1) {
        if (RENDERER_CurrentRenderedFunctions->pfnDestroyWindow() == 0) {
            szFormat = "D2GfxSwitchModule: *** GFXCloseSurface failed.";
        } else if (RENDERER_CurrentRenderedFunctions->pfnRelease() == 0) {
            szFormat = "D2GfxSwitchModule: *** GFXClose failed.";
        } else {
            RENDERER_CurrentRenderedFunctions = 0;
            MacRendererFunctions* pRenderer = g_apMacRenderers[eMode];
            if (pRenderer == 0) {
                LOGFILE_WriteStringToFile("Unsupported video mode - %d", eMode);
                szFormat = "D2GfxSwitchModule: *** sLoadGfxDLLFromType failed.";
            } else {
                RENDERER_CurrentRenderedFunctions = pRenderer;
                if (pRenderer->pfnInitialize(g_nRendererInitOption) == 0) {
                    szFormat = "D2GfxSwitchModule: *** GFXDetect failed.";
                } else {
                    g_bMacFullScreen = bFullScreen;
                    eRendererMode = eMode;
                    RENDERER_CurrentRenderedFunctions->pfnCheckCutScene(8, bFullScreen == 1);
                    int bHardware = eMode > 3;
                    if (eRendererMode <= 3) {
                        bHardware = 0;
                    }
                    g_bMacHardware = bHardware;
                    bResult = RENDERER_CurrentRenderedFunctions->pfnInitPerspective(D2GFXRenderedData, g_apMacHelperTable);
                    if (bResult != 0) {
                        return bResult;
                    }
                    szFormat = "D2GfxSwitchModule: *** GFXInit failed.";
                }
            }
        }
    } else {
        fp_fatal("D2GfxSwitchModule was given an unknown step!");
        return 0;
    }
    LOGFILE_WriteStringToFile(szFormat);
    return 0;
}

// 1.14d mac: 002b4d60
// What D2GFX_SwitchModule stored from its second argument.
int FUN_002b4d60(void) {
    return g_bMacFullScreen;
}

#endif
