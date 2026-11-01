/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"
#include "Fog/Source/SSComp.h"
#include "Storm/3rdparty/zlib/deflate.h"
#include "Storm/3rdparty/zlib/trees.h"
#include "Storm/Source/SComp.h"
#include "Storm/Source/SErr.h"
#include "Storm/Storm.h"


// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

int gnSSCompDeflateFlags[32] = {
    -0x1,
    0x0,
    -0x1,
    0x4,
    -0x1,
    0x2,
    -0x1,
    0x6,
    -0x1,
    0x1,
    -0x1,
    0x5,
    -0x1,
    0x3,
    -0x1,
    0x7,
    -0x1,
    0x1,
    -0x1,
    0x5,
    -0x1,
    0x3,
    -0x1,
    0x7,
    -0x1,
    0x2,
    -0x1,
    0x4,
    -0x1,
    0x6,
    -0x1,
    0x8
}; D2_GLOBAL_GUARD(gnSSCompDeflateFlags, "gnSSCompDeflateFlags");

uint32_t gdwADPCMCoefficient[89] = {
    0x7,
    0x8,
    0x9,
    0xa,
    0xb,
    0xc,
    0xd,
    0xe,
    0x10,
    0x11,
    0x13,
    0x15,
    0x17,
    0x19,
    0x1c,
    0x1f,
    0x22,
    0x25,
    0x29,
    0x2d,
    0x32,
    0x37,
    0x3c,
    0x42,
    0x49,
    0x50,
    0x58,
    0x61,
    0x6b,
    0x76,
    0x82,
    0x8f,
    0x9d,
    0xad,
    0xbe,
    0xd1,
    0xe6,
    0xfd,
    0x117,
    0x133,
    0x151,
    0x173,
    0x198,
    0x1c1,
    0x1ee,
    0x220,
    0x256,
    0x292,
    0x2d4,
    0x31c,
    0x36c,
    0x3c3,
    0x424,
    0x48e,
    0x502,
    0x583,
    0x610,
    0x6ab,
    0x756,
    0x812,
    0x8e0,
    0x9c3,
    0xabd,
    0xbd0,
    0xcff,
    0xe4c,
    0xfba,
    0x114c,
    0x1307,
    0x14ee,
    0x1706,
    0x1954,
    0x1bdc,
    0x1ea5,
    0x21b6,
    0x2515,
    0x28ca,
    0x2cdf,
    0x315b,
    0x364b,
    0x3bb9,
    0x41b2,
    0x4844,
    0x4f7e,
    0x5771,
    0x602f,
    0x69ce,
    0x7462,
    0x7fff
}; D2_GLOBAL_GUARD(gdwADPCMCoefficient, "gdwADPCMCoefficient");

SCompCodecEntryStrc paSCompDecompressCodecTable[5] = {
    {
        /* .dwMethodMask = */ 0x40,
        /* .pfnCodec = */ (pfnSCompCodec)&Storm::Source::SComp::SCOMP_AdpcmMonoDecompress
    },
    {
        /* .dwMethodMask = */ 0x80,
        /* .pfnCodec = */ (pfnSCompCodec)&Storm::Source::SComp::SCOMP_AdpcmStereoDecompress
    },
    {
        /* .dwMethodMask = */ 0x1,
        /* .pfnCodec = */ (pfnSCompCodec)&Storm::Source::SComp::SCOMP_HuffmanDecompress
    },
    {
        /* .dwMethodMask = */ 0x2,
        /* .pfnCodec = */ (pfnSCompCodec)&Storm::Source::SComp::SCOMP_Bzip2Decompress
    },
    {
        /* .dwMethodMask = */ 0x8,
        /* .pfnCodec = */ &Storm::Source::SComp::SCOMP_PkwareDecompress
    }
}; D2_GLOBAL_GUARD(paSCompDecompressCodecTable, "paSCompDecompressCodecTable");

D2SSCompDeflateBlockEntryStrc gaSSCompDeflateBlockTable[10] = {
    {
        /* .nGoodLength = */ 0x0,
        /* .nMaxLazy = */ 0x0,
        /* .nNiceLength = */ 0x0,
        /* .nMaxChain = */ 0x0,
        /* .pfnDeflateBlock = */ &Storm::_3rdparty::zlib::deflate::deflate_stored
    },
    {
        /* .nGoodLength = */ 0x4,
        /* .nMaxLazy = */ 0x4,
        /* .nNiceLength = */ 0x8,
        /* .nMaxChain = */ 0x4,
        /* .pfnDeflateBlock = */ &Storm::_3rdparty::zlib::deflate::deflate_fast
    },
    {
        /* .nGoodLength = */ 0x4,
        /* .nMaxLazy = */ 0x5,
        /* .nNiceLength = */ 0x10,
        /* .nMaxChain = */ 0x8,
        /* .pfnDeflateBlock = */ &Storm::_3rdparty::zlib::deflate::deflate_fast
    },
    {
        /* .nGoodLength = */ 0x4,
        /* .nMaxLazy = */ 0x6,
        /* .nNiceLength = */ 0x20,
        /* .nMaxChain = */ 0x20,
        /* .pfnDeflateBlock = */ &Storm::_3rdparty::zlib::deflate::deflate_fast
    },
    {
        /* .nGoodLength = */ 0x4,
        /* .nMaxLazy = */ 0x4,
        /* .nNiceLength = */ 0x10,
        /* .nMaxChain = */ 0x10,
        /* .pfnDeflateBlock = */ &Storm::_3rdparty::zlib::deflate::deflate_slow
    },
    {
        /* .nGoodLength = */ 0x8,
        /* .nMaxLazy = */ 0x10,
        /* .nNiceLength = */ 0x20,
        /* .nMaxChain = */ 0x20,
        /* .pfnDeflateBlock = */ &Storm::_3rdparty::zlib::deflate::deflate_slow
    },
    {
        /* .nGoodLength = */ 0x8,
        /* .nMaxLazy = */ 0x10,
        /* .nNiceLength = */ 0x80,
        /* .nMaxChain = */ 0x80,
        /* .pfnDeflateBlock = */ &Storm::_3rdparty::zlib::deflate::deflate_slow
    },
    {
        /* .nGoodLength = */ 0x8,
        /* .nMaxLazy = */ 0x20,
        /* .nNiceLength = */ 0x80,
        /* .nMaxChain = */ 0x100,
        /* .pfnDeflateBlock = */ &Storm::_3rdparty::zlib::deflate::deflate_slow
    },
    {
        /* .nGoodLength = */ 0x20,
        /* .nMaxLazy = */ 0x80,
        /* .nNiceLength = */ 0x102,
        /* .nMaxChain = */ 0x400,
        /* .pfnDeflateBlock = */ &Storm::_3rdparty::zlib::deflate::deflate_slow
    },
    {
        /* .nGoodLength = */ 0x20,
        /* .nMaxLazy = */ 0x102,
        /* .nNiceLength = */ 0x102,
        /* .nMaxChain = */ 0x1000,
        /* .pfnDeflateBlock = */ &Storm::_3rdparty::zlib::deflate::deflate_slow
    }
}; D2_GLOBAL_GUARD(gaSSCompDeflateBlockTable, "gaSSCompDeflateBlockTable");

