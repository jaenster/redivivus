// The host test's checks of the mod system: step-limited patterns, the loader, the runtime
// (require rules, events, remote, registries, scripts, storage, reload) and the handle class of
// the test world. Fixture mods live in Redivivus/Lua/test/mods; the TypeScript example mods are
// built by the mod CLI and passed with --ts-mods.
#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include <stdio.h>
#include <string.h>

#include <chrono>
#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_internal.h"
#include "Redivivus/Lua/d2_lua_mods.h"
#include "Redivivus/Lua/d2_lua_runtime.h"
#include "Redivivus/Lua/generated/d2_lua_api_main.gen.h"
#include "Redivivus/Lua/test/d2_lua_test_util.h"
#include "Redivivus/Lua/test/d2_lua_test_world.h"
#include "Redivivus/Lua/test/generated/d2_lua_api_test.gen.h"
#include "lua.hpp"

typedef std::chrono::steady_clock Clock;

static const unsigned kBudgetMs = 250;
static std::string s_sRoot;

static int ElapsedMs(Clock::time_point tStart) {
    return (int)std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - tStart).count();
}

static void CheckLogged(const char* pszText, const char* pszWhat) {
    bool bOk = Logged(pszText);
    if (!bOk) {
        printf("  (log lacks: %s)\n", pszText);
    }
    Check(bOk, pszWhat);
}

// --- patterns ---------------------------------------------------------------------------------------

// Compares the stock string functions (the state's global string table, which mods never see)
// with the mods' step-limited ones over a corpus, in the host's own global environment.
static const char s_szPatternCorpus[] = R"LUA(
local stock, mine = ...
local subjects = {
  "", "a", "aaa", "ab", "hello world", "  padded  ", "abcabcabc", "key = value", "x=1, y=22, z=333",
  "THE (quick) [brown] fox", "a.b.c", "%d%s", "line1\nline2\n", "\0bin\0ary\0", "\206\177\206\178 utf8",
  "f(a(b)c)d", "123abc456", "--[[comment]]", "a,b,,c,", "tab\tsep\tvalues", "CamelCaseWord",
  "THE END.", "aaab", "mississippi", "  x  y  ", "[[nested]]", "1e10 -3.5 +7", "\1\2\3\127",
}
local patterns = {
  "a", ".", "a*", "a+", "a-", "a?", "^a", "a$", "^$", "^", "$", "", "%a+", "%d+", "%s*", "%w+", "%p", "%u%l*",
  "%x+", "%c", "%z", "%A", "%D+", "%S+", "%W", "%L", "%U+", "%X", "%P+", "%C", "%Z", "%g+", "%G",
  "[abc]", "[^abc]+", "[a-z]+", "[%a_][%w_]*", "[%]]", "[]]", "[]a]", "[^]]+", "[^%s]+", "[a-]", "[-a]", "[%-]",
  "(a)", "(a*)", "()a", "()", "(%w+)=(%w+)", "(%w+)%s*=%s*(%w+)", "%((.-)%)", "%b()", "%b[]", "%bxy", "%b%%",
  "%f[%w]%w+", "%f[%W]", "%f[%a]", "%f[^%z]", "(.)%1", "((a)(b))", "^(%s*)(.-)(%s*)$", ".-", ".-b", "x*$",
  "[%d%.]+", "%.", "%%", "[+-]?%d+", "(%d+)(%a+)", "a-b", "(a+)(b?)", "s+i", "(ss)(i)", "i(%w)%1",
  "(%a+)%s(%a+)", "^%s*(.-)%s*$", "[%z\1-\31]", "[\128-\255]+", "%[%[(.-)%]%]", ",", ",,", "[,]?",
  -- malformed: errors must match too
  "%", "[a", "(a", "a)", "(()", "%1", "(%1)", "%b", "%ba", "%f", "%fa", "[%", "[^", "((((((((((((((((((((((((((((((((((a))))))))))))))))))))))))))))))))))",
}
local inits = { nil, 1, 2, 3, 0, -1, -3, 100, -100 }
local function pack(...) return { n = select("#", ...), ... } end
local function same(a, b)
  if a.n ~= b.n then return false end
  for i = 1, a.n do
    local x, y = a[i], b[i]
    if type(x) ~= type(y) then return false end
    if type(x) == "string" then
      -- error messages carry no position from a direct pcall of a C function, in both
      if x ~= y then return false end
    elseif x ~= y and not (x ~= x and y ~= y) then
      return false
    end
  end
  return true
end
local function show(r)
  local out = {}
  for i = 1, r.n do out[#out + 1] = tostring(r[i]) end
  return "{" .. table.concat(out, ", ") .. "}"
end
local count, bad, first = 0, 0, {}
local function compare(what, a, b)
  count = count + 1
  if not same(a, b) then
    bad = bad + 1
    if #first < 12 then first[#first + 1] = what .. ": stock " .. show(a) .. " mine " .. show(b) end
  end
end
local function gm(f, s, p)
  local out = { n = 0 }
  local ok, it = pcall(f, s, p)
  if not ok then return pack(false, it) end
  for i = 1, 60 do
    local r = pack(pcall(it))
    if not r[1] then out.n = out.n + 1 out[out.n] = r[2] break end
    if r[2] == nil then break end
    for k = 2, r.n do out.n = out.n + 1 out[out.n] = r[k] end
  end
  return out
end
local repls = {
  "%1-%0", "<%%>", "x", "%0%0", "%2", "%", "", 7,
  { a = "A", hello = "HELLO", [""] = "E" },
  function(c, d) return c and c:upper() end,
  function() return false end,
  function(c) return #tostring(c) end,
  function() return {} end,
}
for _, s in ipairs(subjects) do
  for _, p in ipairs(patterns) do
    for _, i in ipairs(inits) do
      compare("find(" .. ("%q"):format(s) .. "," .. ("%q"):format(p) .. "," .. tostring(i) .. ")",
        pack(pcall(stock.find, s, p, i)), pack(pcall(mine.find, s, p, i)))
      compare("match(" .. ("%q"):format(s) .. "," .. ("%q"):format(p) .. "," .. tostring(i) .. ")",
        pack(pcall(stock.match, s, p, i)), pack(pcall(mine.match, s, p, i)))
    end
    compare("find plain " .. ("%q"):format(p), pack(pcall(stock.find, s, p, 1, true)), pack(pcall(mine.find, s, p, 1, true)))
    compare("gmatch(" .. ("%q"):format(s) .. "," .. ("%q"):format(p) .. ")", gm(stock.gmatch, s, p), gm(mine.gmatch, s, p))
    for ri, r in ipairs(repls) do
      compare("gsub(" .. ("%q"):format(s) .. "," .. ("%q"):format(p) .. ",repl " .. ri .. ")",
        pack(pcall(stock.gsub, s, p, r)), pack(pcall(mine.gsub, s, p, r)))
      compare("gsub n=2", pack(pcall(stock.gsub, s, p, r, 2)), pack(pcall(mine.gsub, s, p, r, 2)))
    end
  end
end
-- argument errors
compare("find nil", pack(pcall(stock.find, nil, "a")), pack(pcall(mine.find, nil, "a")))
compare("gsub bad repl", pack(pcall(stock.gsub, "a", "a", true)), pack(pcall(mine.gsub, "a", "a", true)))
compare("find number subject", pack(pcall(stock.find, 12345, "3")), pack(pcall(mine.find, 12345, "3")))
compare("gmatch nil", pack(pcall(stock.gmatch, "a", nil)), pack(pcall(mine.gmatch, "a", nil)))
return count, bad, table.concat(first, "\n")
)LUA";

