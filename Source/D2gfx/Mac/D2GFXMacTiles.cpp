// D2GFXMacTiles.cpp - the tile blitters of the Mac software renderer's helper table: floor
// blocks (15 rows, plain or lit per pixel), run-length wall blocks (32 rows, remapped, shaded
// or lit), the blended roof and vis tiles, and the clip rectangles and row table they share.
//
// 1.14d Intel Mac (DiabloII_macho) 002b1a44..002b4975. No __FILE__ names the unit; the surface
// is reached through its PIC pointer cell, so these sit in a unit other than the one that
// holds fpFillYBufferTable (D2GFXMacHelper.cpp). See Source/D2gfx/Mac/README.md.
//
// A run-length row is (skip, count) byte pairs each followed by count pixels, ended by a
// 0,0 pair; the blitters unroll the copy as a fall-through switch on the count, as the
// original does.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC`.
#include "D2GFXMac.h"

#if !defined(_WIN32) && D2_OS_MAC

void fp_fatal(const char* szFormat, ...);
// 1.14d mac: 002ef085
void* FOG_GetFramePointer(void);

// The blend tables of the Mac renderer: an alpha of 0xC0 and up takes the first, 0x80 the
// second, 0x40 the third, below that nothing is drawn.
extern uint8_t** g_ppBlendTables;

// 1.14d mac: 00547e98
// Clip rectangles: [0..2] the bottom, left and right of the whole surface; [3] a flag,
// then bottom, left, right of the ground tile bounds and the bottom of its inner bounds, and
// the same set again for the wall tile bounds.
int g_anClip[17];
// 1.14d mac: 00547edc
// Row offsets from 32 rows above the surface; entry n + 32 is row n.
int g_anRowOffsets[732];
// 1.14d mac: 0053fe98
// The light grid: 32 columns by 32 rows by 32 shades.
uint8_t g_abLightGrid[0x8000];

// 1.14d mac: 003c8090
extern int g_anTileRowDest[15];
// 1.14d mac: 003c80cc
extern uint32_t g_anTileRowWidth[15];
// 1.14d mac: 003c8108
extern int g_anTileRowSrc[15];

#define FRAMEPOINTER_FATAL(nLine) fp_fatal("Unrecoverable internal error %08x line %d", FOG_GetFramePointer(), nLine)

// 1.14d mac: 002b1a44
// A 15-row uncompressed floor block through a remap table.
void FUN_002b1a44(const uint8_t* pSrc, uint8_t* pDest, const uint8_t* pRemap) {
    MacSurface* pSurface = &g_tSurface;
    int* pnDest = g_anTileRowDest;
    int* pnSrc = g_anTileRowSrc;
    uint32_t* pnWidth = g_anTileRowWidth;
    int nRowsLeft = 15;
    do {
        int nSrcOffset = *pnSrc;
        int nDestOffset = *pnDest;
        uint32_t nWidth = *pnWidth;
        for (uint32_t i = 0; i < nWidth; i++) {
            pDest[nDestOffset + i] = pRemap[pSrc[nSrcOffset + i]];
        }
        pDest += pSurface->nPitch;
        pnWidth++;
        pnSrc++;
        pnDest++;
        nRowsLeft--;
    } while (nRowsLeft != 0);
}

// 1.14d mac: 002b1ca9
// A 15-row uncompressed floor block lit per pixel: each pixel goes through the shade table
// with a light value from the grid, the light coordinates stepped per row.
void FUN_002b1ca9(const uint8_t* pSrc, uint8_t* pDest, int nLightX, int nLightXStep, int nLightY, int nLightYStep) {
    MacSurface* pSurface = &g_tSurface;
    int* pnDest = g_anTileRowDest;
    int* pnSrc = g_anTileRowSrc;
    uint32_t* pnWidth = g_anTileRowWidth;
    int nRowsLeft = 15;
    uint8_t* pDestRow = pDest;
    do {
        int nLightOffset = (nLightY >> 7) * 0x20 + (nLightX >> 7) * 0x400;
        int nDestOffset = *pnDest;
        int nSrcOffset = *pnSrc;
        uint32_t nWidth = *pnWidth;
        for (uint32_t i = 0; i < nWidth; i++) {
            pDestRow[nDestOffset + i] =
                g_abMacShadeTable[pSrc[nSrcOffset + i] + g_abLightGrid[nDestOffset + i + nLightOffset] * 0x100];
        }
        nLightY += nLightYStep;
        nLightX += nLightXStep;
        pDestRow += pSurface->nPitch;
        pnWidth++;
        pnSrc++;
        pnDest++;
        nRowsLeft--;
    } while (nRowsLeft != 0);
}

// 1.14d mac: 002b224c
// A run-length block through a remap table.
void FUN_002b224c(const uint8_t* pSrc, uint8_t* pDest, const uint8_t* pRemap, int nRows) {
    do {
        uint8_t* pDst = pDest;
        if (nRows == 0) {
            return;
        }
        while (true) {
            uint32_t nSkip = pSrc[0];
            uint32_t nRun = pSrc[1];
            if (pSrc[1] == 0 && pSrc[0] == 0) {
                break;
            }
            switch (nRun) {
            case 32:
                pDst[nSkip + 31] = pRemap[pSrc[33]];
            case 31:
                pDst[nSkip + 30] = pRemap[pSrc[32]];
            case 30:
                pDst[nSkip + 29] = pRemap[pSrc[31]];
            case 29:
                pDst[nSkip + 28] = pRemap[pSrc[30]];
            case 28:
                pDst[nSkip + 27] = pRemap[pSrc[29]];
            case 27:
                pDst[nSkip + 26] = pRemap[pSrc[28]];
            case 26:
                pDst[nSkip + 25] = pRemap[pSrc[27]];
            case 25:
                pDst[nSkip + 24] = pRemap[pSrc[26]];
            case 24:
                pDst[nSkip + 23] = pRemap[pSrc[25]];
            case 23:
                pDst[nSkip + 22] = pRemap[pSrc[24]];
            case 22:
                pDst[nSkip + 21] = pRemap[pSrc[23]];
            case 21:
                pDst[nSkip + 20] = pRemap[pSrc[22]];
            case 20:
                pDst[nSkip + 19] = pRemap[pSrc[21]];
            case 19:
                pDst[nSkip + 18] = pRemap[pSrc[20]];
            case 18:
                pDst[nSkip + 17] = pRemap[pSrc[19]];
            case 17:
                pDst[nSkip + 16] = pRemap[pSrc[18]];
            case 16:
                pDst[nSkip + 15] = pRemap[pSrc[17]];
            case 15:
                pDst[nSkip + 14] = pRemap[pSrc[16]];
            case 14:
                pDst[nSkip + 13] = pRemap[pSrc[15]];
            case 13:
                pDst[nSkip + 12] = pRemap[pSrc[14]];
            case 12:
                pDst[nSkip + 11] = pRemap[pSrc[13]];
            case 11:
                pDst[nSkip + 10] = pRemap[pSrc[12]];
            case 10:
                pDst[nSkip + 9] = pRemap[pSrc[11]];
            case 9:
                pDst[nSkip + 8] = pRemap[pSrc[10]];
            case 8:
                pDst[nSkip + 7] = pRemap[pSrc[9]];
            case 7:
                pDst[nSkip + 6] = pRemap[pSrc[8]];
            case 6:
                pDst[nSkip + 5] = pRemap[pSrc[7]];
            case 5:
                pDst[nSkip + 4] = pRemap[pSrc[6]];
            case 4:
                pDst[nSkip + 3] = pRemap[pSrc[5]];
            case 3:
                pDst[nSkip + 2] = pRemap[pSrc[4]];
            case 2:
                pDst[nSkip + 1] = pRemap[pSrc[3]];
            case 1:
                pDst[nSkip + 0] = pRemap[pSrc[2]];
            }
            pSrc += nRun + 2;
            pDst += nSkip + nRun;
        }
        pDest += g_tSurface.nPitch;
        nRows--;
        pSrc += 2;
    } while (true);
}

