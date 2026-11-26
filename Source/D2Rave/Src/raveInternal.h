// D2Rave's internal header: the state the module's files share, the engine entry points it
// calls, and every function of the module. Ours, not original: no D2Rave header name is
// attested, and the PPC build keeps no header paths.
//
// Primary build: mac/powerpc/classic/1.09d (Diablo_II PEF, CodeWarrior, big-endian, CFM).
// Addresses in comments are 1.09d unless marked 1.00. Globals are reached through the TOC in
// the binary (r2 = 0x1037c590 in 1.09d); each one notes the address of its storage.
//
// Why this header declares engine functions instead of including the engine's headers: the
// shared tree headers (d2_platform.h and everything on it) do not compile on a Mac target yet,
// so the few engine calls D2Rave makes are declared here by the name the tree gives the same
// function where one is known, with the PPC address of the callee. When the shared headers are
// Mac-clean these declarations go and the real headers are included.
//
// Classic Mac PowerPC only (non-Carbon builds 1.00-1.13d; Carbon builds have no RAVE).
#pragma once

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

#include <stdint.h>
#include <stdarg.h>
#include "Redivivus/Platform/MacClassic/MacClassicToolbox.h"
#include "Redivivus/Platform/MacClassic/MacClassicRave.h"
#include "Redivivus/Platform/MacClassic/MacClassicDrawSprocket.h"
#include "Redivivus/Platform/MacClassic/MacClassicBink.h"
#include "Source/D2Rave/D2Rave.h"

// Every D2Rave file ends with a 12-byte __sinit_<file> that copies +infinity into a file-local
// float: a header-scope static initialised from MSL's __float_huge (HUGE_VALF is not a
// constant expression under CodeWarrior). This is that static; nothing in D2Rave reads it.
static float sgfRaveHugeValue = __builtin_huge_valf();

// ---------------------------------------------------------------------------------------------
// Engine and system entry points D2Rave calls
// ---------------------------------------------------------------------------------------------

// Storm (CFM library "Storm"), reached through the game's import glue.
extern "C" {
void * SMemAlloc(uint32_t nSize, const char * szFile, uint32_t nLine, uint32_t nFlags);  // glue 1031daac
int32_t SMemFree(void * pMemory, const char * szFile, int32_t nLine, uint32_t dwFlags);  // glue 1031dad0
int32_t SFileOpenFileEx(void * hMpq, const char * szFileName, uint32_t dwFlags, void ** phFile);

// Storm's Win32-on-Mac layer (the same imports the Intel Mac build's Win32Mac layer provides).
void ZeroMemory(void * pDest, uint32_t nSize);
void CopyMemory(void * pDest, const void * pSrc, uint32_t nSize);
uint32_t GetTickCount(void);
int32_t CloseHandle(void * hObject);
int32_t FileRefFromHANDLE(void * hFile, int16_t * pRefNum);

// Not Toolbox calls: imported by the PEF from the game's own libraries.
void SetMousePosition(Point where);
void YieldToThinThread(int32_t nThread);
}

// The Mac shell (Mac Shell/GameVideo.cpp and its neighbours). Mangled C++ names in the PEF:
// SetGammaCurve__FPP7GDeviceii, RestoreGammaCurve__FPP7GDevice, SetMouseInputVideoMode__FUc.
void SetGammaCurve(GDHandle hDevice, int nGamma, int nCurve);
void RestoreGammaCurve(GDHandle hDevice);
void SetMouseInputVideoMode(unsigned char bVideoMode);
// 102cd0f8, the shell's DrawSprocket context finder; the name is the one D2Rave's own log
// strings give it ("DSpFindContextEx (%s) failed").
OSStatus DSpFindContextEx(GDHandle hDevice, int16_t nWidth, int16_t nHeight, int32_t nRefresh, int32_t nDepth,
                          DSpContextReference * pContext, DSpContextAttributes * pAttributes);
// 1034210c: the MoreFunctional-style stack crawl (import glue); fills szBuffer.
void StackCrawl(char * szBuffer, int32_t nSize, const char * szPrefix, const char * szSuffix, int32_t nSkip,
                int32_t nMaxFrames);
// The MSL fatal error (".debug::_fp_fatal").
void fp_fatal(const char * szFormat, ...);

// MSL.
extern "C" int vsprintf(char * szBuffer, const char * szFormat, va_list args);  // 1033c414 (via vsnprintf, size -1)
extern "C" void * memmove(void * pDest, const void * pSrc, unsigned long nSize);  // 10339c10

