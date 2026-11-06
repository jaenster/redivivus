/**
 * Data defined in Game/Main.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "globals/globals.h"
#include "Game/Main.h"

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
