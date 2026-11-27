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

// 1.14d mac: 002ef904
void FOG_BuildGammaRamp(int nGamma, double fContrast, int* pTable, int nStep, int nCount, int nMax);

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