// Storm's Win32-on-Mac layer, C++ linkage in the PEF (SetRect__FP7tagRECTiiii).
struct tagRECT {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
};
int SetRect(tagRECT * pRect, int nLeft, int nTop, int nRight, int nBottom);

// The game's host interface, called through a table right after the RAVE window is created
// (lwz r3,-0x73d0(r2); lwz r3,0(r3); method at +0x6c). Not RAVE. Identity and arguments open.
void RaveHostWindowCreated();

// D2Lang/D2Win, by PPC address: the 2-byte Unicode character class and the debug-text calls.
struct Unicode {
    uint16_t wChar;
    Unicode();                                                                 // array ctor at TOC slot 10375220
    static Unicode * toUnicode(Unicode * pDest, const char * szSrc, int nMax);  // 102d0ea0
};
int D2Win_SetFont(int nFont);                                                         // 1030ce9c
void D2Win_DrawText(const Unicode * wszText, int nX, int nY, int nColor, int bCentered);  // 1030bc60

// Fog. 1031f95c logs a printf-style line to the game log; the Windows counterpart it pairs
// with by shape is LOGFILE_WriteStringToFile (a candidate pairing, not proven).
void LOGFILE_WriteStringToFile(const char * szFormat, ...);
// 103178a0: Fog's -direct switch (the Windows renderers call GetUseDirectCommand at this spot).
int GetUseDirectCommand();
// 10340490: registers an exit callback in Fog's 64-entry table; returns -1 when it is full.
int FOG_RegisterExitCallback(void (*pfnCallback)());

// MacSpecific/Source/CD2Textures (CD2Textures.cpp is attested in the PPC build too). The tree's
// 1.14d names for the same functions; PPC addresses in comments.
class CD2Textures;
int CreateTexture(CD2Textures * pThis, void * pPixels, int nWidth, int nHeight, int nOwnerType, void * pOwner,
                  int nOwnerIndex, intptr_t nOwnerKey);                                        // 102cdc78
void CD2TEXTURES_ReleaseById(CD2Textures * pThis, int nTextureId);                       // 102cdc2c
void CD2TEXTURES_TouchEntry(CD2Textures * pThis, int nTextureId);                        // 102cdb00
void EndFrame(CD2Textures * pThis);                                                      // 102cdad8
// 102cdbc4, called by SetOption 9 after the texture-limit flag changes; identification open
// (the tree's CD2TEXTURES_Flush is the candidate).
void CD2TEXTURES_Flush(CD2Textures * pThis);

// D2CMP, by PPC address. The cel cache, the DC6 cel accessors and the cache callbacks.
int D2CMP_GetCelData(struct D2GfxDataStrc * pData, int nUnused, int bLoad);           // 10298190
int D2CMP_CelGetWidth(void * pCel);                                                 // 1029154c (+0x04)
int D2CMP_CelGetHeight(void * pCel);                                                // 10291544 (+0x08)
int D2CMP_CelGetOffsetX(void * pCel);                                               // 1029153c (+0x0c)
int D2CMP_CelGetOffsetY(void * pCel);                                               // 10291534 (+0x10)
void D2CMP_SetSpriteFreeCallback(void (*pfnCallback)(int * pEntries));             // 10291524
void D2CMP_SetTileFreeCallback(void (*pfnCallback)(struct RaveTileEntry * pTile));  // 1029ab54
int D2CMP_LoadTileData(struct RaveTileEntry * pTile, int nUnused, int nUnused2);     // 1029a1f4

// ---------------------------------------------------------------------------------------------
// Types D2Rave reads (PPC layout as the module dereferences it; field names inferred)
// ---------------------------------------------------------------------------------------------

// One block of a D2 tile (the Windows tree calls the entry D2TileLibraryEntryStrc). 0x14 bytes.
struct RaveTileBlock {
    /* 0x00 */ int16_t nPosX;
    /* 0x02 */ int16_t nPosY;
    /* 0x04 */ int16_t nTextureId;   // CD2Textures id + 1-based, 0 = not uploaded
    /* 0x06 */ int16_t nUnknown6;
    /* 0x08 */ int16_t nFormat;      // bit 0: present; bit 2: RLE (else isometric diamond)
    /* 0x0a */ int16_t nUnknownA;
    /* 0x0c */ int32_t nUnknownC;
    /* 0x10 */ uint8_t * pData;
};

