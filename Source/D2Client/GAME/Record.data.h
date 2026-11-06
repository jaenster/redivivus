/**
 * Data defined in D2Client/GAME/Record.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "D2Client/GAME/Record.h"

// Forward declarations
struct D2GameRecordStrc;
typedef void (*RecordCleanupFn)();

extern D2GameRecordStrc gsGameRecordContext;

namespace D2Client::GAME::Record {
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
} // namespace D2Client::GAME::Record
