/**
 * Data defined in D2Game/PLAYER/PlayerMsg.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Game)
#error "D2Game header included by a module that may not use D2Game (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2Game) && !defined(D2_LOADS_D2Game)
#error "internal D2Game header included from another module; include D2Game's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"
#include "Source/globals/globals.h"

// Forward declarations
struct D2UnitStrc;
struct SGAMEDATA;
typedef eD2ServerIncomingStatus (__fastcall *D2NET_SERVER_CLTtoD2GS)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, char * pBytes, uint32_t nSize);

extern D2NET_SERVER_D2GS_ClientToServer NET_D2GS_SERVER_INCOMING[103];
