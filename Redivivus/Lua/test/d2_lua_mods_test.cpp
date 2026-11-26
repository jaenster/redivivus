// The host test's checks of the mod system: step-limited patterns, the loader, the data stage, the
// control stage (one program in two separate states, games, events, registries, storage, reload),
// scripts, enums, the resolver, and the TypeScript mods the mod CLI built. Fixture mods live in
// Redivivus/Lua/test/mods; the TypeScript example mods are built by the mod CLI (--ts-mods).
#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include <stdio.h>
#include <string.h>

#include <chrono>
#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_core.h"
#include "Redivivus/Lua/d2_lua_internal.h"
#include "Redivivus/Lua/d2_lua_mods.h"
#include "Redivivus/Lua/d2_lua_runtime.h"
#include "Redivivus/Lua/generated/d2_lua_api_main.gen.h"
#include "Redivivus/Lua/test/d2_lua_test_util.h"
#include "Redivivus/Lua/test/d2_lua_test_world.h"
#include "Redivivus/Lua/test/generated/d2_lua_api_test.gen.h"
#include "lua.hpp"

typedef std::chrono::steady_clock Clock;

std::string g_sTestRoot;

static int ElapsedMs(Clock::time_point tStart) {
    return (int)std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - tStart).count();
}

void CheckLogged(const char* pszText, const char* pszWhat) {
    bool bOk = Logged(pszText);
    if (!bOk) {
        printf("  (log lacks: %s)\n", pszText);
    }
    Check(bOk, pszWhat);
}

static const D2LuaApiDesc* const s_apApis[] = { &g_D2LuaApi_main, &g_D2LuaApi_test };

std::vector<std::string> TestRoots() {
    return { g_sTestRoot + "/Redivivus/Lua/test/mods" };
}

D2LuaRuntime* TestMakeRuntime(int nSide, const std::vector<std::string>& aRoots, const std::vector<std::string>& aOnly,
                              const std::string& sSettingsFile, const std::vector<std::string>& aTestGrant,
                              const std::vector<std::string>& aDriverGrant) {
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
    tDesc.tLimits.nBudgetMs = kTestBudgetMs;
    tDesc.tLimits.nMemoryBytes = (size_t)64 * 1024 * 1024;
    tDesc.aApis.assign(s_apApis, s_apApis + 2);
    tDesc.sSettingsFile = sSettingsFile;
    tDesc.aTestGrant = aTestGrant;
    tDesc.aDriverGrant = aDriverGrant;
    if (!D2LuaMods_Resolve(aPicked, tDesc.aMods, aErrors)) {
        for (const auto& e : aErrors) {
            printf("  resolve: %s\n", e.c_str());
        }
        return nullptr;
    }
    D2LuaRuntime* pRt = D2LuaRt_Create(tDesc);
#if defined(D2_LUAJIT_CHECKHOOK) && !D2_LUAJIT_CHECKHOOK
    // Without LUAJIT_ENABLE_CHECKHOOK a compiled loop never sees the budget's hook, so the checks
    // that make a mod spin would hang. These states run interpreted, where the hook is seen; the
    // compiled case is the host test's (--runaway).
    if (pRt) {
        luaJIT_setmode(D2LuaRt_State(pRt), 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);
    }
#endif
    return pRt;
}

bool TestRun(D2LuaRuntime* pRt, uint32_t nGame, const char* pszMod, const char* pszSource) {
    return D2LuaRt_Run(pRt, nGame, pszMod, pszSource) == D2LUA_OK;
}

void TestExpect(D2LuaRuntime* pRt, uint32_t nGame, const char* pszMod, const char* pszSource, const char* pszLogged,
                const char* pszWhat) {
    ClearLog();
    bool bRan = TestRun(pRt, nGame, pszMod, pszSource);
    bool bOk = bRan && (!pszLogged || Logged(pszLogged));
    if (!bOk) {
        printf("  ran: %d, wanted in log: %s\n", (int)bRan, pszLogged ? pszLogged : "-");
    }
    Check(bOk, pszWhat);
}

void TestStart(D2LuaRuntime* pRt, uint32_t nGame) {
    std::string sError;
    D2LuaRt_StartGame(pRt, nGame, nullptr, sError);
}

static std::string Order(const std::vector<D2LuaModInfo>& aMods) {
    std::string s;
    for (const auto& t : aMods) {
        s += (s.empty() ? "" : " ") + t.sName;
    }
    return s;
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
compare("find nil", pack(pcall(stock.find, nil, "a")), pack(pcall(mine.find, nil, "a")))
compare("gsub bad repl", pack(pcall(stock.gsub, "a", "a", true)), pack(pcall(mine.gsub, "a", "a", true)))
compare("find number subject", pack(pcall(stock.find, 12345, "3")), pack(pcall(mine.find, 12345, "3")))
compare("gmatch nil", pack(pcall(stock.gmatch, "a", nil)), pack(pcall(mine.gmatch, "a", nil)))
return count, bad, table.concat(first, "\n")
)LUA";

static void PatternTests() {
    printf("--- patterns\n");
    D2LuaLimits tLimits = { kTestBudgetMs, (size_t)64 * 1024 * 1024 };
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
    snprintf(szBudget, sizeof(szBudget), "time budget of %u ms", kTestBudgetMs);
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
        Check(nResult == D2LUA_FAULT && Logged(szBudget) && nMs < (int)kTestBudgetMs * 4, tHang.pszWhat);
    }
    ClearLog();
    int nOk = D2LuaHost_RunString(L, "pat_ok", "print('patterns', ('key = value'):match('(%w+)%s*=%s*(%w+)'), ('a,b'):gsub(',', ';'))", "ok");
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

