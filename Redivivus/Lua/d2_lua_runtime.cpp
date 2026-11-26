// The mod runtime. See d2_lua_runtime.h for the model and d2_lua_rt.h for where its state lives.
//
// What is C++ and what is Lua: the resolver and its rules, the namespaces, the state's set-up,
// storage bytes and source maps are here; registries, events, scripts, channels and the objects a
// mod's exports receive are the core's (Redivivus/Lua/core), called through d2_lua_core.h.
//
// Functions Lua calls (Lua_*) follow the host's rule: no object with a destructor in their
// frames. The C++ helpers they call complete before any Lua call and catch their own exceptions.
#if D2_FEATURE_MODS
#include "Redivivus/Lua/d2_lua_runtime.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "Redivivus/Lua/d2_lua_core.h"
#include "Redivivus/Lua/d2_lua_internal.h"
#include "Redivivus/Lua/d2_lua_json.h"
#include "Redivivus/Lua/d2_lua_rt.h"
#include "lua.hpp"

static char s_RtKey;       // the runtime's registry table
static char s_LoadingKey;  // the sentinel a module's cache slot holds while it loads

// --- the runtime table ---------------------------------------------------------------------------

void D2LuaRt_PushTable(lua_State* L) {
    lua_pushlightuserdata(L, &s_RtKey);
    lua_rawget(L, LUA_REGISTRYINDEX);
}

void D2LuaRt_PushField(lua_State* L, const char* pszField) {
    D2LuaRt_PushTable(L);
    lua_getfield(L, -1, pszField);
    lua_remove(L, -2);
}

D2LuaRuntime* D2LuaRt_FromState(lua_State* L) {
    return (D2LuaRuntime*)D2LuaHost_HookUser(L);
}

D2LuaMod* D2LuaRt_ModByHostId(D2LuaRuntime* pRt, int nHostId) {
    if (!pRt) {
        return nullptr;
    }
    for (auto& pMod : pRt->aMods) {
        if (pMod->nHostId == nHostId) {
            return pMod.get();
        }
    }
    return nullptr;
}

D2LuaMod* D2LuaRt_ModByName(D2LuaRuntime* pRt, const char* pszName) {
    if (!pRt) {
        return nullptr;
    }
    for (auto& pMod : pRt->aMods) {
        if (strcmp(pMod->sName.c_str(), pszName) == 0) {
            return pMod.get();
        }
    }
    return nullptr;
}

void D2LuaRt_PushEnv(lua_State* L, D2LuaMod* pMod) {
    D2LuaRt_PushField(L, "envs");
    lua_getfield(L, -1, pMod->sName.c_str());
    lua_remove(L, -2);
}

bool D2LuaRt_ModFaulted(lua_State* L, D2LuaMod* pMod) {
    return !pMod->bIntrinsic && D2LuaHost_IsModFaulted(L, pMod->sName.c_str()) != 0;
}

static int Lua_InitTable(lua_State* L) {
    lua_pushlightuserdata(L, &s_RtKey);
    lua_newtable(L);
    static const char* const s_apszTables[] = { "envs", "loaded", "events", "registries", "classes", "enums", "wrapenvs" };
    for (size_t i = 0; i < sizeof(s_apszTables) / sizeof(s_apszTables[0]); i++) {
        lua_newtable(L);
        lua_setfield(L, -2, s_apszTables[i]);
    }
    lua_rawset(L, LUA_REGISTRYINDEX);
    return 0;
}

// --- source maps --------------------------------------------------------------------------------

