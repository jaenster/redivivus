// uniCore.cpp - D2Uni's slot layer: the renderer function table and the slots that pick a back
// end. Every slot from 0 to 18 and from 22 to 33 (Mac numbering) asks gnUniMode which back end
// is up - 1 DrawSprocket (full screen), 2 QuickDraw (windowed) - and calls it; SetOption(8)
// chooses. That is the choice Windows makes between two DLLs: D2DDraw for full screen, D2Gdi for
// a window. The drawing slots need no dispatch (one 8-bit back buffer serves both) and live in
// uniEngine.cpp, uniSmack.cpp and uniSprite.cpp.
//
// File name inferred (Source/D2Uni/README.md). Read last to first, the object's functions run in
// the order of D2Gdi gdiCore.cpp's slot layer, with small local moves. MWCC emits an object's
// functions last to first, so the functions below are in source order, highest address first.
//
// The Carbon builds add a third back end, CoreGraphics (gnUniMode 3, cgCore.cpp), and a static
// initializer per object (__sinit_uniCore: the Gestalt 'sysv'/'apvr' test of a header static).
// This file is the classic LoD 1.10b object; the Carbon dispatch is not reconstructed here.
//
// Classic Mac PowerPC only.
#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

#include "Source/D2Uni/Src/uniInternal.h"

int gnUniMode;                                  // 1055e738
D2GfxSettingsStrc * gpUniSettings;              // 1055e600
D2GfxHelperStrc * gpUniHelpers;                 // 1055e608
BYTE * gpUniBackBuffer;                         // 1055e604
int gnUniBackBufferWidth;                       // 1055e734
int gnUniBackBufferHeight;                      // 1055e730
int gnUniBackBufferDepth;                       // 1055e72c
int gnUniScreenWidth;                           // 1055e5c8
int gnUniScreenHeight;                          // 1055e5c4
BOOL gbUniCutsceneQuit;                         // 1055e5cc
BOOL gbUniSurfaceReleased;                      // 1055e5d0
D2MacDisplayModeStrc gaUniDisplayModes[2];      // 1055e5d4
CTabHandle ghUniColorTable;                     // 1055e5f4
GDHandle ghUniDevice;                           // 1055e5f8
BOOL gbUniInScene;                              // 1055e5fc
BYTE * gapUniPaletteTables[72];                 // 1055e60c
int gnUniCutsceneDepth = 32;                    // 104422a4
BOOL gbUniAppActive = 1;                        // 104422a8
int gnUniGamma = 0x9B;                          // 104422ac
int gnUniContrast = 100;                        // 104422b0

// ppc 1.10b: 10059080 (1.00 10088564, 1.13d 1005f0e0, carbon 1.10b 103a85c0, carbon 1.13d 103d7d40)
// Mac slot 14 / Windows 11 ActivateWindow. The back end is chosen by SetOption(8), not here.
BOOL __fastcall UniSetFullScreen(BOOL bFullScreen) {
    return 0;
}

// ppc 1.10b: 10059010 (1.00 100884f4, 1.13d 1005f080, carbon 1.10b 103a8510, carbon 1.13d 103d7ca0)
// Mac slot 0 / Windows 0 Initialize (D2gfx's "GFXDetect"). Both back ends must be available.
BOOL __fastcall UniDetect(HINSTANCE hInstance) {
    LogPrintf("UniDetect: Initializing software video drivers.");
    return DSpDetect() && QDDetect();
}

