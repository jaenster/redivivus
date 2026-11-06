/**
 * Data defined in Storm/Source/SEVT.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_Storm)
#error "Storm header included by a module that may not use Storm (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"
#include "Source/D2Hell/D2Hell.h"
#include "3rdparty/Storm/Storm.h"

// Forward declarations
struct D2StormListHead;

extern D2StormListHead gpSevtBreakCmdListHead;
