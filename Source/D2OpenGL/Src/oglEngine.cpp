// Original path C:\D2\Source\D2OpenGL\Src\oglEngine.cpp. No 1.14d literal names it: the 1999 Windows
// D2OpenGL.dll (1.07 JP) pushes this __FILE__ in OpenGLDrawBox, OpenGLDrawBoxAlpha, OpenGLUtilRect and
// OpenGLUtilFilledRect, the interface slots the Mac's solid-rect and rect primitives fill; the PPC 1.10
// Beta Carbon build has the file too. The Mac links it after oglDebugFont.cpp (002dff15-002e07ec);
// see asm_matching/mac/coverage/file-evidence.tsv.


#include "Source/D2OpenGL/Src/oglEngine.h"
#include "Source/D2OpenGL/Src/oglCore.h"
#include "Source/D2OpenGL/Src/oglDebugFont.h"
#include "Source/D2CMP/Src/CelCmp.h"
#include "Source/D2CMP/Src/SubTile.h"
#include "Source/D2CMP/Src/LRUCache.h"
#include "Source/D2CMP/Src/SpriteCache.h"
#include "Source/D2Client/D2GfxDataStrc.h"
#include "Source/D2gfx/D2GfxHelperStrc.h"
#include "Source/D2Client/D2GfxSettingsStrc.h"
#include "Source/D2Client/Engine/Gfx.h"
#include "Source/D2Client/Game/Draw.h"
#include "Source/D2Common/D2TileLibraryEntryStrc.h"
#include "Source/D2Lang/D2Lang.h"
#include "Source/MacSpecific/Source/CD2Textures/CD2Textures.h"
#include "Source/D2OpenGL/Src/COGLAGPTextures.h"
#include "Source/D2OpenGL/Src/COGLTextures.h"
#include "Source/D2OpenGL/Src/globals.h"
#include "Source/D2OpenGL/Src/oglBlocks.h"
#include "Source/D2OpenGL/Src/oglPerspective.h"
#include "Source/D2OpenGL/Src/oglSmack.h"
#include "Source/D2OpenGL/Src/oglSprite.h"
#include "Source/D2OpenGL/Src/oglVertex.h"
#include "Source/D2Sound/Src/D2SoundSmp.h"
#include "Source/D2Win/DC6.h"
#include "Source/D2Win/DC6Block.h"
#include "Source/D2Win/Src/D2WinFont.h"
#include "Source/D2Win/Src/D2WinImage.h"
#include "Source/D2gfx/Src/D2GFX.h"
#include "Source/Fog/Engine/Application.h"
#include "Source/Game/Main.h"
#include "Source/Storm/Source/SMem.h"
#include "Source/Fog/Source/SFile.h"
#include "Source/Fog/Src/ErrorManager.h"
#include "Source/Fog/Src/LogManager.h"
#include "Source/Storm/Source/SFile.h"
#include "Source/D2OpenGL/Src/oglCore.h"
#include "Source/D2OpenGL/Src/BinkCutscene.h"
#include "Source/D2OpenGL/Src/oglBlocks.h"
#include "Source/D2OpenGL/Src/oglFloorCache.h"
#include "Source/D2OpenGL/Src/oglMac.h"
#include "Source/D2OpenGL/Src/oglPerspective.h"
#include "Source/D2OpenGL/Src/oglSmack.h"
#include "Source/D2OpenGL/Src/oglSprite.h"
#include "Source/D2OpenGL/Src/oglVertex.h"
#include "Source/D2OpenGL/Src/MacDisplayDevice.h"


#define DEFAULT_SECURITY_COOKIE 0xBB40E64E


#ifndef RECON_FPTD_D2OGLFrameCallbackFn
#define RECON_FPTD_D2OGLFrameCallbackFn
typedef void (*D2OGLFrameCallbackFn)();
#endif

