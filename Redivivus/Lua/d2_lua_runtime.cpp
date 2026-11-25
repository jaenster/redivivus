// The mod runtime. See d2_lua_runtime.h for the model and d2_lua_rt.h for where its state lives.
//
// What is C++ and what is Lua: everything here is C++ against the Lua C API. The host keeps no
// privileged Lua of its own beyond the two small wrappers in d2_lua_host.cpp (xpcall,
// coroutine.wrap), so there is no host Lua for a mod to reach; the runtime's Lua-side state is a
// registry table no mod environment holds.
//
// Functions Lua calls (Lua_*) follow the host's rule: no object with a destructor in their
// frames. The C++ helpers they call (source maps) complete before any Lua call and catch their
// own exceptions.
#if D2_FEATURE_MODS
#include "Redivivus/Lua/d2_lua_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    return D2LuaHost_IsModFaulted(L, pMod->sName.c_str()) != 0;
}

static int Lua_InitTable(lua_State* L) {
    lua_pushlightuserdata(L, &s_RtKey);
    lua_newtable(L);
    static const char* const s_apszTables[] = {
        "envs", "handlers", "init", "load", "loaded", "remote", "events", "registries", "scripts", "classes", "enums", "messages",
    };
    for (size_t i = 0; i < sizeof(s_apszTables) / sizeof(s_apszTables[0]); i++) {
        lua_newtable(L);
        lua_setfield(L, -2, s_apszTables[i]);
    }
    lua_newtable(L);
    lua_newtable(L);
    lua_pushliteral(L, "k");
    lua_setfield(L, -2, "__mode");
    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "script_threads");
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
        // resolved against the chunk's directory ("__mod__/server/control.lua" with
        // "control.ts" -> "__mod__/server/control.ts").
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

// --- require ------------------------------------------------------------------------------------

// Split a module name into the mod it lives in and the mod-relative path ("shared/x"). Dots
// become slashes; a trailing ".lua" is dropped. Returns null on success, or why not.
static const char* ParseModuleName(const char* pszName, const char* pszSelf, char* pszTarget, size_t nTarget,
                                   char* pszRest, size_t nRest) {
    const char* p = pszName;
    if (p[0] == '_' && p[1] == '_') {
        const char* pEnd = strstr(p + 2, "__");
        if (!pEnd || (pEnd[2] != '/' && pEnd[2] != '.') || pEnd == p + 2) {
            return "a mod prefix is written __name__/";
        }
        size_t n = (size_t)(pEnd - (p + 2));
        if (n + 1 > nTarget) {
            return "the mod name is too long";
        }
        memcpy(pszTarget, p + 2, n);
        pszTarget[n] = '\0';
        p = pEnd + 3;
    } else {
        snprintf(pszTarget, nTarget, "%s", pszSelf);
    }
    size_t nLen = strlen(p);
    if (nLen >= 4 && strcmp(p + nLen - 4, ".lua") == 0) {
        nLen -= 4;
    }
    if (nLen == 0 || nLen + 1 > nRest) {
        return "the module path is empty or too long";
    }
    bool bSegmentStart = true;
    for (size_t i = 0; i < nLen; i++) {
        char c = p[i];
        if (c == '.' || c == '/') {
            if (bSegmentStart) {
                return "empty path segment (no '..', no leading or doubled separators)";
            }
            pszRest[i] = '/';
            bSegmentStart = true;
            continue;
        }
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-')) {
            return "module paths use letters, digits, '_', '-' and '.' or '/' between parts";
        }
        pszRest[i] = c;
        bSegmentStart = false;
    }
    if (bSegmentStart) {
        return "the module path ends in a separator";
    }
    pszRest[nLen] = '\0';
    return nullptr;
}

static const char* SideName(int nSide) {
    return nSide == D2LUA_SIDE_DATA ? "data" : nSide == D2LUA_SIDE_SERVER ? "server" : "client";
}

