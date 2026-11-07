/**
 * Data defined in Bnclient/net_sid.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_Bnclient)
#error "Bnclient header included by a module that may not use Bnclient (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_Bnclient) && !defined(D2_LOADS_Bnclient)
#error "internal Bnclient header included from another module; include Bnclient's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2Srp;

extern uint32_t g_dwLastAdId;
extern D2Srp * g_pSrpContext;
