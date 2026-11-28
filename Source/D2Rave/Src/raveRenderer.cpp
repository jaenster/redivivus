// The D2Rave renderer function table: the RAVE renderer as a D2gfx renderer, typed with the
// shared D2RendererFunctionsStrc the Windows renderers fill (Source/D2gfx/D2gfx.public.h).
//
// Restoration code, not in any binary, after the pattern of Source/D2OpenGL/Src/D2gfxRenderer.cpp.
// The PPC D2gfx selects a 57-slot table for DISPLAYTYPE_RAVE (1.09d: 103f26b0, selector
// 103f3bc0[7]); the shared struct has the 54 Windows 1.14d slots. The mapping, by behaviour:
// - Mac slots 0-10 are Windows slots 0-10;
// - Mac slots 11-13 (RaveProcessEvent, RaveGlobalToLocal, RaveLocalToGlobal) have no Windows
//   slot; they are reached directly (D2Rave.h / raveInternal.h);
// - Mac slots 14-56 are Windows slots 11-53 (14 RaveSetFullScreen = ActivateWindow,
//   15 RaveSetOption = SetOption, and from there each Mac slot is the Windows slot plus 3).
// This agrees with D2OpenGL's mapping of its own 57-slot Mac table. The 1.00 table has 56 slots
// (no DrawClippedImage).
// Each adapter converts only the calling convention and the argument types: the Windows slots
// pass palette colours as DWORDs and rectangles as RECT/POINT, the Mac functions take bytes and
// int arrays with the same layout.

#include "Redivivus/Platform/MacClassic/MacClassicPrelude.h"
#include "Source/D2gfx/D2gfx.public.h"
#include "Source/D2Rave/D2Rave.h"

#if !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC

