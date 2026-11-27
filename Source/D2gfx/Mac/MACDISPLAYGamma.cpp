// MACDISPLAYGamma.cpp - the display gamma of the Mac D2gfx: the original transfer table is
// saved once, a byte table built from the game's gamma and contrast replaces it, and the
// saved table is put back on exit.
//
// 1.14d Intel Mac (DiabloII_macho) 002f7324..002f744e. No __FILE__ names the unit.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC`.
#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <ApplicationServices/ApplicationServices.h>
#include <stdint.h>
#include <math.h>

void fp_fatal(const char* szFormat, ...);
// 1.14d mac: 002ef085
void* FOG_GetFramePointer(void);

#define FRAMEPOINTER_FATAL(nLine) fp_fatal("Unrecoverable internal error %08x line %d", FOG_GetFramePointer(), nLine)

// 1.14d mac: 002ef904
// Fills nEntries values of nEntrySize bytes (1, 2 or 4) with a power curve of 100 / nGamma blended with
// a linear term by fContrast percent, scaled to nMaxValue. The binary places it in Fog (between
// FOG_GetFramePointer and FOG_StartLogThread); no source file names it.
void FOG_BuildGammaRamp(int nGamma, double fContrast, int* pRamp, int nEntrySize, int nEntries, int nMaxValue) {
    if (nEntrySize != 1 && nEntrySize != 2 && nEntrySize != 4) {
        FRAMEPOINTER_FATAL(14);
    }
    if (nGamma < 0) {
        FRAMEPOINTER_FATAL(15);
    }
    if (!(fContrast >= 0.0 && 100.0 >= fContrast)) {
        FRAMEPOINTER_FATAL(16);
    }
    double fLastIndex = (double)nEntries - 1.0;
    uint8_t* pByte = (uint8_t*)pRamp;
    uint16_t* pWord = (uint16_t*)pRamp;
    int* pDword = pRamp;
    for (double fIndex = 0.0; fIndex <= fLastIndex; fIndex += 1.0) {
        double fPower = pow(fIndex / fLastIndex, 100.0 / (double)nGamma);
        double fLinear = (double)nGamma / 100.0 * fIndex * (double)nMaxValue / fLastIndex;
        if (fLinear > (double)nMaxValue) {
            fLinear = (double)nMaxValue;
        }
        int nValue = (int)((fPower * (double)nMaxValue * (100.0 - fContrast) + fLinear * fContrast) / 100.0);
        if (nEntrySize == 4) {
            *pDword++ = nValue;
        } else if (nEntrySize == 2) {
            *pWord++ = (uint16_t)nValue;
        } else if (nEntrySize == 1) {
            *pByte++ = (uint8_t)nValue;
        }
    }
}

// 1.14d mac: 00553894
static bool s_bTransferTableSaved;
// 1.14d mac: 00553898
static CGGammaValue s_afSavedRed[256];
// 1.14d mac: 00553c98
static CGGammaValue s_afSavedGreen[256];
// 1.14d mac: 00554098
static CGGammaValue s_afSavedBlue[256];
// 1.14d mac: 00554498
static uint32_t s_nSavedTableCount;

// 1.14d mac: 002f7324
void MACDISPLAY_SetGammaRamp(uint32_t nDisplayId, int nGamma, int nContrast) {
    int abGammaTable[64];
    if (!s_bTransferTableSaved) {
        CGGetDisplayTransferByTable(nDisplayId, 0x100, s_afSavedRed, s_afSavedGreen, s_afSavedBlue, &s_nSavedTableCount);
        s_bTransferTableSaved = true;
    }
    FOG_BuildGammaRamp(nGamma, (double)nContrast, abGammaTable, 1, 0x100, 0xff);
    CGSetDisplayTransferByByteTable(nDisplayId, 0x100, (const CGByteValue*)abGammaTable,
                                    (const CGByteValue*)abGammaTable, (const CGByteValue*)abGammaTable);
}

// 1.14d mac: 002f73f8
void MACDISPLAY_RestoreGammaRamp(uint32_t nDisplayId) {
    if (s_bTransferTableSaved) {
        CGSetDisplayTransferByTable(nDisplayId, s_nSavedTableCount, s_afSavedRed, s_afSavedGreen, s_afSavedBlue);
        s_bTransferTableSaved = false;
    }
}

#endif