struct RaveTileEntry {
    /* 0x00 */ uint8_t unknown0[0x42];
    /* 0x42 */ int16_t nTextureId;
    /* 0x44 */ uint8_t unknown44[0xc];
    /* 0x50 */ int32_t nBlocks;
    /* 0x54 */ RaveTileBlock * pBlocks;
};

// A frame buffer as RaveChangeRes and RaveCutsceneCreateWindow describe one to sRaveFillDisplay:
// built from the device's PixMap (baseAddr, bounds width/height, pixelSize >> 3, rowBytes).
struct RaveFrameBuffer {
    /* 0x00 */ uint8_t * pBase;
    /* 0x04 */ int32_t nUnused;
    /* 0x08 */ int16_t nWidth;
    /* 0x0a */ int16_t nHeight;
    /* 0x0c */ int16_t nBytesPerPixel;
    /* 0x0e */ int16_t nRowBytes;
};

// One DrawSprocket display mode (bss 1052efb0, indexed by resolution mode).
struct RaveDisplayMode {
    int16_t nWidth;
    int16_t nHeight;
    int32_t nRefresh;
};

// The leading fields of the 2D vertex the sprite, floor and wall drawers light (field names
// from the tree's D2OGLVertexStrc, the OpenGL sibling).
struct RaveVertex {
    /* 0x00 */ float fX;
    /* 0x04 */ float fY;
    /* 0x08 */ uint8_t nRed;
    /* 0x09 */ uint8_t nGreen;
    /* 0x0a */ uint8_t nBlue;
    /* 0x0b */ uint8_t nAlpha;
    /* 0x0c */ float fU;
    /* 0x10 */ float fV;
    /* 0x14 */ float fUnused;
};

// The sprite descriptor the image drawers build on the stack (0x30 bytes): the cel's size and
// offset, its power-of-two texture size and texture coordinates.
struct RaveSprite {
    /* 0x00 */ struct D2GfxDataStrc * pData;
    /* 0x04 */ int nWidth;
    /* 0x08 */ int nHeight;
    /* 0x0c */ int nOffsetX;
    /* 0x10 */ int nOffsetY;
    /* 0x14 */ int nTexWidth;
    /* 0x18 */ int nTexHeight;
    /* 0x1c */ int nTexArea;
    /* 0x20 */ float fU0;
    /* 0x24 */ float fV0;
    /* 0x28 */ float fU1;
    /* 0x2c */ float fV1;
};

class CRaveTextures;

