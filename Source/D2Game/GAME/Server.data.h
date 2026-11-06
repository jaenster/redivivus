/**
 * Data defined in D2Game/GAME/Server.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

// Forward declarations
struct D2BattleNetEventCallbackTable;

extern uint32_t GameSeed;
extern D2BattleNetEventCallbackTable * BattleNetServerService;
extern BOOL IsBattleNetServer;
