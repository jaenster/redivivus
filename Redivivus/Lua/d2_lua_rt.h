// The mod runtime's own structures, shared by d2_lua_runtime.cpp, d2_lua_natives.cpp,
// d2_lua_storage.cpp and d2_lua_handle.cpp. Not for the game: it uses d2_lua_runtime.h.
//
// Lua-side state lives in one registry table (D2LuaRt_PushTable), never reachable from a mod:
//   envs[mod]           the mod's environment
//   handlers[mod]       { [event] = function }
//   init[mod], load[mod]
//   loaded[mod]         { [module key] = value or the loading sentinel }
//   remote[name]        { owner = mod, fns = { [name] = function } }
//   events[name]        the @events map that declared the event (for this side)
//   registries[name]    { kind = 0|1, entries = { [id] = entry } }, a slot registry uses id ""
//                       entry = { impl = f, owner = mod id or -1, wrappers = { w... } }
//                       w = { fn = f, mod = mod id, order = load index, seq = n }
//   scripts             array of { co, owner, due, event, id, holder } in start order; owner is the
//                       mod, holder the handle whose object the script lives as long as (or nil)
//   script_threads      { [thread] = true }, weak keys
//   enums[name]         { values = { [member] = value }, mt = the metatable of each mod's proxy }
//   messages[mod]       the mod's script.on_message handler
//   classes[name]       handle class: { mt = metatable, intern = { [id] = handle } (weak values),
//                       cls = lightuserdata D2LuaHandleClass* }
#pragma once

#if D2_FEATURE_MODS
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_api.h"
#include "Redivivus/Lua/d2_lua_runtime.h"

struct D2LuaMod {
    D2LuaRuntime* pRt = nullptr;
    int nIndex = 0;   // load order
    int nHostId = 0;  // d2_lua_internal.h mod id
    std::string sName;
    std::string sPath;
    // Mods this one may require from: itself and every dependency that is loaded (required,
    // optional, hidden optional, unordered).
    std::vector<std::string> aRequirable;
};

struct D2LuaSourceMap {
    std::string sSource;     // the TypeScript file, mod-relative ("server/control.ts")
    std::vector<int> aLines;  // generated line (index) -> source line (1-based), 0 when unknown
};

struct D2LuaRuntime {
    D2LuaRuntimeDesc tDesc;
    lua_State* L = nullptr;
    std::vector<std::unique_ptr<D2LuaMod>> aMods;
    std::map<std::string, D2LuaSourceMap> tMaps;  // chunk "__mod__/server/control.lua" -> map
    long long nTick = 0;
    int nWrapperSeq = 0;
    long long nScriptSeq = 0;  // the last script id given out
    D2LuaTransport tTransport = { nullptr, nullptr };
    D2Json tSettingsFile;                     // read once, at creation
    std::vector<D2LuaSettingInfo> aSettings;  // declared in the current state
};

// Read the settings file named in the description (d2_lua_settings.cpp). Plain C++.
void D2LuaRt_LoadSettingsFile(D2LuaRuntime* pRt);

// Push the runtime's registry table / one of its fields.
void D2LuaRt_PushTable(lua_State* L);
void D2LuaRt_PushField(lua_State* L, const char* pszField);
D2LuaRuntime* D2LuaRt_FromState(lua_State* L);
D2LuaMod* D2LuaRt_ModByHostId(D2LuaRuntime* pRt, int nHostId);
D2LuaMod* D2LuaRt_ModByName(D2LuaRuntime* pRt, const char* pszName);
// Push the mod's environment (nil when faulted).
void D2LuaRt_PushEnv(lua_State* L, D2LuaMod* pMod);
bool D2LuaRt_ModFaulted(lua_State* L, D2LuaMod* pMod);

// Load a source map for chunk sChunk (mod-relative "__mod__/x.lua") from sMapPath when it exists.
// Plain C++; returns before any Lua call.
void D2LuaRt_LoadSourceMap(D2LuaRuntime* pRt, const char* pszChunk, const char* pszMapPath);

// pairs for mod environments (enum proxies walk their members); upvalue 1 is next.
int D2LuaRt_EnvPairs(lua_State* L);

// The registry natives' installer: registries table for env at nEnv.
void D2LuaRt_InstallRegistries(lua_State* L, int nEnv, D2LuaMod* pMod);
// Call a registry entry with args at nArgBase..top as one chain walk; raises on error.
int D2LuaRt_RegistryCallRaw(lua_State* L, const char* pszRegistry, const char* pszId, int nArgBase);
// The same, protected: an error that comes up through the chain is raised as the error it carries.
int D2LuaRt_RegistryCallProtected(lua_State* L, const char* pszRegistry, const char* pszId, int nArgBase);

// storage (d2_lua_storage.cpp). Serialise the value at nIndex, appending to sOut; restore one
// value from the bytes at *pnPos, pushing it. Both run inside a protected call and raise errors.
void D2LuaStorage_Write(lua_State* L, int nIndex, std::string* pOut, const char* pszMod);
void D2LuaStorage_Read(lua_State* L, const std::string* pIn, size_t* pnPos);
#endif
