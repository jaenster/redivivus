/**
 * Data defined in D2Client/UNIT/Object.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2Client) && !defined(D2_LOADS_D2Client)
#error "internal D2Client header included from another module; include D2Client's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"
#include "Source/globals/globals.h"

// Forward declarations
struct D2ShrinesTxt;
struct D2UnitStrc;
typedef void (__fastcall *fnShrineClientOverlayUpdate)(D2UnitStrc * pUnit, D2ShrinesTxt * pShrinesTxt);
typedef void (__fastcall *fnShrineServerActivate)(D2UnitStrc * pUnit, D2UnitStrc * pPlayer, D2ShrinesTxt * dwParam);

namespace D2Client::UNIT::Object {
extern D2ShrineDispatchEntryStrc aShrineDispatchTable[23];
extern int nShrineDispatchTableSize;
} // namespace D2Client::UNIT::Object
