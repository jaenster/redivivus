// D2GFXMac.h - what the Mac-only D2gfx units share: the renderer table, the surface, the
// clip rectangles and the tile blitters (1.14d Intel Mac build, DiabloII_macho).
#pragma once

#include "D2Arch.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include <stddef.h>
#include <stdint.h>

typedef int BOOL;

#include "Source/StormMac/H/StormApi.h"
void fp_fatal(const char* szFormat, ...);
namespace Fog { namespace Src { namespace LogManager {
void LOGFILE_WriteStringToFile(char* szFormat, ...);
}}}
using Fog::Src::LogManager::LOGFILE_WriteStringToFile;

// The renderer's function table as the Mac build lays it out: only the slots D2gfx calls
// itself are named.
struct MacRendererFunctions {
    BOOL(*pfnInitialize)(int nOption);                 // 0x00
    BOOL(*pfnInitPerspective)(void* pSettings, void* pHelper);  // 0x04
    BOOL(*pfnRelease)(void);                           // 0x08
    BOOL(*pfnCreateWindow)(int nWindow, int nResolutionMode);   // 0x0C
    BOOL(*pfnDestroyWindow)(void);                     // 0x10
    void* unk14[6];                                    // 0x14
    BOOL(*pfnSlot2C)(void* pEvent);                            // 0x2C
    BOOL(*pfnSlot30)(void* pPoint);                            // 0x30
    BOOL(*pfnSlot34)(void* pPoint);                            // 0x34
    void* unk38;                                       // 0x38
    BOOL(*pfnCheckCutScene)(int nCommand, int nValue); // 0x3C
    void* unk40[35];                                   // 0x40
    void(*pfnDrawLine)(int nXStart, int nYStart, int nXEnd, int nYEnd, uint32_t nColor, uint32_t nAlpha);  // 0xCC
};

// The surface D2gfx draws into: what the helper-table slot fpFillYBufferTable stores.
struct MacSurface {
    void* pBits;   // 0x00
    int nPitch;    // 0x04
    int nHeight;   // 0x08
    int n0C;       // 0x0C
};

extern MacRendererFunctions* RENDERER_CurrentRenderedFunctions;
extern MacSurface g_tSurface;
extern int g_bMacFullScreen;
extern int g_nRendererInitOption;
extern int g_nMacScreenSide;
extern int g_nRowTableWidth;
// The 49 shift tables of 256 bytes (defined with the palette helpers); the tile code indexes them
// flat as light level * 256 + pixel.
extern uint8_t g_abShiftTables[49][256];
#define MAC_SHIFT_TABLES (&g_abShiftTables[0][0])

// A floor tile's sub-block (0x14 bytes): where it sits in the tile, its light cell and its
// pixels (uncompressed, or run-length when bit 2 of the flags is set).
struct MacGroundBlock {
    int16_t nX;            // 0x00
    int16_t nY;            // 0x02
    uint8_t unk04[2];
    uint8_t nLightX;       // 0x06
    uint8_t nLightY;       // 0x07
    uint8_t nFlags;        // 0x08: 1 drawn, 4 run-length
    uint8_t unk09[7];
    const uint8_t* pData;  // 0x10
};
struct MacGroundTile {
    uint8_t unk00[0x50];
    int nBlocks;                // 0x50
    MacGroundBlock* pBlocks;    // 0x54
};

extern int D2GFXRenderedData[];
struct MacClipBounds {
    int nBottom;   // 0x00
    int nLeft;     // 0x04
    int nRight;    // 0x08
};
struct MacTileClip {
    bool bTall;            // 0x00
    MacClipBounds tOuter;  // 0x04
    MacClipBounds tInner;  // 0x10
};
extern MacClipBounds g_tSurfaceClip;
extern MacTileClip g_tGroundClip;
extern MacTileClip g_tWallClip;
extern int g_anRowOffsets[732];
extern uint8_t g_abLightGrid[];
extern int g_anTileRowDest[15];
extern int g_anTileRowSrc[15];
extern uint32_t g_anTileRowWidth[15];

void* FOG_GetFramePointer(void);

void FUN_002b1a44(const uint8_t* pSrc, uint8_t* pDest, const uint8_t* pRemap);
void FUN_002b1ca9(const uint8_t* pSrc, uint8_t* pDest, int nLightX, int nLightXStep, int nLightY, int nLightYStep);
void FUN_002b224c(const uint8_t* pSrc, uint8_t* pDest, const uint8_t* pRemap, int nRows);
void FUN_002b24c4(const uint8_t* pSrc, uint8_t* pDest, int nLightX, int nLightXStep, int nLightY, int nLightYStep, int nRows, uint8_t nShift);
void FUN_002b2a08(int nX, int nY, const uint8_t* pSrc, uint8_t* pDest, const uint8_t* pRemap, int nRows);
void FUN_002b2da8(int nX, int nY, const uint8_t* pSrc, uint8_t* pDest, int nLightX, int nLightXStep, int nLightY, int nLightYStep, int nRows, uint8_t nShift);
void FUN_002b4270(const MacGroundBlock* pBlock, uint8_t* pDest, uint8_t nLight);
void FUN_002b42cf(const MacGroundBlock* pBlock, uint8_t* pDest, int* pCornerLights);
void FUN_002b4354(const MacGroundBlock* pBlock, int nX, int nY, uint8_t* pDest, uint8_t nLight);
void FUN_002b44c2(const MacGroundBlock* pBlock, int nX, int nY, uint8_t* pDest, int* pCornerLights);
void fpDrawVisTile(const uint8_t* pSrc, int nX, int nY);

#endif