// 1.14d mac: 002b24c4
// A run-length block lit per pixel; the light coordinates are stepped per row and shifted
// down before they index the grid.
void FUN_002b24c4(const uint8_t* pSrc, uint8_t* pDest, int nLightX, int nLightXStep, int nLightY, int nLightYStep,
                  int nRows, uint8_t nShift) {
    int nRowsLeft = nRows;
    if (nRows != 0) {
        const uint8_t* pLight = g_abLightGrid + (nLightY >> (nShift & 0x1f)) * 0x20 + (nLightX >> (nShift & 0x1f)) * 0x400;
        uint8_t* pDestRow = pDest;
        do {
            while (true) {
                uint32_t nSkip = pSrc[0];
                uint32_t nRun = pSrc[1];
                if (pSrc[1] == 0 && pSrc[0] == 0) {
                    break;
                }
                uint8_t* pDst = pDest;
                switch (nRun) {
                case 32:
                    pDst[nSkip + 31] = g_abMacShadeTable[pSrc[33] + pLight[nSkip + 31] * 0x100];
                case 31:
                    pDst[nSkip + 30] = g_abMacShadeTable[pSrc[32] + pLight[nSkip + 30] * 0x100];
                case 30:
                    pDst[nSkip + 29] = g_abMacShadeTable[pSrc[31] + pLight[nSkip + 29] * 0x100];
                case 29:
                    pDst[nSkip + 28] = g_abMacShadeTable[pSrc[30] + pLight[nSkip + 28] * 0x100];
                case 28:
                    pDst[nSkip + 27] = g_abMacShadeTable[pSrc[29] + pLight[nSkip + 27] * 0x100];
                case 27:
                    pDst[nSkip + 26] = g_abMacShadeTable[pSrc[28] + pLight[nSkip + 26] * 0x100];
                case 26:
                    pDst[nSkip + 25] = g_abMacShadeTable[pSrc[27] + pLight[nSkip + 25] * 0x100];
                case 25:
                    pDst[nSkip + 24] = g_abMacShadeTable[pSrc[26] + pLight[nSkip + 24] * 0x100];
                case 24:
                    pDst[nSkip + 23] = g_abMacShadeTable[pSrc[25] + pLight[nSkip + 23] * 0x100];
                case 23:
                    pDst[nSkip + 22] = g_abMacShadeTable[pSrc[24] + pLight[nSkip + 22] * 0x100];
                case 22:
                    pDst[nSkip + 21] = g_abMacShadeTable[pSrc[23] + pLight[nSkip + 21] * 0x100];
                case 21:
                    pDst[nSkip + 20] = g_abMacShadeTable[pSrc[22] + pLight[nSkip + 20] * 0x100];
                case 20:
                    pDst[nSkip + 19] = g_abMacShadeTable[pSrc[21] + pLight[nSkip + 19] * 0x100];
                case 19:
                    pDst[nSkip + 18] = g_abMacShadeTable[pSrc[20] + pLight[nSkip + 18] * 0x100];
                case 18:
                    pDst[nSkip + 17] = g_abMacShadeTable[pSrc[19] + pLight[nSkip + 17] * 0x100];
                case 17:
                    pDst[nSkip + 16] = g_abMacShadeTable[pSrc[18] + pLight[nSkip + 16] * 0x100];
                case 16:
                    pDst[nSkip + 15] = g_abMacShadeTable[pSrc[17] + pLight[nSkip + 15] * 0x100];
                case 15:
                    pDst[nSkip + 14] = g_abMacShadeTable[pSrc[16] + pLight[nSkip + 14] * 0x100];
                case 14:
                    pDst[nSkip + 13] = g_abMacShadeTable[pSrc[15] + pLight[nSkip + 13] * 0x100];
                case 13:
                    pDst[nSkip + 12] = g_abMacShadeTable[pSrc[14] + pLight[nSkip + 12] * 0x100];
                case 12:
                    pDst[nSkip + 11] = g_abMacShadeTable[pSrc[13] + pLight[nSkip + 11] * 0x100];
                case 11:
                    pDst[nSkip + 10] = g_abMacShadeTable[pSrc[12] + pLight[nSkip + 10] * 0x100];
                case 10:
                    pDst[nSkip + 9] = g_abMacShadeTable[pSrc[11] + pLight[nSkip + 9] * 0x100];
                case 9:
                    pDst[nSkip + 8] = g_abMacShadeTable[pSrc[10] + pLight[nSkip + 8] * 0x100];
                case 8:
                    pDst[nSkip + 7] = g_abMacShadeTable[pSrc[9] + pLight[nSkip + 7] * 0x100];
                case 7:
                    pDst[nSkip + 6] = g_abMacShadeTable[pSrc[8] + pLight[nSkip + 6] * 0x100];
                case 6:
                    pDst[nSkip + 5] = g_abMacShadeTable[pSrc[7] + pLight[nSkip + 5] * 0x100];
                case 5:
                    pDst[nSkip + 4] = g_abMacShadeTable[pSrc[6] + pLight[nSkip + 4] * 0x100];
                case 4:
                    pDst[nSkip + 3] = g_abMacShadeTable[pSrc[5] + pLight[nSkip + 3] * 0x100];
                case 3:
                    pDst[nSkip + 2] = g_abMacShadeTable[pSrc[4] + pLight[nSkip + 2] * 0x100];
                case 2:
                    pDst[nSkip + 1] = g_abMacShadeTable[pSrc[3] + pLight[nSkip + 1] * 0x100];
                case 1:
                    pDst[nSkip + 0] = g_abMacShadeTable[pSrc[2] + pLight[nSkip + 0] * 0x100];
                }
                pLight += nSkip + nRun;
                pDest += nSkip + nRun;
                pSrc += nRun + 2;
            }
            nLightY += nLightYStep;
            nLightX += nLightXStep;
            pLight = g_abLightGrid + (nLightY >> (nShift & 0x1f)) * 0x20 + (nLightX >> (nShift & 0x1f)) * 0x400;
            pDest = pDestRow + g_tSurface.nPitch;
            nRowsLeft--;
            pSrc += 2;
            pDestRow = pDest;
        } while (nRowsLeft != 0);
    }
}

