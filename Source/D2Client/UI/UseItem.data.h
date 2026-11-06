/**
 * Data defined in D2Client/UI/UseItem.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "D2Client/UI/UseItem.h"

// Forward declarations
struct D2NpcCashTabStrc;

// @data gaNpcCashTabs_nTooltipStringId_col
// @wasnamed gnNpcCashTabBoundsY1
// @evidence u16 = 0x0FB1. Read at 004bfef3 as movzx word [piVar1-0x16] and passed
// to GetLocaleString. A string id, not a Y bound. @wasnamed gnNpcCashTabBoundsY1 (3-way collision).
extern D2NpcCashTabStrc gaNpcCashTabs[2];
