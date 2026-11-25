// Redivivus' own code, not recovered from any Blizzard binary.
//
// Support state the server needs that the game keeps in files the server does not link.
#include "Redivivus/d2_trace.h"

// The per-opcode packet counters the engine's packet handlers bump (declared in
// Redivivus/d2_trace.h). A development build defines and reports them in
// Redivivus/d2_control.cpp; otherwise Redivivus/d2_trace.cpp holds them, and the server
// leaves that file out (it is the development tracer, and it probes D2gfx state), so
// they live here: counted, never read.
#if !defined(REDIVIVUS_DEV)
unsigned g_anClientPktCount[256];
unsigned g_anServerPktCount[256];
#endif
