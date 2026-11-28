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
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_MAC_CARBON`.
#include "D2GFXMac.h"

#include <stdlib.h>

#if !defined(_WIN32) && D2_MAC_CARBON

void fp_fatal(const char* szFormat, ...);
// 1.14d mac: 002ef085
void* FOG_GetFramePointer(void);

// The palette-table set (D2GFXMacHelper.cpp): its first three entries are the blend tables,
// taken for an alpha of 0xC0 and up, 0x80 and up, 0x40 and up; below that nothing is drawn.
extern uint8_t** g_ppPaletteTables;

// 1.14d mac: 00547e98
// The surface's clip: rows below nBottom and columns outside [nLeft, nRight) are not drawn.
MacClipBounds g_tSurfaceClip;
// 1.14d mac: 00547ea4
// Where a floor tile's origin must lie to be drawn at all (tOuter) and to be drawn unclipped
// (tInner); bTall lets it start 32 rows above the top.
MacTileClip g_tGroundClip;
// 1.14d mac: 00547ec0
// The same for a wall block.
MacTileClip g_tWallClip;
// 1.14d mac: 00547edc
// Row offsets from 32 rows above the surface; entry n + 32 is row n.
int g_anRowOffsets[732];
// 1.14d mac: 0053fe98
// The light grid: 32 columns by 32 rows by 32 shades.
uint8_t g_abLightGrid[0x8000];

// A floor block is a diamond of 15 rows: where each row starts in the destination, how many
// pixels it has, and where it starts in the packed source.
// 1.14d mac: 003c8090
int g_anTileRowDest[15] = {14, 12, 10, 8, 6, 4, 2, 0, 2, 4, 6, 8, 10, 12, 14};
// 1.14d mac: 003c80cc
uint32_t g_anTileRowWidth[15] = {4, 8, 12, 16, 20, 24, 28, 32, 28, 24, 20, 16, 12, 8, 4};
// 1.14d mac: 003c8108
int g_anTileRowSrc[15] = {0, 4, 12, 24, 40, 60, 84, 112, 144, 172, 196, 216, 232, 244, 252};

// A run-length row's run header: pixels to skip, then pixels that follow.
struct MacRunHeader {
    uint8_t nSkip;
    uint8_t nCount;
};

// Whether a tile's origin lies in the bounds, rows from nTop.
static inline bool MacTileInBounds(const MacClipBounds* pBounds, int nTop, int nX, int nY) {
    return nTop <= nY && nY < pBounds->nBottom && pBounds->nLeft <= nX && nX < pBounds->nRight;
}

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
                MAC_SHIFT_TABLES[pSrc[nSrcOffset + i] + g_abLightGrid[nDestOffset + i + nLightOffset] * 0x100];
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
        pSrc += sizeof(uint16_t);
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
                    pDst[nSkip + 31] = MAC_SHIFT_TABLES[pSrc[33] + pLight[nSkip + 31] * 0x100];
                case 31:
                    pDst[nSkip + 30] = MAC_SHIFT_TABLES[pSrc[32] + pLight[nSkip + 30] * 0x100];
                case 30:
                    pDst[nSkip + 29] = MAC_SHIFT_TABLES[pSrc[31] + pLight[nSkip + 29] * 0x100];
                case 29:
                    pDst[nSkip + 28] = MAC_SHIFT_TABLES[pSrc[30] + pLight[nSkip + 28] * 0x100];
                case 28:
                    pDst[nSkip + 27] = MAC_SHIFT_TABLES[pSrc[29] + pLight[nSkip + 27] * 0x100];
                case 27:
                    pDst[nSkip + 26] = MAC_SHIFT_TABLES[pSrc[28] + pLight[nSkip + 26] * 0x100];
                case 26:
                    pDst[nSkip + 25] = MAC_SHIFT_TABLES[pSrc[27] + pLight[nSkip + 25] * 0x100];
                case 25:
                    pDst[nSkip + 24] = MAC_SHIFT_TABLES[pSrc[26] + pLight[nSkip + 24] * 0x100];
                case 24:
                    pDst[nSkip + 23] = MAC_SHIFT_TABLES[pSrc[25] + pLight[nSkip + 23] * 0x100];
                case 23:
                    pDst[nSkip + 22] = MAC_SHIFT_TABLES[pSrc[24] + pLight[nSkip + 22] * 0x100];
                case 22:
                    pDst[nSkip + 21] = MAC_SHIFT_TABLES[pSrc[23] + pLight[nSkip + 21] * 0x100];
                case 21:
                    pDst[nSkip + 20] = MAC_SHIFT_TABLES[pSrc[22] + pLight[nSkip + 20] * 0x100];
                case 20:
                    pDst[nSkip + 19] = MAC_SHIFT_TABLES[pSrc[21] + pLight[nSkip + 19] * 0x100];
                case 19:
                    pDst[nSkip + 18] = MAC_SHIFT_TABLES[pSrc[20] + pLight[nSkip + 18] * 0x100];
                case 18:
                    pDst[nSkip + 17] = MAC_SHIFT_TABLES[pSrc[19] + pLight[nSkip + 17] * 0x100];
                case 17:
                    pDst[nSkip + 16] = MAC_SHIFT_TABLES[pSrc[18] + pLight[nSkip + 16] * 0x100];
                case 16:
                    pDst[nSkip + 15] = MAC_SHIFT_TABLES[pSrc[17] + pLight[nSkip + 15] * 0x100];
                case 15:
                    pDst[nSkip + 14] = MAC_SHIFT_TABLES[pSrc[16] + pLight[nSkip + 14] * 0x100];
                case 14:
                    pDst[nSkip + 13] = MAC_SHIFT_TABLES[pSrc[15] + pLight[nSkip + 13] * 0x100];
                case 13:
                    pDst[nSkip + 12] = MAC_SHIFT_TABLES[pSrc[14] + pLight[nSkip + 12] * 0x100];
                case 12:
                    pDst[nSkip + 11] = MAC_SHIFT_TABLES[pSrc[13] + pLight[nSkip + 11] * 0x100];
                case 11:
                    pDst[nSkip + 10] = MAC_SHIFT_TABLES[pSrc[12] + pLight[nSkip + 10] * 0x100];
                case 10:
                    pDst[nSkip + 9] = MAC_SHIFT_TABLES[pSrc[11] + pLight[nSkip + 9] * 0x100];
                case 9:
                    pDst[nSkip + 8] = MAC_SHIFT_TABLES[pSrc[10] + pLight[nSkip + 8] * 0x100];
                case 8:
                    pDst[nSkip + 7] = MAC_SHIFT_TABLES[pSrc[9] + pLight[nSkip + 7] * 0x100];
                case 7:
                    pDst[nSkip + 6] = MAC_SHIFT_TABLES[pSrc[8] + pLight[nSkip + 6] * 0x100];
                case 6:
                    pDst[nSkip + 5] = MAC_SHIFT_TABLES[pSrc[7] + pLight[nSkip + 5] * 0x100];
                case 5:
                    pDst[nSkip + 4] = MAC_SHIFT_TABLES[pSrc[6] + pLight[nSkip + 4] * 0x100];
                case 4:
                    pDst[nSkip + 3] = MAC_SHIFT_TABLES[pSrc[5] + pLight[nSkip + 3] * 0x100];
                case 3:
                    pDst[nSkip + 2] = MAC_SHIFT_TABLES[pSrc[4] + pLight[nSkip + 2] * 0x100];
                case 2:
                    pDst[nSkip + 1] = MAC_SHIFT_TABLES[pSrc[3] + pLight[nSkip + 1] * 0x100];
                case 1:
                    pDst[nSkip + 0] = MAC_SHIFT_TABLES[pSrc[2] + pLight[nSkip + 0] * 0x100];
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
            pSrc += sizeof(uint16_t);
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
                if (g_tSurfaceClip.nBottom <= nRow + nY) {
                    return;
                }
                if (nRow + nY < 0) {
                    pNext = pRun + nRun + 2;
                    pRunDst = pRunDst + nSkip + nRun;
                } else {
                    int nPos = ((int)(pRunDst + nSkip) - (int)pDestRow) + nX;
                    int nEnd = g_tSurfaceClip.nRight - nPos;
                    if (nEnd < 0) {
                        nEnd = 0;
                    }
                    if ((int)nRun < nEnd) {
                        nEnd = nRun;
                    }
                    int nStart = g_tSurfaceClip.nLeft - nPos;
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
            pRun = pRun + sizeof(uint16_t);
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
                if (g_tSurfaceClip.nBottom <= nRow + nY) {
                    return;
                }
                if (nRow + nY < 0) {
                    pSrc = pRun + nRun + 2;
                    pRunDst = pRunDst + nSkip + nRun;
                    pNextLight = pLight + nSkip + nRun;
                } else {
                    int nPos = ((int)(pRunDst + nSkip) - (int)pDestRow) + nX;
                    int nEnd = g_tSurfaceClip.nRight - nPos;
                    if (nEnd < 0) {
                        nEnd = 0;
                    }
                    if ((int)nRun < nEnd) {
                        nEnd = nRun;
                    }
                    int nStart = g_tSurfaceClip.nLeft - nPos;
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
                            pRunDst[nDest + 31] = MAC_SHIFT_TABLES[pRun[nStart + 33] + pLight[nSkip + 31] * 0x100];
                        case 31:
                            pRunDst[nDest + 30] = MAC_SHIFT_TABLES[pRun[nStart + 32] + pLight[nSkip + 30] * 0x100];
                        case 30:
                            pRunDst[nDest + 29] = MAC_SHIFT_TABLES[pRun[nStart + 31] + pLight[nSkip + 29] * 0x100];
                        case 29:
                            pRunDst[nDest + 28] = MAC_SHIFT_TABLES[pRun[nStart + 30] + pLight[nSkip + 28] * 0x100];
                        case 28:
                            pRunDst[nDest + 27] = MAC_SHIFT_TABLES[pRun[nStart + 29] + pLight[nSkip + 27] * 0x100];
                        case 27:
                            pRunDst[nDest + 26] = MAC_SHIFT_TABLES[pRun[nStart + 28] + pLight[nSkip + 26] * 0x100];
                        case 26:
                            pRunDst[nDest + 25] = MAC_SHIFT_TABLES[pRun[nStart + 27] + pLight[nSkip + 25] * 0x100];
                        case 25:
                            pRunDst[nDest + 24] = MAC_SHIFT_TABLES[pRun[nStart + 26] + pLight[nSkip + 24] * 0x100];
                        case 24:
                            pRunDst[nDest + 23] = MAC_SHIFT_TABLES[pRun[nStart + 25] + pLight[nSkip + 23] * 0x100];
                        case 23:
                            pRunDst[nDest + 22] = MAC_SHIFT_TABLES[pRun[nStart + 24] + pLight[nSkip + 22] * 0x100];
                        case 22:
                            pRunDst[nDest + 21] = MAC_SHIFT_TABLES[pRun[nStart + 23] + pLight[nSkip + 21] * 0x100];
                        case 21:
                            pRunDst[nDest + 20] = MAC_SHIFT_TABLES[pRun[nStart + 22] + pLight[nSkip + 20] * 0x100];
                        case 20:
                            pRunDst[nDest + 19] = MAC_SHIFT_TABLES[pRun[nStart + 21] + pLight[nSkip + 19] * 0x100];
                        case 19:
                            pRunDst[nDest + 18] = MAC_SHIFT_TABLES[pRun[nStart + 20] + pLight[nSkip + 18] * 0x100];
                        case 18:
                            pRunDst[nDest + 17] = MAC_SHIFT_TABLES[pRun[nStart + 19] + pLight[nSkip + 17] * 0x100];
                        case 17:
                            pRunDst[nDest + 16] = MAC_SHIFT_TABLES[pRun[nStart + 18] + pLight[nSkip + 16] * 0x100];
                        case 16:
                            pRunDst[nDest + 15] = MAC_SHIFT_TABLES[pRun[nStart + 17] + pLight[nSkip + 15] * 0x100];
                        case 15:
                            pRunDst[nDest + 14] = MAC_SHIFT_TABLES[pRun[nStart + 16] + pLight[nSkip + 14] * 0x100];
                        case 14:
                            pRunDst[nDest + 13] = MAC_SHIFT_TABLES[pRun[nStart + 15] + pLight[nSkip + 13] * 0x100];
                        case 13:
                            pRunDst[nDest + 12] = MAC_SHIFT_TABLES[pRun[nStart + 14] + pLight[nSkip + 12] * 0x100];
                        case 12:
                            pRunDst[nDest + 11] = MAC_SHIFT_TABLES[pRun[nStart + 13] + pLight[nSkip + 11] * 0x100];
                        case 11:
                            pRunDst[nDest + 10] = MAC_SHIFT_TABLES[pRun[nStart + 12] + pLight[nSkip + 10] * 0x100];
                        case 10:
                            pRunDst[nDest + 9] = MAC_SHIFT_TABLES[pRun[nStart + 11] + pLight[nSkip + 9] * 0x100];
                        case 9:
                            pRunDst[nDest + 8] = MAC_SHIFT_TABLES[pRun[nStart + 10] + pLight[nSkip + 8] * 0x100];
                        case 8:
                            pRunDst[nDest + 7] = MAC_SHIFT_TABLES[pRun[nStart + 9] + pLight[nSkip + 7] * 0x100];
                        case 7:
                            pRunDst[nDest + 6] = MAC_SHIFT_TABLES[pRun[nStart + 8] + pLight[nSkip + 6] * 0x100];
                        case 6:
                            pRunDst[nDest + 5] = MAC_SHIFT_TABLES[pRun[nStart + 7] + pLight[nSkip + 5] * 0x100];
                        case 5:
                            pRunDst[nDest + 4] = MAC_SHIFT_TABLES[pRun[nStart + 6] + pLight[nSkip + 4] * 0x100];
                        case 4:
                            pRunDst[nDest + 3] = MAC_SHIFT_TABLES[pRun[nStart + 5] + pLight[nSkip + 3] * 0x100];
                        case 3:
                            pRunDst[nDest + 2] = MAC_SHIFT_TABLES[pRun[nStart + 4] + pLight[nSkip + 2] * 0x100];
                        case 2:
                            pRunDst[nDest + 1] = MAC_SHIFT_TABLES[pRun[nStart + 3] + pLight[nSkip + 1] * 0x100];
                        case 1:
                            pRunDst[nDest + 0] = MAC_SHIFT_TABLES[pRun[nStart + 2] + pLight[nSkip + 0] * 0x100];
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

// 1.14d mac: 002b35e9
// Sets the clip rectangles of the tile blitters from the surface size.
void FUN_002b35e9(int nWidth, int nHeight) {
    int nBottom = nHeight - 0x2f;
    int nLeft = 0;
    if (g_nMacScreenSide == 2) {
        nLeft = nWidth / 2;
    } else if (g_nMacScreenSide == 1) {
        nWidth -= nWidth / 2;
    }
    g_tGroundClip.bTall = true;
    g_tGroundClip.tOuter.nBottom = nBottom;
    g_tGroundClip.tOuter.nLeft = nLeft - 32;
    g_tGroundClip.tOuter.nRight = nWidth;
    g_tGroundClip.tInner.nBottom = nHeight - 0x3f;
    g_tGroundClip.tInner.nLeft = nLeft;
    g_tGroundClip.tInner.nRight = nWidth - 32;
    g_tWallClip.bTall = true;
    g_tWallClip.tOuter.nBottom = nBottom;
    g_tWallClip.tOuter.nLeft = nLeft - 32;
    g_tWallClip.tOuter.nRight = nWidth;
    g_tWallClip.tInner.nBottom = nHeight - 0x4f;
    g_tWallClip.tInner.nLeft = nLeft;
    g_tWallClip.tInner.nRight = nWidth - 32;
    g_tSurfaceClip.nBottom = nBottom;
    g_tSurfaceClip.nLeft = nLeft;
    g_tSurfaceClip.nRight = nWidth;
}

// 1.14d mac: 002b36b6
// Fills the row-offset table with multiples of the surface pitch, from 32 rows above zero.
void FUN_002b36b6(void) {
    int nPitch = g_tSurface.nPitch;
    int nOffset = nPitch * -0x20;
    int* pEntry = g_anRowOffsets;
    int nCount = 0x2dc;
    do {
        *pEntry = nOffset;
        nOffset += nPitch;
        pEntry++;
        nCount--;
    } while (nCount != 0);
}

void fpDrawVisTile(const uint8_t* pSrc, int nX, int nY);
void FUN_002b38f3(const uint8_t* pSrc, int nX, int nY, uint8_t nLight);

// 1.14d mac: 002b36e8
// Slot nfpDrawVisTile: an unshaded run-length wall block, 32 rows, with a clipped path.
void fpDrawVisTile(const uint8_t* pSrc, int nX, int nY) {
    int nTop = 0;
    if (g_tWallClip.bTall != 0) {
        nTop = -0x20;
    }
    if (MacTileInBounds(&g_tWallClip.tOuter, nTop, nX, nY)) {
        uint8_t* pBase = (uint8_t*)g_tSurface.pBits;
        if (pBase == 0) {
            FRAMEPOINTER_FATAL(0x2f4);
            pBase = (uint8_t*)g_tSurface.pBits;
        }
        uint8_t* pDst = g_anRowOffsets[nY + 32] + nX + pBase;
        uint8_t* pDestRow = pDst;
        if (MacTileInBounds(&g_tWallClip.tInner, 0, nX, nY)) {
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
                    SMemCopy(pRunDst, (void*)(pSrc + 2), nRun);
                    pSrc = pSrc + nRun + 2;
                }
                pDst = pDestRow + g_tSurface.nPitch;
                nRows--;
                pSrc = pSrc + sizeof(uint16_t);
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
                    if (g_tSurfaceClip.nBottom <= nRow + nY) {
                        return;
                    }
                    if (nRow + nY < 0) {
                        pDst = pRunStart + nSkip + nRun;
                        pNext = pRun + nRun + 2;
                    } else {
                        int nPos = ((int)(pRunStart + nSkip) - (int)pDestRow) + nX;
                        int nEnd = g_tSurfaceClip.nRight - nPos;
                        if (nEnd < 0) {
                            nEnd = 0;
                        }
                        if ((int)nRun < nEnd) {
                            nEnd = nRun;
                        }
                        int nStart = g_tSurfaceClip.nLeft - nPos;
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
                            SMemCopy(pRunStart + nSkip + nStart, (void*)(pRun + nStart + 2), nSize);
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
    int nTop = g_tWallClip.bTall ? -32 : 0;
    if (!MacTileInBounds(&g_tWallClip.tOuter, nTop, nX, nY)) {
        return;
    }
    int nOffset = g_anRowOffsets[nY + 32] + nX;
    uint8_t* pBits = (uint8_t*)g_tSurface.pBits;
    uint8_t* pShift = g_abShiftTables[nLight >> 3];
    if (pBits == 0) {
        FRAMEPOINTER_FATAL(0x373);
    }
    uint8_t* pDest = pBits + nOffset;
    if (MacTileInBounds(&g_tWallClip.tInner, 0, nX, nY)) {
        FUN_002b224c(pSrc, pDest, pShift, 32);
    } else {
        FUN_002b2a08(nX, nY, pSrc, pDest, pShift, 32);
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
            if (g_tWallClip.bTall != 0) {
                nTop = -0x20;
            }
            if (!MacTileInBounds(&g_tWallClip.tOuter, nTop, nX, nY)) {
                return;
            }
            uint8_t* pBase = (uint8_t*)g_tSurface.pBits;
            if (pBase == 0) {
                FRAMEPOINTER_FATAL(0x3be);
                pBase = (uint8_t*)g_tSurface.pBits;
            }
            uint8_t* pDest = g_anRowOffsets[nY + 32] + nX + pBase;
            if (MacTileInBounds(&g_tWallClip.tInner, 0, nX, nY)) {
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

// 1.14d mac: 002b3be5
// Slot nfpDrawBlendedVisTile: a run-length wall block alpha-blended through a blend table
// chosen by the alpha byte, with a clipped path.
void fpDrawBlendedVisTile(const uint8_t* pSrc, int nX, int nY, uint8_t nAlpha) {
    uint8_t** ppTable;
    if (nAlpha >= 0xc0) {
        ppTable = &g_ppPaletteTables[0];
    } else if (nAlpha >= 0x80) {
        ppTable = &g_ppPaletteTables[1];
    } else if (nAlpha >= 0x40) {
        ppTable = &g_ppPaletteTables[2];
    } else {
        return;
    }
    int nTop = g_tWallClip.bTall ? -32 : 0;
    if (!MacTileInBounds(&g_tWallClip.tOuter, nTop, nX, nY)) {
        return;
    }
    const uint8_t* pBlend = *ppTable;
    MacSurface* pSurface = &g_tSurface;
    uint8_t* pBits = (uint8_t*)pSurface->pBits;
    if (pBits == 0) {
        FRAMEPOINTER_FATAL(0x3f8);
        pBits = (uint8_t*)pSurface->pBits;
    }
    uint8_t* pDestRow = g_anRowOffsets[nY + 32] + nX + pBits;
    if (MacTileInBounds(&g_tWallClip.tInner, 0, nX, nY)) {
        int nRows = 32;
        do {
            uint8_t* pDst = pDestRow;
            while (true) {
                uint32_t nSkip = pSrc[0];
                uint32_t nRun = pSrc[1];
                pSrc += sizeof(MacRunHeader);
                if (nSkip == 0 && nRun == 0) {
                    break;
                }
                pDst += nSkip;
                const uint8_t* pEnd = pSrc + nRun;
                while (pSrc < pEnd) {
                    *pDst = pBlend[(*pDst << 8) | *pSrc];
                    pDst++;
                    pSrc++;
                }
            }
            pDestRow += pSurface->nPitch;
        } while (--nRows != 0);
        return;
    }
    for (int nRow = 0; nRow < 32; nRow++) {
        int nRowY = nRow + nY;
        uint8_t* pDst = pDestRow;
        while (true) {
            uint32_t nSkip = pSrc[0];
            uint32_t nRun = pSrc[1];
            if (nSkip == 0 && nRun == 0) {
                break;
            }
            if (nRowY >= g_tSurfaceClip.nBottom) {
                return;
            }
            if (nRowY < 0) {
                pSrc += nRun + sizeof(MacRunHeader);
                pDst += nSkip + nRun;
                continue;
            }
            int nPos = (pDst + nSkip) - pDestRow + nX;
            int nEnd = g_tSurfaceClip.nRight - nPos;
            if (nEnd < 0) {
                nEnd = 0;
            }
            if (nEnd > (int)nRun) {
                nEnd = nRun;
            }
            int nStart = g_tSurfaceClip.nLeft - nPos;
            if (nStart < 0) {
                nStart = 0;
            }
            int nVisible = nEnd - nStart;
            if (nVisible < 0) {
                nVisible = 0;
            }
            const uint8_t* pRun = pSrc + sizeof(MacRunHeader);
            uint8_t* pRunDst = pDst + nSkip;
            pSrc += nRun + sizeof(MacRunHeader);
            pDst += nSkip + nRun;
            if (nVisible != 0) {
                for (int i = nStart; i < nVisible + nStart; i++) {
                    pRunDst[i] = pBlend[(pRunDst[i] << 8) | pRun[i]];
                }
            }
        }
        pDestRow += pSurface->nPitch;
        pSrc += sizeof(MacRunHeader);
    }
}

// 1.14d mac: 002b3e8b
// Slot nfpDrawRoofTile: a run-length wall block that is alpha-blended and lit from the light
// grid, the light coordinates stepped per row from the four corner lights. The clipped path
// starts a run's light at its first pixel, not at its first visible one.
void fpDrawRoofTile(const uint8_t* pSrc, int nX, int nY, const int* pCornerLights, uint8_t nAlpha) {
    uint8_t** ppTable;
    if (nAlpha >= 0xc0) {
        ppTable = &g_ppPaletteTables[0];
    } else if (nAlpha >= 0x80) {
        ppTable = &g_ppPaletteTables[1];
    } else if (nAlpha >= 0x40) {
        ppTable = &g_ppPaletteTables[2];
    } else {
        return;
    }
    int nTop = g_tWallClip.bTall ? -32 : 0;
    if (!MacTileInBounds(&g_tWallClip.tOuter, nTop, nX, nY)) {
        return;
    }
    int nLight0 = pCornerLights[0];
    int nLight1 = pCornerLights[1];
    int nLight2 = pCornerLights[2];
    int nLight3 = pCornerLights[3];
    const uint8_t* pBlend = *ppTable;
    int nLightY = nLight1 << 5;
    uint32_t nLightX = nLight0 << 5;
    MacSurface* pSurface = &g_tSurface;
    uint8_t* pBits = (uint8_t*)pSurface->pBits;
    if (pBits == 0) {
        FRAMEPOINTER_FATAL(0x48b);
        pBits = (uint8_t*)pSurface->pBits;
    }
    int nStepY = nLight2 - nLight1;
    int nStepX = nLight3 - nLight0;
    uint8_t* pDestRow = g_anRowOffsets[nY + 32] + nX + pBits;
    const uint8_t* pLight = g_abLightGrid + (nLightX >> 8) * 0x400 + (nLightY >> 8) * 0x20;
    if (MacTileInBounds(&g_tWallClip.tInner, 0, nX, nY)) {
        int nRows = 32;
        do {
            uint8_t* pDst = pDestRow;
            while (true) {
                uint32_t nSkip = pSrc[0];
                uint32_t nRun = pSrc[1];
                pSrc += sizeof(MacRunHeader);
                if (nSkip == 0 && nRun == 0) {
                    break;
                }
                pLight += nSkip;
                pDst += nSkip;
                const uint8_t* pEnd = pSrc + nRun;
                while (pSrc < pEnd) {
                    *pDst = pBlend[(g_abShiftTables[*pLight][*pSrc] << 8) | *pDst];
                    pDst++;
                    pLight++;
                    pSrc++;
                }
            }
            nLightY += nStepY;
            nLightX += nStepX;
            pLight = g_abLightGrid + (nLightX >> 8) * 0x400 + (nLightY >> 8) * 0x20;
            pDestRow += pSurface->nPitch;
        } while (--nRows != 0);
        return;
    }
    for (int nRow = 0; nRow < 32; nRow++) {
        int nRowY = nRow + nY;
        uint8_t* pDst = pDestRow;
        while (true) {
            uint32_t nSkip = pSrc[0];
            uint32_t nRun = pSrc[1];
            if (nSkip == 0 && nRun == 0) {
                break;
            }
            if (nRowY >= g_tSurfaceClip.nBottom) {
                return;
            }
            if (nRowY < 0) {
                pSrc += nRun + sizeof(MacRunHeader);
                pDst += nSkip + nRun;
                pLight += nSkip + nRun;
                continue;
            }
            const uint8_t* pRunLight = pLight + nSkip;
            int nPos = (pDst + nSkip) - pDestRow + nX;
            int nEnd = g_tSurfaceClip.nRight - nPos;
            if (nEnd < 0) {
                nEnd = 0;
            }
            if (nEnd > (int)nRun) {
                nEnd = nRun;
            }
            int nStart = g_tSurfaceClip.nLeft - nPos;
            if (nStart < 0) {
                nStart = 0;
            }
            int nVisible = nEnd - nStart;
            if (nVisible < 0) {
                nVisible = 0;
            }
            const uint8_t* pRun = pSrc + sizeof(MacRunHeader);
            uint8_t* pRunDst = pDst + nSkip;
            pSrc += nRun + sizeof(MacRunHeader);
            pLight += nRun + nSkip;
            pDst += nRun + nSkip;
            if (nVisible != 0) {
                for (int i = nStart; i < nVisible + nStart; i++) {
                    pRunDst[i] = pBlend[(g_abShiftTables[*pRunLight][pRun[i]] << 8) | pRunDst[i]];
                    pRunLight++;
                }
            }
        }
        nLightY += nStepY;
        nLightX += nStepX;
        pLight = g_abLightGrid + (nLightX >> 8) * 0x400 + (nLightY >> 8) * 0x20;
        pDestRow += pSurface->nPitch;
        pSrc += sizeof(MacRunHeader);
    }
}

// 1.14d mac: 002b4270
// A 15-row floor sub-block with one light level.
void FUN_002b4270(const MacGroundBlock* pBlock, uint8_t* pDest, uint8_t nLight) {
    if ((pBlock->nFlags & 4) == 0) {
        FUN_002b1a44(pBlock->pData, pDest, g_abShiftTables[nLight >> 3]);
    } else {
        FUN_002b224c(pBlock->pData, pDest, g_abShiftTables[nLight >> 3], 0xf);
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
// A 15-row floor sub-block with one light level, clipped against the surface clip;
// run-length blocks go to the clipped run-length blitter.
void FUN_002b4354(const MacGroundBlock* pBlock, int nX, int nY, uint8_t* pDest, uint8_t nLight) {
    int nShift = nLight >> 3;
    if (pBlock->nFlags & 4) {
        FUN_002b2a08(nX, nY, pBlock->pData, pDest, g_abShiftTables[nShift], 15);
        return;
    }
    int nRowEnd = g_tSurfaceClip.nBottom - nY;
    if (nRowEnd > 15) {
        nRowEnd = 15;
    }
    int nRow = nY < 0 ? -nY : 0;
    if (nRow >= nRowEnd) {
        return;
    }
    MacSurface* pSurface = &g_tSurface;
    pDest += pSurface->nPitch * nRow;
    for (; nRow < nRowEnd; nRow++) {
        int nRowX = g_anTileRowDest[nRow] + nX;
        int nEnd = g_tSurfaceClip.nRight - nRowX;
        if (nEnd < 0) {
            nEnd = 0;
        }
        if (nEnd > (int)g_anTileRowWidth[nRow]) {
            nEnd = g_anTileRowWidth[nRow];
        }
        int nStart = g_tSurfaceClip.nLeft - nRowX;
        if (nStart < 0) {
            nStart = 0;
        }
        int nCount = nEnd - nStart;
        if (nCount < 0) {
            nCount = 0;
        }
        if (nCount != 0) {
            int nSrc = g_anTileRowSrc[nRow] + nStart;
            const uint8_t* pData = pBlock->pData;
            uint8_t* pPlot = pDest + g_anTileRowDest[nRow] + nStart;
            for (const uint8_t* pPixel = pData + nSrc; pPixel < pData + nSrc + nCount; pPixel++) {
                *pPlot++ = g_abShiftTables[nShift][*pPixel];
            }
        }
        pDest += pSurface->nPitch;
    }
}

// 1.14d mac: 002b44c2
// A 15-row floor sub-block lit per pixel, clipped against the surface clip; the four corner
// lights give the start and steps, run-length blocks go to the clipped lit blitter. A row's
// light starts at the row's first pixel, not at its first visible one.
void FUN_002b44c2(const MacGroundBlock* pBlock, int nX, int nY, uint8_t* pDest, int* pCornerLights) {
    int nStepY = pCornerLights[2] - pCornerLights[1];
    int nStepX = pCornerLights[3] - pCornerLights[0];
    int nLightY = pCornerLights[1] << 4;
    uint32_t nLightX = pCornerLights[0] << 4;
    if (pBlock->nFlags & 4) {
        FUN_002b2da8(nX, nY, pBlock->pData, pDest, nLightX, nStepX, nLightY, nStepY, 15, 7);
        return;
    }
    int nRowEnd = g_tSurfaceClip.nBottom - nY;
    if (nRowEnd > 15) {
        nRowEnd = 15;
    }
    int nRow = nY < 0 ? -nY : 0;
    if (nRow >= nRowEnd) {
        return;
    }
    MacSurface* pSurface = &g_tSurface;
    pDest += pSurface->nPitch * nRow;
    for (; nRow < nRowEnd; nRow++) {
        int nRowX = g_anTileRowDest[nRow] + nX;
        int nEnd = g_tSurfaceClip.nRight - nRowX;
        if (nEnd < 0) {
            nEnd = 0;
        }
        if (nEnd > (int)g_anTileRowWidth[nRow]) {
            nEnd = g_anTileRowWidth[nRow];
        }
        int nStart = g_tSurfaceClip.nLeft - nRowX;
        if (nStart < 0) {
            nStart = 0;
        }
        int nCount = nEnd - nStart;
        if (nCount < 0) {
            nCount = 0;
        }
        if (nCount != 0) {
            int nSrc = g_anTileRowSrc[nRow] + nStart;
            const uint8_t* pData = pBlock->pData;
            uint8_t* pPlot = pDest + g_anTileRowDest[nRow] + nStart;
            const uint8_t* pLight = g_abLightGrid + (nLightX >> 7) * 0x400 + (nLightY >> 7) * 0x20 + g_anTileRowDest[nRow];
            for (const uint8_t* pPixel = pData + nSrc; pPixel < pData + nSrc + nCount; pPixel++) {
                *pPlot++ = g_abShiftTables[*pLight++][*pPixel];
            }
        }
        pDest += pSurface->nPitch;
        nLightX += nStepX;
        nLightY += nStepY;
    }
}

// 1.14d mac: 002b4696
// Slot nfpDrawGroundTile: walks the tile's sub-blocks, takes their corner lights from the
// light data and hands each to the floor-block helper that fits. The light data is a grid
// of 12-byte cells, 8 to a row; a sub-block's cell is at (nLightX + 1, nLightY + 1).
void fpDrawGroundTile(const MacGroundTile* pTile, int nScreenX, int nScreenY, const uint8_t* pLights) {
    if (pTile->nBlocks <= 0) {
        return;
    }
    MacSurface* pSurface = &g_tSurface;
    uint8_t* pBits = (uint8_t*)pSurface->pBits;
    int nScreenOffset = pSurface->nPitch * nScreenY + nScreenX;
    for (int nBlock = 0; nBlock < pTile->nBlocks; nBlock++) {
        const MacGroundBlock* pBlock = &pTile->pBlocks[nBlock];
        if (!(pBlock->nFlags & 1)) {
            continue;
        }
        int nBlockX = pBlock->nX;
        int nBlockY = pBlock->nY;
        int nTop = g_tGroundClip.bTall ? -16 : 0;
        int nY = nBlockY + nScreenY;
        int nX = nBlockX + nScreenX;
        if (!MacTileInBounds(&g_tGroundClip.tOuter, nTop, nX, nY)) {
            continue;
        }
        const uint8_t* pCell = pLights + (pBlock->nLightX + pBlock->nLightY * 8) * 12;
        uint8_t nLight = pCell[0x6c];
        bool bPerPixel = false;
        uint32_t anCorner[4];
        if (D2GFXRenderedData[2] == 0) {
            uint32_t nLight1 = pCell[0x78];
            uint32_t nLight3 = pCell[0xd8];
            uint32_t nLight2 = pCell[0xcc];
            uint32_t nLight0 = nLight;
            if (abs((int)(nLight1 - nLight0)) + abs((int)(nLight2 - nLight0)) + abs((int)(nLight3 - nLight1)) >= 10) {
                anCorner[0] = (pCell[0x60] + nLight0 + pCell[0xc0] + nLight2) >> 2;
                anCorner[1] = (nLight0 + nLight1 + pCell[0x0c] + pCell[0x18]) >> 2;
                anCorner[2] = (pCell[0x84] + nLight1 + nLight2 + pCell[0xe4]) >> 2;
                anCorner[3] = (nLight3 + nLight2 + pCell[0x12c] + pCell[0x138]) >> 2;
                nLight = 0;
                bPerPixel = true;
            }
        }
        uint8_t* pDest = nBlockY * pSurface->nPitch + nScreenOffset + nBlockX + pBits;
        if (MacTileInBounds(&g_tGroundClip.tInner, 0, nX, nY)) {
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

#endif
