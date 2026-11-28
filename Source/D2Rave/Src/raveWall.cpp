// raveWall.cpp - wall, transparent-wall and shadow tiles.
//
// File name inferred (the Windows renderers' wall code is glWall.cpp/d3dWall.cpp). The unit
// runs 102b98bc-102bba50 and ends with __sinit_raveWall_cpp at 102bba44.
//
// Reconstructed here: the three table slots, the wall init and the panel clip rectangle. Not
// yet: the rasterisers the slots dispatch to (flat and perspective, wall and shadow); see
// d2-surveys/macppc/d2rave/recon.md.

#include "Source/D2Rave/Src/raveInternal.h"

#if !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC

namespace D2Rave {

// Windows slot 39: an opaque wall tile, perspective or flat.
// mac ppc classic/1.09d: 102b98bc (1.00: 1007f5c8)
int RaveDrawWallTile(RaveTileEntry * pTile, int nX, int nY, void * pLight, int nScreenPanels) {
    int bDrawn = 0;
    if (D2CMP_LoadTileData(pTile, 1, 0)) {
        sRaveSetAlphaState(1);
        sRaveSetBlendMode(1);
        sRaveSetTextureFilter(2);
        if (sRaveGetCurrentTexture() == 0) {
            bDrawn = sRaveDrawWallTileFlat(pTile, nX, nY, pLight, 0xff, nScreenPanels);
        } else {
            bDrawn = sRaveDrawWallTilePerspective(pTile, nX, nY, pLight, 0xff, nScreenPanels);
        }
    }
    return bDrawn;
}

// Windows slot 40: as DrawWallTile, alpha-blended unless nAlpha is 0xff.
// mac ppc classic/1.09d: 102b9990 (1.00: 1007f698)
int RaveDrawTransWallTile(RaveTileEntry * pTile, int nX, int nY, void * pLight, int nScreenPanels, uint8_t nAlpha) {
    int bDrawn = 0;
    if (D2CMP_LoadTileData(pTile, 1, 0)) {
        sRaveSetAlphaState(1);
        if (nAlpha == 0xff) {
            sRaveSetBlendMode(1);
        } else {
            sRaveSetBlendMode(4);
        }
        sRaveSetTextureFilter(2);
        if (sRaveGetCurrentTexture() == 0) {
            bDrawn = sRaveDrawWallTileFlat(pTile, nX, nY, pLight, nAlpha, nScreenPanels);
        } else {
            bDrawn = sRaveDrawWallTilePerspective(pTile, nX, nY, pLight, nAlpha, nScreenPanels);
        }
    }
    return bDrawn;
}

// Windows slot 41: a shadow tile, at 75% when the settings ask for blended shadows.
// mac ppc classic/1.09d: 102ba838 (1.00: 10080640)
int RaveDrawShadowTile(RaveTileEntry * pTile, int nX, int nY, int nDrawMode, int nScreenPanels) {
    int bDrawn = 0;
    if (D2CMP_LoadTileData(pTile, 1, 0)) {
        int nAlpha = 0xff;
        sRaveSetAlphaState(1);
        if (sRaveGetSettings14() == 0) {
            sRaveSetBlendMode(1);
        } else {
            sRaveSetBlendMode(4);
            nAlpha = 0xc0;
        }
        sRaveSetTextureFilter(2);
        if (sRaveGetCurrentTexture() == 0) {
            bDrawn = sRaveDrawShadowTileFlat(pTile, nX, nY, nDrawMode, nAlpha, nScreenPanels);
        } else {
            bDrawn = sRaveDrawShadowTilePerspective(pTile, nX, nY, nDrawMode, nAlpha, nScreenPanels);
        }
    }
    return bDrawn;
}

namespace {
const float sgfRaveWallU0 = 1.0f / 64;        // 1037c8b8
const float sgfRaveWallUMid = 0.5078125f;     // 1037c8ac
const float sgfRaveWallRowScale = 0.984375f;  // 1037c8b4
const float sgfRaveWallRowStep = 0.25f;       // 1037c8b0
const float sgfRaveWallZero = 0.0f;           // 1037c8d8
const float sgfRaveWallOne = 1.0f;            // 1037c8dc
int sbRaveWallTrianglesFixed;                 // 1037c8a4
} // namespace

RaveVertex gRaveWallQuad[4];
RaveVertex gRaveWallGrid[5][3];
// 60 corner records in initialised data (read from 103f2958): two coordinates, shifted to 16.16
// by sRaveWallInit, and a third value left as is.
int gRaveWallTriangles[5][12][3] = {
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
     {0, 5, -40}, {0, 3, -56}, {0, 1, -72}, {0, -1, -88}, {0, -3, -104}, {0, -5, -120}},
    {{-5, 0, -120}, {-3, 0, -104}, {-1, 0, -88}, {1, 0, -72}, {3, 0, -56}, {5, 0, -40},
     {0, 5, -40}, {0, 3, -56}, {0, 1, -72}, {1, 0, -72}, {3, 0, -56}, {5, 0, -40}},
    {{-5, 0, -120}, {-3, 0, -104}, {-1, 0, -88}, {0, -1, -88}, {0, -3, -104}, {0, -5, -120},
     {0, 5, -40}, {0, 3, -56}, {0, 1, -72}, {1, 0, -72}, {3, 0, -56}, {5, 0, -40}},
    {{5, 10, 40}, {5, 8, 24}, {5, 6, 8}, {5, 4, -8}, {5, 2, -24}, {5, 0, -40},
     {0, 5, -40}, {2, 5, -24}, {4, 5, -8}, {6, 5, 8}, {8, 5, 24}, {10, 5, 40}},
    {{0, 5, -40}, {2, 5, -24}, {4, 5, -8}, {5, 4, -8}, {5, 2, -24}, {5, 0, -40},
     {0, 5, -40}, {2, 5, -24}, {4, 5, -8}, {5, 4, -8}, {5, 2, -24}, {5, 0, -40}},
};

