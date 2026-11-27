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

namespace D2gfx::D2GFX {
// 1.14d mac: 002b19a2
void D2GFX_InitializeWindowSize(void);
}
using D2gfx::D2GFX::D2GFX_InitializeWindowSize;
struct tagPALETTEENTRY;
namespace D2gfx::D2GFX {
// 1.14d mac: 002b5170
void D2GFX_SetPalette(tagPALETTEENTRY* pPalette);
}
using D2gfx::D2GFX::D2GFX_SetPalette;
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
// 1.14d mac: 005cbb9c
int g_nMacScreenSide;
// 1.14d mac: 003c8144
// Defined by the shared D2GFX.cpp (initialised there).
extern int D2GFXRenderedData[];
// 1.14d mac: 003c8148
int g_bMacHardware;
namespace D2OpenGL {
namespace Renderer {
extern void* OpenGLFunctionTable[57];
}
}

// The helper table's slots (D2GFXMacHelper.cpp and D2GFXMacTiles.cpp).
void fpFillYBufferTable(void* pBits, int nPitch, int nHeight, int n0C);
void fpDrawVisTile(const uint8_t* pSrc, int nX, int nY);
void FUN_002b38f3(const uint8_t* pSrc, int nX, int nY, uint8_t nLight);
void fpDrawGroundTile(const MacGroundTile* pTile, int nScreenX, int nScreenY, const uint8_t* pLights);
void fpDrawWallTile(const uint8_t* pSrc, int nX, int nY, const uint32_t* pCornerLights);
void fpDrawBlendedVisTile(const uint8_t* pSrc, int nX, int nY, uint8_t nAlpha);
void fpDrawRoofTile(const uint8_t* pSrc, int nX, int nY, const int* pCornerLights, uint8_t nAlpha);

// 1.14d mac: 003c815c
// What the renderer's InitPerspective takes: the surface, tile and palette slots, then a flag.
void* g_apMacHelperTable[8] = {(void*)fpFillYBufferTable,
                               (void*)fpDrawVisTile,
                               (void*)FUN_002b38f3,
                               (void*)fpDrawGroundTile,
                               (void*)fpDrawWallTile,
                               (void*)fpDrawBlendedVisTile,
                               (void*)fpDrawRoofTile,
                               (void*)1};
// 1.14d mac: 003c817c
// Renderer function tables by renderer mode; mode 5 is the OpenGL one (003ce6ec).
MacRendererFunctions* g_apMacRenderers[8] = {0, 0, 0, 0, 0, (MacRendererFunctions*)&D2OpenGL::Renderer::OpenGLFunctionTable, 0, 0};

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
                LOGFILE_WriteStringToFile((char*)"Unsupported video mode - %d", eMode);
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
    LOGFILE_WriteStringToFile((char*)szFormat);
    return 0;
}

// 1.14d mac: 002b4d60
// What D2GFX_SwitchModule stored from its second argument.
int FUN_002b4d60(void) {
    return g_bMacFullScreen;
}

#endif
