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

// What CallAs and ResumeAs came to: CallStatus in core/native/core.d.ts has the same values.
enum {
    D2LUA_CALL_ERROR = 0,      // an error: its value and a traceback string are pushed
    D2LUA_CALL_OK = 1,         // returned (CallAs: one result pushed) or finished (ResumeAs)
    D2LUA_CALL_YIELD = 2,      // ResumeAs: yielded; the values are on the coroutine
    D2LUA_CALL_FATAL = -1,     // time or memory ran out inside an entry still running: the error is pushed
    D2LUA_CALL_FAULTED = -2,   // the outermost entry ran out: the charged mod is faulted (logged)
    D2LUA_CALL_DISABLED = -3,  // the mod was faulted before; nothing ran
};
// The prefix of the core's chunk names. The watchdog lets a function of such a chunk finish (the
// deadline is acted on in the next mod instruction); load refuses the prefix to mods.
#define D2LUA_CORE_CHUNK_PREFIX "=[core]"

// Call the function under nArgs arguments at the top of the stack as mod nModId (-1: no owner
// change), for the core: an entry of its own when no entry runs, nested otherwise (no new budget).
// Leaves *pnPushed values as the D2LUA_CALL_ value says. A C boundary: nothing yields past it.
int D2LuaHost_CallAs(lua_State* L, int nModId, int nArgs, int* pnPushed);
// Resume co as mod nModId with nArgs values on co's stack, for the core's scheduler.
int D2LuaHost_ResumeAs(lua_State* L, lua_State* co, int nModId, int nArgs, int* pnResults);
// Whether a guarded entry is running.
int D2LuaHost_InEntry(lua_State* L);

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
// Log the error text at the top of the stack as mod pszMod's (through the error hook, which maps
// generated Lua lines to TypeScript). Pops it.
void D2LuaHost_LogErrorTop(lua_State* L, const char* pszMod);

// The step-limited pattern functions, set into the string table at nStringTable.
void D2LuaPattern_Install(lua_State* L, int nStringTable);
#endif
