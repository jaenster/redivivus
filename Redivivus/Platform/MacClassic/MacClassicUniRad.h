// STUB HEADER - Redivivus-owned, not a RAD Game Tools header.
//
// The Bink and Smacker calls the PowerPC D2Uni module makes, declared from their call sites so
// the module can be syntax-checked. Bink is the CFM "Bink Shared Library"; Smacker is linked
// into the game fragment (its code sits beside the "RADMalFPLib.cpp" string, 1.10b 100a3990).
// Only the record fields D2Uni reads are named, plus the three Bink calls the shared
// d2_platform.h does not declare. Nothing is copied from bink.h or smack.h.
//
// Classic Mac PowerPC only. See Source/D2Uni/README.md (the stub set).
#pragma once

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

#include <stdint.h>
#include "Redivivus/Platform/MacClassic/MacClassicUniToolbox.h"

struct BINKRECT {
    int32_t Left;
    int32_t Top;
    int32_t Width;
    int32_t Height;
};

struct BINK {
    /* 0x00 */ uint32_t Width;
    /* 0x04 */ uint32_t Height;
    /* 0x08 */ uint32_t Frames;
    /* 0x0C */ uint32_t FrameNum;
    /* 0x10 */ uint32_t unknown10[9];
    /* 0x34 */ BINKRECT FrameRects[8];
    /* 0xB4 */ int32_t NumRects;
};
typedef BINK * HBINK;

// BinkOpen flags D2Uni passes (the same values as Windows D2DDraw BINK_PlayVideoFile).
enum {
    BINKFILEHANDLE = 0x00800000,
};

// BinkOpen, BinkClose, BinkDoFrame, BinkNextFrame, BinkWait, BinkCopyToBuffer and
// BinkSetSoundSystem are declared for every target by Source/D2Headers/d2_platform.h (void *
// handles; on the Mac it spells __stdcall away). The Mac adds these three.
extern "C" {
int32_t BinkGetRects(void * pBink, uint32_t flags);
uint32_t BinkGDSurfaceType(GDHandle device);
void * BinkOpenSoundManager(uint32_t param);
}

// Smacker, the fields the D2SmackerContextStrc users read (the published Smack layout).
struct Smack {
    /* 0x000 */ uint32_t Version;
    /* 0x004 */ uint32_t Width;
    /* 0x008 */ uint32_t Height;
    /* 0x00C */ uint32_t Frames;
    /* 0x010 */ uint32_t unknown10[22];
    /* 0x068 */ uint32_t NewPalette;
    /* 0x06C */ uint8_t Palette[772];
    /* 0x370 */ uint32_t PalType;
    /* 0x374 */ uint32_t FrameNum;
};

enum {
    SMACKTRACKS = 0x000FE000, // every audio track
};

// SmackOpen, SmackClose, SmackToBuffer, SmackWait, SmackDoFrame and SmackNextFrame are declared
// by d2_platform.h. The Mac copies linked into the game fragment are, in LoD 1.10b classic:
// SmackOpen 100a46fc (takes the movie's FSSpec in place of a path), SmackClose 100a4f58,
// SmackToBuffer 100a5958, SmackWait 100a5c54, SmackDoFrame 100a55e0, SmackNextFrame 100a5894.

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
