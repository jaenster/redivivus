// The host test's checks of the kit (Redivivus/Lua/kit, compiled into mods/base): settings,
// channels over the loopback transport, the script scheduler and persisted state, all used from
// plain Lua (the fixture mod kitty and chunks run as a mod). The TypeScript side is checked by the
// type tests and by the example mods in d2_lua_mods_test.cpp.
#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_internal.h"
#include "Redivivus/Lua/d2_lua_runtime.h"
#include "Redivivus/Lua/test/d2_lua_test_util.h"
#include "Redivivus/Lua/test/d2_lua_test_world.h"
#include "lua.hpp"

static std::string s_sRoot;

static std::vector<std::string> Roots() {
    return { s_sRoot + "/Redivivus/Lua/mods", s_sRoot + "/Redivivus/Lua/test/mods" };
}

static std::string SettingsFile() {
    return s_sRoot + "/Redivivus/Lua/test/settings.json";
}

static bool Run(D2LuaRuntime* pRt, const char* pszMod, const char* pszSource) {
    return D2LuaHost_RunString(D2LuaRt_State(pRt), pszMod, pszSource, "=test") == D2LUA_OK;
}

static void ExpectRun(D2LuaRuntime* pRt, const char* pszMod, const char* pszSource, const char* pszLogged, const char* pszWhat) {
    ClearLog();
    bool bRan = Run(pRt, pszMod, pszSource);
    bool bOk = bRan && (!pszLogged || Logged(pszLogged));
    if (!bOk) {
        printf("  ran: %d, wanted in log: %s\n", (int)bRan, pszLogged ? pszLogged : "-");
    }
    Check(bOk, pszWhat);
}

static void CheckLogged(const char* pszText, const char* pszWhat) {
    if (!Logged(pszText)) {
        printf("  wanted in log: %s\n", pszText);
    }
    Check(Logged(pszText), pszWhat);
}

static const D2LuaSettingInfo* FindSetting(D2LuaRuntime* pRt, const char* pszName) {
    for (const D2LuaSettingInfo& t : D2LuaRt_Settings(pRt)) {
        if (t.sMod == "kitty" && t.sName == pszName) {
            return &t;
        }
    }
    return nullptr;
}

// --- settings -------------------------------------------------------------------------------------

