/**
 * Data defined in D2Client/GAME/View.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2Client) && !defined(D2_LOADS_D2Client)
#error "internal D2Client header included from another module; include D2Client's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2SeedStrc;

extern uint32_t gaSpriteArcaneStarYOffset;
extern int nScaleFactor;

namespace D2Client::GAME::View {
extern D2SeedStrc gAncientsCloudSeed;
extern uint32_t SHAKE_StartTickcount;
extern uint32_t SHAKE_Arg3;
extern uint32_t SHAKE_Arg1;
extern uint32_t SHAKE_Arg2;
extern uint32_t SHAKE_OFFSET_Y;
extern int gnSoundShakeVolume;
extern uint32_t SHAKE_Amplitude;
extern uint32_t SHAKE_OFFSET_X;
extern uint8_t * * gpViewPalPointerTable32x8;
extern uint8_t * gpViewPalByteTable32x16;
extern uint8_t * gpViewPalByteTable16x32;
extern uint8_t * * gpViewPalPointerTable32x32;
extern uint8_t * gpViewPalByteTable32x40;
} // namespace D2Client::GAME::View
