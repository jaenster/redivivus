// raveVertex.cpp - the global light and the 256x256 light multiplication table.
//
// File name attested: __FILE__ "raveVertex.cpp" (string 10353a1f) in the table's allocation.
// The unit runs 102b95fc-102b98bc and ends with __sinit_raveVertex_cpp at 102b98b0.

#include "Source/D2Rave/Src/raveInternal.h"

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

namespace D2Rave {

namespace {
const char sgszRaveVertexFile[] = "raveVertex.cpp";
} // namespace

uint8_t gnRaveGlobalBlue;    // 1037c898
uint8_t gnRaveGlobalGreen;   // 1037c899
uint8_t gnRaveGlobalRed;     // 1037c89a
uint8_t * gpRaveLightTable;  // TOC slot 103759a4 (1052f648): [intensity][value] = value * intensity / 255

// Windows slot 30.
// mac ppc classic/1.09d: 102b95fc (1.00: 1007f314)
void RaveSetGlobalLight(uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
    gnRaveGlobalRed = nRed;
    gnRaveGlobalGreen = nGreen;
    gnRaveGlobalBlue = nBlue;
}

// Lights a vertex with a packed colour: the low byte is the intensity, bytes 1-3 the red, green
// and blue to scale.
// mac ppc classic/1.09d: 102b960c (1.00: 1007f324)
void sRaveApplyLight(RaveVertex * pVertex, uint32_t dwLight) {
    const uint8_t * pRow = gpRaveLightTable + (dwLight & 0xff) * 0x100;
    pVertex->nRed = pRow[(dwLight >> 8) & 0xff];
    pVertex->nGreen = pRow[(dwLight >> 16) & 0xff];
    pVertex->nBlue = pRow[dwLight >> 24];
}

// Lights a vertex with the global light at the intensity in the low byte.
// mac ppc classic/1.09d: 102b9644 (1.00: 1007f35c)
void sRaveApplyGlobalLight(RaveVertex * pVertex, uint32_t dwLight) {
    const uint8_t * pRow = gpRaveLightTable + (dwLight & 0xff) * 0x100;
    const uint8_t nGreen = pRow[gnRaveGlobalGreen];
    const uint8_t nBlue = pRow[gnRaveGlobalBlue];
    pVertex->nRed = pRow[gnRaveGlobalRed];
    pVertex->nGreen = nGreen;
    pVertex->nBlue = nBlue;
}

// mac ppc classic/1.09d: 102b967c (1.00: 1007f394)
void sRaveVertexInit() {
    for (int nIntensity = 0; nIntensity < 0x100; ++nIntensity) {
        for (int nValue = 0; nValue < 0x100; ++nValue) {
            gpRaveLightTable[nIntensity * 0x100 + nValue] = (uint8_t)((nValue * nIntensity) / 0xff);
        }
    }
}

// mac ppc classic/1.09d: 102b9824 (1.00: 1007f53c)
void sRaveVertexFree() {
    if (gpRaveLightTable != nullptr) {
        SMemFree(gpRaveLightTable, sgszRaveVertexFile, 30, 0);
        gpRaveLightTable = nullptr;
    }
}

// mac ppc classic/1.09d: 102b9874 (1.00: 1007f58c)
void sRaveVertexAlloc() {
    gpRaveLightTable = (uint8_t *)SMemAlloc(0x10000, sgszRaveVertexFile, 23, 0);
}

} // namespace D2Rave

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
