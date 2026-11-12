/**
 * Data defined in D2gfx/D2GfxHelperStrc.cpp that other translation units name.
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

// Render buffer pitch (bytes per scanline) for tile drawing. Set by fpFillYBufferTable from nWidth parameter. Used by fpDrawVisTile, fpDrawBlendedVisTile, fpDrawRoofTile, fpDrawGroundTile to calculate Y-offsets when blitting tile data to framebuffer.
extern int32_t gRenderBufferPitch;
