// The host test's checks of what the core offers beyond events and registries: typed settings,
// channels over the loopback transport, scripts with their waits and tasks, and persisted state,
// all used from plain Lua (the fixture mod kitty and chunks run as a mod). The TypeScript side is
// checked by the type tests and by the example mods in d2_lua_mods_test.cpp.
#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_internal.h"
#include "Redivivus/Lua/d2_lua_runtime.h"
#include "Redivivus/Lua/test/d2_lua_test_util.h"
#include "Redivivus/Lua/test/d2_lua_test_world.h"
#include "lua.hpp"

static std::string SettingsFile() {
    return g_sTestRoot + "/Redivivus/Lua/test/settings.json";
}

static const D2LuaSettingInfo* FindSetting(D2LuaRuntime* pRt, const char* pszName) {
    for (const D2LuaSettingInfo& t : D2LuaRt_Settings(pRt)) {
        if (t.sMod == "kitty" && t.sName == pszName) {
            return &t;
        }
    }
    return nullptr;
}

#define LOG "local log = require('redivivus:shared').log "

static void SettingsTests() {
    printf("--- settings\n");
    ClearLog();
    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_DATA, TestRoots(), { "kitty" }, SettingsFile());
    Check(pRt != nullptr, "data stage with the settings file");
    if (!pRt) {
        return;
    }
    CheckLogged("[kitty] data stage settings: 4 0.5 settings file", "the data stage reads startup settings, file values over defaults");
    CheckLogged("setting 'rate' in the settings file: 7 is outside 0..1; the default is used", "  a value outside the range is refused, logged");
    const std::vector<D2LuaSettingInfo>& a = D2LuaRt_Settings(pRt);
    std::string sOrder;
    for (const D2LuaSettingInfo& t : a) {
        sOrder += t.sName + " ";
    }
    Check(a.size() == 5 && sOrder == "boost hud mode motto rate ", "the host lists every declared setting, in name order");
    const D2LuaSettingInfo* pBoost = FindSetting(pRt, "boost");
    Check(pBoost && pBoost->sType == "int" && pBoost->sScope == "startup" && pBoost->bHasMin && pBoost->nMin == 0 && pBoost->nMax == 5
              && pBoost->tDefault.nValue == 1 && pBoost->tValue.nValue == 4 && pBoost->sTitle == "Boost",
          "  with type, scope, range, default, value and title");
    const D2LuaSettingInfo* pMode = FindSetting(pRt, "mode");
    Check(pMode && pMode->aChoices.size() == 2 && pMode->aChoices[1] == "hard" && pMode->tValue.sValue == "hard", "  a choice setting's choices and its value from the file");
    TestExpect(pRt, 0, "kitty", LOG "local ok, e = pcall(function() return require('settingsdef').mode end) log('data reads game setting:', e)",
               "setting 'mode' is a game setting; it is read on the server", "a game setting is not read in the data stage");
    TestExpect(pRt, 0, "kitty", LOG "local ok, e = pcall(function() require('settingsdef').boost = 2 end) log('write:', e)", "settings are read-only",
               "the settings object is read-only");
    TestExpect(pRt, 0, "kitty", LOG "local ok, e = pcall(function() return require('settingsdef').nope end) log('unknown:', e)", "no setting 'nope' in this set",
               "an undeclared setting raises");
    TestExpect(pRt, 0, "kitty",
               LOG "local S = require('redivivus:shared') local E = require('redivivus:enums') "
               "local ok, e = pcall(S.defineSettings, { boost = S.setting.int({ scope = E.SettingScope.Startup, default = 1 }) }) log('twice:', e)",
               "setting 'boost' is declared twice", "a setting is declared once");
    TestExpect(pRt, 0, "kitty",
               LOG "local S = require('redivivus:shared') local ok, e = pcall(S.defineSettings, { x = S.setting.int({ scope = 'startup', default = 9, max = 5 }) }) log('bad default:', e)",
               "bad default: 9 is outside", "a default outside its range is refused");
    TestExpect(pRt, 0, "kitty",
               LOG "local S = require('redivivus:shared') local ok, e = pcall(S.defineSettings, { y = S.setting.choice({ scope = 'game', default = 'c', choices = { 'a', 'b' } }) }) log('bad choice:', e)",
               "'c' is not one of the choices", "a choice default must be a choice");
    TestExpect(pRt, 0, "kitty",
               LOG "local S = require('redivivus:shared') local ok, e = pcall(S.defineSettings, { z = { declaration = { type = 'int', scope = 'startup', default = 1 } } }) log('forged:', e)",
               "'z' is not a setting made with setting.*", "a declaration the core did not make is refused");
    TestExpect(pRt, 0, "kitty", LOG "local l = require('redivivus:shared').listSettings() log('lua list:', #l, l[1].name, l[1].value, l[1].mod)",
               "lua list: 5 boost 4 kitty", "listSettings gives the same to Lua");
    D2LuaRt_Destroy(pRt);

    pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, TestRoots(), { "kitty" }, SettingsFile());
    if (pRt) {
        TestExpect(pRt, 0, "kitty", LOG "local s = require('settingsdef') log('server reads:', s.boost, s.mode, s.motto)", "server reads: 4 hard settings file",
                   "the server reads startup and game settings");
        TestExpect(pRt, 0, "kitty", LOG "local ok, e = pcall(function() return require('settingsdef').hud end) log('server hud:', e)",
                   "setting 'hud' is a player setting; it is read on the client", "  not player settings");
        D2LuaRt_Destroy(pRt);
    }
    pRt = TestMakeRuntime(D2LUA_SIDE_CLIENT, TestRoots(), { "kitty" }, std::string());
    if (pRt) {
        TestExpect(pRt, 0, "kitty", LOG "local s = require('settingsdef') log('client reads:', s.hud, s.boost, pcall(function() return s.mode end))",
                   "client reads: true 1 false", "the client reads player and startup settings, not game ones");
        D2LuaRt_Destroy(pRt);
    }
}