// ppc 1.10b: 10058e70 (1.00 10088140, 1.13d 1005eee0, carbon 1.10b 103a8350, carbon 1.13d 103d7ae0)
// Mac slot 1 / Windows 1 InitPerspective (D2gfx's "GFXInit"). Builds the shared 256-entry colour
// table once (value i at entry i, all black), then initializes the chosen back end.
BOOL __fastcall UniInit(D2GfxSettingsStrc * pSettings, D2GfxHelperStrc * pHelpers) {
    if (!ghUniColorTable) {
        ghUniColorTable = (CTabHandle)NewHandleClear(0x808);
        if (!ghUniColorTable) {
            LogPrintf("UniDetect: *** NewHandleClear failed.");
            return 0;
        }
        HLockHi((Handle)ghUniColorTable);
        (*ghUniColorTable)->ctSeed = GetCTSeed();
        (*ghUniColorTable)->ctFlags = 0xC000;
        (*ghUniColorTable)->ctSize = 255;
        for (int16_t i = 0; i < 256; i++) {
            (*ghUniColorTable)->ctTable[i].value = i;
        }
    }

    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpInit(pSettings, pHelpers);
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDInit(pSettings, pHelpers);
    }
    return 0;
}

// ppc 1.10b: 10058df0 (1.00 100880c8, 1.13d 1005ee60, carbon 1.10b 103a82c0, carbon 1.13d 103d7a50)
// Mac slot 2 / Windows 2 Release (D2gfx's "GFXClose").
BOOL __fastcall UniRelease() {
    if (ghUniColorTable) {
        DisposeHandle((Handle)ghUniColorTable);
        ghUniColorTable = nullptr;
    }
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpRelease();
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDClose();
    }
    return 0;
}

// ppc 1.10b: 10058d90 (1.00 1008806c, 1.13d 1005ee00, carbon 1.10b 103a8250, carbon 1.13d 103d79e0)
// Mac slot 4 / Windows 4 DestroyWindow (D2gfx's "GFXCloseSurface"). Ends any movie first.
BOOL __stdcall UniCloseSurface() {
    gbUniCutsceneQuit = 1;
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpCloseSurface();
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDCloseSurface();
    }
    return 0;
}

// ppc 1.10b: 10058d20 (1.00 10088010, 1.13d 1005ed90, carbon 1.10b 103a81d0, carbon 1.13d 103d7960)
// Mac slot 9 / Windows 9 ResizeWindow. Resolution 2 is treated as 1.
BOOL __fastcall UniChangeRes(HWND hWnd, BOOL nResolution) {
    if (nResolution == 2) {
        nResolution = 1;
    }
    gbUniCutsceneQuit = 1;
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpChangeRes(hWnd, nResolution);
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDChangeRes(hWnd, nResolution);
    }
    return 0;
}

// ppc 1.10b: 10058d10 (1.00 1008800c, 1.13d 1005ed80, carbon 1.10b 103a81c0, carbon 1.13d 103d7950)
// Mac slot 56 / Windows 53. Nothing is cached.
void __fastcall UniClearCaches() {
}

// ppc 1.10b: 10058bf0 (1.00 10087ef8, 1.13d 1005ec60, carbon 1.10b 103a8060, carbon 1.13d 103d77f0)
// Mac slot 15 / Windows 12. A 12-way switch (1.10b jump table 104422b4). On the Mac options 1, 2,
// 3 and 10 pass a pointer in nValue.
BOOL __fastcall UniSetOption(int nOption, int nValue) {
    switch (nOption) {
        case 0:
            // The device to run on.
            ghUniDevice = (GDHandle)(intptr_t)nValue;
            QDSetGDevice();
            DSpSetGDevice();
            return 1;
        case 1:
            // The device in use: DrawSprocket's is the one set, QuickDraw's the one under the window.
            if (gnUniMode == UNIMODE_DRAWSPROCKET) {
                *(GDHandle *)(intptr_t)nValue = ghUniDevice;
                return 1;
            }
            if (gnUniMode == UNIMODE_QUICKDRAW) {
                *(GDHandle *)(intptr_t)nValue = QDGetGDevice();
                return 1;
            }
            return 0;
        case 2:
            gaUniDisplayModes[0] = *(const D2MacDisplayModeStrc *)(intptr_t)nValue;
            return 1;
        case 3:
            gaUniDisplayModes[1] = *(const D2MacDisplayModeStrc *)(intptr_t)nValue;
            return 1;
        case 8:
            // Windowed (non-zero low byte) runs QuickDraw, full screen runs DrawSprocket.
            gnUniMode = (BYTE)nValue ? UNIMODE_QUICKDRAW : UNIMODE_DRAWSPROCKET;
            return 1;
        case 10:
            *(BOOL *)(intptr_t)nValue = gbUniInScene;
            return 1;
        case 11:
            // The contrast for the gamma curve (D2DDraw stores gnDirectDrawPaletteSize here).
            gnUniContrast = nValue;
            return 1;
    }
    return 0;
}

