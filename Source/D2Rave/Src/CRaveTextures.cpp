// CRaveTextures.cpp - the RAVE texture cache (see CRaveTextures.h for the naming evidence).
//
// The unit runs 102bba50-102bc040 and ends with __sinit_CRaveTextures_cpp at 102bc034. Its
// first function, ConvertPixels, is a table target with no Ghidra function; it is read from the
// disassembly.

#include "Source/D2Rave/Src/CRaveTextures.h"
#include "Source/D2Rave/Src/raveInternal.h"

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

using namespace D2Rave;

// Converts 8-bit palette indices into ARGB1555 texels in the staging buffer: the source rows
// land in the bottom nSrcHeight rows of an nTexHeight-row texture (rowBytes 0x200). Before each
// row the cache lines from the end of the source pixels to the texture width are zeroed (dcbz),
// then the pixels are converted four at a time.
// mac ppc classic/1.09d: 102bba50 (no 1.00 counterpart: 1.00 converts inline)
void CRaveTextures::ConvertPixels(int nTexWidth, int nTexHeight, int nSrcWidth, int nSrcHeight, uint8_t * pSrc) {
    uint8_t * pRow = pStaging + ((nTexHeight - nSrcHeight) << 9);
    const int nGroups = nSrcWidth >> 2;
    const int nClearFrom = (nSrcWidth << 1) & ~0x1f;
    const int nTexBytes = nTexWidth << 1;
    for (int nRow = 0; nRow < nSrcHeight; ++nRow) {
        if (nClearFrom < nTexBytes) {
            const unsigned int nLines = (unsigned int)(nTexBytes + 0x1f - nClearFrom) >> 5;
            for (unsigned int nLine = 0; nLine < nLines; ++nLine) {
                uint32_t * pLine = (uint32_t *)(pRow + nClearFrom + (nLine << 5));
                for (int n = 0; n < 8; ++n) {
                    pLine[n] = 0;
                }
            }
        }
        uint16_t * pOut = (uint16_t *)pRow;
        pRow += 0x200;
        for (int nGroup = 0; nGroup < nGroups; ++nGroup) {
            // Big-endian: the first pixel is the high byte of the word.
            const uint32_t dwIndices = *(const uint32_t *)pSrc;
            pSrc += 4;
            pOut[0] = gwRavePaletteARGB1555[(dwIndices >> 24) & 0xff];
            pOut[1] = gwRavePaletteARGB1555[(dwIndices >> 16) & 0xff];
            pOut[2] = gwRavePaletteARGB1555[(dwIndices >> 8) & 0xff];
            pOut[3] = gwRavePaletteARGB1555[dwIndices & 0xff];
            pOut += 4;
        }
    }
}

// mac ppc classic/1.09d: 102bbd4c (1.00: 10081b3c)
bool CRaveTextures::IsTextureBound(D2TextureNodeStrc * pNode) {
    return pNode->pTexture == gpRaveDrawContext->getPtr(gpRaveDrawContext, kQATag_Texture);
}

// mac ppc classic/1.09d: 102bbdac (1.00: 10081b9c)
void CRaveTextures::BindTexture(D2TextureNodeStrc * pNode) {
    gpRaveDrawContext->setPtr(gpRaveDrawContext, kQATag_Texture, pNode->pTexture);
}

// mac ppc classic/1.09d: 102bbde4 (1.00: 10081bd4)
void CRaveTextures::ReleaseTexture(D2TextureNodeStrc * pNode) {
    if (pNode->pTexture != nullptr) {
        QATextureDelete(gpRaveEngine, pNode->pTexture);
        pNode->pTexture = nullptr;
    }
}

// Creates the RAVE texture from the staging pixels and detaches it (RAVE copies the pixels).
// Returns false only when RAVE is out of memory, so the cache can evict and retry.
// mac ppc classic/1.09d: 102bbe30 (1.00: 10081c20)
bool CRaveTextures::UploadTexture(D2TextureNodeStrc * pNode, void * pPixels) {
    TQAImage image;
    image.width = pNode->nWidth;
    image.height = pNode->nHeight;
    image.rowBytes = 0x200;
    image.pixmap = pPixels;
    TQAError nError = QATextureNew(gpRaveEngine, 5, kQAPixel_ARGB16, &image, &pNode->pTexture);
    if (nError != kQAOutOfMemory) {
        RaveCheckError(nError);
        RaveCheckError(QATextureDetach(gpRaveEngine, pNode->pTexture));
    }
    return nError != kQAOutOfMemory;
}

// mac ppc classic/1.09d: 102bbed0 (1.00: 10081cc0)
int CRaveTextures::GetFreeMemory(bool bExcludeReserve) {
    int nMemory = 0;
    QAEngineGestalt(gpRaveEngine, kQAGestalt_FastTextureMemory, &nMemory);
    if (bExcludeReserve) {
        nMemory -= nReserveBytes;
    }
    return nMemory;
}

// The deleting destructor at 102bbf40 (1.00: 10081d30) resets the vtable, releases every entry,
// runs the base destructor and frees the object when asked to.
// mac ppc classic/1.09d: 102bbf40 (1.00: 10081d30)
CRaveTextures::~CRaveTextures() {
    ReleaseAll();
}

// 256x256 maximum, 0x200 bytes per row, no fixed budget: the budget is the engine's fast
// texture memory, and the cache works in 16-bit texels.
// mac ppc classic/1.09d: 102bbfb4 (1.00: 10081da4)
CRaveTextures::CRaveTextures() : CD2Textures(0x100, 0x100, 0x200, 0) {
    nMemoryBudget = GetFreeMemory(false);
    Init(2);
}

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
