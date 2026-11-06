/**
 * Data defined in D2Game/GAME/Clients.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Game)
#error "D2Game header included by a module that may not use D2Game (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2ClientStrc;

extern D2ClientStrc * QServerClientBucketById[256];
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern CRITICAL_SECTION gQServerClientByIdCs;