static void PatternTests() {
    printf("--- patterns\n");
    D2LuaLimits tLimits = { kBudgetMs, (size_t)64 * 1024 * 1024 };
    lua_State* L = D2LuaHost_Create(TestLog, &tLimits);
    if (luaL_loadstring(L, s_szPatternCorpus) != 0) {
        printf("corpus: %s\n", lua_tostring(L, -1));
        Check(0, "pattern corpus loads");
        D2LuaHost_Destroy(L);
        return;
    }
    lua_getglobal(L, "string");
    D2LuaHost_NewModEnv(L, "patterns");
    lua_getfield(L, -1, "string");
    lua_remove(L, -2);
    if (lua_pcall(L, 2, 3, 0) != 0) {
        printf("corpus: %s\n", lua_tostring(L, -1));
        Check(0, "pattern corpus runs");
    } else {
        int nCount = (int)lua_tointeger(L, -3);
        int nBad = (int)lua_tointeger(L, -2);
        printf("  %d comparisons, %d differ\n%s\n", nCount, nBad, lua_tostring(L, -1));
        Check(nCount > 30000 && nBad == 0, "step-limited find/match/gmatch/gsub equal the stock ones over the corpus");
    }
    lua_settop(L, 0);

    char szBudget[96];
    snprintf(szBudget, sizeof(szBudget), "time budget of %u ms", kBudgetMs);
    static const struct { const char* pszMod; const char* pszWhat; const char* pszSource; } s_aHangs[] = {
        { "hang_find", "(\"a\"):rep(2000):find(\".-.-.-b\") is stopped at the budget", "local s = ('a'):rep(2000) return s:find('.-.-.-b')" },
        { "hang_find2", "  as string.find too", "return string.find(string.rep('a', 2000), '.-.-.-b')" },
        { "hang_match", "  string.match", "return string.match(string.rep('a', 2000), '.-.-.-b')" },
        { "hang_gsub", "  string.gsub", "return string.gsub(string.rep('a', 2000), '.-.-.-b', 'x')" },
        { "hang_gmatch", "  string.gmatch", "for x in string.gmatch(string.rep('a', 2000), '.-.-.-b') do end" },
        { "hang_pcall", "  inside a pcall the mod spins on", "while true do pcall(string.find, string.rep('a', 2000), '.-.-.-b') end" },
        { "hang_plain", "  a long plain find is counted too", "local s = string.rep('a', 4000000) for i = 1, 1000 do s:find(string.rep('a', 2000) .. 'b', 1, true) end" },
    };
    for (const auto& tHang : s_aHangs) {
        ClearLog();
        Clock::time_point tStart = Clock::now();
        int nResult = D2LuaHost_RunString(L, tHang.pszMod, tHang.pszSource, tHang.pszMod);
        int nMs = ElapsedMs(tStart);
        printf("  stopped after %d ms\n", nMs);
        Check(nResult == D2LUA_FAULT && Logged(szBudget) && nMs < (int)kBudgetMs * 4, tHang.pszWhat);
    }
    ClearLog();
    int nOk = D2LuaHost_RunString(L, "pat_ok",
                                  "print('patterns', ('key = value'):match('(%w+)%s*=%s*(%w+)'), ('a,b'):gsub(',', ';'))", "ok");
    Check(nOk == D2LUA_OK && Logged("[lua] patterns\tkey\ta;b\t1"), "patterns work in a mod after the hangs");
    D2LuaHost_Destroy(L);
}

// --- the loader -------------------------------------------------------------------------------------

static bool ParseMods(const std::vector<const char*>& aJson, std::vector<D2LuaModInfo>& aOut, std::string& sError) {
    aOut.clear();
    for (const char* pszJson : aJson) {
        D2LuaModInfo tInfo;
        if (!D2LuaMods_ParseInfo(pszJson, "/nowhere", tInfo, sError)) {
            return false;
        }
        aOut.push_back(tInfo);
    }
    return true;
}

static std::string Order(const std::vector<D2LuaModInfo>& aMods) {
    std::string s;
    for (const auto& t : aMods) {
        s += (s.empty() ? "" : " ") + t.sName;
    }
    return s;
}

// Resolve and return the order, or "ERR: <errors joined>".
static std::string ResolveJson(const std::vector<const char*>& aJson) {
    std::vector<D2LuaModInfo> aMods;
    std::string sError;
    if (!ParseMods(aJson, aMods, sError)) {
        return "PARSE: " + sError;
    }
    std::vector<D2LuaModInfo> aOrdered;
    std::vector<std::string> aErrors;
    if (!D2LuaMods_Resolve(aMods, aOrdered, aErrors)) {
        std::string s = "ERR:";
        for (const auto& e : aErrors) {
            s += " " + e + ";";
        }
        return s;
    }
    return Order(aOrdered);
}

static void ExpectResolve(const std::vector<const char*>& aJson, const char* pszWant, const char* pszWhat) {
    std::string sGot = ResolveJson(aJson);
    bool bOk = sGot.find(pszWant) != std::string::npos;
    if (!bOk) {
        printf("  got: %s\n  wanted: %s\n", sGot.c_str(), pszWant);
    }
    Check(bOk, pszWhat);
}

#define BASE "{\"name\":\"base\",\"version\":\"0.1.0\"}"