static void LoaderTests() {
    printf("--- loader\n");
    ExpectResolve({ "{\"name\":\"zeta\",\"version\":\"1.0.0\"}", "{\"name\":\"alpha2\",\"version\":\"1\",\"dependencies\":[\"zeta\"]}",
                    "{\"name\":\"mid\",\"version\":\"1\"}" },
                  "redivivus mid zeta alpha2", "load order: redivivus first, dependencies first, ties by name");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"nothere\"]}" },
                  "ERR: mod 'a' requires 'nothere', which is not installed", "missing dependency");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"redivivus >= 2.0\"]}" },
                  "mod 'a' requires 'redivivus >= 2.0.0', but version 0.1.0 is installed", "a redivivus version too new is refused");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"redivivus >= 0.1\"]}" }, "redivivus a",
                  "\"redivivus >= 0.1\" is met by the built-in 0.1.0");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"b < 1.5\"]}", "{\"name\":\"b\",\"version\":\"1.5.0\"}" },
                  "mod 'a' requires 'b < 1.5.0', but version 1.5.0 is installed", "version mismatch (<)");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"b = 1.5\"]}", "{\"name\":\"b\",\"version\":\"1.5.0\"}" },
                  "redivivus b a", "version match (=)");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"! b\"]}", "{\"name\":\"b\",\"version\":\"1\"}" },
                  "mod 'a' is incompatible with 'b', and both are enabled", "incompatibility");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"b\"]}", "{\"name\":\"b\",\"version\":\"1\",\"dependencies\":[\"c\"]}",
                    "{\"name\":\"c\",\"version\":\"1\",\"dependencies\":[\"a\"]}" },
                  "ERR: dependency cycle: a -> b -> c -> a", "a cycle is named in order");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"? nothere\"]}" }, "redivivus a", "absent optional dependency is fine");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"? z\"]}", "{\"name\":\"z\",\"version\":\"1\"}" },
                  "redivivus z a", "present optional dependency orders");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"~ z\"]}", "{\"name\":\"z\",\"version\":\"1\"}" },
                  "redivivus a z", "~ dependency does not order");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"base >= 0.1\"]}" },
                  "requires 'base', which is not installed", "there is no base mod any more");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"? redivivus\"]}" },
                  "PARSE: info.json: redivivus is built into the game and cannot be optional", "redivivus cannot be optional");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"redivivus_version\":\"0.1\"}" }, "PARSE: info.json: \"redivivus_version\" is gone",
                  "the redivivus_version field is refused with the way to say it now");
    ExpectResolve({ "{\"name\":\"redivivus\",\"version\":\"9.0.0\"}" }, "PARSE: info.json: the name \"redivivus\" is reserved (redivivus is built into the game",
                  "a mod named redivivus is refused");
    ExpectResolve({ "{\"name\":\"ReDiViVuS\",\"version\":\"9.0.0\"}" }, "is reserved", "  in any case");
    ExpectResolve({ "{\"name\":\"mod\",\"version\":\"1\"}" }, "PARSE: info.json: the name \"mod\" is reserved", "a mod may not be named mod");
    ExpectResolve({ "{\"name\":\"base\",\"version\":\"1\"}" }, "is reserved", "  nor base");
    ExpectResolve({ "{\"name\":\"redivivus-extras\",\"version\":\"1\"}" }, "is reserved", "  nor anything starting with redivivus");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",\"dependencies\":[\"b >> 1\"]}" }, "PARSE: info.json: dependency \"b >> 1\"", "bad dependency syntax");
    ExpectResolve({ "{\"name\":\"Bad Name\",\"version\":\"1\"}" }, "PARSE: info.json: name \"Bad Name\" is not valid", "bad mod name");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1.x\"}" }, "PARSE: info.json: version \"1.x\"", "bad version");
    ExpectResolve({ "{\"name\":\"a\"}" }, "PARSE: info.json: \"version\" is missing", "missing version");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\",}" }, "PARSE: info.json: line 1", "malformed json names the position");
    ExpectResolve({ "{\"name\":\"a\",\"version\":\"1\"}", "{\"name\":\"a\",\"version\":\"1\"}" }, "ERR: mod 'a' is listed twice", "a mod listed twice");

    std::vector<D2LuaModInfo> aFound;
    std::vector<std::string> aErrors;
    D2LuaMods_Discover(TestRoots(), aFound, aErrors);
    Check(aErrors.empty() && Order(aFound) == "alpha beta evil faulty gamma guardmod kitty worldmod", "discovery finds the fixture mods (redivivus is not on disk)");
    aFound.clear();
    aErrors.clear();
    D2LuaMods_Discover({ g_sTestRoot + "/Redivivus/Lua/test/mods", g_sTestRoot + "/Redivivus/Lua/test/mods" }, aFound, aErrors);
    Check(aErrors.size() == 8 && aErrors[0].find("found twice") != std::string::npos, "a mod found in two roots is an error");
    aFound.clear();
    aErrors.clear();
    D2LuaMods_Discover({ g_sTestRoot + "/Redivivus/Lua/test/boot/mods" }, aFound, aErrors);
    bool bRedivivus = false, bMod = false;
    for (const std::string& e : aErrors) {
        bRedivivus = bRedivivus || e.find("a mod folder named redivivus is not allowed") != std::string::npos;
        bMod = bMod || e.find("the name \"mod\" is reserved") != std::string::npos;
    }
    Check(bRedivivus, "a folder named redivivus under mods/ is refused with a clear error");
    Check(bMod, "a folder holding a mod named mod is refused");
}

// --- the data stage ---------------------------------------------------------------------------------

static void DataStageTests() {
    printf("--- data stage\n");
    ClearLog();
    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_DATA, TestRoots(), { "alpha", "beta" }, "");
    Check(pRt != nullptr, "data stage state created");
    if (!pRt) {
        return;
    }
    Check(Order(D2LuaRt_Mods(pRt)) == "redivivus alpha beta", "redivivus is the first mod of every state");
    CheckLogged("[lua] [alpha] alpha settings stage", "settings.lua runs");
    CheckLogged("[lua] [alpha] alpha data stage, helper doubles 8", "data.lua runs and requires its own modules");
    CheckLogged("[lua] [alpha] alpha final fixes sees 8 beta-shield", "data-final-fixes runs after every mod's data-updates");
    TestExpect(pRt, 0, "alpha", "local ok, e = pcall(require('redivivus:data').data.extend, {}) local log = require('redivivus:shared').log log('extend ok:', ok)",
               "extend ok: true", "data.extend works in the data stage");
    TestExpect(pRt, 0, "alpha", "require('redivivus:shared').log('data globals:', script, remote, storage, registries, log, data, game)",
               "data globals: nil nil nil nil nil nil nil", "no API globals in a mod environment");
    D2LuaRt_Destroy(pRt);
}

