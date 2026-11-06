/**
 * Data defined in D2Gdi/Renderer/Windowed.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Gdi)
#error "D2Gdi header included by a module that may not use D2Gdi (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"
#include "Source/D2CMP/Draw.h"

// Forward declarations (used only by pointer here)
struct D2SmackerGlideBufferStrc;

// prgbq parameter of SetDIBColorTable
extern RGBQUAD prgbq_00989c40[256];
extern BYTE * gpGlidePaletteTableData[72];
extern HPALETTE RENDERER_WINDOWED_hPalette;
extern HBITMAP RENDERER_WINDOWED_hDIBitmap;
extern uint32_t RENDERER_WINDOWED_nScaleOffsetY;
extern uint32_t RENDERER_WINDOWED_nScaleWidth;
extern HFONT RENDERER_WINDOWED_hDebugFont;
extern D2SmackerGlideBufferStrc gnSmackerGlideYOffset;
extern uint32_t gnSmackerDecodeState;