static void SettingsTests() {
    printf("--- kit: settings\n");
    ClearLog();
    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_DATA, Roots(), { "base", "kitty" }, SettingsFile());
    Check(pRt != nullptr, "data stage with the settings file");
    if (!pRt) {
        return;
    }
    CheckLogged("[kitty] data stage settings: 4 0.5 settings file", "the data stage reads startup settings, file values over defaults");
    CheckLogged("setting 'rate' in the settings file: 7 is outside 0..1; the default is used", "  a value outside the range is refused, logged");
    const std::vector<D2LuaSettingInfo>& a = D2LuaRt_Settings(pRt);
    Check(a.size() == 5, "the host lists every declared setting");
    std::string sOrder;
    for (const D2LuaSettingInfo& t : a) {
        sOrder += t.sName + " ";
    }
    Check(sOrder == "boost hud mode motto rate ", "  in name order");
    const D2LuaSettingInfo* pBoost = FindSetting(pRt, "boost");
    Check(pBoost && pBoost->sType == "int" && pBoost->sScope == "startup" && pBoost->bHasMin && pBoost->nMin == 0 && pBoost->nMax == 5
              && pBoost->tDefault.nValue == 1 && pBoost->tValue.nValue == 4 && pBoost->sTitle == "Boost",
          "  with type, scope, range, default, value and title");
    const D2LuaSettingInfo* pMode = FindSetting(pRt, "mode");
    Check(pMode && pMode->aChoices.size() == 2 && pMode->aChoices[1] == "hard" && pMode->tValue.sValue == "hard",
          "  a choice setting's choices and its value from the file");
    const D2LuaSettingInfo* pRate = FindSetting(pRt, "rate");
    Check(pRate && pRate->tValue.nValue == 0.5 && pRate->sProblem.find("outside") != std::string::npos,
          "  the refused value's problem is kept for the settings screen");
    ExpectRun(pRt, "kitty", "local ok, e = pcall(function() return require('shared.settings').mode end) log('data reads game setting:', e)",
              "setting 'mode' is a game setting; it is read on the server", "a game setting is not read in the data stage");
    ExpectRun(pRt, "kitty", "local ok, e = pcall(function() require('shared.settings').boost = 2 end) log('write:', e)",
              "settings are read-only", "the settings object is read-only");
    ExpectRun(pRt, "kitty", "local ok, e = pcall(function() return require('shared.settings').nope end) log('unknown:', e)",
              "no setting 'nope' in this set", "an undeclared setting raises");
    ExpectRun(pRt, "kitty", "local ok, e = pcall(mod_settings.declare, 'boost', { type = SettingType.Int, scope = SettingScope.Startup, default = 1 }) log('twice:', e)",
              "setting 'boost' is declared twice", "a setting is declared once");
    ExpectRun(pRt, "kitty", "local ok, e = pcall(mod_settings.declare, 'x', { type = 'int', scope = 'startup', default = 9, max = 5 }) log('bad default:', e)",
              "bad default: 9 is outside", "a default outside its range is refused");
    ExpectRun(pRt, "kitty", "local ok, e = pcall(mod_settings.declare, 'y', { type = 'choice', scope = 'game', default = 'c', choices = { 'a', 'b' } }) log('bad choice:', e)",
              "'c' is not one of the choices", "a choice default must be a choice");
    ExpectRun(pRt, "kitty", "local ok, e = pcall(mod_settings.declare, 'z', { type = 'int', scope = 'forever', default = 1 }) log('bad scope:', e)",
              "has no known scope", "an unknown scope is refused");
    ExpectRun(pRt, "kitty", "local ok, e = pcall(mod_settings.declare, 'w', { type = 'bool', scope = 'startup', default = 1 }) log('bad type:', e)",
              "the default is a number, not a bool value", "a default of the wrong type is refused");
    ExpectRun(pRt, "kitty", "local l = mod_settings.list() log('lua list:', #l, l[1].name, l[1].value, l[1].mod)", "lua list: 5 boost 4 kitty",
              "mod_settings.list gives the same to Lua");
    D2LuaRt_Destroy(pRt);

    pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, Roots(), { "base", "kitty" }, SettingsFile());
    if (pRt) {
        ExpectRun(pRt, "kitty", "local s = require('shared.settings') log('server reads:', s.boost, s.mode, s.motto)",
                  "server reads: 4 hard settings file", "the server reads startup and game settings");
        ExpectRun(pRt, "kitty", "local ok, e = pcall(function() return require('shared.settings').hud end) log('server hud:', e)",
                  "setting 'hud' is a player setting; it is read on the client", "  not player settings");
        D2LuaRt_Destroy(pRt);
    }
    pRt = TestMakeRuntime(D2LUA_SIDE_CLIENT, Roots(), { "base", "kitty" }, std::string());
    if (pRt) {
        ExpectRun(pRt, "kitty", "local s = require('shared.settings') log('client reads:', s.hud, s.boost, pcall(function() return s.mode end))",
                  "client reads: true 1 false", "the client reads player and startup settings (defaults without a file), not game ones");
        D2LuaRt_Destroy(pRt);
    }
}

// --- channels ---------------------------------------------------------------------------------------

