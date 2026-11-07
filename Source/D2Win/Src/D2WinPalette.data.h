/**
 * Data defined in D2Win/Src/D2WinPalette.cpp that other translation units name.
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

extern uint8_t gPaletteAct1[196608];
extern uint8_t gPaletteActNeutral[65536];
