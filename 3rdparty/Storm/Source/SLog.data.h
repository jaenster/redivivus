/**
 * Data defined in Storm/Source/SLog.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_Storm)
#error "Storm header included by a module that may not use Storm (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_Storm) && !defined(D2_LOADS_Storm)
#error "internal Storm header included from another module; include Storm's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// param_2 parameter of wsprintfA
extern char param_2_006cff28[3];