static void ChannelTests() {
    printf("--- kit: channels (loopback)\n");
    TestWorld_Clear();
    ClearLog();
    D2LuaRuntime* pServer = TestMakeRuntime(D2LUA_SIDE_SERVER, Roots(), { "base", "kitty" }, std::string());
    D2LuaRuntime* pClient7 = TestMakeRuntime(D2LUA_SIDE_CLIENT, Roots(), { "base", "kitty" }, std::string());
    D2LuaRuntime* pClient8 = TestMakeRuntime(D2LUA_SIDE_CLIENT, Roots(), { "base", "kitty" }, std::string());
    Check(pServer && pClient7 && pClient8, "a server and two clients over kitty");
    if (!pServer || !pClient7 || !pClient8) {
        return;
    }
    std::string sError;
    D2LuaRt_Start(pServer, nullptr, sError);
    D2LuaRt_Start(pClient7, nullptr, sError);
    D2LuaRt_Start(pClient8, nullptr, sError);
    ExpectRun(pClient7, "kitty", "local ok, e = pcall(script.send, 'x') log('unconnected:', e)", "mod messages are not connected",
              "without a transport sending is a clean error");
    D2LuaLoopback* pLoop = D2LuaLoopback_Create(pServer);
    D2LuaLoopback_AddClient(pLoop, pClient7, 7);
    D2LuaLoopback_AddClient(pLoop, pClient8, 8);

    ExpectRun(pClient7, "kitty", "require('shared.chat').call.say('hello', 2)", nullptr, "a client calls a typed request");
    ClearLog();
    int nOne = D2LuaLoopback_Pump(pLoop);
    CheckLogged("[kitty] server got say 7 hello 2", "  the server's implementation runs with the sender's peer and the arguments");
    CheckLogged("[kitty] client said 7 hello", "  its event reaches the clients");
    const char* pSaid = strstr(g_szTestLog, "client said 7 hello");
    Check(nOne == 3 && pSaid && strstr(pSaid + 1, "client said 7 hello"), "  one request, then the event broadcast to both clients, in one pump");
    ExpectRun(pClient8, "kitty", "require('shared.chat').call.move({ x = -3, y = 70000 }, 'run') require('shared.chat').call.move({ x = 1, y = 2 })",
              nullptr, "records, enum choices and optionals encode");
    ClearLog();
    D2LuaLoopback_Pump(pLoop);
    CheckLogged("[kitty] server got move 8 -3 70000 run", "  and decode with negative and large ints and the choice");
    CheckLogged("[kitty] server got move 8 1 2 nil", "  an optional left out arrives as nil");
    ExpectRun(pServer, "kitty", "require('shared.chat').emit.tags(8, { 'a', 'bb', 'ccc' }, true, 2.75)", nullptr, "the server emits to one client");
    ClearLog();
    D2LuaLoopback_Pump(pLoop);
    CheckLogged("[kitty] client tags 3 a ccc true 2.75", "  lists, booleans and numbers arrive");
    const char* pFirst = strstr(g_szTestLog, "client tags");
    Check(pFirst && !strstr(pFirst + 1, "client tags"), "  at that client only");

    // What a sender gets wrong is its own error.
    ExpectRun(pClient7, "kitty", "local ok, e = pcall(require('shared.chat').call.say, 5, 1) log('encode:', e)",
              "chat.say argument 1: a string of at most 20 bytes expected", "a wrong argument raises in the sender");
    ExpectRun(pClient7, "kitty", "local ok, e = pcall(require('shared.chat').call.say, 'x', 9) log('range:', e)",
              "a whole number from 0 to 3 expected", "  as does one out of range");
    ExpectRun(pServer, "kitty", "local ok, e = pcall(require('shared.chat').call.say, 'x', 1) log('server calls:', e)",
              "is for the client side", "requests are sent from the client only");
    ExpectRun(pClient7, "kitty", "local ok, e = pcall(require('shared.chat').emit.said, 0, 1, 'x') log('client emits:', e)",
              "is for the server side", "events are emitted from the server only");

    // What a client sends is not trusted: forged payloads are dropped, their handlers never run.
    struct Forged {
        const char* pszLua;
        const char* pszWhy;
        const char* pszWhat;
    } aForged[] = {
        { "string.char(1, 1, 4) .. 'chat' .. string.char(1, 30) .. string.rep('x', 30) .. string.char(2)", "a string of 30 bytes, at most 20",
          "a string longer than declared" },
        { "string.char(1, 1, 4) .. 'chat' .. string.char(1, 1) .. 'x' .. string.char(18)", "9 is outside 0..3", "an int out of range" },
        { "string.char(1, 1, 4) .. 'chat' .. string.char(1, 1) .. 'x' .. string.char(2, 0)", "bytes after the last argument",
          "trailing bytes" },
        { "string.char(1, 1, 4) .. 'chat' .. string.char(1, 1) .. 'x'", "the message ends early", "a message cut short" },
        { "string.char(1, 2, 4) .. 'chat' .. string.char(0)", "only requests reach the server", "an event sent to the server" },
        { "string.char(1, 1, 4) .. 'chaz' .. string.char(0)", "no channel 'chaz'", "an unknown channel" },
        { "string.char(1, 1, 4) .. 'chat' .. string.char(5)", "has no message 5", "an unknown message" },
        { "string.char(9, 1)", "unknown channel format", "another format version" },
        { "string.char(1, 1, 4) .. 'chat' .. string.char(0, 0, 0, 1, 5)", "not one of the choices", "a choice index past the choices" },
    };
    for (const Forged& f : aForged) {
        std::string sSend = std::string("script.send(") + f.pszLua + ")";
        Run(pClient7, "kitty", sSend.c_str());
        ClearLog();
        D2LuaLoopback_Pump(pLoop);
        std::string sWhat = std::string("forged: ") + f.pszWhat + " is dropped";
        Check(Logged("channel message from peer 7 dropped") && Logged(f.pszWhy) && !Logged("server got"), sWhat.c_str());
    }

    // The envelope is checked before any mod sees it.
    ClearLog();
    Check(!D2LuaRt_Deliver(pServer, 7, "\x02\x00\x00", 3) && Logged("unknown format version"), "an envelope of another version is dropped");
    ClearLog();
    Check(!D2LuaRt_Deliver(pServer, 7, "\x01\x63\x00", 3) && Logged("no such mod"), "  as is one naming a mod that is not loaded");
    std::string sBig(4096 + 4, 'x');
    sBig[0] = 1;
    sBig[1] = 1;
    sBig[2] = 0;
    ClearLog();
    Check(!D2LuaRt_Deliver(pServer, 7, sBig.data(), sBig.size()) && Logged("bad size"), "  and one past the size limit");
    ExpectRun(pClient7, "kitty", "local ok, e = pcall(script.send, string.rep('x', 4097)) log('too big:', e)", "a message carries at most 4096",
              "a mod cannot send past the limit");

    // A handler that runs out of time faults the receiving mod, on its side only.
    Run(pServer, "kitty", "require('shared.chat').implement({ say = function() while true do end end, move = function() end })");
    Run(pClient7, "kitty", "require('shared.chat').call.say('spin', 0)");
    ClearLog();
    D2LuaLoopback_Pump(pLoop);
    Check(D2LuaRt_IsFaulted(pServer, "kitty") && !D2LuaRt_IsFaulted(pClient7, "kitty") && Logged("mod 'kitty' ran past"),
          "the time budget applies on receipt: a spinning handler faults the receiving mod");
    D2LuaLoopback_Destroy(pLoop);
    D2LuaRt_Destroy(pClient8);
    D2LuaRt_Destroy(pClient7);
    D2LuaRt_Destroy(pServer);
}

