// ravePerspective.cpp - the perspective projection slots and their reciprocal table.
//
// File name attested: __FILE__ "ravePerspective.cpp" (string 103539ee) in the table's
// allocation. The unit runs 102b5db8-102b6164 and ends with __sinit_ravePerspective_cpp at
// 102b6158. The fixed-point constants are the same as the Windows renderers' perspective code.

#include "Source/D2Rave/Src/raveInternal.h"

#if !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC

namespace D2Rave {

namespace {
const char sgszRavePerspectiveFile[] = "ravePerspective.cpp";
int snRavePerspectiveCenterY;      // 1037c814
int snRavePerspectiveCenterX;      // 1037c818
int * spnRavePerspectiveTable;     // 1037c81c, 0x2000 entries
} // namespace

int gnRavePerspectiveFactor;       // TOC slot 103759a8 (1052f57c)

// Windows slot 27; asm only in the database.
// mac ppc classic/1.09d: 102b5db8 (1.00: 1007bfd8)
void RaveSetDefaultPerspectiveFactor() {
    gnRavePerspectiveFactor = 0x100;
}

// Windows slot 26: projects a world position and sets the perspective factor for it (scaled
// by 1.2 when bScale, else by 1.1). nUnused is not read. Asm only in the database.
// mac ppc classic/1.09d: 102b5dc8 (1.00: 1007bfe8)
void RavePerspectiveScalePosition(int nX, int nY, int nUnused, int * pX, int * pY, int bScale) {
    (void)nUnused;
    const int nDX = ((nX - snRavePerspectiveCenterX) * 0x18) >> 0x10;
    const int nDY = ((nY - snRavePerspectiveCenterY) * 0x18) >> 0x10;
    const int nSum = nDX * 0x2d4 + nDY * 0x2d4;
    const int nDepth = (-nSum << 9) >> 10;
    const int nIndex = (nSum * 0x376) >> 0x14;
    const int nScale = spnRavePerspectiveTable[(nIndex + 0x1000) & 0x1fff];
    *pX = gnRaveScreenWidth / 2 + (((nDX * -0x2d4 + nDY * 0x2d4) * nScale) >> 0x14);
    const int nOffsetY = (nDepth * nScale) >> 0x14;
    *pY = nOffsetY + (gnRaveScreenHeight / 2 - 0x10);
    gnRavePerspectiveFactor =
        nOffsetY - (((nDepth + 0x37400) * spnRavePerspectiveTable[(nIndex + 0x1080) & 0x1fff]) >> 0x14);
    if (bScale) {
        gnRavePerspectiveFactor = (gnRavePerspectiveFactor * 0x4cc) >> 10;
    } else {
        gnRavePerspectiveFactor = (gnRavePerspectiveFactor * 0x466) >> 10;
    }
}

// Windows slot 25.
// mac ppc classic/1.09d: 102b5ed4 (1.00: 1007c194)
void RaveAdjustPerspectivePosition(int nX, int nY, int nBias, int * pX, int * pY) {
    const int nDX = ((nX - snRavePerspectiveCenterX) * 0x18) >> 0x10;
    const int nDY = ((nY - snRavePerspectiveCenterY) * 0x18) >> 0x10;
    const int nSum = nDX * 0x2d4 + nDY * 0x2d4;
    const int nScale = spnRavePerspectiveTable[(((nSum * 0x376 + nBias * 0x99800) >> 0x14) + 0x1000) & 0x1fff];
    *pX = gnRaveScreenWidth / 2 + (((nDX * -0x2d4 + nDY * 0x2d4) * nScale) >> 0x14);
    *pY = gnRaveScreenHeight / 2 + ((((nSum * -0x200 + nBias * 0x109a08) >> 10) * nScale) >> 0x14) - 0x20;
}

// Windows slot 24; asm only in the database.
// mac ppc classic/1.09d: 102b5f8c (1.00: 1007c310)
void RaveSetPerspectiveScale(int nCenterX, int nCenterY) {
    snRavePerspectiveCenterX = nCenterX;
    snRavePerspectiveCenterY = nCenterY;
}

// The D2Gfx settings' first dword when its second is set (the return is undefined otherwise;
// the caller only uses it in that case).
// mac ppc classic/1.09d: 102b5f98 (1.00: 1007c31c)
int sRaveGetCurrentTexture() {
    const int * pSettings = (const int *)gpRaveSettings;
    if (pSettings[1] != 0) {
        return pSettings[0];
    }
    return 0;
}

// The reciprocal table: entry i is 0x2ee000 / (clamp(i - 0x1000, -2999, 2999) - 3000).
// mac ppc classic/1.09d: 102b5fbc (1.00: 1007c340)
void sRavePerspectiveInit() {
    for (int i = 0; i < 0x2000; ++i) {
        int nValue = i - 0x1000;
        if (nValue < -0xbb6) {
            nValue = -2999;
        }
        if (nValue > 0xbb6) {
            nValue = 2999;
        }
        spnRavePerspectiveTable[i] = 0x2ee000 / (nValue - 3000);
    }
    gnRavePerspectiveFactor = 0x100;
}

// mac ppc classic/1.09d: 102b60d8 (1.00: 1007c5b0)
void sRavePerspectiveFree() {
    if (spnRavePerspectiveTable != nullptr) {
        SMemFree(spnRavePerspectiveTable, sgszRavePerspectiveFile, 62, 0);
        spnRavePerspectiveTable = nullptr;
    }
}

// mac ppc classic/1.09d: 102b611c (1.00: 1007c678)
void sRavePerspectiveAlloc() {
    spnRavePerspectiveTable = (int *)SMemAlloc(0x8000, sgszRavePerspectiveFile, 55, 0);
}

} // namespace D2Rave

#endif // !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC
