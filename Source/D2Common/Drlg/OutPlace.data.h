/**
 * Data defined in D2Common/Drlg/OutPlace.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Common)
#error "D2Common header included by a module that may not use D2Common (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"
#include "Source/D2Common/DUNGEON/Dungeon.h"
#include "Source/D2Common/Drlg/D2DrlgLevelDataList.h"
#include "Source/D2Common/Drlg/Drlg.h"

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
