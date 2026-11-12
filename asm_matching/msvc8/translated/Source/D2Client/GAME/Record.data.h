/**
 * Data defined in D2Client/GAME/Record.cpp that other translation units name.
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
struct D2GameRecordStrc;
typedef void (*RecordCleanupFn)();

extern D2GameRecordStrc gsGameRecordContext;

namespace D2Client { namespace GAME { namespace Record {
extern RecordCleanupFn CleanUpFn;
extern uint32_t NotRecording;
extern BOOL SettedUp;
// @data gbIsRecording
// @wasnamed IsRecording
// @evidence BOOL flag read by D2Game::Game::Record::IsRecording @00451980 (which is the
// only reader) and written by LoadGameRecordFromFile, StopRecordingSetFlags and SetupRecording. It had
// the SAME fully-qualified name as the accessor function, which the emitter cannot express.
extern BOOL gbIsRecording;
// @data gbIsPlaybackActive
// @wasnamed IsPlaybackActive
// @evidence BOOL flag read by D2Game::Game::Record::IsPlaybackActive @00451990 and
// written by RecordCurrentStep, SetUnkownOn1 and LoadGameRecordFromFile. Same name as its accessor.
extern BOOL gbIsPlaybackActive;
}}} // namespace D2Client::GAME::Record