// ppc 1.10b: 10058b90 (1.00 10087ea8, 1.13d 1005ec00, carbon 1.10b 103a7ff0, carbon 1.13d 103d7780)
// Mac slot 3 / Windows 3 CreateWindow (D2gfx's "GFXCreateSurface"). Resolution 2 is treated as 1.
BOOL __fastcall UniCreateSurface(HWND hWnd, int nResolution) {
    if (nResolution == 2) {
        nResolution = 1;
    }
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpCreateSurface(hWnd, nResolution);
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDCreateSurfaceEntry(hWnd, nResolution);
    }
    return 0;
}

// ppc 1.10b: 10058af0 (1.00 10087e1c, 1.13d 1005eb60, carbon 1.10b 103a7f30, carbon 1.13d 103d76c0)
// Mac slot 5 / Windows 5.
void __fastcall UniEndCutScene(HWND hWnd, int nResolution, int bRelease) {
    if (nResolution == 2) {
        nResolution = 1;
    }
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        DSpEndCutScene(hWnd, nResolution, bRelease);
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        QDEndCutScene(hWnd, nResolution, bRelease);
    }
}

// ppc 1.10b: 10058aa0 (1.00 10087dcc, 1.13d 1005eb10, carbon 1.10b 103a7ec0, carbon 1.13d 103d7650)
// Mac slot 16 / Windows 13 BeginCutScene.
BOOL __stdcall UniCutsceneCreateWindow() {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpCutsceneCreateWindow();
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDCutsceneCreateWindow();
    }
    return 0;
}

// ppc 1.10b: 10058a50 (1.00 10087d84, 1.13d 1005eac0, carbon 1.10b 103a7e60, carbon 1.13d 103d75f0)
// Mac slot 17 / Windows 14.
void __fastcall UniPlayCutscene(char * szFile, int nResolution, fnBinkFrameCallback pfnFrame) {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        DSpPlayCutsceneGuarded(szFile, nResolution, pfnFrame);
    } else if (gnUniMode == UNIMODE_QUICKDRAW) {
        QDPlayCutscene(szFile, nResolution, pfnFrame);
    }
}

// ppc 1.10b: 10058a00 (1.00 10087d34, 1.13d 1005ea70, carbon 1.10b 103a7df0, carbon 1.13d 103d7580)
// Mac slot 18 / Windows 15.
BOOL __fastcall UniCheckCutscene() {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpCheckCutscene();
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDCheckCutscene();
    }
    return 0;
}

// ppc 1.10b: 100589b0 (1.00 10087ce4, 1.13d 1005ea20, carbon 1.10b 103a7d80, carbon 1.13d 103d7510)
// Mac slot 6 / Windows 6 BeginScene. The clear colour is ignored: the clear is to index 0.
BOOL __fastcall UniStartDraw(BOOL bClear, BYTE nRed, BYTE nGreen, BYTE nBlue) {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpStartDraw(bClear);
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDStartDraw(bClear);
    }
    return 0;
}

// ppc 1.10b: 10058960 (1.00 10087c94, 1.13d 1005e9d0, carbon 1.10b 103a7d10, carbon 1.13d 103d74a0)
// Mac slot 7 / Windows 7 EndScene1.
BOOL __fastcall UniEndDraw() {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpEndDraw();
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDEndDraw();
    }
    return 0;
}