// The wall quad's texture corners (positions are set per draw; y starts at the screen
// height), a 5x3 grid of texture coordinates down a wall block, and, once, the triangle table's
// coordinates converted to 16.16.
// mac ppc classic/1.09d: 102bb634 (1.00: 10081544)
void sRaveWallInit() {
    const float fHeight = (float)gnRaveScreenHeight;
    gRaveWallQuad[0].fX = sgfRaveWallZero;
    gRaveWallQuad[0].fY = fHeight;
    gRaveWallQuad[0].fU = sgfRaveWallZero;
    gRaveWallQuad[0].fV = sgfRaveWallZero;
    gRaveWallQuad[1].fX = sgfRaveWallZero;
    gRaveWallQuad[1].fY = fHeight;
    gRaveWallQuad[1].fU = sgfRaveWallZero;
    gRaveWallQuad[1].fV = sgfRaveWallOne;
    gRaveWallQuad[2].fX = sgfRaveWallZero;
    gRaveWallQuad[2].fY = fHeight;
    gRaveWallQuad[2].fU = sgfRaveWallOne;
    gRaveWallQuad[2].fV = sgfRaveWallOne;
    gRaveWallQuad[3].fX = sgfRaveWallZero;
    gRaveWallQuad[3].fY = fHeight;
    gRaveWallQuad[3].fU = sgfRaveWallOne;
    gRaveWallQuad[3].fV = sgfRaveWallZero;
    for (int nRow = 0; nRow < 5; ++nRow) {
        const float fV = sgfRaveWallRowScale * (float)nRow * sgfRaveWallRowStep + sgfRaveWallU0;
        gRaveWallGrid[nRow][0].fU = sgfRaveWallU0;
        gRaveWallGrid[nRow][0].fV = fV;
        gRaveWallGrid[nRow][1].fU = sgfRaveWallUMid;
        gRaveWallGrid[nRow][1].fV = fV;
        gRaveWallGrid[nRow][2].fU = sgfRaveWallOne;
        gRaveWallGrid[nRow][2].fV = fV;
    }
    if (sbRaveWallTrianglesFixed != 0) {
        return;
    }
    for (int nGroup = 0; nGroup < 5; ++nGroup) {
        for (int nCorner = 0; nCorner < 12; ++nCorner) {
            gRaveWallTriangles[nGroup][nCorner][0] <<= 0x10;
            gRaveWallTriangles[nGroup][nCorner][1] <<= 0x10;
        }
    }
    sbRaveWallTrianglesFixed = 1;
}

// The drawable rectangle with the side panels open: nPanels 1 closes the right half (or
// quarter), 2 the left; with bOffset the world shifts by a quarter screen instead. The bottom
// 0x2f rows are the control panel.
// mac ppc classic/1.09d: 102bb94c (no 1.00 pair)
void sRaveGetPanelClipRect(int nPanels, int bOffset, int * pRect, int * pOffset) {
    pRect[0] = 0;
    pRect[2] = gnRaveScreenWidth;
    pRect[1] = 0;
    pRect[3] = gnRaveScreenHeight - 0x2f;
    if (bOffset) {
        const int nQuarter = gnRaveScreenWidth / 4;
        if (nPanels == 2) {
            *pOffset = nQuarter;
            pRect[2] -= nQuarter;
            pRect[0] += nQuarter;
        } else if (nPanels == 1) {
            *pOffset = -nQuarter;
            pRect[2] -= nQuarter;
            pRect[0] += nQuarter;
        }
        return;
    }
    pRect[0] -= 0x20;
    pRect[3] += 0x20;
    if (nPanels == 2) {
        pRect[0] += gnRaveScreenWidth / 2;
    } else if (nPanels == 1) {
        pRect[2] -= gnRaveScreenWidth / 2;
    }
}

} // namespace D2Rave

#endif // !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC
