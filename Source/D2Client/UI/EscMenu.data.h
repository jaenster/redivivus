/**
 * Data defined in D2Client/UI/EscMenu.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2EscMenuUIFlagStrc;

// 38 records of {nEnabled,nHideable,nSavedState}. Split into three labels, so opening the ESC menu wrote 456 bytes past a 4-byte object and read its per-flag config from garbage.
extern D2EscMenuUIFlagStrc gaEscMenuUIFlags[38];