static void LoaderTests() {
    printf("--- loader\n");
    ExpectResolve({ "{\"name\":\"zeta\",\"version\":\"1.0.0\"}", "{\"name\":\"alpha2\",\"version\":\"1\",\"dependencies\":[\"zeta\"]}",
                    "{\"name\":\"mid\",\"version\":\"1\"}", BASE },
                  "base mid zeta alpha2", "load order: dependencies first, ties by name, base implicit");
    ExpectResolve({ BASE, "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"nothere\"]}" },
                  "ERR: mod 'a' requires 'nothere', which is not installed", "missing dependency");
    ExpectResolve({ BASE, "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"base >= 2.0\"]}" },
                  "mod 'a' requires 'base >= 2.0.0', but version 0.1.0 is installed", "version mismatch (>=)");
    ExpectResolve({ BASE, "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"b < 1.5\"]}", "{\"name\":\"b\",\"version\":\"1.5.0\"}" },
                  "mod 'a' requires 'b < 1.5.0', but version 1.5.0 is installed", "version mismatch (<)");
    ExpectResolve({ BASE, "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"b = 1.5\"]}", "{\"name\":\"b\",\"version\":\"1.5.0\"}" },
                  "base b a", "version match (=)");
    ExpectResolve({ BASE, "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"! b\"]}", "{\"name\":\"b\",\"version\":\"1\"}" },
                  "mod 'a' is incompatible with 'b', and both are enabled", "incompatibility");
    ExpectResolve({ BASE, "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"! b\"]}" }, "base a", "incompatibility with an absent mod is fine");
    ExpectResolve({ BASE, "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"b\"]}", "{\"name\":\"b\",\"version\":\"1\",\"dependencies\":[\"c\"]}",
                    "{\"name\":\"c\",\"version\":\"1\",\"dependencies\":[\"a\"]}" },
                  "ERR: dependency cycle: a -> b -> c -> a", "a cycle is named in order");
    ExpectResolve({ BASE, "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"? nothere\"]}" }, "base a", "absent optional dependency is fine");
    ExpectResolve({ BASE, "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"? z\"]}", "{\"name\":\"z\",\"version\":\"1\"}" },
                  "base z a", "present optional dependency orders");
    ExpectResolve({ BASE, "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"(?) z\"]}", "{\"name\":\"z\",\"version\":\"1\"}" },
                  "base z a", "hidden optional dependency orders");
    ExpectResolve({ BASE, "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"? z >= 2\"]}", "{\"name\":\"z\",\"version\":\"1\"}" },
                  "mod 'a' requires 'z >= 2.0.0', but version 1 is installed", "optional dependency with the wrong version");
    ExpectResolve({ BASE, "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"~ z\"]}", "{\"name\":\"z\",\"version\":\"1\"}" },
                  "base a z", "~ dependency does not order");
    ExpectResolve({ BASE, "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"~ z\"]}" },
                  "requires 'z', which is not installed", "~ dependency is still required");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\"}" }, "ERR: mod 'a' requires 'base'", "base is implicit");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"? base\"]}" }, "a", "naming base optional opts out");
    ExpectResolve({ BASE, "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"base >> 1\"]}" }, "PARSE: info.json: dependency \"base >> 1\"",
                  "bad dependency syntax");
    ExpectResolve({ "{\"name\":\"Bad Name\",\"version\":\"1\"}" }, "PARSE: info.json: name \"Bad Name\" is not valid", "bad mod name");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1.x\"}" }, "PARSE: info.json: version \"1.x\"", "bad version");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"redivivus_version\":\"9.0\"}" }, "PARSE: mod 'a' needs Redivivus API 9.0",
                  "redivivus_version newer than the host");
    ExpectResolve({ "{\"name\":\"a\"}" }, "PARSE: info.json: \"version\" is missing", "missing version");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":\"base\"}" }, "must be an array", "dependencies not an array");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",}" }, "PARSE: info.json: line 1", "malformed json names the position");
    ExpectResolve({ BASE, BASE }, "ERR: mod 'base' is listed twice", "a mod listed twice");

    std::vector<D2LuaModInfo> aFound;
    std::vector<std::string> aErrors;
    D2LuaMods_Discover({ s_sRoot + "/Redivivus/Lua/mods", s_sRoot + "/Redivivus/Lua/test/mods" }, aFound, aErrors);
    Check(aErrors.empty() && Order(aFound) == "base alpha beta faulty gamma kitty", "discovery finds base and the fixture mods");
    if (!aErrors.empty() || Order(aFound) != "base alpha beta faulty gamma kitty") {
        printf("  found: %s, %d errors%s%s\n", Order(aFound).c_str(), (int)aErrors.size(), aErrors.empty() ? "" : ": ",
               aErrors.empty() ? "" : aErrors[0].c_str());
    }
    aFound.clear();
    aErrors.clear();
    D2LuaMods_Discover({ s_sRoot + "/Redivivus/Lua/mods", s_sRoot + "/Redivivus/Lua/mods" }, aFound, aErrors);
    Check(aErrors.size() == 1 && aErrors[0].find("found twice") != std::string::npos, "a mod found in two roots is an error");
}

// --- the runtime ------------------------------------------------------------------------------------

static const D2LuaApiDesc* const s_apApis[] = { &g_D2LuaApi_main, &g_D2LuaApi_test };

D2LuaRuntime* TestMakeRuntime(int nSide, const std::vector<std::string>& aRoots, const std::vector<std::string>& aOnly,
                              const std::string& sSettingsFile) {
    std::vector<D2LuaModInfo> aFound;
    std::vector<std::string> aErrors;
    D2LuaMods_Discover(aRoots, aFound, aErrors);
    std::vector<D2LuaModInfo> aPicked;
    for (const auto& t : aFound) {
        for (const auto& s : aOnly) {
            if (t.sName == s) {
                aPicked.push_back(t);
            }
        }
    }
    D2LuaRuntimeDesc tDesc;
    tDesc.nSide = nSide;
    tDesc.pfnLog = TestLog;
    tDesc.tLimits.nBudgetMs = kBudgetMs;
    tDesc.tLimits.nMemoryBytes = (size_t)64 * 1024 * 1024;
    tDesc.aApis.assign(s_apApis, s_apApis + 2);
    tDesc.sSettingsFile = sSettingsFile;
    if (!D2LuaMods_Resolve(aPicked, tDesc.aMods, aErrors)) {
        for (const auto& e : aErrors) {
            printf("  resolve: %s\n", e.c_str());
        }
        return nullptr;
    }
    return D2LuaRt_Create(tDesc);
}

static D2LuaRuntime* MakeRuntime(int nSide, const std::vector<std::string>& aRoots, const std::vector<std::string>& aOnly) {
    return TestMakeRuntime(nSide, aRoots, aOnly, std::string());
}

// Run a chunk as mod pszMod of the runtime; true when it ran without an error.
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

static std::vector<std::string> FixtureRoots() {
    return { s_sRoot + "/Redivivus/Lua/mods", s_sRoot + "/Redivivus/Lua/test/mods" };
}

static void DataStageTests() {
    printf("--- data stage\n");
    ClearLog();
    D2LuaRuntime* pRt = MakeRuntime(D2LUA_SIDE_DATA, FixtureRoots(), { "base", "alpha", "beta" });
    Check(pRt != nullptr, "data stage state created");
    if (!pRt) {
        return;
    }
    CheckLogged("[lua] [alpha] alpha settings stage", "settings.lua runs");
    CheckLogged("[lua] [alpha] alpha data stage, helper doubles 8", "data.lua runs and requires mod files and shared/");
    CheckLogged("[lua] [alpha] alpha final fixes sees 8 beta-shield", "data-final-fixes runs after every mod's data-updates");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(require, 'server.secret') log('data requires server:', ok, e)",
              "the data stage cannot require control-stage code", "the data stage cannot require server/");
    ExpectRun(pRt, "alpha", "log('data globals:', script, remote, storage, registries, type(data.extend))", "data globals: nil nil nil nil function",
              "the data stage has data and no control-stage API");
    D2LuaRt_Destroy(pRt);
}

static void ClientStageTests() {
    printf("--- client stage\n");
    D2LuaRuntime* pRt = MakeRuntime(D2LUA_SIDE_CLIENT, FixtureRoots(), { "base", "alpha" });
    Check(pRt != nullptr, "client stage state created");
    if (!pRt) {
        return;
    }
    ExpectRun(pRt, "alpha", "log('client ui:', require('client.ui').side)", "client ui: client", "client code requires client/");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(require, 'server.secret') log('client requires server:', ok, e)",
              "client code cannot require server/", "client code cannot require server/");
    ExpectRun(pRt, "alpha", "log('client globals:', type(script.on_event), storage, registries, test_world, type(remote.call))",
              "client globals: function nil nil nil function", "the client has its own API, without the server's");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(script.on_event, 'on_test_unit_damaged', print) log('client event:', ok, e)",
              "no event 'on_test_unit_damaged' on this side", "server events are unknown on the client");
    D2LuaRt_Destroy(pRt);
}