// --- one program, two states ----------------------------------------------------------------------

static void ControlTests() {
    printf("--- control stage: one program, separate states\n");
    TestWorld_Clear();
    ClearLog();
    D2LuaRuntime* pServer = TestMakeRuntime(D2LUA_SIDE_SERVER, TestRoots(), { "alpha", "beta", "gamma", "faulty" }, "", { "alpha", "beta", "gamma" });
    D2LuaRuntime* pClient = TestMakeRuntime(D2LUA_SIDE_CLIENT, TestRoots(), { "alpha", "beta", "gamma", "faulty" }, "");
    Check(pServer && pClient, "a server and a client state over the same mods");
    if (!pServer || !pClient) {
        return;
    }
    Check(Order(D2LuaRt_Mods(pServer)) == "redivivus alpha beta faulty gamma", "runtime load order");
    CheckLogged("[lua] [alpha] alpha main 42 true true", "main runs; ./helper, helper and alpha:helper are one instance");
    CheckLogged("[lua] [beta] beta main 10 beta 5", "a dependency's module, instantiated for the requirer; a plain import of another mod's export");
    CheckLogged("mod 'gamma' does not depend on 'alpha'", "requiring a mod without declaring it fails");
    TestExpect(pServer, 0, "alpha", "local s = require('redivivus:server') local c = require('redivivus:client') "
               "require('redivivus:shared').log('both resolve on the server:', type(s), type(c), s.Event.Tick)",
               "both resolve on the server: table table tick", "redivivus:server and redivivus:client both resolve in the server state");
    TestExpect(pClient, 0, "alpha", "local s = require('redivivus:server') require('redivivus:shared').log('client side:', type(s), s.Event.Draw)",
               "client side: table draw", "  and in the client state");

    // Games: the server state calls server(game), the client state client(client).
    ClearLog();
    TestStart(pServer, 1);
    TestStart(pClient, 1);
    CheckLogged("[lua] [alpha] alpha client sees server_wrote = nil", "a variable the server export wrote is not visible to the client export, even in one process");
    TestExpect(pServer, 1, "alpha", "require('redivivus:shared').log('server state:', server_wrote, clients)", "server state: server 1 nil",
               "the server state never called client()");
    TestExpect(pClient, 1, "alpha", "require('redivivus:shared').log('client state:', games, clients[1].gameId)", "client state: nil 1",
               "the client state never called server()");
    TestExpect(pServer, 1, "alpha", "local g = games[1] require('redivivus:shared').log('game:', g.id, g.loaded, g.storage('state', {}).inits)",
               "game: 1 false 1", "the game object: id, loaded, a storage slot made from its defaults");
    ClearLog();
    for (int i = 0; i < 4; i++) {
        D2LuaRt_Tick(pServer, 1);
    }
    CheckLogged("mod 'faulty' ran past its time budget", "a handler that spins is stopped");
    Check(D2LuaRt_IsFaulted(pServer, "faulty") && !D2LuaRt_IsFaulted(pServer, "gamma"), "  only that mod is faulted");
    TestExpect(pServer, 1, "alpha", "require('redivivus:shared').log('ticks alpha:', games[1].storage('state', {}).counter)", "ticks alpha: 4",
               "  a mod before it saw every tick");
    TestExpect(pServer, 1, "gamma", "require('redivivus:shared').log('ticks gamma:', gamma_ticks)", "ticks gamma: 4", "  and a mod after it too");

    // API surface on the game object.
    TestExpect(pServer, 1, "alpha", "local ok, e = pcall(function() return games[1].nope end) require('redivivus:shared').log('unknown member:', e)",
               "game has no member 'nope'", "unknown members of the game raise");
    TestExpect(pServer, 1, "alpha", "local ok, e = pcall(games[1].on, 'tik', print) require('redivivus:shared').log('misspelled:', e)",
               "no event \"tik\" on this side", "a misspelled event name is refused");
    TestExpect(pServer, 1, "alpha", "local ok, e = pcall(games[1].on, 'tick', 5) require('redivivus:shared').log('bad handler:', e)",
               "function or nil expected, got number", "a handler of the wrong type is refused");
    TestExpect(pServer, 1, "alpha", "local ok, e = pcall(games[1].on, 'draw', print) require('redivivus:shared').log('client event on server:', e)",
               "no event \"draw\" on this side", "a client event is not raised on the server");
    TestExpect(pServer, 1, "alpha", "local ok, e = pcall(function() games[1].id = 5 end) require('redivivus:shared').log('ro:', e)",
               "game.id is read-only", "the game object is read-only");
    TestExpect(pServer, 1, "alpha", "local ok, e = pcall(require('redivivus:data').data.extend, {}) require('redivivus:shared').log('extend in control:', e)",
               "data.extend is for the data stage", "a data-stage call outside the data stage raises, naming the stage");

    // Registries, per game.
    printf("--- registries\n");
    TestExpect(pServer, 1, "gamma", "require('redivivus:shared').log('greet native:', games[1].registry.example.call('redivivus:greet', 'x'))",
               "greet native: hello, x", "a native entry at the bottom of the chain");
    TestRun(pServer, 1, "beta", "games[1].registry.example.wrap('redivivus:greet', function(next, name) return '<b ' .. next(name) .. '>' end)");
    TestRun(pServer, 1, "alpha", "games[1].registry.example.wrap('redivivus:greet', function(next, name) return '[a ' .. next(name) .. ']' end)");
    TestExpect(pServer, 1, "gamma", "require('redivivus:shared').log('greet wrapped:', games[1].registry.example.call('redivivus:greet', 'x'))",
               "greet wrapped: <b [a hello, x]>", "two mods wrap one entry in load order, whatever order they wrapped in");
    TestRun(pServer, 1, "gamma", "games[1].registry.example.wrap('redivivus:greet', function(next, name) error('gamma wrapper breaks') end)");
    TestExpect(pServer, 1, "gamma", "require('redivivus:shared').log('greet faulty wrapper:', games[1].registry.example.call('redivivus:greet', 'x'))",
               "greet faulty wrapper: <b [a hello, x]>", "a wrapper that errors falls through with the original arguments");
    CheckLogged("a wrapper on example \"redivivus:greet\" failed and is removed from the chain", "  and is logged and removed");
    TestExpect(pServer, 1, "gamma", "require('redivivus:shared').log('greet after removal:', games[1].registry.example.call('redivivus:greet', 'y'))",
               "greet after removal: <b [a hello, y]>", "  the rest of the chain keeps working");
    Check(!Logged("failed and is removed"), "  the removed wrapper does not run again");
    TestRun(pServer, 1, "alpha", "games[1].registry.example.register('redivivus:greet', function(name) return 'hi ' .. name end)");
    TestExpect(pServer, 1, "gamma", "require('redivivus:shared').log('greet replaced:', games[1].registry.example.call('redivivus:greet', 'z'))",
               "greet replaced: <b [a hi z]>", "replace-then-wrap: register replaces the bottom, wrappers stay");
    TestRun(pServer, 1, "beta", "games[1].registry.example.register('beta:boom', function() error('boom at the bottom') end)"
                                " games[1].registry.example.wrap('beta:boom', function(next) beta_wrapped = (beta_wrapped or 0) + 1 return next() end)");
    TestExpect(pServer, 1, "gamma", "local ok, e = pcall(games[1].registry.example.call, 'beta:boom') require('redivivus:shared').log('bottom error:', ok, e)",
               "boom at the bottom", "an error from the bottom reaches the caller as itself");
    TestExpect(pServer, 1, "beta", "pcall(games[1].registry.example.call, 'beta:boom') require('redivivus:shared').log('wrapper kept:', beta_wrapped)",
               "wrapper kept: 2", "  and the wrapper it passed through is not removed");
    TestExpect(pServer, 1, "gamma", "local ok, e = pcall(games[1].registry.example.wrap, 'nope:x', print) require('redivivus:shared').log('wrap unknown:', e)",
               "has no entry \"nope:x\"", "wrapping an id that does not exist");
    TestExpect(pServer, 1, "gamma", "require('redivivus:shared').log('slot:', games[1].registry.exampleSlot.call(21))", "slot: 42", "a slot registry's native");
    TestRun(pServer, 1, "alpha", "games[1].registry.exampleSlot.wrap(function(next, v) return next(v) + 1 end)");
    TestExpect(pServer, 1, "gamma", "require('redivivus:shared').log('slot wrapped:', games[1].registry.exampleSlot.call(21))", "slot wrapped: 43", "  wrap(fn) on a slot");
    TestRun(pServer, 1, "beta", "games[1].registry.exampleSlot.set(function(v) return v end)");
    TestExpect(pServer, 1, "gamma", "require('redivivus:shared').log('slot set:', games[1].registry.exampleSlot.call(21))", "slot set: 22", "  set(impl) keeps the wrapper");
    {
        lua_State* L = D2LuaRt_State(pServer);
        lua_pushstring(L, "host");
        int nResult = D2LuaRt_CallRegistry(pServer, 1, "example", "redivivus:greet", 1);
        Check(nResult == D2LUA_OK && lua_isstring(L, -1) && strcmp(lua_tostring(L, -1), "<b [a hi host]>") == 0, "the host calls a registry entry through the chain");
        lua_settop(L, 0);
    }

    // Handles of the test world (redivivus:test, granted to these mods by the test runner).
    printf("--- handles\n");
    TestExpect(pServer, 1, "alpha",
               "local T = require('redivivus:test') local L = require('redivivus:shared').log "
               "local u = T.world.spawn('zombie', T.TestUnitKind.Monster, 10) unit1 = u "
               "L('unit:', u.valid, u.name, u.kind == T.TestUnitKind.Monster, u.health, u.objectName)",
               "unit: true zombie true 10 TestUnit", "a handle's read-only properties");
    TestExpect(pServer, 1, "alpha", "unit1.health = 5 require('redivivus:shared').log('health set:', unit1.health)", "health set: 5", "a read-write property");
    TestExpect(pServer, 1, "alpha", "local ok, e = pcall(function() unit1.name = 'x' end) require('redivivus:shared').log('ro:', e)", "TestUnit.name is read-only",
               "writing a read-only property raises");
    TestExpect(pServer, 1, "alpha", "require('redivivus:shared').log('nearest alone:', unit1:findNearest())", "nearest alone: nil", "an optional result: nil");
    TestExpect(pServer, 1, "alpha",
               "local T = require('redivivus:test') unit2 = T.world.spawn('guard', T.TestUnitKind.Npc, 50) "
               "require('redivivus:shared').log('nearest:', unit1:findNearest().name, unit1:findNearest(T.TestUnitKind.Player), unit1:findNearest(T.TestUnitKind.Npc) == unit2)",
               "nearest: guard nil true", "an optional result: a handle, filtered by an enum argument");
    TestExpect(pServer, 1, "alpha", "local T = require('redivivus:test') require('redivivus:shared').log('interned:', rawequal(T.world.getUnit(unit1.id), unit1))",
               "interned: true", "the same object gives the same handle");
    TestExpect(pServer, 1, "alpha", "local T = require('redivivus:test') local ok, e = pcall(T.world.spawn, 'x', 7, 1) require('redivivus:shared').log('enum:', e)",
               "7 is not a TestUnitKind", "an enum argument outside the enum");
    TestExpect(pServer, 1, "alpha", "local ok, e = pcall(unit1.damage, {}, 1) require('redivivus:shared').log('self:', e)", "TestUnit expected, got table",
               "a method called on something else");
    TestExpect(pServer, 1, "alpha",
               "games[1].on('testUnitDamaged', function(e) require('redivivus:shared').log('damaged:', e.name, e.unit.valid, e.amount, e.killed) end) unit1:damage(2)",
               "damaged: testUnitDamaged true 2 false", "an event carrying a handle, raised in the game the call is in");
    TestExpect(pServer, 1, "alpha", "require('redivivus:shared').log('died:', unit1:damage(100), unit1.valid)", "died: true false", "  a kill, then the handle is invalid");
    CheckLogged("damaged: testUnitDamaged false 100 true", "  inside the handler the killed unit's handle is already invalid");
    TestExpect(pServer, 1, "alpha", "local ok, e = pcall(function() return unit1.health end) require('redivivus:shared').log('after death:', ok, e)",
               "TestUnit.health: this TestUnit is not valid", "use after invalidation is a clean error");
    TestExpect(pServer, 1, "gamma", "require('redivivus:shared').log('formula:', games[1].registry.example.call('test:damageFormula', require('redivivus:test').world.getUnit(2), 30))",
               "formula: 15", "a native registry entry taking a handle");

    // Storage.
    printf("--- storage\n");
    TestRun(pServer, 1, "alpha",
            "local s = games[1].storage('bag', {}) s.cycle = {} s.cycle.self = s.cycle s.shared = { 1, 2 } s.again = s.shared "
            "s.nan = 0/0 s[1] = true s[2.5] = 'half' s[true] = 'yes' s.unit = unit2 s.dead = unit1 s.deep = { a = { b = { c = 'deep' } } }");
    std::string sSaved, sError;
    bool bSaved = D2LuaRt_SaveStorage(pServer, 1, sSaved, sError);
    Check(bSaved && sSaved.size() > 20, "storage saves");
    if (!bSaved) {
        printf("  %s\n", sError.c_str());
    }
    std::string sScratch;
    TestRun(pServer, 1, "beta", "games[1].storage('x', {}).f = function() end");
    Check(!D2LuaRt_SaveStorage(pServer, 1, sScratch, sError) && sError.find("cannot save a function (at storage of mod 'beta': storage.x.f)") != std::string::npos,
          "a function in storage is refused with its path");
    TestRun(pServer, 1, "beta", "games[1].storage('x', {}).f = nil games[1].storage('x', {}).t = setmetatable({}, {})");
    Check(!D2LuaRt_SaveStorage(pServer, 1, sScratch, sError) && sError.find("with a metatable") != std::string::npos, "a table with a metatable is refused");
    TestRun(pServer, 1, "beta", "games[1].storage('x', {}).t = nil");
    TestExpect(pServer, 1, "alpha", "local ok, e = pcall(games[1].storage, '__persist', {}) require('redivivus:shared').log('reserved key:', e)",
               "keys starting with __ are the core's", "storage keys starting with __ are the core's");

    D2LuaRuntime* pLoaded = TestMakeRuntime(D2LUA_SIDE_SERVER, TestRoots(), { "alpha", "beta", "gamma" }, "", { "alpha" });
    ClearLog();
    bool bStarted = pLoaded && D2LuaRt_StartGame(pLoaded, 7, &sSaved, sError);
    Check(bStarted && Logged("[lua] [alpha] alpha loaded, counter 4"), "storage restores into a game of a fresh state; game.loaded is true");
    if (pLoaded) {
        TestExpect(pLoaded, 7, "alpha",
                   "local s = games[7].storage('bag', {}) require('redivivus:shared').log('restored:', s.cycle.self == s.cycle, s.again == s.shared, s.shared[2], "
                   "s.nan ~= s.nan, s[1], s[2.5], s[true], s.deep.a.b.c)",
                   "restored: true true 2 true true half yes deep", "  cycles, shared tables, NaN and key types survive");
        TestExpect(pLoaded, 7, "alpha", "local s = games[7].storage('bag', {}) require('redivivus:shared').log('handles:', s.unit.valid, s.unit.name, s.dead.valid)",
                   "handles: true guard false", "  handles come back as handles");
        std::string sBad = sSaved.substr(0, sSaved.size() - 3);
        Check(!D2LuaRt_StartGame(pLoaded, 8, &sBad, sError) && sError.find("corrupt save") != std::string::npos, "a truncated save is refused");
        D2LuaRt_Destroy(pLoaded);
    }

    // Reload of one game.
    ClearLog();
    TestRun(pServer, 1, "alpha", "reload_marker = { games[1] }");
    bool bReloaded = D2LuaRt_ReloadGame(pServer, 1, sError);
    Check(bReloaded && Logged("[lua] [alpha] alpha main 42") && Logged("[lua] [alpha] alpha loaded, counter 4"),
          "reload loads the modules again and calls server(game) with storage handed back");
    TestExpect(pServer, 1, "alpha", "require('redivivus:shared').log('new object:', games[1] ~= reload_marker[1], reload_marker[1].id)",
               "new object: true 1", "  the export gets a new object for the same game");
    TestExpect(pServer, 1, "alpha", "local ok, e = pcall(reload_marker[1].registry.example.call, 'redivivus:greet', 'r') require('redivivus:shared').log('old registry:', ok, e)",
               "old registry: true hello, r", "  what was registered before is gone: the chain is the native entry again");
    D2LuaRt_Tick(pServer, 1);
    TestExpect(pServer, 1, "alpha", "require('redivivus:shared').log('ticking after reload:', games[1].storage('state', {}).counter)",
               "ticking after reload: 5", "  the new handler runs, once");

    // Ending the game drops what was registered through it.
    Check(D2LuaRt_EndGame(pServer, 1), "the game ends");
    TestExpect(pServer, 1, "alpha", "local ok, e = pcall(games[1].on, 'tick', print) require('redivivus:shared').log('ended:', e)",
               "this game has ended", "an object of an ended game refuses");
    D2LuaRt_Destroy(pClient);
    D2LuaRt_Destroy(pServer);
}

