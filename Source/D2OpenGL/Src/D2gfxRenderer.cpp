// The D2OpenGL renderer function table for D2gfx renderer mode 5.
//
// D2gfx calls its renderer slots with the Windows 1.14d types and conventions
// (D2RendererFunctionsStrc, mostly __fastcall); the D2OpenGL functions are the Mac
// 1.14d code, cdecl, with the Mac's types. Each adapter below converts only that:
// - the calling convention;
// - the return type, where one side returns a value and the other nothing
//   (an adapter that must return one returns TRUE);
// - the colour argument of the solid-fill and line slots: a palette index in a
//   DWORD on Windows, a BYTE on the Mac;
// - pointer types that are untyped or differently typed on one side.
// The Mac table has 57 slots: Windows slots 0-10 are Mac slots 0-10, Windows
// 11-53 are Mac 14-56. Mac slots 11-13 have no Windows dispatcher.
// The table is restoration code: 1.14d Windows ships an empty mode-5 slot.

#include "Source/D2OpenGL/Src/D2gfxRenderer.h"
#include "Source/D2OpenGL/Src/oglCore.h"
#include "Source/D2OpenGL/Src/BinkCutscene.h"
#include "Source/D2OpenGL/Src/oglBlocks.h"
#include "Source/D2OpenGL/Src/oglDebugFont.h"
#include "Source/D2OpenGL/Src/oglEngine.h"
#include "Source/D2OpenGL/Src/oglFloorCache.h"
#include "Source/D2OpenGL/Src/oglMac.h"
#include "Source/D2OpenGL/Src/oglPerspective.h"
#include "Source/D2OpenGL/Src/oglSprite.h"
#include "Source/D2OpenGL/Src/oglVertex.h"
#include "Source/D2OpenGL/Src/MacDisplayDevice.h"
#include "Source/D2gfx/Src/D2GFX.h"
#include "Source/D2OpenGL/Src/oglSmack.h"
#include "Source/D2OpenGL/Src/MacSdkOnWin32.h"

