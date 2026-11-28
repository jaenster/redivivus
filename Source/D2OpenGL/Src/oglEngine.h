#pragma once

#include "Source/D2Headers/d2_platform.h"
#include "Source/D2OpenGL/Src/placeholder_types.h"
#include "Source/D2OpenGL/Src/MacSdkOnWin32.h"
#include "Source/D2OpenGL/Src/oglCore.h"

namespace D2OpenGL::oglEngine {

// Function declarations
// Draws a solid rectangle using Glide API with blend mode
// From Windows 1.14d Game.exe 0050bf50 (Mac pair tier T1e).
void OGL_DrawSolidRectEx(int nX0, int nY0, int nX1, int nY1, BYTE nPaletteIndex, DRAWMODE nDrawMode); // 1.14d mac: 002dff15
void OGL_DrawSolidRectAlpha(int nX0, int nY0, int nX1, int nY1, BYTE nPaletteIndex, BYTE nAlpha); // 1.14d mac: 002e00c0
void OGL_DrawLine(int nX0, int nY0, int nX1, int nY1, BYTE nPaletteIndex, BYTE nAlpha); // 1.14d mac: 002e0245
void OGL_DrawRect(RECT * pRect, BYTE nPaletteIndex); // 1.14d mac: 002e03c0
void OGL_DrawRectEx(RECT * pRect, BYTE nPaletteIndex); // 1.14d mac: 002e0534
void OGL_DrawSolidRect(RECT * pRect, BYTE nPaletteIndex); // 1.14d mac: 002e06bd
void OGL_DrawSolidSquare(POINT * pPoint, BYTE nSize, BYTE nPaletteIndex); // 1.14d mac: 002e0757
void OGL_ClearScreen(BOOL bPartial); // 1.14d mac: 002e07d5

} // namespace D2OpenGL::oglEngine