// ppc 1.10b: 10058910 (1.00 10087c44, 1.13d 1005e980, carbon 1.10b 103a7ca0, carbon 1.13d 103d7430)
// Mac slot 8 / Windows 8: present the frame.
BOOL __fastcall UniEndScene2() {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpBlit();
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDBlit();
    }
    return 0;
}

// ppc 1.10b: 100588c0 (1.00 10087bf4, 1.13d 1005e930, carbon 1.10b 103a7c30, carbon 1.13d 103d73c0)
// Presents the changed part of a Bink frame (UniBinkRenderFrame).
BOOL UniBinkBlit(Rect * pRect) {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpBinkBlit(pRect);
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDBinkBlit(pRect);
    }
    return 0;
}

// ppc 1.10b: 10058730 (1.00 10087ba4, 1.13d 1005e7a0, carbon 1.10b 103a7a80, carbon 1.13d 103d7210)
// Mac slot 10 / Windows 10: the screenshot. Writes the back buffer as 24-bit RGB rows, through
// the colour table at 8 bits or from the pixels at 32. At 32 bits the row skip adds
// (pitch - width) bytes, not pixels, as the binary does.
BOOL __fastcall UniGetBackBuffer(BYTE * pBuffer) {
    uint32_t nWidth;
    uint32_t nHeight;

    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        DSpGetScreenSize((int *)&nWidth, (int *)&nHeight);
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        QDGetScreenSize((int *)&nWidth, (int *)&nHeight);
    }

    if (gnUniBackBufferDepth == 8) {
        const BYTE * pSrc = gpUniBackBuffer;
        const ColorSpec * pTable = (*ghUniColorTable)->ctTable;
        for (uint32_t nRow = 0; nRow < nHeight; nRow++) {
            for (uint32_t nCol = 0; nCol < nWidth; nCol++) {
                const RGBColor & tColor = pTable[*pSrc++].rgb;
                pBuffer[0] = (BYTE)(tColor.red >> 8);
                pBuffer[1] = (BYTE)(tColor.green >> 8);
                pBuffer[2] = (BYTE)(tColor.blue >> 8);
                pBuffer += 3;
            }
            pSrc += gnUniBackBufferWidth - nWidth;
        }
        return 1;
    }

    if (gnUniBackBufferDepth == 32) {
        const uint32_t * pSrc = (const uint32_t *)gpUniBackBuffer;
        for (uint32_t nRow = 0; nRow < nHeight; nRow++) {
            for (uint32_t nCol = 0; nCol < nWidth; nCol++) {
                uint32_t nPixel = *pSrc++;
                pBuffer[0] = (BYTE)(nPixel >> 16);
                pBuffer[1] = (BYTE)(nPixel >> 8);
                pBuffer[2] = (BYTE)nPixel;
                pBuffer += 3;
            }
            pSrc = (const uint32_t *)((const BYTE *)pSrc + (gnUniBackBufferWidth - nWidth));
        }
        return 1;
    }

    return 0;
}

// ppc 1.10b: 100586c0 (1.00 10087b2c, 1.13d 1005e730, carbon 1.10b 103a79f0, carbon 1.13d 103d7180)
// Mac slot 23 / Windows 20. The PEF function returns nothing (r3 is left as it is); the table
// slot's int return is 0 here.
int __fastcall UniGetScreenSize(int * pWidth, int * pHeight) {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        DSpGetScreenSize(pWidth, pHeight);
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        QDGetScreenSize(pWidth, pHeight);
    }
    return 0;
}

// ppc 1.10b: 10058660 (1.00 10087ac8, 1.13d 1005e6d0, carbon 1.10b 103a7970, carbon 1.13d 103d7100)
// Mac slot 31 / Windows 28.
void __fastcall UniSetPalette(LPPALETTEENTRY pPalette) {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        DSpSetPalette(pPalette);
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        QDSetPalette(pPalette);
    }
}

