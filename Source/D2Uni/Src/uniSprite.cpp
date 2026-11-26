// uniSprite.cpp - the sprite and wall-tile slots every D2Uni back end shares: visibility tests,
// palette-table choice, then D2CMP's cel blitters or D2gfx's tile renderers into the 8-bit back
// buffer.
//
// File name inferred (Source/D2Uni/README.md). Read last to first, the object's functions run in
// the order of D2DDraw ddSprite.cpp (DrawImage, Shifted, VerticalCrop, Clipped, Shadow, ImageFast,
// DebugFill, Perspective), which is the Windows twin. MWCC emits an object's functions last to
// first, so the functions below are in source order, highest address first.
//
// The PPC builds keep the 1.10 D2GfxDataStrc, whose first word is the cel (the RAVE analysis,
// docs/analysis/d2-rave-renderer.md); UniGetCel reads it. The 1.14d layout in the tree has the
// cel at +0x3C.
//
// Classic Mac PowerPC only.
#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

#include "Source/D2Uni/Src/uniInternal.h"

using namespace D2CMP::SRC;

// The horizontal clip every sprite blit uses, set by UniSpriteSetResolution.
static int gnUniSpriteClipRight;    // 10434498
static int gnUniSpriteClipLeft;     // 1043449c

// Which kind of table UniGetPaletteTable returns for draw modes 0-6: 2 = a 256x256 blend table,
// 1 = a 256-entry remap, 0 = none. The same values as D2DDraw's gdwDirectDrawSurfaceHeights.
static const int gaUniPaletteTableKinds[7] = {2, 2, 2, 1, 1, 0, 1}; // 104420f8

static inline DC6Block * UniGetCel(D2GfxDataStrc * pData) {
    return *(DC6Block **)pData;
}

// The palette table a draw mode blits through, inlined into every sprite slot below; the same
// switch as D2DDraw RENDERER_DIRECTDRAW_GetPalettePointer.
static inline BYTE * UniPaletteTableForMode(int nDrawMode) {
    switch (nDrawMode) {
        case 0:
            return gapUniPaletteTables[2];
        case 1:
            return gapUniPaletteTables[1];
        case 2:
            return gapUniPaletteTables[0];
        case 3:
            return gapUniPaletteTables[0x41];
        case 4:
            return gapUniPaletteTables[0x42];
        case 6:
            return gapUniPaletteTables[0x43];
    }
    return nullptr;
}

// ppc 1.10b: 100560c0 (1.00 10084ff8, 1.13d 1005c200, carbon 1.10b 103a5070, carbon 1.13d 103d48e0)
// Classic 1.10b clips the panels at a fixed 320 pixels, not at half the buffer as DrawWallTile does.
BOOL __fastcall UniDrawShadowTile(D2TileLibraryEntryStrc * pTile, int nPosX, int nPosY, int nUnused, int nScreenPanels) {
    BOOL bDrawn = 0;
    if (!SpriteCache::SPRITECACHE_AllocEntry(pTile, 1, 0)) {
        return 0;
    }

    int nClipBottom = gnUniBackBufferHeight;
    int nClipLeft = -0x20;
    int nClipRight = gnUniBackBufferWidth;
    if (nScreenPanels == 1) {
        nClipRight -= 0x140;
    } else if (nScreenPanels == 2) {
        nClipLeft = 0x120;
    }

    for (int i = 0; i < pTile->nSubTiles; i++) {
        D2GroundSubTileStrc * pSubTile = &pTile->pSubTiles[i];
        int nX = nPosX + pSubTile->nOffsetX;
        int nY = nPosY + pSubTile->nOffsetY;
        if (nX >= nClipLeft && nX < nClipRight && nY > -0x21 && nY < nClipBottom + 0x20) {
            bDrawn = 1;
            if (!UniAreBlendedShadowsEnabled()) {
                gpUniHelpers->nfpDrawVisTile(pSubTile->pPixelData, nX, nY);
            } else {
                gpUniHelpers->nfpDrawBlendedVisTile(pSubTile->pPixelData, nX, nY, 0xC0);
            }
        }
    }
    return bDrawn;
}

