/**
 * Data defined in D2Win/Src/D2Comp.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Win)
#error "D2Win header included by a module that may not use D2Win (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2Win) && !defined(D2_LOADS_D2Win)
#error "internal D2Win header included from another module; include D2Win's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

extern uint32_t gnStringTruncateConstant;
extern char gszDatExtension[5];
// 9 item palette-transform slots of 0x6900 bytes (D2FILE_ReadFile writes 0x1500 at +nSlot*0x6900) = 0x3B100 bytes.
// The guild-emblem primary colour table follows at 0087d810; D2COMP_LoadGuildEmblemIcon reads it as base-1 (0087d80f + n, n = 1..10).
extern uint8_t gaItemPaletteTransformData[241920];
