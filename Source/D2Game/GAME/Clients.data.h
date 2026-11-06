/**
 * Data defined in D2Game/GAME/Clients.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

// Forward declarations
struct D2ClientStrc;

extern D2ClientStrc * QServerClientBucketById[256];
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern CRITICAL_SECTION gQServerClientByIdCs;
