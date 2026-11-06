/**
 * Data defined in D2Hell/D2Hell.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Hell)
#error "D2Hell header included by a module that may not use D2Hell (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct StormLinkedListStrc;

extern StormLinkedListStrc gFaultHandlerList;
