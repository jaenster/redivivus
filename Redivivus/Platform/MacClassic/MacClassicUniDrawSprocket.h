// STUB HEADER - Redivivus-owned, not an Apple header.
//
// The DrawSprocket calls the PowerPC D2Uni module makes (weak CFM imports from DrawSprocketLib
// in the classic LoD 1.10b PEF), written from the public DrawSprocket documentation so the
// module can be syntax-checked without the Apple SDK. Nothing is copied from Apple's
// DrawSprocket.h. DSpContextAttributes is opaque: D2Uni only copies a zeroed 0x48-byte template
// into one and hands it to the shell's DSpFindContextEx and to DSpContext_Reserve.
//
// Classic Mac PowerPC only. See Source/D2Uni/README.md (the stub set).
#pragma once

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

#include <stdint.h>
#include "Redivivus/Platform/MacClassic/MacClassicUniToolbox.h"

typedef struct OpaqueDSpContextReference * DSpContextReference;

enum DSpContextState {
    kDSpContextState_Active = 0,
    kDSpContextState_Paused = 1,
    kDSpContextState_Inactive = 2,
};

// kDSpContextAlreadyReservedErr / "state already set": the -30448 (0x8910) DSpContext_SetState
// returns when the context is already active. D2Uni treats it as success.
enum { kDSpStateAlreadySetErr = -30448 };

struct DSpContextAttributes {
    uint32_t data[0x48 / 4];
};

extern "C" {
uint32_t DSpGetVersion(void);
OSStatus DSpContext_Reserve(DSpContextReference inContext, DSpContextAttributes * inDesiredAttributes);
OSStatus DSpContext_Release(DSpContextReference inContext);
OSStatus DSpContext_SetState(DSpContextReference inContext, DSpContextState inState);
OSStatus DSpContext_FadeGamma(DSpContextReference inContext, int32_t inPercentOfOriginalIntensity,
                              RGBColor * inZeroIntensityColor);
OSStatus DSpContext_GetFrontBuffer(DSpContextReference inContext, CGrafPtr * outFrontBuffer);
OSStatus DSpContext_SetCLUTEntries(DSpContextReference inContext, const ColorSpec * inEntries,
                                   uint16_t inStartingEntry, uint16_t inLastEntry);
OSStatus DSpProcessEvent(EventRecord * inEvent, Boolean * outEventWasProcessed);
}

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
