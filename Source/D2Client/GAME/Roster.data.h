/**
 * Data defined in D2Client/GAME/Roster.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2RosterPartyListStrc;
struct D2RosterStrc;

namespace D2Client::GAME::Roster {
extern D2RosterStrc * gpRosterOther;
extern D2RosterPartyListStrc * pRosterPartyList;
} // namespace D2Client::GAME::Roster