// --- scripts and persisted state ----------------------------------------------------------------

static void SchedulerTests() {
    printf("--- kit: scripts\n");
    TestWorld_Clear();
    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, Roots(), { "base", "alpha", "beta", "kitty" }, std::string());
    if (!pRt) {
        Check(0, "kit scripts: runtime");
        return;
    }
    std::string sError;
    D2LuaRt_Start(pRt, nullptr, sError);
    Run(pRt, "kitty",
        "K = require('__base__/server/kit/scripts') "
        "task = K.startScript(function() log('ks start', script.tick) K.wait() log('ks wait', script.tick) "
        "  log('ks waitTicks', K.waitTicks(3)) "
        "  flag = false "
        "  log('ks until done', K.waitUntil(function() return flag end, 10) == Outcome.Done, script.tick) "
        "  log('ks until timeout', K.waitUntil(function() return false end, 2) == Outcome.Timeout, script.tick) "
        "  log('ks until now', K.waitUntil(function() return true end, 0) == Outcome.Done, script.tick) "
        "end) log('ks id', type(task.id), task.isRunning())");
    Check(Logged("ks id number true"), "startScript gives a task with an id, running");
    ClearLog();
    long long nBase = D2LuaRt_CurrentTick(pRt);
    for (int i = 0; i < 5; i++) {
        D2LuaRt_Tick(pRt);
    }
    char szWant[96];
    snprintf(szWant, sizeof(szWant), "ks start %lld", nBase + 1);
    CheckLogged(szWant, "  it first runs on the next tick");
    snprintf(szWant, sizeof(szWant), "ks wait %lld", nBase + 2);
    CheckLogged(szWant, "  wait() is one tick");
    snprintf(szWant, sizeof(szWant), "ks waitTicks %lld", nBase + 5);
    CheckLogged(szWant, "  waitTicks(3) resumes three ticks later with the tick");
    Run(pRt, "kitty", "flag = true");
    ClearLog();
    for (int i = 0; i < 4; i++) {
        D2LuaRt_Tick(pRt);
    }
    snprintf(szWant, sizeof(szWant), "ks until done true %lld", nBase + 6);
    CheckLogged(szWant, "  waitUntil gives Done on the tick the predicate holds");
    snprintf(szWant, sizeof(szWant), "ks until timeout true %lld", nBase + 8);
    CheckLogged(szWant, "  and Timeout once its ticks run out");
    snprintf(szWant, sizeof(szWant), "ks until now true %lld", nBase + 8);
    CheckLogged(szWant, "  a predicate that already holds does not wait");
    ExpectRun(pRt, "kitty", "log('ks finished:', task.isRunning(), task.cancel())", "ks finished: false false", "a finished task is not running");

    // cancellation
    Run(pRt, "kitty", "t2 = K.startScript(function() while true do K.wait() log('t2 ran') end end)");
    D2LuaRt_Tick(pRt);
    ClearLog();
    D2LuaRt_Tick(pRt);
    Check(Logged("t2 ran"), "a looping script runs each tick");
    ExpectRun(pRt, "kitty", "log('cancel:', t2.cancel(), t2.isRunning(), t2.cancel())", "cancel: true false false", "cancel stops it, once");
    ClearLog();
    D2LuaRt_Tick(pRt);
    D2LuaRt_Tick(pRt);
    Check(!Logged("t2 ran"), "  and it is not resumed again");
    Run(pRt, "kitty", "t3 = K.startScript(function() while true do K.wait() end end) remote.add_interface('kitty_ids', { t3 = function() return t3.id end })");
    ExpectRun(pRt, "alpha", "local id = remote.call('kitty_ids', 't3') log('foreign cancel:', script.cancel(id), script.running(id))",
              "foreign cancel: false false", "a mod cannot cancel another mod's script");
    Run(pRt, "kitty", "t3.cancel()");

    // owner handles
    Run(pRt, "kitty",
        "owner = test_world.spawn('owner', TestUnitKind.Monster, 5) "
        "t4 = K.startScript(function() while true do K.wait() log('t4 alive') end end, owner) "
        "t5 = K.startScript(function() script.wait_event(Event.TestUnitDamaged) log('t5 woke') end, owner)");
    D2LuaRt_Tick(pRt);
    ClearLog();
    D2LuaRt_Tick(pRt);
    Check(Logged("t4 alive"), "a script with an owner runs while the owner exists");
    ExpectRun(pRt, "kitty", "owner:damage(100) log('owner dead:', owner.valid)", "owner dead: false", "the owner dies");
    ClearLog();
    D2LuaRt_Tick(pRt);
    D2LuaRt_Tick(pRt);
    Check(!Logged("t4 alive"), "  its script is dropped");
    ExpectRun(pRt, "kitty", "log('owned tasks:', t4.isRunning(), t5.isRunning())", "owned tasks: false false",
              "  a script waiting for an event too, without being resumed");
    Check(!Logged("t5 woke"), "  (the waiting script never woke)");
    ExpectRun(pRt, "kitty", "local ok, e = pcall(K.startScript, function() end, {}) log('bad owner:', e)", "a handle expected as the owner",
              "an owner must be a handle");
    ExpectRun(pRt, "kitty", "local ok, e = pcall(K.startScript, function() end, owner) log('dead owner:', e)", "is not valid",
              "  and a valid one");

    // deterministic order: across mods, in start order
    Run(pRt, "beta", "local K = require('__base__/server/kit/scripts') K.startScript(function() K.wait() log('order beta-1') end)");
    Run(pRt, "kitty", "K.startScript(function() K.wait() log('order kitty-2') end)");
    Run(pRt, "alpha", "local K = require('__base__/server/kit/scripts') K.startScript(function() K.wait() log('order alpha-3') end)");
    ClearLog();
    D2LuaRt_Tick(pRt);
    D2LuaRt_Tick(pRt);
    const char* p1 = strstr(g_szTestLog, "order beta-1");
    const char* p2 = strstr(g_szTestLog, "order kitty-2");
    const char* p3 = strstr(g_szTestLog, "order alpha-3");
    Check(p1 && p2 && p3 && p1 < p2 && p2 < p3, "due scripts resume in the order they were started, whatever the mod");
    ExpectRun(pRt, "kitty", "local ok, e = pcall(K.waitTicks, 1) log('outside:', e)", "can only be used inside a script",
              "a kit wait outside a script is a clean error");
    D2LuaRt_Destroy(pRt);
}

