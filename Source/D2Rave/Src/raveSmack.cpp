// raveSmack.cpp - Smacker videos (the game's .smk panels) through two RAVE textures.
//
// File name attested: __FILE__ "raveSmack.cpp" (string 10353a02) in the buffer allocations.
// The unit runs 102b6164-102b7368 and ends with __sinit_raveSmack_cpp at 102b7358. Smacker is
// linked statically into the PPC game (SmackOpen 102c9684 ... SmackClose 102c9ee0).
//
// Reconstructed here: RaveCloseSmacker, sRaveSmackFree, sRaveSmackInit. Not yet: the frame
// upload (sRaveDrawSmackerFrame 102b6164), RavePlaySmacker (102b6834) and RaveDecodeSmacker
// (102b6ba8); see d2-surveys/macppc/d2rave/recon.md.

#include "Source/D2Rave/Src/raveInternal.h"

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

extern "C" void SmackClose(void * pSmack);  // 102c9ee0

namespace D2Rave {

namespace {
const char sgszRaveSmackFile[] = "raveSmack.cpp";

// The leading fields of D2gfx's Smacker context as this unit reads them.
struct RaveSmackerContext {
    /* 0x00 */ void * pSmack;
    /* 0x04 */ char nKind;  // 's' while a Smacker file is open
};
} // namespace

uint8_t * gpRaveSmackBuffer;
uint8_t * gpRaveSmackLineBuffer;
TQATexture * gpRaveSmackTextures[2];
RaveVertex gRaveSmackVertices[4];

// Windows slot 18.
// mac ppc classic/1.09d: 102b67d8 (1.00: 1007ce10)
void RaveCloseSmacker(D2SmackerContextStrc * pContext) {
    RaveSmackerContext * pSmacker = reinterpret_cast<RaveSmackerContext *>(pContext);
    if (pSmacker->nKind == 's' && pSmacker->pSmack != nullptr) {
        SmackClose(pSmacker->pSmack);
        pSmacker->pSmack = nullptr;
    }
    pSmacker->nKind = 0;
}

// mac ppc classic/1.09d: 102b7184 (1.00: 1007d228)
void sRaveSmackFree() {
    if (gpRaveSmackBuffer != nullptr) {
        SMemFree(gpRaveSmackBuffer, sgszRaveSmackFile, 82, 0);
        gpRaveSmackBuffer = nullptr;
    }
    if (gpRaveSmackLineBuffer != nullptr) {
        SMemFree(gpRaveSmackLineBuffer, sgszRaveSmackFile, 88, 0);
        gpRaveSmackLineBuffer = nullptr;
    }
    QATextureDelete(gpRaveEngine, gpRaveSmackTextures[0]);
    QATextureDelete(gpRaveEngine, gpRaveSmackTextures[1]);
}

// Two 256x64 ARGB16 textures over a 64 KB staging buffer, and a white quad to draw them on.
// mac ppc classic/1.09d: 102b7234 (1.00: 1007d2d8)
void sRaveSmackInit() {
    gpRaveSmackBuffer = (uint8_t *)SMemAlloc(0x10000, sgszRaveSmackFile, 46, 0);
    gpRaveSmackLineBuffer = (uint8_t *)SMemAlloc(0x8000, sgszRaveSmackFile, 47, 0);
    TQAImage image;
    image.width = 0x100;
    image.height = 0x40;
    image.rowBytes = 0x200;
    image.pixmap = gpRaveSmackBuffer;
    for (int i = 0; i < 2; ++i) {
        QATextureNew(gpRaveEngine, 5, kQAPixel_ARGB16, &image, &gpRaveSmackTextures[i]);
    }
    for (int i = 0; i < 4; ++i) {
        gRaveSmackVertices[i].nRed = 0xff;
        gRaveSmackVertices[i].nGreen = 0xff;
        gRaveSmackVertices[i].nBlue = 0xff;
        gRaveSmackVertices[i].nAlpha = 0xff;
    }
}

} // namespace D2Rave

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
