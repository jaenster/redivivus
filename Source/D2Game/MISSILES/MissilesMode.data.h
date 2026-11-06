/**
 * Data defined in D2Game/MISSILES/MissilesMode.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "D2Client/UNIT/Missile.h"

// Forward declarations
struct D2MissileSrvDoFuncData;
struct D2UnitStrc;
typedef int (__fastcall *fpMissileSrvDoFuncData)(D2UnitStrc * pUnit, void * pContext);

extern D2MissileSrvDoFuncData gaMissileSrvDoFuncTable[9];