static void PersistTests() {
    printf("--- kit: persisted state\n");
    TestWorld_Clear();
    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, Roots(), { "base", "kitty" }, std::string());
    if (!pRt) {
        Check(0, "kit persist: runtime");
        return;
    }
    std::string sError;
    D2LuaRt_Start(pRt, nullptr, sError);
    ExpectRun(pRt, "kitty",
              "P = require('__base__/server/kit/persist') "
              "u1 = test_world.spawn('keeper', TestUnitKind.Npc, 50) u2 = test_world.spawn('victim', TestUnitKind.Monster, 5) "
              "local d = { charges = 0, seen = { first = 1 } } "
              "local s = P.persist(u1, d) s.charges = s.charges + 2 s.seen.first = 9 "
              "log('persist:', P.persist(u1, d).charges, d.charges, d.seen.first, P.persist(u1, d) == s)",
              "persist: 2 0 1 true", "persist gives one table per owner, a copy of the defaults");
    ExpectRun(pRt, "kitty", "P.persist(u2, { n = 1 }) log('owners:', P.hasPersisted(u1), P.hasPersisted(u2), #storage.__kit_persist)",
              "owners: true true 2", "  kept in the mod's storage");
    ExpectRun(pRt, "kitty", "local ok, e = pcall(P.persist, {}, {}) log('bad owner:', e)", "the owner must be a valid handle",
              "the owner must be a handle");
    Check(D2LuaRt_Reload(pRt, sError), "reload with persisted state");
    ExpectRun(pRt, "kitty",
              "P = require('__base__/server/kit/persist') u1 = test_world.get_unit(1) "
              "log('after reload:', P.persist(u1, { charges = 0 }).charges, P.persist(u1, {}).seen.first)",
              "after reload: 2 9", "  it survives a reload (save and restore of storage)");
    ExpectRun(pRt, "kitty", "test_world.get_unit(2):damage(100) log('victim:', test_world.get_unit(2))", "victim: nil", "an owner dies");
    D2LuaRt_Tick(pRt);
    D2LuaRt_Tick(pRt);
    ExpectRun(pRt, "kitty", "log('after death:', #storage.__kit_persist, P.hasPersisted(test_world.get_unit(1)))", "after death: 1 true",
              "  its state is removed within a tick; the living owner's stays");
    D2LuaRt_Destroy(pRt);
}

void RunKitTests(const char* pszRoot) {
    s_sRoot = pszRoot;
    SettingsTests();
    ChannelTests();
    SchedulerTests();
    PersistTests();
}
#endif