// 1.14d mac: 002b2a08
// A run-length block through a remap table, every run clipped against the clip rectangle;
// stops once a row passes the bottom edge.
void FUN_002b2a08(int nX, int nY, const uint8_t* pSrc, uint8_t* pDest, const uint8_t* pRemap, int nRows) {
    if (0 < nRows) {
        int nRow = 0;
        uint8_t* pDestRow = pDest;
        const uint8_t* pRun = pSrc;
        do {
            uint8_t* pRunDst = pDestRow;
            const uint8_t* pNext = pRun;
            while (true) {
                pRun = pNext;
                uint32_t nSkip = pRun[0];
                uint32_t nRun = pRun[1];
                if (pRun[1] == 0 && pRun[0] == 0) {
                    break;
                }
                if (g_anClip[0] <= nRow + nY) {
                    return;
                }
                if (nRow + nY < 0) {
                    pNext = pRun + nRun + 2;
                    pRunDst = pRunDst + nSkip + nRun;
                } else {
                    int nPos = ((int)(pRunDst + nSkip) - (int)pDestRow) + nX;
                    int nEnd = g_anClip[2] - nPos;
                    if (nEnd < 0) {
                        nEnd = 0;
                    }
                    if ((int)nRun < nEnd) {
                        nEnd = nRun;
                    }
                    int nStart = g_anClip[1] - nPos;
                    if (nStart < 0) {
                        nStart = 0;
                    }
                    int nVisible = nEnd - nStart;
                    if (nVisible < 0) {
                        nVisible = 0;
                    }
                    pNext = pRun + nRun + 2;
                    uint8_t* pNextDst = pRunDst + nRun + nSkip;
                    if (nVisible != 0) {
                        int nDest = nSkip + nStart;
                        switch (nVisible) {
                        case 32:
                            pRunDst[nDest + 31] = pRemap[pRun[nStart + 33]];
                        case 31:
                            pRunDst[nDest + 30] = pRemap[pRun[nStart + 32]];
                        case 30:
                            pRunDst[nDest + 29] = pRemap[pRun[nStart + 31]];
                        case 29:
                            pRunDst[nDest + 28] = pRemap[pRun[nStart + 30]];
                        case 28:
                            pRunDst[nDest + 27] = pRemap[pRun[nStart + 29]];
                        case 27:
                            pRunDst[nDest + 26] = pRemap[pRun[nStart + 28]];
                        case 26:
                            pRunDst[nDest + 25] = pRemap[pRun[nStart + 27]];
                        case 25:
                            pRunDst[nDest + 24] = pRemap[pRun[nStart + 26]];
                        case 24:
                            pRunDst[nDest + 23] = pRemap[pRun[nStart + 25]];
                        case 23:
                            pRunDst[nDest + 22] = pRemap[pRun[nStart + 24]];
                        case 22:
                            pRunDst[nDest + 21] = pRemap[pRun[nStart + 23]];
                        case 21:
                            pRunDst[nDest + 20] = pRemap[pRun[nStart + 22]];
                        case 20:
                            pRunDst[nDest + 19] = pRemap[pRun[nStart + 21]];
                        case 19:
                            pRunDst[nDest + 18] = pRemap[pRun[nStart + 20]];
                        case 18:
                            pRunDst[nDest + 17] = pRemap[pRun[nStart + 19]];
                        case 17:
                            pRunDst[nDest + 16] = pRemap[pRun[nStart + 18]];
                        case 16:
                            pRunDst[nDest + 15] = pRemap[pRun[nStart + 17]];
                        case 15:
                            pRunDst[nDest + 14] = pRemap[pRun[nStart + 16]];
                        case 14:
                            pRunDst[nDest + 13] = pRemap[pRun[nStart + 15]];
                        case 13:
                            pRunDst[nDest + 12] = pRemap[pRun[nStart + 14]];
                        case 12:
                            pRunDst[nDest + 11] = pRemap[pRun[nStart + 13]];
                        case 11:
                            pRunDst[nDest + 10] = pRemap[pRun[nStart + 12]];
                        case 10:
                            pRunDst[nDest + 9] = pRemap[pRun[nStart + 11]];
                        case 9:
                            pRunDst[nDest + 8] = pRemap[pRun[nStart + 10]];
                        case 8:
                            pRunDst[nDest + 7] = pRemap[pRun[nStart + 9]];
                        case 7:
                            pRunDst[nDest + 6] = pRemap[pRun[nStart + 8]];
                        case 6:
                            pRunDst[nDest + 5] = pRemap[pRun[nStart + 7]];
                        case 5:
                            pRunDst[nDest + 4] = pRemap[pRun[nStart + 6]];
                        case 4:
                            pRunDst[nDest + 3] = pRemap[pRun[nStart + 5]];
                        case 3:
                            pRunDst[nDest + 2] = pRemap[pRun[nStart + 4]];
                        case 2:
                            pRunDst[nDest + 1] = pRemap[pRun[nStart + 3]];
                        case 1:
                            pRunDst[nDest + 0] = pRemap[pRun[nStart + 2]];
                        }
                    }
                    pRunDst = pNextDst;
                }
            }
            pDestRow += g_tSurface.nPitch;
            pRun = pRun + 2;
            nRow++;
        } while (nRow < nRows);
    }
}

