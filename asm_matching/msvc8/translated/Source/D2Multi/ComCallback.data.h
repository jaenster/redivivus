/**
 * Data defined in D2Multi/ComCallback.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Multi)
#error "D2Multi header included by a module that may not use D2Multi (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2Multi) && !defined(D2_LOADS_D2Multi)
#error "internal D2Multi header included from another module; include D2Multi's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2ChannelUserStrc;

extern char gszCurrentChannelName[256];
extern D2ChannelUserStrc * gpCurrentChatRoomContext;
extern uint32_t gdwLastChatSendTick;
