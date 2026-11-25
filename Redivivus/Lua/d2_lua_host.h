// The game-independent half of the Lua host: the sandboxed state, the per-mod environments,
// the guards on time and memory, print, and running a script with its errors reported. It
// knows nothing of the game, so it is built and run on its own by
// Redivivus/Lua/test/d2_lua_host_test.cpp; Redivivus/Lua/d2_lua.cpp ties it to the game's log.
#pragma once

#if D2_FEATURE_MODS
#include <stddef.h>

struct lua_State;

// Receives every line the host writes: print output and errors. One call per line, no
// trailing newline.
typedef void (*D2LuaLogFn)(const char* pszLine);

// Limits for one state. A zero field takes the default.
struct D2LuaLimits {
    // Wall-clock time one call from the host into a mod may run. When it runs out, every
    // following Lua instruction of that call raises an error until control is back in the
    // host, and the mod is faulted. Default 200 ms.
    unsigned nBudgetMs;
    // Bytes the whole state may hold while a mod runs. An allocation past it fails as a Lua
    // out-of-memory error and faults the mod, even when the mod catches the error itself.
    // Default 128 MiB.
    size_t nMemoryBytes;
};

// What a run or call came to.
enum {
    D2LUA_DISABLED = -3,  // the mod is faulted; nothing was run
    D2LUA_FAULT = -2,     // the mod ran out of time or memory and is now faulted
    D2LUA_ERROR = -1,     // a load or runtime error, logged; the mod stays enabled
    D2LUA_MISSING = 0,    // the file or function does not exist; nothing was run
    D2LUA_OK = 1,
};

// One LuaJIT state with the JIT on. Mods never see its global table: each mod runs in an
// environment of its own (D2LuaHost_NewModEnv). pLimits may be null for the defaults.
// Returns null (and has logged why) on failure.
lua_State* D2LuaHost_Create(D2LuaLogFn pfnLog, const D2LuaLimits* pLimits);

// Push the environment of mod pszMod, creating it on first use: base functions without
// getfenv, setfenv, newproxy, dofile, loadfile; load and loadstring that refuse bytecode and
// bind loaded chunks to this environment; collectgarbage("count") only; and copies of the
// string, table, math, bit and coroutine tables, so whatever one mod does to its own copies
// no other mod sees. _G is the environment itself. Pushes nil for a faulted mod.
// Returns 1 on success, 0 for a faulted mod, -1 on error (logged, nil pushed).
int D2LuaHost_NewModEnv(lua_State* L, const char* pszMod);

// Run a Lua source string as mod pszMod (in its environment, under the time and memory
// limits). Bytecode is refused. Returns one of the D2LUA_ values.
int D2LuaHost_RunString(lua_State* L, const char* pszMod, const char* pszSource, const char* pszChunkName);

// Run a Lua source file as mod pszMod. A missing file is not an error: D2LUA_MISSING, silent.
int D2LuaHost_RunFile(lua_State* L, const char* pszMod, const char* pszPath, const char* pszChunkName);

// Call the global function pszFunction of mod pszMod with no arguments, under the limits.
// D2LUA_MISSING when the mod has no such function.
int D2LuaHost_CallMod(lua_State* L, const char* pszMod, const char* pszFunction);

// 1 when the mod has been faulted (out of time or memory) and is no longer run.
int D2LuaHost_IsModFaulted(lua_State* L, const char* pszMod);

// Bytes the state holds now (the allocator's count).
size_t D2LuaHost_MemoryUsed(lua_State* L);

void D2LuaHost_Destroy(lua_State* L);

// "LuaJIT 2.1.<n>", the version linked in.
const char* D2LuaHost_Version(void);
#endif