namespace D2OpenGL::oglEngine {

// The Src files call one another unqualified.
using namespace D2OpenGL::oglCore;
using namespace D2OpenGL::BinkCutscene;
using namespace D2OpenGL::oglBlocks;
using namespace D2OpenGL::oglDebugFont;
using namespace D2OpenGL::oglEngine;
using namespace D2OpenGL::oglFloorCache;
using namespace D2OpenGL::oglMac;
using namespace D2OpenGL::oglPerspective;
using namespace D2OpenGL::oglSmack;
using namespace D2OpenGL::oglSprite;
using namespace D2OpenGL::oglVertex;
using namespace D2OpenGL::MacDisplayDevice;

// Draws a solid rectangle using Glide API with blend mode
// From Windows 1.14d Game.exe 0050bf50 (Mac pair tier T1e).
// 1.14d mac: 002dff15
void OGL_DrawSolidRectEx(int nX0, int nY0, int nX1, int nY1, BYTE nPaletteIndex, DRAWMODE nDrawMode) {
    int* piVar1 = (&INT_005e9e28);
    if (nX1 < 0) {
      nX1 = 0;
    }

    int iVar2 = INT_005e9e24 - 1;
    if (iVar2 < nX1) {
      nX1 = iVar2;
    }
    if (nX0 < 0) {
      nX0 = 0;
    }
    if (iVar2 < nX0) {
      nX0 = iVar2;
    }
    if (nX0 == nX1) {
      return;
    }
    if (nY1 < 0) {
      nY1 = 0;
    }

    iVar2 = INT_005e9e28 - 1;
    if (iVar2 < nY1) {
      nY1 = iVar2;
    }

    int iVar5 = 0;
    if (-1 < nY0) {
      iVar5 = nY0;
    }
    if (iVar2 < iVar5) {
      iVar5 = iVar2;
    }
    if (iVar5 == nY1) {
      return;
    }
    if (nY1 <= iVar5) {
      void* pvVar3;
      pvVar3 = Fog::Src::ErrorManager::GetInstructionPointer();
      fp_fatal("Unrecoverable internal error %08x line %d", pvVar3, 0x29);
    }

    OGL_SetColorCombine(2);
    OGL_SetBlendMode(4);
    iVar2 = *piVar1;
    uint32_t uVar4 = OGL_ScalePaletteValue((uint32_t)nPaletteIndex);
    GLubyte alpha = '@';
    if (nDrawMode != DRAW_VERYTRANSPARENT) {
      if (nDrawMode == DRAW_LESSTRANSPARENT) {
        alpha = 0xc0;
      } else if (nDrawMode == DRAW_TRANSPARENT) {
        alpha = 0x80;
      } else {
        alpha = 0xff;
      }
    }

    float y = (float)(iVar2 - nY1);
    float y_00 = (float)(iVar2 - iVar5);
    glColor4ub((GLubyte)uVar4, (GLubyte)(uVar4 >> 8), (GLubyte)(uVar4 >> 0x10), alpha);
    glBegin(6);
    glVertex2f((float)nX0, y_00);
    glVertex2f((float)nX0, y);
    glVertex2f((float)nX1, y);
    glVertex2f((float)nX1, y_00);
    glEnd();
}

// 1.14d mac: 002e00c0
void OGL_DrawSolidRectAlpha(int nX0, int nY0, int nX1, int nY1, BYTE nPaletteIndex, BYTE nAlpha) {
    int* piVar1 = (&INT_005e9e28);
    if (nX1 < 0) {
      nX1 = 0;
    }

    int iVar2 = INT_005e9e24 - 1;
    if (iVar2 < nX1) {
      nX1 = iVar2;
    }
    if (nX0 < 0) {
      nX0 = 0;
    }
    if (iVar2 < nX0) {
      nX0 = iVar2;
    }
    if (nX0 == nX1) {
      return;
    }
    if (nY1 < 0) {
      nY1 = 0;
    }

    iVar2 = INT_005e9e28 - 1;
    if (iVar2 < nY1) {
      nY1 = iVar2;
    }

    int iVar5 = 0;
    if (-1 < nY0) {
      iVar5 = nY0;
    }
    if (iVar2 < iVar5) {
      iVar5 = iVar2;
    }
    if (iVar5 == nY1) {
      return;
    }
    if (nY1 <= iVar5) {
      void* pvVar3;
      pvVar3 = Fog::Src::ErrorManager::GetInstructionPointer();
      fp_fatal("Unrecoverable internal error %08x line %d", pvVar3, 99);
    }

    OGL_SetColorCombine(2);
    OGL_SetBlendMode(4);
    iVar2 = *piVar1;
    uint32_t uVar4 = OGL_ScalePaletteValue((uint32_t)nPaletteIndex);
    glColor4ub((GLubyte)uVar4, (GLubyte)(uVar4 >> 8), (GLubyte)(uVar4 >> 0x10), nAlpha);
    float y = (float)(iVar2 - iVar5);
    glBegin(6);
    float y_00 = (float)(iVar2 - nY1);
    glVertex2f((float)nX0, y);
    glVertex2f((float)nX0, y_00);
    glVertex2f((float)nX1, y_00);
    glVertex2f((float)nX1, y);
    glEnd();
}

// 1.14d mac: 002e0245
void OGL_DrawLine(int nX0, int nY0, int nX1, int nY1, BYTE nPaletteIndex, BYTE nAlpha) {
    int* piVar1 = &INT_005e9e28;
    if (-1 < (nX0 & nX1) && (nX1 < INT_005e9e24 || nX0 < INT_005e9e24) && -1 < (nY0 & nY1) && (nY1 < *piVar1 || nY0 < *piVar1)) {
      uint32_t uColor = OGL_ScalePaletteValue((uint32_t)nPaletteIndex);
      OGL_SetColorCombine(2);
      if (nAlpha == 0xff) {
        OGL_SetBlendMode(1);
      } else {
        OGL_SetBlendMode(4);
      }

      glColor4ub((GLubyte)uColor, (GLubyte)(uColor >> 8), (GLubyte)(uColor >> 0x10), nAlpha);
      GLfloat x = (GLfloat)nX0;
      GLfloat y;
      if (nX0 == nX1 && nY0 == nY1) {
        y = (float)*piVar1 - (float)nY0;
        glBegin(0);
      } else {
        float fHeight = (float)*piVar1;
        glBegin(1);
        y = fHeight - (float)nY1;
        glVertex2f(x, fHeight - (float)nY0);
        x = (GLfloat)nX1;
      }

      glVertex2f(x, y);
      glEnd();
    }
}

// 1.14d mac: 002e03c0
void OGL_DrawRect(RECT * pRect, BYTE nPaletteIndex) {
    void* pvVar1;
    int nRectLeft;
    if (pRect->left > pRect->right) {
      void* nErrorAddr;
      nErrorAddr = Fog::Src::ErrorManager::GetInstructionPointer();
      fp_fatal("Unrecoverable internal error %08x line %d", nErrorAddr, 0xb6);
    }

    int nRectTop = pRect->top;
    int nYEnd = pRect->bottom;
    if (nRectTop > nYEnd) {
      pvVar1 = Fog::Src::ErrorManager::GetInstructionPointer();
      fp_fatal("Unrecoverable internal error %08x line %d", pvVar1, 0xb7);
      nRectTop = pRect->top;
      nYEnd = pRect->bottom;
    }

    int nX1 = pRect->left;
    int nXEnd = pRect->right;
    if (!(nXEnd - nX1 && nYEnd - nRectTop && -1 < nXEnd && nX1 <= INT_005e9e24 && -1 < nYEnd && nRectTop <= INT_005e9e28)) {
      return;
    }

    int nYEnd_00 = ((uint32_t)(nYEnd - nRectTop) >> 1) + nRectTop;
    int nXStart = ((uint32_t)(nXEnd - nX1) >> 1) + nX1;
    OGL_DrawLine(nXStart, nRectTop, nXEnd, nYEnd_00, nPaletteIndex, 0xff);
    OGL_DrawLine(nXEnd, nYEnd_00, nXStart, nYEnd, nPaletteIndex, 0xff);
    OGL_DrawLine(nXStart, nYEnd, nX1, nYEnd_00, nPaletteIndex, 0xff);
    OGL_DrawLine(nX1, nYEnd_00, nXStart, nRectTop, nPaletteIndex, 0xff);
}

// 1.14d mac: 002e0534
void OGL_DrawRectEx(RECT * pRect, BYTE nPaletteIndex) {
    void* pvVar1;
    if (pRect->left > pRect->right) {
      pvVar1 = Fog::Src::ErrorManager::GetInstructionPointer();
      fp_fatal("Unrecoverable internal error %08x line %d", pvVar1, 0xdc);
    }
    if (pRect->top > pRect->bottom) {
      pvVar1 = Fog::Src::ErrorManager::GetInstructionPointer();
      fp_fatal("Unrecoverable internal error %08x line %d", pvVar1, 0xdd);
    }

    int nX0 = pRect->left;
    int nX1 = pRect->right;
    if (nX1 < 0) {
      nX1 = 0;
    }

    int iVar2 = INT_005e9e24 - 1;
    if (nX1 > iVar2) {
      nX1 = iVar2;
    }
    if (nX0 < 0) {
      nX0 = 0;
    }
    if (nX0 > iVar2) {
      nX0 = iVar2;
    }
    if (nX0 == nX1) {
      return;
    }

    iVar2 = pRect->bottom;
    if (iVar2 < 0) {
      iVar2 = 0;
    }

    int iVar3 = INT_005e9e28 - 1;
    if (iVar2 > iVar3) {
      iVar2 = iVar3;
    }

    int nY0 = 0;
    if (pRect->top >= 0) {
      nY0 = pRect->top;
    }
    if (nY0 > iVar3) {
      nY0 = iVar3;
    }
    if (nY0 == iVar2) {
      return;
    }
    if (iVar2 <= nY0) {
      pvVar1 = Fog::Src::ErrorManager::GetInstructionPointer();
      fp_fatal("Unrecoverable internal error %08x line %d", pvVar1, 0xed);
    }

    OGL_DrawLine(nX0, nY0, nX1, nY0, nPaletteIndex, 0xff);
    OGL_DrawLine(nX1, nY0, nX1, iVar2, nPaletteIndex, 0xff);
    OGL_DrawLine(nX1, iVar2, nX0, iVar2, nPaletteIndex, 0xff);
    OGL_DrawLine(nX0, iVar2, nX0, nY0, nPaletteIndex, 0xff);
}

// 1.14d mac: 002e06bd
void OGL_DrawSolidRect(RECT * pRect, BYTE nPaletteIndex) {
    void* pvVar1;
    if (pRect->left > pRect->right) {
      void* nErrorAddr;
      nErrorAddr = Fog::Src::ErrorManager::GetInstructionPointer();
      fp_fatal("Unrecoverable internal error %08x line %d", nErrorAddr, 0xfd);
    }

    int nY0 = pRect->top;
    int nY1 = pRect->bottom;
    if (nY0 > nY1) {
      pvVar1 = Fog::Src::ErrorManager::GetInstructionPointer();
      fp_fatal("Unrecoverable internal error %08x line %d", pvVar1, 0xfe);
      nY0 = pRect->top;
      nY1 = pRect->bottom;
    }

    OGL_DrawSolidRectEx(pRect->left, nY0, pRect->right, nY1, nPaletteIndex, DRAW_SOLID);
}

// 1.14d mac: 002e0757
void OGL_DrawSolidSquare(POINT * pPoint, BYTE nSize, BYTE nPaletteIndex) {
    // Frame slots -36..-20 are ONE object: &local_24 is passed out and the callee reaches the rest by byte offset.
    // Separate locals let the compiler reorder them, so the run is one struct.
    struct __frame0_t {
      int local_24;
      int local_20;
      int local_1c;
      int local_18;
    };
    static_assert(sizeof(void*) != 4 || sizeof(__frame0_t) == 16, "frame group __frame0_t must reproduce the Ghidra frame layout");
    __frame0_t __frame0;
    if (nSize == '\0') {
      return;
    }

    uint32_t nHalfSize = (uint32_t)(uint8_t)(nSize - 1);
    SetRect(&__frame0.local_24, pPoint->x - nHalfSize, pPoint->y - nHalfSize, nHalfSize + 1 + pPoint->x, nHalfSize + 1 + pPoint->y);
    OGL_DrawSolidRectEx(__frame0.local_24, __frame0.local_20, __frame0.local_1c, __frame0.local_18, nPaletteIndex, DRAW_SOLID);
}

// 1.14d mac: 002e07d5
void OGL_ClearScreen(BOOL bPartial) {
    glClear(0x4000);
}

} // namespace D2OpenGL::oglEngine
