// The host test's checks of function overrides (d2_lua_override.h) on the fixture's functions
// (test/override/): the registry, patching and restoring entries, next() and chains in dependency
// order, both tiers, faults, sides and threads, refusals, and that a hot override runs compiled.
// On a target that cannot patch (not x86) the fixture's thunks are called directly, which is what a
// patched entry jumps to; the byte checks are skipped there.
#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include <stdio.h>
#include <string.h>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "Redivivus/Lua/d2_lua_override.h"
#include "Redivivus/Lua/d2_lua_runtime.h"
#include "Redivivus/Lua/test/d2_lua_test_util.h"
#include "Redivivus/Lua/test/override/d2_lua_override_fixture.h"
#include "lua.hpp"

typedef std::chrono::steady_clock Clock;

namespace {

std::vector<std::string> Roots() {
    return { g_sTestRoot + "/Redivivus/Lua/test/override-mods" };
}

// How the fixture is called: through its entry where entries are patched, else through the thunk.
template <class P>
P Via(P pDirect, const char* pszPath) {
    if (D2LuaOv_CanPatch()) {
        return pDirect;
    }
    const D2LuaOvEntry* e = D2LuaOv_Find(pszPath);
    return e && e->pThunk ? (P)e->pThunk : pDirect;
}
#define FX(NS, NAME) Via(&NS::NAME, #NS "." #NAME)

bool EntryIs(const char* pszPath, const uint8_t* pExpect, size_t n, int nOffset) {
    const D2LuaOvEntry* e = D2LuaOv_Find(pszPath);
    return e && memcmp((const uint8_t*)e->pEntry + nOffset, pExpect, n) == 0;
}

const uint8_t kNops[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };

struct Bound {
    D2LuaRuntime* pWasServer;
    D2LuaRuntime* pWasClient;
    Bound(D2LuaRuntime* pServer, D2LuaRuntime* pClient)
        : pWasServer(D2LuaOv_BindThread(D2LUA_SIDE_SERVER, pServer)), pWasClient(D2LuaOv_BindThread(D2LUA_SIDE_CLIENT, pClient)) {}
    ~Bound() {
        D2LuaOv_BindThread(D2LUA_SIDE_SERVER, pWasServer);
        D2LuaOv_BindThread(D2LUA_SIDE_CLIENT, pWasClient);
    }
};

D2LuaRuntime* Make(int nSide, const std::vector<std::string>& aOnly, const std::vector<std::string>& aRaw = {}) {
    return TestMakeRuntime(nSide, Roots(), aOnly, "", {}, {}, aRaw);
}

void RegistryTests() {
    printf("--- overrides: the registry\n");
    printf("  %s\n", D2LuaOv_Summary().c_str());
    printf("  this target %s patch entries\n", D2LuaOv_CanPatch() ? "can" : "cannot");
    const D2LuaOvEntry* e = D2LuaOv_Find("TestShared.Add");
    Check(e && strcmp(e->pszKinds, "nnn") == 0 && e->pThunk && e->nSide == (D2LUA_SIDE_SERVER | D2LUA_SIDE_CLIENT),
          "TestShared.Add: found by its engine path, kinds number(number, number), both sides");
#if defined(__i386__)
    Check(e && e->nConv == D2LUA_OV_FASTCALL && D2LuaOv_Find("TestShared.Scale")->nConv == D2LUA_OV_STDCALL &&
              D2LuaOv_Find("TestShared.Read")->nConv == D2LUA_OV_THISCALL && D2LuaOv_Find("TestShared.Half")->nConv == D2LUA_OV_CDECL,
          "  the conventions come from the functions' own types (fastcall, stdcall, thiscall, cdecl)");
#endif
    Check(D2LuaOv_Find("TestServer.Pick") && strcmp(D2LuaOv_Find("TestServer.Pick")->pszKinds, "wwwn") == 0 &&
              strcmp(D2LuaOv_Find("TestServer.Pick")->pszPointees, "D2UnitStrc|D2UnitStrc|D2UnitStrc") == 0,
          "TestServer.Pick: units in and out (w), their pointee names for the raw tier");
    Check(D2LuaOv_Find("TestShared.Sum") && D2LuaOv_Find("TestShared.Sum")->nRefused == D2LUA_OV_VARIADIC, "a variadic function is listed, refused");
    Check(D2LuaOv_Find("TestShared.Inlined") && D2LuaOv_Find("TestShared.Inlined")->nRefused == D2LUA_OV_INLINED,
          "an always_inline function is listed, refused as inlined");
    Check(D2LuaOv_Find("TestShared.Area") && D2LuaOv_Find("TestShared.Area")->nRefused == D2LUA_OV_BY_VALUE,
          "a record by value is listed, refused");
    Check(!D2LuaOv_Find("TestShared.NoSuch"), "an unknown path finds nothing");
    if (D2LuaOv_CanPatch()) {
        Check(EntryIs("TestShared.Add", kNops, 5, 0), "an entry nobody overrides: five nops at it");
    }
}

void TypedTests() {
    printf("--- overrides: the typed tier, one mod\n");
    ClearLog();
    D2LuaRuntime* pRt = Make(D2LUA_SIDE_SERVER, { "ovdouble" });
    Check(pRt != nullptr, "a server state with an overriding mod");
    if (!pRt) {
        return;
    }
    CheckLogged("[ovdouble] patch side 2 TestShared.Add nil", "patch(engine) ran once, before the game; engine.side; no raw tier");
    CheckLogged("[ovdouble] variadic false", "  overriding a variadic function raises");
    CheckLogged("variable argument list", "  with the reason");
    CheckLogged("[ovdouble] inlined false", "  overriding an inlined function raises");
    CheckLogged("inlined (always_inline) at its call sites", "  with the reason");
    CheckLogged("[ovdouble] byvalue false", "  overriding a function taking a record by value raises");
    CheckLogged("[ovdouble] server only true client only false", "a server state's engine has the server's functions, not the client's");
    CheckLogged("[ovdouble] unknown nil nil", "an unknown function or module is nil");
    if (D2LuaOv_CanPatch()) {
        Check(((const uint8_t*)D2LuaOv_Find("TestShared.Add")->pEntry)[0] == 0xE9, "  the entry is patched: a jump to the thunk");
        Check(EntryIs("TestShared.Plain", kNops, 5, 0), "  an entry nobody overrides is not");
    }
    auto pAdd = FX(TestShared, Add);
    Check(pAdd(2, 3) == 5, "a thread bound to no state runs the original");
    {
        Bound b(pRt, nullptr);
        Check(pAdd(2, 3) == 10, "bound to the state: the override runs (next(a, b) * 2)");
        Check(FX(TestShared, Half)(3.0) == 1.75, "  a double in and out");
        Check(FX(TestShared, IsEven)(2) == false && FX(TestShared, IsEven)(3) == true, "  a bool in and out");
        Check(FX(TestShared, Wrap)(7) == 0xFFFFFFFFu && FX(TestShared, Wrap)(5) == 5, "  -1 returned for an unsigned result wraps");
        g_nFixtureCount = 0;
        FX(TestShared, Count)(3);
        Check(g_nFixtureCount == 6, "  void: next() called twice runs the original twice");
        int nRec = FX(TestShared, Recurse)(3);
        int nWant = D2LuaOv_CanPatch() ? 4006 : 1006;
        if (nRec != nWant) {
            printf("  Recurse(3) = %d, want %d\n", nRec, nWant);
        }
        Check(nRec == nWant, "  recursion: the original's own calls go through the patched entry (nested overrides)");
        FixtureThing a = { 1, { 10, 20, 30, 40 } }, c = { 2, { 0, 0, 0, 0 } };
        Check(FX(TestShared, Echo)(&a) == &a, "  an opaque pointer passes through");
        Check(FX(TestShared, Other)(&a, &c) == &a, "  pointers reordered by the mod: next(q, p)");
        Check(FX(TestShared, Read)(&a, 1) == 30, "  thiscall: the object and an argument");
        Check(FX(TestShared, CallsInlined)(2) == 111, "  a caller of an inlined function is overridable");
        int nOther = -1;
        std::thread t([&] { nOther = pAdd(2, 3); });
        t.join();
        Check(nOther == 5, "another thread, bound to no state, runs the original meanwhile");
    }
    Check(pAdd(2, 3) == 5, "unbound again: the original");
    ClearLog();
    TestStart(pRt, 1);
    CheckLogged("[ovdouble] late false", "an override outside patch(engine) raises");
    CheckLogged("overrides are made in patch(engine)", "  saying where they belong");
    D2LuaOvJournal j = D2LuaOv_JournalTotals();
    D2LuaRt_EndGame(pRt, 1);
    D2LuaRt_Destroy(pRt);
    if (D2LuaOv_CanPatch()) {
        Check(EntryIs("TestShared.Add", kNops, 5, 0) && D2LuaOv_PatchedCount() == 0,
              "the state destroyed: every entry it patched is restored (the nops back)");
        D2LuaOvJournal j2 = D2LuaOv_JournalTotals();
        Check(j2.nPatches == j2.nRestores && j.nPatches > 0, "  the journal: as many restores as patches");
        printf("  journal: %d patches, %d restores\n", j2.nPatches, j2.nRestores);
    }
    Check(pAdd(2, 3) == 5, "  and the function runs as before");
}

void ChainTests() {
    printf("--- overrides: chains in dependency order\n");
    ClearLog();
    D2LuaRuntime* pRt = Make(D2LUA_SIDE_SERVER, { "ovtop", "ovbase" });
    if (!pRt) {
        Check(0, "chain state");
        return;
    }
    Bound b(pRt, nullptr);
    ClearLog();
    int n = FX(TestShared, Scale)(2);
    Check(n == 70, "two mods: the dependent's override wraps its dependency's, next() reaching the original ((2*3+1)*10)");
    const char* pszLog = g_szTestLog;
    const char* p1 = strstr(pszLog, "top in 2");
    const char* p2 = strstr(pszLog, "base in 2");
    const char* p3 = strstr(pszLog, "base out 7");
    const char* p4 = strstr(pszLog, "top out 70");
    Check(p1 && p2 && p3 && p4 && p1 < p2 && p2 < p3 && p3 < p4, "  in order: top, base, original, base, top");
    Check(FX(TestShared, Chained)(5) == 14, "  next() with changed arguments ((5+1)+1)*2");
    D2LuaRt_Destroy(pRt);
}

void FaultTests() {
    printf("--- overrides: faults\n");
    FixtureThing a = { 1, { 0, 0, 0, 0 } }, c = { 2, { 0, 0, 0, 0 } };
    {
        ClearLog();
        D2LuaRuntime* pRt = Make(D2LUA_SIDE_SERVER, { "ovfault" });
        Bound b(pRt, nullptr);
        Check(FX(TestShared, Fail)(1) == 6, "an override that works");
        ClearLog();
        Check(FX(TestShared, Fail)(13) == 13, "an override that raises: the call falls back to the original");
        CheckLogged("its override of TestShared.Fail raised", "  logged");
        CheckLogged("unlucky", "  with the error");
        Check(D2LuaRt_IsFaulted(pRt, "ovfault"), "  the mod is faulted");
        Check(FX(TestShared, Fail)(1) == 1 && FX(TestShared, Echo)(&a) == &a, "  and every override of it is gone");
        D2LuaRt_Destroy(pRt);
    }
    {
        ClearLog();
        D2LuaRuntime* pRt = Make(D2LUA_SIDE_SERVER, { "ovfault" });
        Bound b(pRt, nullptr);
        ClearLog();
        Check(FX(TestShared, Echo)(&a) == &a, "a result of the wrong type (a string for a pointer): the original's result");
        CheckLogged("returned not a pointer of the call", "  logged as what it returned");
        Check(D2LuaRt_IsFaulted(pRt, "ovfault"), "  the mod is faulted");
        D2LuaRt_Destroy(pRt);
    }
    {
        ClearLog();
        D2LuaRuntime* pRt = Make(D2LUA_SIDE_SERVER, { "ovbadptr" });
        Bound b(pRt, nullptr);
        FX(TestShared, Echo)(&a);
        ClearLog();
        Check(FX(TestShared, Other)(&c, &c) == &c, "a pointer kept from another call handed to next(): refused, the original runs");
        CheckLogged("is a pointer this call was not given", "  next() says why");
        Check(D2LuaRt_IsFaulted(pRt, "ovbadptr"), "  the mod is faulted");
        D2LuaRt_Destroy(pRt);
    }
    {
        ClearLog();
        D2LuaRuntime* pRt = Make(D2LUA_SIDE_SERVER, { "ovspin" });
        Bound b(pRt, nullptr);
        Clock::time_point t0 = Clock::now();
        int n = FX(TestShared, Slow)(4);
        int nMs = (int)std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - t0).count();
        Check(n == 4 && nMs < (int)kTestBudgetMs * 4, "an override that never ends: stopped by the time budget, the original's result");
        printf("  stopped after %d ms\n", nMs);
        CheckLogged("ran past its time budget", "  the budget's fault");
        Check(D2LuaRt_IsFaulted(pRt, "ovspin"), "  the mod is faulted");
        D2LuaRt_Destroy(pRt);
    }
    {
        ClearLog();
        D2LuaRuntime* pRt = Make(D2LUA_SIDE_SERVER, { "ovspin" });
        Bound b(pRt, nullptr);
        Check(FX(TestShared, Yields)(5) == 5, "an override that yields: refused, the original runs");
        CheckLogged("yielded (an override runs to its end", "  logged");
        D2LuaRt_Destroy(pRt);
    }
    {
        ClearLog();
        D2LuaRuntime* pRt = Make(D2LUA_SIDE_SERVER, { "ovpatchfail" });
        Bound b(pRt, nullptr);
        CheckLogged("patch fails after one override", "a patch that raises is logged");
        Check(FX(TestShared, Plain)(1) == 2, "  and keeps none of its overrides");
        if (D2LuaOv_CanPatch()) {
            Check(EntryIs("TestShared.Plain", kNops, 5, 0), "  its entry is restored");
        }
        D2LuaRt_Destroy(pRt);
    }
    {
        ClearLog();
        D2LuaRuntime* pRt = Make(D2LUA_SIDE_SERVER, { "ovbadkey" });
        CheckLogged("the mod has no entry point 'pach'", "defineMod with a key that is no entry point: the mod does not load");
        D2LuaRt_Destroy(pRt);
    }
}