static int Base64Value(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

// Decode the "mappings" of a version 3 source map: for every generated line, the source line of
// its first segment that has one.
static bool DecodeMappings(const std::string& sMappings, std::vector<int>& aLines) {
    aLines.assign(2, 0);  // index 0 unused: generated lines count from 1
    int nSrcLine = 0;
    int aField[5];
    size_t i = 0;
    bool bLineHasMapping = false;
    while (i <= sMappings.size()) {
        if (i == sMappings.size() || sMappings[i] == ';') {
            aLines.push_back(0);
            bLineHasMapping = false;
            i++;
            continue;
        }
        if (sMappings[i] == ',') {
            i++;
            continue;
        }
        int nFields = 0;
        while (i < sMappings.size() && sMappings[i] != ',' && sMappings[i] != ';') {
            int nValue = 0;
            int nShift = 0;
            int nDigit;
            do {
                if (i >= sMappings.size()) {
                    return false;
                }
                nDigit = Base64Value(sMappings[i++]);
                if (nDigit < 0) {
                    return false;
                }
                nValue += (nDigit & 31) << nShift;
                nShift += 5;
            } while (nDigit & 32);
            int nDecoded = (nValue & 1) ? -(nValue >> 1) : (nValue >> 1);
            if (nFields < 5) {
                aField[nFields] = nDecoded;
            }
            nFields++;
        }
        if (nFields >= 4) {
            nSrcLine += aField[2];
            if (!bLineHasMapping) {
                aLines.back() = nSrcLine + 1;
                bLineHasMapping = true;
            }
        }
    }
    aLines.pop_back();
    return true;
}

void D2LuaRt_LoadSourceMap(D2LuaRuntime* pRt, const char* pszChunk, const char* pszMapPath) {
    try {
        std::string sText;
        if (pRt->tMaps.count(pszChunk) || !D2LuaMods_ReadFile(pszMapPath, sText)) {
            return;
        }
        D2Json tMap;
        std::string sError;
        if (!D2Json_Parse(sText.data(), sText.size(), tMap, sError)) {
            return;
        }
        const D2Json* pSources = tMap.Get("sources");
        const D2Json* pMappings = tMap.Get("mappings");
        if (!pSources || pSources->eType != D2Json::Array || pSources->aItems.empty()
            || pSources->aItems[0].eType != D2Json::String || !pMappings || pMappings->eType != D2Json::String) {
            return;
        }
        D2LuaSourceMap tEntry;
        if (!DecodeMappings(pMappings->sValue, tEntry.aLines)) {
            return;
        }
        // The source path is relative to the map; the map sits beside the chunk, so it is
        // resolved against the chunk's directory ("mods/my-mod/main.lua" with
        // "main.ts" -> "mods/my-mod/main.ts").
        std::string sChunk = pszChunk;
        std::string sDir = sChunk.substr(0, sChunk.rfind('/') + 1);
        std::string sSource = pSources->aItems[0].sValue;
        while (sSource.compare(0, 2, "./") == 0) {
            sSource = sSource.substr(2);
        }
        while (sSource.compare(0, 3, "../") == 0 && sDir.size() > 1) {
            sSource = sSource.substr(3);
            size_t nCut = sDir.rfind('/', sDir.size() - 2);
            sDir = nCut == std::string::npos ? std::string() : sDir.substr(0, nCut + 1);
        }
        tEntry.sSource = sDir + sSource;
        pRt->tMaps[sChunk] = tEntry;
    } catch (...) {
    }
}

// Rewrite "chunk.lua:LINE" for every chunk with a map to "source.ts:LINE".
static void ErrorHook(const char* pszIn, char* pszOut, size_t nOut, void* pUser) {
    D2LuaRuntime* pRt = (D2LuaRuntime*)pUser;
    std::string sOut;
    try {
        std::string sIn = pszIn;
        size_t nPos = 0;
        while (true) {
            size_t nHit = sIn.find(".lua:", nPos);
            if (nHit == std::string::npos) {
                break;
            }
            size_t nStart = nHit;
            while (nStart > 0) {
                char c = sIn[nStart - 1];
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-'
                    || c == '/' || c == '.') {
                    nStart--;
                } else {
                    break;
                }
            }
            size_t nDigits = nHit + 5;
            size_t nEnd = nDigits;
            while (nEnd < sIn.size() && sIn[nEnd] >= '0' && sIn[nEnd] <= '9') {
                nEnd++;
            }
            std::string sChunk = sIn.substr(nStart, nHit + 4 - nStart);
            auto it = pRt->tMaps.find(sChunk);
            if (nEnd > nDigits && it != pRt->tMaps.end()) {
                int nLine = atoi(sIn.substr(nDigits, nEnd - nDigits).c_str());
                if (nLine > 0 && (size_t)nLine < it->second.aLines.size() && it->second.aLines[(size_t)nLine] > 0) {
                    sOut += sIn.substr(nPos, nStart - nPos);
                    sOut += it->second.sSource + ":" + std::to_string(it->second.aLines[(size_t)nLine]);
                    nPos = nEnd;
                    continue;
                }
            }
            sOut += sIn.substr(nPos, nEnd - nPos);
            nPos = nEnd;
        }
        sOut += sIn.substr(nPos);
    } catch (...) {
        sOut = pszIn;
    }
    snprintf(pszOut, nOut, "%s", sOut.c_str());
}

// --- namespaces ---------------------------------------------------------------------------------
// The built-in mod's own program (Redivivus/Lua/builtin, compiled like a mod and embedded in the
// exe) is the namespace of redivivus: its files are found here, never on disk.
static const char kBuiltinPrefix[] = "\x01builtin:";

// A mod's files are the tree mods/<name>/ of the one file tree every archive and folder makes; in
// folder mode that is the mod's folder. Names are case-insensitive, as in an MPQ: a path is looked
// up as written first, then segment by segment without case. A mod is only ever served files
// below its own folder: paths are checked segments, never "..".

static std::string Lower(const std::string& s) {
    std::string out = s;
    for (char& c : out) {
        if (c >= 'A' && c <= 'Z') {
            c = (char)(c - 'A' + 'a');
        }
    }
    return out;
}

static bool IsFile(const std::string& sPath) {
    struct stat tStat;
    return stat(sPath.c_str(), &tStat) == 0 && S_ISREG(tStat.st_mode);
}

// Find sRel ("shared/ids.lua", checked segments) in the mod's namespace. sCanonical gets the path as
// it is on disk. Plain C++, catches its own exceptions.
static bool FindModFile(const D2LuaMod* pMod, const std::string& sRel, std::string& sReal, std::string& sCanonical) {
    try {
        if (pMod->bIntrinsic) {
            const D2LuaCoreChunk* pFile = D2LuaCore_FindBuiltinFile(sRel.c_str());
            if (!pFile) {
                return false;
            }
            sReal = std::string(kBuiltinPrefix) + pFile->pszName;
            sCanonical = pFile->pszName;
            return true;
        }
        if (pMod->sPath.empty()) {
            return false;
        }
        if (IsFile(pMod->sPath + "/" + sRel)) {
            sReal = pMod->sPath + "/" + sRel;
            sCanonical = sRel;
            return true;
        }
        std::string sDir = pMod->sPath;
        std::string sOut;
        size_t nPos = 0;
        while (nPos <= sRel.size()) {
            size_t nEnd = sRel.find('/', nPos);
            if (nEnd == std::string::npos) {
                nEnd = sRel.size();
            }
            std::string sWant = Lower(sRel.substr(nPos, nEnd - nPos));
            DIR* pDir = opendir(sDir.c_str());
            if (!pDir) {
                return false;
            }
            std::string sFound;
            while (struct dirent* pEntry = readdir(pDir)) {
                if (Lower(pEntry->d_name) == sWant) {
                    sFound = pEntry->d_name;
                    break;
                }
            }
            closedir(pDir);
            if (sFound.empty()) {
                return false;
            }
            sDir += "/" + sFound;
            sOut += (sOut.empty() ? "" : "/") + sFound;
            nPos = nEnd + 1;
        }
        if (!IsFile(sDir)) {
            return false;
        }
        sReal = sDir;
        sCanonical = sOut;
        return true;
    } catch (...) {
        return false;
    }
}

// --- require ------------------------------------------------------------------------------------