ZlibHuffCode gaZlibInflateTableMid[512] = {
    ZlibHuffCode(
        /* .op = */ '`',
        /* .bits = */ '\x07',
        /* .val = */ 0x0
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x50
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x10
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x08',
        /* .val = */ 0x73
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x1f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x70
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x30
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xc0
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0xa
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x60
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x20
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xa0
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x0
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x80
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x40
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xe0
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x6
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x58
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x18
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x90
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x3b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x78
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x38
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xd0
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0x11
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x68
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x28
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xb0
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x8
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x88
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x48
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xf0
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x4
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x54
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x14
    ),
    ZlibHuffCode(
        /* .op = */ '\x15',
        /* .bits = */ '\x08',
        /* .val = */ 0xe3
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x2b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x74
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x34
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xc8
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0xd
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x64
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x24
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xa8
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x4
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x84
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x44
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xe8
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x8
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x5c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x1c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x98
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x07',
        /* .val = */ 0x53
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x7c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x3c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xd8
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x17
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x6c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x2c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xb8
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0xc
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x8c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x4c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xf8
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x3
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x52
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x12
    ),
    ZlibHuffCode(
        /* .op = */ '\x15',
        /* .bits = */ '\x08',
        /* .val = */ 0xa3
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x23
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x72
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x32
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xc4
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0xb
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x62
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x22
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xa4
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x2
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x82
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x42
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xe4
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x7
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x5a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x1a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x94
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x07',
        /* .val = */ 0x43
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x7a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x3a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xd4
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x13
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x6a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x2a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xb4
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0xa
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x8a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x4a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xf4
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x5
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x56
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x16
    ),
    ZlibHuffCode(
        /* .op = */ '@',
        /* .bits = */ '\x08',
        /* .val = */ 0x0
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x33
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x76
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x36
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xcc
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0xf
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x66
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x26
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xac
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x6
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x86
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x46
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xec
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x9
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x5e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x1e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x9c
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x07',
        /* .val = */ 0x63
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x7e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x3e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xdc
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x1b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x6e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x2e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xbc
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0xe
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x8e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x4e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xfc
    ),
    ZlibHuffCode(
        /* .op = */ '`',
        /* .bits = */ '\x07',
        /* .val = */ 0x0
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x51
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x11
    ),
    ZlibHuffCode(
        /* .op = */ '\x15',
        /* .bits = */ '\x08',
        /* .val = */ 0x83
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x1f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x71
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x31
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xc2
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0xa
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x61
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x21
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xa2
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x1
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x81
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x41
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xe2
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x6
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x59
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x19
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x92
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x3b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x79
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x39
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xd2
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0x11
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x69
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x29
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xb2
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x9
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x89
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x49
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xf2
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x4
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x55
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x15
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x08',
        /* .val = */ 0x102
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x2b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x75
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x35
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xca
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0xd
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x65
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x25
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xaa
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x5
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x85
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x45
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xea
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x8
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x5d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x1d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x9a
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x07',
        /* .val = */ 0x53
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x7d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x3d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xda
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x17
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x6d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x2d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xba
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0xd
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x8d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x4d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xfa
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x3
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x53
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x13
    ),
    ZlibHuffCode(
        /* .op = */ '\x15',
        /* .bits = */ '\x08',
        /* .val = */ 0xc3
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x23
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x73
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x33
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xc6
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0xb
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x63
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x23
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xa6
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x3
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x83
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x43
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xe6
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x7
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x5b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x1b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x96
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x07',
        /* .val = */ 0x43
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x7b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x3b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xd6
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x13
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x6b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x2b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xb6
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0xb
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x8b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x4b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xf6
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x5
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x57
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x17
    ),
    ZlibHuffCode(
        /* .op = */ '@',
        /* .bits = */ '\x08',
        /* .val = */ 0x0
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x33
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x77
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x37
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xce
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0xf
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x67
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x27
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xae
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x7
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x87
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x47
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xee
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x9
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x5f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x1f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x9e
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x07',
        /* .val = */ 0x63
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x7f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x3f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xde
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x1b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x6f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x2f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xbe
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0xf
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x8f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x4f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xfe
    ),
    ZlibHuffCode(
        /* .op = */ '`',
        /* .bits = */ '\x07',
        /* .val = */ 0x0
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x50
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x10
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x08',
        /* .val = */ 0x73
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x1f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x70
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x30
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xc1
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0xa
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x60
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x20
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xa1
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x0
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x80
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x40
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xe1
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x6
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x58
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x18
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x91
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x3b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x78
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x38
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xd1
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0x11
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x68
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x28
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xb1
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x8
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x88
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x48
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xf1
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x4
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x54
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x14
    ),
    ZlibHuffCode(
        /* .op = */ '\x15',
        /* .bits = */ '\x08',
        /* .val = */ 0xe3
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x2b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x74
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x34
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xc9
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0xd
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x64
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x24
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xa9
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x4
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x84
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x44
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xe9
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x8
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x5c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x1c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x99
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x07',
        /* .val = */ 0x53
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x7c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x3c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xd9
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x17
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x6c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x2c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xb9
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0xc
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x8c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x4c
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xf9
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x3
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x52
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x12
    ),
    ZlibHuffCode(
        /* .op = */ '\x15',
        /* .bits = */ '\x08',
        /* .val = */ 0xa3
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x23
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x72
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x32
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xc5
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0xb
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x62
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x22
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xa5
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x2
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x82
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x42
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xe5
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x7
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x5a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x1a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x95
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x07',
        /* .val = */ 0x43
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x7a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x3a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xd5
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x13
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x6a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x2a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xb5
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0xa
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x8a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x4a
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xf5
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x5
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x56
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x16
    ),
    ZlibHuffCode(
        /* .op = */ '@',
        /* .bits = */ '\x08',
        /* .val = */ 0x0
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x33
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x76
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x36
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xcd
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0xf
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x66
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x26
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xad
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x6
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x86
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x46
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xed
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x9
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x5e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x1e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x9d
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x07',
        /* .val = */ 0x63
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x7e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x3e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xdd
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x1b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x6e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x2e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xbd
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0xe
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x8e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x4e
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xfd
    ),
    ZlibHuffCode(
        /* .op = */ '`',
        /* .bits = */ '\x07',
        /* .val = */ 0x0
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x51
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x11
    ),
    ZlibHuffCode(
        /* .op = */ '\x15',
        /* .bits = */ '\x08',
        /* .val = */ 0x83
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x1f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x71
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x31
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xc3
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0xa
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x61
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x21
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xa3
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x1
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x81
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x41
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xe3
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x6
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x59
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x19
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x93
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x3b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x79
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x39
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xd3
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0x11
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x69
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x29
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xb3
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x9
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x89
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x49
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xf3
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x4
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x55
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x15
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x08',
        /* .val = */ 0x102
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x2b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x75
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x35
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xcb
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0xd
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x65
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x25
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xab
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x5
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x85
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x45
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xeb
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x8
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x5d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x1d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x9b
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x07',
        /* .val = */ 0x53
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x7d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x3d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xdb
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x17
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x6d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x2d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xbb
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0xd
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x8d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x4d
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xfb
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x3
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x53
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x13
    ),
    ZlibHuffCode(
        /* .op = */ '\x15',
        /* .bits = */ '\x08',
        /* .val = */ 0xc3
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x23
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x73
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x33
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xc7
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0xb
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x63
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x23
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xa7
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x3
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x83
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x43
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xe7
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x7
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x5b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x1b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x97
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x07',
        /* .val = */ 0x43
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x7b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x3b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xd7
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x13
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x6b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x2b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xb7
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0xb
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x8b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x4b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xf7
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x5
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x57
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x17
    ),
    ZlibHuffCode(
        /* .op = */ '@',
        /* .bits = */ '\x08',
        /* .val = */ 0x0
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x07',
        /* .val = */ 0x33
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x77
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x37
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xcf
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x07',
        /* .val = */ 0xf
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x67
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x27
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xaf
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x7
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x87
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x47
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xef
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x07',
        /* .val = */ 0x9
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x5f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x1f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0x9f
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x07',
        /* .val = */ 0x63
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x7f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x3f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xdf
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x07',
        /* .val = */ 0x1b
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x6f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x2f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xbf
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0xf
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x8f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x08',
        /* .val = */ 0x4f
    ),
    ZlibHuffCode(
        /* .op = */ '\x00',
        /* .bits = */ '\x09',
        /* .val = */ 0xff
    )
}; D2_GLOBAL_GUARD(gaZlibInflateTableMid, "gaZlibInflateTableMid");

ZlibHuffCode gaZlibInflateTableStart[32] = {
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x05',
        /* .val = */ 0x1
    ),
    ZlibHuffCode(
        /* .op = */ '\x17',
        /* .bits = */ '\x05',
        /* .val = */ 0x101
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x05',
        /* .val = */ 0x11
    ),
    ZlibHuffCode(
        /* .op = */ '\x1b',
        /* .bits = */ '\x05',
        /* .val = */ 0x1001
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x05',
        /* .val = */ 0x5
    ),
    ZlibHuffCode(
        /* .op = */ '\x19',
        /* .bits = */ '\x05',
        /* .val = */ 0x401
    ),
    ZlibHuffCode(
        /* .op = */ '\x15',
        /* .bits = */ '\x05',
        /* .val = */ 0x41
    ),
    ZlibHuffCode(
        /* .op = */ '\x1d',
        /* .bits = */ '\x05',
        /* .val = */ 0x4001
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x05',
        /* .val = */ 0x3
    ),
    ZlibHuffCode(
        /* .op = */ '\x18',
        /* .bits = */ '\x05',
        /* .val = */ 0x201
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x05',
        /* .val = */ 0x21
    ),
    ZlibHuffCode(
        /* .op = */ '\x1c',
        /* .bits = */ '\x05',
        /* .val = */ 0x2001
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x05',
        /* .val = */ 0x9
    ),
    ZlibHuffCode(
        /* .op = */ '\x1a',
        /* .bits = */ '\x05',
        /* .val = */ 0x801
    ),
    ZlibHuffCode(
        /* .op = */ '\x16',
        /* .bits = */ '\x05',
        /* .val = */ 0x81
    ),
    ZlibHuffCode(
        /* .op = */ '@',
        /* .bits = */ '\x05',
        /* .val = */ 0x0
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x05',
        /* .val = */ 0x2
    ),
    ZlibHuffCode(
        /* .op = */ '\x17',
        /* .bits = */ '\x05',
        /* .val = */ 0x181
    ),
    ZlibHuffCode(
        /* .op = */ '\x13',
        /* .bits = */ '\x05',
        /* .val = */ 0x19
    ),
    ZlibHuffCode(
        /* .op = */ '\x1b',
        /* .bits = */ '\x05',
        /* .val = */ 0x1801
    ),
    ZlibHuffCode(
        /* .op = */ '\x11',
        /* .bits = */ '\x05',
        /* .val = */ 0x7
    ),
    ZlibHuffCode(
        /* .op = */ '\x19',
        /* .bits = */ '\x05',
        /* .val = */ 0x601
    ),
    ZlibHuffCode(
        /* .op = */ '\x15',
        /* .bits = */ '\x05',
        /* .val = */ 0x61
    ),
    ZlibHuffCode(
        /* .op = */ '\x1d',
        /* .bits = */ '\x05',
        /* .val = */ 0x6001
    ),
    ZlibHuffCode(
        /* .op = */ '\x10',
        /* .bits = */ '\x05',
        /* .val = */ 0x4
    ),
    ZlibHuffCode(
        /* .op = */ '\x18',
        /* .bits = */ '\x05',
        /* .val = */ 0x301
    ),
    ZlibHuffCode(
        /* .op = */ '\x14',
        /* .bits = */ '\x05',
        /* .val = */ 0x31
    ),
    ZlibHuffCode(
        /* .op = */ '\x1c',
        /* .bits = */ '\x05',
        /* .val = */ 0x3001
    ),
    ZlibHuffCode(
        /* .op = */ '\x12',
        /* .bits = */ '\x05',
        /* .val = */ 0xd
    ),
    ZlibHuffCode(
        /* .op = */ '\x1a',
        /* .bits = */ '\x05',
        /* .val = */ 0xc01
    ),
    ZlibHuffCode(
        /* .op = */ '\x16',
        /* .bits = */ '\x05',
        /* .val = */ 0xc1
    ),
    ZlibHuffCode(
        /* .op = */ '@',
        /* .bits = */ '\x05',
        /* .val = */ 0x0
    )
}; D2_GLOBAL_GUARD(gaZlibInflateTableStart, "gaZlibInflateTableStart");

// also labelled: gnSSCompLZHBufferState, gaZlibBlTreeCodeOrderMinus3, cZlibBuildBlTree_0
D2ZlibExtraLBitsBlOrderStrc gZlibExtraLBitsBlOrder = {
    /* .extra_lbits = */ {
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x1,
        0x1,
        0x1,
        0x1,
        0x2,
        0x2,
        0x2,
        0x2,
        0x3,
        0x3,
        0x3,
        0x3,
        0x4,
        0x4,
        0x4,
        0x4,
        0x5,
        0x5,
        0x5,
        0x5,
        0x0
    },
    /* .bl_order = */ {
        0x10,
        0x11,
        0x12,
        0x0,
        0x8,
        0x7,
        0x9,
        0x6,
        0xa,
        0x5,
        0xb,
        0x4,
        0xc,
        0x3,
        0xd,
        0x2,
        0xe,
        0x1,
        0xf
    },
    /* .pad = */ 0x0
}; D2_GLOBAL_GUARD(gZlibExtraLBitsBlOrder, "gZlibExtraLBitsBlOrder");

// also labelled: gnSSCompLZHCompressedSize
int gaZlibExtraDBits[30] = {
    0x0,
    0x0,
    0x0,
    0x0,
    0x1,
    0x1,
    0x2,
    0x2,
    0x3,
    0x3,
    0x4,
    0x4,
    0x5,
    0x5,
    0x6,
    0x6,
    0x7,
    0x7,
    0x8,
    0x8,
    0x9,
    0x9,
    0xa,
    0xa,
    0xb,
    0xb,
    0xc,
    0xc,
    0xd,
    0xd
}; D2_GLOBAL_GUARD(gaZlibExtraDBits, "gaZlibExtraDBits");