// --- two games in one state ------------------------------------------------------------------------

static void TwoGameTests() {
    printf("--- two games in one state\n");
    TestWorld_Clear();
    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, TestRoots(), { "alpha", "beta", "gamma" }, "", { "alpha" });
    if (!pRt) {
        Check(0, "two games: runtime");
        return;
    }
    TestStart(pRt, 1);
    TestStart(pRt, 2);
    TestExpect(pRt, 0, "alpha", "require('redivivus:shared').log('two objects:', games[1].id, games[2].id, games[1] ~= games[2])", "two objects: 1 2 true",
               "one loaded mod, its export called once per game, a separate object each");
    for (int i = 0; i < 3; i++) {
        D2LuaRt_Tick(pRt, 1);
    }
    D2LuaRt_Tick(pRt, 2);
    TestExpect(pRt, 0, "alpha", "require('redivivus:shared').log('counters:', games[1].storage('state', {}).counter, games[2].storage('state', {}).counter)",
               "counters: 3 1", "handlers and storage are per game");
    TestRun(pRt, 1, "alpha", "games[1].registry.example.wrap('redivivus:greet', function(next, n) return 'G1(' .. next(n) .. ')' end)");
    TestExpect(pRt, 0, "alpha", "require('redivivus:shared').log('registries:', games[1].registry.example.call('redivivus:greet', 'x'), games[2].registry.example.call('redivivus:greet', 'x'))",
               "registries: G1(hello, x) hello, x", "a wrapper registered in one game is not in the other");
    TestRun(pRt, 2, "alpha", "games[2].startScript(function() while true do require('redivivus:shared').wait() g2_runs = (g2_runs or 0) + 1 end end)");
    D2LuaRt_Tick(pRt, 1);
    D2LuaRt_Tick(pRt, 1);
    TestExpect(pRt, 0, "alpha", "require('redivivus:shared').log('g2 script after g1 ticks:', g2_runs)", "g2 script after g1 ticks: nil",
               "a script of one game is not resumed by the other's ticks");
    D2LuaRt_Tick(pRt, 2);
    D2LuaRt_Tick(pRt, 2);
    TestExpect(pRt, 0, "alpha", "require('redivivus:shared').log('g2 script:', g2_runs)", "g2 script: 1", "  it runs on its own game's ticks");
    std::string sError;
    Check(D2LuaRt_ReloadGame(pRt, 2, sError), "reload one game");
    TestExpect(pRt, 0, "alpha", "require('redivivus:shared').log('after reloading 2:', games[1].registry.example.call('redivivus:greet', 'y'))",
               "after reloading 2: G1(hello, y)", "  the other game keeps its registrations");
    D2LuaRt_EndGame(pRt, 1);
    D2LuaRt_Tick(pRt, 2);
    TestExpect(pRt, 0, "alpha", "require('redivivus:shared').log('game 2 after game 1 ended:', games[2].storage('state', {}).counter, pcall(games[1].on, 'tick', print))",
               "game 2 after game 1 ended: 4 false", "ending one game leaves the other intact");
    D2LuaRt_Destroy(pRt);
}

// --- scripts ----------------------------------------------------------------------------------------

static const char s_szLuaScript[] = R"LUA(
local S = require("redivivus:shared")
local log, wait = S.log, S.wait
local g = games[1]
local function deep3() return wait(2) end
local function deep2() return deep3() end
local function deep1() return deep2() end
g.startScript(function()
  log("script: start", g.tick)
  coroutine.yield()
  log("script: bare yield resumed", g.tick)
  log("script: nested wait gives", deep1())
  local ok, v = pcall(function() return wait(1) end)
  log("script: wait inside pcall", ok, v)
  local ok2, v2 = xpcall(function() return wait(1) end, function(e) return "handled " .. tostring(e) end)
  log("script: wait inside xpcall", ok2, v2)
  for k in function(_, prev) if prev == nil then wait(1) return "k" end end do
    log("script: wait inside an iterator", k, g.tick)
  end
  local t = setmetatable({}, { __index = function(_, key) return wait(1) end })
  log("script: wait inside __index", t.x)
  local e = g.waitEvent("testUnitDamaged")
  log("script: waited for an event", e.amount, e.unit.name)
  local r = coroutine.yield({ ticks = 3 })
  log("script: yielded a wait value, outcome", r)
  log("script: across table.sort", pcall(table.sort, { 3, 2, 1 }, function(a, b) wait(1) return a < b end))
  log("script: across gsub", pcall(string.gsub, "ab", "%w", function(c) wait(1) return c end))
  log("script: across a registry call", pcall(g.registry.example.call, "beta:waits"))
  log("script: across require", pcall(require, "beta:waits"))
  log("script: done", g.tick)
end)
)LUA";

static void ScriptTests() {
    printf("--- scripts (Lua)\n");
    TestWorld_Clear();
    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, TestRoots(), { "alpha", "beta", "gamma" }, "", { "beta" });
    if (!pRt) {
        Check(0, "scripts: runtime");
        return;
    }
    TestStart(pRt, 1);
    TestRun(pRt, 1, "beta", "games[1].registry.example.register('beta:waits', function() return require('redivivus:shared').wait(1) end)");
    ClearLog();
    TestExpect(pRt, 1, "beta", s_szLuaScript, nullptr, "a plain Lua script starts");
    Check(!Logged("script: start"), "  it does not run inside the call that starts it");
    ClearLog();
    for (int i = 0; i < 9; i++) {
        D2LuaRt_Tick(pRt, 1);
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
    TestRun(pRt, 1, "beta", "local u = require('redivivus:test').world.spawn('target', 0, 10) u:damage(4)");
    CheckLogged("[beta] script: waited for an event 4 target", "  waitEvent resumes with the payload");
    ClearLog();
    for (int i = 0; i < 4; i++) {
        D2LuaRt_Tick(pRt, 1);
    }
    CheckLogged("[beta] script: yielded a wait value, outcome 12", "  yielding { ticks = 3 } resumes 3 ticks later with the tick");
    Check(Logged("script: across table.sort false") && Logged("attempt to yield across C-call boundary"), "  table.sort's comparator cannot yield");
    Check(Logged("script: across gsub false") && Logged("attempt to yield across C-call boundary"), "  gsub's callback cannot yield");
    Check(Logged("script: across a registry call false") && Logged("attempt to yield across C-call boundary"), "  a registry entry cannot yield (a native C boundary)");
    Check(Logged("script: across require false") && Logged("attempt to yield across C-call boundary"), "  a module's main chunk cannot yield");
    CheckLogged("[beta] script: done 12", "  the script runs to its end");

    TestExpect(pRt, 1, "alpha", "local ok, e = pcall(require('redivivus:shared').wait) require('redivivus:shared').log('outside:', e)",
               "wait can only be used inside a script", "waiting outside a script is a clean error");
    TestExpect(pRt, 1, "alpha", "local ok, e = pcall(coroutine.wrap(function() require('redivivus:shared').wait() end)) require('redivivus:shared').log('own coroutine:', e)",
               "can only be used inside a script", "  also from a mod's own coroutine");

    // Per-resume budget.
    {
        TestRun(pRt, 1, "gamma", "function spin(n) local s = 0 for i = 1, n do s = s + i % 3 end return s end");
        Clock::time_point tStart = Clock::now();
        TestRun(pRt, 1, "gamma", "spin(4000000)");
        int nMs = ElapsedMs(tStart);
        double nPerMs = 4000000.0 / (nMs > 0 ? nMs : 1);
        long nWork = (long)(nPerMs * kTestBudgetMs * 0.4);
        char szScript[600];
        snprintf(szScript, sizeof(szScript),
                 "local w = require('redivivus:shared').wait games[1].startScript(function() for i = 1, 5 do spin(%ld) w() end require('redivivus:shared').log('long script finished') end)", nWork);
        TestRun(pRt, 1, "gamma", szScript);
        ClearLog();
        tStart = Clock::now();
        for (int i = 0; i < 7; i++) {
            D2LuaRt_Tick(pRt, 1);
        }
        int nTotal = ElapsedMs(tStart);
        printf("  five resumes took %d ms in all (budget %u ms per entry)\n", nTotal, kTestBudgetMs);
        Check(Logged("long script finished") && !D2LuaRt_IsFaulted(pRt, "gamma") && nTotal > (int)kTestBudgetMs, "the budget is per resume, not per script");
    }
    TestRun(pRt, 1, "gamma", "games[1].startScript(function() require('redivivus:shared').wait() while true do end end)");
    ClearLog();
    D2LuaRt_Tick(pRt, 1);
    D2LuaRt_Tick(pRt, 1);
    Check(D2LuaRt_IsFaulted(pRt, "gamma") && Logged("mod 'gamma' ran past"), "a script that spins faults the mod that started it");
    D2LuaRt_Destroy(pRt);

    // A wrapper that runs out of time faults its mod, and the chain recovers without it.
    pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, TestRoots(), { "alpha", "beta", "gamma" }, "");
    TestStart(pRt, 1);
    TestRun(pRt, 1, "alpha", "games[1].registry.example.wrap('redivivus:greet', function(next, n) return 'A(' .. next(n) .. ')' end)");
    TestRun(pRt, 1, "beta", "games[1].registry.example.wrap('redivivus:greet', function(next, n) while true do end end)");
    ClearLog();
    TestRun(pRt, 1, "gamma", "games[1].registry.example.call('redivivus:greet', 'q')");
    Check(D2LuaRt_IsFaulted(pRt, "beta") && !D2LuaRt_IsFaulted(pRt, "gamma") && !D2LuaRt_IsFaulted(pRt, "alpha"), "a wrapper that spins faults its own mod only");
    CheckLogged("(in a call that entered through mod 'gamma', which stays enabled)", "  the mod whose call reached it stays enabled");
    TestExpect(pRt, 1, "gamma", "require('redivivus:shared').log('greet without beta:', games[1].registry.example.call('redivivus:greet', 'q'))",
               "greet without beta: A(hello, q)", "  the chain carries on without the faulted mod's wrapper");
    D2LuaRt_Destroy(pRt);
}

