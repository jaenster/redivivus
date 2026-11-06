/**
 * Data defined in Game/Main.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_Game)
#error "Game header included by a module that may not use Game (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2IniConfigValuesStrc;

namespace Game::Main {
extern D2IniConfigValuesStrc D2IniConfigValues[58];
// lpServiceStatus parameter of SetServiceStatus
extern _SERVICE_STATUS lpServiceStatus_007065fc;
extern eD2ApplicationMode geModState;
extern SERVICE_STATUS_HANDLE ghServiceCtrlHandler;
extern HINSTANCE WinMain_hInstance;
extern uint32_t gnServiceRunning;
} // namespace Game::Main