// also labelled: gnSSCompBLZCompressedSize
ct_data gaZlibStaticLTree[288] = {
    ct_data(
        /* .fc = */ 0xc,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x8c,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x4c,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xcc,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x2c,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xac,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x6c,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xec,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x1c,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x9c,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x5c,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xdc,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x3c,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xbc,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x7c,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xfc,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x2,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x82,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x42,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xc2,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x22,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xa2,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x62,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xe2,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x12,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x92,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x52,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xd2,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x32,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xb2,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x72,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xf2,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xa,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x8a,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x4a,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xca,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x2a,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xaa,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x6a,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xea,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x1a,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x9a,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x5a,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xda,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x3a,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xba,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x7a,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xfa,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x6,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x86,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x46,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xc6,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x26,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xa6,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x66,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xe6,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x16,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x96,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x56,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xd6,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x36,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xb6,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x76,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xf6,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xe,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x8e,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x4e,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xce,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x2e,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xae,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x6e,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xee,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x1e,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x9e,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x5e,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xde,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x3e,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xbe,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x7e,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xfe,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x1,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x81,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x41,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xc1,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x21,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xa1,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x61,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xe1,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x11,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x91,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x51,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xd1,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x31,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xb1,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x71,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xf1,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x9,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x89,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x49,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xc9,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x29,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xa9,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x69,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xe9,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x19,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x99,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x59,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xd9,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x39,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xb9,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x79,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xf9,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x5,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x85,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x45,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xc5,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x25,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xa5,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x65,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xe5,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x15,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x95,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x55,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xd5,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x35,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xb5,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x75,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xf5,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xd,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x8d,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x4d,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xcd,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x2d,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xad,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x6d,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xed,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x1d,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x9d,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x5d,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xdd,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x3d,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xbd,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x7d,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xfd,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x13,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x113,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x93,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x193,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x53,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x153,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xd3,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1d3,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x33,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x133,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xb3,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1b3,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x73,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x173,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xf3,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1f3,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xb,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x10b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x8b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x18b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x4b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x14b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xcb,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1cb,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x2b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x12b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xab,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1ab,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x6b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x16b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xeb,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1eb,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x11b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x9b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x19b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x5b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x15b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xdb,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1db,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x3b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x13b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xbb,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1bb,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x7b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x17b,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xfb,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1fb,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x7,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x107,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x87,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x187,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x47,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x147,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xc7,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1c7,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x27,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x127,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xa7,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1a7,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x67,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x167,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xe7,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1e7,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x17,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x117,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x97,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x197,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x57,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x157,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xd7,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1d7,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x37,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x137,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xb7,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1b7,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x77,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x177,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xf7,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1f7,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xf,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x10f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x8f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x18f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x4f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x14f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xcf,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1cf,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x2f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x12f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xaf,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1af,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x6f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x16f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xef,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1ef,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x11f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x9f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x19f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x5f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x15f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xdf,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1df,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x3f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x13f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xbf,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1bf,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x7f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x17f,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0xff,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x1ff,
        /* .dl = */ 0x9
    ),
    ct_data(
        /* .fc = */ 0x0,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x40,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x20,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x60,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x10,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x50,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x30,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x70,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x8,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x48,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x28,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x68,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x18,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x58,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x38,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x78,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x4,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x44,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x24,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x64,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x14,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x54,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x34,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x74,
        /* .dl = */ 0x7
    ),
    ct_data(
        /* .fc = */ 0x3,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x83,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x43,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xc3,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x23,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xa3,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0x63,
        /* .dl = */ 0x8
    ),
    ct_data(
        /* .fc = */ 0xe3,
        /* .dl = */ 0x8
    )
}; D2_GLOBAL_GUARD(gaZlibStaticLTree, "gaZlibStaticLTree");

// also labelled: gnSSCompPHDCompressedSize, gaZlibDistCodeMinus1
D2ZlibStaticDTreeDistCodeStrc gZlibStaticDTreeDistCode = {
    /* .static_dtree = */ {
        ct_data(
            /* .fc = */ 0x0,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x10,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x8,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x18,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x4,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x14,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0xc,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x1c,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x2,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x12,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0xa,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x1a,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x6,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x16,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0xe,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x1e,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x1,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x11,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x9,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x19,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x5,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x15,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0xd,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x1d,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x3,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x13,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0xb,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x1b,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x7,
            /* .dl = */ 0x5
        ),
        ct_data(
            /* .fc = */ 0x17,
            /* .dl = */ 0x5
        )
    },
    /* ._dist_code = */ {
        0x0,
        0x1,
        0x2,
        0x3,
        0x4,
        0x4,
        0x5,
        0x5,
        0x6,
        0x6,
        0x6,
        0x6,
        0x7,
        0x7,
        0x7,
        0x7,
        0x8,
        0x8,
        0x8,
        0x8,
        0x8,
        0x8,
        0x8,
        0x8,
        0x9,
        0x9,
        0x9,
        0x9,
        0x9,
        0x9,
        0x9,
        0x9,
        0xa,
        0xa,
        0xa,
        0xa,
        0xa,
        0xa,
        0xa,
        0xa,
        0xa,
        0xa,
        0xa,
        0xa,
        0xa,
        0xa,
        0xa,
        0xa,
        0xb,
        0xb,
        0xb,
        0xb,
        0xb,
        0xb,
        0xb,
        0xb,
        0xb,
        0xb,
        0xb,
        0xb,
        0xb,
        0xb,
        0xb,
        0xb,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xc,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xd,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xe,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0xf,
        0x0,
        0x0,
        0x10,
        0x11,
        0x12,
        0x12,
        0x13,
        0x13,
        0x14,
        0x14,
        0x14,
        0x14,
        0x15,
        0x15,
        0x15,
        0x15,
        0x16,
        0x16,
        0x16,
        0x16,
        0x16,
        0x16,
        0x16,
        0x16,
        0x17,
        0x17,
        0x17,
        0x17,
        0x17,
        0x17,
        0x17,
        0x17,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x19,
        0x19,
        0x19,
        0x19,
        0x19,
        0x19,
        0x19,
        0x19,
        0x19,
        0x19,
        0x19,
        0x19,
        0x19,
        0x19,
        0x19,
        0x19,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1a,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1b,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1c,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d,
        0x1d
    }
}; D2_GLOBAL_GUARD(gZlibStaticDTreeDistCode, "gZlibStaticDTreeDistCode");

uint8_t gaZlibLengthCodeTable[256] = {
    0x0,
    0x1,
    0x2,
    0x3,
    0x4,
    0x5,
    0x6,
    0x7,
    0x8,
    0x8,
    0x9,
    0x9,
    0xa,
    0xa,
    0xb,
    0xb,
    0xc,
    0xc,
    0xc,
    0xc,
    0xd,
    0xd,
    0xd,
    0xd,
    0xe,
    0xe,
    0xe,
    0xe,
    0xf,
    0xf,
    0xf,
    0xf,
    0x10,
    0x10,
    0x10,
    0x10,
    0x10,
    0x10,
    0x10,
    0x10,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x12,
    0x12,
    0x12,
    0x12,
    0x12,
    0x12,
    0x12,
    0x12,
    0x13,
    0x13,
    0x13,
    0x13,
    0x13,
    0x13,
    0x13,
    0x13,
    0x14,
    0x14,
    0x14,
    0x14,
    0x14,
    0x14,
    0x14,
    0x14,
    0x14,
    0x14,
    0x14,
    0x14,
    0x14,
    0x14,
    0x14,
    0x14,
    0x15,
    0x15,
    0x15,
    0x15,
    0x15,
    0x15,
    0x15,
    0x15,
    0x15,
    0x15,
    0x15,
    0x15,
    0x15,
    0x15,
    0x15,
    0x15,
    0x16,
    0x16,
    0x16,
    0x16,
    0x16,
    0x16,
    0x16,
    0x16,
    0x16,
    0x16,
    0x16,
    0x16,
    0x16,
    0x16,
    0x16,
    0x16,
    0x17,
    0x17,
    0x17,
    0x17,
    0x17,
    0x17,
    0x17,
    0x17,
    0x17,
    0x17,
    0x17,
    0x17,
    0x17,
    0x17,
    0x17,
    0x17,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x19,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1a,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1b,
    0x1c
}; D2_GLOBAL_GUARD(gaZlibLengthCodeTable, "gaZlibLengthCodeTable");

