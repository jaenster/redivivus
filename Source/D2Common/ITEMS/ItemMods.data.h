/**
 * Data defined in D2Common/ITEMS/ItemMods.cpp that other translation units name.
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

// Forward declarations
struct D2UnitStrc;
typedef uint32_t (__fastcall *fpProperties)(uint32_t param_1, D2UnitStrc * param_2, D2UnitStrc * param_3, uint32_t param_4, uint32_t param_5, int param_6, uint32_t param_7, uint32_t param_8, int param_9, uint32_t param_10, uint32_t param_11);

extern fpProperties PROPERTIESFUNCTIONS[37];
