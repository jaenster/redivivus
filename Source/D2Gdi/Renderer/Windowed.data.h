/**
 * Data defined in D2Gdi/Renderer/Windowed.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "globals/globals.h"

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
