/**
 * Data defined in D2Game/GAME/Task.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Game)
#error "D2Game header included by a module that may not use D2Game (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// lpFrequency parameter of QueryPerformanceFrequency
extern LARGE_INTEGER gnPerformanceFrequency;
extern uint32_t gbPerformanceCounterAvailable;
extern CRITICAL_SECTION aTaskQueueCriticalSections[32];
