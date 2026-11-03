/**
 * Data defined in D2Client/Renderer/Glide.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

// Forward declarations
typedef FxU32 (__stdcall *grGet)(FxU32 pname, FxU32 plength, FxI32 * params);
typedef FxU32 (__stdcall *grTexMinAddress)(GrChipID_t tmu);
typedef FxU32 (__stdcall *pfnGlideDrawVertexArray)(FxU32 mode, FxU32 Count, void * pointers);
typedef void (__stdcall *grAlphaBlendFunction)(GrAlphaBlendFnc_t rgb_sf, GrAlphaBlendFnc_t rgb_df, GrAlphaBlendFnc_t alpha_sf, GrAlphaBlendFnc_t alpha_df);
typedef void (__stdcall *grDrawVertexArrayContiguous)(FxU32 mode, FxU32 Count, void * pointers, FxU32 stride);
typedef void (__stdcall *grFinish)();
typedef void (__stdcall *grGlideInit)();
typedef void (__stdcall *grTexCombine)(GrChipID_t tmu, GrCombineFunction_t rgb_function, GrCombineFactor_t rgb_factor, GrCombineFunction_t alpha_function, GrCombineFactor_t alpha_factor, FxBool rgb_invert, FxBool alpha_invert);
typedef void (__stdcall *grTexFilterMode)(GrChipID_t tmu, GrTextureFilterMode_t minfilter_mode, GrTextureFilterMode_t magfilter_mode);

extern grDrawVertexArrayContiguous GLIDEDLL_grDrawVertexArrayContiguous_16;
extern grAlphaBlendFunction GLIDEDLL_grAlphaBlendFunction_16;
extern grTexCombine GLIDEDLL_grTexCombine_28;
extern grTexFilterMode GLIDEDLL_grTexFilterMode_12;
extern grGlideInit GLIDEDLL_grGlideInit_0;
extern grFinish GLIDEDLL_grFinish_0;
extern pfnGlideDrawVertexArray GLIDEDLL_grDrawVertexArray_12;
extern grGet GLIDEDLL_grGet_12;
extern grTexMinAddress GLIDEDLL_grTexMinAddress_4;
