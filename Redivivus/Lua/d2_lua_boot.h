// Mods at process start: discover the mods, leave out the ones the player turned off in the mod
// list, put the rest in load order (disabling the ones that cannot load), run the settings stage
// so every declared setting is known, and log all of it. What it learnt is kept as the catalogue
// (d2_lua_ui.h) the Mods screen reads.
// The stages that need engine hooks the tree does not have yet (the data stage's table hooks, the
// control stages' game hooks) are not run. The game calls this from D2LUA_Boot (d2_lua.cpp); the
// host test calls it over a fixture directory.
#pragma once

#if D2_FEATURE_MODS
#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_runtime.h"
#include "Redivivus/Lua/d2_lua_ui.h"

struct D2LuaBootDesc {
    std::vector<std::string> aRoots;  // mod directories; the game's is mods/ beside the executable
    std::string sSettingsFile;        // the mods' settings file (mods/settings.json)
    std::string sModListFile;         // the player's mod list (mods/mod-list.json); empty: every mod is on
    std::vector<const D2LuaApiDesc*> aApis;
    D2LuaLogFn pfnLog = nullptr;
    D2LuaLimits tLimits = { 0, 0 };
};

struct D2LuaBootResult {
    std::vector<D2LuaModInfo> aFound;   // every mod discovered
    std::vector<D2LuaModInfo> aOrder;   // the ones that load, in load order
    std::vector<std::string> aErrors;   // discovery errors and disabled mods, as logged
    D2LuaRuntime* pSettings = nullptr;  // the settings stage, kept for the settings screen
    D2LuaCatalog tCatalog;              // every mod found and how it stands, the settings, the mod list file
    // Mods the player granted the raw tier of the overrides (engine.unsafe): the names listed in
    // mods/unsafe-mods.json, { "mods": ["name", ...] }, a file only the player writes.
    std::vector<std::string> aRawGrant;
};

void D2LuaBoot_Run(const D2LuaBootDesc& tDesc, D2LuaBootResult& tOut);
void D2LuaBoot_Close(D2LuaBootResult& tResult);
#endif