// Whether code of side nSide may require the mod-relative path pszRest; null when it may.
static const char* SideRule(int nSide, const char* pszRest) {
    size_t nFirst = strcspn(pszRest, "/");
    bool bHasDir = pszRest[nFirst] == '/';
    auto IsDir = [&](const char* pszDir) { return bHasDir && nFirst == strlen(pszDir) && strncmp(pszRest, pszDir, nFirst) == 0; };
    if (nSide == D2LUA_SIDE_DATA) {
        if (IsDir("server") || IsDir("client")) {
            return "the data stage cannot require control-stage code (server/ or client/)";
        }
        return nullptr;
    }
    const char* pszOwn = SideName(nSide);
    if (IsDir(pszOwn) || IsDir("shared")) {
        return nullptr;
    }
    if (IsDir("server") || IsDir("client")) {
        return nSide == D2LUA_SIDE_SERVER ? "server code cannot require client/ (each side runs in its own state)"
                                          : "client code cannot require server/ (each side runs in its own state)";
    }
    return nSide == D2LUA_SIDE_SERVER ? "server code can require modules under server/ and shared/ only"
                                      : "client code can require modules under client/ and shared/ only";
}

// Push the contents of a file as a Lua string, or nil when it cannot be read. The buffer is a
// userdata, so an error on the way leaks nothing.
static void PushFile(lua_State* L, const char* pszPath) {
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

static int Lua_Require(lua_State* L) {
    D2LuaMod* pMod = (D2LuaMod*)lua_touserdata(L, lua_upvalueindex(1));
    D2LuaRuntime* pRt = pMod->pRt;
    const char* pszName = luaL_checkstring(L, 1);
    lua_settop(L, 1);
    char szTarget[80];
    char szRest[400];
    const char* pszWhy = ParseModuleName(pszName, pMod->sName.c_str(), szTarget, sizeof(szTarget), szRest, sizeof(szRest));
    if (pszWhy) {
        return luaL_error(L, "require '%s': %s", pszName, pszWhy);
    }
    D2LuaMod* pTarget = D2LuaRt_ModByName(pRt, szTarget);
    bool bAllowed = false;
    for (const std::string& sName : pMod->aRequirable) {
        if (strcmp(sName.c_str(), szTarget) == 0) {
            bAllowed = true;
        }
    }
    if (!bAllowed || !pTarget) {
        return luaL_error(L, "require '%s': mod '%s' does not depend on '%s' (declare it in info.json)", pszName,
                          pMod->sName.c_str(), szTarget);
    }
    pszWhy = SideRule(pRt->tDesc.nSide, szRest);
    if (pszWhy) {
        return luaL_error(L, "require '%s': %s", pszName, pszWhy);
    }
    lua_pushfstring(L, "__%s__/%s", szTarget, szRest);  // 2: key
    D2LuaRt_PushField(L, "loaded");
    lua_getfield(L, -1, pMod->sName.c_str());  // 4: this mod's cache
    lua_remove(L, 3);                           // 3: cache
    lua_pushvalue(L, 2);
    lua_rawget(L, 3);
    if (lua_touserdata(L, -1) == &s_LoadingKey) {
        return luaL_error(L, "require '%s': loop or previous error loading module", pszName);
    }
    if (!lua_isnil(L, -1)) {
        return 1;
    }
    lua_pop(L, 1);
    lua_pushfstring(L, "%s/%s.lua", pTarget->sPath.c_str(), szRest);  // 4: path
    PushFile(L, lua_tostring(L, 4));                                   // 5: source
    if (lua_isnil(L, 5)) {
        return luaL_error(L, "require '%s': module not found (no file %s.lua in mod '%s')", pszName, szRest, szTarget);
    }
    lua_pushfstring(L, "%s.map", lua_tostring(L, 4));
    D2LuaRt_LoadSourceMap(pRt, lua_tostring(L, 2), lua_tostring(L, -1));
    lua_pop(L, 1);
    lua_pushfstring(L, "@%s.lua", lua_tostring(L, 2));  // 6: chunk name
    size_t nSource = 0;
    const char* pSource = lua_tolstring(L, 5, &nSource);
    if (luaL_loadbufferx(L, pSource, nSource, lua_tostring(L, 6), "t") != 0) {
        return lua_error(L);
    }
    D2LuaRt_PushEnv(L, pMod);
    lua_setfenv(L, -2);
    lua_pushvalue(L, 2);
    lua_pushlightuserdata(L, &s_LoadingKey);
    lua_rawset(L, 3);
    lua_pushvalue(L, 1);
    lua_call(L, 1, 1);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_pushboolean(L, 1);
    }
    lua_pushvalue(L, 2);
    lua_pushvalue(L, -2);
    lua_rawset(L, 3);
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
    lua_pushcclosure(L, Lua_Require, 1);
    lua_setfield(L, nEnv, "require");
    lua_getfield(L, nEnv, "next");
    lua_pushcclosure(L, D2LuaRt_EnvPairs, 1);
    lua_setfield(L, nEnv, "pairs");
    for (const D2LuaApiDesc* pApi : pRt->tDesc.aApis) {
        if (pApi->pfnInstall) {
            pApi->pfnInstall(L, nEnv, pMod, pRt->tDesc.nSide);
        }
    }
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

static void LogLine(D2LuaRuntime* pRt, const std::string& sLine) {
    if (pRt->tDesc.pfnLog) {
        pRt->tDesc.pfnLog(sLine.c_str());
    }
}

static bool BuildState(D2LuaRuntime* pRt) {
    lua_State* L = D2LuaHost_Create(pRt->tDesc.pfnLog, &pRt->tDesc.tLimits);
    if (!L) {
        return false;
    }
    pRt->L = L;
    pRt->aMods.clear();
    pRt->aSettings.clear();
    for (size_t i = 0; i < pRt->tDesc.aMods.size(); i++) {
        const D2LuaModInfo& tInfo = pRt->tDesc.aMods[i];
        std::unique_ptr<D2LuaMod> pMod(new D2LuaMod);
        pMod->pRt = pRt;
        pMod->nIndex = (int)i;
        pMod->nHostId = D2LuaHost_ModId(L, tInfo.sName.c_str());
        pMod->sName = tInfo.sName;
        pMod->sPath = tInfo.sPath;
        pMod->aRequirable.push_back(tInfo.sName);
        for (const D2LuaDependency& tDep : tInfo.aDependencies) {
            if (tDep.eKind == D2LuaDepKind::Incompatible) {
                continue;
            }
            for (const D2LuaModInfo& tOther : pRt->tDesc.aMods) {
                if (tOther.sName == tDep.sName) {
                    pMod->aRequirable.push_back(tDep.sName);
                }
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
        D2LuaHost_NewModEnv(L, pMod->sName.c_str());
        lua_pop(L, 1);
    }
    return true;
}

// Run one mod-relative file of a mod, when it exists.
static int RunModFile(D2LuaRuntime* pRt, D2LuaMod* pMod, const char* pszRel) {
    lua_State* L = pRt->L;
    if (D2LuaRt_ModFaulted(L, pMod)) {
        return D2LUA_DISABLED;
    }
    std::string sPath = pMod->sPath + "/" + pszRel;
    std::string sSource;
    if (!D2LuaMods_ReadFile(sPath, sSource)) {
        return D2LUA_MISSING;
    }
    std::string sChunk = "__" + pMod->sName + "__/" + pszRel;
    D2LuaRt_LoadSourceMap(pRt, sChunk.c_str(), (sPath + ".map").c_str());
    if (luaL_loadbufferx(L, sSource.data(), sSource.size(), ("@" + sChunk).c_str(), "t") != 0) {
        LogLine(pRt, "[lua] " + pMod->sName + ": " + lua_tostring(L, -1));
        lua_pop(L, 1);
        return D2LUA_ERROR;
    }
    D2LuaRt_PushEnv(L, pMod);
    lua_setfenv(L, -2);
    return D2LuaHost_GuardedCall(L, pMod->sName.c_str(), 0, 0);
}

static void RunEntryFiles(D2LuaRuntime* pRt) {
    static const char* const s_apszData[] = { "settings.lua", "data.lua", "data-updates.lua", "data-final-fixes.lua" };
    if (pRt->tDesc.nSide == D2LUA_SIDE_DATA) {
        for (const char* pszStage : s_apszData) {
            if (pRt->tDesc.bSettingsStageOnly && strcmp(pszStage, "settings.lua") != 0) {
                break;
            }
            for (auto& pMod : pRt->aMods) {
                RunModFile(pRt, pMod.get(), pszStage);
            }
        }
        return;
    }
    std::string sControl = std::string(SideName(pRt->tDesc.nSide)) + "/control.lua";
    for (auto& pMod : pRt->aMods) {
        RunModFile(pRt, pMod.get(), sControl.c_str());
    }
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

// Call every mod's on_init or on_load handler, in load order.
static void CallLifecycle(D2LuaRuntime* pRt, const char* pszWhich) {
    lua_State* L = pRt->L;
    for (auto& pMod : pRt->aMods) {
        if (D2LuaRt_ModFaulted(L, pMod.get())) {
            continue;
        }
        D2LuaRt_PushField(L, pszWhich);
        lua_getfield(L, -1, pMod->sName.c_str());
        lua_remove(L, -2);
        if (!lua_isfunction(L, -1)) {
            lua_pop(L, 1);
            continue;
        }
        D2LuaHost_GuardedCall(L, pMod->sName.c_str(), 0, 0);
    }
}

// Read the storage blob: "RDVS" 1, mod count, then (name, value) pairs. Runs protected.
static int Lua_RestoreStorage(lua_State* L) {
    const std::string* pIn = (const std::string*)lua_touserdata(L, 1);
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
    lua_newtable(L);  // 1: name -> value
    for (unsigned i = 0; i < nMods; i++) {
        D2LuaStorage_Read(L, pIn, &nPos);  // the name
        if (lua_type(L, -1) != LUA_TSTRING) {
            return luaL_error(L, "storage: corrupt save (mod name)");
        }
        D2LuaStorage_Read(L, pIn, &nPos);
        lua_rawset(L, 1);
    }
    if (nPos != pIn->size()) {
        return luaL_error(L, "storage: corrupt save (trailing bytes)");
    }
    for (auto& pMod : pRt->aMods) {
        lua_getfield(L, 1, pMod->sName.c_str());
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            continue;
        }
        D2LuaRt_PushEnv(L, pMod.get());
        if (lua_istable(L, -1)) {
            lua_pushvalue(L, -2);
            lua_setfield(L, -2, "storage");
        }
        lua_pop(L, 2);
    }
    return 0;
}

bool D2LuaRt_Start(D2LuaRuntime* pRt, const std::string* pStorage, std::string& sError) {
    if (pRt->tDesc.nSide == D2LUA_SIDE_DATA) {
        return true;
    }
    lua_State* L = pRt->L;
    if (pStorage) {
        lua_pushcfunction(L, Lua_RestoreStorage);
        lua_pushlightuserdata(L, (void*)pStorage);
        if (lua_pcall(L, 1, 0, 0) != 0) {
            sError = lua_tostring(L, -1);
            lua_pop(L, 1);
            CallLifecycle(pRt, "init");
            return false;
        }
        CallLifecycle(pRt, "load");
        return true;
    }
    CallLifecycle(pRt, "init");
    return true;
}

static int Lua_SaveStorage(lua_State* L) {
    std::string* pOut = (std::string*)lua_touserdata(L, 1);
    D2LuaRuntime* pRt = D2LuaRt_FromState(L);
    lua_settop(L, 0);
    unsigned nMods = 0;
    std::string* pBody = (std::string*)lua_touserdata(L, lua_upvalueindex(1));
    for (auto& pMod : pRt->aMods) {
        if (D2LuaRt_ModFaulted(L, pMod.get())) {
            continue;
        }
        D2LuaRt_PushEnv(L, pMod.get());
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            continue;
        }
        lua_pushliteral(L, "storage");
        lua_rawget(L, -2);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 2);
            continue;
        }
        if (!lua_istable(L, -1)) {
            return luaL_error(L, "storage of mod '%s' must be a table, not a %s", pMod->sName.c_str(), luaL_typename(L, -1));
        }
        lua_pushstring(L, pMod->sName.c_str());
        D2LuaStorage_Write(L, -1, pBody, "");
        lua_pop(L, 1);
        D2LuaStorage_Write(L, -1, pBody, pMod->sName.c_str());
        lua_pop(L, 2);
        nMods++;
    }
    pOut->assign("RDVS\x01", 5);
    for (int i = 0; i < 4; i++) {
        pOut->push_back((char)((nMods >> (8 * i)) & 0xff));
    }
    pOut->append(*pBody);
    return 0;
}

