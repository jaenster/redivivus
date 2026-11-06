// In-process control and telemetry channel.
//
// Driving the reconstruction from OUTSIDE (SetCursorPos + mouse_event) needs the game
// window to hold the foreground, moves the user's real pointer, and silently does nothing
// whenever another application steals focus - three ways for a test run to lie. This runs
// inside the process instead: a socket thread accepts commands and a pump hook applies
// them ON THE GAME THREAD, so input is deterministic and needs no focus at all.
//
// A development tool: it listens on a TCP port with no authentication, so it exists
// only in a REDIVIVUS_DEV build. Otherwise both calls compile to nothing.
#pragma once

// The loud build's per-frame guard walk rides on the same two pump sites.
#include "support/d2_loud.h"

#ifdef REDIVIVUS_DEV
// Start the listener. Safe to call more than once; only the first call binds.
extern "C" void D2CONTROL_Start(void);

// Drain queued actions and publish a state snapshot. MUST be called from the game thread -
// it dispatches through the game's own WndProc, exactly as Windows would.
extern "C" void D2CONTROL_Pump(void);

// d2_control.cpp names DRAW_GetBottomPanelY under D2Client::UI::Draw; the function now
// lives in D2Client::UI::ui (ui.cpp). Retire when d2_control.cpp calls it there.
#include "D2Client/UI/ui.h"
namespace D2Client::UI::Draw { using D2Client::UI::ui::DRAW_GetBottomPanelY; }
#else
static inline void D2CONTROL_Start(void) {}
static inline void D2CONTROL_Pump(void) {}
#endif
