/**
 * Data defined in D2Glide/Renderer/Glide.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "D2Client/Renderer/Glide.h"

// Forward declarations
struct D2GlideVertexTexCacheBlockStrc;
typedef FxBool (__stdcall *grSstWinClose)(GrContext_t context);
typedef void (__stdcall *grConstantColorValue)(GrColor_t value);
typedef void (__stdcall *grTexSource)(GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo * info);
typedef void (__stdcall *grVertexLayout)(FxU32 param, FxI32 offset, FxU32 mode);

extern uint64_t gaEnvironmentConstantData1;
extern int gaPerspectiveImageSizeTable[14];
extern grTexSource GLIDEDLL_grTexSource_16;
extern grVertexLayout GLIDEDLL_grVertexLayout_12;
extern uint32_t gGlidePaletteEntry0[256];
extern grConstantColorValue GLIDEDLL_grConstantColorValue_4;
extern grSstWinClose _GLIDEDLL_grSstWinClose_4;
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
