/**
 * Data defined in D2CMP/SRC/FastCmp.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2CMP)
#error "D2CMP header included by a module that may not use D2CMP (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

namespace D2CMP::SRC::FastCmp {
extern BOOL gbCodecUseFallbackDecompressor;
} // namespace D2CMP::SRC::FastCmp