// 1.14d mac: 002b2da8
// A run-length block lit per pixel from the light grid, every run clipped against the clip
// rectangle. The light of a clipped run starts at the run's own start, not at its first
// visible pixel.
void FUN_002b2da8(int nX, int nY, const uint8_t* pSrc, uint8_t* pDest, int nLightX, int nLightXStep, int nLightY,
                  int nLightYStep, int nRows, uint8_t nShift) {
    if (0 < nRows) {
        uint8_t* pDestRow = pDest;
        const uint8_t* pLight = g_abLightGrid + (nLightY >> (nShift & 0x1f)) * 0x20 + (nLightX >> (nShift & 0x1f)) * 0x400;
        int nRow = 0;
        do {
            uint8_t* pRunDst = pDestRow;
            const uint8_t* pNextLight = pLight;
            const uint8_t* pRun;
            while (true) {
                pLight = pNextLight;
                pRun = pSrc;
                uint32_t nSkip = pRun[0];
                uint32_t nRun = pRun[1];
                if (pRun[1] == 0 && pRun[0] == 0) {
                    break;
                }
                if (g_anClip[0] <= nRow + nY) {
                    return;
                }
                if (nRow + nY < 0) {
                    pSrc = pRun + nRun + 2;
                    pRunDst = pRunDst + nSkip + nRun;
                    pNextLight = pLight + nSkip + nRun;
                } else {
                    int nPos = ((int)(pRunDst + nSkip) - (int)pDestRow) + nX;
                    int nEnd = g_anClip[2] - nPos;
                    if (nEnd < 0) {
                        nEnd = 0;
                    }
                    if ((int)nRun < nEnd) {
                        nEnd = nRun;
                    }
                    int nStart = g_anClip[1] - nPos;
                    if (nStart < 0) {
                        nStart = 0;
                    }
                    int nVisible = nEnd - nStart;
                    if (nVisible < 0) {
                        nVisible = 0;
                    }
                    pSrc = pRun + nRun + 2;
                    pNextLight = pLight + nRun + nSkip;
                    uint8_t* pNextDst = pRunDst + nRun + nSkip;
                    if (nVisible != 0) {
                        int nDest = nStart + nSkip;
                        switch (nVisible) {
                        case 32:
                            pRunDst[nDest + 31] = g_abMacShadeTable[pRun[nStart + 33] + pLight[nSkip + 31] * 0x100];
                        case 31:
                            pRunDst[nDest + 30] = g_abMacShadeTable[pRun[nStart + 32] + pLight[nSkip + 30] * 0x100];
                        case 30:
                            pRunDst[nDest + 29] = g_abMacShadeTable[pRun[nStart + 31] + pLight[nSkip + 29] * 0x100];
                        case 29:
                            pRunDst[nDest + 28] = g_abMacShadeTable[pRun[nStart + 30] + pLight[nSkip + 28] * 0x100];
                        case 28:
                            pRunDst[nDest + 27] = g_abMacShadeTable[pRun[nStart + 29] + pLight[nSkip + 27] * 0x100];
                        case 27:
                            pRunDst[nDest + 26] = g_abMacShadeTable[pRun[nStart + 28] + pLight[nSkip + 26] * 0x100];
                        case 26:
                            pRunDst[nDest + 25] = g_abMacShadeTable[pRun[nStart + 27] + pLight[nSkip + 25] * 0x100];
                        case 25:
                            pRunDst[nDest + 24] = g_abMacShadeTable[pRun[nStart + 26] + pLight[nSkip + 24] * 0x100];
                        case 24:
                            pRunDst[nDest + 23] = g_abMacShadeTable[pRun[nStart + 25] + pLight[nSkip + 23] * 0x100];
                        case 23:
                            pRunDst[nDest + 22] = g_abMacShadeTable[pRun[nStart + 24] + pLight[nSkip + 22] * 0x100];
                        case 22:
                            pRunDst[nDest + 21] = g_abMacShadeTable[pRun[nStart + 23] + pLight[nSkip + 21] * 0x100];
                        case 21:
                            pRunDst[nDest + 20] = g_abMacShadeTable[pRun[nStart + 22] + pLight[nSkip + 20] * 0x100];
                        case 20:
                            pRunDst[nDest + 19] = g_abMacShadeTable[pRun[nStart + 21] + pLight[nSkip + 19] * 0x100];
                        case 19:
                            pRunDst[nDest + 18] = g_abMacShadeTable[pRun[nStart + 20] + pLight[nSkip + 18] * 0x100];
                        case 18:
                            pRunDst[nDest + 17] = g_abMacShadeTable[pRun[nStart + 19] + pLight[nSkip + 17] * 0x100];
                        case 17:
                            pRunDst[nDest + 16] = g_abMacShadeTable[pRun[nStart + 18] + pLight[nSkip + 16] * 0x100];
                        case 16:
                            pRunDst[nDest + 15] = g_abMacShadeTable[pRun[nStart + 17] + pLight[nSkip + 15] * 0x100];
                        case 15:
                            pRunDst[nDest + 14] = g_abMacShadeTable[pRun[nStart + 16] + pLight[nSkip + 14] * 0x100];
                        case 14:
                            pRunDst[nDest + 13] = g_abMacShadeTable[pRun[nStart + 15] + pLight[nSkip + 13] * 0x100];
                        case 13:
                            pRunDst[nDest + 12] = g_abMacShadeTable[pRun[nStart + 14] + pLight[nSkip + 12] * 0x100];
                        case 12:
                            pRunDst[nDest + 11] = g_abMacShadeTable[pRun[nStart + 13] + pLight[nSkip + 11] * 0x100];
                        case 11:
                            pRunDst[nDest + 10] = g_abMacShadeTable[pRun[nStart + 12] + pLight[nSkip + 10] * 0x100];
                        case 10:
                            pRunDst[nDest + 9] = g_abMacShadeTable[pRun[nStart + 11] + pLight[nSkip + 9] * 0x100];
                        case 9:
                            pRunDst[nDest + 8] = g_abMacShadeTable[pRun[nStart + 10] + pLight[nSkip + 8] * 0x100];
                        case 8:
                            pRunDst[nDest + 7] = g_abMacShadeTable[pRun[nStart + 9] + pLight[nSkip + 7] * 0x100];
                        case 7:
                            pRunDst[nDest + 6] = g_abMacShadeTable[pRun[nStart + 8] + pLight[nSkip + 6] * 0x100];
                        case 6:
                            pRunDst[nDest + 5] = g_abMacShadeTable[pRun[nStart + 7] + pLight[nSkip + 5] * 0x100];
                        case 5:
                            pRunDst[nDest + 4] = g_abMacShadeTable[pRun[nStart + 6] + pLight[nSkip + 4] * 0x100];
                        case 4:
                            pRunDst[nDest + 3] = g_abMacShadeTable[pRun[nStart + 5] + pLight[nSkip + 3] * 0x100];
                        case 3:
                            pRunDst[nDest + 2] = g_abMacShadeTable[pRun[nStart + 4] + pLight[nSkip + 2] * 0x100];
                        case 2:
                            pRunDst[nDest + 1] = g_abMacShadeTable[pRun[nStart + 3] + pLight[nSkip + 1] * 0x100];
                        case 1:
                            pRunDst[nDest + 0] = g_abMacShadeTable[pRun[nStart + 2] + pLight[nSkip + 0] * 0x100];
                        }
                    }
                    pRunDst = pNextDst;
                }
            }
            nLightY += nLightYStep;
            nLightX += nLightXStep;
            pLight = g_abLightGrid + (nLightY >> (nShift & 0x1f)) * 0x20 + (nLightX >> (nShift & 0x1f)) * 0x400;
            pDestRow += g_tSurface.nPitch;
            pSrc = pRun + 2;
            nRow++;
        } while (nRow < nRows);
    }
}