bool D2LuaRt_SaveStorage(D2LuaRuntime* pRt, std::string& sOut, std::string& sError) {
    lua_State* L = pRt->L;
    std::string sBody;
    sOut.clear();
    try {
        lua_pushlightuserdata(L, &sBody);
        lua_pushcclosure(L, Lua_SaveStorage, 1);
        lua_pushlightuserdata(L, &sOut);
        if (lua_pcall(L, 1, 0, 0) != 0) {
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

bool D2LuaRt_Reload(D2LuaRuntime* pRt, std::string& sError) {
    std::string sStorage;
    if (!D2LuaRt_SaveStorage(pRt, sStorage, sError)) {
        return false;
    }
    long long nTick = pRt->nTick;
    D2LuaHost_Destroy(pRt->L);
    pRt->L = nullptr;
    pRt->tMaps.clear();
    if (!BuildState(pRt)) {
        sError = "the state could not be rebuilt";
        return false;
    }
    pRt->nTick = nTick;
    RunEntryFiles(pRt);
    return D2LuaRt_Start(pRt, &sStorage, sError);
}

// --- events and scripts -------------------------------------------------------------------------

// Resume the script record at index nRecord (absolute) with the nArgs values at the top of the
// stack (popped). Updates the record: due, event, done.
static void ResumeScript(D2LuaRuntime* pRt, int nRecord, int nArgs) {
    lua_State* L = pRt->L;
    lua_getfield(L, nRecord, "co");
    lua_State* co = lua_tothread(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, nRecord, "owner");
    char szOwner[80];
    snprintf(szOwner, sizeof(szOwner), "%s", lua_tostring(L, -1));
    lua_pop(L, 1);
    if (!co || D2LuaHost_IsModFaulted(L, szOwner)) {
        lua_pop(L, nArgs);
        lua_pushboolean(L, 1);
        lua_setfield(L, nRecord, "done");
        return;
    }
    lua_xmove(L, co, nArgs);
    int nResults = 0;
    int nResult = D2LuaHost_GuardedResume(L, co, szOwner, nArgs, &nResults);
    bool bDone = nResult != D2LUA_YIELD;
    lua_pushnil(L);
    lua_setfield(L, nRecord, "event");
    if (!bDone) {
        long long nDue = pRt->nTick + 1;
        if (nResults >= 1 && lua_istable(co, -nResults)) {
            int nWait = lua_gettop(co) - nResults + 1;
            lua_getfield(co, nWait, "ticks");
            lua_getfield(co, nWait, "event");
            if (lua_isnumber(co, -2)) {
                long long nTicks = (long long)lua_tonumber(co, -2);
                nDue = pRt->nTick + (nTicks < 1 ? 1 : nTicks);
            } else if (lua_type(co, -1) == LUA_TSTRING) {
                D2LuaRt_PushField(L, "events");
                lua_pushstring(L, lua_tostring(co, -1));
                lua_rawget(L, -2);
                bool bKnown = !lua_isnil(L, -1);
                lua_pop(L, 2);
                if (bKnown) {
                    lua_pushstring(L, lua_tostring(co, -1));
                    lua_setfield(L, nRecord, "event");
                } else {
                    LogLine(pRt, std::string("[lua] ") + szOwner + ": a script waits for unknown event '"
                                     + lua_tostring(co, -1) + "'; the script is stopped");
                    bDone = true;
                }
            } else {
                LogLine(pRt, std::string("[lua] ") + szOwner
                                 + ": a script yielded a table that is not a wait ({ ticks = n } or { event = name }); "
                                   "the script is stopped");
                bDone = true;
            }
        } else if (nResults >= 1 && !lua_isnil(co, -nResults)) {
            LogLine(pRt, std::string("[lua] ") + szOwner + ": a script yielded a " + luaL_typename(co, -nResults)
                             + ", which is not a wait; the script is stopped");
            bDone = true;
        }
        lua_pushnumber(L, (lua_Number)nDue);
        lua_setfield(L, nRecord, "due");
    }
    lua_settop(co, 0);
    if (bDone) {
        lua_pushboolean(L, 1);
        lua_setfield(L, nRecord, "done");
    }
}

// Drop finished scripts from the list.
static void CompactScripts(lua_State* L) {
    D2LuaRt_PushField(L, "scripts");
    int nScripts = lua_gettop(L);
    int nCount = (int)lua_objlen(L, nScripts);
    int nOut = 0;
    for (int i = 1; i <= nCount; i++) {
        lua_rawgeti(L, nScripts, i);
        lua_getfield(L, -1, "done");
        bool bDone = lua_toboolean(L, -1) != 0;
        lua_pop(L, 1);
        if (bDone) {
            lua_pop(L, 1);
            continue;
        }
        lua_rawseti(L, nScripts, ++nOut);
    }
    for (int i = nOut + 1; i <= nCount; i++) {
        lua_pushnil(L);
        lua_rawseti(L, nScripts, i);
    }
    lua_pop(L, 1);
}

// Resume the scripts selected by pszEvent (waiting for that event; payload at nPayload) or, when
// null, the scripts whose tick has come. Scripts started during the pass wait for the next one.
static void ResumeScripts(D2LuaRuntime* pRt, const char* pszEvent, int nPayload) {
    lua_State* L = pRt->L;
    D2LuaRt_PushField(L, "scripts");
    int nScripts = lua_gettop(L);
    int nCount = (int)lua_objlen(L, nScripts);
    for (int i = 1; i <= nCount; i++) {
        lua_rawgeti(L, nScripts, i);
        int nRecord = lua_gettop(L);
        lua_getfield(L, nRecord, "holder");
        if (!lua_isnil(L, -1) && !D2LuaHandle_IsLive(L, -1)) {
            lua_pushboolean(L, 1);
            lua_setfield(L, nRecord, "done");  // the object it lived on is gone
        }
        lua_pop(L, 1);
        lua_getfield(L, nRecord, "done");
        lua_getfield(L, nRecord, "event");
        lua_getfield(L, nRecord, "due");
        bool bDone = lua_toboolean(L, -3) != 0;
        bool bRun = false;
        if (!bDone) {
            if (pszEvent) {
                bRun = lua_type(L, -2) == LUA_TSTRING && strcmp(lua_tostring(L, -2), pszEvent) == 0;
            } else {
                bRun = lua_isnil(L, -2) && lua_tonumber(L, -1) <= (lua_Number)pRt->nTick;
            }
        }
        lua_pop(L, 3);
        if (bRun) {
            if (pszEvent) {
                lua_pushvalue(L, nPayload);
            } else {
                lua_pushnumber(L, (lua_Number)pRt->nTick);
            }
            ResumeScript(pRt, nRecord, 1);
        }
        lua_settop(L, nRecord - 1);
    }
    lua_settop(L, nScripts - 1);
    CompactScripts(L);
}

bool D2LuaRt_RaiseEvent(D2LuaRuntime* pRt, const char* pszEvent) {
    lua_State* L = pRt->L;
    int nPayload = lua_gettop(L);
    D2LuaRt_PushField(L, "events");
    lua_getfield(L, -1, pszEvent);
    bool bKnown = !lua_isnil(L, -1);
    lua_pop(L, 2);
    if (!bKnown) {
        LogLine(pRt, std::string("[lua] the host raised unknown event '") + pszEvent + "' on the "
                         + SideName(pRt->tDesc.nSide) + " side");
        lua_settop(L, nPayload - 1);
        return false;
    }
    lua_pushstring(L, pszEvent);
    lua_setfield(L, nPayload, "name");
    lua_pushnumber(L, (lua_Number)pRt->nTick);
    lua_setfield(L, nPayload, "tick");
    for (auto& pMod : pRt->aMods) {
        if (D2LuaRt_ModFaulted(L, pMod.get())) {
            continue;
        }
        D2LuaRt_PushField(L, "handlers");
        lua_getfield(L, -1, pMod->sName.c_str());
        lua_remove(L, -2);
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            continue;
        }
        lua_getfield(L, -1, pszEvent);
        lua_remove(L, -2);
        if (!lua_isfunction(L, -1)) {
            lua_pop(L, 1);
            continue;
        }
        lua_pushvalue(L, nPayload);
        D2LuaHost_GuardedCall(L, pMod->sName.c_str(), 1, 0);
    }
    ResumeScripts(pRt, pszEvent, nPayload);
    lua_settop(L, nPayload - 1);
    return true;
}

int D2LuaApi_RaiseEvent(lua_State* L, const char* pszEvent) {
    D2LuaRuntime* pRt = D2LuaRt_FromState(L);
    if (!pRt) {
        lua_pop(L, 1);
        return 0;
    }
    return D2LuaRt_RaiseEvent(pRt, pszEvent) ? 1 : 0;
}

void D2LuaRt_Tick(D2LuaRuntime* pRt) {
    lua_State* L = pRt->L;
    pRt->nTick++;
    D2LuaRt_PushField(L, "events");
    lua_getfield(L, -1, "on_tick");
    bool bHasTick = !lua_isnil(L, -1);
    lua_pop(L, 2);
    if (bHasTick) {
        lua_newtable(L);
        D2LuaRt_RaiseEvent(pRt, "on_tick");
    }
    ResumeScripts(pRt, nullptr, 0);
}

long long D2LuaRt_CurrentTick(D2LuaRuntime* pRt) {
    return pRt->nTick;
}

// --- registries from the host ---------------------------------------------------------------------

static int Lua_HostRegistryCall(lua_State* L) {
    const char* pszRegistry = lua_tostring(L, lua_upvalueindex(1));
    const char* pszId = lua_tostring(L, lua_upvalueindex(2));
    return D2LuaRt_RegistryCallProtected(L, pszRegistry, pszId, 1);
}

int D2LuaRt_CallRegistry(D2LuaRuntime* pRt, const char* pszRegistry, const char* pszId, int nArgs, int nResults) {
    lua_State* L = pRt->L;
    lua_pushstring(L, pszRegistry);
    lua_pushstring(L, pszId ? pszId : "");
    lua_pushcclosure(L, Lua_HostRegistryCall, 2);
    lua_insert(L, -nArgs - 1);
    return D2LuaHost_GuardedCall(L, "(host)", nArgs, nResults);
}

// --- accessors ------------------------------------------------------------------------------------

lua_State* D2LuaRt_State(D2LuaRuntime* pRt) {
    return pRt ? pRt->L : nullptr;
}

int D2LuaRt_Side(D2LuaRuntime* pRt) {
    return pRt->tDesc.nSide;
}

const std::vector<D2LuaModInfo>& D2LuaRt_Mods(D2LuaRuntime* pRt) {
    return pRt->tDesc.aMods;
}

bool D2LuaRt_IsFaulted(D2LuaRuntime* pRt, const char* pszMod) {
    return D2LuaHost_IsModFaulted(pRt->L, pszMod) != 0;
}

void D2LuaRt_Destroy(D2LuaRuntime* pRt) {
    if (!pRt) {
        return;
    }
    D2LuaHost_Destroy(pRt->L);
    delete pRt;
}
#endif
