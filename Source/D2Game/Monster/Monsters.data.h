/**
 * Data defined in D2Game/Monster/Monsters.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

// Forward declarations
struct D2UnitStrc;
typedef void (__fastcall *fpUmodInit)(D2UnitStrc * pUnit, int32_t nIndex, BOOL bUnknown);

namespace D2Game::Monsters::Umod {
extern fpUmodInit aUmodInitTable[43];
} // namespace D2Game::Monsters::Umod
