/**
 * Data defined in D2Game/PLAYER/PlayerMsg.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "globals/globals.h"

// Forward declarations
struct D2UnitStrc;
struct SGAMEDATA;
typedef eD2ServerIncomingStatus (__fastcall *D2NET_SERVER_CLTtoD2GS)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, char * pBytes, uint32_t nSize);

extern D2NET_SERVER_D2GS_ClientToServer NET_D2GS_SERVER_INCOMING[103];