// ppc 1.10b: 10055f80 (1.00 10084eb8, 1.13d 1005c0c0, carbon 1.10b 103a4f30, carbon 1.13d 103d47a0)
// The light is one DWORD per 32-pixel column; its low byte is the intensity.
BOOL __fastcall UniDrawTransWallTile(D2TileLibraryEntryStrc * pTile, int nPosX, int nPosY, D2GfxLightStrc * pLight,
                                     int nScreenPanels, BYTE nAlpha) {
    BOOL bDrawn = 0;
    uint32_t aCornerLight[4];
    if (!SpriteCache::SPRITECACHE_AllocEntry(pTile, 1, 0)) {
        return 0;
    }

    int nClipBottom = gnUniBackBufferHeight;
    int nClipLeft = -0x20;
    int nClipRight = gnUniBackBufferWidth;
    if (nScreenPanels == 1) {
        nClipRight -= 0x140;
    } else if (nScreenPanels == 2) {
        nClipLeft = 0x120;
    }

    const DWORD * pColumnLight = (const DWORD *)pLight;
    for (int i = 0; i < pTile->nSubTiles; i++) {
        D2GroundSubTileStrc * pSubTile = &pTile->pSubTiles[i];
        int nX = nPosX + pSubTile->nOffsetX;
        int nY = nPosY + pSubTile->nOffsetY;
        if (nX >= nClipLeft && nX < nClipRight && nY > -0x21 && nY < nClipBottom + 0x20) {
            const DWORD * pEntry = &pColumnLight[pSubTile->nOffsetX >> 5];
            bDrawn = 1;
            aCornerLight[0] = pEntry[0] & 0xFF;
            aCornerLight[1] = pEntry[1] & 0xFF;
            aCornerLight[2] = aCornerLight[1];
            aCornerLight[3] = aCornerLight[0];
            gpUniHelpers->nfpDrawRoofTile(pSubTile->pPixelData, nX, nY, aCornerLight, nAlpha);
        }
    }
    return bDrawn;
}

// ppc 1.10b: 10055e20 (1.00 10084d80, 1.13d 1005bf60, carbon 1.10b 103a4dd0, carbon 1.13d 103d4640)
BOOL __fastcall UniDrawWallTile(D2TileLibraryEntryStrc * pTile, int nPosX, int nPosY, D2GfxLightStrc * pLight,
                                int nScreenPanels) {
    BOOL bDrawn = 0;
    uint32_t aCornerLight[4];
    if (!SpriteCache::SPRITECACHE_AllocEntry(pTile, 1, 0)) {
        return 0;
    }

    int nClipLeft = -0x20;
    int nClipRight = gnUniBackBufferWidth;
    int nClipBottom = gnUniBackBufferHeight;
    if (nScreenPanels == 2) {
        nClipLeft = nClipRight / 2 - 0x20;
    } else if (nScreenPanels == 1) {
        nClipRight -= nClipRight / 2;
    }

    const DWORD * pColumnLight = (const DWORD *)pLight;
    for (int i = 0; i < pTile->nSubTiles; i++) {
        D2GroundSubTileStrc * pSubTile = &pTile->pSubTiles[i];
        int nX = nPosX + pSubTile->nOffsetX;
        int nY = nPosY + pSubTile->nOffsetY;
        if (nX >= nClipLeft && nX < nClipRight && nY > -0x21 && nY < nClipBottom + 0x20) {
            const DWORD * pEntry = &pColumnLight[pSubTile->nOffsetX >> 5];
            bDrawn = 1;
            aCornerLight[0] = pEntry[0] & 0xFF;
            aCornerLight[1] = pEntry[1] & 0xFF;
            aCornerLight[2] = aCornerLight[1];
            aCornerLight[3] = aCornerLight[0];
            gpUniHelpers->nfpDrawWallTile(pSubTile->pPixelData, nX, nY, aCornerLight);
        }
    }
    return bDrawn;
}