// Split a module name into the mod it names and the path in that mod ("shared/ids"): "<mod>:<path>",
// or a path of the requiring mod's own. '.' and '/' both separate segments; a leading "./" and a
// trailing ".lua" are dropped. Returns null on success, or why not.
static const char* ParseModuleName(const char* pszName, const char* pszSelf, std::string& sTarget, std::string& sPath) {
    const char* pColon = strchr(pszName, ':');
    const char* p = pszName;
    if (pColon) {
        sTarget.assign(pszName, (size_t)(pColon - pszName));
        if (!D2LuaMods_IsValidName(sTarget)) {
            return "a module of another mod is written <mod>:<path>";
        }
        p = pColon + 1;
    } else {
        sTarget = pszSelf;
    }
    while (p[0] == '.' && p[1] == '/') {
        p += 2;
    }
    size_t nLen = strlen(p);
    if (nLen >= 4 && strcmp(p + nLen - 4, ".lua") == 0) {
        nLen -= 4;
    }
    if (nLen == 0 || nLen > 400) {
        return "the module path is empty or too long";
    }
    sPath.clear();
    bool bSegmentStart = true;
    for (size_t i = 0; i < nLen; i++) {
        char c = p[i];
        if (c == '.' || c == '/') {
            if (bSegmentStart) {
                return "empty path segment (no '..', no leading or doubled separators)";
            }
            sPath += '/';
            bSegmentStart = true;
            continue;
        }
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-')) {
            return "module paths use letters, digits, '_', '-' and '.' or '/' between parts";
        }
        sPath += c;
        bSegmentStart = false;
    }
    if (bSegmentStart) {
        return "the module path ends in a separator";
    }
    return nullptr;
}

static const char* SideName(int nSide) {
    return nSide == D2LUA_SIDE_DATA ? "data" : nSide == D2LUA_SIDE_SERVER ? "server" : "client";
}

// Push the contents of a file as a Lua string, or nil when it cannot be read. The buffer is a
// userdata, so an error on the way leaks nothing.
static void PushFile(lua_State* L, const char* pszPath) {
    if (strncmp(pszPath, kBuiltinPrefix, strlen(kBuiltinPrefix)) == 0) {
        const D2LuaCoreChunk* pBuiltin = D2LuaCore_FindBuiltinFile(pszPath + strlen(kBuiltinPrefix));
        if (pBuiltin) {
            lua_pushlstring(L, pBuiltin->pszSource, pBuiltin->nSize);
        } else {
            lua_pushnil(L);
        }
        return;
    }
    FILE* pFile = fopen(pszPath, "rb");
    if (!pFile) {
        lua_pushnil(L);
        return;
    }
    long nSize = -1;
    if (fseek(pFile, 0, SEEK_END) == 0) {
        nSize = ftell(pFile);
    }
    if (nSize < 0 || fseek(pFile, 0, SEEK_SET) != 0) {
        fclose(pFile);
        lua_pushnil(L);
        return;
    }
    // A Lua error here (out of memory) would leave the file open; take the buffer first.
    char* pBuffer = (char*)lua_newuserdata(L, (size_t)nSize + 1);
    size_t nRead = fread(pBuffer, 1, (size_t)nSize, pFile);
    fclose(pFile);
    if (nRead != (size_t)nSize) {
        lua_pop(L, 1);
        lua_pushnil(L);
        return;
    }
    lua_pushlstring(L, pBuffer, (size_t)nSize);
    lua_remove(L, -2);
}

static bool Contains(const std::vector<std::string>& a, const std::string& s) {
    for (const std::string& x : a) {
        if (x == s) {
            return true;
        }
    }
    return false;
}

// The list of redivivus's modules, for messages.
static void PushModuleList(lua_State* L) {
    luaL_Buffer tBuffer;
    luaL_buffinit(L, &tBuffer);
    for (const D2LuaCoreModule* p = g_aD2LuaCoreModules; p->pszEntry; p++) {
        if (p != g_aD2LuaCoreModules) {
            luaL_addstring(&tBuffer, ", ");
        }
        luaL_addstring(&tBuffer, p->pszEntry);
    }
    luaL_pushresult(&tBuffer);
}

// The environment a module of mod pBase runs in when mod pMod requires it: pMod's globals, with a
// require whose bare names are pBase's own modules (the code is pBase's; its instance is pMod's).
static int Lua_Require(lua_State* L);
static void PushModuleEnv(lua_State* L, D2LuaMod* pMod, D2LuaMod* pBase) {
    if (pBase == pMod) {
        D2LuaRt_PushEnv(L, pMod);
        return;
    }
    D2LuaRt_PushField(L, "wrapenvs");
    lua_pushfstring(L, "%s\n%s", pMod->sName.c_str(), pBase->sName.c_str());
    lua_rawget(L, -2);
    if (lua_istable(L, -1)) {
        lua_remove(L, -2);
        return;
    }
    lua_pop(L, 1);
    lua_createtable(L, 0, 1);
    lua_pushlightuserdata(L, pMod);
    lua_pushlightuserdata(L, pBase);
    lua_pushcclosure(L, Lua_Require, 2);
    lua_setfield(L, -2, "require");
    lua_createtable(L, 0, 2);
    D2LuaRt_PushEnv(L, pMod);
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "__index");
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L, -2);
    lua_pushfstring(L, "%s\n%s", pMod->sName.c_str(), pBase->sName.c_str());
    lua_pushvalue(L, -2);
    lua_rawset(L, -4);
    lua_remove(L, -2);
}

void D2LuaRt_Require(lua_State* L, D2LuaMod* pMod, const char* pszName, bool bOptional) {
    D2LuaRt_RequireFrom(L, pMod, pMod, pszName, bOptional);
}

