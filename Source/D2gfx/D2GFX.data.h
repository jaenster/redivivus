/**
 * Data defined in D2gfx/D2GFX.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2gfx)
#error "D2gfx header included by a module that may not use D2gfx (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"
#include "Source/globals/globals.h"
#include "Source/D2Client/D2GfxSettingsStrc.h"
#include "Source/D2Direct3D/Renderer/Direct3D.h"
#include "Source/D2Direct3D/d3dFloorCache.h"
#include "Source/D2Direct3D/d3dSubtileCache.h"
#include "Source/D2Direct3D/d3dTextureCache.h"
#include "Source/D2Glide/Renderer/Glide.h"
#include "Source/D2Glide/glFloorCache.h"
#include "Source/D2Glide/glSubtileCache.h"
#include "Source/D2Hell/D2Hell.h"
#include "Source/D2gfx/D2GFX.h"
#include "Source/Fog/D2MemoryManagerStrc.h"
#include "Source/Fog/D2PoolManagerStrc.h"
#include "Source/Fog/D2QServerStrc.h"
#include "Source/Fog/Memory.h"
#include "Source/Fog/Save.h"
#include "Source/Fog/Source/SFile.h"
#include "Source/Fog/Source/SSComp.h"
#include "Source/Fog/Src/LogManager.h"
#include "3rdparty/Storm/3rdparty/zlib/deflate.h"
#include "3rdparty/Storm/H/stpl.h"
#include "3rdparty/Storm/Source/SCMD.h"
#include "3rdparty/Storm/Source/SComp.h"
#include "3rdparty/Storm/Source/SErr.h"
#include "3rdparty/Storm/Source/SMem.h"
#include "3rdparty/Storm/Storm.h"
#include "Source/D2Headers/d2_platform.h"
#include "Source/D2Client/D2GfxSettingsStrc.h"

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