void SideTests() {
    printf("--- overrides: sides and threads\n");
    ClearLog();
    D2LuaRuntime* pClient = Make(D2LUA_SIDE_CLIENT, { "ovclient" });
    D2LuaRuntime* pServer = Make(D2LUA_SIDE_SERVER, { "ovhot" });
    CheckLogged("[ovclient] client engine 4 true true", "a client state's engine: side 4, the client's functions, not the server's");
    auto pAdd = FX(TestShared, Add);
    auto pClientOnly = FX(TestClient, ClientOnly);
    auto pServerOnly = FX(TestServer, ServerOnly);
    {
        Bound b(nullptr, pClient);
        Check(pAdd(1, 2) == 1003 && pClientOnly(1) == 3, "the client's thread: the client state's overrides");
        Check(pServerOnly(4) == 4, "  a server function on the client's thread runs the original");
    }
    {
        Bound b(pServer, pClient);
        Check(pAdd(1, 2) == 3, "a shared function inside a server scope uses the server state (no override there)");
        Check(pClientOnly(1) == 3, "  a client function there still uses the client state");
    }
    D2LuaRt_Destroy(pServer);
    D2LuaRt_Destroy(pClient);
}

// Entries patched and restored while another thread keeps calling through them (bound to no state,
// so it runs the original every time): it never stops in the middle of a jump.
void ConcurrentTests() {
    printf("--- overrides: patching while another thread runs the entry\n");
    if (!D2LuaOv_CanPatch()) {
        printf("SKIP no patching on this target\n");
        return;
    }
    std::atomic<bool> bStop{ false };
    std::atomic<long> nCalls{ 0 }, nWrong{ 0 };
    auto pHot = FX(TestShared, Hot);
    std::thread t([&] {
        while (!bStop) {
            for (int i = 0; i < 1000; i++) {
                if (pHot(i) != i + 1) {
                    nWrong++;
                }
            }
            nCalls += 1000;
        }
    });
    D2LuaOvJournal j0 = D2LuaOv_JournalTotals();
    for (int n = 0; n < 40; n++) {
        D2LuaRuntime* pRt = Make(D2LUA_SIDE_SERVER, { "ovhot" });
        D2LuaRt_Destroy(pRt);
    }
    bStop = true;
    t.join();
    D2LuaOvJournal j1 = D2LuaOv_JournalTotals();
    printf("  %ld calls on the other thread, %d patches and %d restores meanwhile\n", nCalls.load(), j1.nPatches - j0.nPatches,
           j1.nRestores - j0.nRestores);
    Check(nWrong == 0 && nCalls > 0 && j1.nPatches - j0.nPatches == 40, "  every call there ran the original; every patch was made");
}

