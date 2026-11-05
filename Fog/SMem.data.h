/**
 * Data defined in Fog/SMem.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "Fog/SMem.h"

// Forward declarations
struct D2SMemTablesStrc;
struct SMemHeapStatsStrc;

extern D2SMemTablesStrc g_SMemSizeClassTable;
extern SMemHeapStatsStrc gaSMemHeapStats[4096];