static void RuntimeTests() {
    printf("--- server runtime\n");
    TestWorld_Clear();
    ClearLog();
    D2LuaRuntime* pRt = MakeRuntime(D2LUA_SIDE_SERVER, FixtureRoots(), { "base", "alpha", "beta", "gamma", "faulty" });
    Check(pRt != nullptr, "server state created over base and the fixtures");
    if (!pRt) {
        return;
    }
    Check(Order(D2LuaRt_Mods(pRt)) == "base alpha beta faulty gamma", "runtime load order");
    CheckLogged("[lua] [alpha] alpha control 42 2 true", "control.lua runs; own and base modules; one instance per mod");
    CheckLogged("[lua] [beta] beta control 10 beta", "a declared dependency's shared module, instantiated for the requirer");
    CheckLogged("mod 'gamma' does not depend on 'alpha'", "requiring a mod without declaring it fails");

    // require rules
    ExpectRun(pRt, "alpha", "log('server secret:', require('server.secret').side)", "server secret: server", "server code requires server/");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(require, 'client/ui') log('server requires client:', ok, e)",
              "server code cannot require client/", "server code cannot require client/");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(require, 'prototypes.items') log('server requires data file:', ok, e)",
              "server code can require modules under server/ and shared/ only", "server code cannot require data-stage files");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(require, '__beta__/shared/data_bits') log('alpha requires beta:', ok, e)",
              "mod 'alpha' does not depend on 'beta'", "no require into a mod that is not a dependency");
    ExpectRun(pRt, "beta", "log('beta requires alpha server:', require('__alpha__.server.secret').side)", "beta requires alpha server: server",
              "a dependency's server/ is reachable from server code");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(require, '../escape') log('dotdot:', ok, e)", "empty path segment", "'..' is refused");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(require, 'shared.nope') log('missing:', ok, e)", "module not found", "a missing module");
    ExpectRun(pRt, "alpha", "log('cached:', require('shared.helper') == require('shared/helper.lua'), require('shared.helper').loads)",
              "cached: true 1", "modules are cached per mod");

    // API surface
    ExpectRun(pRt, "alpha", "log('mod_name:', script.mod_name, pcall(function() script.mod_name = 'x' end))", "mod_name: alpha false",
              "script.mod_name is the mod's and read-only");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(function() return script.nope end) log('unknown member:', e)", "script has no member 'nope'",
              "unknown members raise");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(script.on_event, 'on_tik', print) log('misspelled:', e)", "no event 'on_tik' on this side",
              "a misspelled event name is refused");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(script.on_event, 'on_tick', 5) log('bad handler:', e)", "function expected, got number",
              "a handler of the wrong type is refused");

    // lifecycle and events
    std::string sError;
    ClearLog();
    D2LuaRt_Start(pRt, nullptr, sError);
    ExpectRun(pRt, "alpha", "log('after init:', storage.counter, storage.inits)", "after init: 0 1", "on_init ran with fresh storage");
    ClearLog();
    for (int i = 0; i < 4; i++) {
        D2LuaRt_Tick(pRt);
    }
    CheckLogged("mod 'faulty' ran past its time budget", "a handler that spins is stopped");
    Check(D2LuaRt_IsFaulted(pRt, "faulty") && !D2LuaRt_IsFaulted(pRt, "gamma"), "  only that mod is faulted");
    ExpectRun(pRt, "alpha", "log('ticks alpha:', storage.counter)", "ticks alpha: 4", "  a mod before it saw every tick");
    ExpectRun(pRt, "gamma", "log('ticks gamma:', gamma_ticks)", "ticks gamma: 4", "  and a mod after it too");

    // remote
    ExpectRun(pRt, "beta", "log('remote add:', remote.call('alpha', 'add', 2, 3), remote.call('alpha', 'whoami'))", "remote add: 5 alpha",
              "remote.call runs the interface's function");
    ExpectRun(pRt, "beta", "local ok, e = pcall(remote.call, 'alpha', 'fail') log('remote fail:', ok, e)", "remote fail: false",
              "an error in a remote function reaches the caller");
    ExpectRun(pRt, "beta", "local ok, e = pcall(remote.call, 'nope', 'x') log('remote unknown:', e)", "unknown interface 'nope'",
              "an unknown interface");
    ExpectRun(pRt, "beta", "log('interfaces:', remote.interfaces.alpha.add, remote.interfaces.alpha.nope)", "interfaces: true nil",
              "remote.interfaces lists functions");
    ExpectRun(pRt, "gamma", "local ok, e = pcall(remote.add_interface, 'alpha', {}) log('dup:', e)", "already exists (added by mod 'alpha')",
              "an interface name is taken once");

    // registries
    printf("--- registries\n");
    ExpectRun(pRt, "gamma", "log('greet native:', registries.example.call('base:greet', 'x'))", "greet native: hello, x",
              "a native entry at the bottom of the chain");
    // beta wraps before alpha does, but alpha loads first, so beta's wrapper ends up outside.
    Run(pRt, "beta", "registries.example.wrap('base:greet', function(next, name) return '<b ' .. next(name) .. '>' end)");
    Run(pRt, "alpha", "registries.example.wrap('base:greet', function(next, name) return '[a ' .. next(name) .. ']' end)");
    ExpectRun(pRt, "gamma", "log('greet wrapped:', registries.example.call('base:greet', 'x'))", "greet wrapped: <b [a hello, x]>",
              "two mods wrap one entry in load order, whatever order they wrapped in");
    Run(pRt, "gamma", "registries.example.wrap('base:greet', function(next, name) error('gamma wrapper breaks') end)");
    ExpectRun(pRt, "gamma", "log('greet faulty wrapper:', registries.example.call('base:greet', 'x'))", "greet faulty wrapper: <b [a hello, x]>",
              "a wrapper that errors falls through with the original arguments");
    CheckLogged("a wrapper on example 'base:greet' failed and is removed from the chain", "  and is logged and removed");
    ExpectRun(pRt, "gamma", "log('greet after removal:', registries.example.call('base:greet', 'y'))", "greet after removal: <b [a hello, y]>",
              "  the rest of the chain keeps working");
    Check(!Logged("failed and is removed"), "  the removed wrapper does not run again");
    Run(pRt, "alpha", "registries.example.register('base:greet', function(name) return 'hi ' .. name end)");
    ExpectRun(pRt, "gamma", "log('greet replaced:', registries.example.call('base:greet', 'z'))", "greet replaced: <b [a hi z]>",
              "replace-then-wrap: register replaces the bottom, wrappers stay");
    Run(pRt, "beta", "registries.example.register('beta:boom', function() error('boom at the bottom') end)"
                     " registries.example.wrap('beta:boom', function(next) beta_wrapped = (beta_wrapped or 0) + 1 return next() end)");
    ExpectRun(pRt, "gamma", "local ok, e = pcall(registries.example.call, 'beta:boom') log('bottom error:', ok, e)", "boom at the bottom",
              "an error from the bottom reaches the caller as itself");
    ExpectRun(pRt, "beta", "pcall(registries.example.call, 'beta:boom') log('wrapper kept:', beta_wrapped)", "wrapper kept: 2",
              "  and the wrapper it passed through is not removed");
    ExpectRun(pRt, "gamma", "local ok, e = pcall(registries.example.wrap, 'nope:x', print) log('wrap unknown:', e)", "has no entry 'nope:x'",
              "wrapping an id that does not exist");
    ExpectRun(pRt, "gamma", "log('slot:', registries.example_slot.call(21))", "slot: 42", "a slot registry's native");
    Run(pRt, "alpha", "registries.example_slot.wrap(function(next, v) return next(v) + 1 end)");
    ExpectRun(pRt, "gamma", "log('slot wrapped:', registries.example_slot.call(21))", "slot wrapped: 43", "  wrap(fn) on a slot");
    Run(pRt, "beta", "registries.example_slot.set(function(v) return v end)");
    ExpectRun(pRt, "gamma", "log('slot set:', registries.example_slot.call(21))", "slot set: 22", "  set(impl) keeps the wrapper");
    {
        lua_State* L = D2LuaRt_State(pRt);
        lua_pushstring(L, "host");
        int nResult = D2LuaRt_CallRegistry(pRt, "example", "base:greet", 1, 1);
        Check(nResult == D2LUA_OK && lua_isstring(L, -1) && strcmp(lua_tostring(L, -1), "<b [a hi host]>") == 0,
              "the host calls a registry entry through the chain");
        if (nResult == D2LUA_OK) {
            lua_pop(L, 1);
        }
    }

    // handles
    printf("--- handles\n");
    ExpectRun(pRt, "alpha",
              "local u = test_world.spawn('zombie', TestUnitKind.Monster, 10) unit1 = u "
              "log('unit:', u.valid, u.name, u.kind == TestUnitKind.Monster, u.health, u.object_name)",
              "unit: true zombie true 10 TestUnit", "a handle's read-only properties");
    ExpectRun(pRt, "alpha", "unit1.health = 5 log('health set:', unit1.health)", "health set: 5", "a read-write property");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(function() unit1.name = 'x' end) log('ro:', e)", "TestUnit.name is read-only",
              "writing a read-only property raises");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(function() return unit1.nope end) log('unknown:', e)", "TestUnit has no member 'nope'",
              "an unknown member raises");
    ExpectRun(pRt, "alpha", "log('nearest alone:', unit1:find_nearest())", "nearest alone: nil", "an optional result: nil");
    ExpectRun(pRt, "alpha",
              "unit2 = test_world.spawn('guard', TestUnitKind.Npc, 50) "
              "log('nearest:', unit1:find_nearest().name, unit1:find_nearest(TestUnitKind.Player), "
              "unit1:find_nearest(TestUnitKind.Npc) == unit2)",
              "nearest: guard nil true", "an optional result: a handle, filtered by an enum argument");
    ExpectRun(pRt, "alpha", "log('interned:', test_world.get_unit(unit1.id) == unit1, rawequal(test_world.get_unit(unit1.id), unit1))",
              "interned: true true", "the same object gives the same handle");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(test_world.spawn, 'x', 7, 1) log('enum:', e)", "7 is not a TestUnitKind",
              "an enum argument outside the enum");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(unit1.damage, {}, 1) log('self:', e)", "TestUnit expected, got table",
              "a method called on something else");
    ExpectRun(pRt, "alpha",
              "script.on_event('on_test_unit_damaged', function(e) log('damaged:', e.name, e.unit.valid, e.amount, e.killed) end)"
              " unit1:damage(2)",
              "damaged: on_test_unit_damaged true 2 false", "an event carrying a handle");
    ExpectRun(pRt, "alpha", "log('died:', unit1:damage(100), unit1.valid)", "died: true false", "  the event of a kill, then the handle is invalid");
    CheckLogged("damaged: on_test_unit_damaged false 100 true", "  inside the handler the killed unit's handle is already invalid");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(function() return unit1.health end) log('use after death:', ok, e)",
              "TestUnit.health: this TestUnit is not valid", "use after invalidation is a clean error");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(unit1.damage, unit1, 1) log('method after death:', ok, e)",
              "TestUnit.damage: this TestUnit is not valid", "  for methods too");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(function() unit1.health = 1 end) log('set after death:', ok, e)",
              "this TestUnit is not valid", "  and setters");
    ExpectRun(pRt, "alpha", "log('tostring:', tostring(unit1))", "tostring: TestUnit: invalid", "tostring of an invalid handle");
    {
        uint64_t nId = 0;
        lua_State* L = D2LuaRt_State(pRt);
        Run(pRt, "alpha", "unit3 = test_world.spawn('host-removed', TestUnitKind.Monster, 1) unit3_id = unit3.id");
        D2LuaHost_NewModEnv(L, "alpha");
        lua_getfield(L, -1, "unit3_id");
        nId = (uint64_t)lua_tonumber(L, -1);
        lua_pop(L, 2);
        TestWorld_Remove(nId);
        ExpectRun(pRt, "alpha", "log('host removed:', unit3.valid, pcall(function() return unit3.name end))", "host removed: false false",
                  "an object the host removes invalidates its handles");
    }
    ExpectRun(pRt, "gamma", "log('formula:', registries.example.call('test:damage_formula', test_world.get_unit(" "2" "), 30))",
              "formula: 15", "a native registry entry taking a handle");

    // storage
    printf("--- storage\n");
    Run(pRt, "alpha",
        "storage.cycle = {} storage.cycle.self = storage.cycle storage.shared = { 1, 2 } storage.again = storage.shared "
        "storage.nan = 0/0 storage[1] = true storage[2.5] = 'half' storage[true] = 'yes' storage.unit = unit2 "
        "storage.dead = unit1 storage.deep = { a = { b = { c = 'deep' } } }");
    std::string sSaved;
    bool bSaved = D2LuaRt_SaveStorage(pRt, sSaved, sError);
    Check(bSaved && sSaved.size() > 20, "storage saves");
    if (!bSaved) {
        printf("  %s\n", sError.c_str());
    }
    std::string sScratch;
    Run(pRt, "beta", "storage.f = function() end");
    Check(!D2LuaRt_SaveStorage(pRt, sScratch, sError) && sError.find("cannot save a function (at storage of mod 'beta': storage.f)") != std::string::npos,
          "a function in storage is refused with its path");
    Run(pRt, "beta", "storage.f = nil storage.t = { x = { coroutine.create(print) } }");
    Check(!D2LuaRt_SaveStorage(pRt, sScratch, sError) && sError.find("cannot save a thread (at storage of mod 'beta': storage.t.x[1])") != std::string::npos,
          "a coroutine is refused with its path");
    Run(pRt, "beta", "storage.t = setmetatable({}, {})");
    Check(!D2LuaRt_SaveStorage(pRt, sScratch, sError) && sError.find("with a metatable") != std::string::npos, "a table with a metatable is refused");
    Run(pRt, "beta", "storage.t = { [{}] = 1 }");
    Check(!D2LuaRt_SaveStorage(pRt, sScratch, sError) && sError.find("as a table key") != std::string::npos, "a table key is refused");
    Run(pRt, "beta", "storage.t = nil");

    D2LuaRuntime* pLoaded = MakeRuntime(D2LUA_SIDE_SERVER, FixtureRoots(), { "base", "alpha", "beta", "gamma", "faulty" });
    ClearLog();
    bool bStarted = pLoaded && D2LuaRt_Start(pLoaded, &sSaved, sError);
    Check(bStarted && Logged("[lua] [alpha] alpha on_load, counter 4"), "storage restores into a fresh state; on_load runs");
    if (pLoaded) {
        ExpectRun(pLoaded, "alpha",
                  "log('restored:', storage.cycle.self == storage.cycle, storage.again == storage.shared, storage.shared[2], "
                  "storage.nan ~= storage.nan, storage[1], storage[2.5], storage[true], storage.deep.a.b.c)",
                  "restored: true true 2 true true half yes deep", "  cycles, shared tables, NaN and key types survive");
        ExpectRun(pLoaded, "alpha", "log('restored handles:', storage.unit.valid, storage.unit.name, storage.dead.valid)",
                  "restored handles: true guard false", "  handles come back as handles");
        ExpectRun(pLoaded, "alpha", "log('no init on load:', storage.inits)", "no init on load: 1", "  on_init does not run on load");
        std::string sBad = sSaved.substr(0, sSaved.size() - 3);
        D2LuaRuntime* pBad = MakeRuntime(D2LUA_SIDE_SERVER, FixtureRoots(), { "base", "alpha" });
        Check(pBad && !D2LuaRt_Start(pBad, &sBad, sError) && sError.find("corrupt save") != std::string::npos, "a truncated save is refused");
        D2LuaRt_Destroy(pBad);
        D2LuaRt_Destroy(pLoaded);
    }

    // reload
    ClearLog();
    Run(pRt, "beta", "beta_marker = 'before reload'");
    bool bReloaded = D2LuaRt_Reload(pRt, sError);
    Check(bReloaded && Logged("[lua] [alpha] alpha on_load, counter 4"), "reload rebuilds the control stage and runs on_load");
    ExpectRun(pRt, "beta", "log('after reload:', beta_marker, storage.counter)", "after reload: nil nil", "  globals start over");
    ExpectRun(pRt, "alpha", "log('reloaded storage:', storage.counter, storage.unit.name)", "reloaded storage: 4 guard", "  storage is handed back");
    ExpectRun(pRt, "gamma", "log('greet after reload:', registries.example.call('base:greet', 'r'))", "greet after reload: hello, r",
              "  registries start from the natives again");
    D2LuaRt_Tick(pRt);
    ExpectRun(pRt, "alpha", "log('ticking after reload:', storage.counter)", "ticking after reload: 5", "  events flow after a reload");

    // the last check here faults alpha: a remote function that spins
    ClearLog();
    Run(pRt, "gamma", "remote.call('alpha', 'spin')");
    Check(D2LuaRt_IsFaulted(pRt, "alpha") && !D2LuaRt_IsFaulted(pRt, "gamma") && Logged("mod 'alpha' ran past"),
          "a remote function that spins faults the mod that owns it, not the caller");
    D2LuaRt_Destroy(pRt);
}

