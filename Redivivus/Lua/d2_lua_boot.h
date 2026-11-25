// Mods at process start: discover the mods, put them in load order (disabling the ones that
// cannot load), run the settings stage so every declared setting is known, and log all of it.
// The stages that need engine hooks the tree does not have yet (the data stage's table hooks, the
// control stages' game hooks) are not run. The game calls this from D2LUA_Boot (d2_lua.cpp); the
// host test calls it over a fixture directory.
#pragma once

#if D2_FEATURE_MODS
#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_runtime.h"

struct D2LuaBootDesc {
    std::vector<std::string> aRoots;  // mod directories; the game's is mods/ beside the executable
    std::string sSettingsFile;        // the mods' settings file (mods/settings.json)
    std::vector<const D2LuaApiDesc*> aApis;
    D2LuaLogFn pfnLog = nullptr;
    D2LuaLimits tLimits = { 0, 0 };
};

struct D2LuaBootResult {
    std::vector<D2LuaModInfo> aFound;   // every mod discovered
    std::vector<D2LuaModInfo> aOrder;   // the ones that load, in load order
    std::vector<std::string> aErrors;   // discovery errors and disabled mods, as logged
    D2LuaRuntime* pSettings = nullptr;  // the settings stage, kept for the settings screen
};

void D2LuaBoot_Run(const D2LuaBootDesc& tDesc, D2LuaBootResult& tOut);
void D2LuaBoot_Close(D2LuaBootResult& tResult);
#endif