void RawTests() {
    printf("--- overrides: the raw tier\n");
    ClearLog();
    D2LuaRuntime* pRt = Make(D2LUA_SIDE_SERVER, { "ovraw" }, { "ovraw" });
    CheckLogged("[ovraw] raw tier true", "a granted mod gets engine.unsafe");
    {
        Bound b(pRt, nullptr);
        FixtureThing a = { 7, { 10, 20, 30, 40 } };
        ClearLog();
        int n = FX(TestShared, Read)(&a, 1);
        CheckLogged("[ovraw] raw arg cdata 7 30", "  pointers arrive as cdata of their type, fields read through ffi");
        Check(n == 30 && a.nValue == 8, "  written through ffi, next() takes the cdata back");
        Check(FX(TestShared, Echo)(&a) == &a, "  a cdata result becomes the pointer");
    }
    D2LuaRt_Destroy(pRt);
    ClearLog();
    pRt = Make(D2LUA_SIDE_SERVER, { "ovraw" });
    CheckLogged("[ovraw] raw tier false", "without the grant engine.unsafe is nil");
    D2LuaRt_Destroy(pRt);
    ClearLog();
    pRt = Make(D2LUA_SIDE_SERVER, { "ovnoraw" });
    CheckLogged("[ovnoraw] no raw tier true nil false", "the engine is read-only and holds no raw tier");
    CheckLogged("[ovnoraw] pairs 0 false", "  pairs walks nothing, the metatable is locked");
    {
        Bound b(pRt, nullptr);
        FixtureThing a = { 7, { 0, 0, 0, 0 } };
        ClearLog();
        FX(TestShared, Echo)(&a);
        CheckLogged("[ovnoraw] opaque userdata false false nil nil", "a typed override's pointer cannot be read or indexed; no ffi, no jit");
    }
    D2LuaRt_Destroy(pRt);
}

