// raveBlocks.cpp - floor and wall tile decoding into RAVE texture uploads.
//
// File name attested: the PPC build's __FILE__ for the scratch-buffer allocations is
// "raveBlocks.cpp" (string 103531f9). The unit ends with __sinit_raveBlocks_cpp at 102b1d10.
// The line numbers passed to SMemAlloc/SMemFree are the original source lines.

#include "Source/D2Rave/Src/raveInternal.h"
#include "Source/D2Rave/Src/CRaveTextures.h"

#if !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC

namespace D2Rave {

namespace {
const char sgszRaveBlocksFile[] = "raveBlocks.cpp";
} // namespace

uint8_t * gpRaveWallTileBuffer;
uint8_t * gpRaveBlockBuffer;
uint8_t * gpRaveFloorTileBuffer;

// D2CMP's tile-cache release callback (installed by sInitRaveContext through TVector 103845d0):
// drops the tile's floor texture and every block's wall texture.
// mac ppc classic/1.09d: 102b1744 (1.00: 10077338)
void sRaveFreeTileTextures(RaveTileEntry * pTile) {
    if (pTile->nTextureId > 0) {
        CD2TEXTURES_ReleaseById(gpRaveTextures, pTile->nTextureId);
    }
    for (int i = 0; i < pTile->nBlocks; ++i) {
        if (pTile->pBlocks[i].nTextureId > 0) {
            CD2TEXTURES_ReleaseById(gpRaveTextures, pTile->pBlocks[i].nTextureId);
        }
    }
}

// Uploads a 32x32 wall block on first use, then marks it used this frame.
// mac ppc classic/1.09d: 102b17e0 (1.00: 100773d4)
void sRaveLoadWallTexture(RaveTileBlock * pBlock) {
    if (pBlock->nTextureId == 0) {
        sRaveDecodeWallTile(pBlock->pData, gpRaveWallTileBuffer);
        CreateTexture(gpRaveTextures, gpRaveWallTileBuffer, 0x20, 0x20, 2, pBlock, 0, 0);
    }
    CD2TEXTURES_TouchEntry(gpRaveTextures, pBlock->nTextureId);
}

// Uploads a 256x128 floor tile on first use, then marks it used this frame.
// mac ppc classic/1.09d: 102b1864 (1.00: 10077458)
int sRaveLoadFloorTexture(RaveTileEntry * pTile) {
    int bLoaded = 0;
    if (D2CMP_LoadTileData(pTile, 1, 0)) {
        if (pTile->nTextureId == 0) {
            sRaveDecodeFloorTile(pTile, gpRaveFloorTileBuffer, 0x100);
            CreateTexture(gpRaveTextures, gpRaveFloorTileBuffer, 0x100, 0x80, 1, pTile, 0, 0);
        }
        CD2TEXTURES_TouchEntry(gpRaveTextures, pTile->nTextureId);
        bLoaded = 1;
    }
    return bLoaded;
}

// Unpacks a floor tile's blocks into an 8-bit image of 128 rows: RLE blocks (pairs of skip,
// count; 0,0 ends a row) or raw isometric 32x15 diamonds.
// mac ppc classic/1.09d: 102b191c (1.00: 1007750c)
void sRaveDecodeFloorTile(RaveTileEntry * pTile, uint8_t * pDest, int nPitch) {
    ZeroMemory(pDest, nPitch << 7);
    for (int i = 0; i < pTile->nBlocks; ++i) {
        RaveTileBlock * pBlock = &pTile->pBlocks[i];
        uint8_t * pRow = pDest + pBlock->nPosY * nPitch + pBlock->nPosX;
        if ((pBlock->nFormat & 1) == 0) {
            continue;
        }
        if (pBlock->nFormat & 4) {
            const uint8_t * pSrc = pBlock->pData;
            uint8_t * pOut = pRow;
            int nRows = 15;
            while (nRows != 0) {
                uint8_t nSkip = pSrc[0];
                uint8_t nCount = pSrc[1];
                pSrc += 2;
                if (nSkip == 0 && nCount == 0) {
                    pRow += nPitch;
                    --nRows;
                    pOut = pRow;
                    continue;
                }
                pOut += nSkip;
                for (unsigned int n = nCount; n != 0; --n) {
                    *pOut++ = *pSrc++;
                }
            }
        } else {
            const uint8_t * pSrc = pBlock->pData;
            uint8_t * pOut = pRow + 14;
            int nWidth = 4;
            for (int nRow = 0; nRow < 15; ++nRow) {
                CopyMemory(pOut, pSrc, nWidth);
                pSrc += nWidth;
                int nStep;
                if (nRow < 7) {
                    nWidth += 4;
                    nStep = nPitch - 2;
                } else {
                    nWidth -= 4;
                    nStep = nPitch + 2;
                }
                pOut += nStep;
            }
        }
    }
}

// Unpacks one RLE wall block into a 32x32 8-bit image.
// mac ppc classic/1.09d: 102b1ad8 (1.00: 100776c8)
void sRaveDecodeWallTile(const uint8_t * pSrc, uint8_t * pDest) {
    int nRows = 0x20;
    ZeroMemory(pDest, 0x400);
    uint8_t * pRow = pDest;
    uint8_t * pOut = pDest;
    while (nRows != 0) {
        uint8_t nSkip = pSrc[0];
        uint8_t nCount = pSrc[1];
        pSrc += 2;
        if (nSkip == 0 && nCount == 0) {
            pRow += 0x20;
            --nRows;
            pOut = pRow;
            continue;
        }
        pOut += nSkip;
        for (unsigned int n = nCount; n != 0; --n) {
            *pOut++ = *pSrc++;
        }
    }
}

// mac ppc classic/1.09d: 102b1bf4 (1.00: 100777e4)
void sRaveBlocksFree() {
    if (gpRaveWallTileBuffer != nullptr) {
        SMemFree(gpRaveWallTileBuffer, sgszRaveBlocksFile, 50, 0);
        gpRaveWallTileBuffer = nullptr;
    }
    if (gpRaveBlockBuffer != nullptr) {
        SMemFree(gpRaveBlockBuffer, sgszRaveBlocksFile, 56, 0);
        gpRaveBlockBuffer = nullptr;
    }
    if (gpRaveFloorTileBuffer != nullptr) {
        SMemFree(gpRaveFloorTileBuffer, sgszRaveBlocksFile, 62, 0);
        gpRaveFloorTileBuffer = nullptr;
    }
}

// mac ppc classic/1.09d: 102b1c94 (1.00: 10077884)
void sRaveBlocksInit() {
    gpRaveWallTileBuffer = (uint8_t *)SMemAlloc(0x400, sgszRaveBlocksFile, 41, 0);
    gpRaveBlockBuffer = (uint8_t *)SMemAlloc(0x1000, sgszRaveBlocksFile, 42, 0);
    gpRaveFloorTileBuffer = (uint8_t *)SMemAlloc(0x20000, sgszRaveBlocksFile, 43, 0);
}

} // namespace D2Rave

#endif // !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC
