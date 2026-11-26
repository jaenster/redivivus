// The host test's checks of what the core may never give away and what the native layer promises
// about it, with the core running in the same Lua state as the mods:
//   - escapes: a mod tries every route to a native function of the core's table, an internal table
//     of the core, or another mod's environment (return values, errors and tracebacks, callback
//     arguments, metatables, tostring, the string metatable, module names, hostile values handed
//     in, forged chunk names). After each attempt everything the mod got hold of is walked as the
//     mod could walk it (raw contents, pairs over the core's read-only views, metatables it can
//     read) and compared with the set of forbidden values.
//   - billing: time in the core on a mod's behalf is the mod's; a fault in a callback the core
//     called is the callback's mod's.
//   - interruption: a deadline or the memory limit that passes inside a core critical section is
//     acted on once control is back in mod code, and the section is never cut.
//   - resolution: every declared redivivus:* module resolves where it is declared and nothing
//     else does; redivivus:test and redivivus:automation only with their grants.
#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include <stdio.h>
#include <string.h>

#include <chrono>
#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_core.h"
#include "Redivivus/Lua/d2_lua_internal.h"
#include "Redivivus/Lua/d2_lua_runtime.h"
#include "Redivivus/Lua/test/d2_lua_test_util.h"
#include "Redivivus/Lua/test/d2_lua_test_world.h"
#include "lua.hpp"

typedef std::chrono::steady_clock Clock;

static char s_ForbiddenKey;

static void AddForbidden(lua_State* L, int nSet, int nValue) {
    if (lua_istable(L, nValue) || lua_isfunction(L, nValue) || lua_isuserdata(L, nValue) || lua_isthread(L, nValue)) {
        lua_pushvalue(L, nValue);
        lua_pushboolean(L, 1);
        lua_rawset(L, nSet);
    }
}

