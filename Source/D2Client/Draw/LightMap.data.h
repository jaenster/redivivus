/**
 * Data defined in D2Client/Draw/LightMap.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2LightMapCellStrc;

extern uint32_t gLightMapBuffer[4096];
extern uint32_t gnLightmapInterpolationState[257];
extern D2LightMapCellStrc gLightmapCellData[2304];
extern D2LightMapCellStrc gLightmapCellDataRed[2304];
extern D2LightMapCellStrc gLightmapCellDataGreen[2304];
extern D2LightMapCellStrc gLightmapCellDataBlue[2304];
