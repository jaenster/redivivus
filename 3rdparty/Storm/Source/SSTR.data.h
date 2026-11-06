/**
 * Data defined in Storm/Source/SSTR.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_Storm)
#error "Storm header included by a module that may not use Storm (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

extern double gdConst1;
extern double gfFloatScalingFactor;
// UTF-8 lead byte mark by sequence length (0,0,C0,E0,F0,F8,FC); split from gaUTF8ByteCountTable, read by the SSTR UTF-8 encoder
extern int gaUTF8LeadByteMarks[8];
