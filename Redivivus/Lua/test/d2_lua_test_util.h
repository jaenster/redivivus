// What the host test's files share: the check counter and the captured log.
#pragma once

#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include <stddef.h>

// The host log of everything since the last ClearLog, one line per entry.
extern char g_szTestLog[262144];
void TestLog(const char* pszLine);
void ClearLog();
void Check(int bOk, const char* pszWhat);
// Whether the captured log contains pszText.
bool Logged(const char* pszText);

// The checks of d2_lua_mods_test.cpp: loader, runtime, registries, scripts, storage, handles,
// patterns, and (when pszTsMods is set) the TypeScript mods built by the CLI into that directory.
// pszRoot is the repository root.
void RunModTests(const char* pszRoot, const char* pszTsMods);

#include <string>
#include <vector>
struct D2LuaRuntime;
// A runtime of the side over the mods named in aOnly, found under aRoots, with the test's APIs.
D2LuaRuntime* TestMakeRuntime(int nSide, const std::vector<std::string>& aRoots, const std::vector<std::string>& aOnly,
                              const std::string& sSettingsFile);
// The kit's checks (d2_lua_kit_test.cpp).
void RunKitTests(const char* pszRoot);
// The boot path the game takes (d2_lua_boot_test.cpp).
void RunBootTests(const char* pszRoot);
#endif
