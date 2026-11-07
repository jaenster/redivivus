/**
 * Data defined in Fog/Memory.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_Fog)
#error "Fog header included by a module that may not use Fog (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_Fog) && !defined(D2_LOADS_Fog)
#error "internal Fog header included from another module; include Fog's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2MemoryManagerStrc;
struct D2PoolBlockEntryStrc;
struct D2PoolBlockStrc;

namespace Fog::Memory {
extern uint32_t gpServerMemHeap;
extern D2MemoryManagerStrc pGlobalPoolSystem;
} // namespace Fog::Memory