namespace D2OpenGL::Renderer {

namespace {

using namespace D2OpenGL::oglCore;
using namespace D2OpenGL::BinkCutscene;
using namespace D2OpenGL::oglBlocks;
using namespace D2OpenGL::oglDebugFont;
using namespace D2OpenGL::oglEngine;
using namespace D2OpenGL::oglFloorCache;
using namespace D2OpenGL::oglMac;
using namespace D2OpenGL::oglPerspective;
using namespace D2OpenGL::oglSmack;
using namespace D2OpenGL::oglSprite;
using namespace D2OpenGL::oglVertex;
using namespace D2OpenGL::MacDisplayDevice;

// Win 0 / Mac 0
BOOL __fastcall Initialize(HINSTANCE hInstance) {
#if !defined(_WIN32) && D2_BACKEND_POSIX
    // D2gfx makes the game window after this: on POSIX it must be made for a GL context.
    D2OpenGL::POSIX_PrepareGameWindowForGL();
#endif
    return OGL_Initialize(hInstance);
}

// Win 1 / Mac 1
BOOL __fastcall InitPerspective(D2GfxSettingsStrc * pSettings, ::D2GfxHelperStrc * pHelpers) {
    return OGL_InitPerspective(pSettings, pHelpers);
}

// Win 3 / Mac 3
BOOL __fastcall CreateRenderWindow(HWND hWnd, int nResolutionMode) {
    return OGL_CreateWindow(hWnd, nResolutionMode);
}

// Win 4 / Mac 4
BOOL __stdcall DestroyRenderWindow() {
    return OGL_DestroyWindow();
}

// Win 5 / Mac 5
void __fastcall EndCutScene(HWND hWnd, int nResolutionMode, int nWindowState) {
    OGL_PauseSurface(hWnd, nResolutionMode, nWindowState);
}

// Win 6 / Mac 6
BOOL __fastcall BeginScene(BOOL bClear, BYTE nRed, BYTE nGreen, BYTE nBlue) {
    return OGL_BeginScene(bClear, nRed, nGreen, nBlue);
}

// Win 9 / Mac 9
BOOL __fastcall ResizeWindow(HWND hWnd, BOOL nResolutionMode) {
    return OGL_ResizeWindow(hWnd, nResolutionMode);
}

// Win 10 / Mac 10
BOOL __fastcall GetBackBuffer(BYTE * pBuffer) {
    return OGL_GetBackBuffer(pBuffer);
}

// Win 11 / Mac 14. D2GFX_ActivateWindow stores !bActive in bD2GFX_Windowed right
// before the call, and IsFullscreen reads it back, so the argument itself is not needed.
BOOL __fastcall ActivateWindow(BOOL bActive) {
    return OGL_ActivateWindow((BOOL)D2gfx::D2GFX::IsFullscreen());
}

// Win 12 / Mac 15
BOOL __fastcall SetOption(int nOption, intptr_t nValue) {
    return OGL_SetOption(nOption, nValue);
}

// Win 13 / Mac 16
BOOL __stdcall BeginCutScene() {
    return OGL_BeginCutScene();
}

// Win 14 / Mac 17. The frame callback takes no arguments, so __stdcall and cdecl agree.
void __fastcall PlayCutScene(char * szFile, int nResolutionMode, fnBinkFrameCallback pfnFrame) {
    OGL_PlayCutScene(szFile, nResolutionMode, reinterpret_cast<D2OGLFrameCallbackFn>(pfnFrame));
}

// Win 16 / Mac 19. The Mac takes the mode as a char.
void __fastcall DecodeSmacker(char * szSmacker, D2SmackerContextStrc * pContext, int nVersion) {
    OGL_DecodeSmacker(szSmacker, pContext, (char)nVersion);
}

// Win 17 / Mac 20
void __fastcall PlayerSmacker(D2SmackerContextStrc * pContext) {
    OGL_PlayerSmacker(pContext);
}

// Win 18 / Mac 21
void __fastcall CloseSmacker(D2SmackerContextStrc * pContext) {
    OGL_CloseSmacker(pContext);
}

// Win 19 / Mac 22
int * __fastcall GetRenderStatistics() {
    return reinterpret_cast<int *>(OGL_GetRenderStatistics());
}

// Win 20 / Mac 23
int __fastcall GetScreenSize(int * pWidth, int * pHeight) {
    OGL_GetScreenSize(pWidth, pHeight);
    return TRUE;
}

// Win 21 / Mac 24
void __fastcall UpdateScaleFactor(int nScaleFactor) {
    OGL_UpdateScaleFactor(nScaleFactor);
}

// Win 22 / Mac 25
void __fastcall SetGamma(int nGamma) {
    OGL_SetGamma(nGamma);
}

// Win 24 / Mac 27
void __fastcall SetPerspectiveScale(int nCenterX, int nCenterY) {
    OGL_SetPerspectiveScale(nCenterX, nCenterY);
}

// Win 25 / Mac 28
void __fastcall AdjustPerspectivePosition(int nX, int nY, int nBias, int * pXAdjust, int * pYAdjust) {
    OGL_AdjustPerspectivePosition(nX, nY, nBias, pXAdjust, pYAdjust);
}

// Win 26 / Mac 29
void __fastcall PerspectiveScalePosition(int nX, int nY, int nAngle, int * pXAdjust, int * pYAdjust, BOOL bOrder) {
    OGL_PerspectiveScalePosition(nX, nY, nAngle, pXAdjust, pYAdjust, bOrder);
}

// Win 28 / Mac 31. The palette is 256 PALETTEENTRY, read by the Mac as bytes.
void __fastcall SetPalette(LPPALETTEENTRY pPalette) {
    OGL_SetPalette(reinterpret_cast<BYTE *>(pPalette));
}

// Win 29 / Mac 32
void __fastcall SetPaletteTable(BYTE * * pPaletteTable) {
    OGL_SetPaletteTable(pPaletteTable);
}

// Win 30 / Mac 33
void __fastcall SetGlobalLight(int nRed, int nGreen, int nBlue) {
    OGL_SetGlobalLight(static_cast<BYTE>(nRed), static_cast<BYTE>(nGreen), static_cast<BYTE>(nBlue));
}

// Win 31 / Mac 34. The Windows slot returns nothing.
void __fastcall DrawGroundTile(D2TileLibraryEntryStrc * pTile, D2GfxLightExStrc * pLight, int nX, int nY, int nWorldX, int nWorldY, BYTE nAlpha, int nScreenPanels, void * pTileData) {
    OGL_DrawGroundTile(pTile, pLight, nX, nY, nWorldX, nWorldY, nAlpha, nScreenPanels, pTileData);
}

// Win 32 / Mac 35
void __fastcall DrawPerspectiveImage(D2GfxDataStrc * pData, int nX, int nY, DWORD dwGamma, int nDrawMode, int nScreenMode, BYTE * pPalette) {
    OGL_DrawPerspectiveImage(pData, nX, nY, dwGamma, nDrawMode, nScreenMode, pPalette);
}

// Win 33 / Mac 36. The Mac returns nothing.
BOOL __fastcall DrawImage(D2GfxDataStrc * pData, int nX, int nY, DWORD dwGamma, int nDrawMode, BYTE * pPalette) {
    OGL_DrawImage(pData, nX, nY, dwGamma, nDrawMode, pPalette);
    return TRUE;
}

// Win 34 / Mac 37
void __fastcall DrawShiftedImage(D2GfxDataStrc * pData, int nX, int nY, DWORD dwGamma, DRAWMODE nDrawMode, GLOBALPALLETE nColorIndex) {
    OGL_DrawShiftedImage(pData, nX, nY, dwGamma, nDrawMode, nColorIndex);
}

// Win 35 / Mac 38
void __fastcall DrawVerticalCropImage(D2GfxDataStrc * pData, int nX, int nY, int nSkipLines, int nDrawLines, int nDrawMode) {
    OGL_DrawVerticalCropImage(pData, nX, nY, nSkipLines, nDrawLines, nDrawMode);
}

// Win 36 / Mac 39
void __fastcall DrawShadow(D2GfxDataStrc * pData, int nX, int nY) {
    OGL_DrawShadow(pData, nX, nY);
}

// Win 37 / Mac 40
void __fastcall DrawImageFast(D2GfxDataStrc * pData, int nX, int nY, BYTE nPaletteIndex) {
    OGL_DrawImageFast(pData, nX, nY, nPaletteIndex);
}

// Win 38 / Mac 41. The crop rectangle is untyped on Windows.
void __fastcall DrawClippedImage(D2GfxDataStrc * pData, int nX, int nY, void * pCropRect, int nDrawMode) {
    OGL_DrawClippedImage(pData, nX, nY, static_cast<D2GfxTileRectStrc *>(pCropRect), nDrawMode);
}

// Win 39 / Mac 42
BOOL __fastcall DrawWallTile(D2TileLibraryEntryStrc * pTile, int nX, int nY, D2GfxLightStrc * pLight, int nScreenPanels) {
    return OGL_DrawWallTile(pTile, nX, nY, pLight, nScreenPanels);
}

// Win 40 / Mac 43
BOOL __fastcall DrawTransWallTile(D2TileLibraryEntryStrc * pTile, int nX, int nY, D2GfxLightStrc * pLight, int nScreenPanels, BYTE nAlpha) {
    return OGL_DrawTransWallTile(pTile, nX, nY, pLight, nScreenPanels, nAlpha);
}

// Win 41 / Mac 44
BOOL __fastcall DrawShadowTile(D2TileLibraryEntryStrc * pTile, int nX, int nY, int nDrawMode, int nScreenPanels) {
    return OGL_DrawShadowTile(pTile, nX, nY, nDrawMode, nScreenPanels);
}

// Win 42 / Mac 45
void __fastcall DrawRect(RECT * pRect, BYTE nPaletteIndex) {
    OGL_DrawRect(pRect, nPaletteIndex);
}

// Win 43 / Mac 46
void __fastcall DrawRectEx(RECT * pRect, BYTE nPaletteIndex) {
    OGL_DrawRectEx(pRect, nPaletteIndex);
}

// Win 44 / Mac 47
void __fastcall DrawSolidRect(RECT * pRect, BYTE nPaletteIndex) {
    OGL_DrawSolidRect(pRect, nPaletteIndex);
}

// Win 45 / Mac 48
void __fastcall DrawSolidSquare(POINT * pPoint, BYTE nSize, BYTE nPaletteIndex) {
    OGL_DrawSolidSquare(pPoint, nSize, nPaletteIndex);
}

// Win 46 / Mac 49. The colour is a palette index.
void __fastcall DrawSolidRectEx(int nX0, int nY0, int nX1, int nY1, BYTE nColor, DRAWMODE nDrawMode) {
    OGL_DrawSolidRectEx(nX0, nY0, nX1, nY1, nColor, nDrawMode);
}

// Win 47 / Mac 50. The colour is a palette index.
void __fastcall DrawSolidRectAlpha(int nX0, int nY0, int nX1, int nY1, DWORD dwColor, BYTE nAlpha) {
    OGL_DrawSolidRectAlpha(nX0, nY0, nX1, nY1, static_cast<BYTE>(dwColor), nAlpha);
}

// Win 48 / Mac 51. The colour is a palette index.
void __fastcall DrawLine(int nX0, int nY0, int nX1, int nY1, BYTE dwColor, BYTE nAlpha) {
    OGL_DrawLine(nX0, nY0, nX1, nY1, static_cast<BYTE>(dwColor), nAlpha);
}

// Win 49 / Mac 52
void __fastcall ClearScreen(BOOL bPartial) {
    OGL_ClearScreen(bPartial);
}

// Win 50 / Mac 53. The Mac types the argument list as the char * it is on i386.
void __fastcall DrawString(int nX, int nY, char * szFormat, va_list va) {
    OGL_DrawString(nX, nY, szFormat, reinterpret_cast<char *>(va));
}

// Win 51 / Mac 54
void __fastcall DrawLight(DWORD * pLight, DWORD * pPlayerLight, int nX, int nY) {
    OGL_DrawLight(pLight, pPlayerLight, nX, nY);
}

// Win 52 / Mac 55
void __fastcall DebugFillBackBuffer(int nX, int nY) {
    OGL_DebugFillBackBuffer(nX, nY);
}

} // namespace

// Slots without an adapter take no arguments and have the same return type on
// both sides, so the Mac function is the slot. The Windows table types them
// __fastcall; with no arguments that is the same call as the Mac's default, so
// the cast changes the type only.
D2RendererFunctionsStrc OpenGLFunctionTable = {
    /* .nfpInitialize = */ &Initialize,
    /* .nfpInitPerspective = */ &InitPerspective,
    /* .nfpRelease = */ (decltype(D2RendererFunctionsStrc::nfpRelease))&D2OpenGL::oglCore::OGL_Release,
    /* .nfpCreateWindow = */ &CreateRenderWindow,
    /* .nfpDestroyWindow = */ &DestroyRenderWindow,
    /* .nfpEndCutScene = */ &EndCutScene,
    /* .nfpBeginScene = */ &BeginScene,
    /* .fpEndScene1 = */ (decltype(D2RendererFunctionsStrc::fpEndScene1))&D2OpenGL::oglCore::OGL_EndScene1,
    /* .fpEndScene2 = */ (decltype(D2RendererFunctionsStrc::fpEndScene2))&D2OpenGL::oglCore::OGL_EndScene2,
    /* .nfpResizeWindow = */ &ResizeWindow,
    /* .nfpGetBackBuffer = */ &GetBackBuffer,
    /* .nfpActivateWindow = */ &ActivateWindow,
    /* .nfpSetOption = */ &SetOption,
    /* .nfpBeginCutScene = */ &BeginCutScene,
    /* .nfpPlayCutScene = */ &PlayCutScene,
    /* .nfpCheckCutScene = */ (decltype(D2RendererFunctionsStrc::nfpCheckCutScene))&D2OpenGL::oglCore::OGL_CheckCutScene,
    /* .nfpDecodeSmacker = */ &DecodeSmacker,
    /* .nfpPlayerSmacker = */ &PlayerSmacker,
    /* .nfpCloseSmacker = */ &CloseSmacker,
    /* .nfpGetRenderStatistics = */ &GetRenderStatistics,
    /* .nfpGetScreenSize = */ &GetScreenSize,
    /* .nfpUpdateScaleFactor = */ &UpdateScaleFactor,
    /* .nfpSetGamma = */ &SetGamma,
    /* .nfpCheckGamma = */ (decltype(D2RendererFunctionsStrc::nfpCheckGamma))&D2OpenGL::oglCore::OGL_CheckGamma,
    /* .nfpSetPerspectiveScale = */ &SetPerspectiveScale,
    /* .nfpAdjustPerspectivePosition = */ &AdjustPerspectivePosition,
    /* .nfpPerspectiveScalePosition = */ &PerspectiveScalePosition,
    /* .nfpSetDefaultPerspectiveFactor = */ (decltype(D2RendererFunctionsStrc::nfpSetDefaultPerspectiveFactor))&D2OpenGL::oglPerspective::OGL_SetDefaultPerspectiveFactor,
    /* .nfpSetPalette = */ &SetPalette,
    /* .nfpSetPaletteTable = */ &SetPaletteTable,
    /* .nfpSetGlobalLight = */ &SetGlobalLight,
    /* .nfpDrawGroundTile = */ &DrawGroundTile,
    /* .nfpDrawPerspectiveImage = */ &DrawPerspectiveImage,
    /* .nfpDrawImage = */ &DrawImage,
    /* .nfpDrawShiftedImage = */ &DrawShiftedImage,
    /* .nfpDrawVerticalCropImage = */ &DrawVerticalCropImage,
    /* .nfpDrawShadow = */ &DrawShadow,
    /* .nfpDrawImageFast = */ &DrawImageFast,
    /* .nfpDrawClippedImage = */ &DrawClippedImage,
    /* .nfpDrawWallTile = */ &DrawWallTile,
    /* .nfpDrawTransWallTile = */ &DrawTransWallTile,
    /* .nfpDrawShadowTile = */ &DrawShadowTile,
    /* .nfpDrawRect = */ &DrawRect,
    /* .nfpDrawRectEx = */ &DrawRectEx,
    /* .nfpDrawSolidRect = */ &DrawSolidRect,
    /* .nfpDrawSolidSquare = */ &DrawSolidSquare,
    /* .nfpDrawSolidRectEx = */ &DrawSolidRectEx,
    /* .nfpDrawSolidRectAlpha = */ &DrawSolidRectAlpha,
    /* .nfpDrawLine = */ &DrawLine,
    /* .nfpClearScreen = */ &ClearScreen,
    /* .nfpDrawString = */ &DrawString,
    /* .nfpDrawLight = */ &DrawLight,
    /* .nfpDebugFillBackBuffer = */ &DebugFillBackBuffer,
    /* .nfpClearCaches = */ (decltype(D2RendererFunctionsStrc::nfpClearCaches))&D2OpenGL::oglCore::OGL_ClearCaches,
};

} // namespace D2OpenGL::Renderer
