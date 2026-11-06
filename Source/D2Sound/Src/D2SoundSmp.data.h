/**
 * Data defined in D2Sound/Src/D2SoundSmp.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

extern uint8_t gEAXPropertyGuid_Reserved1[16];
// @data ghSoundThreadStopEvent
// @wasnamed ghSoundThread
// @evidence Stores the result of CreateEventA at 0051654a - an EVENT, not a thread.
// SetEvent(it) at 00515129 and CloseHandle at 00515148. The genuine thread handle is 00881810, which
// stores CreateThread at 00516566 and is the one passed to WaitForSingleObject at 0051513a.
extern HANDLE ghSoundThreadStopEvent;
extern HANDLE ghSoundThread;
