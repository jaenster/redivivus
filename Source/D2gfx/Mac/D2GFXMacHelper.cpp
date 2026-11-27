// D2GFXMacHelper.cpp - the surface and palette slots of the Mac renderer's helper table.
//
// 1.14d Intel Mac (DiabloII_macho). No __FILE__ names the unit: the Mac renderer's helper
// table (D2GfxHelperStrc) slots and the tile code sit among D2gfx's functions, and which
// translation unit each belongs to follows from direct versus indirect (PIC pointer cell)
// access to the globals alone. See Source/D2gfx/Mac/README.md.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC`; cmake/mac.cmake globs
// this folder.
#include "D2GFXMac.h"

#if !defined(_WIN32) && D2_OS_MAC


namespace D2gfx::D2GFX {
// 1.14d mac: 002b19a2
void D2GFX_InitializeWindowSize(void);
}
using D2gfx::D2GFX::D2GFX_InitializeWindowSize;
namespace D2gfx::D2GFX {
// 1.14d mac: 002b51ca
void D2GFX_SetPaletteTable(uint8_t** ppPaletteTables);
}
using D2gfx::D2GFX::D2GFX_SetPaletteTable;
void FUN_002b36b6(void);
void FUN_002b35e9(int nWidth, int nHeight);

// 1.14d mac: 0053fd74
int g_nRowTableWidth;
// 1.14d mac: 0053fd78
// Three pointers, then the 49 shift tables.
uint8_t* g_apPaletteTables[72];
// 1.14d mac: 005c8a6c
MacSurface g_tSurface;
// 1.14d mac: 005c8a7c
uint8_t** g_ppPaletteTables;
// 1.14d mac: 005c8a80
uint8_t g_abShiftTables[49][256];

// 1.14d mac: 002b18a8
// Slot 0 of the helper table: remembers the surface and rebuilds the row table when its
// pitch changes.
void fpFillYBufferTable(void* pBits, int nPitch, int nHeight, int n0C) {
    g_tSurface.pBits = pBits;
    g_tSurface.nPitch = nPitch;
    g_tSurface.nHeight = nHeight;
    g_tSurface.n0C = n0C;
    if (g_nRowTableWidth != nPitch) {
        g_tSurface.nPitch = nPitch;
        g_nRowTableWidth = nPitch;
        FUN_002b36b6();
    }
    if (g_nMacScreenSide != -1) {
        FUN_002b35e9(nPitch, nHeight);
    }
}

// 1.14d mac: 002b1913
// Takes a copy of the palette-table set and of its 49 shift tables.
void PALETTE_InitColorLookupTables(uint8_t** ppPaletteTables) {
    SMemCopy(g_apPaletteTables, ppPaletteTables, sizeof(g_apPaletteTables));
    g_ppPaletteTables = g_apPaletteTables;
    for (int i = 0; i < 49; i++) {
        SMemCopy(g_abShiftTables[i], g_apPaletteTables[i + 3], sizeof(g_abShiftTables[i]));
    }
}

// 1.14d mac: 002b1983
// Applies the stored palette-table set again.
void FUN_002b1983(void) {
    D2GFX_SetPaletteTable(g_apPaletteTables);
}

#endif