void D2LuaRt_RequireFrom(lua_State* L, D2LuaMod* pMod, D2LuaMod* pBase, const char* pszName, bool bOptional) {
    D2LuaRuntime* pRt = pMod->pRt;
    lua_pushstring(L, pszName);
    int nName = lua_gettop(L);
    pszName = lua_tostring(L, nName);
    char szTarget[80];
    char szPath[420];
    {
        // C++ strings must be gone before any Lua call that may raise.
        std::string sTarget, sPath;
        const char* pszWhy = nullptr;
        try {
            pszWhy = ParseModuleName(pszName, pBase->sName.c_str(), sTarget, sPath);
        } catch (...) {
            pszWhy = "out of memory";
        }
        if (pszWhy) {
            snprintf(szPath, sizeof(szPath), "%s", pszWhy);
            szTarget[0] = '\0';
        } else {
            snprintf(szTarget, sizeof(szTarget), "%s", sTarget.c_str());
            snprintf(szPath, sizeof(szPath), "%s", sPath.c_str());
        }
    }
    if (!szTarget[0]) {
        luaL_error(L, "require '%s': %s", pszName, szPath);
    }
    // The cache key: one instance per module per requiring mod, whichever form named it.
    {
        char szKey[sizeof(szTarget) + sizeof(szPath) + 2];
        snprintf(szKey, sizeof(szKey), "%s:%s", szTarget, szPath);
        for (char* q = szKey; *q; q++) {
            if (*q >= 'A' && *q <= 'Z') {
                *q = (char)(*q - 'A' + 'a');
            }
        }
        lua_pushstring(L, szKey);
    }
    int nKey = lua_gettop(L);
    D2LuaRt_PushField(L, "loaded");
    lua_getfield(L, -1, pMod->sName.c_str());
    lua_remove(L, -2);
    int nCache = lua_gettop(L);
    lua_pushvalue(L, nKey);
    lua_rawget(L, nCache);
    if (lua_touserdata(L, -1) == &s_LoadingKey) {
        luaL_error(L, "require '%s': loop or previous error loading module", pszName);
    }
    if (!lua_isnil(L, -1)) {
        lua_replace(L, nName);
        lua_settop(L, nName);
        return;
    }
    lua_pop(L, 1);

    // The built-in mod's own files: only its own code reaches them, by a bare name (its compiled
    // relative imports); "redivivus:<entry>" always names a public module.
    bool bBuiltinFile = false;
    if (strcmp(szTarget, D2LUA_INTRINSIC_MOD) == 0 && pBase->bIntrinsic && !strchr(pszName, ':')) {
        char szFile[440];
        snprintf(szFile, sizeof(szFile), "%s.lua", szPath);
        for (char* q = szFile; *q; q++) {
            if (*q == '.' && strcmp(q, ".lua") != 0) {
                *q = '/';
            }
        }
        bBuiltinFile = D2LuaCore_FindBuiltinFile(szFile) != nullptr;
    }
    if (strcmp(szTarget, D2LUA_INTRINSIC_MOD) == 0 && !bBuiltinFile) {
        const D2LuaCoreModule* pModule = D2LuaCore_FindModule(szPath);
        if (!pModule) {
            PushModuleList(L);
            luaL_error(L, "require '%s': redivivus has no module '%s' (its modules: %s)", pszName, szPath, lua_tostring(L, -1));
        }
        if ((pModule->nGrant & D2LUA_GRANT_TEST) && !pMod->bTestGrant) {
            luaL_error(L, "require '%s': only a test the test runner starts can import redivivus:%s; a mod cannot", pszName,
                       pModule->pszEntry);
        }
        if ((pModule->nGrant & D2LUA_GRANT_MANAGER) && !pMod->bManagerGrant) {
            luaL_error(L,
                       "require '%s': only the game's own redivivus mod can import redivivus:%s; a mod cannot (it turns "
                       "mods on and off and restarts the game)",
                       pszName, pModule->pszEntry);
        }
        if ((pModule->nGrant & D2LUA_GRANT_DRIVER) && !pMod->bDriverGrant) {
            luaL_error(L,
                       "require '%s': only a driver script started from the command line can import redivivus:%s; a mod "
                       "cannot (nothing a server sends may drive a player's client)",
                       pszName, pModule->pszEntry);
        }
        if (!(pModule->nSides & pRt->tDesc.nSide)) {
            luaL_error(L, "require '%s': redivivus:%s is not available in the %s state", pszName, pModule->pszEntry,
                       SideName(pRt->tDesc.nSide));
        }
        D2LuaCore_PushModule(L, pRt, pMod, pModule);
    } else {
        D2LuaMod* pTarget = D2LuaRt_ModByName(pRt, szTarget);
        if (Contains(pBase->aOptionalAbsent, szTarget)) {
            if (bOptional) {
                lua_settop(L, nName - 1);
                lua_pushnil(L);
                return;
            }
            luaL_error(L, "require '%s': mod '%s' is not installed (an optional dependency: use optionalImport)", pszName, szTarget);
        }
        if (!pTarget || !Contains(pBase->aRequirable, szTarget)) {
            luaL_error(L, "require '%s': mod '%s' does not depend on '%s' (declare it in info.json)", pszName,
                       pBase->sName.c_str(), szTarget);
        }
        char szReal[1200];
        char szCanonical[440];
        bool bFound;
        {
            std::string sReal, sCanonical;
            bFound = FindModFile(pTarget, std::string(szPath) + ".lua", sReal, sCanonical);
            snprintf(szReal, sizeof(szReal), "%s", sReal.c_str());
            snprintf(szCanonical, sizeof(szCanonical), "%s", sCanonical.c_str());
        }
        if (!bFound) {
            luaL_error(L, "require '%s': module not found (no file mods/%s/%s.lua)", pszName, szTarget, szPath);
        }
        PushFile(L, szReal);
        if (lua_isnil(L, -1)) {
            luaL_error(L, "require '%s': cannot read mods/%s/%s", pszName, szTarget, szCanonical);
        }
        int nSource = lua_gettop(L);
        lua_pushfstring(L, "mods/%s/%s", szTarget, szCanonical);
        lua_pushfstring(L, "%s.map", szReal);
        D2LuaRt_LoadSourceMap(pRt, lua_tostring(L, -2), lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pushfstring(L, "@%s", lua_tostring(L, -1));
        size_t nSize = 0;
        const char* pSource = lua_tolstring(L, nSource, &nSize);
        if (luaL_loadbufferx(L, pSource, nSize, lua_tostring(L, -1), "t") != 0) {
            lua_error(L);
        }
        PushModuleEnv(L, pMod, pTarget);
        lua_setfenv(L, -2);
        lua_pushvalue(L, nKey);
        lua_pushlightuserdata(L, &s_LoadingKey);
        lua_rawset(L, nCache);
        lua_pushvalue(L, nName);
        lua_call(L, 1, 1);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            lua_pushboolean(L, 1);
        }
    }
    lua_pushvalue(L, nKey);
    lua_pushvalue(L, -2);
    lua_rawset(L, nCache);
    lua_replace(L, nName);
    lua_settop(L, nName);
}

