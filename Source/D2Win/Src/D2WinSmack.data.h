/**
 * Data defined in D2Win/Src/D2WinSmack.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Win)
#error "D2Win header included by a module that may not use D2Win (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

extern char gpSmackerVideoBuffer[260];
extern uint8_t gbSmackerVideoFlags;

namespace D2Win::Src::D2WinSmack {
extern uint32_t gbSmackerKeepTempFile;
} // namespace D2Win::Src::D2WinSmack
