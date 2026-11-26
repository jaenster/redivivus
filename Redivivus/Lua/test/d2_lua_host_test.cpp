// The Lua host without the game: boots the sandboxed state, runs mods, and checks what a mod
// can and cannot reach, that one mod cannot change what another sees, and that runaway time and
// memory fault the mod without taking the state down. Prints every host log line prefixed
// "log: ", then one "PASS"/"FAIL" line per check and a total. Exit code 0 when every check passes.
//
//   cmake -S Redivivus/Lua/test -B build-luatest [-DCMAKE_TOOLCHAIN_FILE=cmake/mingw-i686.cmake]
//   cmake --build build-luatest && build-luatest/d2_lua_host_test[.exe]
//   (in the game's build directory with REDIVIVUS_MODS=ON: --target d2_lua_host_test)
//
//   d2_lua_host_test [mods/boot.lua]   also run that file the way the game runs mods/boot.lua
//   d2_lua_host_test --root DIR         the repository root (default: the current directory), for
//                                      the fixture mods under Redivivus/Lua/test/mods and base
//   d2_lua_host_test --ts-mods DIR      also run the TypeScript example mods the mod CLI built
//                                      into DIR (rdvmod build --out DIR)
//   d2_lua_host_test --bench           time a few hot loops (CHECKHOOK costs show here)
//   d2_lua_host_test --bench-dispatch ROOT TMP   time event dispatch and wrap chains through the core
//   d2_lua_host_test --runaway         run the compiled-loop checks even in a build without
//                                      LUAJIT_ENABLE_CHECKHOOK (they then never return)
//
// Compiled only with D2_LUA_HOST_TEST, so a build that globs every .cpp never links a main().
#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include <stdio.h>
#include <string.h>

#include <chrono>

#include "Redivivus/Lua/d2_lua_host.h"
#include "Redivivus/Lua/test/d2_lua_test_util.h"
#include "lua.hpp"

#ifndef D2_LUAJIT_CHECKHOOK
#define D2_LUAJIT_CHECKHOOK 1
#endif

typedef std::chrono::steady_clock Clock;

char g_szTestLog[262144];
#define s_szLog g_szTestLog

void TestLog(const char* pszLine) {
    printf("log: %s\n", pszLine);
    fflush(stdout);
    size_t nUsed = strlen(s_szLog);
    snprintf(s_szLog + nUsed, sizeof(s_szLog) - nUsed, "%s\n", pszLine);
}

void ClearLog() {
    s_szLog[0] = '\0';
}

bool Logged(const char* pszText) {
    return strstr(s_szLog, pszText) != nullptr;
}

static void QuietLog(const char* pszLine) {
    (void)pszLine;
}

static int s_nFailed = 0;
static int s_nPassed = 0;

void Check(int bOk, const char* pszWhat) {
    printf("%s %s\n", bOk ? "PASS" : "FAIL", pszWhat);
    fflush(stdout);
    if (bOk) {
        s_nPassed++;
    } else {
        s_nFailed++;
    }
}

static int s_nLastMs = 0;

// Run a chunk as mod pszMod with an empty log, then check its result and what it logged.
static void Expect(lua_State* L, const char* pszMod, const char* pszWhat, const char* pszSource, int nResult,
                   const char* pszLogged = nullptr, const char* pszLogged2 = nullptr) {
    s_szLog[0] = '\0';
    Clock::time_point tStart = Clock::now();
    int nGot = D2LuaHost_RunString(L, pszMod, pszSource, pszWhat);
    s_nLastMs = (int)std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - tStart).count();
    int bOk = nGot == nResult && (!pszLogged || strstr(s_szLog, pszLogged))
        && (!pszLogged2 || strstr(s_szLog, pszLogged2));
    if (!bOk) {
        printf("  got %d (wanted %d) after %d ms\n", nGot, nResult, s_nLastMs);
    }
    Check(bOk, pszWhat);
}