namespace D2Rave {

// ---------------------------------------------------------------------------------------------
// Shared state (raveCore.cpp unless noted)
// ---------------------------------------------------------------------------------------------

extern TQADrawContext * gpRaveDrawContext;   // 1052f0fc
extern TQAEngine * gpRaveEngine;             // 1052f100
extern char gbRaveInDraw;                    // 1052f104
extern uint32_t gnRaveBinkSurfaceType;       // 1052f108
extern int gbRavePaused;                     // 1052f10c
extern int gnRaveScreenHeight;               // 1052f110
extern int gnRaveScreenWidth;                // 1052f114
extern char gbRaveModeFlag;                  // 1052f118, per mode from gnRaveModeFlags
extern uint8_t ** gpRavePaletteTable;        // 1052f11c
extern void * gpRaveHelpers;                 // 1052f120, RaveInit's second argument
extern void * gpRaveSettings;                // 1052f124, RaveInit's first argument
extern int gbRaveFastTexture;                // 1052f128, kQAFast_Texture (FastFeatures bit 2)
extern CRaveTextures * gpRaveTextures;       // 1052efd4
extern int gnRaveGamma;                      // 103f2794 = 155
extern int gnRaveRequestedDepth;             // 103f27c8 = 16
extern int gnRaveRequestedEngineID;          // 103f27cc = -1 (any engine)
extern float gRaveClipRect[4];               // 103f2c64: top, left, bottom, right (pixel centres)
extern uint8_t gRavePaletteRGB[256][3];      // RGB24
extern uint8_t gRavePaletteRGBA[256][4];     // RGBA, alpha 0 for black
extern uint32_t gdwRavePaletteARGB32[256];   // 1052ebb0, 0xFFrrggbb (0 for black)
extern uint16_t gwRavePaletteARGB1555[256];  // 0x8000|rgb555 (0 for black)
extern Boolean gbRaveSwapBuffers;            // 1037c760, OptionalFeatures2 bit 4
extern Boolean gbRaveContextOpen;            // 1037c761
extern WindowPtr gpRaveWindow;               // 1037c764
extern int gnRaveDisplayDepth;               // 1037c768
extern DSpContextReference ghRaveDSpContext; // 1037c76c
extern Point gRaveWindowOrigin;              // 1037c770
extern GDHandle ghRaveDevice;                // 1037c774
extern int gbRaveCutscenePlaying;            // 1037c75c

// ---------------------------------------------------------------------------------------------
// raveBink.cpp
// ---------------------------------------------------------------------------------------------
void RavePlayBinkFile(const char * szFile, int nMode, uint8_t * pDest, int nPitch, void (*pfnFrame)());

// ---------------------------------------------------------------------------------------------
// raveBlocks.cpp
// ---------------------------------------------------------------------------------------------
extern uint8_t * gpRaveWallTileBuffer;   // 1037c738, 0x400
extern uint8_t * gpRaveBlockBuffer;      // 1037c734, 0x1000
extern uint8_t * gpRaveFloorTileBuffer;  // 1037c730, 0x20000
void sRaveFreeTileTextures(RaveTileEntry * pTile);
void sRaveLoadWallTexture(RaveTileBlock * pBlock);
int sRaveLoadFloorTexture(RaveTileEntry * pTile);
void sRaveDecodeFloorTile(RaveTileEntry * pTile, uint8_t * pDest, int nPitch);
void sRaveDecodeWallTile(const uint8_t * pSrc, uint8_t * pDest);
void sRaveBlocksFree();
void sRaveBlocksInit();

// ---------------------------------------------------------------------------------------------
// raveCore.cpp
// ---------------------------------------------------------------------------------------------
int RaveSetOption(unsigned int nOption, intptr_t nValue);
int RaveSetFullScreen(int bFullScreen);
int RaveLocalToGlobal(Point * pPoint);
int RaveGlobalToLocal(Point * pPoint);
Boolean RaveProcessEvent(EventRecord * pEvent);
int * RaveGetRenderStatistics();
void sRaveSetTextureFilter(int nFilter);
void sRaveSetBlendMode(int nMode);
void sRaveSetAlphaState(int nState);
int RaveChangeRes(int nUnused, int nMode);
void RaveSetPaletteTable(uint8_t ** pTable);
uint32_t sRaveGetPaletteARGB32(int nIndex);
void RaveSetPalette(const uint8_t * pEntries);
void RaveGetScreenSize(int * pWidth, int * pHeight);
int RaveScreenGrab(uint8_t * pDest);
int RaveBlit();
int RaveCheckGamma();
void RaveSetGamma(int nGamma);
void RaveUpdateScaleFactor(int nScaleFactor);
int RaveEndDraw();
int RaveStartDraw();
int RaveCheckCutscene();
void RavePlayCutscene(const char * szFile, int nMode, void (*pfnFrame)());
int RaveCutsceneCreateWindow();
int iRaveSwapContext(int nMode, int nDepth);
void RavePauseSurface(void * hWindow, int nMode, int bPause);
void RaveClearCaches();
int RaveCloseSurface();
int RaveCreateSurface(void * hWindow, int nMode);
int RaveRelease();
void sRaveCloseAll();
int RaveInit(void * pSettings, void * pHelpers);
int sInitRaveContext(int nMode);
int sCloseRaveContext();
int sInitRaveDrawSprocket(int nMode);
int sRaveEngineReport(TQAEngine * pEngine, int bReport);
int RaveDetect();
void RaveCheckError(TQAError nError);
void sRaveFillDisplay(const RaveFrameBuffer * pBuffer, uint8_t nValue, unsigned int nFirstRow, unsigned int nRows,
                      char bInterlaced);


// ---------------------------------------------------------------------------------------------
// raveDebug.cpp
// ---------------------------------------------------------------------------------------------
void RaveDrawLight(uint32_t * pLight, uint32_t * pColor, int nX, int nY);
void RaveDrawString(int nX, int nY, const char * szFormat, va_list args);
void sRaveHookStub0();
void sRaveHookStub1();
void sRaveHookStub2();
void sRaveHookStub3();

// ---------------------------------------------------------------------------------------------
// raveEngine.cpp
// ---------------------------------------------------------------------------------------------
void RaveClearScreen(int bClear);
void RaveDrawSolidSquare(const int * pPoint, uint8_t nSize, uint8_t nColor);
void RaveDrawSolidRect(const int * pRect, uint8_t nColor);
void RaveDrawRectEx(const int * pRect, uint8_t nColor);
void RaveDrawRect(const int * pRect, uint8_t nColor);
void RaveDrawLine(int nX1, int nY1, int nX2, int nY2, uint8_t nColor, uint8_t nAlpha);
void RaveDrawSolidRectAlpha(int nX1, int nY1, int nX2, int nY2, uint8_t nColor, uint32_t nAlpha);
void RaveDrawSolidRectEx(int nX1, int nY1, int nX2, int nY2, uint8_t nColor, int nDrawMode);
int sRaveGetSettings14();

// ---------------------------------------------------------------------------------------------
// raveFloor.cpp
// ---------------------------------------------------------------------------------------------
void RaveDrawGroundTile(RaveTileEntry * pTile, void * pLight, int nX, int nY, int nWorldX, int nWorldY,
                        uint8_t nAlpha, int nScreenPanels, void * pTileData);
extern RaveVertex gRaveFloorVertices[6 * 6];   // 1052f21c
extern RaveVertex * gpRaveFloorStrips[5][12];  // 1052f12c
void sRaveFloorInit();

// ---------------------------------------------------------------------------------------------
// ravePerspective.cpp
// ---------------------------------------------------------------------------------------------
extern int gnRavePerspectiveFactor;
void RaveSetDefaultPerspectiveFactor();
void RavePerspectiveScalePosition(int nX, int nY, int nUnused, int * pX, int * pY, int bScale);
void RaveAdjustPerspectivePosition(int nX, int nY, int nBias, int * pX, int * pY);
void RaveSetPerspectiveScale(int nCenterX, int nCenterY);
int sRaveGetCurrentTexture();
void sRavePerspectiveInit();
void sRavePerspectiveFree();
void sRavePerspectiveAlloc();

// ---------------------------------------------------------------------------------------------
// raveSmack.cpp
// ---------------------------------------------------------------------------------------------
void sRaveDrawSmackerFrame(D2SmackerContextStrc * pContext);
void RaveCloseSmacker(D2SmackerContextStrc * pContext);
void RavePlaySmacker(D2SmackerContextStrc * pContext);
void RaveDecodeSmacker(const char * szFile, D2SmackerContextStrc * pContext, char nVersion);
extern uint8_t * gpRaveSmackBuffer;       // 1037c828, 0x10000
extern uint8_t * gpRaveSmackLineBuffer;   // 1037c82c, 0x8000
extern TQATexture * gpRaveSmackTextures[2];  // 1052f580
extern RaveVertex gRaveSmackVertices[4];     // 1052f588
void sRaveSmackFree();
void sRaveSmackInit();

// ---------------------------------------------------------------------------------------------
// raveSprite.cpp
// ---------------------------------------------------------------------------------------------
void RaveDebugFillBackBuffer(int nX, int nY);
extern uint8_t * gpRaveSpriteBufferA;   // 1037c864, 0x10000
extern uint8_t * gpRaveSpriteBufferB;   // 1037c860, 0x10000
extern uint8_t * gpRaveSpriteBufferC;   // 1037c85c, 0x40000
int sRaveSetSpriteDrawMode(int nDrawMode);
void sRaveGetSpriteTexture(RaveSprite * pSprite);
void sRaveReleaseSpriteTextures(int * pEntries);
int sRaveClipSpriteRect(RaveSprite * pSprite, int nX, int nY, void * pCropRect);
int sRaveUploadSprite(RaveSprite * pSprite, uint8_t * pPalette, int nPaletteIndex, int nKind);
void sRaveDrawSpriteQuad(RaveSprite * pSprite, int nX, int nY, uint32_t dwColorTint, int nPalette, int nAlpha,
                         int bFast, int nUnused);
void sRaveDrawShadowQuad(RaveSprite * pSprite, int nX, int nY, int nAlpha);
void RaveDrawImageFast(D2GfxDataStrc * pData, int nX, int nY, uint8_t nPaletteIndex);
void RaveDrawShadow(D2GfxDataStrc * pData, int nX, int nY);
void RaveDrawClippedImage(D2GfxDataStrc * pData, int nX, int nY, void * pCropRect, int nDrawMode);
void RaveDrawVerticalCropImage(D2GfxDataStrc * pData, int nX, int nY, int nSkipLines, int nDrawLines, int nDrawMode);
int RaveDrawShiftedImage(D2GfxDataStrc * pData, int nX, int nY, uint32_t dwColorTint, int nDrawMode, int nPalette);
int RaveDrawImage(D2GfxDataStrc * pData, int nX, int nY, uint32_t dwColorTint, int nDrawMode, uint8_t * pPalette);
void RaveDrawPerspectiveImage(D2GfxDataStrc * pData, int nX, int nY, uint32_t dwColorTint, int nDrawMode,
                              int nPalette, uint8_t * pPalette);
void sRaveSpriteInit();
void sRaveSpriteFree();

// ---------------------------------------------------------------------------------------------
// raveVertex.cpp
// ---------------------------------------------------------------------------------------------
extern uint8_t gnRaveGlobalBlue;
extern uint8_t gnRaveGlobalGreen;
extern uint8_t gnRaveGlobalRed;
extern uint8_t * gpRaveLightTable;
void RaveSetGlobalLight(uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
void sRaveApplyLight(RaveVertex * pVertex, uint32_t dwLight);
void sRaveApplyGlobalLight(RaveVertex * pVertex, uint32_t dwLight);
void sRaveVertexInit();
void sRaveVertexFree();
void sRaveVertexAlloc();

// ---------------------------------------------------------------------------------------------
// raveWall.cpp
// ---------------------------------------------------------------------------------------------
int RaveDrawWallTile(RaveTileEntry * pTile, int nX, int nY, void * pLight, int nScreenPanels);
int RaveDrawTransWallTile(RaveTileEntry * pTile, int nX, int nY, void * pLight, int nScreenPanels, uint8_t nAlpha);
int RaveDrawShadowTile(RaveTileEntry * pTile, int nX, int nY, int nDrawMode, int nScreenPanels);
int sRaveDrawWallTilePerspective(RaveTileEntry * pTile, int nX, int nY, void * pLight, int nAlpha, int nScreenPanels);
int sRaveDrawWallTileFlat(RaveTileEntry * pTile, int nX, int nY, void * pLight, int nAlpha, int nScreenPanels);
int sRaveDrawShadowTilePerspective(RaveTileEntry * pTile, int nX, int nY, int nDrawMode, int nAlpha, int nScreenPanels);
int sRaveDrawShadowTileFlat(RaveTileEntry * pTile, int nX, int nY, int nDrawMode, int nAlpha, int nScreenPanels);
extern RaveVertex gRaveWallQuad[4];          // 1052f7b4
extern RaveVertex gRaveWallGrid[5][3];       // 1052f64c
extern int gRaveWallTriangles[5][12][3];     // 103f2958, 16.16 after sRaveWallInit
void sRaveWallInit();
void sRaveGetPanelClipRect(int nPanels, int bOffset, int * pRect, int * pOffset);

// ---------------------------------------------------------------------------------------------
// raveClip.cpp
// ---------------------------------------------------------------------------------------------
void QADrawClippedVTexture(TQADrawContext * pContext, uint32_t nVertices, uint32_t nMode, TQAVTexture * pVertices,
                           const uint32_t * pFlags);
void QADrawClippedLine(TQADrawContext * pContext, TQAVGouraud * pVertices);
int sQAClipPolygon(TQAVTexture * pIn, int nIn, TQAVTexture * pOut);
void sQAClipEdgeA(const TQAVTexture * pA, const TQAVTexture * pB, TQAVTexture * pOut);
void sQAClipEdgeB(const TQAVTexture * pA, const TQAVTexture * pB, TQAVTexture * pOut);
void sQAClipEdgeC(const TQAVTexture * pA, const TQAVTexture * pB, TQAVTexture * pOut);
void sQAClipEdgeD(const TQAVTexture * pA, const TQAVTexture * pB, TQAVTexture * pOut);
int sQAInsideA(const TQAVTexture * pVertex);
int sQAInsideB(const TQAVTexture * pVertex);
int sQAInsideC(const TQAVTexture * pVertex);
int sQAInsideD(const TQAVTexture * pVertex);
int sQAClipLineCode(const TQAVGouraud * pVertices, int nVertices);
int sQAClipHelper(const TQAVTexture * pVertices, int nVertices);

} // namespace D2Rave

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