// ppc 1.10b: 10055db0 (1.00 10084d1c, 1.13d 1005bef0, carbon 1.10b 103a4d60, carbon 1.13d 103d45d0)
// The out-of-line copy of the palette-table switch, used by UniDrawSolidRectEx; *pnTableKind
// gets the table's kind (see gaUniPaletteTableKinds).
BYTE * UniGetPaletteTable(int nDrawMode, int * pnTableKind) {
    *pnTableKind = gaUniPaletteTableKinds[nDrawMode];
    switch (nDrawMode) {
        case 0:
            return gapUniPaletteTables[2];
        case 1:
            return gapUniPaletteTables[1];
        case 2:
            return gapUniPaletteTables[0];
        case 3:
            return gapUniPaletteTables[0x41];
        case 4:
            return gapUniPaletteTables[0x42];
        case 6:
            return gapUniPaletteTables[0x43];
    }
    return nullptr;
}

// ppc 1.10b: 10055d80 (1.00 10084cec, 1.13d 1005bec0, carbon 1.10b 103a4d30, carbon 1.13d 103d45a0)
// The D2DDraw RENDERER_DDRAW_SetHorizontalBounds twin.
void UniSpriteSetResolution(BOOL bLarge) {
    if (!bLarge) {
        gnUniSpriteClipLeft = 0;
        gnUniSpriteClipRight = 0x280;
        return;
    }
    gnUniSpriteClipLeft = 0;
    gnUniSpriteClipRight = 800;
}

// ppc 1.10b: 10055c00 (1.13d 1005bd10, carbon 1.10b 103a4bb0, carbon 1.13d 103d43f0)
// Clips a cel against a crop rectangle {left, right, top, bottom}; fills pClip with
// {left, right, rows to skip, rows to draw}. Returns 1 when nothing is left. The D2DDraw
// RENDERER_DDRAW_ComputeDC6ClipRect twin, without its zero-height halt.
static BOOL sUniClipImage(DC6Block * pCel, int nPosX, int nPosY, const int * pCrop, int * pClip) {
    int nWidth = CelCmp::GetDC6Width(pCel);
    int nHeight = CelCmp::GetDC6Height(pCel);
    int nLeft = pCrop[0] < 0 ? 0 : pCrop[0];
    int nRight = pCrop[1] < gnUniBackBufferWidth ? pCrop[1] : gnUniBackBufferWidth;
    if (nRight < nLeft) {
        return 1;
    }

    int nX = nPosX + CelCmp::GetDC6OffsetX(pCel);
    if (!(nX < nRight && nLeft <= nX + nWidth)) {
        return 1;
    }

    int nTop = pCrop[2] < 0 ? 0 : pCrop[2];
    int nBottom = pCrop[3] < gnUniBackBufferHeight ? pCrop[3] : gnUniBackBufferHeight;
    if (nBottom < nTop) {
        return 1;
    }

    int nY = nPosY + CelCmp::GetDC6OffsetY(pCel);
    if (nY < nTop || nBottom < nY - nHeight) {
        return 1;
    }

    pClip[0] = nLeft;
    pClip[1] = nRight;
    pClip[2] = nY > nBottom ? nY - nBottom - 1 : 0;
    if (nY - nHeight < nTop) {
        pClip[3] = nY - nTop - pClip[2] + 1;
    } else {
        pClip[3] = nHeight - pClip[2];
    }
    return 0;
}

// The on-screen test the image slots share. The vertical one adds half of what the +0x10 cel
// accessor returns; the PEF code shifts it right by one (srawi r0,r3,1) in every image slot,
// while sUniClipImage adds it whole.
static inline BOOL UniIsCelOffScreen(DC6Block * pCel, int nPosX, int nPosY) {
    int nWidth = CelCmp::GetDC6Width(pCel);
    int nHeight = CelCmp::GetDC6Height(pCel);
    int nX = nPosX + CelCmp::GetDC6OffsetX(pCel);
    if (!(nX < gnUniBackBufferWidth && nX + nWidth >= 0)) {
        return 1;
    }
    int nY = nPosY + (CelCmp::GetDC6OffsetY(pCel) >> 1);
    return nY < 0 || nY - nHeight >= gnUniBackBufferHeight;
}

