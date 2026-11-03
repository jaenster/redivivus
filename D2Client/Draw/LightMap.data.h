/**
 * Data defined in D2Client/Draw/LightMap.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "D2Client/Draw/LightMap.h"

// Forward declarations
struct D2LightMapCellStrc;

extern uint32_t gLightMapBuffer[4096];
extern uint32_t gnLightmapInterpolationState[257];
extern D2LightMapCellStrc gLightmapCellData[2304];
extern D2LightMapCellStrc gLightmapCellDataRed[2304];
extern D2LightMapCellStrc gLightmapCellDataGreen[2304];
extern D2LightMapCellStrc gLightmapCellDataBlue[2304];
