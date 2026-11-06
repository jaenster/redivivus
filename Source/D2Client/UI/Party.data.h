/**
 * Data defined in D2Client/UI/Party.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2UiPartyStrc;
struct DC6;

namespace D2Client::UI::Party {
extern D2UiPartyStrc * pUiParty;
extern DC6 * pMenu_party;
extern DC6 * pMenu_partybuttons;
extern DC6 * pMenu_partyboxes;
extern DC6 * pPanel_inv_goldbtn;
extern DC6 * pMenu_partybar;
extern uint8_t cRed;
} // namespace D2Client::UI::Party