// ppc 1.10b: 10055a40 (1.00 10084b38, 1.13d 1005bb50, carbon 1.10b 103a49f0, carbon 1.13d 103d4230)
// The software renderer has no perspective: DrawImage with the 7-argument slot's palette.
void __fastcall UniDrawPerspectiveImage(D2GfxDataStrc * pData, int nPosX, int nPosY, DWORD dwGamma, int nDrawMode,
                                        int nScreenMode, BYTE * pPalette) {
    if (!SpriteCache::SPRITECACHE_GetOrLoadSprite(pData, 0, 1)) {
        return;
    }
    if (UniIsCelOffScreen(UniGetCel(pData), nPosX, nPosY)) {
        return;
    }

    BYTE * pTable = (unsigned)nDrawMode <= 6 ? UniPaletteTableForMode(nDrawMode) : nullptr;
    BYTE * pBlend = nullptr;
    if ((dwGamma & 0xFF) != 0xFF) {
        pBlend = gapUniPaletteTables[((int)(dwGamma & 0xFF) >> 3) + 3];
    }
    if (nDrawMode == 7) {
        pBlend = gapUniPaletteTables[0x46];
    }
    CelCmp::CELCMP_DrawCelFrame(pData, nPosX, nPosY, gnUniSpriteClipLeft, gnUniSpriteClipRight, gpUniBackBuffer,
                                gnUniBackBufferHeight, gnUniBackBufferWidth, pBlend, pTable, pPalette);
}

// ppc 1.10b: 10055880 (1.00 10084984, 1.13d 1005b990, carbon 1.10b 103a4830, carbon 1.13d 103d4070)
BOOL __fastcall UniDrawImage(D2GfxDataStrc * pData, int nPosX, int nPosY, DWORD dwGamma, int nDrawMode, BYTE * pPalette) {
    if (!SpriteCache::SPRITECACHE_GetOrLoadSprite(pData, 0, 1)) {
        return 0;
    }
    if (UniIsCelOffScreen(UniGetCel(pData), nPosX, nPosY)) {
        return 0;
    }

    BYTE * pTable = (unsigned)nDrawMode <= 6 ? UniPaletteTableForMode(nDrawMode) : nullptr;
    BYTE * pBlend = nullptr;
    if ((dwGamma & 0xFF) != 0xFF) {
        pBlend = gapUniPaletteTables[((int)(dwGamma & 0xFF) >> 3) + 3];
    }
    if (nDrawMode == 7) {
        pBlend = gapUniPaletteTables[0x46];
    }
    CelCmp::CELCMP_DrawCelFrame(pData, nPosX, nPosY, gnUniSpriteClipLeft, gnUniSpriteClipRight, gpUniBackBuffer,
                                gnUniBackBufferHeight, gnUniBackBufferWidth, pBlend, pTable, pPalette);
    return 1;
}

// ppc 1.10b: 100556b0 (1.00 100847b8, 1.13d 1005b7c0, carbon 1.10b 103a4660, carbon 1.13d 103d3ea0)
void __fastcall UniDrawShiftedImage(D2GfxDataStrc * pData, int nPosX, int nPosY, DWORD dwGamma, DRAWMODE eDrawMode,
                                    GLOBALPALLETE nGlobalPaletteShift) {
    int nDrawMode = (int)eDrawMode;
    if (!SpriteCache::SPRITECACHE_GetOrLoadSprite(pData, 0, 1)) {
        return;
    }
    if (UniIsCelOffScreen(UniGetCel(pData), nPosX, nPosY)) {
        return;
    }

    BYTE * pTable = (unsigned)nDrawMode <= 6 ? UniPaletteTableForMode(nDrawMode) : nullptr;
    BYTE * pBlend = nullptr;
    BYTE * pShift = nullptr;
    if ((dwGamma & 0xFF) != 0xFF) {
        pBlend = gapUniPaletteTables[((int)(dwGamma & 0xFF) >> 3) + 3];
    }
    if (nDrawMode == 7) {
        pBlend = gapUniPaletteTables[0x46];
    }
    if (nGlobalPaletteShift) {
        pShift = gapUniPaletteTables[nGlobalPaletteShift + 0x34];
    }
    CelCmp::CELCMP_DrawCelFrame(pData, nPosX, nPosY, gnUniSpriteClipLeft, gnUniSpriteClipRight, gpUniBackBuffer,
                                gnUniBackBufferHeight, gnUniBackBufferWidth, pBlend, pTable, pShift);
}

