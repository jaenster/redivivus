/**
 * Data defined in D2Game/GAME/Server.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Game)
#error "D2Game header included by a module that may not use D2Game (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2BattleNetEventCallbackTable;

extern uint32_t GameSeed;
extern D2BattleNetEventCallbackTable * BattleNetServerService;
extern BOOL IsBattleNetServer;
