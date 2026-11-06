/**
 * Data defined in Storm/Source/SMem.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_Storm)
#error "Storm header included by a module that may not use Storm (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

extern CRITICAL_SECTION gUICtrlSkillColumnState[256];
extern CRITICAL_SECTION gUICtrlPanelArrowState[256];