void fpDrawVisTile(const uint8_t* pSrc, int nX, int nY);
void FUN_002b38f3(const uint8_t* pSrc, int nX, int nY, uint8_t nLight);

// 1.14d mac: 002b36e8
// Slot nfpDrawVisTile: an unshaded run-length wall block, 32 rows, with a clipped path.
void fpDrawVisTile(const uint8_t* pSrc, int nX, int nY) {
    int nTop = 0;
    if (g_anClip[10] != 0) {
        nTop = -0x20;
    }
    if (nX < g_anClip[13] && (nY < g_anClip[11] && nTop <= nY) && g_anClip[12] <= nX) {
        uint8_t* pBase = (uint8_t*)g_tSurface.pBits;
        if (pBase == 0) {
            FRAMEPOINTER_FATAL(0x2f4);
            pBase = (uint8_t*)g_tSurface.pBits;
        }
        uint8_t* pDst = g_anRowOffsets[nY + 32] + nX + pBase;
        uint8_t* pDestRow = pDst;
        if (nX < g_anClip[16] && (nY < g_anClip[14] && -1 < nY) && g_anClip[15] <= nX) {
            int nRows = 0x20;
            do {
                while (true) {
                    uint8_t nSkip = pSrc[0];
                    uint32_t nRun = pSrc[1];
                    if (pSrc[1] == 0 && nSkip == 0) {
                        break;
                    }
                    uint8_t* pRunDst = pDst + nSkip;
                    pDst = pDst + nRun + nSkip;
                    SMemCopy(pRunDst, pSrc + 2, nRun);
                    pSrc = pSrc + nRun + 2;
                }
                pDst = pDestRow + g_tSurface.nPitch;
                nRows--;
                pSrc = pSrc + 2;
                pDestRow = pDst;
            } while (nRows != 0);
        } else {
            int nRow = 0;
            do {
                pDst = pDestRow;
                const uint8_t* pNext = pSrc;
                const uint8_t* pRun;
                while (true) {
                    pRun = pNext;
                    uint8_t* pRunStart = pDst;
                    uint32_t nSkip = pRun[0];
                    uint32_t nRun = pRun[1];
                    if (pRun[1] == 0 && pRun[0] == 0) {
                        break;
                    }
                    if (g_anClip[0] <= nRow + nY) {
                        return;
                    }
                    if (nRow + nY < 0) {
                        pDst = pRunStart + nSkip + nRun;
                        pNext = pRun + nRun + 2;
                    } else {
                        int nPos = ((int)(pRunStart + nSkip) - (int)pDestRow) + nX;
                        int nEnd = g_anClip[2] - nPos;
                        if (nEnd < 0) {
                            nEnd = 0;
                        }
                        if ((int)nRun < nEnd) {
                            nEnd = nRun;
                        }
                        int nStart = g_anClip[1] - nPos;
                        if (nStart < 0) {
                            nStart = 0;
                        }
                        int nSize = nEnd - nStart;
                        if (nSize < 0) {
                            nSize = 0;
                        }
                        pDst = pRunStart + nRun + nSkip;
                        pNext = pRun + nRun + 2;
                        if (nSize != 0) {
                            SMemCopy(pRunStart + nSkip + nStart, pRun + nStart + 2, nSize);
                        }
                    }
                }
                pDestRow = pDestRow + g_tSurface.nPitch;
                pSrc = pRun + 2;
                nRow++;
            } while (nRow < 0x20);
        }
    }
}

// 1.14d mac: 002b38f3
// A 32-row run-length wall block with one light level: the unclipped blit when it lies wholly
// inside the clip rectangle, the clipped one otherwise.
void FUN_002b38f3(const uint8_t* pSrc, int nX, int nY, uint8_t nLight) {
    int nTop = 0;
    if (g_anClip[10] != 0) {
        nTop = -0x20;
    }
    if (nX < g_anClip[13] && (nY < g_anClip[11] && nTop <= nY) && g_anClip[12] <= nX) {
        int nRowOffset = g_anRowOffsets[nY + 32];
        uint8_t* pBase = (uint8_t*)g_tSurface.pBits;
        if (pBase == 0) {
            FRAMEPOINTER_FATAL(0x373);
        }
        uint8_t* pDest = pBase + nRowOffset + nX;
        if (nX < g_anClip[16] && (nY < g_anClip[14] && -1 < nY) && g_anClip[15] <= nX) {
            FUN_002b224c(pSrc, pDest, g_abMacShadeTable + (nLight & 0xf8) * 0x20, 0x20);
        } else {
            FUN_002b2a08(nX, nY, pSrc, pDest, g_abMacShadeTable + (nLight & 0xf8) * 0x20, 0x20);
        }
    }
}

// 1.14d mac: 002b3a14
// Slot nfpDrawWallTile: a wall block lit per pixel from its four corner lights when they
// differ enough and the renderer lights per pixel, one uniform shade otherwise, or the
// unshaded vis blit at full bright.
void fpDrawWallTile(const uint8_t* pSrc, int nX, int nY, const uint32_t* pCornerLights) {
    uint32_t nLight = pCornerLights[0];
    uint32_t nLightX = nLight << 5;
    if (D2GFXRenderedData[2] == 0) {
        uint32_t nLight1 = pCornerLights[1];
        int nStepY = pCornerLights[2] - nLight1;
        int nStepX = pCornerLights[3] - nLight;
        int nDiff1 = nLight1 - nLight;
        int nAbsX = -nStepX;
        if (0 < nStepX) {
            nAbsX = nStepX;
        }
        int nAbs1 = -nDiff1;
        if (0 < nDiff1) {
            nAbs1 = nDiff1;
        }
        int nAbsY = -nStepY;
        if (0 < nStepY) {
            nAbsY = nStepY;
        }
        if (9 < nAbsY + nAbs1 + nAbsX) {
            int nTop = 0;
            if (g_anClip[10] != 0) {
                nTop = -0x20;
            }
            if (g_anClip[13] <= nX) {
                return;
            }
            if ((g_anClip[11] <= nY || nY < nTop) || nX < g_anClip[12]) {
                return;
            }
            uint8_t* pBase = (uint8_t*)g_tSurface.pBits;
            if (pBase == 0) {
                FRAMEPOINTER_FATAL(0x3be);
                pBase = (uint8_t*)g_tSurface.pBits;
            }
            uint8_t* pDest = g_anRowOffsets[nY + 32] + nX + pBase;
            if (nX < g_anClip[16] && (nY < g_anClip[14] && -1 < nY) && g_anClip[15] <= nX) {
                FUN_002b24c4(pSrc, pDest, nLightX, nStepX, nLight1 << 5, nStepY, 0x20, 8);
                return;
            }
            FUN_002b2da8(nX, nY, pSrc, pDest, nLightX, nStepX, nLight1 << 5, nStepY, 0x20, 8);
            return;
        }
    }
    if ((nLightX & 0xffffff00) != 0x1f00) {
        FUN_002b38f3(pSrc, nX, nY, (uint8_t)nLight);
        return;
    }
    fpDrawVisTile(pSrc, nX, nY);
}

