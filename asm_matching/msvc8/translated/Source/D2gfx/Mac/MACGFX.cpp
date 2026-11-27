// MACGFX.cpp - the display-mode enumeration of the Mac D2gfx: the 32-bit CoreGraphics modes of
// the main display, de-duplicated and sorted into a 200-entry table, with the best mode for the
// two stock resolutions marked.
//
// 1.14d Intel Mac (DiabloII_macho) 002b603a..002b6584. No __FILE__ names the unit; it sits
// between the D2gfx forwarders and the file I/O wrappers. See Source/D2gfx/Mac/README.md.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC`.
#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <ApplicationServices/ApplicationServices.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum {
    MACGFX_MODE_COUNT = 200,
    MACGFX_TARGET_COUNT = 2,
};

// One display mode, twelve bytes.
struct MacDisplayMode {
    uint32_t nSize;                   // bit 0 a flag (IO flag 0x800), bits 1..15 the width, 16..31 the height
    int32_t nRefreshRate;             // 16.16
    uint8_t anRating[2];              // per target: 3 or 4 a candidate, 1 chosen, 2 the one it replaced
    uint8_t abSelected[2];            // per target: this is the mode the target takes
};

// 1.14d mac: 00548a90
static CGDirectDisplayID s_nDisplayID;
// 1.14d mac: 00548a94
static MacDisplayMode s_atModes[MACGFX_MODE_COUNT];

// 1.14d mac: 00333ef8
// The two stock resolutions the modes are rated against.
static const int s_anTargetWidth[MACGFX_TARGET_COUNT] = {640, 800};
// 1.14d mac: 00333f00
static const int s_anTargetHeight[MACGFX_TARGET_COUNT] = {480, 600};

// The mode the target has selected, or else the first one rated 3 or 4 for it; NULL when
// there is none.
static MacDisplayMode* FUN_002b64d5(int nTarget);

// 1.14d mac: 002b652b
// The qsort comparator: area, then refresh rate, then the flag bit.
int MACGFX_CompareDisplayModes(const MacDisplayMode* pModeA, const MacDisplayMode* pModeB) {
    uint32_t dwModeB = pModeB->nSize;
    uint32_t dwModeA = pModeA->nSize;
    int nResult = (dwModeA >> 1 & 0x7fff) * (dwModeA >> 0x10) - (dwModeB >> 1 & 0x7fff) * (dwModeB >> 0x10);
    if (nResult == 0 && (nResult = pModeA->nRefreshRate - pModeB->nRefreshRate, nResult == 0)) {
        nResult = 0;
        if ((dwModeA & 1) != (dwModeB & 1)) {
            nResult = (dwModeA & 1) * 2 - 1;
        }
    }
    return nResult;
}

// 1.14d mac: 002b603a
void MACGFX_EnumerateDisplayModes(int* pnDisplayModule, int* pnOption4, int* pnOption5, uint32_t* pDisplayID,
                                  MacDisplayMode* pMode640x480, MacDisplayMode* pMode800x600) {
    s_nDisplayID = CGMainDisplayID();
    bzero(s_atModes, sizeof(s_atModes));
    CGMainDisplayID();
    CFArrayRef pModes = CGDisplayCopyAllDisplayModes(s_nDisplayID, 0);
    int nCount = 0;
    if (pModes != 0) {
        int nModes = CFArrayGetCount(pModes);
        if (nModes >= 1) {
            for (int i = 0; i != nModes; i++) {
                CGDisplayModeRef pMode = (CGDisplayModeRef)CFArrayGetValueAtIndex(pModes, i);
                if (pMode == 0) {
                    continue;
                }
                CFStringRef szEncoding = CGDisplayModeCopyPixelEncoding(pMode);
                if (CFStringGetLength(szEncoding) != 0x20) {
                    continue;
                }
                MacDisplayMode* pEntry = &s_atModes[nCount];
                uint32_t nFlags = CGDisplayModeGetIOFlags(pMode);
                pEntry->nSize = pEntry->nSize & 0xfffffffe | nFlags >> 0xb & 1;
                size_t nWidth = CGDisplayModeGetWidth(pMode);
                pEntry->nSize = nWidth * 2 & 0xffff | pEntry->nSize & 0xffff0001;
                size_t nHeight = CGDisplayModeGetHeight(pMode);
                ((uint16_t*)&pEntry->nSize)[1] = (uint16_t)nHeight;
                double fRate = CGDisplayModeGetRefreshRate(pMode);
                pEntry->nRefreshRate = (int)(fRate * 65536.0);
                *(uint16_t*)pEntry->anRating = 0x101;
                *(uint16_t*)pEntry->abSelected = 0;
                if (nCount != 0) {
                    MacDisplayMode* pPrev = &s_atModes[nCount - 1];
                    uint32_t nDiff = pEntry->nSize ^ pPrev->nSize;
                    if ((nDiff & 0xfffe) == 0 && pPrev->nSize >> 0x10 == pEntry->nSize >> 0x10 &&
                        pPrev->nRefreshRate == (int)(fRate * 65536.0) && (nDiff & 1) == 0) {
                        continue;
                    }
                }
                nCount++;
            }
        }
        CFRelease(pModes);
    }
    s_atModes[nCount].nRefreshRate = 0;
    s_atModes[nCount].nSize = 0;
    qsort(s_atModes, nCount, sizeof(MacDisplayMode),
          (int (*)(const void*, const void*))MACGFX_CompareDisplayModes);
    for (int nTarget = 0; nTarget < MACGFX_TARGET_COUNT; nTarget++) {
        int nBestScore = 0x7fffffff;
        MacDisplayMode* pBest = 0;
        MacDisplayMode* pMode = s_atModes;
        uint32_t nSize = 0;
        bool bFound = false;
        for (int i = 0; i < MACGFX_MODE_COUNT; i++, pMode++) {
            nSize = pMode->nSize;
            int nWidth = nSize >> 1 & 0x7fff;
            if (s_anTargetWidth[nTarget] <= nWidth && s_anTargetHeight[nTarget] <= (int)(nSize >> 0x10)) {
                int nDelta = nWidth * (nSize >> 0x10) - s_anTargetHeight[nTarget] * s_anTargetWidth[nTarget];
                int nScore = -nDelta;
                if (0 < nDelta) {
                    nScore = nDelta;
                }
                int nPenalised = nScore + 100;
                if ((nSize & 1) == 0) {
                    nPenalised = nScore;
                }
                int nRate = (pMode->nRefreshRate >> 0x10) - 0x48;
                if (nRate < 0) {
                    nRate = 0x48 - (pMode->nRefreshRate >> 0x10);
                }
                nRate = nRate + nPenalised;
                if (nRate < nBestScore) {
                    pBest = pMode;
                    nBestScore = nRate;
                    if (nRate == 0) {
                        bFound = true;
                        break;
                    }
                }
            }
        }
        if (bFound || pBest != 0) {
            if (!bFound) {
                nSize = pBest->nSize;
            } else {
                pBest = pMode;
                nSize = pBest->nSize;
            }
            uint32_t nBestRate = pBest->nRefreshRate;
            uint32_t nBestWidth = nSize >> 1 & 0x7fff;
            MacDisplayMode* pSame = 0;
            MacDisplayMode* pScan = s_atModes;
            for (int i = MACGFX_MODE_COUNT; i != 0; i--, pScan++) {
                uint32_t nScanSize = pScan->nSize;
                if ((nScanSize & 1) == (nSize & 1) && (nScanSize >> 1 & 0x7fff) == nBestWidth &&
                    nScanSize >> 0x10 == nSize >> 0x10 && (uint32_t)pScan->nRefreshRate == nBestRate) {
                    pSame = pScan;
                }
                if ((uint8_t)(pScan->anRating[nTarget] - 3) < 2) {
                    pScan->anRating[nTarget] = 1;
                }
            }
            pScan = s_atModes;
            if (pSame != 0) {
                for (int i = 0; i != 2; i++) {
                    if (pSame->anRating[i] == 2) {
                        pSame->anRating[i] = 1;
                    }
                }
                pSame->anRating[nTarget] = 3;
                pScan = s_atModes;
            }
            for (int i = MACGFX_MODE_COUNT; i != 0; i--, pScan++) {
                uint32_t nScanSize = pScan->nSize;
                if ((nScanSize & 1) == (nSize & 1) && (nScanSize >> 1 & 0x7fff) == nBestWidth &&
                    nScanSize >> 0x10 == nSize >> 0x10 && (uint32_t)pScan->nRefreshRate == nBestRate) {
                    pScan->abSelected[nTarget] = 1;
                } else {
                    pScan->abSelected[nTarget] = 0;
                }
            }
        }
    }
    if (pnDisplayModule != 0) {
        *pnDisplayModule = 3;
    }
    if (pnOption4 != 0) {
        *pnOption4 = -1;
    }
    if (pnOption5 != 0) {
        *pnOption5 = 1;
    }
    if (pDisplayID != 0) {
        *pDisplayID = s_nDisplayID;
    }
    if (pMode640x480 != 0) {
        MacDisplayMode tDefault = {0x01e00500, 0, {0, 0}, {0, 0}};
        MacDisplayMode* pFound = FUN_002b64d5(0);
        MacDisplayMode* pUse = &tDefault;
        if (pFound != 0) {
            pUse = pFound;
        }
        pMode640x480->nSize = pUse->nSize;
        pMode640x480->nRefreshRate = pUse->nRefreshRate;
    }
    if (pMode800x600 != 0) {
        MacDisplayMode tDefault = {0x02580640, 0, {0, 0}, {0, 0}};
        MacDisplayMode* pFound = FUN_002b64d5(1);
        MacDisplayMode* pUse = &tDefault;
        if (pFound != 0) {
            pUse = pFound;
        }
        pMode800x600->nSize = pUse->nSize;
        pMode800x600->nRefreshRate = pUse->nRefreshRate;
    }
}

// 1.14d mac: 002b64d5
static MacDisplayMode* FUN_002b64d5(int nTarget) {
    for (int i = 0; i < MACGFX_MODE_COUNT; i++) {
        if (s_atModes[i].abSelected[nTarget] != 0) {
            return &s_atModes[i];
        }
    }
    for (int i = 0; i < MACGFX_MODE_COUNT; i++) {
        if ((uint8_t)(s_atModes[i].anRating[nTarget] - 3) < 2) {
            return &s_atModes[i];
        }
    }
    return 0;
}

#endif
