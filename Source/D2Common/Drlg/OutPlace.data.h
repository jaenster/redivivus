/**
 * Data defined in D2Common/Drlg/OutPlace.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "D2Common/DUNGEON/Dungeon.h"
#include "D2Common/Drlg/D2DrlgLevelDataList.h"
#include "D2Common/Drlg/Drlg.h"

// Forward declarations
struct D2DrlgLevelDataList;
struct D2DrlgLevelPlacementStrc;
typedef BOOL (__fastcall *fpLevelDataEntry)(D2DrlgLevelPlacementStrc * pLevelUnknown);

extern D2DrlgLevelDataList ACT1_D2DrlgLevelData1;
extern D2DrlgLevelDataList ACT1_D2DrlgLevelData2;
extern D2DrlgLevelDataList ACT2_D2DrlgLevelData1;
extern D2DrlgLevelDataList ACT2_D2DrlgLevelData2;
extern D2DrlgLevelDataList ACT5_D2DrlgLevelData3;
extern D2DrlgLevelDataList ACT4_D2DrlgLevelData1;
extern D2DrlgLevelDataList ACT4_D2DrlgLevelData2;
extern int gaRoadPresetDirectionLookup[9];
extern int gaOutPlaceAdjacentRoadIndex[81];
