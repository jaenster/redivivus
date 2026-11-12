/**
 * Data defined in D2Client/UI/Inv.cpp that other translation units name.
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
struct D2GridScreenCoordinates;

extern int gnInvMouseX;
extern int gpInvCursorItem;

namespace D2Client { namespace UI { namespace Inv {
extern D2GridScreenCoordinates pTradePage2GridScreenCoordinates;
extern D2GridScreenCoordinates pBigBankPageGridScreenCoordinates;
// One 5-byte palette of inventory/automap highlight colours, indexed as an array. Split across three globals and two function-local statics it could never be contiguous.
extern uint8_t gaInvHighlightColors[5];
extern D2GridScreenCoordinates pTradePage1GridScreenCoordinates;
extern D2GridScreenCoordinates pBankPageGridScreenCoordinates;
extern D2GridScreenCoordinates pNpcInventoryGridScreenCoordinates;
extern D2GridScreenCoordinates pTransmogrifyBoxGridScreenCoordinates;
extern D2GridScreenCoordinates pInventoryGridScreenCoordinated;
extern D2GridScreenCoordinates pHirelingGridScreenCoordinates;
}}} // namespace D2Client::UI::Inv
