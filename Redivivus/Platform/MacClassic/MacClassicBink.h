// STUB HEADER - Redivivus-owned, not a RAD Game Tools header.
//
// The Mac "Bink Shared Library" calls the PowerPC D2Rave module makes (CFM imports of the
// classic 1.09d PEF), declared from their call sites so the module can be syntax-checked.
// Only the four leading BINK fields D2Rave reads are named. Not copied from bink.h.
//
// Classic Mac PowerPC only. See README.md beside this file.
#pragma once

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

#include <stdint.h>
#include "Redivivus/Platform/MacClassic/MacClassicToolbox.h"

struct BINK {
    /* 0x00 */ uint32_t Width;
    /* 0x04 */ uint32_t Height;
    /* 0x08 */ uint32_t Frames;
    /* 0x0c */ uint32_t FrameNum;
};
typedef BINK * HBINK;

typedef void * (*BINKSNDSYSOPEN)(uint32_t param);

extern "C" {
HBINK BinkOpen(int32_t fileRefNum, uint32_t flags);
void BinkClose(HBINK bink);
int32_t BinkDoFrame(HBINK bink);
void BinkNextFrame(HBINK bink);
int32_t BinkWait(HBINK bink);
int32_t BinkCopyToBuffer(HBINK bink, void * dest, int32_t destPitch, uint32_t destHeight, uint32_t destX,
                         uint32_t destY, uint32_t flags);
int32_t BinkSetSoundSystem(BINKSNDSYSOPEN open, uint32_t param);
uint32_t BinkGDSurfaceType(GDHandle device);
// The Sound Manager back end D2Rave hands to BinkSetSoundSystem (TOC slot 10375848).
void * BinkOpenSoundManager(uint32_t param);
}

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