// The blend table of an alpha: 0xC0 and up the first, 0x80 the second, 0x40 the third; below
// that nothing is drawn (NULL).
static uint8_t** MacBlendTableSlot(uint8_t nAlpha) {
    if (nAlpha < 0xc0) {
        if ((char)nAlpha < '\0') {
            return g_ppBlendTables + 1;
        }
        if (nAlpha < 0x40) {
            return 0;
        }
        return g_ppBlendTables + 2;
    }
    return g_ppBlendTables;
}

// 1.14d mac: 002b3be5
// Slot nfpDrawBlendedVisTile: a run-length wall block alpha-blended through a blend table
// chosen by the alpha byte, with a clipped path.
void fpDrawBlendedVisTile(const uint8_t* pSrc, int nX, int nY, uint8_t nAlpha) {
    uint8_t** ppTable = MacBlendTableSlot(nAlpha);
    if (ppTable == 0) {
        return;
    }
    int nTop = 0;
    if (g_anClip[10] != 0) {
        nTop = -0x20;
    }
    if (nX < g_anClip[13] && (nY < g_anClip[11] && nTop <= nY) && g_anClip[12] <= nX) {
        const uint8_t* pBlend = *ppTable;
        uint8_t* pBase = (uint8_t*)g_tSurface.pBits;
        if (pBase == 0) {
            FRAMEPOINTER_FATAL(0x3f8);
            pBase = (uint8_t*)g_tSurface.pBits;
        }
        uint8_t* pDestRow = g_anRowOffsets[nY + 32] + nX + pBase;
        if (nX < g_anClip[16] && (nY < g_anClip[14] && -1 < nY) && g_anClip[15] <= nX) {
            int nRows = 0x20;
            uint8_t* pDst = pDestRow;
            do {
                while (true) {
                    const uint8_t* pRun = pSrc;
                    uint8_t nSkip = pRun[0];
                    uint32_t nRun = pRun[1];
                    pSrc = pRun + 2;
                    if (pRun[1] == 0 && nSkip == 0) {
                        break;
                    }
                    pDst += nSkip;
                    for (uint32_t i = 0; i < nRun; i++) {
                        *pDst = pBlend[(*pDst << 8) | *pSrc];
                        pDst++;
                        pSrc++;
                    }
                }
                pDestRow += g_tSurface.nPitch;
                nRows--;
                pDst = pDestRow;
            } while (nRows != 0);
        } else {
            int nRow = 0;
            do {
                uint8_t* pDst = pDestRow;
                const uint8_t* pRun;
                while (true) {
                    pRun = pSrc;
                    uint32_t nSkip = pRun[0];
                    uint32_t nRun = pRun[1];
                    if (pRun[1] == 0 && pRun[0] == 0) {
                        break;
                    }
                    if (g_anClip[0] <= nRow + nY) {
                        return;
                    }
                    if (nRow + nY < 0) {
                        pSrc = pRun + nRun + 2;
                        pDst = pDst + nSkip + nRun;
                    } else {
                        int nPos = ((int)(pDst + nSkip) - (int)pDestRow) + nX;
                        int nEnd = g_anClip[2] - nPos;
                        if (nEnd < 0) {
                            nEnd = 0;
                        }
                        if ((int)nRun < nEnd) {
                            nEnd = nRun;
                        }
                        int nStart = g_anClip[1] - nPos;
                        if (nStart < 0) {
                            nStart = 0;
                        }
                        int nVisible = nEnd - nStart;
                        if (nVisible < 0) {
                            nVisible = 0;
                        }
                        pSrc = pRun + nRun + 2;
                        uint8_t* pNextDst = pDst + nRun + nSkip;
                        if (nVisible != 0) {
                            const uint8_t* pPixel = pRun + nStart + 2;
                            uint8_t* pPlot = pDst + nStart + nSkip;
                            while (pPixel < pRun + nVisible + 2 + nStart) {
                                *pPlot = pBlend[(*pPlot << 8) | *pPixel];
                                pPlot++;
                                pPixel++;
                            }
                        }
                        pDst = pNextDst;
                    }
                }
                pDestRow += g_tSurface.nPitch;
                pSrc = pRun + 2;
                nRow++;
            } while (nRow < 0x20);
        }
    }
}

