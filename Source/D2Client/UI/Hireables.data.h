/**
 * Data defined in D2Client/UI/Hireables.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2Client) && !defined(D2_LOADS_D2Client)
#error "internal D2Client header included from another module; include D2Client's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct DC6;

namespace D2Client::UI::Hireables {
extern DC6 * ClassIcons[7];
extern uint8_t HealthBarGreen;
extern uint8_t HealthBarYellowish;
extern uint8_t HealthBarRed;
extern bool HealthBarColorsInitialized;
} // namespace D2Client::UI::Hireables