// --- scripts ----------------------------------------------------------------------------------------

static const char s_szLuaScript[] = R"LUA(
local function deep3() return script.wait(2) end
local function deep2() return deep3() end
local function deep1() return deep2() end
remote.add_interface("waiter", { wait = function() return script.wait(1) end })
script.start(function()
  log("script: start", script.tick)
  coroutine.yield()
  log("script: bare yield resumed", script.tick)
  log("script: nested wait gives", deep1())
  local ok, v = pcall(function() return script.wait(1) end)
  log("script: wait inside pcall", ok, v)
  local ok2, v2 = xpcall(function() return script.wait(1) end, function(e) return "handled " .. tostring(e) end)
  log("script: wait inside xpcall", ok2, v2)
  for k in function(_, prev) if prev == nil then script.wait(1) return "k" end end do
    log("script: wait inside an iterator", k, script.tick)
  end
  local t = setmetatable({}, { __index = function(_, key) return script.wait(1) end })
  log("script: wait inside __index", t.x)
  local e = script.wait_event("on_test_unit_damaged")
  log("script: waited for an event", e.amount, e.unit.name)
  local r = coroutine.yield({ ticks = 3 })
  log("script: yielded a wait value, outcome", r)
  log("script: across table.sort", pcall(table.sort, { 3, 2, 1 }, function(a, b) script.wait(1) return a < b end))
  log("script: across gsub", pcall(string.gsub, "ab", "%w", function(c) script.wait(1) return c end))
  log("script: across tostring", pcall(tostring, setmetatable({}, { __tostring = function() script.wait(1) return "x" end })))
  log("script: across remote.call", pcall(remote.call, "waiter", "wait"))
  log("script: across require", pcall(require, "shared.waits"))
  log("script: done", script.tick)
end)
)LUA";

