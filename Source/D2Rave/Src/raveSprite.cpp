// raveSprite.cpp - the DC6 image drawers and the sprite texture cache.
//
// File name attested: __FILE__ "raveSprite.cpp" (string 10353a10) in the buffer allocations.
// The unit runs 102b7368-102b95fc and ends with __sinit_raveSprite_cpp at 102b95f0.
//
// Reconstructed here: the image-fast, shadow, shifted and plain image slots, the debug fill
// slot, the draw-mode and texture-size helpers, the cache release callback and init/free. Not
// yet: RaveDrawClippedImage (102b75e4), sRaveClipSpriteRect (102b7888), RaveDrawVerticalCropImage
// (102b7a18), RaveDrawPerspectiveImage (102b7f30), sRaveDrawSpriteQuad (102b88a0),
// sRaveDrawShadowQuad (102b8cc8) and sRaveUploadSprite (102b9080); see
// d2-surveys/macppc/d2rave/recon.md.

#include "Source/D2Rave/Src/raveInternal.h"
#include "Source/D2Rave/Src/CRaveTextures.h"

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

namespace D2Rave {

namespace {
const char sgszRaveSpriteFile[] = "raveSprite.cpp";
const float sgfRaveSpriteU = 0.0f;  // 1037c878
const float sgfRaveSpriteV = 1.0f;  // 1037c890

// The cel behind a D2GfxDataStrc (its first field).
inline void * sRaveCel(D2GfxDataStrc * pData) {
    return *(void **)pData;
}

// Ours: the five cel reads every drawer starts with.
inline void sRaveDescribeSprite(RaveSprite * pSprite, D2GfxDataStrc * pData) {
    pSprite->pData = pData;
    pSprite->nWidth = D2CMP_CelGetWidth(sRaveCel(pData));
    pSprite->nOffsetX = D2CMP_CelGetOffsetX(sRaveCel(pData));
    pSprite->nHeight = D2CMP_CelGetHeight(sRaveCel(pData));
    pSprite->nOffsetY = D2CMP_CelGetOffsetY(sRaveCel(pData));
}

// The drawers' common visibility test: off when the sprite lies wholly outside the screen.
inline bool sRaveSpriteOffscreen(const RaveSprite & sprite, int nX, int nY) {
    if (nX + sprite.nOffsetX < gnRaveScreenWidth && nX + sprite.nOffsetX + sprite.nWidth > -1) {
        return nY + sprite.nOffsetY < 0 || gnRaveScreenHeight <= (nY + sprite.nOffsetY) - sprite.nHeight;
    }
    return true;
}
} // namespace

uint8_t * gpRaveSpriteBufferA;
uint8_t * gpRaveSpriteBufferB;
uint8_t * gpRaveSpriteBufferC;

// Windows slot 52. Empty on RAVE; asm only in the database.
// mac ppc classic/1.09d: 102b7364 (1.00: 1007d3fc)
void RaveDebugFillBackBuffer(int nX, int nY) {
    (void)nX;
    (void)nY;
}

// Windows slot 37: a one-colour image (the automap and text), with a one-texel border.
// mac ppc classic/1.09d: 102b7368 (1.00: 1007d400)
void RaveDrawImageFast(D2GfxDataStrc * pData, int nX, int nY, uint8_t nPaletteIndex) {
    const int nAlpha = sRaveSetSpriteDrawMode(0);
    if (!D2CMP_GetCelData(pData, 0, 1)) {
        return;
    }
    RaveSprite sprite;
    sRaveDescribeSprite(&sprite, pData);
    sprite.nWidth += 2;
    sprite.nHeight += 2;
    if (sprite.nHeight < 0x101 && sprite.nWidth < 0x101) {
        sRaveGetSpriteTexture(&sprite);
        if (sRaveUploadSprite(&sprite, nullptr, nPaletteIndex, 3)) {
            sRaveDrawSpriteQuad(&sprite, nX - 1, nY + 1, 0xffffffff, 0, nAlpha, 1, 0);
        }
    }
}

// Windows slot 36: a unit shadow, blended at 75% when the settings ask for it.
// mac ppc classic/1.09d: 102b749c (1.00: 1007d548)
void RaveDrawShadow(D2GfxDataStrc * pData, int nX, int nY) {
    int nAlpha = 0xff;
    if (!D2CMP_GetCelData(pData, 0, 1)) {
        return;
    }
    RaveSprite sprite;
    sRaveDescribeSprite(&sprite, pData);
    if (sprite.nHeight < 0x101 && sprite.nWidth < 0x101) {
        sRaveGetSpriteTexture(&sprite);
        if (sRaveUploadSprite(&sprite, nullptr, 0, 4)) {
            sRaveSetAlphaState(1);
            if (sRaveGetSettings14() == 0) {
                sRaveSetSpriteDrawMode(5);
            } else {
                sRaveSetBlendMode(4);
                nAlpha = 0xc0;
            }
            sRaveDrawShadowQuad(&sprite, nX, nY, nAlpha);
        }
    }
}

// Windows slot 34: an image through one of the global palette shifts.
// mac ppc classic/1.09d: 102b7bd4 (1.00: 1007d870)
int RaveDrawShiftedImage(D2GfxDataStrc * pData, int nX, int nY, uint32_t dwColorTint, int nDrawMode, int nPalette) {
    if (!D2CMP_GetCelData(pData, 0, 1)) {
        return 0;
    }
    RaveSprite sprite;
    sRaveDescribeSprite(&sprite, pData);
    if (sprite.nHeight >= 0x101 || sprite.nWidth >= 0x101) {
        return 1;
    }
    if (sRaveSpriteOffscreen(sprite, nX, nY)) {
        return 0;
    }
    sRaveGetSpriteTexture(&sprite);
    if (!sRaveUploadSprite(&sprite, nullptr, 0, 1)) {
        return 1;
    }
    const int nAlpha = sRaveSetSpriteDrawMode(nDrawMode);
    sRaveDrawSpriteQuad(&sprite, nX, nY, dwColorTint, nPalette, nAlpha, 0, 0);
    return 1;
}

// Windows slot 33. Draw mode 7 uploads the cel as kind 2, everything else as kind 1.
// mac ppc classic/1.09d: 102b7d68 (1.00: 1007da20)
int RaveDrawImage(D2GfxDataStrc * pData, int nX, int nY, uint32_t dwColorTint, int nDrawMode, uint8_t * pPalette) {
    if (!D2CMP_GetCelData(pData, 0, 1)) {
        return 0;
    }
    RaveSprite sprite;
    sRaveDescribeSprite(&sprite, pData);
    if (sprite.nHeight >= 0x101 || sprite.nWidth >= 0x101) {
        return 1;
    }
    if (sRaveSpriteOffscreen(sprite, nX, nY)) {
        return 0;
    }
    const int nAlpha = sRaveSetSpriteDrawMode(nDrawMode);
    sRaveGetSpriteTexture(&sprite);
    if (!sRaveUploadSprite(&sprite, pPalette, 0, nDrawMode == 7 ? 2 : 1)) {
        return 1;
    }
    sRaveDrawSpriteQuad(&sprite, nX, nY, dwColorTint, 0, nAlpha, 0, 0);
    return 1;
}

// Sets filter and blend for a draw mode and returns the alpha: modes 0-2 are 25/50/75%
// blends, 3 additive, 4 modulate, 5 and 7 opaque.
// mac ppc classic/1.09d: 102b87b0 (1.00: 1007e374)
int sRaveSetSpriteDrawMode(int nDrawMode) {
    int nAlpha = 0xff;
    if (nDrawMode == 5 || nDrawMode == 7) {
        sRaveSetTextureFilter(1);
        sRaveSetBlendMode(1);
        return nAlpha;
    }
    if (nDrawMode == 3) {
        sRaveSetTextureFilter(2);
        sRaveSetBlendMode(2);
        return 0xff;
    }
    if (nDrawMode == 4) {
        sRaveSetTextureFilter(2);
        sRaveSetBlendMode(3);
        return 0xff;
    }
    sRaveSetTextureFilter(1);
    sRaveSetBlendMode(4);
    if (nDrawMode == 0) {
        nAlpha = 0x40;
    }
    if (nDrawMode == 1) {
        nAlpha = 0x80;
    }
    if (nDrawMode == 2) {
        nAlpha = 0xc0;
    }
    return nAlpha;
}

// The power-of-two texture size and the texture coordinates for a sprite.
// mac ppc classic/1.09d: 102b93b4 (1.00: 1007f0d8)
void sRaveGetSpriteTexture(RaveSprite * pSprite) {
    int nTexWidth = 1;
    while (nTexWidth < pSprite->nWidth) {
        nTexWidth <<= 1;
    }
    int nTexHeight = 1;
    while (nTexHeight < pSprite->nHeight) {
        nTexHeight <<= 1;
    }
    pSprite->nTexWidth = nTexWidth;
    pSprite->nTexHeight = nTexHeight;
    sRaveGetCurrentTexture();
    pSprite->fU0 = sgfRaveSpriteU;
    pSprite->fV0 = sgfRaveSpriteV;
    pSprite->fU1 = sgfRaveSpriteU;
    pSprite->fV1 = sgfRaveSpriteV;
    pSprite->nTexArea = pSprite->nTexWidth * pSprite->nTexHeight;
}

// D2CMP's sprite-cache release callback (TVector 10384748): five (key, texture id) pairs.
// mac ppc classic/1.09d: 102b9444 (1.00: 1007f168)
void sRaveReleaseSpriteTextures(int * pEntries) {
    for (int i = 0; i < 5; ++i, pEntries += 2) {
        if (pEntries[0] - 1 > -1 && pEntries[1] > 0) {
            CD2TEXTURES_ReleaseById(gpRaveTextures, pEntries[1]);
        }
    }
}

// mac ppc classic/1.09d: 102b9568 (1.00: 1007f28c)
void sRaveSpriteInit() {
    gpRaveSpriteBufferA = (uint8_t *)SMemAlloc(0x10000, sgszRaveSpriteFile, 92, 0);
    gpRaveSpriteBufferB = (uint8_t *)SMemAlloc(0x10000, sgszRaveSpriteFile, 93, 0);
    gpRaveSpriteBufferC = (uint8_t *)SMemAlloc(0x40000, sgszRaveSpriteFile, 94, 0);
    D2CMP_SetSpriteFreeCallback(sRaveReleaseSpriteTextures);
}

// mac ppc classic/1.09d: 102b94bc (1.00: 1007f1e0)
void sRaveSpriteFree() {
    if (gpRaveSpriteBufferA != nullptr) {
        SMemFree(gpRaveSpriteBufferA, sgszRaveSpriteFile, 109, 0);
        gpRaveSpriteBufferA = nullptr;
    }
    if (gpRaveSpriteBufferB != nullptr) {
        SMemFree(gpRaveSpriteBufferB, sgszRaveSpriteFile, 115, 0);
        gpRaveSpriteBufferB = nullptr;
    }
    if (gpRaveSpriteBufferC != nullptr) {
        SMemFree(gpRaveSpriteBufferC, sgszRaveSpriteFile, 121, 0);
        gpRaveSpriteBufferC = nullptr;
    }
    D2CMP_SetSpriteFreeCallback(nullptr);
}

} // namespace D2Rave

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
