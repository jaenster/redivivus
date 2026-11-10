/**
 * Data defined in D2gfx/D2GFX.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2gfx)
#error "D2gfx header included by a module that may not use D2gfx (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2gfx) && !defined(D2_LOADS_D2gfx)
#error "internal D2gfx header included from another module; include D2gfx's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations (used only by pointer here)
struct D2RenderStatisticsStrc;

// Forward declarations
struct D2GfxSettingsStrc;
struct D2RendererCoreStrc;
struct D2RendererFunctionsStrc;

extern D2GfxSettingsStrc D2GFXRenderedData;
extern uint32_t gnD2GFXResolutionMode;
extern D2RendererFunctionsStrc * RENDERER_CurrentRenderedFunctions;
extern uint8_t gaDirectDrawDitherPaletteRemap[1024];
extern D2RenderStatisticsStrc gnTextureMemoryUsageBytes;
extern D2RendererCoreStrc * RENDERER_SelectedRendererCore;
extern HWND gnGlideWindowHandle;
extern int gnRenderState;
extern D2GfxSettingsStrc * pD2GFXRenderedDataStrc;
extern BYTE * * gpPaletteTable;
// ppvBits parameter of CreateDIBSection
extern BYTE * gpRenderBackBufferBits;
// struct tagRECT rInitialWindowRect
// lpRect parameter of GetWindowRect
extern tagRECT rInitialWindowRect;
extern uint32_t D2GFX_EndCutScene;
