// The music stream's play step. The 1.14d Mac binary places it in D2SoundStrm.cpp: the
// streaming functions (002e9778..002e9cce) sit after D2SoundSmp.cpp and before D2SoundUtil.cpp,
// a unit of their own apart from D2SoundFast.cpp (002e7df1..), and the Windows asserts in
// SOUND_GetAmbientSoundEntry and its inlined copies name D2Sound/Src/D2SoundStrm.cpp. Here the
// rest of that unit is still shared with Windows in D2SoundFast.cpp; only this Mac-only
// function lives at its own path. Windows has no such function (SOUND_PlayNextAmbientSound
// inlines it), so the file compiles away there.

#if !defined(_WIN32) && D2_OS_MAC

#include "Source/D2Sound/Src/D2SoundSmp.h"
#include "Source/D2Sound/D2Sound.globals.h"
#include "Source/D2Headers/d2_platform.h"

namespace D2Sound::Src::D2SoundStrm {

using D2SoundFast::gpMusicStreamBuffer;

// 1.14d mac: 002e9952
BOOL SOUNDSTRM_PlayMusicFile(char * szFile, int32_t bIsLooping) {
    if (gpMusicStreamBuffer && D2SoundSmp::SOUNDSMP_GetEffectiveVolume() && !D2SoundSmp::SOUNDSMP_IsStreaming(gpMusicStreamBuffer) && szFile) {
      D2SoundSmp::SOUNDSMP_Reset(gpMusicStreamBuffer);
      D2SoundSmp::SOUNDSMP_SetVolume(gpMusicStreamBuffer, 0x6e);
      if (D2SoundSmp::SOUNDSMP_OpenAndStreamFile(gpMusicStreamBuffer, szFile, 0, bIsLooping)) {
        return 1;
      }
    }

    return 0;
}

} // namespace D2Sound::Src::D2SoundStrm

#endif
