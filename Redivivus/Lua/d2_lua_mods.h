// Mod discovery, manifests and load order. Plain C++, no
// Lua state involved: the same list is what the game will later send on join.
//
// A mod is a directory holding info.json:
//   { "name": "my-mod", "version": "1.2.0", "title": "...", "author": "...",
//     "redivivus_version": "0.1", "dependencies": ["base >= 0.1", "? other", "! enemy"] }
// The directory is named after the mod ("my-mod") or name and version ("my-mod_1.2.0").
//
// Dependencies use Factorio's syntax:
//   "name", "name >= 1.2"   required (operators < <= = >= >)
//   "? name"                optional: ordered after it when present
//   "(?) name"              hidden optional: the same, not shown to players
//   "! name"                incompatible: both cannot be loaded
//   "~ name"                required, but does not affect load order
// Every mod but base depends on base implicitly. Naming base in the list in any form replaces
// the implicit dependency, so "? base" or "~ base" opts out of the hard requirement.
//
// Load order is a topological sort of the ordering edges; among mods whose dependencies are
// all loaded, the smallest name (byte order) goes first. The same mod set gives the same order
// on every machine.
#pragma once

#if D2_FEATURE_MODS
#include <string>
#include <vector>

// The API level the host offers. A mod's redivivus_version must not be newer (major.minor).
#define D2LUA_REDIVIVUS_VERSION "0.1"

struct D2LuaVersion {
    int a[3] = { 0, 0, 0 };
};

enum class D2LuaDepKind { Required, Optional, HiddenOptional, Incompatible, Unordered };
enum class D2LuaVersionOp { Any, Less, LessEqual, Equal, GreaterEqual, Greater };

struct D2LuaDependency {
    D2LuaDepKind eKind = D2LuaDepKind::Required;
    std::string sName;
    D2LuaVersionOp eOp = D2LuaVersionOp::Any;
    D2LuaVersion tVersion;
    std::string sText;  // as written in info.json
};

struct D2LuaModInfo {
    std::string sName;
    std::string sVersion;
    D2LuaVersion tVersion;
    std::string sTitle;
    std::string sAuthor;
    std::string sRedivivusVersion;
    std::vector<D2LuaDependency> aDependencies;  // as declared, plus the implicit base
    std::string sPath;                           // the mod's directory, no trailing slash
};

// Parse "1", "1.2" or "1.2.3" (missing parts are 0). False on anything else.
bool D2LuaMods_ParseVersion(const std::string& sText, D2LuaVersion& tOut);
int D2LuaMods_CompareVersions(const D2LuaVersion& a, const D2LuaVersion& b);
bool D2LuaMods_ParseDependency(const std::string& sText, D2LuaDependency& tOut, std::string& sError);
// [A-Za-z0-9_-]+, at most 64 bytes.
bool D2LuaMods_IsValidName(const std::string& sName);

// Read and check sDir/info.json. On failure sError says why (without the path).
bool D2LuaMods_ReadInfo(const std::string& sDir, D2LuaModInfo& tOut, std::string& sError);
// The same from info.json's text, for a mod in sDir.
bool D2LuaMods_ParseInfo(const std::string& sText, const std::string& sDir, D2LuaModInfo& tOut, std::string& sError);

// Every mod directory directly under each root. Errors (unreadable info.json, a mod found twice,
// a directory named after another mod) go to aErrors; the mods that are fine are still returned.
void D2LuaMods_Discover(const std::vector<std::string>& aRoots, std::vector<D2LuaModInfo>& aOut,
                        std::vector<std::string>& aErrors);

// Check the dependencies of aMods and put them in load order. Returns false with every problem
// in aErrors (missing dependency, version mismatch, incompatibility, a cycle with its members
// named in order); aOrdered is then empty.
bool D2LuaMods_Resolve(const std::vector<D2LuaModInfo>& aMods, std::vector<D2LuaModInfo>& aOrdered,
                       std::vector<std::string>& aErrors);

// Resolve as far as the mods allow: a mod that cannot load (a missing, disabled or wrong-version
// dependency, an incompatible mod enabled, a dependency cycle) is disabled with the reason in
// aDisabled ("mod 'x' is disabled: why"), and the rest are put in load order. Repeats until the
// remaining set resolves, so the dependents of a disabled mod are disabled too.
void D2LuaMods_ResolveEnabled(const std::vector<D2LuaModInfo>& aMods, std::vector<D2LuaModInfo>& aOrdered,
                              std::vector<std::string>& aDisabled);

// Read a whole file. False when it cannot be opened or read.
bool D2LuaMods_ReadFile(const std::string& sPath, std::string& sOut);
#endif
