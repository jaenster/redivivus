/**
 * Data defined in Fog/Memory.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "Fog/D2MemoryManagerStrc.h"
#include "Fog/D2PoolManagerStrc.h"
#include "Fog/Memory.h"

// Forward declarations
struct D2MemoryManagerStrc;
struct D2PoolBlockEntryStrc;
struct D2PoolBlockStrc;

namespace Fog::Memory {
extern D2MemoryManagerStrc pGlobalPoolSystem;
} // namespace Fog::Memory