// ppc 1.10b: 10058600 (1.00 10087a64, 1.13d 1005e670, carbon 1.10b 103a78f0, carbon 1.13d 103d7080)
// Mac slot 32 / Windows 29.
void __fastcall UniSetPaletteTable(BYTE ** ppTables) {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        DSpSetPaletteTable(ppTables);
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        QDSetPaletteTable(ppTables);
    }
}

// ppc 1.10b: 100585a0 (1.00 10087a00, 1.13d 1005e610, carbon 1.10b 103a7870, carbon 1.13d 103d7000)
// Mac slot 24 / Windows 21.
void __fastcall UniUpdateScaleFactor(int nScale) {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        DSpUpdateScaleFactor();
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        QDUpdateScaleFactor(nScale);
    }
}

// ppc 1.10b: 10058540 (1.00 1008799c, 1.13d 1005e5b0, carbon 1.10b 103a77f0, carbon 1.13d 103d6f80)
// Mac slot 25 / Windows 22.
void __fastcall UniSetGamma(int nGamma) {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        DSpSetGamma(nGamma);
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        QDSetGamma(nGamma);
    }
}

// ppc 1.10b: 100584f0 (1.00 1008794c, 1.13d 1005e560, carbon 1.10b 103a7780, carbon 1.13d 103d6f10)
// Mac slot 26 / Windows 23.
int __fastcall UniCheckGamma() {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpCheckGamma();
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDCheckGamma();
    }
    return 0;
}

// ppc 1.10b: 10058470 (1.00 100878c0, 1.13d 1005e4e0, carbon 1.10b 103a76e0, carbon 1.13d 103d6e70)
// Mac slot 33 / Windows 30.
void __fastcall UniSetGlobalLight(int nRed, int nGreen, int nBlue) {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        DSpSetGlobalLight();
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        QDSetGlobalLight(nRed, nGreen, nBlue);
    }
}

// ppc 1.10b: 10058400 (1.00 10087848, 1.13d 1005e470, carbon 1.10b 103a7650, carbon 1.13d 103d6de0)
// Mac slot 27 / Windows 24.
void __fastcall UniSetPerspectiveScale(int nScaleX, int nScaleY) {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        DSpSetPerspectiveScale();
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        QDSetPerspectiveScale(nScaleX, nScaleY);
    }
}

// ppc 1.10b: 10058380 (1.00 100877bc, 1.13d 1005e3f0, carbon 1.10b 103a75b0, carbon 1.13d 103d6d40)
// Mac slot 28 / Windows 25.
void __fastcall UniAdjustPerspectivePosition(int nPosX, int nPosY, int nBias, int * pXAdjust, int * pYAdjust) {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        DSpAdjustPerspectivePosition();
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        QDAdjustPerspectivePosition(nPosX, nPosY, nBias, pXAdjust, pYAdjust);
    }
}

// ppc 1.10b: 10058300 (1.00 10087724, 1.13d 1005e370, carbon 1.10b 103a7500, carbon 1.13d 103d6c90)
// Mac slot 29 / Windows 26.
void __fastcall UniPerspectiveScalePosition(int nPosX, int nPosY, int nAngle, int * pXAdjust, int * pYAdjust,
                                            BOOL bOrder) {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        DSpPerspectiveScalePosition();
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        QDPerspectiveScalePosition(nPosX, nPosY, nAngle, pXAdjust, pYAdjust, bOrder);
    }
}

// ppc 1.10b: 100582b0 (1.00 100876d4, 1.13d 1005e320, carbon 1.10b 103a7490, carbon 1.13d 103d6c20)
// Mac slot 30 / Windows 27.
void __fastcall UniSetDefaultPerspectiveFactor() {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        DSpSetDefaultPerspectiveFactor();
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        QDSetDefaultPerspectiveFactor();
    }
}

