/**
 * Data defined in D2Client/UNIT/Object.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "globals/globals.h"

// Forward declarations
struct D2ShrinesTxt;
struct D2UnitStrc;
typedef void (__fastcall *fnShrineClientOverlayUpdate)(D2UnitStrc * pUnit, D2ShrinesTxt * pShrinesTxt);
typedef void (__fastcall *fnShrineServerActivate)(D2UnitStrc * pUnit, D2UnitStrc * pPlayer, uint32_t dwParam);

namespace D2Client::UNIT::Object {
extern D2ShrineDispatchEntryStrc aShrineDispatchTable[23];
extern int nShrineDispatchTableSize;
} // namespace D2Client::UNIT::Object