static void ScriptTests() {
    printf("--- scripts (Lua)\n");
    TestWorld_Clear();
    D2LuaRuntime* pRt = MakeRuntime(D2LUA_SIDE_SERVER, FixtureRoots(), { "base", "alpha", "beta", "gamma" });
    if (!pRt) {
        Check(0, "scripts: runtime");
        return;
    }
    std::string sError;
    D2LuaRt_Start(pRt, nullptr, sError);
    ClearLog();
    ExpectRun(pRt, "beta", s_szLuaScript, nullptr, "a plain Lua script starts");
    Check(!Logged("script: start"), "  it does not run inside the call that starts it");
    ClearLog();
    for (int i = 0; i < 9; i++) {
        D2LuaRt_Tick(pRt);
    }
    CheckLogged("[beta] script: start 1", "  first resume on the next tick");
    CheckLogged("[beta] script: bare yield resumed 2", "  a bare yield resumes on the next tick");
    CheckLogged("[beta] script: nested wait gives 4", "  a wait three plain calls deep, its outcome the tick");
    CheckLogged("[beta] script: wait inside pcall true 5", "  a wait inside pcall");
    CheckLogged("[beta] script: wait inside xpcall true 6", "  a wait inside the mods' xpcall");
    CheckLogged("[beta] script: wait inside an iterator k 7", "  a wait inside a for-in iterator");
    CheckLogged("[beta] script: wait inside __index 8", "  a wait inside a metamethod");
    Check(!Logged("script: waited for an event"), "  waiting for an event does not resume on ticks");
    ClearLog();
    Run(pRt, "beta", "local u = test_world.spawn('target', TestUnitKind.Monster, 10) u:damage(4)");
    CheckLogged("[beta] script: waited for an event 4 target", "  wait_event resumes with the payload");
    ClearLog();
    for (int i = 0; i < 4; i++) {
        D2LuaRt_Tick(pRt);
    }
    CheckLogged("[beta] script: yielded a wait value, outcome 12", "  yielding { ticks = 3 } resumes 3 ticks later with the tick");
    CheckLogged("script: across table.sort false attempt to yield across C-call boundary", "  table.sort's comparator cannot yield");
    CheckLogged("script: across gsub false attempt to yield across C-call boundary", "  gsub's callback cannot yield");
    CheckLogged("script: across tostring true x", "  __tostring through tostring can yield (LuaJIT resumes metamethods)");
    CheckLogged("script: across remote.call false attempt to yield across C-call boundary", "  a remote function cannot yield");
    CheckLogged("script: across require false attempt to yield across C-call boundary", "  a module's main chunk cannot yield");
    CheckLogged("[beta] script: done 13", "  the script runs to its end");

    ExpectRun(pRt, "alpha", "local ok, e = pcall(script.wait) log('outside:', e)", "script.wait can only be used inside a script",
              "waiting outside a script is a clean error");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(coroutine.wrap(function() script.wait() end)) log('own coroutine:', e)",
              "can only be used inside a script", "  also from a mod's own coroutine");
    Run(pRt, "alpha", "script.on_event('on_tick', function() script.wait(1) end)");
    ClearLog();
    D2LuaRt_Tick(pRt);
    CheckLogged("script.wait can only be used inside a script", "  and from an event handler the host calls");
    Run(pRt, "alpha", "script.on_event('on_tick', nil)");

    // per-resume budget: each resume does a bit more than a third of the budget of work, five
    // times, so the script as a whole runs well past one budget.
    {
        lua_State* L = D2LuaRt_State(pRt);
        Run(pRt, "gamma", "function spin(n) local s = 0 for i = 1, n do s = s + i % 3 end return s end");
        Clock::time_point tStart = Clock::now();
        Run(pRt, "gamma", "spin(4000000)");
        int nMs = ElapsedMs(tStart);
        double nPerMs = 4000000.0 / (nMs > 0 ? nMs : 1);
        long nWork = (long)(nPerMs * kBudgetMs * 0.4);
        char szScript[512];
        snprintf(szScript, sizeof(szScript),
                 "script.start(function() for i = 1, 5 do spin(%ld) script.wait() end log('long script finished') end)", nWork);
        Run(pRt, "gamma", szScript);
        ClearLog();
        tStart = Clock::now();
        for (int i = 0; i < 7; i++) {
            D2LuaRt_Tick(pRt);
        }
        int nTotal = ElapsedMs(tStart);
        printf("  five resumes took %d ms in all (budget %u ms per entry)\n", nTotal, kBudgetMs);
        Check(Logged("long script finished") && !D2LuaRt_IsFaulted(pRt, "gamma") && nTotal > (int)kBudgetMs,
              "the budget is per resume, not per script");
        (void)L;
    }

    // a script that spins faults its owner
    Run(pRt, "gamma", "script.start(function() script.wait() while true do end end)");
    ClearLog();
    D2LuaRt_Tick(pRt);
    D2LuaRt_Tick(pRt);
    Check(D2LuaRt_IsFaulted(pRt, "gamma") && Logged("mod 'gamma' ran past"), "a script that spins faults the mod that started it");
    D2LuaRt_Destroy(pRt);

    // coroutine ownership through a remote call
    pRt = MakeRuntime(D2LUA_SIDE_SERVER, FixtureRoots(), { "base", "alpha", "beta", "gamma" });
    D2LuaRt_Start(pRt, nullptr, sError);
    Run(pRt, "alpha",
        "local co = coroutine.create(function() coroutine.yield('first') while true do end end) "
        "remote.add_interface('alpha_co', { get = function() return co end })");
    ExpectRun(pRt, "beta", "local co = remote.call('alpha_co', 'get') log('resume 1:', coroutine.resume(co))", "resume 1: true first",
              "another mod resumes a coroutine alpha created");
    ClearLog();
    Run(pRt, "beta", "local co = remote.call('alpha_co', 'get') coroutine.resume(co)");
    Check(D2LuaRt_IsFaulted(pRt, "alpha") && !D2LuaRt_IsFaulted(pRt, "beta") && Logged("mod 'alpha' ran past")
              && Logged("which stays enabled"),
          "  a fault inside it is charged to alpha, the creator, not beta, the resumer");
    D2LuaRt_Destroy(pRt);

    // a wrapper that runs out of time faults its mod, and the entry recovers without it
    pRt = MakeRuntime(D2LUA_SIDE_SERVER, FixtureRoots(), { "base", "alpha", "beta", "gamma" });
    D2LuaRt_Start(pRt, nullptr, sError);
    Run(pRt, "alpha", "registries.example.wrap('base:greet', function(next, n) return 'A(' .. next(n) .. ')' end)");
    Run(pRt, "beta", "registries.example.wrap('base:greet', function(next, n) while true do end end)");
    ClearLog();
    Run(pRt, "gamma", "registries.example.call('base:greet', 'q')");
    Check(D2LuaRt_IsFaulted(pRt, "beta") && !D2LuaRt_IsFaulted(pRt, "gamma") && !D2LuaRt_IsFaulted(pRt, "alpha"),
          "a wrapper that spins faults its own mod only");
    ExpectRun(pRt, "gamma", "log('greet without beta:', registries.example.call('base:greet', 'q'))", "greet without beta: A(hello, q)",
              "  the chain carries on without the faulted mod's wrapper");
    D2LuaRt_Destroy(pRt);
}

