/**
 * Data defined in D2Client/UI/Inv.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "D2Common/INVENTORY/Inventory.h"

// Forward declarations
struct D2GridScreenCoordinates;

extern uint8_t gbInvImageLoadToggle;

namespace D2Client::UI::Inv {
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
} // namespace D2Client::UI::Inv