namespace D2Rave::Renderer {

namespace {

// Win 0 / Mac 0
BOOL __fastcall Initialize(HINSTANCE hInstance) {
    (void)hInstance;
    return RaveDetect();
}

// Win 1 / Mac 1
BOOL __fastcall InitPerspective(D2GfxSettingsStrc * pSettings, ::D2GfxHelperStrc * pHelpers) {
    return RaveInit(pSettings, pHelpers);
}

// Win 2 / Mac 2
BOOL __fastcall Release() {
    return RaveRelease();
}

// Win 3 / Mac 3
BOOL __fastcall CreateRenderWindow(HWND hWnd, int nResolutionMode) {
    return RaveCreateSurface((void *)hWnd, nResolutionMode);
}

// Win 4 / Mac 4
BOOL __stdcall DestroyRenderWindow() {
    return RaveCloseSurface();
}

// Win 5 / Mac 5
void __fastcall EndCutScene(HWND hWnd, int nResolutionMode, int nWindowState) {
    RavePauseSurface((void *)hWnd, nResolutionMode, nWindowState);
}

// Win 6 / Mac 6. RAVE clears in RaveClearScreen; the Mac slot takes no arguments.
BOOL __fastcall BeginScene(BOOL bClear, BYTE nRed, BYTE nGreen, BYTE nBlue) {
    (void)bClear;
    (void)nRed;
    (void)nGreen;
    (void)nBlue;
    return RaveStartDraw();
}

// Win 7 / Mac 7
BOOL __fastcall EndScene1() {
    return RaveEndDraw();
}

// Win 8 / Mac 8
BOOL __fastcall EndScene2() {
    return RaveBlit();
}

// Win 9 / Mac 9
BOOL __fastcall ResizeWindow(HWND hWnd, BOOL nResolutionMode) {
    return RaveChangeRes((int)(intptr_t)hWnd, nResolutionMode);
}

// Win 10 / Mac 10
BOOL __fastcall GetBackBuffer(BYTE * pBuffer) {
    return RaveScreenGrab(pBuffer);
}

// Win 11 / Mac 14
BOOL __fastcall ActivateWindow(BOOL bActive) {
    return RaveSetFullScreen(bActive);
}

// Win 12 / Mac 15
BOOL __fastcall SetOption(int nOption, intptr_t nValue) {
    return RaveSetOption((unsigned int)nOption, nValue);
}

// Win 13 / Mac 16
BOOL __stdcall BeginCutScene() {
    return RaveCutsceneCreateWindow();
}

// Win 14 / Mac 17. The frame callback takes no arguments, so __stdcall and cdecl agree.
void __fastcall PlayCutScene(char * szFile, int nResolutionMode, fnBinkFrameCallback pfnFrame) {
    RavePlayCutscene(szFile, nResolutionMode, reinterpret_cast<void (*)()>(pfnFrame));
}

// Win 15 / Mac 18
BOOL __fastcall CheckCutScene() {
    return RaveCheckCutscene();
}

// Win 16 / Mac 19
void __fastcall DecodeSmacker(char * szSmacker, D2SmackerContextStrc * pContext, int nVersion) {
    RaveDecodeSmacker(szSmacker, pContext, (char)nVersion);
}

// Win 17 / Mac 20
void __fastcall PlayerSmacker(D2SmackerContextStrc * pContext) {
    RavePlaySmacker(pContext);
}

// Win 18 / Mac 21
void __fastcall CloseSmacker(D2SmackerContextStrc * pContext) {
    RaveCloseSmacker(pContext);
}

// Win 19 / Mac 22
int * __fastcall GetRenderStatistics() {
    return RaveGetRenderStatistics();
}

// Win 20 / Mac 23. The Mac slot returns nothing.
int __fastcall GetScreenSize(int * pWidth, int * pHeight) {
    RaveGetScreenSize(pWidth, pHeight);
    return TRUE;
}

// Win 21 / Mac 24
void __fastcall UpdateScaleFactor(int nScaleFactor) {
    RaveUpdateScaleFactor(nScaleFactor);
}

// Win 22 / Mac 25
void __fastcall SetGamma(int nGamma) {
    RaveSetGamma(nGamma);
}

// Win 23 / Mac 26
int __fastcall CheckGamma() {
    return RaveCheckGamma();
}

// Win 24 / Mac 27
void __fastcall SetPerspectiveScale(int nCenterX, int nCenterY) {
    RaveSetPerspectiveScale(nCenterX, nCenterY);
}

// Win 25 / Mac 28
void __fastcall AdjustPerspectivePosition(int nX, int nY, int nBias, int * pXAdjust, int * pYAdjust) {
    RaveAdjustPerspectivePosition(nX, nY, nBias, pXAdjust, pYAdjust);
}

// Win 26 / Mac 29
void __fastcall PerspectiveScalePosition(int nX, int nY, int nBias, int * pX, int * pY, BOOL bOrder) {
    RavePerspectiveScalePosition(nX, nY, nBias, pX, pY, bOrder);
}

// Win 27 / Mac 30
void __fastcall SetDefaultPerspectiveFactor() {
    RaveSetDefaultPerspectiveFactor();
}

// Win 28 / Mac 31
void __fastcall SetPalette(LPPALETTEENTRY pPalette) {
    RaveSetPalette(reinterpret_cast<const uint8_t *>(pPalette));
}

// Win 29 / Mac 32
void __fastcall SetPaletteTable(BYTE ** pTable) {
    RaveSetPaletteTable(pTable);
}

// Win 30 / Mac 33
void __fastcall SetGlobalLight(int nRed, int nGreen, int nBlue) {
    RaveSetGlobalLight((uint8_t)nRed, (uint8_t)nGreen, (uint8_t)nBlue);
}

// Win 31 / Mac 34
void __fastcall DrawGroundTile(D2TileLibraryEntryStrc * pTile, D2GfxLightExStrc * pLight, int nX, int nY,
                               int nWorldX, int nWorldY, BYTE nAlpha, int nScreenPanels, void * pTileData) {
    RaveDrawGroundTile(reinterpret_cast<RaveTileEntry *>(pTile), pLight, nX, nY, nWorldX, nWorldY, nAlpha,
                       nScreenPanels, pTileData);
}

// Win 32 / Mac 35
void __fastcall DrawPerspectiveImage(D2GfxDataStrc * pData, int nX, int nY, DWORD dwGamma, int nDrawMode,
                                     int nScreenMode, BYTE * pPalette) {
    RaveDrawPerspectiveImage(pData, nX, nY, dwGamma, nDrawMode, nScreenMode, pPalette);
}

// Win 33 / Mac 36
BOOL __fastcall DrawImage(D2GfxDataStrc * pData, int nX, int nY, DWORD dwGamma, int nDrawMode, BYTE * pPalette) {
    return RaveDrawImage(pData, nX, nY, dwGamma, nDrawMode, pPalette);
}

// Win 34 / Mac 37
void __fastcall DrawShiftedImage(D2GfxDataStrc * pData, int nX, int nY, DWORD dwGamma, DRAWMODE eDrawMode,
                                 GLOBALPALLETE ePalette) {
    RaveDrawShiftedImage(pData, nX, nY, dwGamma, (int)eDrawMode, (int)ePalette);
}

// Win 35 / Mac 38
void __fastcall DrawVerticalCropImage(D2GfxDataStrc * pData, int nX, int nY, int nSkipLines, int nDrawLines,
                                      int nDrawMode) {
    RaveDrawVerticalCropImage(pData, nX, nY, nSkipLines, nDrawLines, nDrawMode);
}

// Win 36 / Mac 39
void __fastcall DrawShadow(D2GfxDataStrc * pData, int nX, int nY) {
    RaveDrawShadow(pData, nX, nY);
}

// Win 37 / Mac 40
void __fastcall DrawImageFast(D2GfxDataStrc * pData, int nX, int nY, BYTE nPaletteIndex) {
    RaveDrawImageFast(pData, nX, nY, nPaletteIndex);
}

// Win 38 / Mac 41 (not in the 1.00 table)
void __fastcall DrawClippedImage(D2GfxDataStrc * pData, int nX, int nY, void * pCropRect, int nDrawMode) {
    RaveDrawClippedImage(pData, nX, nY, pCropRect, nDrawMode);
}

// Win 39 / Mac 42
BOOL __fastcall DrawWallTile(D2TileLibraryEntryStrc * pTile, int nX, int nY, D2GfxLightStrc * pLight,
                             int nScreenPanels) {
    return RaveDrawWallTile(reinterpret_cast<RaveTileEntry *>(pTile), nX, nY, pLight, nScreenPanels);
}

// Win 40 / Mac 43
BOOL __fastcall DrawTransWallTile(D2TileLibraryEntryStrc * pTile, int nX, int nY, D2GfxLightStrc * pLight,
                                  int nScreenPanels, BYTE nAlpha) {
    return RaveDrawTransWallTile(reinterpret_cast<RaveTileEntry *>(pTile), nX, nY, pLight, nScreenPanels, nAlpha);
}

// Win 41 / Mac 44
BOOL __fastcall DrawShadowTile(D2TileLibraryEntryStrc * pTile, int nX, int nY, int nDrawMode, int nScreenPanels) {
    return RaveDrawShadowTile(reinterpret_cast<RaveTileEntry *>(pTile), nX, nY, nDrawMode, nScreenPanels);
}

// Win 42 / Mac 45. RECT is four ints in the same order on both sides.
void __fastcall DrawRect(RECT * pRect, BYTE nColor) {
    RaveDrawRect(reinterpret_cast<const int *>(pRect), nColor);
}

// Win 43 / Mac 46
void __fastcall DrawRectEx(RECT * pRect, BYTE nColor) {
    RaveDrawRectEx(reinterpret_cast<const int *>(pRect), nColor);
}

// Win 44 / Mac 47
void __fastcall DrawSolidRect(RECT * pRect, BYTE nColor) {
    RaveDrawSolidRect(reinterpret_cast<const int *>(pRect), nColor);
}

// Win 45 / Mac 48
void __fastcall DrawSolidSquare(POINT * pPoint, BYTE nSize, BYTE nColor) {
    RaveDrawSolidSquare(reinterpret_cast<const int *>(pPoint), nSize, nColor);
}

// Win 46 / Mac 49
void __fastcall DrawSolidRectEx(int nX1, int nY1, int nX2, int nY2, BYTE nColor, DRAWMODE eDrawMode) {
    RaveDrawSolidRectEx(nX1, nY1, nX2, nY2, nColor, (int)eDrawMode);
}

// Win 47 / Mac 50. Windows passes the colour as a DWORD and the alpha as a BYTE.
void __fastcall DrawSolidRectAlpha(int nX1, int nY1, int nX2, int nY2, DWORD dwColor, BYTE nAlpha) {
    RaveDrawSolidRectAlpha(nX1, nY1, nX2, nY2, (uint8_t)dwColor, nAlpha);
}

// Win 48 / Mac 51
void __fastcall DrawLine(int nX1, int nY1, int nX2, int nY2, BYTE dwColor, BYTE nAlpha) {
    RaveDrawLine(nX1, nY1, nX2, nY2, (uint8_t)dwColor, nAlpha);
}

// Win 49 / Mac 52
void __fastcall ClearScreen(BOOL bClear) {
    RaveClearScreen(bClear);
}

// Win 50 / Mac 53
void __fastcall DrawString(int nX, int nY, char * szFormat, va_list args) {
    RaveDrawString(nX, nY, szFormat, args);
}

// Win 51 / Mac 54
void __fastcall DrawLight(DWORD * pLight, DWORD * pColor, int nX, int nY) {
    RaveDrawLight(reinterpret_cast<uint32_t *>(pLight), reinterpret_cast<uint32_t *>(pColor), nX, nY);
}

// Win 52 / Mac 55
void __fastcall DebugFillBackBuffer(int nX, int nY) {
    RaveDebugFillBackBuffer(nX, nY);
}

// Win 53 / Mac 56
void __fastcall ClearCaches() {
    RaveClearCaches();
}

} // namespace

D2RendererFunctionsStrc RaveFunctionTable = {
    &Initialize,
    &InitPerspective,
    &Release,
    &CreateRenderWindow,
    &DestroyRenderWindow,
    &EndCutScene,
    &BeginScene,
    &EndScene1,
    &EndScene2,
    &ResizeWindow,
    &GetBackBuffer,
    &ActivateWindow,
    &SetOption,
    &BeginCutScene,
    &PlayCutScene,
    &CheckCutScene,
    &DecodeSmacker,
    &PlayerSmacker,
    &CloseSmacker,
    &GetRenderStatistics,
    &GetScreenSize,
    &UpdateScaleFactor,
    &SetGamma,
    &CheckGamma,
    &SetPerspectiveScale,
    &AdjustPerspectivePosition,
    &PerspectiveScalePosition,
    &SetDefaultPerspectiveFactor,
    &SetPalette,
    &SetPaletteTable,
    &SetGlobalLight,
    &DrawGroundTile,
    &DrawPerspectiveImage,
    &DrawImage,
    &DrawShiftedImage,
    &DrawVerticalCropImage,
    &DrawShadow,
    &DrawImageFast,
    &DrawClippedImage,
    &DrawWallTile,
    &DrawTransWallTile,
    &DrawShadowTile,
    &DrawRect,
    &DrawRectEx,
    &DrawSolidRect,
    &DrawSolidSquare,
    &DrawSolidRectEx,
    &DrawSolidRectAlpha,
    &DrawLine,
    &ClearScreen,
    &DrawString,
    &DrawLight,
    &DebugFillBackBuffer,
    &ClearCaches,
};

} // namespace D2Rave::Renderer

#endif // !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC
