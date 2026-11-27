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
    uint32_t bIOFlag800 : 1;          // the mode's IO flag 0x800
    uint32_t nWidth : 15;
    uint32_t nHeight : 16;
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
int MACGFX_CompareDisplayModes(const void* pA, const void* pB) {
    const MacDisplayMode* pModeA = (const MacDisplayMode*)pA;
    const MacDisplayMode* pModeB = (const MacDisplayMode*)pB;
    int nResult = pModeA->nWidth * pModeA->nHeight - pModeB->nWidth * pModeB->nHeight;
    if (nResult == 0) {
        nResult = pModeA->nRefreshRate - pModeB->nRefreshRate;
        if (nResult == 0) {
            nResult = 0;
            if (pModeA->bIOFlag800 != pModeB->bIOFlag800) {
                nResult = pModeA->bIOFlag800 * 2 - 1;
            }
        }
    }
    return nResult;
}

static inline int MACGFX_SameMode(const MacDisplayMode* pModeA, const MacDisplayMode* pModeB) {
    return pModeA->bIOFlag800 == pModeB->bIOFlag800 && pModeA->nWidth == pModeB->nWidth &&
           pModeA->nHeight == pModeB->nHeight && pModeA->nRefreshRate == pModeB->nRefreshRate;
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
        for (int i = 0; i < nModes; i++) {
            CGDisplayModeRef pMode = (CGDisplayModeRef)CFArrayGetValueAtIndex(pModes, i);
            if (pMode == 0) {
                continue;
            }
            if (CFStringGetLength(CGDisplayModeCopyPixelEncoding(pMode)) != 32) {
                continue;
            }
            MacDisplayMode* pEntry = &s_atModes[nCount];
            pEntry->bIOFlag800 = CGDisplayModeGetIOFlags(pMode) >> 11;
            pEntry->nWidth = CGDisplayModeGetWidth(pMode);
            pEntry->nHeight = CGDisplayModeGetHeight(pMode);
            int nRefreshRate = (int)(CGDisplayModeGetRefreshRate(pMode) * 65536.0);
            pEntry->nRefreshRate = nRefreshRate;
            *(uint16_t*)pEntry->anRating = 0x101;
            *(uint16_t*)pEntry->abSelected = 0;
            if (nCount != 0) {
                MacDisplayMode* pPrev = &s_atModes[nCount - 1];
                if (pPrev->nWidth == pEntry->nWidth && pPrev->nHeight == pEntry->nHeight &&
                    pPrev->nRefreshRate == nRefreshRate && pPrev->bIOFlag800 == pEntry->bIOFlag800) {
                    continue;
                }
            }
            nCount++;
        }
        CFRelease(pModes);
    }
    s_atModes[nCount].nRefreshRate = 0;
    *(uint32_t*)&s_atModes[nCount] = 0;
    qsort(s_atModes, nCount, sizeof(MacDisplayMode), MACGFX_CompareDisplayModes);
    for (int nTarget = 0; nTarget < MACGFX_TARGET_COUNT; nTarget++) {
        int nTargetWidth = s_anTargetWidth[nTarget];
        int nTargetHeight = s_anTargetHeight[nTarget];
        int nTargetArea = nTargetHeight * nTargetWidth;
        int nBestScore = 0x7fffffff;
        MacDisplayMode* pBest = 0;
        for (int i = 0; i < MACGFX_MODE_COUNT; i++) {
            MacDisplayMode* pMode = &s_atModes[i];
            if (pMode->nWidth >= nTargetWidth && pMode->nHeight >= nTargetHeight) {
                int nScore = abs((int)(pMode->nWidth * pMode->nHeight) - nTargetArea);
                if (pMode->bIOFlag800) {
                    nScore += 100;
                }
                nScore += abs((pMode->nRefreshRate >> 16) - 72);
                if (nScore < nBestScore) {
                    nBestScore = nScore;
                    pBest = pMode;
                    if (nScore == 0) {
                        break;
                    }
                }
            }
        }
        if (pBest == 0) {
            continue;
        }
        int nBestHeight = pBest->nHeight;
        int nBestRate = pBest->nRefreshRate;
        int bBestFlag = pBest->bIOFlag800;
        int nBestWidth = pBest->nWidth;
        MacDisplayMode* pSame = 0;
        for (int i = 0; i < MACGFX_MODE_COUNT; i++) {
            MacDisplayMode* pMode = &s_atModes[i];
            if (pMode->bIOFlag800 == bBestFlag && pMode->nWidth == nBestWidth && pMode->nHeight == nBestHeight &&
                pMode->nRefreshRate == nBestRate) {
                pSame = pMode;
            }
            if (pMode->anRating[nTarget] == 3 || pMode->anRating[nTarget] == 4) {
                pMode->anRating[nTarget] = 1;
            }
        }
        if (pSame != 0) {
            for (int i = 0; i < MACGFX_TARGET_COUNT; i++) {
                if (pSame->anRating[i] == 2) {
                    pSame->anRating[i] = 1;
                }
            }
            pSame->anRating[nTarget] = 3;
        }
        for (int i = 0; i < MACGFX_MODE_COUNT; i++) {
            MacDisplayMode* pMode = &s_atModes[i];
            if (pMode->bIOFlag800 == bBestFlag && pMode->nWidth == nBestWidth && pMode->nHeight == nBestHeight &&
                pMode->nRefreshRate == nBestRate) {
                pMode->abSelected[nTarget] = 1;
            } else {
                pMode->abSelected[nTarget] = 0;
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
        MacDisplayMode tDefault = {0, 640, 480, 0, {0, 0}, {0, 0}};
        MacDisplayMode* pFound = FUN_002b64d5(0);
        MacDisplayMode* pUse = pFound != 0 ? pFound : &tDefault;
        pMode640x480->bIOFlag800 = pUse->bIOFlag800;
        pMode640x480->nWidth = pUse->nWidth;
        pMode640x480->nHeight = pUse->nHeight;
        pMode640x480->nRefreshRate = pUse->nRefreshRate;
    }
    if (pMode800x600 != 0) {
        MacDisplayMode tDefault = {0, 800, 600, 0, {0, 0}, {0, 0}};
        MacDisplayMode* pFound = FUN_002b64d5(1);
        MacDisplayMode* pUse = pFound != 0 ? pFound : &tDefault;
        pMode800x600->bIOFlag800 = pUse->bIOFlag800;
        pMode800x600->nWidth = pUse->nWidth;
        pMode800x600->nHeight = pUse->nHeight;
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
