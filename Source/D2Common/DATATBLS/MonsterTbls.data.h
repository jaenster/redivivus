/**
 * Data defined in D2Common/DATATBLS/MonsterTbls.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Common)
#error "D2Common header included by a module that may not use D2Common (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2Common) && !defined(D2_LOADS_D2Common)
#error "internal D2Common header included from another module; include D2Common's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

extern uint32_t gszHirelingFieldName_act;
extern char s_A_006ed5ac[2];
extern char s_B_006ed5b0[2];
