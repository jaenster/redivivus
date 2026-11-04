/**
 * Data defined in D2gfx/D2GfxHelperStrc.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

// Render buffer pitch (bytes per scanline) for tile drawing. Set by fpFillYBufferTable from nWidth parameter. Used by fpDrawVisTile, fpDrawBlendedVisTile, fpDrawRoofTile, fpDrawGroundTile to calculate Y-offsets when blitting tile data to framebuffer.
extern int32_t gRenderBufferPitch;
