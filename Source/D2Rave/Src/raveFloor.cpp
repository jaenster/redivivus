// raveFloor.cpp - floor tiles as a 6x6 grid of textured vertices.
//
// File name inferred (the Windows renderers' floor code is glFloorCache.cpp/d3dFloorCache.cpp).
// The unit runs 102b5614-102b5db8 and ends with __sinit_raveFloor_cpp at 102b5dac.
//
// Reconstructed here: sRaveFloorInit. Not yet: RaveDrawGroundTile (102b5614); see
// d2-surveys/macppc/d2rave/recon.md.

#include "Source/D2Rave/Src/raveInternal.h"

#if !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC

namespace D2Rave {

namespace {
const float sgfRaveFloorVNudge = 1.0f / 128;  // 1037c7e0
const float sgfRaveFloorStep = 1.0f / 16;     // 1037c7e4
const float sgfRaveFloorUNudge = 1.0f / 256;  // 1037c7e8
} // namespace

RaveVertex gRaveFloorVertices[6 * 6];
RaveVertex * gpRaveFloorStrips[5][12];

// The texture coordinates of the 6x6 floor grid in the 256x128 floor texture (each vertex
// nudged half a texel inward at the grid edges), and five strips of 12 vertices.
// mac ppc classic/1.09d: 102b5ba0 (1.00: 1007bdcc)
void sRaveFloorInit() {
    RaveVertex * pVertex = gRaveFloorVertices;
    for (int nRow = 0; nRow < 6; ++nRow) {
        for (int nCol = 0; nCol < 6; ++nCol) {
            int nNudgeU = 0;
            int nNudgeV = 0;
            if (nCol == 0) {
                nNudgeU = 1;
                nNudgeV = 1;
            } else if (nCol == 5) {
                nNudgeU = -1;
                nNudgeV = -1;
            }
            if (nRow == 0) {
                nNudgeU -= 1;
                nNudgeV += 1;
            } else if (nRow == 5) {
                nNudgeU += 1;
                nNudgeV -= 1;
            }
            pVertex->fU = sgfRaveFloorStep * (float)(nCol - nRow) + (float)(nNudgeU + 0x50) * sgfRaveFloorUNudge;
            pVertex->fV = sgfRaveFloorStep * (float)(nCol + nRow) + (float)nNudgeV * sgfRaveFloorVNudge;
            ++pVertex;
        }
    }
    for (int nStrip = 0; nStrip < 5; ++nStrip) {
        for (int nCol = 0; nCol < 6; ++nCol) {
            gpRaveFloorStrips[nStrip][nCol * 2] = &gRaveFloorVertices[nStrip * 6 + nCol];
            gpRaveFloorStrips[nStrip][nCol * 2 + 1] = &gRaveFloorVertices[(nStrip + 1) * 6 + nCol];
        }
    }
}

} // namespace D2Rave

#endif // !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC
