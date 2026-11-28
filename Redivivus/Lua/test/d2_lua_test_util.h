// What the host test's files share: the check counter, the captured log, runtimes over fixture mods.
#pragma once

#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

// The host log of everything since the last ClearLog, one line per entry.
extern char g_szTestLog[262144];
void TestLog(const char* pszLine);
void ClearLog();
void Check(int bOk, const char* pszWhat);
// Whether the captured log contains pszText.
bool Logged(const char* pszText);
void CheckLogged(const char* pszText, const char* pszWhat);

struct D2LuaRuntime;

// The budget every test runtime gets.
const unsigned kTestBudgetMs = 250;

// A runtime of the side over the mods named in aOnly, found under aRoots (redivivus is added),
// with the test's APIs. aTestGrant: mods that may import redivivus:test.
D2LuaRuntime* TestMakeRuntime(int nSide, const std::vector<std::string>& aRoots, const std::vector<std::string>& aOnly,
                              const std::string& sSettingsFile, const std::vector<std::string>& aTestGrant = {},
                              const std::vector<std::string>& aDriverGrant = {}, const std::vector<std::string>& aRawGrant = {});
// Run a chunk as mod pszMod with game nGame current; true when it ran without an error.
bool TestRun(D2LuaRuntime* pRt, uint32_t nGame, const char* pszMod, const char* pszSource);
// Run it with an empty log, then check it ran and logged pszLogged.
void TestExpect(D2LuaRuntime* pRt, uint32_t nGame, const char* pszMod, const char* pszSource, const char* pszLogged,
                const char* pszWhat);
// Start game nGame with empty storage.
void TestStart(D2LuaRuntime* pRt, uint32_t nGame);
// The fixture mod roots.
std::vector<std::string> TestRoots();
extern std::string g_sTestRoot;

void RunModTests(const char* pszRoot, const char* pszTsMods);
void RunKitTests(const char* pszRoot);
void RunBootTests(const char* pszRoot);
void RunEscapeTests(const char* pszRoot);
void RunWorldTests(const char* pszTsMods);
void RunMenuTests(const char* pszRoot);
int RunDispatchBench(const char* pszRoot, const char* pszTmp);
// Function overrides on the fixture (d2_lua_override_test.cpp); bBench adds the per-call timings.
void RunOverrideTests(const char* pszRoot, bool bBench);
struct D2UnitStrc;
// Units through an override, in a server state with a world running mod ovunit (the stand-in engine).
void RunOverrideUnitTests(D2LuaRuntime* pRt, D2UnitStrc* pA, D2UnitStrc* pB);
#endif
