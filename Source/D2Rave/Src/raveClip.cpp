// raveClip.cpp - clipping RAVE primitives to the screen before drawing them.
//
// File name inferred; QADrawClippedVTexture is the name its own log string uses
// ("QADrawClippedVTexture: *** Can't clip ve..."). The unit runs 102bc040-102bd068 and ends with
// __sinit_raveClip_cpp at 102bd05c. The clip rectangle is gRaveClipRect (pixel centres, set by
// sInitRaveContext); the four edge tests follow it in data as TVectors (102bcc58, 102bcc98,
// 102bcc38, 102bcc78).
//
// Reconstructed here: the line path (QADrawClippedLine and its reject test), the texture
// reject test, the four edge intersectors and the four inside tests. Not yet:
// QADrawClippedVTexture (102bc040) and sQAClipPolygon (102bc594), whose pass tables pair the
// inside tests with the intersectors; see d2-surveys/macppc/d2rave/recon.md.

#include "Source/D2Rave/Src/raveInternal.h"

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

namespace D2Rave {

namespace {
inline float sRaveClipTop() { return gRaveClipRect[0]; }     // 103f2c64
inline float sRaveClipLeft() { return gRaveClipRect[1]; }    // 103f2c68
inline float sRaveClipBottom() { return gRaveClipRect[2]; }  // 103f2c6c
inline float sRaveClipRight() { return gRaveClipRect[3]; }   // 103f2c70

const float sgfRaveClipZ = 0.0f;     // 1037c8f4
const float sgfRaveClipInvW = 1.0f;  // 1037c8f0
const double sgdRaveClipOne = 1.0;   // 1037c8f8

// Ours: the shared tail of the four intersectors - z and invW reset, and the nine colour and
// texture floats (r, g, b, a, uOverW, vOverW, kd_r, kd_g, kd_b) interpolated at t.
inline void sQALerpAttributes(const TQAVTexture * pA, const TQAVTexture * pB, float fT, TQAVTexture * pOut) {
    const float fS = (float)(sgdRaveClipOne - fT);
    pOut->z = sgfRaveClipZ;
    pOut->invW = sgfRaveClipInvW;
    const float * pFromA = &pA->r;
    const float * pFromB = &pB->r;
    float * pTo = &pOut->r;
    for (int i = 0; i < 9; ++i) {
        pTo[i] = fS * pFromA[i] + fT * pFromB[i];
    }
}

// Ours: where the edge a-b crosses a vertical clip line.
inline void sQAClipAtX(const TQAVTexture * pA, const TQAVTexture * pB, float fEdge, TQAVTexture * pOut) {
    const float fDX = pB->x - pA->x;
    const float fSlope = (pB->y - pA->y) / fDX;
    pOut->x = fEdge;
    const float fT = (fEdge - pA->x) / fDX;
    pOut->y = fSlope * (fEdge - pA->x) + pA->y;
    sQALerpAttributes(pA, pB, fT, pOut);
}

// Ours: where the edge a-b crosses a horizontal clip line.
inline void sQAClipAtY(const TQAVTexture * pA, const TQAVTexture * pB, float fEdge, TQAVTexture * pOut) {
    const float fDY = pB->y - pA->y;
    const float fSlope = (pB->x - pA->x) / fDY;
    const float fT = (fEdge - pA->y) / fDY;
    pOut->x = fSlope * (fEdge - pA->y) + pA->x;
    pOut->y = fEdge;
    sQALerpAttributes(pA, pB, fT, pOut);
}
} // namespace

// The crossing with the right edge. Asm only in the database.
// mac ppc classic/1.09d: 102bc7f0 (no 1.00 pair)
void sQAClipEdgeA(const TQAVTexture * pA, const TQAVTexture * pB, TQAVTexture * pOut) {
    sQAClipAtX(pA, pB, sRaveClipRight(), pOut);
}

// The crossing with the left edge. Asm only in the database.
// mac ppc classic/1.09d: 102bc904 (no 1.00 pair)
void sQAClipEdgeB(const TQAVTexture * pA, const TQAVTexture * pB, TQAVTexture * pOut) {
    sQAClipAtX(pA, pB, sRaveClipLeft(), pOut);
}

// The crossing with the bottom edge. Asm only in the database.
// mac ppc classic/1.09d: 102bca18 (no 1.00 pair)
void sQAClipEdgeC(const TQAVTexture * pA, const TQAVTexture * pB, TQAVTexture * pOut) {
    sQAClipAtY(pA, pB, sRaveClipBottom(), pOut);
}

// The crossing with the top edge. Asm only in the database.
// mac ppc classic/1.09d: 102bcb28 (no 1.00 pair)
void sQAClipEdgeD(const TQAVTexture * pA, const TQAVTexture * pB, TQAVTexture * pOut) {
    sQAClipAtY(pA, pB, sRaveClipTop(), pOut);
}

// Inside the bottom edge. Asm only in the database.
// mac ppc classic/1.09d: 102bcc38 (no 1.00 pair)
int sQAInsideA(const TQAVTexture * pVertex) {
    return pVertex->y <= sRaveClipBottom();
}

// Inside the top edge. Asm only in the database.
// mac ppc classic/1.09d: 102bcc58 (no 1.00 pair)
int sQAInsideB(const TQAVTexture * pVertex) {
    return pVertex->y >= sRaveClipTop();
}

// Inside the right edge. Asm only in the database.
// mac ppc classic/1.09d: 102bcc78 (no 1.00 pair)
int sQAInsideC(const TQAVTexture * pVertex) {
    return pVertex->x <= sRaveClipRight();
}

// Inside the left edge. Asm only in the database.
// mac ppc classic/1.09d: 102bcc98 (no 1.00 pair)
int sQAInsideD(const TQAVTexture * pVertex) {
    return pVertex->x >= sRaveClipLeft();
}

// Clips both ends of a line to the clip rectangle along the line's slope, then draws it;
// nothing is drawn when both ends lie beyond the same edge.
// mac ppc classic/1.09d: 102bccb8 (1.00: 10082aa0)
void QADrawClippedLine(TQADrawContext * pContext, TQAVGouraud * pVertices) {
    TQAVGouraud * pV0 = &pVertices[0];
    TQAVGouraud * pV1 = &pVertices[1];
    const float fSlopeYX = (pV1->y - pV0->y) / (pV1->x - pV0->x);
    const float fSlopeXY = (pV1->x - pV0->x) / (pV1->y - pV0->y);
    if (sQAClipLineCode(pVertices, 2)) {
        return;
    }
    TQAVGouraud * const pEnds[2] = {pV0, pV1};
    for (TQAVGouraud * pV : pEnds) {
        const float fX = pV->x;
        if (fX < sRaveClipLeft()) {
            pV->y = fSlopeYX * (sRaveClipLeft() - fX) + pV->y;
            pV->x = sRaveClipLeft();
        } else if (sRaveClipRight() < fX) {
            pV->y = fSlopeYX * (sRaveClipRight() - fX) + pV->y;
            pV->x = sRaveClipRight();
        }
        const float fY = pV->y;
        if (fY < sRaveClipTop()) {
            pV->x = fSlopeXY * (sRaveClipTop() - fY) + pV->x;
            pV->y = sRaveClipTop();
        } else if (sRaveClipBottom() < fY) {
            pV->x = fSlopeXY * (sRaveClipBottom() - fY) + pV->x;
            pV->y = sRaveClipBottom();
        }
    }
    pContext->drawLine(pContext, pV0, pV1);
}

// 1 when every Gouraud vertex lies beyond one and the same clip edge (trivial reject).
// mac ppc classic/1.09d: 102bce94 (1.00: 10082c7c)
int sQAClipLineCode(const TQAVGouraud * pVertices, int nVertices) {
    int n;
    for (n = 0; n < nVertices && pVertices[n].x < sRaveClipLeft(); ++n) {
    }
    if (n == nVertices) {
        return 1;
    }
    for (n = 0; n < nVertices && pVertices[n].x > sRaveClipRight(); ++n) {
    }
    if (n == nVertices) {
        return 1;
    }
    for (n = 0; n < nVertices && pVertices[n].y < sRaveClipTop(); ++n) {
    }
    if (n == nVertices) {
        return 1;
    }
    for (n = 0; n < nVertices && pVertices[n].y > sRaveClipBottom(); ++n) {
    }
    return n == nVertices;
}

// The same reject test for textured vertices.
// mac ppc classic/1.09d: 102bcf78 (1.00: 10082d60)
int sQAClipHelper(const TQAVTexture * pVertices, int nVertices) {
    int n;
    for (n = 0; n < nVertices && pVertices[n].x < sRaveClipLeft(); ++n) {
    }
    if (n == nVertices) {
        return 1;
    }
    for (n = 0; n < nVertices && pVertices[n].x > sRaveClipRight(); ++n) {
    }
    if (n == nVertices) {
        return 1;
    }
    for (n = 0; n < nVertices && pVertices[n].y < sRaveClipTop(); ++n) {
    }
    if (n == nVertices) {
        return 1;
    }
    for (n = 0; n < nVertices && pVertices[n].y > sRaveClipBottom(); ++n) {
    }
    return n == nVertices;
}

} // namespace D2Rave

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