// require in a mod environment: upvalue 1 is the mod whose instance it loads, upvalue 2 the mod whose
// code calls it (bare names are that mod's modules).
static int Lua_Require(lua_State* L) {
    D2LuaMod* pMod = (D2LuaMod*)lua_touserdata(L, lua_upvalueindex(1));
    D2LuaMod* pBase = (D2LuaMod*)lua_touserdata(L, lua_upvalueindex(2));
    const char* pszName = luaL_checkstring(L, 1);
    lua_settop(L, 1);
    D2LuaRt_RequireFrom(L, pMod, pBase, pszName, false);
    return 1;
}

// --- environments -------------------------------------------------------------------------------

static void EnvHook(lua_State* L, int nEnv, int nModId, void* pUser) {
    D2LuaRuntime* pRt = (D2LuaRuntime*)pUser;
    D2LuaMod* pMod = D2LuaRt_ModByHostId(pRt, nModId);
    if (!pMod) {
        return;
    }
    lua_pushlightuserdata(L, pMod);
    lua_pushlightuserdata(L, pMod);
    lua_pushcclosure(L, Lua_Require, 2);
    lua_setfield(L, nEnv, "require");
    lua_getfield(L, nEnv, "next");
    lua_pushcclosure(L, D2LuaRt_EnvPairs, 1);
    lua_setfield(L, nEnv, "pairs");
    D2LuaRt_PushField(L, "envs");
    lua_pushvalue(L, nEnv);
    lua_setfield(L, -2, pMod->sName.c_str());
    lua_pop(L, 1);
    D2LuaRt_PushField(L, "loaded");
    lua_newtable(L);
    lua_setfield(L, -2, pMod->sName.c_str());
    lua_pop(L, 1);
}

static int Lua_RegisterApi(lua_State* L) {
    const D2LuaApiDesc* pApi = (const D2LuaApiDesc*)lua_touserdata(L, 1);
    int nSide = (int)lua_tointeger(L, 2);
    lua_settop(L, 0);
    pApi->pfnRegister(L, nSide);
    return 0;
}

static int Lua_StartCore(lua_State* L) {
    D2LuaCore_Start(L, (D2LuaRuntime*)lua_touserdata(L, 1));
    return 0;
}

static void LogLine(D2LuaRuntime* pRt, const std::string& sLine) {
    if (pRt->tDesc.pfnLog) {
        pRt->tDesc.pfnLog(sLine.c_str());
    }
}

// The built-in mod's program runs in a client state: its main exports only menu, which only a
// client calls.
static bool RunsBuiltinProgram(const D2LuaRuntime* pRt) {
    return pRt->tDesc.nSide == D2LUA_SIDE_CLIENT && D2LuaCore_FindBuiltinFile("main.lua") != nullptr;
}