// --- enums ---------------------------------------------------------------------------------------------

static void EnumTests() {
    printf("--- enums\n");
    D2LuaRuntime* pRt = MakeRuntime(D2LUA_SIDE_SERVER, FixtureRoots(), { "base", "alpha", "beta" });
    if (!pRt) {
        Check(0, "enums: runtime");
        return;
    }
    ExpectRun(pRt, "alpha", "log('enum values:', Event.Tick, TestUnitKind.Npc, ExampleId.Greet, Event.TestUnitDamaged)",
              "enum values: on_tick 1 base:greet on_test_unit_damaged",
              "enums are tables in the environment; the test set's members merge into Event");
    ExpectRun(pRt, "alpha", "local ok, e = pcall(function() Event.Tick = 'x' end) log('enum write:', ok, e, Event.Tick)",
              "enum write: false", "writing an existing member raises");
    CheckLogged("Event is read-only (an enum)", "  with the enum's name");
    ExpectRun(pRt, "alpha", "local ok = pcall(function() TestUnitKind.Dragon = 9 end) log('enum add:', ok, TestUnitKind.Dragon)",
              "enum add: false nil", "adding a member raises");
    ExpectRun(pRt, "alpha", "log('enum metatable:', getmetatable(Event), pcall(setmetatable, Event, {}))", "enum metatable: false false",
              "the metatable is locked");
    ExpectRun(pRt, "alpha",
              "local n, sum = 0, 0 for k, v in pairs(TestUnitKind) do n = n + 1 sum = sum + v end log('enum pairs:', n, sum)",
              "enum pairs: 3 3", "pairs walks an enum's members");
    ExpectRun(pRt, "alpha", "local n = 0 for k in pairs({ a = 1, b = 2 }) do n = n + 1 end log('plain pairs:', n)", "plain pairs: 2",
              "  and plain tables as before");
    ExpectRun(pRt, "alpha", "rawset(TestUnitKind, 'Monster', 42) log('own rawset:', TestUnitKind.Monster)", "own rawset: 42",
              "a rawset into its own proxy changes that mod's view only");
    ExpectRun(pRt, "beta", "log('other mod:', TestUnitKind.Monster)", "other mod: 0", "  every other mod still reads the value");
    ExpectRun(pRt, "beta", "log('registry by enum:', registries.example.call(ExampleId.Greet, 'e'))", "registry by enum: hello, e",
              "plain Lua names a registry id through its enum");
    ExpectRun(pRt, "beta", "log('registry by string:', registries.example.call('base:greet', 's'))",
              "registry by string: hello, s", "  or by the same value as a string");
    ExpectRun(pRt, "alpha",
              "log('engine enums:', UnitType.Monster, PlayerMode.Run, MonsterMode.Run, UiVar.Inventory, AiTargeting.Basic, "
              "QuestId.DenOfEvil, ItemQuality.Unique, DamageType.Cold, StatId.Strength, StatId.ItemArmorPercent, SkillId.Teleport, "
              "DrawMode.Solid, Difficulty.Hell, PlayerClass.Assassin, BodyLoc.LArm, ItemFlag.Ethereal, ItemFlag.Tagged, "
              "MonsterTypeFlag.Champion, Act.V, Act.None)",
              "engine enums: 1 3 15 1 1 1 7 4 0 16 54 5 2 6 5 4194304 -2147483648 4 4 255",
              "engine enums carry the header's values under the generated names");
    ExpectRun(pRt, "alpha", "log('engine enum sentinels:', UnitType.Count, PlayerClass.Count, BodyLoc.NumBodylocs, UiVar.Ui0x1D)",
              "engine enum sentinels: nil nil nil nil", "  excluded members are absent");
    ExpectRun(pRt, "alpha", "local ok = pcall(function() SkillId.Teleport = 1 end) log('engine enum write:', ok, SkillId.Teleport)",
              "engine enum write: false 54", "  and read-only like every enum");
    D2LuaRt_Destroy(pRt);

    pRt = MakeRuntime(D2LUA_SIDE_DATA, FixtureRoots(), { "base", "alpha" });
    if (pRt) {
        ExpectRun(pRt, "alpha", "log('data enums:', UnitType.Item, SkillId.Attack, Event.Tick)", "data enums: 4 0 on_tick",
                  "the data stage has the engine enums");
        D2LuaRt_Destroy(pRt);
    }

    pRt = MakeRuntime(D2LUA_SIDE_CLIENT, FixtureRoots(), { "base", "alpha" });
    if (pRt) {
        ExpectRun(pRt, "alpha", "log('client engine enum:', UiVar.Automap)", "client engine enum: 10", "the client has them too");
        ExpectRun(pRt, "alpha", "log('client enums:', Event.Draw, Event.TestUnitDamaged, TestUnitKind, ExampleId)",
                  "client enums: on_draw nil nil nil", "an enum exists on the sides its declaring file is bound to");
        ExpectRun(pRt, "alpha", "script.on_event(Event.Draw, function(e) log('drawn:', e.name == Event.Draw) end)", nullptr,
                  "a client handler subscribed by enum");
        ClearLog();
        lua_newtable(D2LuaRt_State(pRt));
        D2LuaRt_RaiseEvent(pRt, "on_draw");
        CheckLogged("drawn: true", "  runs with the payload's name equal to the member");
        D2LuaRt_Destroy(pRt);
    }
}

