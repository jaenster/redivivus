/**
 * Data defined in D2Client/UI/Text.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"
#include "Source/D2Client/GAME/Sound.h"

// Forward declarations
struct D2DialogStrc;
struct D2SoundStringKeyStrc;

extern D2SoundStringKeyStrc gaClassSoundStringKeys[7][74];

namespace D2Client::UI::Dialog {
extern D2DialogStrc * DialogHandle;
} // namespace D2Client::UI::Dialog