// Every table and function the evil mod must never hold: the core's tables (its environment, the
// natives table and every native, its module caches, every internal module's exports and the
// functions in them, the per-mod environments of public modules, the raw exports of every public
// module instance), the registry, the global table, every other mod's environment. The stock
// library functions the core's environment shares with every mod environment are not on it.
static void CollectForbidden(D2LuaRuntime* pRt, const char* pszSelf) {
    lua_State* L = D2LuaRt_State(pRt);
    lua_pushlightuserdata(L, &s_ForbiddenKey);
    lua_newtable(L);
    int nSet = lua_gettop(L);
    D2LuaCore_PushInternals(L);
    int nIn = lua_gettop(L);
    const char* const apszTables[] = { "env", "natives", "cache", "instances", "modEnvs", "host" };
    for (const char* pszName : apszTables) {
        lua_getfield(L, nIn, pszName);
        AddForbidden(L, nSet, -1);
        lua_pop(L, 1);
    }
    // Natives: natives.core.*, natives.testWorld.*; the host module's functions.
    const char* const apszFunctionTables[] = { "natives", "host" };
    for (const char* pszName : apszFunctionTables) {
        lua_getfield(L, nIn, pszName);
        int nT = lua_gettop(L);
        lua_pushnil(L);
        while (lua_next(L, nT)) {
            AddForbidden(L, nSet, -1);
            if (lua_istable(L, -1)) {
                int nInner = lua_gettop(L);
                lua_pushnil(L);
                while (lua_next(L, nInner)) {
                    AddForbidden(L, nSet, -1);
                    lua_pop(L, 1);
                }
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    // The core's modules: every internal module's exports and their functions and tables.
    lua_getfield(L, nIn, "cache");
    int nCache = lua_gettop(L);
    lua_pushnil(L);
    while (lua_next(L, nCache)) {
        // internal.data holds data.raw, the prototype table every mod shares by design.
        if (lua_type(L, -2) == LUA_TSTRING && strncmp(lua_tostring(L, -2), "internal.", 9) == 0 && strcmp(lua_tostring(L, -2), "internal.data") != 0) {
            AddForbidden(L, nSet, -1);
            if (lua_istable(L, -1)) {
                int nInner = lua_gettop(L);
                lua_pushnil(L);
                while (lua_next(L, nInner)) {
                    AddForbidden(L, nSet, -1);
                    lua_pop(L, 1);
                }
            }
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    // Per-mod core environments and the raw exports of every public module instance.
    const char* const apszPerMod[] = { "modEnvs", "instances" };
    for (const char* pszName : apszPerMod) {
        lua_getfield(L, nIn, pszName);
        int nT = lua_gettop(L);
        lua_pushnil(L);
        while (lua_next(L, nT)) {
            AddForbidden(L, nSet, -1);
            if (strcmp(pszName, "instances") == 0 && lua_istable(L, -1)) {
                int nInner = lua_gettop(L);
                lua_pushnil(L);
                while (lua_next(L, nInner)) {
                    AddForbidden(L, nSet, -1);
                    lua_pop(L, 1);
                }
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    lua_pushvalue(L, LUA_REGISTRYINDEX);
    AddForbidden(L, nSet, -1);
    lua_pop(L, 1);
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    AddForbidden(L, nSet, -1);
    lua_pop(L, 1);
    D2LuaRt_PushField(L, "envs");
    int nEnvs = lua_gettop(L);
    lua_pushnil(L);
    while (lua_next(L, nEnvs)) {
        if (strcmp(lua_tostring(L, -2), pszSelf) != 0) {
            AddForbidden(L, nSet, -1);
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

// Walk the value at nIndex as a mod could: raw contents of tables, the values a read-only view
// walks in pairs, metatables getmetatable hands out. Counts forbidden values met.
static int Walk(lua_State* L, int nIndex, int nSet, int nSeen, int nDepth, std::string& sWhere, const std::string& sPath) {
    nIndex = nIndex < 0 ? lua_gettop(L) + nIndex + 1 : nIndex;
    int t = lua_type(L, nIndex);
    if (t != LUA_TTABLE && t != LUA_TFUNCTION && t != LUA_TUSERDATA && t != LUA_TTHREAD) {
        return 0;
    }
    lua_pushvalue(L, nIndex);
    lua_rawget(L, nSeen);
    bool bSeen = !lua_isnil(L, -1);
    lua_pop(L, 1);
    if (bSeen || nDepth > 12) {
        return 0;
    }
    lua_pushvalue(L, nIndex);
    lua_pushboolean(L, 1);
    lua_rawset(L, nSeen);
    lua_pushvalue(L, nIndex);
    lua_rawget(L, nSet);
    int nFound = lua_isnil(L, -1) ? 0 : 1;
    lua_pop(L, 1);
    if (nFound && sWhere.empty()) {
        sWhere = sPath + " (" + lua_typename(L, t) + ")";
    }
    if (t == LUA_TTABLE) {
        luaL_checkstack(L, 8, "walk");
        lua_pushnil(L);
        while (lua_next(L, nIndex)) {
            std::string sKey = lua_type(L, -2) == LUA_TSTRING ? lua_tostring(L, -2) : std::string("[") + luaL_typename(L, -2) + "]";
            nFound += Walk(L, -2, nSet, nSeen, nDepth + 1, sWhere, sPath + "." + sKey + "(key)");
            nFound += Walk(L, -1, nSet, nSeen, nDepth + 1, sWhere, sPath + "." + sKey);
            lua_pop(L, 1);
        }
    }
    if ((t == LUA_TTABLE || t == LUA_TUSERDATA) && lua_getmetatable(L, nIndex)) {
        int nMt = lua_gettop(L);
        lua_getfield(L, nMt, "__metatable");
        bool bLocked = !lua_isnil(L, -1);
        lua_pop(L, 1);
        if (!bLocked) {
            nFound += Walk(L, nMt, nSet, nSeen, nDepth + 1, sWhere, sPath + ".<metatable>");
        }
        lua_pushlightuserdata(L, D2LuaRt_WalkKey());
        lua_rawget(L, nMt);
        if (lua_istable(L, -1)) {
            nFound += Walk(L, -1, nSet, nSeen, nDepth + 1, sWhere, sPath + ".<view>");
        }
        lua_pop(L, 2);
    }
    return nFound;
}

// Run an attempt as the evil mod; it leaves what it got hold of in the global `got` and may set
// `ok_flags`. Then walk `got`. True when it ran and nothing forbidden was reached.
static bool Attempt(D2LuaRuntime* pRt, const char* pszWhat, const char* pszSource, bool bExpectRun = true) {
    lua_State* L = D2LuaRt_State(pRt);
    std::string sSource = std::string("got = {} local got = got ") + pszSource;
    ClearLog();
    bool bRan = TestRun(pRt, 1, "evil", sSource.c_str());
    CollectForbidden(pRt, "evil");
    D2LuaHost_NewModEnv(L, "evil");
    int nEnv = lua_gettop(L);
    lua_pushlightuserdata(L, &s_ForbiddenKey);
    lua_rawget(L, LUA_REGISTRYINDEX);
    int nSet = lua_gettop(L);
    lua_newtable(L);
    int nSeen = lua_gettop(L);
    // The mod's own environment is its to hold; walking it covers everything it stored.
    std::string sWhere;
    int nFound = Walk(L, nEnv, nSet, nSeen, 0, sWhere, "env");
    lua_settop(L, nEnv - 1);
    bool bOk = (bRan || !bExpectRun) && nFound == 0;
    if (!bOk) {
        printf("  ran %d, %d forbidden value(s), first at %s\n", (int)bRan, nFound, sWhere.c_str());
    }
    Check(bOk, pszWhat);
    return bOk;
}

#define LOG " local log = require('redivivus:shared').log "

static void EscapeTests() {
    printf("--- escapes: the core in the mods' state gives nothing away\n");
    TestWorld_Clear();
    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, TestRoots(), { "alpha", "beta", "evil" }, "", { "evil", "alpha" });
    if (!pRt) {
        Check(0, "escapes: runtime");
        return;
    }
    TestStart(pRt, 1);

    Attempt(pRt, "return values: every module, every object, every call's result",
            "for _, m in ipairs({ 'shared', 'server', 'client', 'data', 'enums', 'lualib', 'test' }) do got[m] = require('redivivus:' .. m) end "
            "local S, g = got.shared, games[1] "
            "got.game = g got.reg = g.registry got.ex = g.registry.example got.slot = g.registry.exampleSlot "
            "got.storage = g.storage('k', { a = 1 }) got.task = g.startScript(function() end) "
            "got.ch = g.channel(S.defineChannel('c', { requests = { r = { S.t.int() } }, events = { e = { S.t.bool() } } })) "
            "got.settings = S.setting got.t = S.t got.sch = S.t.record({ x = S.t.int() }) got.list = S.listSettings() "
            "got.data = got.data.data got.world = got.test.world got.unit = got.world.spawn('u', 0, 5) got.probe = got.test.probe "
            "got.call = g.registry.example.call('redivivus:greet', 'x') "
            "got.side = S.side got.name = S.modName got.helper = require('alpha:helper') got.main = require('alpha:main')");
    Attempt(pRt, "error values and tracebacks are strings that name types, never values",
            "local errs = {} local function e(f, ...) local ok, v = pcall(f, ...) errs[#errs + 1] = v end "
            "local g = games[1] local S = require('redivivus:shared') "
            "e(g.on, {}, print) e(g.on, 'tick', {}) e(g.registry.example.call, {}) e(g.registry.example.call, 'none') "
            "e(g.startScript, {}) e(g.storage, {}, {}) e(g.persist, {}, {}) e(S.defineChannel, {}, {}) e(S.defineSettings, { x = {} }) "
            "e(S.wait) e(S.optionalImport, 'nothere:x') e(require, 'redivivus:internal/host') e(g.channel, {}) "
            "for i, v in ipairs(errs) do assert(type(v) == 'string', 'error ' .. i .. ' is a ' .. type(v)) end got.errs = errs "
            "local ok, tb = xpcall(function() g.on(5) end, function(m) return m end) got.tb = tb");
    Attempt(pRt, "callback arguments: next, payloads, game objects",
            "local g = games[1] "
            "g.registry.example.wrap('redivivus:greet', function(next, ...) got.next = next got.args = { ... } return next(...) end) "
            "g.registry.example.call('redivivus:greet', 'cb') "
            "g.on('testUnitDamaged', function(e) got.payload = e end) require('redivivus:test').world.spawn('v', 0, 9):damage(1) "
            "got.nextResult = got.next('again')");
    Attempt(pRt, "metatables of everything the core hands out are locked",
            "local S = require('redivivus:shared') local g = games[1] "
            "for k, v in pairs({ S, require('redivivus:server'), require('redivivus:enums'), require('redivivus:enums').Event, g, g.registry, g.registry.example, S.t, S.setting }) do "
            "  local mt = getmetatable(v) assert(mt == false, 'metatable of ' .. tostring(k) .. ' is ' .. tostring(mt)) "
            "  assert(not pcall(setmetatable, v, {}), 'setmetatable worked on ' .. tostring(k)) "
            "  got[#got + 1] = mt end");
    Attempt(pRt, "tostring of core objects is a string and leads nowhere",
            "local g = games[1] for _, v in ipairs({ g, g.registry, g.on, require('redivivus:shared').log }) do got[#got + 1] = tostring(v) end "
            "assert(type(got[1]) == 'string')");
    Attempt(pRt, "the string metatable stays locked; the core's string functions are its own",
            "assert(getmetatable('') == false) local s = string s.format = function() return 'pwned' end "
            "got.r = games[1].registry.example.call('redivivus:greet', 'str') assert(got.r == 'hello, str', got.r) "
            "local ok = pcall(function() getmetatable('').__index.upper = nil end) assert(not ok)");
    Attempt(pRt, "no module name reaches the core's modules or its natives",
            "for _, n in ipairs({ 'redivivus:internal/host', 'redivivus:internal.host', 'redivivus.internal.host', 'internal.host', "
            "'redivivus:native', 'redivivus:__native', 'redivivus:engine-enums', 'redivivus:ui/signal', 'redivivus:internal/native', "
            "'lualib_bundle', 'redivivus:../alpha/main', 'redivivus:./server', 'beta:main', 'evil:../alpha/main' }) do "
            "  local ok, v = pcall(require, n) got[#got + 1] = v assert(not ok or n == 'redivivus:./server', 'resolved: ' .. n) end");
    Attempt(pRt, "hostile values handed in: no metamethod of the mod runs inside the core",
            "ran = {} local function trap(name) return function() ran[#ran + 1] = name while true do end end end "
            "local evilmt = { __index = trap('__index'), __newindex = trap('__newindex'), __call = trap('__call'), __len = trap('__len'), "
            "  __eq = trap('__eq'), __lt = trap('__lt'), __le = trap('__le'), __concat = trap('__concat'), __tostring = trap('__tostring'), "
            "  __unm = trap('__unm'), __add = trap('__add') } "
            "local function h(t) return setmetatable(t or {}, evilmt) end "
            "local S = require('redivivus:shared') local g = games[1] local D = require('redivivus:data').data "
            "pcall(g.storage, 'h1', h({ x = h() })) pcall(g.persist, require('redivivus:test').world.spawn('p', 0, 3), h({ y = h() })) "
            "pcall(S.defineChannel, 'hc', h({ requests = h({ m = h({ S.t.int() }) }) })) pcall(S.defineSettings, h({ a = S.setting.bool(h({ scope = 'startup', default = true })) })) "
            "pcall(S.t.record, h({ f = S.t.int() })) pcall(S.t.oneOf, h({ 'a', 'b' })) pcall(D.extend, h({ h({ type = 'x', name = 'y' }) })) "
            "pcall(g.on, h(), print) pcall(g.on, 'tick', h()) pcall(g.registry.example.call, h()) pcall(g.registry.example.register, h(), print) "
            "pcall(g.registry.example.register, 'x:y', h()) pcall(g.startScript, h()) pcall(g.startScript, function() end, h()) "
            "local ch = g.channel(S.defineChannel('hc2', { requests = { r = { S.t.record({ a = S.t.int() }), S.t.list(S.t.int(), 3) } } })) "
            "pcall(ch.implement, h({ r = print })) "
            "pcall(require('redivivus:shared').log, 'plain') "
            "got.ran = ran assert(#ran == 0, 'ran inside the core: ' .. table.concat(ran, ','))");
    Attempt(pRt, "hostile coroutines and callables are refused by type",
            "local g = games[1] local co = coroutine.create(function() end) local callable = setmetatable({}, { __call = function() end }) "
            "for i, f in ipairs({ function() return g.on('tick', co) end, function() return g.on('tick', callable) end, "
            "  function() return g.startScript(co) end, function() return g.registry.example.wrap('redivivus:greet', callable) end }) do "
            "  local ok, e = pcall(f) assert(not ok, 'accepted ' .. i) got[i] = e end");
    Attempt(pRt, "the core's chunk names cannot be forged by load (a forged name is still stopped)",
            "local f = load('return 1', '=[core] internal.host') got.f = f local ok, info = pcall(function() return debug end) got.d = info");
    {
        Clock::time_point tStart = Clock::now();
        ClearLog();
        int nResult = D2LuaRt_Run(pRt, 1, "beta", "local f = load('while true do end', '=[core] internal.host') f()");
        int nMs = (int)std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - tStart).count();
        Check(nResult == D2LUA_FAULT && nMs < (int)kTestBudgetMs * 4 && Logged("mod 'beta' ran past"),
              "  a mod's chunk named like the core's is stopped at the budget like any other");
    }
    Attempt(pRt, "another mod's module runs in the requirer's environment; nothing of the other mod's own reaches it",
            "local h = require('alpha:helper') got.h = h assert(h.loaded_by == 'evil', 'helper instance of ' .. tostring(h.loaded_by)) "
            "got.getfenv = getfenv got.setfenv = setfenv assert(getfenv == nil and setfenv == nil)");
    Attempt(pRt, "a handle method with a forged self is refused",
            "local u = require('redivivus:test').world.spawn('m', 0, 5) local damage = u.damage "
            "local ok, e = pcall(damage, setmetatable({}, getmetatable(u) or {}), 1) got.e = e assert(not ok)");
    Attempt(pRt, "a core function registered as a handler runs as the mod and gives nothing",
            "local g = games[1] g.on('tick', require('redivivus:shared').log) got.done = true");
    D2LuaRt_Tick(pRt, 1);
    Check(!D2LuaRt_IsFaulted(pRt, "evil"), "  (and the tick runs it without a fault)");
    Attempt(pRt, "handlers that error, yield or never return are contained",
            "local g = games[1] g.on('testUnitDamaged', function() error({ table = 'error object' }) end) "
            "local w = require('redivivus:test').world.spawn('z', 0, 9) w:damage(1) "
            "g.registry.example.register('evil:yield', function() coroutine.yield(1) end) "
            "local ok, e = pcall(g.registry.example.call, 'evil:yield') got.e = e assert(not ok and tostring(e):find('yield across'), tostring(e))");
    D2LuaRt_Destroy(pRt);
}

static void BillingTests() {
    printf("--- billing and fault attribution with the core in Lua\n");
    TestWorld_Clear();
    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, TestRoots(), { "alpha", "beta", "gamma", "evil" }, "", { "evil", "alpha", "beta", "gamma" });
    TestStart(pRt, 1);
    // An event raised by a native that alpha called: gamma's handler spins; gamma is charged.
    TestRun(pRt, 1, "gamma", "games[1].on('testUnitDamaged', function() while true do end end)");
    ClearLog();
    TestRun(pRt, 1, "alpha", "require('redivivus:test').world.spawn('b', 0, 50):damage(1)");
    Check(D2LuaRt_IsFaulted(pRt, "gamma") && !D2LuaRt_IsFaulted(pRt, "alpha"),
          "a handler the core calls for an event alpha's call raised: the handler's mod is faulted, not alpha");
    CheckLogged("(in a call that entered through mod 'alpha', which stays enabled)", "  the log names who was charged and who entered");
    // Time in the core on evil's behalf is evil's.
    ClearLog();
    TestRun(pRt, 1, "evil", "require('redivivus:test').probe.criticalSection(" "400" ")");
    Check(D2LuaRt_IsFaulted(pRt, "evil") && Logged("mod 'evil' ran past its time budget") && !D2LuaRt_IsFaulted(pRt, "beta"),
          "core work done for a mod (a 400 ms core section, budget 250) is billed to that mod");
    D2LuaRt_Destroy(pRt);
}

static void InterruptionTests() {
    printf("--- interruption: a deadline inside the core waits for mod code\n");
    TestWorld_Clear();
    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, TestRoots(), { "alpha", "beta", "evil" }, "", { "evil", "alpha", "beta" });
    TestStart(pRt, 1);
    ClearLog();
    Clock::time_point tStart = Clock::now();
    int nResult = D2LuaRt_Run(pRt, 1, "evil",
                              "local P = require('redivivus:test').probe local spins = P.criticalSection(400) "
                              "after_section = 'mod code ran after the section'");
    int nMs = (int)std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - tStart).count();
    printf("  the entry took %d ms (budget %u, section 400)\n", nMs, kTestBudgetMs);
    Check(nResult == D2LUA_FAULT && nMs >= 400, "the budget ran out inside a core section; the section ran to its end (not cut at 250 ms)");
    TestExpect(pRt, 1, "alpha", "local P = require('redivivus:test').probe require('redivivus:shared').log('core state:', P.consistent(), P.sections())",
               "core state: true 1", "  the core's state is consistent: both steps of the section are done");
    Check(D2LuaRt_IsFaulted(pRt, "evil"), "  the mod is faulted, at its next instruction");
    TestExpect(pRt, 1, "alpha", "require('redivivus:shared').log('evil assignment:', after_section)", "evil assignment: nil",
               "  (alpha's view; evil's own next statement never ran)");

    // The section is the whole of the call: the deadline passes in the core, which returns to the
    // host without any mod code in between. The entry still ends as the mod's fault.
    TestRun(pRt, 1, "beta", "games[1].registry.example.register('beta:section', require('redivivus:test').probe.criticalSection)");
    ClearLog();
    lua_State* L = D2LuaRt_State(pRt);
    lua_pushnumber(L, 400);
    D2LuaRt_CallRegistry(pRt, 1, "example", "beta:section", 1);
    lua_settop(L, 0);
    Check(D2LuaRt_IsFaulted(pRt, "beta") && Logged("mod 'beta' ran past"), "a deadline that passed inside the core faults the owner when the entry ends in the core");
    TestExpect(pRt, 1, "alpha", "local P = require('redivivus:test').probe require('redivivus:shared').log('core state 2:', P.consistent(), P.sections())",
               "core state 2: true 2", "  and the core's state is consistent");

    // Memory: over the limit inside a core section; the section completes, then the mod is faulted.
    ClearLog();
    int nMem = D2LuaRt_Run(pRt, 1, "alpha", "local n = require('redivivus:test').probe.allocate(70 * 1024) alloc_done = n");
    Check(nMem == D2LUA_FAULT && Logged("mod 'alpha' ran out of memory"), "the memory limit passed inside a core section faults the mod after it");
    D2LuaRt_Destroy(pRt);
}

static void ResolutionTests() {
    printf("--- resolution: declared redivivus modules, grants\n");
    static const int s_anSides[] = { D2LUA_SIDE_DATA, D2LUA_SIDE_SERVER, D2LUA_SIDE_CLIENT };
    int nDeclared = 0, nOk = 0, nValuesOk = 0, nValues = 0;
    std::string sBad;
    for (int nSide : s_anSides) {
        D2LuaRuntime* pRt = TestMakeRuntime(nSide, TestRoots(), { "alpha", "gamma" }, "", { "alpha" }, { "alpha" });
        for (const D2LuaCoreModule* p = g_aD2LuaCoreModules; p->pszEntry; p++) {
            char szSource[800];
            snprintf(szSource, sizeof(szSource), "local m = require('redivivus:%s') resolved = true", p->pszEntry);
            // No mod can hold the manager grant: only the built-in redivivus mod imports it.
            bool bShould = (p->nSides & nSide) != 0 && !(p->nGrant & D2LUA_GRANT_MANAGER);
            TestRun(pRt, 0, "alpha", "resolved = false");
            bool bRan = TestRun(pRt, 0, "alpha", szSource);
            nDeclared++;
            if (bRan == bShould) {
                nOk++;
            } else {
                sBad += std::string(" ") + p->pszEntry + "@" + std::to_string(nSide);
            }
            if (bRan) {
                for (const char* const* pp = p->apszValues; *pp; pp++) {
                    snprintf(szSource, sizeof(szSource), "assert(require('redivivus:%s')['%s'] ~= nil)", p->pszEntry, *pp);
                    nValues++;
                    if (TestRun(pRt, 0, "alpha", szSource)) {
                        nValuesOk++;
                    } else {
                        sBad += std::string(" ") + p->pszEntry + "." + *pp;
                    }
                }
            }
        }
        D2LuaRt_Destroy(pRt);
    }
    {
        D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_CLIENT, TestRoots(), { "alpha" }, "", { "alpha" }, { "alpha" });
        ClearLog();
        bool bRan = TestRun(pRt, 0, "alpha", "require('redivivus:manager')");
        Check(!bRan && Logged("only the game's own redivivus mod can import redivivus:manager"),
              "a mod cannot import redivivus:manager, whatever grants it holds");
        D2LuaRt_Destroy(pRt);
    }
    printf("  %d module/state pairs, %d values%s%s\n", nDeclared, nValues, sBad.empty() ? "" : "; wrong:", sBad.c_str());
    Check(nOk == nDeclared, "every declared redivivus module resolves exactly in the states it is declared for");
    Check(nValues > 40 && nValuesOk == nValues, "  and every value its declarations export exists at run time");
    int nNotEverywhere = 0;
    for (const D2LuaCoreModule* p = g_aD2LuaCoreModules; p->pszEntry; p++) {
        if (!p->nGrant && p->nSides != (D2LUA_SIDE_DATA | D2LUA_SIDE_SERVER | D2LUA_SIDE_CLIENT)) {
            nNotEverywhere++;
        }
    }
    Check(nNotEverywhere == 0, "  every redivivus module a mod may import is declared for every state (only grants limit one)");

    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, TestRoots(), { "alpha", "gamma" }, "", { "alpha" });
    TestExpect(pRt, 0, "gamma", "local ok, e = pcall(require, 'redivivus:test') require('redivivus:shared').log('test ungranted:', e)",
               "only a test the test runner starts can import redivivus:test; a mod cannot", "redivivus:test does not resolve for a mod without the runner's grant");
    TestExpect(pRt, 0, "alpha", "local ok, e = pcall(require, 'redivivus:automation') require('redivivus:shared').log('automation:', e)",
               "only a driver script started from the command line can import redivivus:automation", "redivivus:automation does not resolve for a mod");
    {
        // Every redivivus module resolves in every state, the UI kit too. What loading it costs a
        // server state that never draws: measured with a full collection on either side.
        lua_State* L = D2LuaRt_State(pRt);
        lua_gc(L, LUA_GCCOLLECT, 0);
        int nBefore = lua_gc(L, LUA_GCCOUNT, 0) * 1024 + lua_gc(L, LUA_GCCOUNTB, 0);
        TestExpect(pRt, 0, "alpha",
                   "local ui = require('redivivus:ui') local ok = pcall(function() ui.Recorder = nil end) "
                   "require('redivivus:shared').log('ui server:', type(ui.mount), ui.Direction.Column, ok)",
                   "ui server: function column false", "redivivus:ui resolves in the server state (ui-mod mounts a panel there)");
        lua_gc(L, LUA_GCCOLLECT, 0);
        int nAfter = lua_gc(L, LUA_GCCOUNT, 0) * 1024 + lua_gc(L, LUA_GCCOUNTB, 0);
        printf("  [info] a server state's memory: %d KiB before the kit, %d KiB with it loaded (+%d KiB)\n",
               nBefore / 1024, nAfter / 1024, (nAfter - nBefore) / 1024);
        Check(nAfter - nBefore < 4 * 1024 * 1024, "  loading the kit in the server state costs under 4 MiB");
    }
    TestExpect(pRt, 0, "alpha", "local ok, e = pcall(require, 'redivivus:nothing') require('redivivus:shared').log('nothing:', e)",
               "redivivus has no module 'nothing'", "an undeclared module does not resolve, and the error lists what does");
    TestExpect(pRt, 0, "alpha", "local ok, e = pcall(require, 'mod:alpha/helper') require('redivivus:shared').log('old form:', e)",
               "mod 'alpha' does not depend on 'mod'", "the dropped mod:<name>/<path> form is just an unknown mod");
    TestExpect(pRt, 0, "alpha", "require('redivivus:shared').log('case:', require('ALPHA:Helper') == require('helper'))", "case: true",
               "module paths are case-insensitive, one instance whatever the case");
    D2LuaRt_Destroy(pRt);
    pRt = TestMakeRuntime(D2LUA_SIDE_CLIENT, TestRoots(), { "alpha" }, "", {}, { "alpha" });
    TestExpect(pRt, 0, "alpha", "require('redivivus:shared').log('driver:', require('redivivus:automation').isDriver)", "driver: true",
               "a driver script (granted) resolves redivivus:automation in the client state");
    TestExpect(pRt, 0, "alpha",
               "local ui = require('redivivus:ui') local ok, e = pcall(function() ui.Recorder = nil end) "
               "require('redivivus:shared').log('ui client:', type(ui.mount), ui.Direction.Column, ok)",
               "ui client: function column false", "redivivus:ui resolves in the client state: read-only, with its enums as tables");
    D2LuaRt_Destroy(pRt);
}

void RunEscapeTests(const char* pszRoot) {
    (void)pszRoot;
    EscapeTests();
    BillingTests();
    InterruptionTests();
    ResolutionTests();
}
#endif
