#pragma once

#include <cstdint>

#ifndef D2_DEFINED_D2GfxCellNodeStrc
#define D2_DEFINED_D2GfxCellNodeStrc
// The renderer record of one cel frame, reached through DC6Block::nNextBlock
// (CELCMP_GetDC6BlockPixelData). Each loaded cel file allocates one per frame.
struct D2GfxCellModeCacheStrc {
    /* 0x00 */ int32_t nSizeClass; // texture size bucket, 0 until first drawn
    /* 0x04 */ int32_t nCacheNode; // 1-based node in that bucket's texture cache, 0 for none
};
struct D2GfxCellNodeStrc {
    /* 0x00 */ struct D2GfxCellModeCacheStrc aModes[4]; // indexed by draw mode (mode 4 shares 1)
    /* 0x20 */ int32_t field_0x20;
    /* 0x24 */ int32_t field_0x24;
    /* 0x28 */ int32_t field_0x28;
};
#endif // D2_DEFINED_D2GfxCellNodeStrc