// ppc 1.10b: 10058260 (1.00 10087684, 1.13d 1005e2d0, carbon 1.10b 103a7420, carbon 1.13d 103d6bb0)
// Mac slot 22 / Windows 19.
int * __fastcall UniGetRenderStatistics() {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpGetRenderStatistics();
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDGetRenderStatistics();
    }
    return nullptr;
}

// ppc 1.10b: 10058210 (1.00 10087634, 1.13d 1005e280, carbon 1.10b 103a73b0, carbon 1.13d 103d6b40)
// Mac slot 11 (no Windows twin).
unsigned char UniProcessEvent(EventRecord * pEvent) {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpProcessEvent(pEvent);
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDProcessEvent(pEvent);
    }
    return 0;
}

// ppc 1.10b: 100581c0 (1.00 100875e4, 1.13d 1005e230, carbon 1.10b 103a7340, carbon 1.13d 103d6ad0)
// Mac slot 12 (no Windows twin).
int UniGlobalToLocal(Point * pPoint) {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpGlobalToLocal(pPoint);
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDGlobalToLocal(pPoint);
    }
    return 0;
}

// ppc 1.10b: 10058170 (1.00 10087594, 1.13d 1005e1e0, carbon 1.10b 103a72d0, carbon 1.13d 103d6a60)
// Mac slot 13 (no Windows twin).
int UniLocalToGlobal(Point * pPoint) {
    if (gnUniMode == UNIMODE_DRAWSPROCKET) {
        return DSpLocalToGlobal(pPoint);
    }
    if (gnUniMode == UNIMODE_QUICKDRAW) {
        return QDLocalToGlobal(pPoint);
    }
    return 0;
}

// The 54 slots the Mac table shares with Windows, in the shared D2RendererFunctionsStrc order.
// In the shipped 57-slot table (1.10b 104421c0) UniMacFunctionTable's three sit between
// GetBackBuffer and ActivateWindow.
D2RendererFunctionsStrc UniFunctionTable = {
    UniDetect,
    UniInit,
    UniRelease,
    UniCreateSurface,
    UniCloseSurface,
    UniEndCutScene,
    UniStartDraw,
    UniEndDraw,
    UniEndScene2,
    UniChangeRes,
    UniGetBackBuffer,
    UniSetFullScreen,
    UniSetOption,
    UniCutsceneCreateWindow,
    UniPlayCutscene,
    UniCheckCutscene,
    UniDecodeSmacker,
    UniPlayerSmacker,
    UniCloseSmacker,
    UniGetRenderStatistics,
    UniGetScreenSize,
    UniUpdateScaleFactor,
    UniSetGamma,
    UniCheckGamma,
    UniSetPerspectiveScale,
    UniAdjustPerspectivePosition,
    UniPerspectiveScalePosition,
    UniSetDefaultPerspectiveFactor,
    UniSetPalette,
    UniSetPaletteTable,
    UniSetGlobalLight,
    UniDrawGroundTile,
    UniDrawPerspectiveImage,
    UniDrawImage,
    UniDrawShiftedImage,
    UniDrawVerticalCropImage,
    UniDrawShadow,
    UniDrawImageFast,
    UniDrawClippedImage,
    UniDrawWallTile,
    UniDrawTransWallTile,
    UniDrawShadowTile,
    UniDrawRect,
    UniDrawRectEx,
    UniDrawSolidRect,
    UniDrawSolidSquare,
    UniDrawSolidRectEx,
    UniDrawSolidRectAlpha,
    UniDrawLine,
    UniClearScreen,
    UniDrawString,
    UniDrawLight,
    UniDebugFillBackBuffer,
    UniClearCaches,
};

D2MacRendererSlotsStrc UniMacFunctionTable = {
    UniProcessEvent,
    UniGlobalToLocal,
    UniLocalToGlobal,
};

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
