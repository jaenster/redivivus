/**
 * Data defined in D2Win/Src/D2Comp.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

extern uint32_t gnStringTruncateConstant;
extern uint32_t gnPaletteLoadBufferSize;
extern uint8_t gaPaletteLoadData;
// 9 item palette-transform slots of 0x6900 bytes (D2FILE_ReadFile writes 0x1500 at +nSlot*0x6900) = 0x3B100 bytes.
// The guild-emblem primary colour table follows at 0087d810; D2COMP_LoadGuildEmblemIcon reads it as base-1 (0087d80f + n, n = 1..10).
extern uint8_t gaItemPaletteTransformData[241920];
