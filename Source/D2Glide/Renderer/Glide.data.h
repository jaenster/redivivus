/**
 * Data defined in D2Glide/Renderer/Glide.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Glide)
#error "D2Glide header included by a module that may not use D2Glide (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"
#include "Source/D2Glide/Renderer/Glide.h"
#include "Source/D2Glide/glFloorCache.h"
#include "Source/D2Glide/glSubtileCache.h"

// Forward declarations
struct D2GlideVertexTexCacheBlockStrc;
struct GlFloorCacheNodeStrc;
struct GlFloorCacheStrc;
struct GlSubtileCacheNodeStrc;
struct GlSubtileCacheStrc;
typedef FxBool (__stdcall *grLfbLock)(GrLock_t type, GrBuffer_t buffer, GrLfbWriteMode_t writeMode, GrOriginLocation_t origin, FxBool pixelPipeline, GrLfbInfo_t * info);
typedef FxBool (__stdcall *grLfbUnlock)(GrLock_t type, GrBuffer_t buffer);
typedef FxU32 (__stdcall *grGet)(FxU32 pname, FxU32 plength, FxI32 * params);
typedef FxU32 (__stdcall *grTexMaxAddress)(GrChipID_t tmu);
typedef FxU32 (__stdcall *pfnGlideDrawVertexArray)(FxU32 mode, FxU32 Count, void * pointers);
typedef FxU32 (__stdcall *pfnGlideGammaCorrectionRGB)(FxFloat red, FxFloat green, FxFloat blue);
typedef GrContext_t (__stdcall *grSstWinOpen)(FxU32 hWnd, GrScreenResolution_t screen_resolution, GrScreenRefresh_t refresh_rate, GrColorFormat_t color_format, GrOriginLocation_t origin_location, int nColBuffers, int nAuxBuffers);
typedef void (__stdcall *grAlphaCombine)(GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert);
typedef void (__stdcall *grBufferClear)(GrColor_t color, GrAlpha_t alpha, FxU32 depth);
typedef void (__stdcall *grBufferSwap)(FxU32 swap_interval);
typedef void (__stdcall *grChromakeyMode)(GrChromakeyMode_t mode);
typedef void (__stdcall *grChromakeyValue)(GrColor_t value);
typedef void (__stdcall *grColorCombine)(GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert);
typedef void (__stdcall *grColorMask)(FxBool rgb, FxBool a);
typedef void (__stdcall *grConstantColorValue)(GrColor_t value);
typedef void (__stdcall *grCoordinateSpace)(GrCoordinateSpaceMode_t mode);
typedef void (__stdcall *grDepthMask)(FxBool mask);
typedef void (__stdcall *grDitherMode)(GrDitherMode_t mode);
typedef void (__stdcall *grDrawLine)(void * v1, void * v2);
typedef void (__stdcall *grDrawPoint)(void * pt);
typedef void (__stdcall *grDrawVertexArrayContiguous)(FxU32 mode, FxU32 Count, void * pointers, FxU32 stride);
typedef void (__stdcall *grGlideInit)();
typedef void (__stdcall *grGlideShutdown)();
typedef void (__stdcall *grLoadGammaTable)(FxU32 nentries, FxU32 * red, FxU32 * green, FxU32 * blue);
typedef void (__stdcall *grSstSelect)(int which_sst);
typedef void (__stdcall *grTexDownloadMipMap)(GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo * info);
typedef void (__stdcall *grTexDownloadTable)(GrTexTable_t type, void * data);
typedef void (__stdcall *grTexSource)(GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo * info);

extern uint64_t gaEnvironmentConstantData1;
extern int gaPerspectiveImageSizeTable[14];
extern grDrawVertexArrayContiguous GLIDEDLL_grDrawVertexArrayContiguous_16;
extern grGlideInit GLIDEDLL_grGlideInit_0;
extern pfnGlideDrawVertexArray GLIDEDLL_grDrawVertexArray_12;
extern grGet GLIDEDLL_grGet_12;
extern grTexSource GLIDEDLL_grTexSource_16;
extern uint32_t gGlidePaletteEntry0[256];
extern grConstantColorValue GLIDEDLL_grConstantColorValue_4;
extern HMODULE lpGLIDE3x_dll;
extern uint32_t gbGlideIsDebugBuild;
extern int gnGlideSecondaryTMUCount[2];
extern uint32_t ghGlideWindow;
extern uint32_t gbGlideInitialized;
extern uint32_t gbPlayingCutScene;
extern int gbWINDOWED;
extern uint32_t gnGlideColorCombineMode;
extern uint32_t gnGlideAlphaBlendMode;
extern uint32_t gnGlideTextureFilteringMode;
extern uint32_t gbGlideInScene;
extern uint32_t gbGlidePerspectiveMode;
extern D2GlideVertexTexCacheBlockStrc gGlideVertex0;
extern uint32_t gaGlideColorDebugGradientIndices[1024];

namespace D2Glide::Renderer::Glide {
extern grDrawLine GLIDEDLL__grDrawLine_8;
extern grDepthMask GLIDEDLL_grDepthMask_4;
extern grDrawPoint GLIDEDLL_grDrawPoint_4;
extern grGlideShutdown GLIDEDLL_grGlideShutdown_0;
extern grDitherMode GLIDEDLL_grDitherMode_4;
extern grTexDownloadMipMap GLIDEDLL_grTexDownloadMipMap_16;
extern pfnGlideGammaCorrectionRGB GLIDEDLL_guGammaCorrectionRGB_12;
extern grLfbUnlock GLIDEDLL_grLfbUnlock_8;
extern grLfbLock GLIDEDLL_grLfbLock_24;
extern grLoadGammaTable GLIDEDLL_grLoadGammaTable_16;
extern grAlphaCombine GLIDEDLL_grAlphaCombine_20;
extern grSstWinOpen GLIDEDLL_grSstWinOpen_28;
extern grColorCombine GLIDEDLL_grColorCombine_20;
extern grColorMask GLIDEDLL_grColorMask_8;
extern grChromakeyValue GLIDEDLL_grChromakeyValue_4;
extern grCoordinateSpace GLIDEDLL_grCoordinateSpace_4;
extern grSstSelect GLIDEDLL_grSstSelect_4;
extern grTexDownloadTable GLIDEDLL_grTexDownloadTable_8;
extern grTexMaxAddress GLIDEDLL_grTexMaxAddress_4;
extern grBufferClear GLIDEDLL_grBufferClear_12;
extern grChromakeyMode GLIDEDLL_grChromakeyMode_4;
extern grBufferSwap GLIDEDLL_grBufferSwap_4;
extern uint32_t gaGlideDebugFontTexStartAddrs[96];
// lpBuffer parameter of GetCharWidthA
extern void * lpBuffer_008802c8[96];
extern void * gpGlideDebugFontTexture;
extern uint32_t gnGlideDebugFontTMU;
extern int * gpnLightmapRowPointers;
// @data gapGlideGroundStripVertexPtrs
// @wasnamed gapGlideTileVertexPtrs
// @evidence Built at runtime by 0050d646 and consumed by the loop at 0050daf8,
// which calls grDrawVertexArray(STRIP, 12, .) five times up to 00880b58. A strip table, not the
// static fan table at 0072f670. @wasnamed gapGlideTileVertexPtrs.
extern void * gapGlideGroundStripVertexPtrs[60];
extern uint8_t gbGlideGlobalLightGreen;
extern uint8_t gbGlideGlobalLightBlue;
extern uint8_t gbGlideGlobalLightRed;
extern uint8_t * gpGlideLightingTable;
extern int gnGlidePerspectiveOriginX;
extern int32_t gnGlidePerspectiveScale;
extern int gnGlidePerspectiveOriginY;
extern void * gpPerspectiveLookupTable;
extern uint8_t (*gpGlideSmackerTexBuf0)[32768];
extern uint8_t (*gpGlideSmackerTexBuf1)[32768];
extern uint32_t gnGlideSmackTex1StartAddr;
extern uint32_t gnGlideSmackTex2StartAddr;
extern uint8_t (*gpGlideSmackerTexBuf2)[32768];
extern uint32_t gnGlideTMUHandle;
} // namespace D2Glide::Renderer::Glide

namespace D2Glide::glBlocks {
extern GlSubtileCacheStrc gGlideSubtileTextureCache;
extern GlFloorCacheStrc gGlideFloorTextureCache;
} // namespace D2Glide::glBlocks
