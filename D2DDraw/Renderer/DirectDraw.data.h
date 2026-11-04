/**
 * Data defined in D2DDraw/Renderer/DirectDraw.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

// Forward declarations
struct IDirectDraw1;
struct IDirectDrawPalette;
struct IDirectDrawSurface1;

extern int gnDirectDrawPaletteSize;
extern uint32_t gnDirectDrawGamma;
extern PALETTEENTRY gaPaletteEntries[256];
extern uint32_t gdwDDrawInitFlags;
extern GUID gGlideOpenWindowConfig;
extern uint32_t gnDDrawSurfaceHeight;
extern uint32_t gnCutSceneActiveFlag;
extern IDirectDraw1 * pWindowHandleBuffer;
extern IDirectDrawSurface1 * gpDDrawPrimarySurface;
extern IDirectDrawSurface1 * gpDDrawBackBuffer;
extern IDirectDrawSurface1 * pDirectDrawSurface;
extern IDirectDrawPalette * pDirectDrawPalette;
extern uint32_t gnDDrawFlipState;
extern uint32_t gnDDrawPaletteSize;
extern uint32_t gnDDrawSurfaceXMin;
extern uint32_t gnDDrawRenderWidth;
extern uint8_t gaSmackerPaletteRemap[256];
extern void * gpSmackerImageBuffer;
// @data gbSmackerPaletteDirty
// @wasnamed gnBinkDDrawYOffset
// @evidence Set to 1 after a palette rebuild at 00513674, gates
// RENDERER_DDRAW_BlitSmackerFrame at 005136aa, cleared to 0 at 00513493. A dirty flag, not an offset.
// @wasnamed gnBinkDDrawYOffset; the genuine offset is 008816b8, computed at 005138ef as
// (WindowHeight - binkHeight) >> 1 and passed to BinkCopyToBuffer.
extern uint32_t gbSmackerPaletteDirty;