// 1.14d mac: 002b3e8b
// Slot nfpDrawRoofTile: a run-length wall block that is alpha-blended and lit from the light
// grid.
void fpDrawRoofTile(const uint8_t* pSrc, int nX, int nY, const int* pCornerLights, uint8_t nAlpha) {
    uint8_t** ppTable = MacBlendTableSlot(nAlpha);
    if (ppTable == 0) {
        return;
    }
    int nTop = 0;
    if (g_anClip[10] != 0) {
        nTop = -0x20;
    }
    if (nX < g_anClip[13] && (nY < g_anClip[11] && nTop <= nY) && g_anClip[12] <= nX) {
        int nLight0 = pCornerLights[0];
        int nLight1 = pCornerLights[1];
        int nLight2 = pCornerLights[2];
        int nLight3 = pCornerLights[3];
        const uint8_t* pBlend = *ppTable;
        int nLightY = nLight1 << 5;
        uint32_t nLightX = nLight0 << 5;
        uint8_t* pBase = (uint8_t*)g_tSurface.pBits;
        if (pBase == 0) {
            FRAMEPOINTER_FATAL(0x48b);
            pBase = (uint8_t*)g_tSurface.pBits;
        }
        int nStepY = nLight2 - nLight1;
        int nStepX = nLight3 - nLight0;
        uint8_t* pDestRow = g_anRowOffsets[nY + 32] + nX + pBase;
        const uint8_t* pLight = g_abLightGrid + (nLightY >> 8) * 0x20 + (nLightX & 0x3fffff00) * 4;
        int nRow;
        if (nX < g_anClip[16]) {
            if ((nY < g_anClip[14] && -1 < nY) && g_anClip[15] <= nX) {
                int nRows = 0x20;
                uint8_t* pDst = pDestRow;
                do {
                    while (true) {
                        const uint8_t* pRun = pSrc;
                        uint32_t nSkip = pRun[0];
                        uint32_t nRun = pRun[1];
                        pSrc = pRun + 2;
                        if (pRun[1] == 0 && pRun[0] == 0) {
                            break;
                        }
                        pLight += nSkip;
                        pDst += nSkip;
                        for (uint32_t i = 0; i < nRun; i++) {
                            *pDst = pBlend[(g_abMacShadeTable[*pSrc + *pLight * 0x100] << 8) | *pDst];
                            pDst++;
                            pLight++;
                            pSrc++;
                        }
                    }
                    nLightY += nStepY;
                    nLightX += nStepX;
                    pDestRow += g_tSurface.nPitch;
                    nRows--;
                    pLight = g_abLightGrid + (nLightY >> 8) * 0x20 + (nLightX & 0x3fffff00) * 4;
                    pDst = pDestRow;
                } while (nRows != 0);
                return;
            }
            nRow = 0;
        } else {
            nRow = 0;
        }
        do {
            uint8_t* pDst = pDestRow;
            const uint8_t* pRun;
            while (true) {
                pRun = pSrc;
                uint32_t nSkip = pRun[0];
                uint32_t nRun = pRun[1];
                if (pRun[1] == 0 && pRun[0] == 0) {
                    break;
                }
                if (g_anClip[0] <= nRow + nY) {
                    return;
                }
                if (nRow + nY < 0) {
                    pLight += nSkip + nRun;
                    pSrc = pRun + nRun + 2;
                    pDst = pDst + nSkip + nRun;
                } else {
                    const uint8_t* pRunLight = pLight + nSkip;
                    int nPos = ((int)(pDst + nSkip) - (int)pDestRow) + nX;
                    int nEnd = g_anClip[2] - nPos;
                    if (nEnd < 0) {
                        nEnd = 0;
                    }
                    if ((int)nRun < nEnd) {
                        nEnd = nRun;
                    }
                    int nStart = g_anClip[1] - nPos;
                    if (nStart < 0) {
                        nStart = 0;
                    }
                    int nVisible = nEnd - nStart;
                    if (nVisible < 0) {
                        nVisible = 0;
                    }
                    pSrc = pRun + nRun + 2;
                    pLight = pLight + nRun + nSkip;
                    uint8_t* pNextDst = pDst + nRun + nSkip;
                    if (nVisible != 0) {
                        const uint8_t* pPixel = pRun + nStart + 2;
                        uint8_t* pPlot = pDst + nStart + nSkip;
                        while (pPixel < pRun + nVisible + 2 + nStart) {
                            *pPlot = pBlend[(g_abMacShadeTable[*pPixel + *pRunLight * 0x100] << 8) | *pPlot];
                            pPlot++;
                            pRunLight++;
                            pPixel++;
                        }
                    }
                    pDst = pNextDst;
                }
            }
            nLightY += nStepY;
            nLightX += nStepX;
            pLight = g_abLightGrid + (nLightY >> 8) * 0x20 + (nLightX & 0x3fffff00) * 4;
            pDestRow += g_tSurface.nPitch;
            pSrc = pRun + 2;
            nRow++;
        } while (nRow < 0x20);
    }
}

// 1.14d mac: 002b4270
// A 15-row floor sub-block with one light level.
void FUN_002b4270(const MacGroundBlock* pBlock, uint8_t* pDest, uint8_t nLight) {
    if ((pBlock->nFlags & 4) == 0) {
        FUN_002b1a44(pBlock->pData, pDest, g_abMacShadeTable + (nLight & 0xf8) * 0x20);
    } else {
        FUN_002b224c(pBlock->pData, pDest, g_abMacShadeTable + (nLight & 0xf8) * 0x20, 0xf);
    }
}

// 1.14d mac: 002b42cf
// A 15-row floor sub-block lit per pixel; the four corner lights give the start and steps.
void FUN_002b42cf(const MacGroundBlock* pBlock, uint8_t* pDest, int* pCornerLights) {
    int nStepY = pCornerLights[2] - pCornerLights[1];
    int nStepX = pCornerLights[3] - pCornerLights[0];
    int nLightY = pCornerLights[1] << 4;
    int nLightX = pCornerLights[0] << 4;
    if ((pBlock->nFlags & 4) == 0) {
        FUN_002b1ca9(pBlock->pData, pDest, nLightX, nStepX, nLightY, nStepY);
    } else {
        FUN_002b24c4(pBlock->pData, pDest, nLightX, nStepX, nLightY, nStepY, 0xf, 7);
    }
}

// 1.14d mac: 002b4354
// A 15-row floor sub-block with one light level, clipped against the clip rectangle;
// run-length blocks go to the clipped run-length blitter.
void FUN_002b4354(const MacGroundBlock* pBlock, int nX, int nY, uint8_t* pDest, uint8_t nLight) {
    if ((pBlock->nFlags & 4) == 0) {
        uint8_t* pDestRow = pDest;
        int nRowEnd0 = g_anClip[0] - nY;
        if (0xf < nRowEnd0) {
            nRowEnd0 = 0xf;
        }
        uint32_t nRow = nY >> 0x1f & -nY;
        if ((int)nRow < nRowEnd0) {
            uint32_t nBelow = (nY - 1) - g_anClip[0];
            uint32_t nRowEnd = 0xf;
            if (-0x10 < (int)nBelow) {
                nRowEnd = ~nBelow;
            }
            int nPitch = g_tSurface.nPitch;
            int nOffset = nPitch * nRow;
            do {
                pDestRow = pDestRow + nOffset;
                int nDestOffset = g_anTileRowDest[nRow] + nX;
                int nEnd = g_anClip[2] - nDestOffset;
                if (nEnd < 0) {
                    nEnd = 0;
                }
                if ((int)g_anTileRowWidth[nRow] < nEnd) {
                    nEnd = g_anTileRowWidth[nRow];
                }
                int nStart = g_anClip[1] - nDestOffset;
                if (nStart < 0) {
                    nStart = 0;
                }
                int nCount = nEnd - nStart;
                if (nCount < 0) {
                    nCount = 0;
                }
                if (nCount != 0) {
                    int nSrcStart = g_anTileRowSrc[nRow] + nStart;
                    if (nSrcStart < nCount + nSrcStart) {
                        const uint8_t* pData = pBlock->pData;
                        const uint8_t* pPixel = nSrcStart + pData;
                        uint8_t* pPlot = nStart + g_anTileRowDest[nRow] + pDestRow;
                        do {
                            *pPlot = g_abMacShadeTable[*pPixel + (nLight >> 3) * 0x100];
                            pPlot++;
                            pPixel++;
                        } while (pPixel < nCount + nSrcStart + pData);
                        nPitch = g_tSurface.nPitch;
                    }
                }
                nRow++;
                nOffset = nPitch;
            } while (nRow != nRowEnd);
        }
    } else {
        FUN_002b2a08(nX, nY, pBlock->pData, pDest, g_abMacShadeTable + (nLight >> 3) * 0x100, 0xf);
    }
}

