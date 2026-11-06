/**
 * Data defined in D2Game/GAME/Task.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

// lpFrequency parameter of QueryPerformanceFrequency
extern LARGE_INTEGER gnPerformanceFrequency;
extern uint32_t gbPerformanceCounterAvailable;
extern CRITICAL_SECTION aTaskQueueCriticalSections[32];
