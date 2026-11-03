/**
 * Data defined in D2Client/GAME/View.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

extern uint32_t gaSpriteArcaneStarYOffset;
extern uint32_t gaSpriteArcaneStarXOffset;
extern int nScaleFactor;

namespace D2Client::GAME::View {
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