uint32_t gdwCrc32Table0[256] = {
    0x0,
    0x77073096,
    0xee0e612c,
    0x990951ba,
    0x76dc419,
    0x706af48f,
    0xe963a535,
    0x9e6495a3,
    0xedb8832,
    0x79dcb8a4,
    0xe0d5e91e,
    0x97d2d988,
    0x9b64c2b,
    0x7eb17cbd,
    0xe7b82d07,
    0x90bf1d91,
    0x1db71064,
    0x6ab020f2,
    0xf3b97148,
    0x84be41de,
    0x1adad47d,
    0x6ddde4eb,
    0xf4d4b551,
    0x83d385c7,
    0x136c9856,
    0x646ba8c0,
    0xfd62f97a,
    0x8a65c9ec,
    0x14015c4f,
    0x63066cd9,
    0xfa0f3d63,
    0x8d080df5,
    0x3b6e20c8,
    0x4c69105e,
    0xd56041e4,
    0xa2677172,
    0x3c03e4d1,
    0x4b04d447,
    0xd20d85fd,
    0xa50ab56b,
    0x35b5a8fa,
    0x42b2986c,
    0xdbbbc9d6,
    0xacbcf940,
    0x32d86ce3,
    0x45df5c75,
    0xdcd60dcf,
    0xabd13d59,
    0x26d930ac,
    0x51de003a,
    0xc8d75180,
    0xbfd06116,
    0x21b4f4b5,
    0x56b3c423,
    0xcfba9599,
    0xb8bda50f,
    0x2802b89e,
    0x5f058808,
    0xc60cd9b2,
    0xb10be924,
    0x2f6f7c87,
    0x58684c11,
    0xc1611dab,
    0xb6662d3d,
    0x76dc4190,
    0x1db7106,
    0x98d220bc,
    0xefd5102a,
    0x71b18589,
    0x6b6b51f,
    0x9fbfe4a5,
    0xe8b8d433,
    0x7807c9a2,
    0xf00f934,
    0x9609a88e,
    0xe10e9818,
    0x7f6a0dbb,
    0x86d3d2d,
    0x91646c97,
    0xe6635c01,
    0x6b6b51f4,
    0x1c6c6162,
    0x856530d8,
    0xf262004e,
    0x6c0695ed,
    0x1b01a57b,
    0x8208f4c1,
    0xf50fc457,
    0x65b0d9c6,
    0x12b7e950,
    0x8bbeb8ea,
    0xfcb9887c,
    0x62dd1ddf,
    0x15da2d49,
    0x8cd37cf3,
    0xfbd44c65,
    0x4db26158,
    0x3ab551ce,
    0xa3bc0074,
    0xd4bb30e2,
    0x4adfa541,
    0x3dd895d7,
    0xa4d1c46d,
    0xd3d6f4fb,
    0x4369e96a,
    0x346ed9fc,
    0xad678846,
    0xda60b8d0,
    0x44042d73,
    0x33031de5,
    0xaa0a4c5f,
    0xdd0d7cc9,
    0x5005713c,
    0x270241aa,
    0xbe0b1010,
    0xc90c2086,
    0x5768b525,
    0x206f85b3,
    0xb966d409,
    0xce61e49f,
    0x5edef90e,
    0x29d9c998,
    0xb0d09822,
    0xc7d7a8b4,
    0x59b33d17,
    0x2eb40d81,
    0xb7bd5c3b,
    0xc0ba6cad,
    0xedb88320,
    0x9abfb3b6,
    0x3b6e20c,
    0x74b1d29a,
    0xead54739,
    0x9dd277af,
    0x4db2615,
    0x73dc1683,
    0xe3630b12,
    0x94643b84,
    0xd6d6a3e,
    0x7a6a5aa8,
    0xe40ecf0b,
    0x9309ff9d,
    0xa00ae27,
    0x7d079eb1,
    0xf00f9344,
    0x8708a3d2,
    0x1e01f268,
    0x6906c2fe,
    0xf762575d,
    0x806567cb,
    0x196c3671,
    0x6e6b06e7,
    0xfed41b76,
    0x89d32be0,
    0x10da7a5a,
    0x67dd4acc,
    0xf9b9df6f,
    0x8ebeeff9,
    0x17b7be43,
    0x60b08ed5,
    0xd6d6a3e8,
    0xa1d1937e,
    0x38d8c2c4,
    0x4fdff252,
    0xd1bb67f1,
    0xa6bc5767,
    0x3fb506dd,
    0x48b2364b,
    0xd80d2bda,
    0xaf0a1b4c,
    0x36034af6,
    0x41047a60,
    0xdf60efc3,
    0xa867df55,
    0x316e8eef,
    0x4669be79,
    0xcb61b38c,
    0xbc66831a,
    0x256fd2a0,
    0x5268e236,
    0xcc0c7795,
    0xbb0b4703,
    0x220216b9,
    0x5505262f,
    0xc5ba3bbe,
    0xb2bd0b28,
    0x2bb45a92,
    0x5cb36a04,
    0xc2d7ffa7,
    0xb5d0cf31,
    0x2cd99e8b,
    0x5bdeae1d,
    0x9b64c2b0,
    0xec63f226,
    0x756aa39c,
    0x26d930a,
    0x9c0906a9,
    0xeb0e363f,
    0x72076785,
    0x5005713,
    0x95bf4a82,
    0xe2b87a14,
    0x7bb12bae,
    0xcb61b38,
    0x92d28e9b,
    0xe5d5be0d,
    0x7cdcefb7,
    0xbdbdf21,
    0x86d3d2d4,
    0xf1d4e242,
    0x68ddb3f8,
    0x1fda836e,
    0x81be16cd,
    0xf6b9265b,
    0x6fb077e1,
    0x18b74777,
    0x88085ae6,
    0xff0f6a70,
    0x66063bca,
    0x11010b5c,
    0x8f659eff,
    0xf862ae69,
    0x616bffd3,
    0x166ccf45,
    0xa00ae278,
    0xd70dd2ee,
    0x4e048354,
    0x3903b3c2,
    0xa7672661,
    0xd06016f7,
    0x4969474d,
    0x3e6e77db,
    0xaed16a4a,
    0xd9d65adc,
    0x40df0b66,
    0x37d83bf0,
    0xa9bcae53,
    0xdebb9ec5,
    0x47b2cf7f,
    0x30b5ffe9,
    0xbdbdf21c,
    0xcabac28a,
    0x53b39330,
    0x24b4a3a6,
    0xbad03605,
    0xcdd70693,
    0x54de5729,
    0x23d967bf,
    0xb3667a2e,
    0xc4614ab8,
    0x5d681b02,
    0x2a6f2b94,
    0xb40bbe37,
    0xc30c8ea1,
    0x5a05df1b,
    0x2d02ef8d
}; D2_GLOBAL_GUARD(gdwCrc32Table0, "gdwCrc32Table0");

uint32_t gdwCrc32Table5[256] = {
    0x0,
    0x6567bcb8,
    0x8bc809aa,
    0xeeafb512,
    0x5797628f,
    0x32f0de37,
    0xdc5f6b25,
    0xb938d79d,
    0xef28b4c5,
    0x8a4f087d,
    0x64e0bd6f,
    0x18701d7,
    0xb8bfd64a,
    0xddd86af2,
    0x3377dfe0,
    0x56106358,
    0x9f571950,
    0xfa30a5e8,
    0x149f10fa,
    0x71f8ac42,
    0xc8c07bdf,
    0xada7c767,
    0x43087275,
    0x266fcecd,
    0x707fad95,
    0x1518112d,
    0xfbb7a43f,
    0x9ed01887,
    0x27e8cf1a,
    0x428f73a2,
    0xac20c6b0,
    0xc9477a08,
    0x3eaf32a0,
    0x5bc88e18,
    0xb5673b0a,
    0xd00087b2,
    0x6938502f,
    0xc5fec97,
    0xe2f05985,
    0x8797e53d,
    0xd1878665,
    0xb4e03add,
    0x5a4f8fcf,
    0x3f283377,
    0x8610e4ea,
    0xe3775852,
    0xdd8ed40,
    0x68bf51f8,
    0xa1f82bf0,
    0xc49f9748,
    0x2a30225a,
    0x4f579ee2,
    0xf66f497f,
    0x9308f5c7,
    0x7da740d5,
    0x18c0fc6d,
    0x4ed09f35,
    0x2bb7238d,
    0xc518969f,
    0xa07f2a27,
    0x1947fdba,
    0x7c204102,
    0x928ff410,
    0xf7e848a8,
    0x3d58149b,
    0x583fa823,
    0xb6901d31,
    0xd3f7a189,
    0x6acf7614,
    0xfa8caac,
    0xe1077fbe,
    0x8460c306,
    0xd270a05e,
    0xb7171ce6,
    0x59b8a9f4,
    0x3cdf154c,
    0x85e7c2d1,
    0xe0807e69,
    0xe2fcb7b,
    0x6b4877c3,
    0xa20f0dcb,
    0xc768b173,
    0x29c70461,
    0x4ca0b8d9,
    0xf5986f44,
    0x90ffd3fc,
    0x7e5066ee,
    0x1b37da56,
    0x4d27b90e,
    0x284005b6,
    0xc6efb0a4,
    0xa3880c1c,
    0x1ab0db81,
    0x7fd76739,
    0x9178d22b,
    0xf41f6e93,
    0x3f7263b,
    0x66909a83,
    0x883f2f91,
    0xed589329,
    0x546044b4,
    0x3107f80c,
    0xdfa84d1e,
    0xbacff1a6,
    0xecdf92fe,
    0x89b82e46,
    0x67179b54,
    0x27027ec,
    0xbb48f071,
    0xde2f4cc9,
    0x3080f9db,
    0x55e74563,
    0x9ca03f6b,
    0xf9c783d3,
    0x176836c1,
    0x720f8a79,
    0xcb375de4,
    0xae50e15c,
    0x40ff544e,
    0x2598e8f6,
    0x73888bae,
    0x16ef3716,
    0xf8408204,
    0x9d273ebc,
    0x241fe921,
    0x41785599,
    0xafd7e08b,
    0xcab05c33,
    0x3bb659ed,
    0x5ed1e555,
    0xb07e5047,
    0xd519ecff,
    0x6c213b62,
    0x94687da,
    0xe7e932c8,
    0x828e8e70,
    0xd49eed28,
    0xb1f95190,
    0x5f56e482,
    0x3a31583a,
    0x83098fa7,
    0xe66e331f,
    0x8c1860d,
    0x6da63ab5,
    0xa4e140bd,
    0xc186fc05,
    0x2f294917,
    0x4a4ef5af,
    0xf3762232,
    0x96119e8a,
    0x78be2b98,
    0x1dd99720,
    0x4bc9f478,
    0x2eae48c0,
    0xc001fdd2,
    0xa566416a,
    0x1c5e96f7,
    0x79392a4f,
    0x97969f5d,
    0xf2f123e5,
    0x5196b4d,
    0x607ed7f5,
    0x8ed162e7,
    0xebb6de5f,
    0x528e09c2,
    0x37e9b57a,
    0xd9460068,
    0xbc21bcd0,
    0xea31df88,
    0x8f566330,
    0x61f9d622,
    0x49e6a9a,
    0xbda6bd07,
    0xd8c101bf,
    0x366eb4ad,
    0x53090815,
    0x9a4e721d,
    0xff29cea5,
    0x11867bb7,
    0x74e1c70f,
    0xcdd91092,
    0xa8beac2a,
    0x46111938,
    0x2376a580,
    0x7566c6d8,
    0x10017a60,
    0xfeaecf72,
    0x9bc973ca,
    0x22f1a457,
    0x479618ef,
    0xa939adfd,
    0xcc5e1145,
    0x6ee4d76,
    0x6389f1ce,
    0x8d2644dc,
    0xe841f864,
    0x51792ff9,
    0x341e9341,
    0xdab12653,
    0xbfd69aeb,
    0xe9c6f9b3,
    0x8ca1450b,
    0x620ef019,
    0x7694ca1,
    0xbe519b3c,
    0xdb362784,
    0x35999296,
    0x50fe2e2e,
    0x99b95426,
    0xfcdee89e,
    0x12715d8c,
    0x7716e134,
    0xce2e36a9,
    0xab498a11,
    0x45e63f03,
    0x208183bb,
    0x7691e0e3,
    0x13f65c5b,
    0xfd59e949,
    0x983e55f1,
    0x2106826c,
    0x44613ed4,
    0xaace8bc6,
    0xcfa9377e,
    0x38417fd6,
    0x5d26c36e,
    0xb389767c,
    0xd6eecac4,
    0x6fd61d59,
    0xab1a1e1,
    0xe41e14f3,
    0x8179a84b,
    0xd769cb13,
    0xb20e77ab,
    0x5ca1c2b9,
    0x39c67e01,
    0x80fea99c,
    0xe5991524,
    0xb36a036,
    0x6e511c8e,
    0xa7166686,
    0xc271da3e,
    0x2cde6f2c,
    0x49b9d394,
    0xf0810409,
    0x95e6b8b1,
    0x7b490da3,
    0x1e2eb11b,
    0x483ed243,
    0x2d596efb,
    0xc3f6dbe9,
    0xa6916751,
    0x1fa9b0cc,
    0x7ace0c74,
    0x9461b966,
    0xf10605de
}; D2_GLOBAL_GUARD(gdwCrc32Table5, "gdwCrc32Table5");

