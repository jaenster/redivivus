// Redivivus' own code, not recovered from any Blizzard binary.
//
// The seam between the game and whatever hosts it as a dedicated server.
//
// 1.14d dispatches each application mode through a two-entry bootstrap
// (Game::Main::sModStateFunc): init(hModule) returns a pointer to the mode's function
// table, ApplicationRunCurrentMode calls table[0](pD2IniConfig), then destroy(). The
// APPMODE_server row ships empty. Before 1.14 the same row was D2Server.dll:
// QueryInterface() returned its table and the host called table[0](config). A server
// host here is exactly that row.
//
// The default host (ServerHost.cpp) is a minimal public one. Other hosts implement
// the same two functions and are chosen at link time; exactly one is linked.
#pragma once

#include "Source/globals/globals.h"     // fnAppModeRun, D2IniConfigStrc, HMODULE

namespace Redivivus { namespace Server {

// Returns the host's function table; table[0] runs the server until it shuts down
// and returns the process exit code. Called once, after the command line is parsed.
fnAppModeRun * __fastcall ServerHost_Init(HMODULE hModule);

// Called once after table[0] returns.
void __stdcall ServerHost_Shutdown();

}}  // namespace Redivivus::Server
