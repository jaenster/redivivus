/**
 * Data defined in Storm/Source/SFile.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_Storm)
#error "Storm header included by a module that may not use Storm (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

extern uint16_t gnSFileSearchLocale;
extern uint32_t gnSSystemOption3;
extern uint32_t gnSSystemOption1;
extern uint32_t gnSSystemWaveBufferSize;
extern uint32_t gnSSystemAsyncReadSize;
extern uint32_t gbCompressionDebugOutput;