// ppc 1.10b: 10055530 (1.00 10084638, 1.13d 1005b640, carbon 1.10b 103a44e0, carbon 1.13d 103d3d20)
void __fastcall UniDrawVerticalCropImage(D2GfxDataStrc * pData, int nPosX, int nPosY, int nSkipLines, int nDrawLines,
                                         int nDrawMode) {
    if (!SpriteCache::SPRITECACHE_GetOrLoadSprite(pData, 0, 1)) {
        return;
    }
    if (UniIsCelOffScreen(UniGetCel(pData), nPosX, nPosY)) {
        return;
    }

    BYTE * pTable = (unsigned)nDrawMode <= 6 ? UniPaletteTableForMode(nDrawMode) : nullptr;
    CelCmp::CELCMP_DrawCelFrameFromGfxData(pData, nPosX, nPosY, gnUniSpriteClipLeft, gnUniSpriteClipRight,
                                           gpUniBackBuffer, gnUniBackBufferHeight, gnUniBackBufferWidth, 0, pTable,
                                           nSkipLines, nDrawLines, 0);
}

// ppc 1.10b: 10055410 (1.13d 1005b520, carbon 1.10b 103a43c0, carbon 1.13d 103d3c00)
// Added with LoD (absent from classic 1.00's 56-slot table).
void __fastcall UniDrawClippedImage(D2GfxDataStrc * pData, int nPosX, int nPosY, void * pCropRect, int nDrawMode) {
    int aClip[4];
    if (!SpriteCache::SPRITECACHE_GetOrLoadSprite(pData, 0, 1)) {
        return;
    }
    if (sUniClipImage(UniGetCel(pData), nPosX, nPosY, (const int *)pCropRect, aClip)) {
        return;
    }

    BYTE * pTable = (unsigned)nDrawMode <= 6 ? UniPaletteTableForMode(nDrawMode) : nullptr;
    CelCmp::CELCMP_DrawCelFrameFromGfxData(pData, nPosX, nPosY, aClip[0], aClip[1], gpUniBackBuffer,
                                           gnUniBackBufferHeight, gnUniBackBufferWidth, 0, pTable, aClip[2], aClip[3],
                                           0);
}

// ppc 1.10b: 100552d0 (1.00 10084500, 1.13d 1005b3e0, carbon 1.10b 103a4280, carbon 1.13d 103d3ac0)
void __fastcall UniDrawShadow(D2GfxDataStrc * pData, int nPosX, int nPosY) {
    if (!SpriteCache::SPRITECACHE_GetOrLoadSprite(pData, 0, 1)) {
        return;
    }

    DC6Block * pCel = UniGetCel(pData);
    int nWidth = CelCmp::GetDC6Width(pCel);
    int nHeight = CelCmp::GetDC6Height(pCel);
    int nX = nPosX - nHeight + CelCmp::GetDC6OffsetX(pCel);
    if (!(nX < gnUniBackBufferWidth && nX + nWidth + nHeight >= 0)) {
        return;
    }
    int nY = nPosY + (CelCmp::GetDC6OffsetY(pCel) >> 1);
    if (nY < 0 || nY - (nHeight >> 1) >= gnUniBackBufferHeight) {
        return;
    }

    BYTE * pBlend = nullptr;
    if (UniAreBlendedShadowsEnabled()) {
        pBlend = gapUniPaletteTables[0];
    }
    CelCmp::CELCMP_DrawCelFrameCentered(pData, nPosX, nPosY, gnUniSpriteClipLeft, gnUniSpriteClipRight, gpUniBackBuffer,
                                        gnUniBackBufferHeight, gnUniBackBufferWidth, pBlend, 0);
}

// ppc 1.10b: 100552c0 (1.00 100844fc, 1.13d 1005b3d0, carbon 1.10b 103a4270, carbon 1.13d 103d3ab0)
void __fastcall UniDrawImageFast(D2GfxDataStrc * pData, int nPosX, int nPosY, BYTE nPaletteIndex) {
}

// ppc 1.10b: 10055290 (1.00 100844d4, 1.13d 1005b3a0, carbon 1.10b 103a4240, carbon 1.13d 103d3a80)
// Not implemented on the Mac: halts with "UniLens" (the Windows twin fills an 80x80 debug square).
void __fastcall UniDebugFillBackBuffer(int nPosX, int nPosY) {
    MacFatal("UniLens");
}

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
