// The core: Redivivus/Lua/core (TypeScript compiled to Lua, embedded by the generator into
// generated/d2_lua_core.gen.cpp) loaded into a state before any mod, with the natives it alone
// gets. It is what the redivivus mod is made of: its public modules are served as
// redivivus:<entry> from here and from nowhere else.
//
// The core runs in an environment of its own, which no mod reaches. Its modules are one instance
// per state, except the public ones (redivivus.*, with internal.context), which are instantiated
// once per mod that requires them, in an environment that names that mod, so every function in a
// mod's redivivus:server acts for that mod. Every core chunk is named "=[core] <module>", the name
// the watchdog recognises.
#pragma once

#if D2_FEATURE_MODS
#include <stddef.h>

#include "Redivivus/Lua/d2_lua_rt.h"

struct D2LuaCoreChunk {
    const char* pszName;
    const char* pszSource;
    size_t nSize;
};

enum { D2LUA_GRANT_TEST = 1, D2LUA_GRANT_DRIVER = 2, D2LUA_GRANT_MANAGER = 4 };

struct D2LuaCoreModule {
    const char* pszEntry;   // redivivus:<entry>
    const char* pszChunk;   // the chunk that makes it
    int nSides;             // the states it resolves in (D2LUA_SIDE_*)
    int nGrant;             // what the requiring mod needs (D2LUA_GRANT_*), 0 for none
    const char* const* apszValues;  // the values its declarations export
};

// Generated: every chunk (null-terminated list) and the public modules.
extern const D2LuaCoreChunk g_aD2LuaCoreChunks[];
extern const D2LuaCoreModule g_aD2LuaCoreModules[];

// Generated: the built-in redivivus mod's own program (Redivivus/Lua/builtin, compiled like a mod),
// by path in its namespace ("main.lua", "mods/model.lua"); null-terminated.
extern const D2LuaCoreChunk g_aD2LuaBuiltinFiles[];
// A file of that program, or null.
const D2LuaCoreChunk* D2LuaCore_FindBuiltinFile(const char* pszPath);

// The public module named entry, or null.
const D2LuaCoreModule* D2LuaCore_FindModule(const char* pszEntry);

// Start the core in the runtime's state: its environment, the natives table (every API set's
// natives), and the module the host calls into. Inside a protected call; raises on failure.
void D2LuaCore_Start(lua_State* L, D2LuaRuntime* pRt);

// Push mod pMod's instance of public module pModule (made on first use, then cached in the mod's
// require cache by the caller). Raises on failure.
void D2LuaCore_PushModule(lua_State* L, D2LuaRuntime* pRt, D2LuaMod* pMod, const D2LuaCoreModule* pModule);

// Drop every mod's instances of the public modules (a reload makes them again).
void D2LuaCore_ForgetInstances(lua_State* L);

// Push the host function pszName of the core (internal.host).
void D2LuaCore_PushHost(lua_State* L, const char* pszName);

// Call host function pszName with the nArgs values on top, leaving nResults results. Outside an
// entry the call is protected: an error is logged as the core's and false returned (nothing left).
// Inside one (a native called from mod code raised an event) errors pass up as they are.
bool D2LuaCore_CallHost(lua_State* L, const char* pszName, int nArgs, int nResults);

// For the host test's escape checks: a table of every table the core keeps out of mods' reach
// (env, natives, cache, instances, modEnvs, host).
void D2LuaCore_PushInternals(lua_State* L);
#endif
