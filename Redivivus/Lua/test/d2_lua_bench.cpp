// Dispatch timings: an event to N handlers (raised by the host, each handler its own entry, and
// raised from inside a mod's call, the handlers nested in it) and a registry entry with a wrap
// chain of depth N, called from a mod in a loop. Mods are written to TMP. --bench-dispatch.
#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include <stdio.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <direct.h>
#define D2_MKDIR(p) _mkdir(p)
#else
#define D2_MKDIR(p) mkdir(p, 0755)
#endif

#include <chrono>
#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_runtime.h"
#include "Redivivus/Lua/test/d2_lua_test_util.h"
#include "lua.hpp"

typedef std::chrono::steady_clock Clock;

static void WriteFile(const std::string& sPath, const std::string& sText) {
    FILE* pFile = fopen(sPath.c_str(), "wb");
    if (pFile) {
        fwrite(sText.data(), 1, sText.size(), pFile);
        fclose(pFile);
    }
}

int RunDispatchBench(const char* pszRoot, const char* pszTmp) {
    g_sTestRoot = pszRoot;
    std::string sTmp = pszTmp;
    const int kMods = 100;
    std::vector<std::string> aNames;
    for (int i = 0; i < kMods; i++) {
        char szName[32];
        snprintf(szName, sizeof(szName), "bm%03d", i);
        std::string sDir = sTmp + "/" + szName;
        D2_MKDIR(sDir.c_str());
        WriteFile(sDir + "/info.json", std::string("{\"name\":\"") + szName + "\",\"version\":\"1\"}");
        WriteFile(sDir + "/main.lua",
                  "return { server = function(game) games = { game } game.on('tick', function(e) end) "
                  "game.on('testUnitDamaged', function(e) end) end }");
        aNames.push_back(szName);
    }
    for (int n : { 1, 10, 100 }) {
        std::vector<std::string> aOnly(aNames.begin(), aNames.begin() + n);
        D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, { sTmp }, aOnly, "", { "bm000" });
        TestStart(pRt, 1);
        int nIters = 200000 / n;
        for (int i = 0; i < 100; i++) {
            D2LuaRt_Tick(pRt, 1);
        }
        Clock::time_point t0 = Clock::now();
        for (int i = 0; i < nIters; i++) {
            D2LuaRt_Tick(pRt, 1);
        }
        double nNs = std::chrono::duration<double, std::nano>(Clock::now() - t0).count();
        printf("bench host-dispatch N=%d: %.0f ns/event, %.0f ns/handler\n", n, nNs / nIters, nNs / nIters / n);
        char szSource[400];
        snprintf(szSource, sizeof(szSource), "local u = require('redivivus:test').world.spawn('x', 0, 1e15) for i = 1, %d do u:damage(1) end", nIters);
        Clock::time_point t1 = Clock::now();
        TestRun(pRt, 1, "bm000", szSource);
        nNs = std::chrono::duration<double, std::nano>(Clock::now() - t1).count();
        printf("bench mod-dispatch N=%d: %.0f ns/event, %.0f ns/handler\n", n, nNs / nIters, nNs / nIters / n);
        D2LuaRt_Destroy(pRt);
    }
    for (int nDepth : { 1, 10, 100 }) {
        D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, { sTmp }, { "bm000" }, "");
        TestStart(pRt, 1);
        char szSource[400];
        snprintf(szSource, sizeof(szSource), "for i = 1, %d do games[1].registry.exampleSlot.wrap(function(next, v) return next(v) end) end", nDepth);
        TestRun(pRt, 1, "bm000", szSource);
        int nIters = 200000 / nDepth;
        snprintf(szSource, sizeof(szSource), "local c = games[1].registry.exampleSlot.call for i = 1, %d do c(1) end", nIters);
        Clock::time_point t1 = Clock::now();
        TestRun(pRt, 1, "bm000", szSource);
        double nNs = std::chrono::duration<double, std::nano>(Clock::now() - t1).count();
        printf("bench wrap-chain depth=%d: %.0f ns/call, %.0f ns/level\n", nDepth, nNs / nIters, nNs / nIters / nDepth);
        D2LuaRt_Destroy(pRt);
    }
    return 0;
}
#endif
