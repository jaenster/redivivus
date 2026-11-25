// STUB HEADER - Redivivus-owned, not an Apple header.
//
// The DrawSprocket calls the PowerPC D2Rave module makes (weak imports from DrawSprocketLib in
// the classic 1.09d PEF), written from the public DrawSprocket documentation so the module can
// be syntax-checked without the Apple SDK. Nothing is copied from Apple's DrawSprocket.h.
// DSpContextAttributes is opaque here: D2Rave only copies a zeroed template into one and hands
// it to the shell's DSpFindContextEx, so its size (0x48 bytes, the template D2Rave copies) is
// all that matters.
//
// Classic Mac PowerPC only. See README.md beside this file.
#pragma once

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

#include <stdint.h>
#include "Redivivus/Platform/MacClassic/MacClassicToolbox.h"

typedef struct OpaqueDSpContextReference * DSpContextReference;

enum DSpContextState {
    kDSpContextState_Active = 0,
    kDSpContextState_Paused = 1,
    kDSpContextState_Inactive = 2,
};

struct DSpContextAttributes {
    uint32_t data[0x48 / 4];
};

// Weak CFM imports: the address is null when the library did not load.
extern "C" {
uint32_t DSpGetVersion(void) __attribute__((weak));
OSStatus DSpContext_Reserve(DSpContextReference inContext, DSpContextAttributes * inDesiredAttributes);
OSStatus DSpContext_Release(DSpContextReference inContext);
OSStatus DSpContext_SetState(DSpContextReference inContext, DSpContextState inState);
OSStatus DSpContext_FadeGamma(DSpContextReference inContext, int32_t inPercentOfOriginalIntensity,
                              RGBColor * inZeroIntensityColor);
OSStatus DSpContext_GetFrontBuffer(DSpContextReference inContext, CGrafPtr * outFrontBuffer);
OSStatus DSpContext_LocalToGlobal(DSpContextReference inContext, Point * ioPoint);
OSStatus DSpProcessEvent(EventRecord * inEvent, Boolean * outEventWasProcessed);
}

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
