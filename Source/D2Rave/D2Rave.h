// Public interface of D2Rave, the classic Mac PowerPC RAVE renderer: the D2gfx function table
// and the renderer entry points it is built from. Ours, not original: no D2Rave header is
// attested. Plain types only, so the table's unit can include the shared D2gfx header beside it.
//
// Classic Mac PowerPC only.
#pragma once

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

#include <stdint.h>
#include <stdarg.h>

struct D2RendererFunctionsStrc;
struct D2SmackerContextStrc;
struct D2GfxDataStrc;
struct RaveTileEntry;

namespace D2Rave {

// The table D2gfx selects for DISPLAYTYPE_RAVE (7). See raveRenderer.cpp for how the Mac's 57
// slots map onto the shared 54-slot D2RendererFunctionsStrc.
namespace Renderer {
extern D2RendererFunctionsStrc RaveFunctionTable;
} // namespace Renderer

int RaveDetect();
int RaveInit(void * pSettings, void * pHelpers);
int RaveRelease();
int RaveCreateSurface(void * hWindow, int nMode);
int RaveCloseSurface();
void RavePauseSurface(void * hWindow, int nMode, int bPause);
int RaveStartDraw();
int RaveEndDraw();
int RaveBlit();
int RaveChangeRes(int nUnused, int nMode);
int RaveScreenGrab(uint8_t * pDest);
int RaveSetFullScreen(int bFullScreen);
int RaveSetOption(unsigned int nOption, int nValue);
int RaveCutsceneCreateWindow();
void RavePlayCutscene(const char * szFile, int nMode, void (*pfnFrame)());
int RaveCheckCutscene();
void RaveDecodeSmacker(const char * szFile, D2SmackerContextStrc * pContext, char nVersion);
void RavePlaySmacker(D2SmackerContextStrc * pContext);
void RaveCloseSmacker(D2SmackerContextStrc * pContext);
int * RaveGetRenderStatistics();
void RaveGetScreenSize(int * pWidth, int * pHeight);
void RaveUpdateScaleFactor(int nScaleFactor);
void RaveSetGamma(int nGamma);
int RaveCheckGamma();
void RaveSetPerspectiveScale(int nCenterX, int nCenterY);
void RaveAdjustPerspectivePosition(int nX, int nY, int nBias, int * pX, int * pY);
void RavePerspectiveScalePosition(int nX, int nY, int nUnused, int * pX, int * pY, int bScale);
void RaveSetDefaultPerspectiveFactor();
void RaveSetPalette(const uint8_t * pEntries);
void RaveSetPaletteTable(uint8_t ** pTable);
void RaveSetGlobalLight(uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
void RaveDrawGroundTile(RaveTileEntry * pTile, void * pLight, int nX, int nY, int nWorldX, int nWorldY,
                        uint8_t nAlpha, int nScreenPanels, void * pTileData);
void RaveDrawPerspectiveImage(D2GfxDataStrc * pData, int nX, int nY, uint32_t dwColorTint, int nDrawMode,
                              int nPalette, uint8_t * pPalette);
int RaveDrawImage(D2GfxDataStrc * pData, int nX, int nY, uint32_t dwColorTint, int nDrawMode, uint8_t * pPalette);
int RaveDrawShiftedImage(D2GfxDataStrc * pData, int nX, int nY, uint32_t dwColorTint, int nDrawMode, int nPalette);
void RaveDrawVerticalCropImage(D2GfxDataStrc * pData, int nX, int nY, int nSkipLines, int nDrawLines, int nDrawMode);
void RaveDrawShadow(D2GfxDataStrc * pData, int nX, int nY);
void RaveDrawImageFast(D2GfxDataStrc * pData, int nX, int nY, uint8_t nPaletteIndex);
void RaveDrawClippedImage(D2GfxDataStrc * pData, int nX, int nY, void * pCropRect, int nDrawMode);
int RaveDrawWallTile(RaveTileEntry * pTile, int nX, int nY, void * pLight, int nScreenPanels);
int RaveDrawTransWallTile(RaveTileEntry * pTile, int nX, int nY, void * pLight, int nScreenPanels, uint8_t nAlpha);
int RaveDrawShadowTile(RaveTileEntry * pTile, int nX, int nY, int nDrawMode, int nScreenPanels);
void RaveDrawRect(const int * pRect, uint8_t nColor);
void RaveDrawRectEx(const int * pRect, uint8_t nColor);
void RaveDrawSolidRect(const int * pRect, uint8_t nColor);
void RaveDrawSolidSquare(const int * pPoint, uint8_t nSize, uint8_t nColor);
void RaveDrawSolidRectEx(int nX1, int nY1, int nX2, int nY2, uint8_t nColor, int nDrawMode);
void RaveDrawSolidRectAlpha(int nX1, int nY1, int nX2, int nY2, uint8_t nColor, uint32_t nAlpha);
void RaveDrawLine(int nX1, int nY1, int nX2, int nY2, uint8_t nColor, uint8_t nAlpha);
void RaveClearScreen(int bClear);
void RaveDrawString(int nX, int nY, const char * szFormat, va_list args);
void RaveDrawLight(uint32_t * pLight, uint32_t * pColor, int nX, int nY);
void RaveDebugFillBackBuffer(int nX, int nY);
void RaveClearCaches();

} // namespace D2Rave

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