// 1.14d mac: 002b44c2
// A 15-row floor sub-block lit per pixel, clipped against the clip rectangle; the four
// corner lights give the start and steps, run-length blocks go to the clipped lit blitter.
void FUN_002b44c2(const MacGroundBlock* pBlock, int nX, int nY, uint8_t* pDest, int* pCornerLights) {
    int nStepY = pCornerLights[2] - pCornerLights[1];
    int nStepX = pCornerLights[3] - pCornerLights[0];
    int nLightY = pCornerLights[1] << 4;
    uint32_t nLightX = pCornerLights[0] << 4;
    if ((pBlock->nFlags & 4) == 0) {
        int nRowEnd0 = g_anClip[0] - nY;
        if (0xf < nRowEnd0) {
            nRowEnd0 = 0xf;
        }
        uint32_t nRow = nY >> 0x1f & -nY;
        if ((int)nRow < nRowEnd0) {
            uint32_t nBelow = (nY - 1) - g_anClip[0];
            uint32_t nRowEnd = 0xf;
            if (-0x10 < (int)nBelow) {
                nRowEnd = ~nBelow;
            }
            int nPitch = g_tSurface.nPitch;
            uint8_t* pDestRow = pDest + nPitch * nRow;
            while (true) {
                int nDestOffset = g_anTileRowDest[nRow];
                int nEnd = g_anClip[2] - (nDestOffset + nX);
                if (nEnd < 0) {
                    nEnd = 0;
                }
                if ((int)g_anTileRowWidth[nRow] < nEnd) {
                    nEnd = g_anTileRowWidth[nRow];
                }
                int nStart = g_anClip[1] - (nDestOffset + nX);
                if (nStart < 0) {
                    nStart = 0;
                }
                int nCount = nEnd - nStart;
                if (nCount < 0) {
                    nCount = 0;
                }
                if (nCount != 0) {
                    int nSrcStart = g_anTileRowSrc[nRow] + nStart;
                    if (nSrcStart < nCount + nSrcStart) {
                        const uint8_t* pData = pBlock->pData;
                        const uint8_t* pPixel = nSrcStart + pData;
                        uint8_t* pPlot = nStart + nDestOffset + pDestRow;
                        const uint8_t* pLight = g_abLightGrid + nDestOffset + (nLightY >> 7) * 0x20 + (nLightX & 0x1fffff80) * 8;
                        do {
                            *pPlot = g_abMacShadeTable[*pPixel + *pLight * 0x100];
                            pPlot++;
                            pLight++;
                            pPixel++;
                        } while (pPixel < nCount + nSrcStart + pData);
                        nPitch = g_tSurface.nPitch;
                    }
                }
                nRow++;
                if (nRow == nRowEnd) {
                    break;
                }
                pDestRow = pDestRow + nPitch;
                nLightX = nLightX + nStepX;
                nLightY = nLightY + nStepY;
            }
        }
    } else {
        FUN_002b2da8(nX, nY, pBlock->pData, pDest, nLightX, nStepX, nLightY, nStepY, 0xf, 7);
    }
}

// 1.14d mac: 002b4696
// Slot nfpDrawGroundTile: walks the tile's sub-blocks, takes their corner lights from the
// light data and hands each to the floor-block helper that fits.
void fpDrawGroundTile(const MacGroundTile* pTile, int nScreenX, int nScreenY, const uint8_t* pLights) {
    if (0 < pTile->nBlocks) {
        uint8_t* pBase = (uint8_t*)g_tSurface.pBits;
        int nPitch = g_tSurface.nPitch;
        int nRowBase = nPitch * nScreenY;
        int nBlock = 0;
        int nOffset = 0;
        while (true) {
            const MacGroundBlock* pBlockArray = pTile->pBlocks;
            const MacGroundBlock* pBlock = (const MacGroundBlock*)((const uint8_t*)pBlockArray + nOffset);
            if ((pBlock->nFlags & 1) != 0) {
                int nTop = 0;
                if (g_anClip[3] != 0) {
                    nTop = -0x10;
                }
                int nY = pBlock->nY + nScreenY;
                int nX = pBlock->nX + nScreenX;
                if (nX < g_anClip[6] && (nY < g_anClip[4] && nTop <= nY) && g_anClip[5] <= nX) {
                    int nCell = (pBlock->nLightX + pBlock->nLightY * 8) * 0xc;
                    uint8_t nLight = pLights[nCell + 0x6c];
                    bool bPerPixel = false;
                    uint32_t anCorner[4];
                    if (D2GFXRenderedData[2] == 0) {
                        uint32_t nLight1 = pLights[nCell + 0x78];
                        uint32_t nLight3 = pLights[nCell + 0xd8];
                        int nDiffX = nLight3 - nLight1;
                        uint32_t nLight2 = pLights[nCell + 0xcc];
                        uint32_t nLight0 = nLight;
                        int nDiffY = nLight2 - nLight0;
                        int nDiff1 = nLight1 - nLight0;
                        int nAbs1 = -nDiff1;
                        if (0 < nDiff1) {
                            nAbs1 = nDiff1;
                        }
                        int nAbsY = -nDiffY;
                        if (0 < nDiffY) {
                            nAbsY = nDiffY;
                        }
                        int nAbsX = -nDiffX;
                        if (0 < nDiffX) {
                            nAbsX = nDiffX;
                        }
                        if (9 < nAbsX + nAbsY + nAbs1) {
                            anCorner[0] = pLights[nCell + 0x60] + nLight0 + pLights[nCell + 0xc0] + nLight2 >> 2;
                            anCorner[1] = nLight0 + nLight1 + pLights[nCell + 0xc] + pLights[nCell + 0x18] >> 2;
                            anCorner[2] = pLights[nCell + 0x84] + nLight1 + nLight2 + pLights[nCell + 0xe4] >> 2;
                            anCorner[3] = nLight3 + nLight2 + pLights[nCell + 300] + pLights[nCell + 0x138] >> 2;
                            nLight = 0;
                            bPerPixel = true;
                        }
                    }
                    uint8_t* pDest = pBase + nScreenX + pBlock->nX + nRowBase + pBlock->nY * nPitch;
                    if (nX < g_anClip[9] && (nY < g_anClip[7] && -1 < nY) && g_anClip[8] <= nX) {
                        if (bPerPixel) {
                            FUN_002b42cf(pBlock, pDest, (int*)anCorner);
                        } else {
                            FUN_002b4270(pBlock, pDest, nLight);
                        }
                    } else if (bPerPixel) {
                        FUN_002b44c2(pBlock, nX, nY, pDest, (int*)anCorner);
                    } else {
                        FUN_002b4354(pBlock, nX, nY, pDest, nLight);
                    }
                }
            }
            nBlock++;
            if (pTile->nBlocks <= nBlock) {
                break;
            }
            nOffset += 0x14;
            nPitch = g_tSurface.nPitch;
        }
    }
}

#endif
