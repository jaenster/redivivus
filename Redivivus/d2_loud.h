// The loud build's game-loop hook (support/d2_loud.cpp). Without REDIVIVUS_LOUD_GUARDS
// it compiles to nothing.
#pragma once

#ifdef REDIVIVUS_LOUD_GUARDS
// Walk the guard zones between globals; called from the game's message loops.
// Throttled to one walk per 40 ms (one game frame at 25 fps).
extern "C" void D2LOUD_GuardPump(void);
#else
// A macro, not an empty inline: at -O0 even an empty call is code, and the normal
// build must stay the same bytes.
#define D2LOUD_GuardPump() ((void)0)
#endif
