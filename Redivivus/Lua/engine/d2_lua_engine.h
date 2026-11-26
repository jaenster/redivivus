// What the engine bindings (d2_lua_engine.cpp, d2_lua_menu.cpp) take from the boot (d2_lua.cpp):
// the mods that load, in order, and where their log lines go; and what the boot calls in the menu
// binding. The game's build only.
#pragma once

#if D2_FEATURE_MODS
#include <string>

#include "Redivivus/Lua/d2_lua_boot.h"

// The boot's result, or null when there was no boot or no mods directory.
const D2LuaBootResult* D2LUA_BootResult(void);
// The mods' settings file (mods/settings.json beside the executable).
const std::string& D2LUA_SettingsFile(void);
// A line in the game's log.
void D2LUA_LogToGame(const char* pszLine);

// d2_lua_menu.cpp: a process the Mods screen's restart started waits here, before the boot, until
// the one that started it has quit; at shutdown the client state closes.
void D2LuaMenu_WaitForPrevious(void);
void D2LuaMenu_Shutdown(void);
#endif