// A hot override, through its patched entry, against the same body not overridden.
void HotTests(bool bBench) {
    printf("--- overrides: a hot override runs compiled\n");
    ClearLog();
    D2LuaRuntime* pRt = Make(D2LUA_SIDE_SERVER, { "ovhot" });
    lua_State* L = D2LuaRt_State(pRt);
    // jit.attach from the host (the mods have no jit): count traces started and stopped in the
    // override's chunk, and the aborts with their reasons.
    lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
    lua_getfield(L, -1, "jit");
    lua_setglobal(L, "ov_jit");
    lua_pop(L, 1);
    // jit.util is preloaded, not loaded: the host opens no package library
    lua_getfield(L, LUA_REGISTRYINDEX, "_PRELOAD");
    lua_getfield(L, -1, "jit.util");
    if (lua_isfunction(L, -1)) {
        lua_call(L, 0, 1);
    }
    lua_setglobal(L, "ov_jitutil");
    lua_pop(L, 1);
    const char* pszAttach =
        "ov_trace = { start = 0, stop = 0, abort = 0, why = {}, lines = {} } local jit = ov_jit local util = ov_jitutil "
        "local cur = {} "
        "jit.attach(function(what, tr, func, pc, otr, oex) "
        "  local info = func and util.funcinfo(func) or {} "
        "  if what == 'start' then cur[tr] = info.source or '?' "
        "    if (info.source or ''):find('ovhot') then ov_trace.start = ov_trace.start + 1 ov_trace.lines[#ov_trace.lines + 1] = 'TRACE ' .. tr .. ' start ' .. tostring(info.loc) end "
        "  elseif what == 'stop' then if (cur[tr] or ''):find('ovhot') then ov_trace.stop = ov_trace.stop + 1 ov_trace.lines[#ov_trace.lines + 1] = 'TRACE ' .. tr .. ' stop' end "
        "  elseif what == 'abort' then if (cur[tr] or ''):find('ovhot') then ov_trace.abort = ov_trace.abort + 1 ov_trace.why[#ov_trace.why + 1] = tostring(otr) .. ' ' .. tostring(oex) end end "
        "end, 'trace')";
    if (luaL_dostring(L, pszAttach) != 0) {
        printf("  attach: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    auto pHot = FX(TestShared, Hot);
    auto pPlain = FX(TestShared, Plain);
    long long nSum = 0;
    {
        Bound b(pRt, nullptr);
        for (int i = 0; i < 20000; i++) {
            nSum += pHot(i);
        }
    }
    Check(nSum == 20000LL * 19999 / 2 + 2 * 20000, "a hot override computes (next(a) + 1, 20000 calls)");
    luaL_dostring(L, "return ov_trace.start, ov_trace.stop, ov_trace.abort, table.concat(ov_trace.lines, '; '), table.concat(ov_trace.why, '; ')");
    int nStart = (int)lua_tointeger(L, -5), nStop = (int)lua_tointeger(L, -4), nAbort = (int)lua_tointeger(L, -3);
    printf("  traces in the override's chunk: %d started, %d stopped, %d aborted\n  %s\n", nStart, nStop, nAbort, lua_tostring(L, -2));
    if (nAbort) {
        printf("  aborts: %s\n", lua_tostring(L, -1));
    }
    lua_pop(L, 5);
    Check(nStop >= 1, "  LuaJIT compiled the override (a trace in its chunk)");
    if (bBench) {
        const int kN = 2000000;
        volatile int nSink = 0;
        Clock::time_point t0 = Clock::now();
        for (int i = 0; i < kN; i++) {
            nSink += pPlain(i);
        }
        double nPlain = std::chrono::duration<double, std::nano>(Clock::now() - t0).count() / kN;
        double nPatchedIdle = 0;
        {
            // a patched entry, called on a thread whose state has no override of it
            Bound b(nullptr, nullptr);
            t0 = Clock::now();
            for (int i = 0; i < kN; i++) {
                nSink += pHot(i);
            }
            nPatchedIdle = std::chrono::duration<double, std::nano>(Clock::now() - t0).count() / kN;
        }
        double nOver = 0;
        {
            Bound b(pRt, nullptr);
            t0 = Clock::now();
            for (int i = 0; i < kN / 4; i++) {
                nSink += pHot(i);
            }
            nOver = std::chrono::duration<double, std::nano>(Clock::now() - t0).count() / (kN / 4);
        }
        printf("bench override: original %.1f ns/call, patched with no override on this thread %.1f ns/call, overridden %.1f ns/call (%d-bit)\n",
               nPlain, nPatchedIdle, nOver, (int)sizeof(void*) * 8);
        (void)nSink;
    }
    D2LuaRt_Destroy(pRt);
}

}  // namespace

// The units of a server state's world (d2_lua_test_engine.cpp runs this with its stand-in world).
void RunOverrideUnitTests(D2LuaRuntime* pRt, D2UnitStrc* pA, D2UnitStrc* pB) {
    Bound b(pRt, nullptr);
    ClearLog();
    D2UnitStrc* p = FX(TestServer, Pick)(pA, pB, 0);
    CheckLogged("[ovunit] pick userdata true true 0", "units arrive as the world's handles, valid");
    Check(p == pB, "  handles handed to next() in another order become those units");
    p = FX(TestServer, Pick)(pA, nullptr, 1);
    Check(p == pA, "  nil where the call was given null");
}

void RunOverrideTests(const char* pszRoot, bool bBench) {
    g_sTestRoot = pszRoot;
    RegistryTests();
    TypedTests();
    ChainTests();
    FaultTests();
    SideTests();
    ConcurrentTests();
    RawTests();
    HotTests(bBench);
    if (D2LuaOv_CanPatch()) {
        Check(D2LuaOv_PatchedCount() == 0, "overrides: every state gone, no entry is patched");
    }
}
#endif