// --- TypeScript mods ----------------------------------------------------------------------------------

static int FindMarkerLine(const std::string& sPath, const char* pszMarker) {
    std::string sText;
    if (!D2LuaMods_ReadFile(sPath, sText)) {
        return -1;
    }
    int nLine = 1;
    size_t nPos = 0;
    size_t nHit = sText.find(pszMarker);
    if (nHit == std::string::npos) {
        return -1;
    }
    while ((nPos = sText.find('\n', nPos)) != std::string::npos && nPos < nHit) {
        nLine++;
        nPos++;
    }
    return nLine;
}

static void TypeScriptTests(const char* pszTsMods) {
    printf("--- TypeScript mods (%s)\n", pszTsMods);
    TestWorld_Clear();
    std::vector<std::string> aRoots = { s_sRoot + "/Redivivus/Lua/mods", pszTsMods, s_sRoot + "/Redivivus/Lua/test/mods-ts" };
    ClearLog();
    D2LuaRuntime* pRt = MakeRuntime(D2LUA_SIDE_SERVER, aRoots, { "base", "first-mod", "second-mod", "lua-user" });
    Check(pRt != nullptr, "the TypeScript mods load");
    if (!pRt) {
        return;
    }
    Check(Order(D2LuaRt_Mods(pRt)) == "base first-mod lua-user second-mod", "  in dependency order");
    CheckLogged("[first-mod] first-mod control loaded, lualib from base", "first-mod's control runs on base's TSTL runtime");
    CheckLogged("[second-mod] second-mod imports from first-mod: hello, second-mod (3 letters doubled: 6)",
                "second-mod uses a typed export of first-mod");
    std::string sError;
    D2LuaRt_Start(pRt, nullptr, sError);
    CheckLogged("[first-mod] first-mod on_init", "on_init from TypeScript");
    CheckLogged("[first-mod] engine enums: 1 54 7", "engine enums from TypeScript");
    CheckLogged("[first-mod] kit settings: 3 slow", "kit settings from TypeScript: typed, with their defaults");
    ExpectRun(pRt, "first-mod", "log('ts channel:', type(require('shared.net').net.implement))", "ts channel: function", "a TypeScript channel loads on the server");
    ExpectRun(pRt, "second-mod", "log('ts registry:', registries.example.call('base:greet', 'ts'))",
              "ts registry: second-mod{first-mod(hello, ts)}", "TypeScript wrappers in load order");
    ExpectRun(pRt, "second-mod", "log('ts registry own id:', registries.example.call('first-mod:shout', 'ts'))",
              "ts registry own id: <TS!>", "an id a mod added by declaration merging, wrapped by another");
    ExpectRun(pRt, "second-mod", "log('ts remote:', remote.call('first-mod', 'add', 4, 5))", "ts remote: 9", "a TypeScript remote interface");
    ClearLog();
    for (int i = 0; i < 12; i++) {
        D2LuaRt_Tick(pRt);
    }
    CheckLogged("[first-mod] ts script: started at 1", "a TS generator script: first resume");
    CheckLogged("[first-mod] ts script: bare yield resumed at 2", "  bare yield waits one tick");
    CheckLogged("[first-mod] ts script: wait value outcome 4", "  yield of a wait value gives its outcome");
    CheckLogged("[first-mod] ts script: nested actions gave 3 ticks, ended at 7", "  yield* into typed actions, nested three deep");
    CheckLogged("[first-mod] ts script: lua action gave 9", "  yield* into an action written in Lua");
    CheckLogged("[first-mod] ts script: done", "  the TS script ends");
    CheckLogged("[first-mod] kit waitUntil: true 3", "a kit script from TypeScript: waitUntil's outcome");
    CheckLogged("[lua-user] lua script: ts action gave 6", "a Lua script calls an action written in TypeScript");
    CheckLogged("[lua-user] lua script: done", "  and ends");
    int nLine = FindMarkerLine(s_sRoot + "/Redivivus/Lua/examples/first-mod/server/control.ts", "THROW-ON-TICK-12");
    char szWant[160];
    snprintf(szWant, sizeof(szWant), "__first-mod__/server/control.ts:%d: first-mod fails on tick 12", nLine);
    CheckLogged(szWant, "a runtime error in a TS mod reports the .ts file and line");
    CheckLogged("__first-mod__/server/control.ts:", "  and the traceback is mapped too");
    D2LuaRt_Destroy(pRt);
}

void RunModTests(const char* pszRoot, const char* pszTsMods) {
    s_sRoot = pszRoot;
    PatternTests();
    LoaderTests();
    DataStageTests();
    ClientStageTests();
    RuntimeTests();
    ScriptTests();
    EnumTests();
    RunKitTests(pszRoot);
    RunBootTests(pszRoot);
    if (pszTsMods) {
        TypeScriptTests(pszTsMods);
    } else {
        printf("SKIP TypeScript mods: no --ts-mods directory\n");
    }
}
#endif
