// What the units of the Lua host share with each other beyond d2_lua_host.h: the guarded call
// with results, coroutine resumes under the guard, mod ids and the owner stack that decides
// which mod a fault belongs to, and the hooks the mod runtime (d2_lua_runtime.cpp) installs.
// Not for the game.
#pragma once

#if D2_FEATURE_MODS
#include <stddef.h>

#include "Redivivus/Lua/d2_lua_host.h"

struct lua_State;

// --- mod ids ---------------------------------------------------------------------------------------
// Every mod name the host has seen gets a small id, stable for the state's lifetime.
int D2LuaHost_ModId(lua_State* L, const char* pszMod);
const char* D2LuaHost_ModName(lua_State* L, int nModId);  // null for an unknown id

// --- the owner stack --------------------------------------------------------------------------------
// Which mod's code is running, as far as the host can tell: every entry from the host into a mod
// (guarded calls, resumes of host scripts) and every host-mediated crossing between mods (remote
// calls, registry entries) pushes the mod whose function runs, with the thread it runs on. When a
// deadline passes or an allocation is refused, the innermost owner is charged:
//   - the budget hook runs on the thread that was executing. When that thread is the thread of
//     the top entry, the top entry's mod is charged; otherwise the thread is a coroutine entered
//     after the top entry, and the mod that created it (D2LuaHost_CoroutineOwner) is charged.
//   - a refused allocation charges the top entry (the allocator cannot tell which thread runs).
void D2LuaHost_PushOwner(lua_State* L, int nModId);
void D2LuaHost_PopOwner(lua_State* L);
int D2LuaHost_CurrentOwner(lua_State* L);  // -1 when no mod code is running
int D2LuaHost_OwnerDepth(lua_State* L);
void D2LuaHost_RestoreOwnerDepth(lua_State* L, int nDepth);
// The mod that created coroutine co (through its environment's coroutine.create or wrap), or -1.
int D2LuaHost_CoroutineOwner(lua_State* L, lua_State* co);
void D2LuaHost_SetCoroutineOwner(lua_State* L, int nThreadIndex, int nModId);

// --- guarded entries -----------------------------------------------------------------------------
// Call the function under nArgs arguments at the top of the stack as mod pszMod, under the
// deadline and the memory limit. On D2LUA_OK the nResults results are left on the stack;
// otherwise nothing is left. Errors are logged; running out of time or memory faults the mod the
// owner stack charges (normally pszMod) and returns D2LUA_FAULT.
int D2LuaHost_GuardedCall(lua_State* L, const char* pszMod, int nArgs, int nResults);

// Resume coroutine co (owned by pszMod) with the nArgs values on top of co's stack, under the
// guard, as one entry. Returns D2LUA_OK when it finished, D2LUA_YIELD when it yielded (the
// yielded values stay on co's stack, *pnResults of them), or an error/fault code as above.
enum { D2LUA_YIELD = 2 };
int D2LuaHost_GuardedResume(lua_State* L, lua_State* co, const char* pszMod, int nArgs, int* pnResults);

// Whether the current guarded call's deadline has passed (the pattern matcher polls this).
int D2LuaHost_DeadlinePassed(lua_State* L);
// Whether an allocation has been refused during the current guarded call.
int D2LuaHost_MemoryRefused(lua_State* L);

// Fault a mod: logged by the caller, environment dropped, never run again.
void D2LuaHost_FaultMod(lua_State* L, const char* pszMod);

// --- hooks for the runtime ---------------------------------------------------------------------
// Called at the end of creating a mod's environment (the environment at nEnv), inside a
// protected call: the runtime installs require, the API tables and storage there.
typedef void (*D2LuaEnvHook)(lua_State* L, int nEnv, int nModId, void* pUser);
// Called with an error message before it is logged; writes the text to log into pszOut (at most
// nOut bytes with the terminator). The runtime maps generated Lua lines back to TypeScript here.
// Plain C++, no Lua calls.
typedef void (*D2LuaErrorHook)(const char* pszIn, char* pszOut, size_t nOut, void* pUser);
void D2LuaHost_SetHooks(lua_State* L, D2LuaEnvHook pfnEnv, D2LuaErrorHook pfnError, void* pUser);
void* D2LuaHost_HookUser(lua_State* L);

// Log a line through the state's log function.
void D2LuaHost_Log(lua_State* L, const char* pszLine);

// The step-limited pattern functions, set into the string table at nStringTable.
void D2LuaPattern_Install(lua_State* L, int nStringTable);
#endif