int gdwUTF8ConversionTab[6] = { 0x0, 0x3080, 0xe2080, 0x3c82080, -0x5f7df80, -0x7df7df80 }; D2_GLOBAL_GUARD(gdwUTF8ConversionTab, "gdwUTF8ConversionTab");

uint8_t gaUTF8ByteCountTable[256] = {
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x4,
    0x4,
    0x4,
    0x4,
    0x5,
    0x5,
    0x5,
    0x5
}; D2_GLOBAL_GUARD(gaUTF8ByteCountTable, "gaUTF8ByteCountTable");

uint16_t gszUnicodeLatin1_1[256] = {
    0x0,
    0x1,
    0x2,
    0x3,
    0x4,
    0x5,
    0x6,
    0x7,
    0x8,
    0x9,
    0xa,
    0xb,
    0xc,
    0xd,
    0xe,
    0xf,
    0x10,
    0x11,
    0x12,
    0x13,
    0x14,
    0x15,
    0x16,
    0x17,
    0x18,
    0x19,
    0x1a,
    0x1b,
    0x1c,
    0x1d,
    0x1e,
    0x1f,
    0x20,
    0x21,
    0x22,
    0x23,
    0x24,
    0x25,
    0x26,
    0x27,
    0x28,
    0x29,
    0x2a,
    0x2b,
    0x2c,
    0x2d,
    0x2e,
    0x2f,
    0x30,
    0x31,
    0x32,
    0x33,
    0x34,
    0x35,
    0x36,
    0x37,
    0x38,
    0x39,
    0x3a,
    0x3b,
    0x3c,
    0x3d,
    0x3e,
    0x3f,
    0x40,
    0x41,
    0x42,
    0x43,
    0x44,
    0x45,
    0x46,
    0x47,
    0x48,
    0x49,
    0x4a,
    0x4b,
    0x4c,
    0x4d,
    0x4e,
    0x4f,
    0x50,
    0x51,
    0x52,
    0x53,
    0x54,
    0x55,
    0x56,
    0x57,
    0x58,
    0x59,
    0x5a,
    0x5b,
    0x5c,
    0x5d,
    0x5e,
    0x5f,
    0x60,
    0x61,
    0x62,
    0x63,
    0x64,
    0x65,
    0x66,
    0x67,
    0x68,
    0x69,
    0x6a,
    0x6b,
    0x6c,
    0x6d,
    0x6e,
    0x6f,
    0x70,
    0x71,
    0x72,
    0x73,
    0x74,
    0x75,
    0x76,
    0x77,
    0x78,
    0x79,
    0x7a,
    0x7b,
    0x7c,
    0x7d,
    0x7e,
    0x7f,
    0x20ac,
    0xffff,
    0x201a,
    0x192,
    0x201e,
    0x2026,
    0x2020,
    0x2021,
    0x2c6,
    0x2030,
    0x160,
    0x2039,
    0x152,
    0xffff,
    0x17d,
    0xffff,
    0xffff,
    0x2018,
    0x2019,
    0x201c,
    0x201d,
    0x2022,
    0x2013,
    0x2014,
    0x2dc,
    0x2122,
    0x161,
    0x203a,
    0x153,
    0xffff,
    0x17e,
    0x178,
    0xa0,
    0xa1,
    0xa2,
    0xa3,
    0xa4,
    0xa5,
    0xa6,
    0xa7,
    0xa8,
    0xa9,
    0xaa,
    0xab,
    0xac,
    0xad,
    0xae,
    0xaf,
    0xb0,
    0xb1,
    0xb2,
    0xb3,
    0xb4,
    0xb5,
    0xb6,
    0xb7,
    0xb8,
    0xb9,
    0xba,
    0xbb,
    0xbc,
    0xbd,
    0xbe,
    0xbf,
    0xc0,
    0xc1,
    0xc2,
    0xc3,
    0xc4,
    0xc5,
    0xc6,
    0xc7,
    0xc8,
    0xc9,
    0xca,
    0xcb,
    0xcc,
    0xcd,
    0xce,
    0xcf,
    0xd0,
    0xd1,
    0xd2,
    0xd3,
    0xd4,
    0xd5,
    0xd6,
    0xd7,
    0xd8,
    0xd9,
    0xda,
    0xdb,
    0xdc,
    0xdd,
    0xde,
    0xdf,
    0xe0,
    0xe1,
    0xe2,
    0xe3,
    0xe4,
    0xe5,
    0xe6,
    0xe7,
    0xe8,
    0xe9,
    0xea,
    0xeb,
    0xec,
    0xed,
    0xee,
    0xef,
    0xf0,
    0xf1,
    0xf2,
    0xf3,
    0xf4,
    0xf5,
    0xf6,
    0xf7,
    0xf8,
    0xf9,
    0xfa,
    0xfb,
    0xfc,
    0xfd,
    0xfe,
    0xff
}; D2_GLOBAL_GUARD(gszUnicodeLatin1_1, "gszUnicodeLatin1_1");

uint16_t gszUnicodeLatin2_1[256] = {
    0x0,
    0x1,
    0x2,
    0x3,
    0x4,
    0x5,
    0x6,
    0x7,
    0x8,
    0x9,
    0xa,
    0xb,
    0xc,
    0xd,
    0xe,
    0xf,
    0x10,
    0x11,
    0x12,
    0x13,
    0x14,
    0x15,
    0x16,
    0x17,
    0x18,
    0x19,
    0x1a,
    0x1b,
    0x1c,
    0x1d,
    0x1e,
    0x1f,
    0x20,
    0x21,
    0x22,
    0x23,
    0x24,
    0x25,
    0x26,
    0x27,
    0x28,
    0x29,
    0x2a,
    0x2b,
    0x2c,
    0x2d,
    0x2e,
    0x2f,
    0x30,
    0x31,
    0x32,
    0x33,
    0x34,
    0x35,
    0x36,
    0x37,
    0x38,
    0x39,
    0x3a,
    0x3b,
    0x3c,
    0x3d,
    0x3e,
    0x3f,
    0x40,
    0x41,
    0x42,
    0x43,
    0x44,
    0x45,
    0x46,
    0x47,
    0x48,
    0x49,
    0x4a,
    0x4b,
    0x4c,
    0x4d,
    0x4e,
    0x4f,
    0x50,
    0x51,
    0x52,
    0x53,
    0x54,
    0x55,
    0x56,
    0x57,
    0x58,
    0x59,
    0x5a,
    0x5b,
    0x5c,
    0x5d,
    0x5e,
    0x5f,
    0x60,
    0x61,
    0x62,
    0x63,
    0x64,
    0x65,
    0x66,
    0x67,
    0x68,
    0x69,
    0x6a,
    0x6b,
    0x6c,
    0x6d,
    0x6e,
    0x6f,
    0x70,
    0x71,
    0x72,
    0x73,
    0x74,
    0x75,
    0x76,
    0x77,
    0x78,
    0x79,
    0x7a,
    0x7b,
    0x7c,
    0x7d,
    0x7e,
    0x7f,
    0xc4,
    0xc5,
    0xc7,
    0xc9,
    0xd1,
    0xd6,
    0xdc,
    0xe1,
    0xe0,
    0xe2,
    0xe4,
    0xe3,
    0xe5,
    0xe7,
    0xe9,
    0xe8,
    0xea,
    0xeb,
    0xed,
    0xec,
    0xee,
    0xef,
    0xf1,
    0xf3,
    0xf2,
    0xf4,
    0xf6,
    0xf5,
    0xfa,
    0xf9,
    0xfb,
    0xfc,
    0x2020,
    0xb0,
    0xa2,
    0xa3,
    0xa7,
    0x2022,
    0xb6,
    0xdf,
    0xae,
    0xa9,
    0x2122,
    0xb4,
    0xa8,
    0x2260,
    0xc6,
    0xd8,
    0x221e,
    0xb1,
    0x2264,
    0x2265,
    0xa5,
    0xb5,
    0x2202,
    0x2211,
    0x220f,
    0x3c0,
    0x222b,
    0xaa,
    0xba,
    0x2126,
    0xe6,
    0xf8,
    0xbf,
    0xa1,
    0xac,
    0x221a,
    0x192,
    0x2248,
    0x2206,
    0xab,
    0xbb,
    0x2026,
    0xa0,
    0xc0,
    0xc3,
    0xd5,
    0x152,
    0x153,
    0x2013,
    0x2014,
    0x201c,
    0x201d,
    0x2018,
    0x2019,
    0xf7,
    0x25ca,
    0xff,
    0x178,
    0x2044,
    0xa4,
    0x2039,
    0x203a,
    0xfb01,
    0xfb02,
    0x2021,
    0xb7,
    0x201a,
    0x201e,
    0x2030,
    0xc2,
    0xca,
    0xc1,
    0xcb,
    0xc8,
    0xcd,
    0xce,
    0xcf,
    0xcc,
    0xd3,
    0xd4,
    0xffff,
    0xd2,
    0xda,
    0xdb,
    0xd9,
    0x131,
    0x2c6,
    0x2dc,
    0xaf,
    0x2d8,
    0x2d9,
    0x2da,
    0xb8,
    0x2dd,
    0x2db,
    0x2c7
}; D2_GLOBAL_GUARD(gszUnicodeLatin2_1, "gszUnicodeLatin2_1");

