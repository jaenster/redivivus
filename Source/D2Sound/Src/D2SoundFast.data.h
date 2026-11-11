/**
 * Data defined in D2Sound/Src/D2SoundFast.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Sound)
#error "D2Sound header included by a module that may not use D2Sound (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2Sound) && !defined(D2_LOADS_D2Sound)
#error "internal D2Sound header included from another module; include D2Sound's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

namespace D2Sound::Src::D2SoundFast {
extern int gaAmbientSoundFlags;
// Ambient sound table for area-based background audio. Array of {char* szFilename, int bPlayed} entries. Count stored in gaAmbientSoundFlags. SOUND_PlayNextAmbientSound iterates through this table, alternates with gaAmbientSoundEntryTable based on gnAmbientSoundLanguageCode flag. Reset by SOUND_ResetAmbientSoundFlags.
extern int gAmbientSoundTable[16];
extern int gaAmbientSoundEntryTable[16];
extern uint32_t gnAmbientSoundCount;
extern uint32_t gnAmbientSoundLanguageCode;
extern uint32_t gbAmbientSoundLangInitialized;
} // namespace D2Sound::Src::D2SoundFast