static bool BuildState(D2LuaRuntime* pRt) {
    lua_State* L = D2LuaHost_Create(pRt->tDesc.pfnLog, &pRt->tDesc.tLimits);
    if (!L) {
        return false;
    }
    pRt->L = L;
    pRt->aMods.clear();
    pRt->aSettings.clear();
    pRt->aInfos = pRt->tDesc.aMods;
    if (pRt->aInfos.empty() || !pRt->aInfos[0].bIntrinsic) {
        pRt->aInfos.insert(pRt->aInfos.begin(), D2LuaMods_Intrinsic());
    }
    const std::vector<D2LuaModInfo>& aInfos = pRt->aInfos;
    for (size_t i = 0; i < aInfos.size(); i++) {
        const D2LuaModInfo& tInfo = aInfos[i];
        std::unique_ptr<D2LuaMod> pMod(new D2LuaMod);
        pMod->pRt = pRt;
        pMod->nIndex = (int)i;
        pMod->nHostId = D2LuaHost_ModId(L, tInfo.sName.c_str());
        pMod->bIntrinsic = tInfo.bIntrinsic;
        pMod->sName = tInfo.sName;
        pMod->sPath = tInfo.sPath;
        pMod->bTestGrant = Contains(pRt->tDesc.aTestGrant, tInfo.sName);
        pMod->bDriverGrant = Contains(pRt->tDesc.aDriverGrant, tInfo.sName);
        pMod->bManagerGrant = tInfo.bIntrinsic;
        pMod->aRequirable.push_back(tInfo.sName);
        pMod->aRequirable.push_back(D2LUA_INTRINSIC_MOD);
        for (const D2LuaDependency& tDep : tInfo.aDependencies) {
            if (tDep.eKind == D2LuaDepKind::Incompatible) {
                continue;
            }
            bool bPresent = false;
            for (const D2LuaModInfo& tOther : aInfos) {
                bPresent = bPresent || tOther.sName == tDep.sName;
            }
            if (bPresent) {
                pMod->aRequirable.push_back(tDep.sName);
            } else {
                pMod->aOptionalAbsent.push_back(tDep.sName);
            }
        }
        pRt->aMods.push_back(std::move(pMod));
    }
    D2LuaHost_SetHooks(L, EnvHook, ErrorHook, pRt);
    lua_pushcfunction(L, Lua_InitTable);
    if (lua_pcall(L, 0, 0, 0) != 0) {
        LogLine(pRt, std::string("[lua] cannot set up the mod runtime: ") + lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    for (const D2LuaApiDesc* pApi : pRt->tDesc.aApis) {
        if (!pApi->pfnRegister) {
            continue;
        }
        lua_pushcfunction(L, Lua_RegisterApi);
        lua_pushlightuserdata(L, (void*)pApi);
        lua_pushinteger(L, pRt->tDesc.nSide);
        if (lua_pcall(L, 2, 0, 0) != 0) {
            LogLine(pRt, std::string("[lua] cannot register API ") + pApi->pszName + ": " + lua_tostring(L, -1));
            lua_pop(L, 1);
            return false;
        }
    }
    for (auto& pMod : pRt->aMods) {
        if (pMod->bIntrinsic && !RunsBuiltinProgram(pRt)) {
            continue;
        }
        D2LuaHost_NewModEnv(L, pMod->sName.c_str());
        lua_pop(L, 1);
    }
    lua_pushcfunction(L, Lua_StartCore);
    lua_pushlightuserdata(L, pRt);
    if (lua_pcall(L, 1, 0, 0) != 0) {
        LogLine(pRt, std::string("[lua] cannot start the core: ") + lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    return true;
}

// Run one file of a mod's namespace (a data stage entry), when it exists.
// A data stage entry file that fails takes its mod out, as running out of time does: the stages
// after it, and the tables built from them, would otherwise see a mod that is half applied.
static int FaultInStage(D2LuaRuntime* pRt, D2LuaMod* pMod, const std::string& sChunk) {
    D2LuaHost_FaultMod(pRt->L, pMod->sName.c_str());
    LogLine(pRt, "[lua] mod '" + pMod->sName + "' is disabled: " + sChunk + " failed");
    return D2LUA_FAULT;
}

static int RunModFile(D2LuaRuntime* pRt, D2LuaMod* pMod, const char* pszRel) {
    lua_State* L = pRt->L;
    if (D2LuaRt_ModFaulted(L, pMod)) {
        return D2LUA_DISABLED;
    }
    std::string sReal, sCanonical;
    if (!FindModFile(pMod, pszRel, sReal, sCanonical)) {
        return D2LUA_MISSING;
    }
    std::string sChunk = "mods/" + pMod->sName + "/" + sCanonical;
    std::string sSource;
    if (!D2LuaMods_ReadFile(sReal, sSource)) {
        LogLine(pRt, "[lua] " + pMod->sName + ": cannot read " + sChunk);
        return FaultInStage(pRt, pMod, sChunk);
    }
    D2LuaRt_LoadSourceMap(pRt, sChunk.c_str(), (sReal + ".map").c_str());
    if (luaL_loadbufferx(L, sSource.data(), sSource.size(), ("@" + sChunk).c_str(), "t") != 0) {
        LogLine(pRt, "[lua] " + pMod->sName + ": " + lua_tostring(L, -1));
        lua_pop(L, 1);
        return FaultInStage(pRt, pMod, sChunk);
    }
    D2LuaRt_PushEnv(L, pMod);
    lua_setfenv(L, -2);
    int nResult = D2LuaHost_GuardedCall(L, pMod->sName.c_str(), 0, 0);
    if (nResult == D2LUA_ERROR) {
        return FaultInStage(pRt, pMod, sChunk);
    }
    return nResult;
}

// Upvalue 1: the mod. Its main module's exports.
static int Lua_LoadMain(lua_State* L) {
    D2LuaMod* pMod = (D2LuaMod*)lua_touserdata(L, lua_upvalueindex(1));
    D2LuaRt_Require(L, pMod, "main", false);
    return 1;
}

// Load every mod's main and hand its exports to the core.
static void LoadMains(D2LuaRuntime* pRt) {
    lua_State* L = pRt->L;
    for (auto& pMod : pRt->aMods) {
        if ((pMod->bIntrinsic && !RunsBuiltinProgram(pRt)) || D2LuaRt_ModFaulted(L, pMod.get())) {
            continue;
        }
        std::string sReal, sCanonical;
        if (!FindModFile(pMod.get(), "main.lua", sReal, sCanonical)) {
            continue;
        }
        lua_pushlightuserdata(L, pMod.get());
        lua_pushcclosure(L, Lua_LoadMain, 1);
        if (D2LuaHost_GuardedCall(L, pMod->sName.c_str(), 0, 1) != D2LUA_OK) {
            continue;
        }
        lua_pushinteger(L, pMod->nHostId);
        lua_insert(L, -2);
        D2LuaCore_CallHost(L, "setMain", 2, 0);
    }
}

static void RunEntryFiles(D2LuaRuntime* pRt) {
    static const char* const s_apszData[] = { "settings.lua", "data.lua", "data-updates.lua", "data-final-fixes.lua" };
    if (pRt->tDesc.nSide == D2LUA_SIDE_DATA) {
        for (const char* pszStage : s_apszData) {
            if (pRt->tDesc.bSettingsStageOnly && strcmp(pszStage, "settings.lua") != 0) {
                break;
            }
            for (auto& pMod : pRt->aMods) {
                if (!pMod->bIntrinsic) {
                    RunModFile(pRt, pMod.get(), pszStage);
                }
            }
        }
        return;
    }
    LoadMains(pRt);
}

D2LuaRuntime* D2LuaRt_Create(const D2LuaRuntimeDesc& tDesc) {
    D2LuaRuntime* pRt = new D2LuaRuntime;
    pRt->tDesc = tDesc;
    D2LuaRt_LoadSettingsFile(pRt);
    if (!BuildState(pRt)) {
        D2LuaHost_Destroy(pRt->L);
        delete pRt;
        return nullptr;
    }
    RunEntryFiles(pRt);
    return pRt;
}

// --- games -----------------------------------------------------------------------------------------

namespace {
// The game the host works on for the length of one call from the host; restored after.
struct GameScope {
    D2LuaRuntime* pRt;
    uint32_t nWas;
    GameScope(D2LuaRuntime* p, uint32_t nGame) : pRt(p), nWas(p->nGame) { p->nGame = nGame; }
    ~GameScope() { pRt->nGame = nWas; }
};
}  // namespace

// Read the storage blob: "RDVS" 1, mod count, then (name, value) pairs, each value into the mod's
// storage table for the game. Runs protected. Arg 1: the bytes, arg 2: the game.
static int Lua_RestoreStorage(lua_State* L) {
    const std::string* pIn = (const std::string*)lua_touserdata(L, 1);
    lua_Number nGame = lua_tonumber(L, 2);
    D2LuaRuntime* pRt = D2LuaRt_FromState(L);
    lua_settop(L, 0);
    if (pIn->size() < 9 || pIn->compare(0, 4, "RDVS") != 0 || (unsigned char)(*pIn)[4] != 1) {
        return luaL_error(L, "storage: not a storage save (bad header)");
    }
    size_t nPos = 5;
    unsigned nMods = 0;
    for (int i = 0; i < 4; i++) {
        nMods |= (unsigned)(unsigned char)(*pIn)[nPos++] << (8 * i);
    }
    for (unsigned i = 0; i < nMods; i++) {
        D2LuaStorage_Read(L, pIn, &nPos, 0);  // the name
        if (lua_type(L, -1) != LUA_TSTRING) {
            return luaL_error(L, "storage: corrupt save (mod name)");
        }
        D2LuaMod* pMod = D2LuaRt_ModByName(pRt, lua_tostring(L, -1));
        lua_pop(L, 1);
        if (!pMod || pMod->bIntrinsic) {
            D2LuaStorage_Read(L, pIn, &nPos, 0);  // a mod that is not loaded: read and drop
            lua_pop(L, 1);
            continue;
        }
        lua_pushnumber(L, nGame);
        lua_pushinteger(L, pMod->nHostId);
        D2LuaCore_CallHost(L, "storageOf", 2, 1);
        if (!lua_istable(L, -1)) {
            return luaL_error(L, "storage: the game is not running");
        }
        D2LuaStorage_Read(L, pIn, &nPos, -1);
        if (!lua_rawequal(L, -1, -2)) {
            return luaL_error(L, "storage: corrupt save (the storage of mod '%s' is not a table)", pMod->sName.c_str());
        }
        lua_pop(L, 2);
        lua_pushnumber(L, nGame);
        lua_pushinteger(L, pMod->nHostId);
        D2LuaCore_CallHost(L, "markLoaded", 2, 0);
    }
    if (nPos != pIn->size()) {
        return luaL_error(L, "storage: corrupt save (trailing bytes)");
    }
    return 0;
}

static bool RestoreStorage(D2LuaRuntime* pRt, uint32_t nGame, const std::string* pStorage, std::string& sError) {
    lua_State* L = pRt->L;
    lua_pushcfunction(L, Lua_RestoreStorage);
    lua_pushlightuserdata(L, (void*)pStorage);
    lua_pushnumber(L, (lua_Number)nGame);
    if (lua_pcall(L, 2, 0, 0) != 0) {
        sError = lua_tostring(L, -1) ? lua_tostring(L, -1) : "storage: error";
        lua_pop(L, 1);
        return false;
    }
    return true;
}

bool D2LuaRt_StartGame(D2LuaRuntime* pRt, uint32_t nGame, const std::string* pStorage, std::string& sError) {
    if (pRt->tDesc.nSide == D2LUA_SIDE_DATA) {
        sError = "the data stage has no games";
        return false;
    }
    GameScope tScope(pRt, nGame);
    lua_State* L = pRt->L;
    if (!pRt->tTicks.count(nGame)) {
        pRt->tTicks[nGame] = 0;
    }
    lua_pushnumber(L, (lua_Number)nGame);
    if (!D2LuaCore_CallHost(L, "newGame", 1, 0)) {
        sError = "the game could not start (see the log)";
        return false;
    }
    bool bOk = true;
    if (pStorage) {
        bOk = RestoreStorage(pRt, nGame, pStorage, sError);
    }
    lua_pushnumber(L, (lua_Number)nGame);
    D2LuaCore_CallHost(L, "runEntries", 1, 0);
    return bOk;
}

bool D2LuaRt_EndGame(D2LuaRuntime* pRt, uint32_t nGame) {
    GameScope tScope(pRt, nGame);
    lua_State* L = pRt->L;
    lua_pushnumber(L, (lua_Number)nGame);
    bool bEnded = D2LuaCore_CallHost(L, "endGame", 1, 1) && lua_toboolean(L, -1);
    lua_settop(L, 0);
    pRt->tTicks.erase(nGame);
    return bEnded;
}

void D2LuaRt_Tick(D2LuaRuntime* pRt, uint32_t nGame) {
    GameScope tScope(pRt, nGame);
    pRt->tTicks[nGame]++;
    lua_pushnumber(pRt->L, (lua_Number)nGame);
    D2LuaCore_CallHost(pRt->L, "tick", 1, 0);
}

bool D2LuaRt_CallHost(D2LuaRuntime* pRt, uint32_t nGame, const char* pszName, int nArgs, int nResults) {
    GameScope tScope(pRt, nGame);
    return D2LuaCore_CallHost(pRt->L, pszName, nArgs, nResults);
}

long long D2LuaRt_CurrentTick(D2LuaRuntime* pRt, uint32_t nGame) {
    auto it = pRt->tTicks.find(nGame);
    return it == pRt->tTicks.end() ? 0 : it->second;
}

static bool RaiseIn(D2LuaRuntime* pRt, uint32_t nGame, const char* pszEvent) {
    lua_State* L = pRt->L;
    int nPayload = lua_gettop(L);
    lua_pushnumber(L, (lua_Number)nGame);
    lua_pushstring(L, pszEvent);
    lua_pushvalue(L, nPayload);
    lua_remove(L, nPayload);
    if (!D2LuaCore_CallHost(L, "dispatchEvent", 3, 1)) {
        return false;
    }
    bool bRaised = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);
    return bRaised;
}

bool D2LuaRt_RaiseEvent(D2LuaRuntime* pRt, uint32_t nGame, const char* pszEvent) {
    GameScope tScope(pRt, nGame);
    return RaiseIn(pRt, nGame, pszEvent);
}

int D2LuaApi_RaiseEvent(lua_State* L, const char* pszEvent) {
    D2LuaRuntime* pRt = D2LuaRt_FromState(L);
    if (!pRt) {
        lua_pop(L, 1);
        return 0;
    }
    return RaiseIn(pRt, pRt->nGame, pszEvent) ? 1 : 0;
}

int D2LuaRt_CallRegistry(D2LuaRuntime* pRt, uint32_t nGame, const char* pszRegistry, const char* pszId, int nArgs) {
    GameScope tScope(pRt, nGame);
    lua_State* L = pRt->L;
    int nFirst = lua_gettop(L) - nArgs + 1;
    lua_pushnumber(L, (lua_Number)nGame);
    lua_insert(L, nFirst);
    lua_pushstring(L, pszRegistry);
    lua_insert(L, nFirst + 1);
    lua_pushstring(L, pszId ? pszId : "");
    lua_insert(L, nFirst + 2);
    return D2LuaCore_CallHost(L, "registryCall", nArgs + 3, 1) ? D2LUA_OK : D2LUA_ERROR;
}

static int Lua_SaveStorage(lua_State* L) {
    std::string* pOut = (std::string*)lua_touserdata(L, 1);
    lua_Number nGame = lua_tonumber(L, 2);
    std::string* pBody = (std::string*)lua_touserdata(L, lua_upvalueindex(1));
    D2LuaRuntime* pRt = D2LuaRt_FromState(L);
    lua_settop(L, 0);
    unsigned nMods = 0;
    for (auto& pMod : pRt->aMods) {
        if (pMod->bIntrinsic || D2LuaRt_ModFaulted(L, pMod.get())) {
            continue;
        }
        lua_pushnumber(L, nGame);
        lua_pushinteger(L, pMod->nHostId);
        D2LuaCore_CallHost(L, "storageOf", 2, 1);
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            continue;
        }
        lua_pushnil(L);
        if (!lua_next(L, -2)) {
            lua_pop(L, 1);  // empty: nothing to save
            continue;
        }
        lua_pop(L, 2);
        lua_pushstring(L, pMod->sName.c_str());
        D2LuaStorage_Write(L, -1, pBody, "");
        lua_pop(L, 1);
        D2LuaStorage_Write(L, -1, pBody, pMod->sName.c_str());
        lua_pop(L, 1);
        nMods++;
    }
    pOut->assign("RDVS\x01", 5);
    for (int i = 0; i < 4; i++) {
        pOut->push_back((char)((nMods >> (8 * i)) & 0xff));
    }
    pOut->append(*pBody);
    return 0;
}

bool D2LuaRt_SaveStorage(D2LuaRuntime* pRt, uint32_t nGame, std::string& sOut, std::string& sError) {
    GameScope tScope(pRt, nGame);
    lua_State* L = pRt->L;
    std::string sBody;
    sOut.clear();
    try {
        lua_pushlightuserdata(L, &sBody);
        lua_pushcclosure(L, Lua_SaveStorage, 1);
        lua_pushlightuserdata(L, &sOut);
        lua_pushnumber(L, (lua_Number)nGame);
        if (lua_pcall(L, 2, 0, 0) != 0) {
            sError = lua_tostring(L, -1);
            lua_pop(L, 1);
            sOut.clear();
            return false;
        }
    } catch (const std::bad_alloc&) {
        sError = "storage: out of memory";
        sOut.clear();
        return false;
    }
    return true;
}

bool D2LuaRt_ReloadGame(D2LuaRuntime* pRt, uint32_t nGame, std::string& sError) {
    std::string sStorage;
    if (!D2LuaRt_SaveStorage(pRt, nGame, sStorage, sError)) {
        return false;
    }
    GameScope tScope(pRt, nGame);
    lua_State* L = pRt->L;
    lua_pushnumber(L, (lua_Number)nGame);
    if (!D2LuaCore_CallHost(L, "prepareReload", 1, 1) || !lua_toboolean(L, -1)) {
        lua_settop(L, 0);
        sError = "the game is not running";
        return false;
    }
    lua_settop(L, 0);
    // Every mod's modules afresh: the require caches and the public module instances.
    D2LuaRt_PushField(L, "loaded");
    for (auto& pMod : pRt->aMods) {
        if (!pMod->bIntrinsic) {
            lua_newtable(L);
            lua_setfield(L, -2, pMod->sName.c_str());
        }
    }
    lua_pop(L, 1);
    D2LuaCore_ForgetInstances(L);
    pRt->nGeneration++;
    pRt->tMaps.clear();
    LoadMains(pRt);
    bool bOk = RestoreStorage(pRt, nGame, &sStorage, sError);
    lua_pushnumber(L, (lua_Number)nGame);
    D2LuaCore_CallHost(L, "runEntries", 1, 0);
    return bOk;
}

// --- accessors ------------------------------------------------------------------------------------

lua_State* D2LuaRt_State(D2LuaRuntime* pRt) {
    return pRt ? pRt->L : nullptr;
}

int D2LuaRt_Side(D2LuaRuntime* pRt) {
    return pRt->tDesc.nSide;
}

const std::vector<D2LuaModInfo>& D2LuaRt_Mods(D2LuaRuntime* pRt) {
    return pRt->aInfos;
}

bool D2LuaRt_IsFaulted(D2LuaRuntime* pRt, const char* pszMod) {
    return D2LuaHost_IsModFaulted(pRt->L, pszMod) != 0;
}

int D2LuaRt_Run(D2LuaRuntime* pRt, uint32_t nGame, const char* pszMod, const char* pszSource) {
    GameScope tScope(pRt, nGame);
    return D2LuaHost_RunString(pRt->L, pszMod, pszSource, "=test");
}

void D2LuaRt_Destroy(D2LuaRuntime* pRt) {
    if (!pRt) {
        return;
    }
    D2LuaHost_Destroy(pRt->L);
    delete pRt;
}
#endif
