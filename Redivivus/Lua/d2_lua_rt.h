// The mod runtime's own structures, shared by the units of Redivivus/Lua. Not for the game: it uses
// d2_lua_runtime.h.
//
// Lua-side state lives in one registry table (D2LuaRt_PushTable), never reachable from a mod:
//   envs[mod]           the mod's environment
//   loaded[mod]         { [module key] = value or the loading sentinel }, the mod's require cache
//   events[name]        the events map that declared the event (for this side)
//   registries[name]    { kind = 0|1, natives = { [id] = C function } }, a slot registry uses id ""
//   enums[name]         { [member] = value }
//   classes[name]       handle class: { mt = metatable, intern = { [id] = handle } (weak values),
//                       cls = lightuserdata D2LuaHandleClass* }
// The core's tables (its environment, the natives, its module caches) are d2_lua_core.cpp's.
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
    int nIndex = 0;   // load order (0 is redivivus)
    int nHostId = 0;  // d2_lua_internal.h mod id
    bool bIntrinsic = false;  // redivivus: served from the exe, no environment, no files
    bool bTestGrant = false;
    bool bDriverGrant = false;
    bool bManagerGrant = false;  // redivivus:manager: the built-in mod only
    bool bRawGrant = false;      // engine.unsafe in patch(engine): the raw tier of the overrides
    std::string sName;
    std::string sPath;  // the folder holding the mod's namespace (mods/<name>/)
    // Mods this one may require from: itself and every dependency that is loaded (required,
    // optional, hidden optional, unordered), redivivus included.
    std::vector<std::string> aRequirable;
    // Declared dependencies that are not installed (for optionalImport).
    std::vector<std::string> aOptionalAbsent;
};

struct D2LuaSourceMap {
    std::string sSource;     // the TypeScript file, namespaced ("mods/my-mod/main.ts")
    std::vector<int> aLines;  // generated line (index) -> source line (1-based), 0 when unknown
};

struct D2LuaRuntime {
    D2LuaRuntimeDesc tDesc;
    lua_State* L = nullptr;
    std::vector<std::unique_ptr<D2LuaMod>> aMods;
    std::vector<D2LuaModInfo> aInfos;  // the mods in load order, redivivus first
    std::map<std::string, D2LuaSourceMap> tMaps;  // chunk "mods/my-mod/main.lua" -> map
    std::map<uint32_t, long long> tTicks;         // game -> its tick
    uint32_t nGame = 0;                           // the game the host is working on
    int nGeneration = 0;                          // loads of the mods' modules (a reload adds one)
    D2LuaTransport tTransport = { nullptr, nullptr };
    D2Json tSettingsFile;                     // read once, at creation
    std::vector<D2LuaSettingInfo> aSettings;  // declared in the current state
    std::vector<int> aInstantiating;          // mods whose public modules the core loader is making
    bool bUiDrawing = false;                  // the host has a frame open for the UI (d2_lua_ui.h)
    struct D2LuaOvState* pOv = nullptr;       // the functions its mods override (d2_lua_override.cpp)
};

// Read the settings file named in the description (d2_lua_settings.cpp). Plain C++.
void D2LuaRt_LoadSettingsFile(D2LuaRuntime* pRt);

// Push the runtime's registry table / one of its fields.
void D2LuaRt_PushTable(lua_State* L);
void D2LuaRt_PushField(lua_State* L, const char* pszField);
D2LuaRuntime* D2LuaRt_FromState(lua_State* L);
D2LuaMod* D2LuaRt_ModByHostId(D2LuaRuntime* pRt, int nHostId);
D2LuaMod* D2LuaRt_ModByName(D2LuaRuntime* pRt, const char* pszName);
// Push the mod's environment (nil when faulted or intrinsic).
void D2LuaRt_PushEnv(lua_State* L, D2LuaMod* pMod);
bool D2LuaRt_ModFaulted(lua_State* L, D2LuaMod* pMod);

// Load a source map for chunk sChunk ("mods/<name>/x.lua") from sMapPath when it exists. Plain
// C++; returns before any Lua call.
void D2LuaRt_LoadSourceMap(D2LuaRuntime* pRt, const char* pszChunk, const char* pszMapPath);

// Resolve a module for mod pMod as its require does, the result pushed. With bOptional, a declared
// dependency that is not installed pushes nil. Raises on everything else that fails.
void D2LuaRt_Require(lua_State* L, D2LuaMod* pMod, const char* pszName, bool bOptional);
// The same for code of mod pBase running in pMod's instance: bare names and dependencies are pBase's.
void D2LuaRt_RequireFrom(lua_State* L, D2LuaMod* pMod, D2LuaMod* pBase, const char* pszName, bool bOptional);

// pairs for mod environments (read-only proxies walk their values); upvalue 1 is next.
int D2LuaRt_EnvPairs(lua_State* L);
// The key a proxy's metatable keeps its values under.
void* D2LuaRt_WalkKey();

// storage (d2_lua_storage.cpp). Serialise the value at nIndex, appending to sOut; restore one
// value from the bytes at *pnPos, pushing it (a table into the table at nInto when nInto is not
// 0, which is pushed again). Both run inside a protected call and raise errors.
void D2LuaStorage_Write(lua_State* L, int nIndex, std::string* pOut, const char* pszMod);
void D2LuaStorage_Read(lua_State* L, const std::string* pIn, size_t* pnPos, int nInto);
#endif