uint16_t gszUnicodeKorean_1[256] = {
    0x0,
    0x1,
    0x2,
    0x3,
    0x4,
    0x5,
    0x6,
    0x7,
    0x8,
    0x9,
    0xa,
    0xb,
    0xc,
    0xd,
    0xe,
    0xf,
    0x10,
    0x11,
    0x12,
    0x13,
    0x14,
    0x15,
    0x16,
    0x17,
    0x18,
    0x19,
    0x1a,
    0x1b,
    0x1c,
    0x1d,
    0x1e,
    0x1f,
    0x20,
    0x21,
    0x22,
    0x23,
    0x24,
    0x25,
    0x26,
    0x27,
    0x28,
    0x29,
    0x2a,
    0x2b,
    0x2c,
    0x2d,
    0x2e,
    0x2f,
    0x30,
    0x31,
    0x32,
    0x33,
    0x34,
    0x35,
    0x36,
    0x37,
    0x38,
    0x39,
    0x3a,
    0x3b,
    0x3c,
    0x3d,
    0x3e,
    0x3f,
    0x40,
    0x41,
    0x42,
    0x43,
    0x44,
    0x45,
    0x46,
    0x47,
    0x48,
    0x49,
    0x4a,
    0x4b,
    0x4c,
    0x4d,
    0x4e,
    0x4f,
    0x50,
    0x51,
    0x52,
    0x53,
    0x54,
    0x55,
    0x56,
    0x57,
    0x58,
    0x59,
    0x5a,
    0x5b,
    0x5c,
    0x5d,
    0x5e,
    0x5f,
    0x60,
    0x61,
    0x62,
    0x63,
    0x64,
    0x65,
    0x66,
    0x67,
    0x68,
    0x69,
    0x6a,
    0x6b,
    0x6c,
    0x6d,
    0x6e,
    0x6f,
    0x70,
    0x71,
    0x72,
    0x73,
    0x74,
    0x75,
    0x76,
    0x77,
    0x78,
    0x79,
    0x7a,
    0x7b,
    0x7c,
    0x7d,
    0x7e,
    0x7f,
    0xc7,
    0xfc,
    0xe9,
    0xe2,
    0xe4,
    0xe0,
    0xe5,
    0xe7,
    0xea,
    0xeb,
    0xe8,
    0xef,
    0xee,
    0xec,
    0xc4,
    0xc5,
    0xc9,
    0xe6,
    0xc6,
    0xf4,
    0xf6,
    0xf2,
    0xfb,
    0xf9,
    0xff,
    0xd6,
    0xdc,
    0xa2,
    0xa3,
    0xa5,
    0x20a7,
    0x192,
    0xe1,
    0xed,
    0xf3,
    0xfa,
    0xf1,
    0xd1,
    0xaa,
    0xba,
    0xbf,
    0x2310,
    0xac,
    0xbd,
    0xbc,
    0xa1,
    0xab,
    0xbb,
    0x2591,
    0x2592,
    0x2593,
    0x2502,
    0x2524,
    0x2561,
    0x2562,
    0x2556,
    0x2555,
    0x2563,
    0x2551,
    0x2557,
    0x255d,
    0x255c,
    0x255b,
    0x2510,
    0x2514,
    0x2534,
    0x252c,
    0x251c,
    0x2500,
    0x253c,
    0x255e,
    0x255f,
    0x255a,
    0x2554,
    0x2569,
    0x2566,
    0x2560,
    0x2550,
    0x256c,
    0x2567,
    0x2568,
    0x2564,
    0x2565,
    0x2559,
    0x2558,
    0x2552,
    0x2553,
    0x256b,
    0x256a,
    0x2518,
    0x250c,
    0x2588,
    0x2584,
    0x258c,
    0x2590,
    0x2580,
    0x3b1,
    0xdf,
    0x393,
    0x3c0,
    0x3a3,
    0x3c3,
    0xb5,
    0x3c4,
    0x3a6,
    0x398,
    0x3a9,
    0x3b4,
    0x221e,
    0x3c6,
    0x3b5,
    0x2229,
    0x2261,
    0xb1,
    0x2265,
    0x2264,
    0x2320,
    0x2321,
    0xf7,
    0x2248,
    0xb0,
    0x2219,
    0xb7,
    0x221a,
    0x207f,
    0xb2,
    0x25a0,
    0xa0
}; D2_GLOBAL_GUARD(gszUnicodeKorean_1, "gszUnicodeKorean_1");

D2StormListHead gMpqArchiveList = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gMpqArchiveList.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gMpqArchiveList, "gMpqArchiveList");

D2StormListHead gSFileAsyncReqQueue = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gSFileAsyncReqQueue.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gSFileAsyncReqQueue, "gSFileAsyncReqQueue");

D2StormListHead gSFileOpenFileList = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gSFileOpenFileList.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gSFileOpenFileList, "gSFileOpenFileList");

D2StormListHead gSFileAudioStreamList = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gSFileAudioStreamList.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gSFileAudioStreamList, "gSFileAudioStreamList");

D2StormListHead gSFileStreamRecordList = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gSFileStreamRecordList.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gSFileStreamRecordList, "gSFileStreamRecordList");