static void ChannelTests() {
    printf("--- channels (loopback)\n");
    TestWorld_Clear();
    ClearLog();
    D2LuaRuntime* pServer = TestMakeRuntime(D2LUA_SIDE_SERVER, TestRoots(), { "kitty" }, std::string());
    D2LuaRuntime* pClient7 = TestMakeRuntime(D2LUA_SIDE_CLIENT, TestRoots(), { "kitty" }, std::string());
    D2LuaRuntime* pClient8 = TestMakeRuntime(D2LUA_SIDE_CLIENT, TestRoots(), { "kitty" }, std::string());
    Check(pServer && pClient7 && pClient8, "a server and two clients over kitty");
    if (!pServer || !pClient7 || !pClient8) {
        return;
    }
    TestStart(pServer, 1);
    TestStart(pClient7, 1);
    TestStart(pClient8, 1);
    TestExpect(pClient7, 1, "kitty", LOG "local ok, e = pcall(channels[1].call.say, 'x', 1) log('unconnected:', e)", "mod messages are not connected",
               "without a transport sending is a clean error");
    D2LuaLoopback* pLoop = D2LuaLoopback_Create(pServer);
    D2LuaLoopback_AddClient(pLoop, pClient7, 7);
    D2LuaLoopback_AddClient(pLoop, pClient8, 8);

    TestExpect(pClient7, 1, "kitty", "channels[1].call.say('hello', 2)", nullptr, "a client calls a typed request");
    ClearLog();
    int nOne = D2LuaLoopback_Pump(pLoop);
    CheckLogged("[kitty] server got say 1 7 hello 2", "  the server's implementation runs, in that game, with the sender's peer and the arguments");
    const char* pSaid = strstr(g_szTestLog, "client said 7 hello");
    Check(nOne == 3 && pSaid && strstr(pSaid + 1, "client said 7 hello"), "  its event reaches both clients in the same pump");
    TestExpect(pClient8, 1, "kitty", "channels[1].call.move({ x = -3, y = 70000 }, 'run') channels[1].call.move({ x = 1, y = 2 })", nullptr,
               "records, enum choices and optionals encode");
    ClearLog();
    D2LuaLoopback_Pump(pLoop);
    CheckLogged("[kitty] server got move 8 -3 70000 run", "  and decode with negative and large ints and the choice");
    CheckLogged("[kitty] server got move 8 1 2 nil", "  an optional left out arrives as nil");
    TestExpect(pServer, 1, "kitty", "channels[1].emit.tags(8, { 'a', 'bb', 'ccc' }, true, 2.75)", nullptr, "the server emits to one client");
    ClearLog();
    D2LuaLoopback_Pump(pLoop);
    CheckLogged("[kitty] client tags 3 a ccc true 2.75", "  lists, booleans and numbers arrive");
    const char* pFirst = strstr(g_szTestLog, "client tags");
    Check(pFirst && !strstr(pFirst + 1, "client tags"), "  at that client only");
    TestExpect(pClient7, 1, "kitty", LOG "local ok, e = pcall(channels[1].call.say, 5, 1) log('encode:', e)", "chat.say argument 1: a string of at most 20 bytes expected",
               "a wrong argument raises in the sender");
    TestExpect(pClient7, 1, "kitty", LOG "local ok, e = pcall(channels[1].call.say, 'x', 9) log('range:', e)", "a whole number from 0 to 3 expected", "  as does one out of range");
    TestExpect(pClient7, 1, "kitty", LOG "local ok, e = pcall(clients[1].channel, require('chat')) log('twice:', e)", "already open in this game",
               "a channel is opened once per game");

    // A forged payload is dropped before any handler runs: the envelope and the channel format.
    struct Forged {
        const char* pBytes;
        size_t nSize;
        const char* pszWhy;
        const char* pszWhat;
    } aForged[] = {
        { "\x01\x01\x00\x01\x01\x04" "chat" "\x01\x1e" "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" "\x02", 6 + 4 + 2 + 30 + 1, "a string of 30 bytes, at most 20", "a string longer than declared" },
        { "\x01\x01\x00\x01\x01\x04" "chat" "\x01\x01" "x" "\x12", 6 + 4 + 2 + 1 + 1, "9 is outside 0..3", "an int out of range" },
        { "\x01\x01\x00\x01\x01\x04" "chat" "\x01\x01" "x" "\x02\x00", 6 + 4 + 2 + 1 + 2, "bytes after the last argument", "trailing bytes" },
        { "\x01\x01\x00\x01\x01\x04" "chat" "\x01\x01" "x", 6 + 4 + 2 + 1, "the message ends early", "a message cut short" },
        { "\x01\x01\x00\x01\x02\x04" "chat" "\x00", 6 + 4 + 1, "only requests reach the server", "an event sent to the server" },
        { "\x01\x01\x00\x01\x01\x04" "chaz" "\x00", 6 + 4 + 1, "no channel 'chaz'", "an unknown channel" },
        { "\x01\x01\x00\x01\x01\x04" "chat" "\x05", 6 + 4 + 1, "has no message 5", "an unknown message" },
        { "\x01\x01\x00\x09\x01", 5, "unknown channel format", "another format version" },
    };
    for (const Forged& f : aForged) {
        ClearLog();
        D2LuaRt_Deliver(pServer, 1, 7, f.pBytes, f.nSize);
        std::string sWhat = std::string("forged: ") + f.pszWhat + " is dropped";
        Check(Logged("channel message from peer 7 dropped") && Logged(f.pszWhy) && !Logged("server got"), sWhat.c_str());
    }
    ClearLog();
    Check(!D2LuaRt_Deliver(pServer, 1, 7, "\x02\x00\x00", 3) && Logged("unknown format version"), "an envelope of another version is dropped");
    ClearLog();
    Check(!D2LuaRt_Deliver(pServer, 1, 7, "\x01\x63\x00", 3) && Logged("no such mod"), "  as is one naming a mod that is not loaded");
    ClearLog();
    Check(!D2LuaRt_Deliver(pServer, 1, 7, "\x01\x00\x00\x01", 4), "  and one for redivivus itself");
    ClearLog();
    Check(!D2LuaRt_Deliver(pServer, 2, 7, "\x01\x01\x00\x01\x01\x04" "chat" "\x00", 11) && Logged("opened no channel in this game"),
          "  and one for a game the mod has no channel in");
    std::string sBig(4096 + 4, 'x');
    sBig[0] = 1;
    sBig[1] = 1;
    sBig[2] = 0;
    ClearLog();
    Check(!D2LuaRt_Deliver(pServer, 1, 7, sBig.data(), sBig.size()) && Logged("bad size"), "  and one past the size limit");

    // A handler that runs out of time faults the receiving mod, on its side only.
    TestStart(pServer, 2);
    TestRun(pServer, 2, "kitty", "local ch = channels[2] ch.implement({ say = function() while true do end end, move = function() end })");
    ClearLog();
    D2LuaRt_Deliver(pServer, 2, 7, "\x01\x01\x00\x01\x01\x04" "chat" "\x01\x01" "x" "\x00", 14);
    Check(D2LuaRt_IsFaulted(pServer, "kitty") && !D2LuaRt_IsFaulted(pClient7, "kitty") && Logged("mod 'kitty' ran past"),
          "the time budget applies on receipt: a spinning handler faults the receiving mod");
    D2LuaLoopback_Destroy(pLoop);
    D2LuaRt_Destroy(pClient8);
    D2LuaRt_Destroy(pClient7);
    D2LuaRt_Destroy(pServer);
}

