/**
 * Data defined in D2Common/Drlg/Preset.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Common)
#error "D2Common header included by a module that may not use D2Common (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2DrlgPresetTypeTablesStrc;

extern D2DrlgPresetTypeTablesStrc gPresetTypeTables;

namespace D2Common::Drlg::Preset {
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0096d610;
} // namespace D2Common::Drlg::Preset
