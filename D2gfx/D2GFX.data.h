/**
 * Data defined in D2gfx/D2GFX.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "globals/globals.h"
#include "D2Client/D2GfxSettingsStrc.h"

// Forward declarations
struct D2GfxSettingsStrc;
struct D2RendererCoreStrc;
struct D2RendererFunctionsStrc;

extern D2GfxSettingsStrc D2GFXRenderedData;
extern HINSTANCE hInstance;
extern uint32_t nResolutionMode;
extern D2RendererFunctionsStrc * RENDERER_CurrentRenderedFunctions;
extern uint8_t gaDirectDrawDitherPaletteRemap[1024];
extern D2RenderStatisticsStrc gnTextureMemoryUsageBytes;
extern D2RendererCoreStrc * RENDERER_SelectedRendererCore;
extern int gnGlideWindowHandle;
extern int gnRenderState;
extern D2GfxSettingsStrc * pD2GFXRenderedDataStrc;
extern BYTE * * gpPaletteTable;
// ppvBits parameter of CreateDIBSection
extern BYTE * gpRenderBackBufferBits;
// struct tagRECT rInitialWindowRect
// lpRect parameter of GetWindowRect
extern tagRECT rInitialWindowRect;
extern uint32_t D2GFX_EndCutScene;