static void SchedulerTests() {
    printf("--- scripts: tasks and waits\n");
    TestWorld_Clear();
    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, TestRoots(), { "alpha", "beta", "kitty" }, std::string(), { "kitty" });
    if (!pRt) {
        Check(0, "scripts: runtime");
        return;
    }
    TestStart(pRt, 1);
    TestRun(pRt, 1, "kitty",
            "S = require('redivivus:shared') E = require('redivivus:enums') g = games[1] "
            "task = g.startScript(function() S.log('ks start', g.tick) S.wait() S.log('ks wait', g.tick) "
            "  S.log('ks wait3', S.wait(3)) "
            "  flag = false "
            "  S.log('ks until done', S.waitUntil(function() return flag end, 10) == E.Outcome.Done, g.tick) "
            "  S.log('ks until timeout', S.waitUntil(function() return false end, 2) == E.Outcome.Timeout, g.tick) "
            "  S.log('ks until now', S.waitUntil(function() return true end, 0) == E.Outcome.Done, g.tick) "
            "end) S.log('ks id', type(task.id), task.isRunning())");
    Check(Logged("ks id number true"), "startScript gives a task with an id, running");
    ClearLog();
    for (int i = 0; i < 5; i++) {
        D2LuaRt_Tick(pRt, 1);
    }
    CheckLogged("ks start 1", "  it first runs on the next tick");
    CheckLogged("ks wait 2", "  wait() is one tick");
    CheckLogged("ks wait3 5", "  wait(3) resumes three ticks later with the tick");
    TestRun(pRt, 1, "kitty", "flag = true");
    ClearLog();
    for (int i = 0; i < 4; i++) {
        D2LuaRt_Tick(pRt, 1);
    }
    CheckLogged("ks until done true 6", "  waitUntil gives Done on the tick the predicate holds");
    CheckLogged("ks until timeout true 8", "  and Timeout once its ticks run out");
    CheckLogged("ks until now true 8", "  a predicate that already holds does not wait");
    TestExpect(pRt, 1, "kitty", "S.log('ks finished:', task.isRunning(), task.cancel())", "ks finished: false false", "a finished task is not running");
    TestRun(pRt, 1, "kitty", "t2 = g.startScript(function() while true do S.wait() S.log('t2 ran') end end)");
    D2LuaRt_Tick(pRt, 1);
    ClearLog();
    D2LuaRt_Tick(pRt, 1);
    Check(Logged("t2 ran"), "a looping script runs each tick");
    TestExpect(pRt, 1, "kitty", "S.log('cancel:', t2.cancel(), t2.isRunning(), t2.cancel())", "cancel: true false false", "cancel stops it, once");
    ClearLog();
    D2LuaRt_Tick(pRt, 1);
    D2LuaRt_Tick(pRt, 1);
    Check(!Logged("t2 ran"), "  and it is not resumed again");
    TestRun(pRt, 1, "kitty",
            "local W = require('redivivus:test').world owner = W.spawn('owner', 0, 5) "
            "t4 = g.startScript(function() while true do S.wait() S.log('t4 alive') end end, owner) "
            "t5 = g.startScript(function() g.waitEvent('testUnitDamaged') S.log('t5 woke') end, owner)");
    D2LuaRt_Tick(pRt, 1);
    ClearLog();
    D2LuaRt_Tick(pRt, 1);
    Check(Logged("t4 alive"), "a script with an owner runs while the owner exists");
    TestExpect(pRt, 1, "kitty", "owner:damage(100) S.log('owner dead:', owner.valid)", "owner dead: false", "the owner dies");
    ClearLog();
    D2LuaRt_Tick(pRt, 1);
    D2LuaRt_Tick(pRt, 1);
    Check(!Logged("t4 alive"), "  its script is dropped");
    TestExpect(pRt, 1, "kitty", "S.log('owned tasks:', t4.isRunning(), t5.isRunning())", "owned tasks: false false", "  a script waiting for an event too");
    Check(!Logged("t5 woke"), "  (the waiting script never woke)");
    TestExpect(pRt, 1, "kitty", "local ok, e = pcall(g.startScript, function() end, {}) S.log('bad owner:', e)", "a handle expected as the owner", "an owner must be a handle");
    TestExpect(pRt, 1, "kitty", "local ok, e = pcall(g.startScript, function() end, owner) S.log('dead owner:', e)", "is not valid", "  and a valid one");
    TestRun(pRt, 1, "beta", "local S = require('redivivus:shared') games[1].startScript(function() S.wait() S.log('order beta-1') end)");
    TestRun(pRt, 1, "kitty", "g.startScript(function() S.wait() S.log('order kitty-2') end)");
    TestRun(pRt, 1, "alpha", "local S = require('redivivus:shared') games[1].startScript(function() S.wait() S.log('order alpha-3') end)");
    ClearLog();
    D2LuaRt_Tick(pRt, 1);
    D2LuaRt_Tick(pRt, 1);
    const char* p1 = strstr(g_szTestLog, "order beta-1");
    const char* p2 = strstr(g_szTestLog, "order kitty-2");
    const char* p3 = strstr(g_szTestLog, "order alpha-3");
    Check(p1 && p2 && p3 && p1 < p2 && p2 < p3, "due scripts resume in the order they were started, whatever the mod");
    D2LuaRt_Destroy(pRt);
}

