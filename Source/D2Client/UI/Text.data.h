/**
 * Data defined in D2Client/UI/Text.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "D2Client/GAME/Sound.h"

// Forward declarations
struct D2DialogStrc;
struct D2SoundStringKeyStrc;

extern D2SoundStringKeyStrc gaClassSoundStringKeys[7][74];

namespace D2Client::UI::Dialog {
extern D2DialogStrc * DialogHandle;
} // namespace D2Client::UI::Dialog
