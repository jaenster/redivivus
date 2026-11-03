/**
 * Data defined in D2Sound/Sound.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

// Forward declarations
struct IDirectSound;

// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION gSoundSmpCs;
extern IDirectSound * gnDirectSoundInitStatus;
extern uint32_t gnDSoundPrimaryBufferDelay;
extern uint32_t gnSound3DLoudness;
extern int gnSoundQualityMode;
extern uint32_t g_bEAXEnabled;
extern uint32_t g_bSoundDriverScanComplete;
