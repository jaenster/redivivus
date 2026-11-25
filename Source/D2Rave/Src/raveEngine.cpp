// raveEngine.cpp - screen clear, rectangles and lines as RAVE Gouraud primitives.
//
// File name inferred: the Windows renderers keep DrawRect* in glEngine.cpp/d3dEngine.cpp (assert
// oracle). The unit runs 102b4878-102b5614 and ends with __sinit_raveEngine_cpp at 102b5608.
// RAVE's y axis points up: every primitive flips y against the screen height.

#include "Source/D2Rave/Src/raveInternal.h"

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

namespace D2Rave {

namespace {
const float sgfRaveZ = 0.0f;          // 1037c7cc
const float sgfRaveInvW = 1.0f;       // 1037c7c8
const float sgfRaveAlphaZ = 0.25f;    // 1037c7b0
const double sgdRaveColorScale = 255.0;  // 1037c7d0

// Ours: fills one vertex; the binary stores the eight floats in place.
inline void sRaveSetVertex(TQAVGouraud & v, float x, float y, float z, float r, float g, float b, float a) {
    v.x = x;
    v.y = y;
    v.z = z;
    v.invW = sgfRaveInvW;
    v.r = r;
    v.g = g;
    v.b = b;
    v.a = a;
}

inline float sRaveColorComponent(uint32_t nValue) {
    return (float)((double)nValue / sgdRaveColorScale);
}
} // namespace

// Windows slot 49. Without fast textures the clear is a full-screen black rectangle.
// mac ppc classic/1.09d: 102b4878 (1.00: 1007a9cc)
void RaveClearScreen(int bClear) {
    (void)bClear;
    if (gbRaveFastTexture == 0) {
        RaveDrawSolidRectEx(0, 0, gnRaveScreenWidth, gnRaveScreenHeight, 0, 5);
    } else {
        gpRaveDrawContext->clearDrawBuffer(gpRaveDrawContext, nullptr, nullptr);
    }
}

// Windows slot 45: a square of side 2*nSize-1 centred on the point.
// mac ppc classic/1.09d: 102b48e8 (1.00: 1007aa4c)
void RaveDrawSolidSquare(const int * pPoint, uint8_t nSize, uint8_t nColor) {
    if (nSize == 0) {
        return;
    }
    const unsigned int nHalf = (uint8_t)(nSize - 1);
    tagRECT rect;
    SetRect(&rect, pPoint[0] - nHalf, pPoint[1] - nHalf, pPoint[0] + nHalf + 1, pPoint[1] + nHalf + 1);
    RaveDrawSolidRectEx(rect.left, rect.top, rect.right, rect.bottom, nColor, 5);
}

// Windows slot 44.
// mac ppc classic/1.09d: 102b4968 (1.00: 1007aacc)
void RaveDrawSolidRect(const int * pRect, uint8_t nColor) {
    RaveDrawSolidRectEx(pRect[0], pRect[1], pRect[2], pRect[3], nColor, 5);
}

// Windows slot 43: the outline of a rectangle clamped to the screen.
// mac ppc classic/1.09d: 102b49a4 (1.00: 1007ab08)
void RaveDrawRectEx(const int * pRect, uint8_t nColor) {
    int nWidth = gnRaveScreenWidth;
    int nLeft = pRect[0] < 1 ? 0 : pRect[0];
    if (nLeft >= nWidth) {
        nLeft = nWidth;
    }
    int nRight = pRect[2] < 1 ? 0 : pRect[2];
    if (nRight < nWidth) {
        nWidth = nRight;
    }
    nRight = nWidth;
    if (nLeft == nRight) {
        return;
    }
    int nHeight = gnRaveScreenHeight;
    int nTop = pRect[1] < 1 ? 0 : pRect[1];
    if (nTop >= nHeight) {
        nTop = nHeight;
    }
    int nBottom = pRect[3] < 1 ? 0 : pRect[3];
    if (nBottom < nHeight) {
        nHeight = nBottom;
    }
    nBottom = nHeight;
    if (nTop == nBottom) {
        return;
    }
    RaveDrawLine(nLeft, nTop, nRight, nTop, nColor, 0xff);
    RaveDrawLine(nRight, nTop, nRight, nBottom, nColor, 0xff);
    RaveDrawLine(nRight, nBottom, nLeft, nBottom, nColor, 0xff);
    RaveDrawLine(nLeft, nBottom, nLeft, nTop, nColor, 0xff);
}

// Windows slot 42: the diamond inscribed in the rectangle (the isometric tile outline).
// mac ppc classic/1.09d: 102b4afc (1.00: 1007ac68)
void RaveDrawRect(const int * pRect, uint8_t nColor) {
    const int nLeft = pRect[0];
    const int nTop = pRect[1];
    const unsigned int nWidth = pRect[2] - nLeft;
    const unsigned int nHeight = pRect[3] - nTop;
    if (nWidth == 0 || nHeight == 0) {
        return;
    }
    const int nRight = nLeft + nWidth;
    const int nMidX = nLeft + (nWidth >> 1);
    const int nMidY = nTop + (nHeight >> 1);
    const int nBottom = nTop + nHeight;
    if (nRight > -1 && nLeft <= gnRaveScreenWidth && nBottom > -1 && nTop <= gnRaveScreenHeight) {
        RaveDrawLine(nMidX, nTop, nRight, nMidY, nColor, 0xff);
        RaveDrawLine(nRight, nMidY, nMidX, nBottom, nColor, 0xff);
        RaveDrawLine(nMidX, nBottom, nLeft, nMidY, nColor, 0xff);
        RaveDrawLine(nLeft, nMidY, nMidX, nTop, nColor, 0xff);
    }
}

// Windows slot 48. A zero-length line is drawn as a point; others go through the clipper.
// mac ppc classic/1.09d: 102b4c08 (1.00: 1007ad74)
void RaveDrawLine(int nX1, int nY1, int nX2, int nY2, uint8_t nColor, uint8_t nAlpha) {
    const int nHeight = gnRaveScreenHeight;
    const int nFlipY1 = (nHeight - nY1) + 1;
    const int nFlipY2 = (nHeight - nY2) + 1;
    if ((nX2 > -1 || nX1 > -1) && (nX2 < gnRaveScreenWidth || nX1 <= gnRaveScreenWidth) &&
        (nFlipY2 > -1 || nFlipY1 > -1) && (nFlipY2 <= nHeight || nFlipY1 <= nHeight)) {
        const uint32_t dwColor = sRaveGetPaletteARGB32(nColor);
        sRaveSetAlphaState(2);
        sRaveSetBlendMode(nAlpha == 0xff ? 1 : 4);
        sRaveSetTextureFilter(1);
        const float r = sRaveColorComponent((dwColor >> 16) & 0xff);
        const float g = sRaveColorComponent((dwColor >> 8) & 0xff);
        const float b = sRaveColorComponent(dwColor & 0xff);
        const float a = sRaveColorComponent(nAlpha);
        if (nX1 == nX2 && nFlipY1 == nFlipY2) {
            TQAVGouraud v;
            sRaveSetVertex(v, (float)(nX1 - 1), (float)(gnRaveScreenHeight - nFlipY1), sgfRaveZ, r, g, b, a);
            gpRaveDrawContext->drawPoint(gpRaveDrawContext, &v);
        } else {
            TQAVGouraud v[2];
            sRaveSetVertex(v[0], (float)(nX1 - 1), (float)(gnRaveScreenHeight - nFlipY1), sgfRaveZ, r, g, b, a);
            sRaveSetVertex(v[1], (float)(nX2 - 1), (float)(gnRaveScreenHeight - nFlipY2), sgfRaveZ, r, g, b, a);
            QADrawClippedLine(gpRaveDrawContext, v);
        }
    }
}

// Windows slot 47: an alpha-blended rectangle at z 0.25.
// mac ppc classic/1.09d: 102b4eb0 (1.00: 1007b00c)
void RaveDrawSolidRectAlpha(int nX1, int nY1, int nX2, int nY2, uint8_t nColor, uint32_t nAlpha) {
    int nWidth = gnRaveScreenWidth;
    int nLeft = nX1 < 1 ? 0 : nX1;
    if (nLeft >= nWidth) {
        nLeft = nWidth;
    }
    int nRight = nX2 < 1 ? 0 : nX2;
    if (nRight < nWidth) {
        nWidth = nRight;
    }
    nRight = nWidth;
    if (nLeft == nRight) {
        return;
    }
    int nHeight = gnRaveScreenHeight;
    int nTop = nY1 - 1 < 1 ? 0 : nY1 - 1;
    if (nTop >= nHeight) {
        nTop = nHeight;
    }
    int nBottom = nY2 - 1 < 1 ? 0 : nY2 - 1;
    if (nBottom < nHeight) {
        nHeight = nBottom;
    }
    nBottom = nHeight;
    if (nTop == nBottom) {
        return;
    }
    sRaveSetAlphaState(2);
    sRaveSetBlendMode(4);
    sRaveSetTextureFilter(1);
    const float fH = (float)gnRaveScreenHeight;
    const float fTop = (float)(gnRaveScreenHeight - nTop);
    const float fBottom = (float)(gnRaveScreenHeight - nBottom);
    const uint32_t dwColor = sRaveGetPaletteARGB32(nColor);
    const float a = sRaveColorComponent(nAlpha & 0xff);
    const float r = sRaveColorComponent((dwColor >> 16) & 0xff);
    const float g = sRaveColorComponent((dwColor >> 8) & 0xff);
    const float b = sRaveColorComponent(dwColor & 0xff);
    TQAVGouraud v[4];
    sRaveSetVertex(v[0], (float)nLeft - sgfRaveInvW, fH - fTop, sgfRaveAlphaZ, r, g, b, a);
    sRaveSetVertex(v[1], (float)nLeft - sgfRaveInvW, fH - fBottom, sgfRaveAlphaZ, r, g, b, a);
    sRaveSetVertex(v[2], (float)nRight - sgfRaveInvW, fH - fBottom, sgfRaveAlphaZ, r, g, b, a);
    sRaveSetVertex(v[3], (float)nRight - sgfRaveInvW, fH - fTop, sgfRaveAlphaZ, r, g, b, a);
    gpRaveDrawContext->drawVGouraud(gpRaveDrawContext, 4, kQAVertexMode_Fan, v, nullptr);
}

// Windows slot 46: a rectangle whose alpha comes from the draw mode (0 25%, 1 50%, 2 75%,
// otherwise opaque).
// mac ppc classic/1.09d: 102b5234 (1.00: 1007b398)
void RaveDrawSolidRectEx(int nX1, int nY1, int nX2, int nY2, uint8_t nColor, int nDrawMode) {
    int nWidth = gnRaveScreenWidth;
    int nLeft = nX1 < 1 ? 0 : nX1;
    if (nLeft >= nWidth) {
        nLeft = nWidth;
    }
    int nRight = nX2 < 1 ? 0 : nX2;
    if (nRight < nWidth) {
        nWidth = nRight;
    }
    nRight = nWidth;
    if (nLeft == nRight) {
        return;
    }
    int nHeight = gnRaveScreenHeight;
    int nTop = nY1 - 1 < 1 ? 0 : nY1 - 1;
    if (nTop >= nHeight) {
        nTop = nHeight;
    }
    int nBottom = nY2 - 1 < 1 ? 0 : nY2 - 1;
    if (nBottom < nHeight) {
        nHeight = nBottom;
    }
    nBottom = nHeight;
    if (nTop == nBottom) {
        return;
    }
    sRaveSetAlphaState(2);
    sRaveSetBlendMode(4);
    sRaveSetTextureFilter(1);
    const float fTop = (float)(gnRaveScreenHeight - nTop);
    const float fBottom = (float)(gnRaveScreenHeight - nBottom);
    const uint32_t dwColor = sRaveGetPaletteARGB32(nColor);
    uint32_t nAlpha;
    if (nDrawMode == 1) {
        nAlpha = 0x80;
    } else if (nDrawMode == 0) {
        nAlpha = 0x40;
    } else if (nDrawMode == 2) {
        nAlpha = 0xc0;
    } else {
        nAlpha = 0xff;
    }
    const float fH = (float)gnRaveScreenHeight;
    const float a = sRaveColorComponent(nAlpha);
    const float r = sRaveColorComponent((dwColor >> 16) & 0xff);
    const float g = sRaveColorComponent((dwColor >> 8) & 0xff);
    const float b = sRaveColorComponent(dwColor & 0xff);
    TQAVGouraud v[4];
    sRaveSetVertex(v[0], (float)nLeft - sgfRaveInvW, fH - fTop, sgfRaveZ, r, g, b, a);
    sRaveSetVertex(v[1], (float)nLeft - sgfRaveInvW, fH - fBottom, sgfRaveZ, r, g, b, a);
    sRaveSetVertex(v[2], (float)nRight - sgfRaveInvW, fH - fBottom, sgfRaveZ, r, g, b, a);
    sRaveSetVertex(v[3], (float)nRight - sgfRaveInvW, fH - fTop, sgfRaveZ, r, g, b, a);
    gpRaveDrawContext->drawVGouraud(gpRaveDrawContext, 4, kQAVertexMode_Fan, v, nullptr);
}

// Reads the D2Gfx settings dword at +0x14 (the caller is in raveFloor.cpp).
// mac ppc classic/1.09d: 102b55f8 (1.00: 1007b764)
int sRaveGetSettings14() {
    return *(int *)((uint8_t *)gpRaveSettings + 0x14);
}

} // namespace D2Rave

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
