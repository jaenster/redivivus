/**
 * Data defined in D2Sound/Sound.cpp that other translation units name.
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