// --- enums ---------------------------------------------------------------------------------------------

static void EnumTests() {
    printf("--- enums\n");
    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, TestRoots(), { "alpha", "beta" }, "", { "alpha", "beta" });
    if (!pRt) {
        Check(0, "enums: runtime");
        return;
    }
    TestExpect(pRt, 0, "alpha",
               "local E = require('redivivus:enums') require('redivivus:shared').log('enum values:', E.Event.Tick, E.ExampleId.Greet, E.Event.TestUnitDamaged, require('redivivus:test').TestUnitKind.Npc)",
               "enum values: tick redivivus:greet testUnitDamaged 1", "enums are read-only tables of redivivus:enums; the test set's members merge into Event");
    TestExpect(pRt, 0, "alpha", "local E = require('redivivus:enums') local ok, e = pcall(function() E.Event.Tick = 'x' end) require('redivivus:shared').log('enum write:', ok, e)",
               "Event.Tick is read-only (an enum)", "writing a member raises, naming the enum");
    TestExpect(pRt, 0, "alpha", "local E = require('redivivus:enums') local ok = pcall(function() E.Act.VI = 6 end) require('redivivus:shared').log('enum add:', ok, E.Act.VI)",
               "enum add: false nil", "adding a member raises");
    TestExpect(pRt, 0, "alpha", "local E = require('redivivus:enums') require('redivivus:shared').log('enum metatable:', getmetatable(E.Event), pcall(setmetatable, E.Event, {}))",
               "enum metatable: false false", "the metatable is locked");
    TestExpect(pRt, 0, "alpha",
               "local T = require('redivivus:test') local n, sum = 0, 0 for k, v in pairs(T.TestUnitKind) do n = n + 1 sum = sum + v end require('redivivus:shared').log('enum pairs:', n, sum)",
               "enum pairs: 3 3", "pairs walks an enum's members");
    TestExpect(pRt, 0, "alpha", "local E = require('redivivus:enums') rawset(E.Act, 'I', 42) require('redivivus:shared').log('own rawset:', E.Act.I)",
               "own rawset: 42", "a rawset into its own table changes that mod's view only");
    TestExpect(pRt, 0, "beta", "require('redivivus:shared').log('other mod:', require('redivivus:enums').Act.I)", "other mod: 0", "  every other mod still reads the value");
    TestExpect(pRt, 0, "alpha", "require('redivivus:shared').log('same table:', require('redivivus:server').Event == require('redivivus:enums').Event)",
               "same table: true", "redivivus:server re-exports the canonical Event of redivivus:enums");
    TestExpect(pRt, 0, "alpha",
               "local E = require('redivivus:enums') require('redivivus:shared').log('engine enums:', E.UnitType.Monster, E.PlayerMode.Run, E.MonsterMode.Run, E.UiFlag.Inventory, E.AiTargeting.Basic, "
               "E.QuestId.DenOfEvil, E.ItemQuality.Unique, E.DamageType.Cold, E.StatId.Strength, E.StatId.ItemArmorPercent, E.SkillId.Teleport, "
               "E.DrawMode.Solid, E.Difficulty.Hell, E.PlayerClassId.Assassin, E.BodyLoc.LArm, E.ItemFlag.Ethereal, E.ItemFlag.Tagged, "
               "E.MonsterTypeFlag.Champion, E.Act.V, E.Act.None)",
               "engine enums: 1 3 15 1 1 1 7 4 0 16 54 5 2 6 5 4194304 -2147483648 4 4 255", "engine enums carry the header's values under the generated names");
    TestExpect(pRt, 0, "alpha", "local E = require('redivivus:enums') require('redivivus:shared').log('sentinels:', E.UnitType.Count, E.PlayerClassId.Count, E.UiVar, E.PlayerClass)",
               "sentinels: nil nil nil nil", "  excluded members and the old names are absent");
    D2LuaRt_Destroy(pRt);
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
    printf("--- TypeScript and plain-Lua example mods (%s)\n", pszTsMods);
    TestWorld_Clear();
    std::vector<std::string> aRoots = { pszTsMods, g_sTestRoot + "/Redivivus/Lua/test/mods-ts" };
    ClearLog();
    D2LuaRuntime* pRt = TestMakeRuntime(D2LUA_SIDE_SERVER, aRoots, { "first-mod", "second-mod", "lua-user", "lua-mod", "ui-mod" }, "");
    D2LuaRuntime* pClient = TestMakeRuntime(D2LUA_SIDE_CLIENT, aRoots, { "first-mod", "second-mod", "lua-user", "lua-mod", "ui-mod" }, "");
    Check(pRt && pClient, "the example mods load in a server and a client state");
    if (!pRt || !pClient) {
        return;
    }
    Check(Order(D2LuaRt_Mods(pRt)) == "redivivus first-mod lua-mod lua-user second-mod ui-mod", "  in dependency order");
    CheckLogged("[first-mod] first-mod main loaded, lualib from redivivus 3", "first-mod's main runs on the TypeScriptToLua runtime redivivus serves");
    CheckLogged("[second-mod] second-mod imports from first-mod: hello, second-mod (3 letters doubled: 6)", "second-mod uses a typed export of first-mod");
    CheckLogged("[lua-mod] lua-mod loaded: redivivus:shared, redivivus:enums and its own module", "the plain-Lua example loads with require(\"redivivus:...\")");
    D2LuaLoopback* pLoop = D2LuaLoopback_Create(pRt);
    D2LuaLoopback_AddClient(pLoop, pClient, 5);
    ClearLog();
    TestStart(pRt, 1);
    TestStart(pClient, 1);
    CheckLogged("[first-mod] first-mod server(game 1), new game", "server(game) from TypeScript");
    CheckLogged("[first-mod] engine enums: 1 54 7", "engine enums from TypeScript, inlined");
    CheckLogged("[first-mod] kit settings: 3 slow", "typed settings from TypeScript: their defaults");
    CheckLogged("[lua-mod] lua-mod server(game 1)", "the plain-Lua example's server export");
    CheckLogged("[lua-mod] lua-mod client(game 1)", "  and its client export, in the client state");
    CheckLogged("[ui-mod] ui-mod server panel in game 1: clicked 2 times, drawn",
                "main imports a TSX panel on redivivus:ui and the server export mounts it: it works unchanged in the server state");
    CheckLogged("[ui-mod] ui-mod client panel in game 1: clicked 2 times, drawn",
                "  and the client export: TSX widgets on redivivus:ui, laid out, drawn, clicked, reactive");
    Check(!D2LuaRt_IsFaulted(pRt, "ui-mod") && !D2LuaRt_IsFaulted(pClient, "ui-mod"), "  no fault in either state");
    D2LuaLoopback_Pump(pLoop);
    D2LuaLoopback_Pump(pLoop);
    CheckLogged("[first-mod] peer 5 waved: hi (fast)", "a TypeScript channel: client call, server implement, server emit, client handler");
    TestExpect(pRt, 1, "second-mod", "require('redivivus:shared').log('ts registry:', ts_game.registry.example.call('redivivus:greet', 'ts'))",
               "ts registry: second-mod{first-mod(hello, ts)}", "TypeScript wrappers in load order");
    TestExpect(pRt, 1, "second-mod", "require('redivivus:shared').log('ts registry own id:', ts_game.registry.example.call('first-mod:shout', 'ts'))",
               "ts registry own id: <TS!>", "an id a mod added by augmentation, wrapped by another");
    ClearLog();
    for (int i = 0; i < 12; i++) {
        D2LuaRt_Tick(pRt, 1);
    }
    CheckLogged("[first-mod] ts script: started at 1", "a TS generator script: first resume");
    CheckLogged("[first-mod] ts script: bare yield resumed at 2", "  bare yield waits one tick");
    CheckLogged("[first-mod] ts script: wait value outcome 4", "  yield of a wait value gives its outcome");
    CheckLogged("[first-mod] ts script: nested actions gave 3 ticks, ended at 7", "  yield* into typed actions, nested three deep");
    CheckLogged("[first-mod] ts script: done", "  the TS script ends");
    CheckLogged("[first-mod] waitUntil: true 3", "waitUntil's outcome from TypeScript");
    CheckLogged("[lua-user] lua script: ts action gave 6", "a Lua script calls an action written in TypeScript");
    CheckLogged("[lua-mod] lua-mod script waited, tick 3", "the plain-Lua example's script");
    int nLine = FindMarkerLine(g_sTestRoot + "/Redivivus/Lua/examples/first-mod/main.ts", "THROW-ON-TICK-12");
    char szWant[160];
    snprintf(szWant, sizeof(szWant), "mods/first-mod/main.ts:%d: first-mod fails on tick 12", nLine);
    CheckLogged(szWant, "a runtime error in a TS mod reports the .ts file and line, in the mod's namespace");
    D2LuaLoopback_Destroy(pLoop);
    D2LuaRt_Destroy(pClient);
    D2LuaRt_Destroy(pRt);
}

void RunModTests(const char* pszRoot, const char* pszTsMods) {
    g_sTestRoot = pszRoot;
    PatternTests();
    LoaderTests();
    DataStageTests();
    ControlTests();
    TwoGameTests();
    ScriptTests();
    EnumTests();
    RunKitTests(pszRoot);
    RunBootTests(pszRoot);
    RunMenuTests(pszRoot);
    RunEscapeTests(pszRoot);
    if (pszTsMods) {
        TypeScriptTests(pszTsMods);
    } else {
        printf("SKIP TypeScript mods: no --ts-mods directory\n");
    }
    RunWorldTests(pszTsMods);
}
#endif
