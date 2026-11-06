/**
 * Data defined in D2Net/Game/Server.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Net)
#error "D2Net header included by a module that may not use D2Net (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2QServerStrc;

extern int NET_D2GS_CLIENT_INCOMING_SIZE[181];
extern D2QServerStrc * pQServer;