// Count the keys of the mod global pszTable after a full collection.
static int WeakCount(D2LuaRuntime* pRt, const char* pszTable) {
    lua_State* L = D2LuaRt_State(pRt);
    lua_gc(L, LUA_GCCOLLECT, 0);
    lua_gc(L, LUA_GCCOLLECT, 0);
    ClearLog();
    std::string sChunk = std::string("local n = 0 for _ in pairs(") + pszTable + ") do n = n + 1 end S.log('weak count', n)";
    TestRun(pRt, 1, "kitty", sChunk.c_str());
    const char* p = strstr(g_szTestLog, "weak count ");
    return p ? atoi(p + strlen("weak count ")) : -1;
}

static void GuardTests() {
    printf("--- scripts: guard\n");
    TestWorld_Clear();
    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, TestRoots(), { "kitty" }, std::string(), { "kitty" });
    if (!pRt) {
        Check(0, "guard: runtime");
        return;
    }
    TestStart(pRt, 1);
    TestRun(pRt, 1, "kitty",
            "S = require('redivivus:shared') E = require('redivivus:enums') g = games[1] "
            "function steps(name, n) return function() for i = 1, n do S.log(name, 'step', i, g.tick) S.wait() end return name .. ' done' end end");

    // The check before the first step: the body never starts.
    ClearLog();
    TestRun(pRt, 1, "kitty",
            "g.startScript(function() local o = S.guard(function() return false end, function() S.log('g0 ran') end) "
            "  S.log('g0 outcome', o == E.Outcome.Interrupted, g.tick) end)");
    D2LuaRt_Tick(pRt, 1);
    Check(!Logged("g0 ran") && Logged("g0 outcome true 1"), "guard: a check false at the start: the body never runs, Interrupted at once");

    // A check that holds: the body's own result, its waits are the script's.
    ClearLog();
    TestRun(pRt, 1, "kitty",
            "g.startScript(function() local o = S.guard(function() return true end, steps('g1', 2)) S.log('g1 outcome', o, g.tick) end)");
    for (int i = 0; i < 4; i++) {
        D2LuaRt_Tick(pRt, 1);
    }
    Check(Logged("g1 step 1 2") && Logged("g1 step 2 3") && Logged("g1 outcome g1 done 4"), "  a check that holds: the body runs to its end and its result is the outcome");

    // The check fails mid-way: the body is not resumed again.
    ClearLog();
    TestRun(pRt, 1, "kitty",
            "checks = 0 "
            "g.startScript(function() local o = S.guard(function() checks = checks + 1 return checks <= 2 end, steps('g2', 5)) "
            "  S.log('g2 outcome', o == E.Outcome.Interrupted, checks) end)");
    for (int i = 0; i < 6; i++) {
        D2LuaRt_Tick(pRt, 1);
    }
    Check(Logged("g2 step 2") && !Logged("g2 step 3") && Logged("g2 outcome true 3"),
          "  a check that fails mid-way: checked before each step, the body dropped where it waited, Interrupted");

    // What the script is resumed with reaches the body; what the body waits for, the script waits for.
    ClearLog();
    TestRun(pRt, 1, "kitty",
            "g.startScript(function() local o = S.guard(function() return true end, function() "
            "    local t0 = g.tick S.log('g3 wait3', S.wait(3) == t0 + 3) local p = g.waitEvent('testUnitDamaged') S.log('g3 event', type(p), p.name) "
            "    S.log('g3 until', S.waitUntil(function() return true end, 2) == E.Outcome.Done) return 42 end) "
            "  S.log('g3 outcome', o) end)");
    for (int i = 0; i < 6; i++) {
        D2LuaRt_Tick(pRt, 1);
    }
    TestRun(pRt, 1, "kitty", "local W = require('redivivus:test').world local u = W.spawn('g3', 0, 50) u:damage(1)");
    D2LuaRt_Tick(pRt, 1);
    Check(Logged("g3 wait3 true") && Logged("g3 event table testUnitDamaged") && Logged("g3 until true") && Logged("g3 outcome 42"),
          "  sent values pass through: wait's tick, an event's payload, waitUntil; the body's return value comes out");

    // Nested guards: an inner check failing ends the inner body only; an outer one ends both.
    ClearLog();
    TestRun(pRt, 1, "kitty",
            "inner, outer = true, true "
            "g.startScript(function() local o = S.guard(function() return outer end, function() "
            "    local i = S.guard(function() return inner end, steps('g4in', 10)) S.log('g4 inner', i) "
            "    local j = S.guard(function() return true end, steps('g4b', 10)) S.log('g4 never', j) end) "
            "  S.log('g4 outer', o) end)");
    for (int i = 0; i < 3; i++) {
        D2LuaRt_Tick(pRt, 1);
    }
    TestRun(pRt, 1, "kitty", "inner = false");
    for (int i = 0; i < 3; i++) {
        D2LuaRt_Tick(pRt, 1);
    }
    TestRun(pRt, 1, "kitty", "outer = false");
    for (int i = 0; i < 3; i++) {
        D2LuaRt_Tick(pRt, 1);
    }
    Check(Logged("g4in step 2") && !Logged("g4in step 4") && Logged("g4 inner 1") && Logged("g4b step 2") && !Logged("g4 never")
              && Logged("g4 outer 1"),
          "  guards nest: the inner check ends the inner body, the outer check ends both");

    // Errors propagate as they were raised: from the check, from the body, the same value.
    ClearLog();
    TestRun(pRt, 1, "kitty",
            "boom = {} "
            "g.startScript(function() "
            "  local ok, e = pcall(S.guard, function() error(boom) end, steps('g5', 3)) S.log('g5 check', ok, e == boom) "
            "  ok, e = pcall(S.guard, function() return true end, function() S.wait() error(boom) end) S.log('g5 body', ok, e == boom) "
            "  ok, e = pcall(S.guard, function() return true end, function() S.wait() error('g5 says no') end) S.log('g5 text', ok, e) "
            "  S.guard(function() error('g5 uncaught') end, steps('g5', 1)) S.log('g5 after') end)");
    for (int i = 0; i < 4; i++) {
        D2LuaRt_Tick(pRt, 1);
    }
    Check(Logged("g5 check false true") && !Logged("g5 step"), "  an error in the check is raised in the script as it was");
    Check(Logged("g5 body false true"), "  and one in the body, the same value, across the body's waits");
    Check(Logged("g5 text false") && Logged("g5 says no"), "  a message keeps its source position");
    Check(Logged("g5 uncaught") && !Logged("g5 after") && !D2LuaRt_IsFaulted(pRt, "kitty"), "  uncaught, it ends the script as any error does; the mod stays enabled");

    // Outside a script.
    TestExpect(pRt, 1, "kitty", "local ok, e = pcall(S.guard, function() return true end, function() end) S.log('g6', e)",
               "guard can only be used inside a script", "  guard outside a script is an error");
    TestExpect(pRt, 1, "kitty", "local co = coroutine.create(function() return S.guard(function() return true end, function() end) end) "
               "S.log('g6b', select(2, coroutine.resume(co)))",
               "guard can only be used inside a script", "  as in a coroutine of the mod's own");
    TestExpect(pRt, 1, "kitty", "local ok, e = pcall(S.guard, function() return true end, 5) S.log('g6c', e)",
               "bad argument #2 to 'guard'", "  the body must be a function (what the build hands over)");

    // No leaks: many bodies dropped mid-way, and a script that ends while guarded.
    TestRun(pRt, 1, "kitty",
            "bodies = setmetatable({}, { __mode = 'k' }) "
            "g.startScript(function() for i = 1, 300 do local n = 0 "
            "    S.guard(function() n = n + 1 return n < 3 end, function() bodies[coroutine.running()] = true while true do S.wait(0) end end) end "
            "  S.log('g7 dropped 300') end)");
    for (int i = 0; i < 700; i++) {
        D2LuaRt_Tick(pRt, 1);
    }
    CheckLogged("g7 dropped 300", "  300 bodies dropped mid-way");
    Check(WeakCount(pRt, "bodies") == 0, "  none of them is left after a collection");
    TestRun(pRt, 1, "kitty",
            "ended = setmetatable({}, { __mode = 'k' }) "
            "t8 = g.startScript(function() ended[coroutine.running()] = true "
            "  S.guard(function() return true end, function() ended[coroutine.running()] = true "
            "    S.guard(function() return true end, function() ended[coroutine.running()] = true while true do S.wait() end end) end) end)");
    D2LuaRt_Tick(pRt, 1);
    D2LuaRt_Tick(pRt, 1);
    Check(WeakCount(pRt, "ended") == 3, "  a script inside two guards: three coroutines while it runs");
    TestRun(pRt, 1, "kitty", "t8.cancel()");
    D2LuaRt_Tick(pRt, 1);
    Check(WeakCount(pRt, "ended") == 0, "  none once it is cancelled");

    // The budget is the script's: a guarded body that spins faults the mod.
    TestRun(pRt, 1, "kitty", "g.startScript(function() S.guard(function() return true end, function() while true do end end) end)");
    ClearLog();
    D2LuaRt_Tick(pRt, 1);
    Check(D2LuaRt_IsFaulted(pRt, "kitty") && Logged("mod 'kitty' ran past"), "  a body that spins runs out of the script's budget and faults the mod");
    D2LuaRt_Destroy(pRt);
}