// Count the traces LuaJIT finishes, through jit.attach (the jit table is only in the registry's
// _LOADED, which no mod reaches). Counting live traces afterwards would not work: a faulted mod's chunk is collected at
// once, and its traces with it.
static void InstallTraceCounter(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
    lua_getfield(L, -1, "jit");
    lua_setglobal(L, "debug_jit");
    lua_pop(L, 1);
    if (luaL_dostring(L, "local jit = debug_jit __traces = 0 jit.attach(function(what) "
                         "if what == 'stop' then __traces = __traces + 1 end end, 'trace')") != 0) {
        printf("trace counter: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

static int TracesFinished(lua_State* L) {
    lua_getglobal(L, "__traces");
    int nTraces = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);
    return nTraces;
}

static int Bench(void) {
    D2LuaLimits tLimits = { 600000, (size_t)512 * 1024 * 1024 };
    lua_State* L = D2LuaHost_Create(QuietLog, &tLimits);
    if (!L) {
        return 1;
    }
    static const struct { const char* pszName; const char* pszSource; } s_aBench[] = {
        { "for-sum", "local s = 0 for i = 1, 200000000 do s = s + i end" },
        { "while-count", "local i = 0 while i < 200000000 do i = i + 1 end" },
        { "table-sum", "local t = {} for i = 1, 1000000 do t[i] = i end local s = 0 "
                       "for j = 1, 100 do for i = 1, #t do s = s + t[i] end end" },
        { "call-heavy", "local function f(x) return x + 1 end local s = 0 for i = 1, 100000000 do s = f(s) end" },
    };
    printf("bench checkhook=%d version=%s pointer=%d-bit\n", D2_LUAJIT_CHECKHOOK, D2LuaHost_Version(),
           (int)sizeof(void*) * 8);
    for (size_t b = 0; b < sizeof(s_aBench) / sizeof(s_aBench[0]); b++) {
        long nBest = -1;
        for (int nRun = 0; nRun < 5; nRun++) {
            Clock::time_point tStart = Clock::now();
            int nResult = D2LuaHost_RunString(L, "bench", s_aBench[b].pszSource, s_aBench[b].pszName);
            long nMs = (long)std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - tStart).count();
            if (nResult != D2LUA_OK) {
                printf("bench %s failed (%d)\n", s_aBench[b].pszName, nResult);
                return 1;
            }
            if (nBest < 0 || nMs < nBest) {
                nBest = nMs;
            }
        }
        printf("bench %-12s best of 5: %ld ms\n", s_aBench[b].pszName, nBest);
    }
    D2LuaHost_Destroy(L);
    return 0;
}

int main(int argc, char** argv) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    const char* pszBootFile = nullptr;
    const char* pszRoot = ".";
    const char* pszTsMods = nullptr;
    int bRunaway = D2_LUAJIT_CHECKHOOK;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--root") == 0 && i + 1 < argc) {
            pszRoot = argv[++i];
        } else if (strcmp(argv[i], "--ts-mods") == 0 && i + 1 < argc) {
            pszTsMods = argv[++i];
        } else if (strcmp(argv[i], "--bench-dispatch") == 0 && i + 2 < argc) {
            return RunDispatchBench(argv[i + 1], argv[i + 2]);
        } else if (strcmp(argv[i], "--bench") == 0) {
            return Bench();
        } else if (strcmp(argv[i], "--runaway") == 0) {
            bRunaway = 1;
        } else {
            pszBootFile = argv[i];
        }
    }

    const unsigned kBudgetMs = 250;
    const size_t kMemoryBytes = (size_t)32 * 1024 * 1024;
    D2LuaLimits tLimits = { kBudgetMs, kMemoryBytes };
    lua_State* L = D2LuaHost_Create(TestLog, &tLimits);
    Check(L != nullptr, "state created");
    if (!L) {
        return 1;
    }
    printf("version: %s, %d-bit, checkhook=%d, budget %u ms, memory %lu bytes\n", D2LuaHost_Version(),
           (int)sizeof(void*) * 8, D2_LUAJIT_CHECKHOOK, kBudgetMs, (unsigned long)kMemoryBytes);
    // Something only the real global table holds: no mod may ever read it.
    lua_pushliteral(L, "host-secret");
    lua_setglobal(L, "__host_secret");

    // --- what a mod has ---
    Expect(L, "base", "print reaches the log", "print('hello from lua', 1 + 2, nil, true)", D2LUA_OK,
           "[lua] hello from lua\t3\tnil\ttrue");
    Expect(L, "base", "string, table, math, bit present",
           "print(string.format('%05.1f', math.pi), table.concat({'a','b'}, '-'), bit.bxor(5, 3), ('ab'):rep(2))",
           D2LUA_OK, "[lua] 003.1\ta-b\t6\tabab");
    Expect(L, "base", "coroutine present",
           "local co = coroutine.wrap(function() coroutine.yield(7) end) print('co', co())", D2LUA_OK, "[lua] co\t7");
    Expect(L, "base", "absent: ffi io os debug package require jit dofile loadfile string.dump module",
           "print('absent', ffi, io, os, debug, package, require, jit, dofile, loadfile, string.dump, module)", D2LUA_OK,
           "[lua] absent\tnil\tnil\tnil\tnil\tnil\tnil\tnil\tnil\tnil\tnil\tnil");
    Expect(L, "base", "load works for text", "local f = load('return 6 * 7') print('load', f())", D2LUA_OK,
           "[lua] load\t42");
    Expect(L, "base", "loadstring works for text", "print('loadstring', loadstring('return 1 + 1')())", D2LUA_OK,
           "[lua] loadstring\t2");
    Expect(L, "base", "load refuses bytecode even when asked for mode b",
           "local f, e = load('\\27LJ\\2\\0', 'x', 'b') print('bc', f, e)", D2LUA_OK, "[lua] bc\tnil\t", "wrong mode");
    Expect(L, "base", "a bytecode chunk handed to the host is refused", "\x1bLJ\x02\x00", D2LUA_ERROR,
           "[lua] base: ", "wrong mode");
    Expect(L, "base", "runtime error logged with traceback, not fatal",
           "local function inner() error('boom') end\nlocal function outer() inner() end\nouter()", D2LUA_ERROR,
           "[lua] base: ", "stack traceback:");
    Expect(L, "base", "syntax error logged, not fatal", "this is not lua", D2LUA_ERROR, "[lua] base: ");
    Expect(L, "base", "error with a table value", "error({})", D2LUA_ERROR, "error object is a table value");
    Expect(L, "base", "a plain error does not disable the mod", "print('still alive')", D2LUA_OK,
           "[lua] still alive");
    Expect(L, "base", "multi-line print split into lines", "print('one\\ntwo')", D2LUA_OK, "[lua] one\n[lua] two\n");
    Expect(L, "base", "JIT loop runs", "local s = 0 for i = 1, 1000000 do s = s + i end print('sum', s)", D2LUA_OK,
           "[lua] sum\t500000500000");
    Expect(L, "base", "xpcall calls its handler",
           "print('xp', xpcall(function(a) error(a, 0) end, function(m) return 'handled ' .. m end, 'e'))", D2LUA_OK,
           "[lua] xp\tfalse\thandled e");
    Expect(L, "base", "  and a coroutine yields across it",
           "local co = coroutine.wrap(function() xpcall(function() coroutine.yield('y') end, print) end) print('xpy', co())",
           D2LUA_OK, "[lua] xpy\ty");
    Expect(L, "base", "globals persist within one mod", "base_value = 41", D2LUA_OK);
    Expect(L, "base", "  and are read back", "print('persist', base_value + 1)", D2LUA_OK, "[lua] persist\t42");

    // --- the global table stays out of reach ---
    Expect(L, "escape", "getfenv, setfenv, newproxy are gone",
           "print('fenv', getfenv, setfenv, newproxy)", D2LUA_OK, "[lua] fenv\tnil\tnil\tnil");
    Expect(L, "escape", "_G is the mod's own environment",
           "print('g', __host_secret, _G.__host_secret, rawget(_G, '__host_secret'), _G == _G._G)", D2LUA_OK,
           "[lua] g\tnil\tnil\tnil\ttrue");
    Expect(L, "escape", "loadstring and load chunks run in the mod's environment",
           "local r = {} r[1] = loadstring('return __host_secret')() r[2] = load('return __host_secret')() "
           "local done = false "
           "r[3] = load(function() if done then return nil end done = true return 'return __host_secret' end)() "
           "r[4] = loadstring('return _G')() == _G "
           "print('loadenv', r[1], r[2], r[3], r[4])", D2LUA_OK, "[lua] loadenv\tnil\tnil\tnil\ttrue");
    Expect(L, "escape", "load with an explicit env table uses it",
           "print('loadarg', load('return x', 'n', 't', { x = 5 })())", D2LUA_OK, "[lua] loadarg\t5");
    Expect(L, "escape", "a coroutine's chunks run in the mod's environment",
           "print('coenv', coroutine.wrap(function() return loadstring('return __host_secret')() end)())", D2LUA_OK,
           "[lua] coenv\tnil");
    Expect(L, "escape", "the functions of print and friends lead nowhere (no getfenv to ask)",
           "print('cfenv', pcall(function() return getfenv(print).__host_secret end))", D2LUA_OK,
           "[lua] cfenv\tfalse");
    Expect(L, "escape", "collectgarbage: count only",
           "print('gc', type(collectgarbage('count')), pcall(collectgarbage, 'stop'), pcall(collectgarbage))",
           D2LUA_OK, "[lua] gc\tnumber\tfalse\t", "not available to mods");
    Expect(L, "escape", "the string metatable is hidden",
           "print('strmt', getmetatable(''), getmetatable('abc'))", D2LUA_OK, "[lua] strmt\tfalse\tfalse");
    Expect(L, "escape", "  and its __index cannot be poisoned",
           "print('poison', pcall(function() getmetatable('').__index.upper = function() return 'pwned' end end))",
           D2LUA_OK, "[lua] poison\tfalse");
    Expect(L, "escape", "  and setmetatable cannot replace it",
           "print('setmt', pcall(setmetatable, '', {}))", D2LUA_OK, "[lua] setmt\tfalse");
    Expect(L, "escape", "clobbering its own globals", "print = nil string = nil _G.pairs = nil", D2LUA_OK);
    {
        lua_getglobal(L, "__host_secret");
        const char* pszSecret = lua_tostring(L, -1);
        int bOk = pszSecret && strcmp(pszSecret, "host-secret") == 0;
        lua_pop(L, 1);
        lua_getglobal(L, "print");
        bOk = bOk && lua_isfunction(L, -1);
        lua_pop(L, 1);
        Check(bOk, "the real global table is untouched");
    }

    // --- one mod cannot change what another sees ---
    Expect(L, "poisoner", "a mod rewrites its string, table, math, bit, coroutine",
           "string.upper = function() return 'pwned' end string.evil = true table.insert = nil "
           "math.floor = function() return 42 end bit.band = nil coroutine.wrap = nil "
           "print('own', string.upper('x'), math.floor(1.5), ('x'):upper())", D2LUA_OK,
           "[lua] own\tpwned\t42\tX");
    Expect(L, "victim", "another mod sees the stock libraries",
           "print('victim', string.upper('abc'), ('abc'):upper(), string.evil, type(table.insert), math.floor(1.5), "
           "type(bit.band), type(coroutine.wrap))", D2LUA_OK, "[lua] victim\tABC\tABC\tnil\tfunction\t1\tfunction\tfunction");
    Expect(L, "victim", "  and print still works for it", "print('victim print')", D2LUA_OK, "[lua] victim print");

    // --- runaway time ---
    char szBudget[96];
    snprintf(szBudget, sizeof(szBudget), "time budget of %u ms", kBudgetMs);
    luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);
    Expect(L, "spin_interp", "while true do end, interpreted (JIT off) is stopped", "while true do end", D2LUA_FAULT,
           "time budget exceeded", szBudget);
    printf("  stopped after %d ms\n", s_nLastMs);
    luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);
    Check(D2LuaHost_IsModFaulted(L, "spin_interp"), "  the mod is faulted");
    Expect(L, "spin_interp", "  and is not run again", "print('ran again')", D2LUA_DISABLED);

    if (bRunaway) {
        InstallTraceCounter(L);
        int nBefore = TracesFinished(L);
        Expect(L, "spin_jit", "while true do end, compiled, is stopped", "while true do end", D2LUA_FAULT,
               "time budget exceeded", szBudget);
        printf("  stopped after %d ms\n", s_nLastMs);
        fflush(stdout);
        int nAfter = TracesFinished(L);
        printf("  traces: %d before, %d after\n", nBefore, nAfter);
        Check(nAfter > nBefore, "  the loop was compiled to a trace");
        Check(s_nLastMs < (int)kBudgetMs * 4, "  and stopped near the budget");

        nBefore = TracesFinished(L);
        Expect(L, "spin_jit2", "a compiled counting loop is stopped",
               "local i = 0 while true do i = i + 1 end", D2LUA_FAULT, szBudget);
        Check(TracesFinished(L) > nBefore, "  the loop was compiled to a trace");

        Expect(L, "spin_pcall", "while true do pcall(function() while true do end end) end is stopped",
               "while true do pcall(function() while true do end end) end", D2LUA_FAULT, szBudget);
        printf("  stopped after %d ms\n", s_nLastMs);
        fflush(stdout);
        Expect(L, "spin_xpcall", "xpcall whose handler also spins is stopped",
               "while true do xpcall(function() while true do end end, function() while true do end end) end",
               D2LUA_FAULT, szBudget);
        Expect(L, "spin_co", "a coroutine that spins, resumed in a loop, is stopped",
               "while true do coroutine.resume(coroutine.create(function() while true do end end)) end",
               D2LUA_FAULT, szBudget);
        Expect(L, "spin_tostring", "an error object whose __tostring spins is stopped",
               "error(setmetatable({}, { __tostring = function() while true do end end }))", D2LUA_FAULT, szBudget);
        Expect(L, "spin_sort", "a table.sort comparator that spins is stopped",
               "table.sort({ 3, 2, 1 }, function(a, b) while true do end end)", D2LUA_FAULT, szBudget);

        Expect(L, "ticker", "a mod defines a handler", "function tick() while true do end end", D2LUA_OK);
        s_szLog[0] = '\0';
        int nTick = D2LuaHost_CallMod(L, "ticker", "tick");
        Check(nTick == D2LUA_FAULT && strstr(s_szLog, szBudget), "  calling the handler that spins faults the mod");
        Check(D2LuaHost_CallMod(L, "ticker", "tick") == D2LUA_DISABLED, "  and it is not called again");
    } else {
        printf("SKIP compiled-loop checks: LuaJIT built without LUAJIT_ENABLE_CHECKHOOK (--runaway runs them)\n");
    }
    Expect(L, "busy", "work inside the budget is not stopped",
           "local s = 0 for i = 1, 2000000 do s = s + i % 7 end print('busy', s)", D2LUA_OK, "[lua] busy\t");

    // --- runaway memory ---
    size_t nBaseline = D2LuaHost_MemoryUsed(L);
    printf("memory in use before the bombs: %lu bytes\n", (unsigned long)nBaseline);
    Expect(L, "bomb_double", "a doubling string bomb faults the mod",
           "local t = {} local s = 'x' while true do s = s .. s t[#t + 1] = s end", D2LUA_FAULT,
           "ran out of memory");
    Expect(L, "bomb_table", "a table bomb faults the mod",
           "local t = {} local i = 0 while true do i = i + 1 t[i] = { i } end", D2LUA_FAULT, "ran out of memory");
    Expect(L, "bomb_rep", "string.rep(x, 2^30) faults the mod at once",
           "local s = string.rep('x', 2^30)", D2LUA_FAULT, "ran out of memory");
    printf("  stopped after %d ms\n", s_nLastMs);
    Expect(L, "bomb_concat", "a huge table.concat faults the mod",
           "local t = {} local s = string.rep('y', 1024 * 1024) for i = 1, 64 do t[i] = s end "
           "local u = table.concat(t)", D2LUA_FAULT, "ran out of memory");
    Expect(L, "bomb_swallow", "an out-of-memory error the mod catches still faults it",
           "local keep = {} for n = 1, 1000 do pcall(function() keep[#keep + 1] = string.rep('z', 1024 * 1024) .. n end) end "
           "print('swallowed', #keep)", D2LUA_FAULT, "ran out of memory");
    size_t nAfter = D2LuaHost_MemoryUsed(L);
    printf("memory in use after the bombs: %lu bytes\n", (unsigned long)nAfter);
    Check(nAfter < nBaseline + (size_t)4 * 1024 * 1024, "the faulted mods' memory is collected");

    // --- the state after all that ---
    Expect(L, "survivor", "a new mod runs after the faults",
           "local s = 0 for i = 1, 1000000 do s = s + i end print('survivor', s, ('ok'):upper())", D2LUA_OK,
           "[lua] survivor\t500000500000\tOK");
    Expect(L, "base", "an earlier mod keeps its globals", "print('persist', base_value + 1)", D2LUA_OK,
           "[lua] persist\t42");
    Expect(L, "victim", "  and its stock libraries", "print('victim', string.upper('abc'))", D2LUA_OK,
           "[lua] victim\tABC");
    if (bRunaway) {
        Expect(L, "survivor", "the budget still applies after a fault", "while true do end", D2LUA_FAULT, szBudget);
    }

    s_szLog[0] = '\0';
    Check(D2LuaHost_RunFile(L, "boot", "no-such-dir/boot.lua", "@no-such") == D2LUA_MISSING && s_szLog[0] == '\0',
          "missing file is silent and returns D2LUA_MISSING");
    Check(D2LuaHost_CallMod(L, "base", "no_such_function") == D2LUA_MISSING, "calling a missing function: D2LUA_MISSING");
    if (pszBootFile) {
        int nResult = D2LuaHost_RunFile(L, "boot", pszBootFile, "@mods/boot.lua");
        printf("run %s -> %d\n", pszBootFile, nResult);
    }

    D2LuaHost_Destroy(L);

    RunModTests(pszRoot, pszTsMods);
    printf("%d passed, %d failed\n", s_nPassed, s_nFailed);
    return s_nFailed ? 1 : 0;
}
#endif
