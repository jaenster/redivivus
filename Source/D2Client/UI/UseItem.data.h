/**
 * Data defined in D2Client/UI/UseItem.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2Client) && !defined(D2_LOADS_D2Client)
#error "internal D2Client header included from another module; include D2Client's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2NpcCashTabStrc;

// @data gaNpcCashTabs_nTooltipStringId_col
// @wasnamed gnNpcCashTabBoundsY1
// @evidence u16 = 0x0FB1. Read at 004bfef3 as movzx word [piVar1-0x16] and passed
// to GetLocaleString. A string id, not a Y bound. @wasnamed gnNpcCashTabBoundsY1 (3-way collision).
extern D2NpcCashTabStrc gaNpcCashTabs[2];