static void PersistTests() {
    printf("--- persisted state\n");
    TestWorld_Clear();
    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, TestRoots(), { "kitty" }, std::string(), { "kitty" });
    if (!pRt) {
        Check(0, "persist: runtime");
        return;
    }
    TestStart(pRt, 1);
    TestExpect(pRt, 1, "kitty",
               "S = require('redivivus:shared') W = require('redivivus:test').world g = games[1] "
               "u1 = W.spawn('keeper', 1, 50) u2 = W.spawn('victim', 0, 5) "
               "local d = { charges = 0, seen = { first = 1 } } "
               "local s = g.persist(u1, d) s.charges = s.charges + 2 s.seen.first = 9 "
               "S.log('persist:', g.persist(u1, d).charges, d.charges, d.seen.first, g.persist(u1, d) == s)",
               "persist: 2 0 1 true", "persist gives one table per owner, a copy of the defaults");
    TestExpect(pRt, 1, "kitty", "g.persist(u2, { n = 1 }) S.log('owners:', g.hasPersisted(u1), g.hasPersisted(u2))", "owners: true true", "  for every owner");
    TestExpect(pRt, 1, "kitty", "local ok, e = pcall(g.persist, {}, {}) S.log('bad owner:', e)", "the owner must be a valid handle", "the owner must be a handle");
    std::string sError;
    Check(D2LuaRt_ReloadGame(pRt, 1, sError), "reload the game with persisted state");
    TestExpect(pRt, 1, "kitty", "S = require('redivivus:shared') g = games[1] S.log('after reload:', g.persist(u1, { charges = 0 }).charges, g.persist(u1, {}).seen.first)",
               "after reload: 2 9", "  it survives a reload (save and restore of storage)");
    TestExpect(pRt, 1, "kitty", "u2:damage(100) S.log('victim:', u2.valid)", "victim: false", "an owner dies");
    D2LuaRt_Tick(pRt, 1);
    D2LuaRt_Tick(pRt, 1);
    TestExpect(pRt, 1, "kitty", "S.log('after death:', g.hasPersisted(u1))", "after death: true", "  the living owner's state stays");
    D2LuaRt_Destroy(pRt);
}

void RunKitTests(const char* pszRoot) {
    (void)pszRoot;
    SettingsTests();
    ChannelTests();
    SchedulerTests();
    GuardTests();
    PersistTests();
}
#endif