char sRTTI_ARCHIVEREC_AllocTag[32] = {
    '.',
    '?',
    'A',
    'U',
    'A',
    'R',
    'C',
    'H',
    'I',
    'V',
    'E',
    'R',
    'E',
    'C',
    '@',
    'S',
    'F',
    'i',
    'l',
    'e',
    '@',
    'S',
    't',
    'o',
    'r',
    'm',
    '@',
    '@',
    '\x00',
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(sRTTI_ARCHIVEREC_AllocTag, "sRTTI_ARCHIVEREC_AllocTag");

char sRTTI_EVENTREC_AllocTag[16] = {
    '.',
    '?',
    'A',
    'U',
    'E',
    'V',
    'E',
    'N',
    'T',
    'R',
    'E',
    'C',
    '@',
    '@',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(sRTTI_EVENTREC_AllocTag, "sRTTI_EVENTREC_AllocTag");

char sRTTI_REQUEST_AllocTag[16] = {
    '.',
    '?',
    'A',
    'U',
    'R',
    'E',
    'Q',
    'U',
    'E',
    'S',
    'T',
    '@',
    '@',
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(sRTTI_REQUEST_AllocTag, "sRTTI_REQUEST_AllocTag");

char sRTTI_AUDIOSTREAM_AllocTag[20] = {
    '.',
    '?',
    'A',
    'U',
    'A',
    'U',
    'D',
    'I',
    'O',
    'S',
    'T',
    'R',
    'E',
    'A',
    'M',
    '@',
    '@',
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(sRTTI_AUDIOSTREAM_AllocTag, "sRTTI_AUDIOSTREAM_AllocTag");

D2StormListHead gnStormErrorCallbackCount = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gnStormErrorCallbackCount.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gnStormErrorCallbackCount, "gnStormErrorCallbackCount");

char gszSSystemListAllocTag[14] = {
    '.',
    '?',
    'A',
    'U',
    'H',
    'A',
    'N',
    'D',
    'L',
    'E',
    'R',
    '@',
    '@',
    '\x00'
}; D2_GLOBAL_GUARD(gszSSystemListAllocTag, "gszSSystemListAllocTag");

char s___AUWNDREC___00708850[16] = {
    '.',
    '?',
    'A',
    'U',
    'W',
    'N',
    'D',
    'R',
    'E',
    'C',
    '@',
    '@',
    '\x00',
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(s___AUWNDREC___00708850, "s___AUWNDREC___00708850");

D2StormListHead gpWndTrackingListHead = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gpWndTrackingListHead.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gpWndTrackingListHead, "gpWndTrackingListHead");

char s__I_00708874[4] = { '.', 'I', '\x00', '\x00' }; D2_GLOBAL_GUARD(s__I_00708874, "s__I_00708874");

char s__E_00708880[4] = { '.', 'E', '\x00', '\x00' }; D2_GLOBAL_GUARD(s__E_00708880, "s__E_00708880");

char s__D_0070888c[4] = { '.', 'D', '\x00', '\x00' }; D2_GLOBAL_GUARD(s__D_0070888c, "s__D_0070888c");

char s___AUBREAKCMD___007088d8[16] = {
    '.',
    '?',
    'A',
    'U',
    'B',
    'R',
    'E',
    'A',
    'K',
    'C',
    'M',
    'D',
    '@',
    '@',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(s___AUBREAKCMD___007088d8, "s___AUBREAKCMD___007088d8");

D2StormListHead gpSevtBreakCmdListHead = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gpSevtBreakCmdListHead.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gpSevtBreakCmdListHead, "gpSevtBreakCmdListHead");

char s___AUTMESSAGESOURCE___0072f9fc[24] = {
    '.',
    '?',
    'A',
    'U',
    'T',
    'M',
    'E',
    'S',
    'S',
    'A',
    'G',
    'E',
    'S',
    'O',
    'U',
    'R',
    'C',
    'E',
    '@',
    '@',
    '\x00',
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(s___AUTMESSAGESOURCE___0072f9fc, "s___AUTMESSAGESOURCE___0072f9fc");

uint8_t gaCRT_PKWareLitLenBitsTableC[256] = {
    0xb,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0x8,
    0x7,
    0xc,
    0xc,
    0x7,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xd,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0x4,
    0xa,
    0x8,
    0xc,
    0xa,
    0xc,
    0xa,
    0x8,
    0x7,
    0x7,
    0x8,
    0x9,
    0x7,
    0x6,
    0x7,
    0x8,
    0x7,
    0x6,
    0x7,
    0x7,
    0x7,
    0x7,
    0x8,
    0x7,
    0x7,
    0x8,
    0x8,
    0xc,
    0xb,
    0x7,
    0x9,
    0xb,
    0xc,
    0x6,
    0x7,
    0x6,
    0x6,
    0x5,
    0x7,
    0x8,
    0x8,
    0x6,
    0xb,
    0x9,
    0x6,
    0x7,
    0x6,
    0x6,
    0x7,
    0xb,
    0x6,
    0x6,
    0x6,
    0x7,
    0x9,
    0x8,
    0x9,
    0x9,
    0xb,
    0x8,
    0xb,
    0x9,
    0xc,
    0x8,
    0xc,
    0x5,
    0x6,
    0x6,
    0x6,
    0x5,
    0x6,
    0x6,
    0x6,
    0x5,
    0xb,
    0x7,
    0x5,
    0x6,
    0x5,
    0x5,
    0x6,
    0xa,
    0x5,
    0x5,
    0x5,
    0x5,
    0x8,
    0x7,
    0x8,
    0x8,
    0xa,
    0xb,
    0xb,
    0xc,
    0xc,
    0xc,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xc,
    0xd,
    0xc,
    0xd,
    0xd,
    0xd,
    0xc,
    0xd,
    0xd,
    0xd,
    0xc,
    0xd,
    0xd,
    0xd,
    0xd,
    0xc,
    0xd,
    0xd,
    0xd,
    0xc,
    0xc,
    0xc,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd,
    0xd
}; D2_GLOBAL_GUARD(gaCRT_PKWareLitLenBitsTableC, "gaCRT_PKWareLitLenBitsTableC");

char s___AUCMDDEF___0074c47c[16] = {
    '.',
    '?',
    'A',
    'U',
    'C',
    'M',
    'D',
    'D',
    'E',
    'F',
    '@',
    '@',
    '\x00',
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(s___AUCMDDEF___0074c47c, "s___AUCMDDEF___0074c47c");

D2StormListHead gStormCmdListHeadBackslash = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gStormCmdListHeadBackslash.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gStormCmdListHeadBackslash, "gStormCmdListHeadBackslash");

D2StormListHead gStormCmdListHeadSlash = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gStormCmdListHeadSlash.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gStormCmdListHeadSlash, "gStormCmdListHeadSlash");

void * g_SMemHeapBuckets[256] = {}; D2_GLOBAL_GUARD(g_SMemHeapBuckets, "g_SMemHeapBuckets");

CRITICAL_SECTION DebugMemoryCriticalSection[256] = {}; D2_GLOBAL_GUARD(DebugMemoryCriticalSection, "DebugMemoryCriticalSection");

int g_SMemHeapCleanupFlags[256] = {}; D2_GLOBAL_GUARD(g_SMemHeapCleanupFlags, "g_SMemHeapCleanupFlags");

double gdPow10Table[200] = {}; D2_GLOBAL_GUARD(gdPow10Table, "gdPow10Table");

char szSFileBasePath[260] = {}; D2_GLOBAL_GUARD(szSFileBasePath, "szSFileBasePath");

_RTL_CRITICAL_SECTION lpCriticalSection_00779068 = {}; D2_GLOBAL_GUARD(lpCriticalSection_00779068, "lpCriticalSection_00779068");

_RTL_CRITICAL_SECTION lpCriticalSection_00779080 = {}; D2_GLOBAL_GUARD(lpCriticalSection_00779080, "lpCriticalSection_00779080");

SErrAppFatalContextStrc gpSSystemThreadContextCallback = {}; D2_GLOBAL_GUARD(gpSSystemThreadContextCallback, "gpSSystemThreadContextCallback");

_RTL_CRITICAL_SECTION lpCriticalSection_007790cc = {}; D2_GLOBAL_GUARD(lpCriticalSection_007790cc, "lpCriticalSection_007790cc");

_RTL_CRITICAL_SECTION lpCriticalSection_00779214 = {}; D2_GLOBAL_GUARD(lpCriticalSection_00779214, "lpCriticalSection_00779214");

CRITICAL_SECTION gaSLogCritSect[4] = {}; D2_GLOBAL_GUARD(gaSLogCritSect, "gaSLogCritSect");

D2SLogStrc * gaSLogHead[4] = {}; D2_GLOBAL_GUARD(gaSLogHead, "gaSLogHead");

_RTL_CRITICAL_SECTION lpCriticalSection_0077930c = {}; D2_GLOBAL_GUARD(lpCriticalSection_0077930c, "lpCriticalSection_0077930c");

// =============================================================================
// Initialized scalars
// =============================================================================

uint32_t gnSFileDataChunkSize = 0x20000; D2_GLOBAL_GUARD(gnSFileDataChunkSize, "gnSFileDataChunkSize");
uint32_t gnSFileReadErrorMode = 0x2; D2_GLOBAL_GUARD(gnSFileReadErrorMode, "gnSFileReadErrorMode");
uint32_t gbSFileSeekOptimization = 0x1; D2_GLOBAL_GUARD(gbSFileSeekOptimization, "gbSFileSeekOptimization");
uint32_t g_SMemPageSize = 0; D2_GLOBAL_GUARD(g_SMemPageSize, "g_SMemPageSize");
uint32_t ProtectMemory = 0; D2_GLOBAL_GUARD(ProtectMemory, "ProtectMemory");
uint32_t g_SMemUnknownFlag = 0; D2_GLOBAL_GUARD(g_SMemUnknownFlag, "g_SMemUnknownFlag");
uint32_t DebugMemory = 0; D2_GLOBAL_GUARD(DebugMemory, "DebugMemory");
void * g_SMemPendingCleanupHeap = 0; D2_GLOBAL_GUARD(g_SMemPendingCleanupHeap, "g_SMemPendingCleanupHeap");
uint32_t g_SMemDebugFillEnabled = 0; D2_GLOBAL_GUARD(g_SMemDebugFillEnabled, "g_SMemDebugFillEnabled");
uint32_t g_SMemInitialized = 0; D2_GLOBAL_GUARD(g_SMemInitialized, "g_SMemInitialized");
unsigned int guStringAlignmentTemp = {}; D2_GLOBAL_GUARD(guStringAlignmentTemp, "guStringAlignmentTemp");
uint32_t gbSStrConversionTablesInitialized = 0; D2_GLOBAL_GUARD(gbSStrConversionTablesInitialized, "gbSStrConversionTablesInitialized");
IDirectSound * g_pDirectSound = 0; D2_GLOBAL_GUARD(g_pDirectSound, "g_pDirectSound");
uint32_t gbAsyncIOWorkerShutdown = 0; D2_GLOBAL_GUARD(gbAsyncIOWorkerShutdown, "gbAsyncIOWorkerShutdown");
D2StormAsyncOpStrc * gpCurrentAsyncIORequest = 0; D2_GLOBAL_GUARD(gpCurrentAsyncIORequest, "gpCurrentAsyncIORequest");
pfnSFileReadError pfnSFileReadErrorHandler = (pfnSFileReadError)0; D2_GLOBAL_GUARD(pfnSFileReadErrorHandler, "pfnSFileReadErrorHandler");
HANDLE ghAsyncIOThread = 0; D2_GLOBAL_GUARD(ghAsyncIOThread, "ghAsyncIOThread");
uint32_t gnSFileWaveStreamBufferSize = 0; D2_GLOBAL_GUARD(gnSFileWaveStreamBufferSize, "gnSFileWaveStreamBufferSize");
void * gpSFileExplodeWorkBuffer = 0; D2_GLOBAL_GUARD(gpSFileExplodeWorkBuffer, "gpSFileExplodeWorkBuffer");
uint32_t gbSFileAsyncIOShutdown = 0; D2_GLOBAL_GUARD(gbSFileAsyncIOShutdown, "gbSFileAsyncIOShutdown");
void * g_pStormMpqCryptTable = 0; D2_GLOBAL_GUARD(g_pStormMpqCryptTable, "g_pStormMpqCryptTable");
uint32_t gnSFileAsyncBudgetBytesPerSec = 0; D2_GLOBAL_GUARD(gnSFileAsyncBudgetBytesPerSec, "gnSFileAsyncBudgetBytesPerSec");
void * ghAsyncIOCompletionPort = 0; D2_GLOBAL_GUARD(ghAsyncIOCompletionPort, "ghAsyncIOCompletionPort");
HANDLE ghAsyncIOEvent = 0; D2_GLOBAL_GUARD(ghAsyncIOEvent, "ghAsyncIOEvent");
uint32_t gnSFileDirectAccessFlags = 0; D2_GLOBAL_GUARD(gnSFileDirectAccessFlags, "gnSFileDirectAccessFlags");
uint16_t gnSFileSearchLocale = 0; D2_GLOBAL_GUARD(gnSFileSearchLocale, "gnSFileSearchLocale");
uint8_t gnSFileSearchHashOffset = 0; D2_GLOBAL_GUARD(gnSFileSearchHashOffset, "gnSFileSearchHashOffset");
pfnSFileOpenCallback fpSFileOpenCallback = (pfnSFileOpenCallback)0; D2_GLOBAL_GUARD(fpSFileOpenCallback, "fpSFileOpenCallback");
void * gpSFileOpenCallbackParam = 0; D2_GLOBAL_GUARD(gpSFileOpenCallbackParam, "gpSFileOpenCallbackParam");
uint32_t gnSSystemOption3 = 0; D2_GLOBAL_GUARD(gnSSystemOption3, "gnSSystemOption3");
uint32_t gnSSystemOption1 = 0; D2_GLOBAL_GUARD(gnSSystemOption1, "gnSSystemOption1");
uint32_t gnSSystemWaveBufferSize = 0; D2_GLOBAL_GUARD(gnSSystemWaveBufferSize, "gnSSystemWaveBufferSize");
uint32_t gnSSystemAsyncReadSize = 0; D2_GLOBAL_GUARD(gnSSystemAsyncReadSize, "gnSSystemAsyncReadSize");
uint32_t gbCompressionDebugOutput = 0; D2_GLOBAL_GUARD(gbCompressionDebugOutput, "gbCompressionDebugOutput");
void * gpErrorModuleList = 0; D2_GLOBAL_GUARD(gpErrorModuleList, "gpErrorModuleList");
uint32_t gbFormatErrorModulesInitialized = 0; D2_GLOBAL_GUARD(gbFormatErrorModulesInitialized, "gbFormatErrorModulesInitialized");
uint32_t gbSSystemShutdownInProgress = 0; D2_GLOBAL_GUARD(gbSSystemShutdownInProgress, "gbSSystemShutdownInProgress");
uint32_t dwLastErrorCode = 0; D2_GLOBAL_GUARD(dwLastErrorCode, "dwLastErrorCode");
pfnBlizzardError pfnBlizzardErrorCallback = (pfnBlizzardError)0; D2_GLOBAL_GUARD(pfnBlizzardErrorCallback, "pfnBlizzardErrorCallback");
void * gpScompTempBuffer = 0; D2_GLOBAL_GUARD(gpScompTempBuffer, "gpScompTempBuffer");
uint32_t gnScompTempBufferOwnerThreadId = 0; D2_GLOBAL_GUARD(gnScompTempBufferOwnerThreadId, "gnScompTempBufferOwnerThreadId");
uint32_t gpActiveWindow = 0; D2_GLOBAL_GUARD(gpActiveWindow, "gpActiveWindow");
uint32_t gnSevtSnapshotActive = 0; D2_GLOBAL_GUARD(gnSevtSnapshotActive, "gnSevtSnapshotActive");
void * gpEventHashBuckets = 0; D2_GLOBAL_GUARD(gpEventHashBuckets, "gpEventHashBuckets");
void * gaEventRegistryHead = 0; D2_GLOBAL_GUARD(gaEventRegistryHead, "gaEventRegistryHead");
uint32_t gnSevtRegisteredEventCount = 0; D2_GLOBAL_GUARD(gnSevtRegisteredEventCount, "gnSevtRegisteredEventCount");
uint32_t gbScmdHasPriorityCommand = 0; D2_GLOBAL_GUARD(gbScmdHasPriorityCommand, "gbScmdHasPriorityCommand");

namespace D2Common::Drlg::Maze {

} // namespace D2Common::Drlg::Maze

namespace Fog::Src::Excel {

} // namespace Fog::Src::Excel

namespace D2Client::Warden::NWardenClient {

} // namespace D2Client::Warden::NWardenClient

namespace D2Client::Warden {

} // namespace D2Client::Warden

namespace D2Client::UI {

} // namespace D2Client::UI

namespace D2Client::UI::Button::ButtonImplementation {

} // namespace D2Client::UI::Button::ButtonImplementation

namespace D2Client::UI::Button::ButtonWrapper {

} // namespace D2Client::UI::Button::ButtonWrapper

namespace D2Client::UI::Editbox::EditBoxImplementation {

} // namespace D2Client::UI::Editbox::EditBoxImplementation

namespace D2Client::UI::Editbox::EditBoxWrapper {

} // namespace D2Client::UI::Editbox::EditBoxWrapper

namespace D2Client::UI::Spinner::SpinnerImplementation {

} // namespace D2Client::UI::Spinner::SpinnerImplementation

namespace D2Client::UI::Spinner::SpinnerWrapper {

} // namespace D2Client::UI::Spinner::SpinnerWrapper

namespace D2Client::UNIT::Object {

} // namespace D2Client::UNIT::Object

namespace D2Client::UI::ListBox::ListBoxWrapper {

} // namespace D2Client::UI::ListBox::ListBoxWrapper

namespace D2Client::UI::ListBox::ListBoxImplementation {

} // namespace D2Client::UI::ListBox::ListBoxImplementation

namespace D2Win::_FONT {

} // namespace D2Win::_FONT

namespace D2Game::GAME::Event::Dispatch {

} // namespace D2Game::GAME::Event::Dispatch

namespace D2Game::GAME::Event::Dispatch::Player {

} // namespace D2Game::GAME::Event::Dispatch::Player

namespace D2Game::GAME::Event::Dispatch::Object {

} // namespace D2Game::GAME::Event::Dispatch::Object

namespace D2Game::GAME::Event::Dispatch::Monster {

} // namespace D2Game::GAME::Event::Dispatch::Monster

namespace D2Common::Chat::Ignorelist::ChatIgnoreList {

} // namespace D2Common::Chat::Ignorelist::ChatIgnoreList

namespace D2Common::Chat::Ignorelist::IgnoreList {

} // namespace D2Common::Chat::Ignorelist::IgnoreList

namespace D2Common::Drlg::Outdoors {

} // namespace D2Common::Drlg::Outdoors

namespace Define_the_symbol__ATL_MIXED {

} // namespace Define_the_symbol__ATL_MIXED

namespace Fog::_3rdparty::crashy {

} // namespace Fog::_3rdparty::crashy

namespace Storm::Source::SFile {

} // namespace Storm::Source::SFile

namespace Class::FILEREC {

} // namespace Class::FILEREC

namespace Class::EVENTREC {

} // namespace Class::EVENTREC

namespace Class::REQUEST {

} // namespace Class::REQUEST

namespace Class::AUDIOSTREAM {

} // namespace Class::AUDIOSTREAM

namespace Class::HANDLER {

} // namespace Class::HANDLER

namespace D2Client::CharScreen {

} // namespace D2Client::CharScreen

namespace D2Client::UI::Automap {

} // namespace D2Client::UI::Automap

namespace D2Game::GAME::Rendering {

} // namespace D2Game::GAME::Rendering

namespace D2Client::UI::Help {

} // namespace D2Client::UI::Help

namespace D2Client::UI::npcmenu {

} // namespace D2Client::UI::npcmenu

namespace D2Client::NPC {

} // namespace D2Client::NPC

namespace D2Client::UNIT::UnitSnd {

} // namespace D2Client::UNIT::UnitSnd

namespace D2Client::UI::ui {

} // namespace D2Client::UI::ui

namespace D2Game::Quests::QUESTS::Quest40 {

} // namespace D2Game::Quests::QUESTS::Quest40

namespace D2Game::Quests::QUESTS::A5Q1 {

} // namespace D2Game::Quests::QUESTS::A5Q1

namespace D2Game::Quests::QUESTS::A5Q2 {

} // namespace D2Game::Quests::QUESTS::A5Q2

namespace D2Game::Quests::QUESTS::A5Q3 {

} // namespace D2Game::Quests::QUESTS::A5Q3

namespace D2Game::Quests::QUESTS::A5Q4 {

} // namespace D2Game::Quests::QUESTS::A5Q4

namespace D2Game::Quests::QUESTS::A5Q5 {

} // namespace D2Game::Quests::QUESTS::A5Q5

namespace D2Game::Quests::QUESTS::A5Q6 {

} // namespace D2Game::Quests::QUESTS::A5Q6

namespace D2Game::Quests::QUESTS::Quest37 {

} // namespace D2Game::Quests::QUESTS::Quest37

namespace D2Game::Quests::QUESTS::A1Q0 {

} // namespace D2Game::Quests::QUESTS::A1Q0

namespace D2Game::Quests::QUESTS::A1Q1 {

} // namespace D2Game::Quests::QUESTS::A1Q1

namespace D2Game::Quests::QUESTS::A1Q2 {

} // namespace D2Game::Quests::QUESTS::A1Q2

namespace D2Game::Quests::QUESTS::A1Q3 {

} // namespace D2Game::Quests::QUESTS::A1Q3

namespace D2Game::Quests::QUESTS::A1Q4 {

} // namespace D2Game::Quests::QUESTS::A1Q4

namespace D2Game::Quests::QUESTS::A1Q5 {

} // namespace D2Game::Quests::QUESTS::A1Q5

namespace D2Game::Quests::QUESTS::A1Q6 {

} // namespace D2Game::Quests::QUESTS::A1Q6

namespace D2Game::Quests::QUESTS::A1Q7 {

} // namespace D2Game::Quests::QUESTS::A1Q7

namespace D2Game::Quests::QUESTS::Quest38 {

} // namespace D2Game::Quests::QUESTS::Quest38

namespace D2Game::Quests::QUESTS::A2Q0 {

} // namespace D2Game::Quests::QUESTS::A2Q0

namespace D2Game::Quests::QUESTS::A2Q1 {

} // namespace D2Game::Quests::QUESTS::A2Q1

namespace D2Game::Quests::QUESTS::A2Q2 {

} // namespace D2Game::Quests::QUESTS::A2Q2

namespace D2Game::Quests::QUESTS::A2Q3 {

} // namespace D2Game::Quests::QUESTS::A2Q3

namespace D2Game::Quests::QUESTS::A2Q4 {

} // namespace D2Game::Quests::QUESTS::A2Q4

namespace D2Game::Quests::QUESTS::A2Q5 {

} // namespace D2Game::Quests::QUESTS::A2Q5

namespace D2Game::Quests::QUESTS::A2Q6 {

} // namespace D2Game::Quests::QUESTS::A2Q6

namespace D2Game::Quests::QUESTS::A2Q7 {

} // namespace D2Game::Quests::QUESTS::A2Q7

namespace D2Game::Quests::QUESTS::A2Q8 {

} // namespace D2Game::Quests::QUESTS::A2Q8

namespace D2Common::Monsters::MONSTERS::Umod {

} // namespace D2Common::Monsters::MONSTERS::Umod

namespace D2Game::Quests::QUESTS::A4Q0 {

} // namespace D2Game::Quests::QUESTS::A4Q0

namespace D2Game::Quests::QUESTS::A4Q1 {

} // namespace D2Game::Quests::QUESTS::A4Q1

namespace D2Game::Quests::QUESTS::A4Q2 {

} // namespace D2Game::Quests::QUESTS::A4Q2

namespace D2Game::Quests::QUESTS::A4Q3 {

} // namespace D2Game::Quests::QUESTS::A4Q3

namespace D2Game::Quests::QUESTS::A4Q4 {

} // namespace D2Game::Quests::QUESTS::A4Q4

namespace D2Game::Quests::QUESTS::Quest39 {

} // namespace D2Game::Quests::QUESTS::Quest39

namespace D2Game::Quests::QUESTS::A3Q0 {

} // namespace D2Game::Quests::QUESTS::A3Q0

namespace D2Game::Quests::QUESTS::A3Q1 {

} // namespace D2Game::Quests::QUESTS::A3Q1

namespace D2Game::Quests::QUESTS::A3Q2 {

} // namespace D2Game::Quests::QUESTS::A3Q2

namespace D2Game::Quests::QUESTS::A3Q3 {

} // namespace D2Game::Quests::QUESTS::A3Q3

namespace D2Game::Quests::QUESTS::A3Q4 {

} // namespace D2Game::Quests::QUESTS::A3Q4

namespace D2Game::Quests::QUESTS::A3Q5 {

} // namespace D2Game::Quests::QUESTS::A3Q5

namespace D2Game::Quests::QUESTS::A3Q6 {

} // namespace D2Game::Quests::QUESTS::A3Q6

namespace D2Client::Waypoint {

} // namespace D2Client::Waypoint

namespace D2Common::Drlg::DrlgGrid {

} // namespace D2Common::Drlg::DrlgGrid

namespace Fog::Memory {

} // namespace Fog::Memory

namespace Fog::Debug {

} // namespace Fog::Debug

namespace D2Client::MCP {

} // namespace D2Client::MCP

namespace D2Client::Bnet {

} // namespace D2Client::Bnet

namespace D2Game::Save {

} // namespace D2Game::Save

namespace D2Game::GameData {

} // namespace D2Game::GameData

namespace D2Client::GAME::Record {

} // namespace D2Client::GAME::Record

namespace D2Client::UI::Dialog {

} // namespace D2Client::UI::Dialog

namespace D2Client::_Selection {

} // namespace D2Client::_Selection

namespace D2Client::Cursor {

} // namespace D2Client::Cursor

namespace D2Client::Draw::Weather {

} // namespace D2Client::Draw::Weather

namespace D2Client::Draw::LightMap {

} // namespace D2Client::Draw::LightMap

namespace D2Client::GAME::View {

} // namespace D2Client::GAME::View

namespace D2Client::GAME::RosterPets {

} // namespace D2Client::GAME::RosterPets

namespace D2Client::GAME::Roster {

} // namespace D2Client::GAME::Roster

namespace D2Client::UI::Inv {

} // namespace D2Client::UI::Inv

namespace D2Client::UI::Hireables {

} // namespace D2Client::UI::Hireables

namespace D2Client::UI::Panel {

} // namespace D2Client::UI::Panel

namespace D2Client::UI::Party {

} // namespace D2Client::UI::Party

namespace D2Client::_Interaction {

} // namespace D2Client::_Interaction

namespace D2Client::Merc {

} // namespace D2Client::Merc

namespace D2Client::Mouse {

} // namespace D2Client::Mouse

namespace D2Client::Forms {

} // namespace D2Client::Forms

namespace D2Client::_CdKey {

} // namespace D2Client::_CdKey

namespace D2Direct3D::Renderer {

} // namespace D2Direct3D::Renderer
