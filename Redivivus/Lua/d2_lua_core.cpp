// The core's loader and the natives it calls. See d2_lua_core.h, core/native/core.d.ts.
//
// Functions Lua calls follow the host's rule: no object with a destructor in their frames.
#if D2_FEATURE_MODS
#include "Redivivus/Lua/d2_lua_core.h"
#include "Redivivus/Lua/d2_lua_ui.h"

#include <stdio.h>
#include <string.h>

#include <chrono>

#include "Redivivus/Lua/d2_lua_internal.h"
#include "lua.hpp"

static char s_CoreEnvKey;    // the core's environment
static char s_CacheKey;      // module name -> exports, the core's own instances
static char s_InstancesKey;  // mod id -> { module name -> exports }, the per-mod instances
static char s_ModEnvsKey;    // mod id -> the environment its public module instances run in
static char s_HostKey;       // internal.host's exports
static char s_LoadingKey;    // the sentinel of a module being loaded

static const D2LuaCoreChunk* FindChunk(const char* pszName) {
    for (const D2LuaCoreChunk* p = g_aD2LuaCoreChunks; p->pszName; p++) {
        if (strcmp(p->pszName, pszName) == 0) {
            return p;
        }
    }
    return nullptr;
}

const D2LuaCoreModule* D2LuaCore_FindModule(const char* pszEntry) {
    for (const D2LuaCoreModule* p = g_aD2LuaCoreModules; p->pszEntry; p++) {
        if (strcmp(p->pszEntry, pszEntry) == 0) {
            return p;
        }
    }
    return nullptr;
}

const D2LuaCoreChunk* D2LuaCore_FindBuiltinFile(const char* pszPath) {
    for (const D2LuaCoreChunk* p = g_aD2LuaBuiltinFiles; p->pszName; p++) {
        const char* a = p->pszName;
        const char* b = pszPath;
        while (*a && *b && (*a == *b || ((*a | 0x20) == (*b | 0x20) && ((*a | 0x20) >= 'a' && (*a | 0x20) <= 'z')))) {
            a++;
            b++;
        }
        if (!*a && !*b) {
            return p;
        }
    }
    return nullptr;
}

static void PushRegistryTable(lua_State* L, void* pKey) {
    lua_pushlightuserdata(L, pKey);
    lua_rawget(L, LUA_REGISTRYINDEX);
}

// Load chunk pszName as a function whose environment is the table at nEnv (absolute).
static void LoadChunk(lua_State* L, const char* pszName, int nEnv, const char* pszChunkName) {
    const D2LuaCoreChunk* pChunk = FindChunk(pszName);
    if (!pChunk) {
        luaL_error(L, "core: no module '%s'", pszName);
    }
    if (luaL_loadbufferx(L, pChunk->pszSource, pChunk->nSize, pszChunkName, "t") != 0) {
        lua_error(L);
    }
    lua_pushvalue(L, nEnv);
    lua_setfenv(L, -2);
}

// The public modules and the UI kit they serve are made once per mod that requires them: a mod's
// widgets and signals are never another mod's.
static bool IsPerMod(const char* pszName) {
    return strcmp(pszName, "internal.context") == 0 || strncmp(pszName, "redivivus.", 10) == 0 || strncmp(pszName, "ui.", 3) == 0;
}

// The context table of a mod (what internal.context reads as __context).
static void PushContext(lua_State* L, D2LuaMod* pMod) {
    lua_createtable(L, 0, 3);
    lua_pushinteger(L, pMod->nHostId);
    lua_setfield(L, -2, "id");
    lua_pushstring(L, pMod->sName.c_str());
    lua_setfield(L, -2, "name");
    lua_pushinteger(L, pMod->nIndex);
    lua_setfield(L, -2, "index");
}

// The environment a mod's public module instances run in: __context, the rest from the core's.
static void PushModCoreEnv(lua_State* L, D2LuaMod* pMod) {
    PushRegistryTable(L, &s_ModEnvsKey);
    lua_rawgeti(L, -1, pMod->nHostId);
    if (lua_istable(L, -1)) {
        lua_remove(L, -2);
        return;
    }
    lua_pop(L, 1);
    lua_createtable(L, 0, 1);
    PushContext(L, pMod);
    lua_setfield(L, -2, "__context");
    lua_createtable(L, 0, 1);
    PushRegistryTable(L, &s_CoreEnvKey);
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);
    lua_pushvalue(L, -1);
    lua_rawseti(L, -3, pMod->nHostId);
    lua_remove(L, -2);
}

// require inside the core: the core's modules, by the names TypeScriptToLua's requires use.
static int Lua_CoreRequire(lua_State* L) {
    const char* pszName = luaL_checkstring(L, 1);
    lua_settop(L, 1);
    D2LuaRuntime* pRt = D2LuaRt_FromState(L);
    bool bPerMod = IsPerMod(pszName);
    D2LuaMod* pMod = nullptr;
    if (bPerMod) {
        if (pRt->aInstantiating.empty()) {
            return luaL_error(L, "core: %s is made per mod and no mod is requiring it", pszName);
        }
        pMod = D2LuaRt_ModByHostId(pRt, pRt->aInstantiating.back());
        PushRegistryTable(L, &s_InstancesKey);  // 2
        lua_rawgeti(L, 2, pMod->nHostId);
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushvalue(L, -1);
            lua_rawseti(L, 2, pMod->nHostId);
        }
        lua_replace(L, 2);  // 2: this mod's instances
    } else {
        PushRegistryTable(L, &s_CacheKey);  // 2
    }
    lua_pushvalue(L, 1);
    lua_rawget(L, 2);
    if (lua_touserdata(L, -1) == &s_LoadingKey) {
        return luaL_error(L, "core: module '%s' requires itself", pszName);
    }
    if (!lua_isnil(L, -1)) {
        return 1;
    }
    lua_pop(L, 1);
    if (bPerMod) {
        PushModCoreEnv(L, pMod);
    } else {
        PushRegistryTable(L, &s_CoreEnvKey);
    }
    int nEnv = lua_gettop(L);
    lua_pushfstring(L, D2LUA_CORE_CHUNK_PREFIX " %s", pszName);
    LoadChunk(L, pszName, nEnv, lua_tostring(L, -1));
    lua_pushvalue(L, 1);
    lua_pushlightuserdata(L, &s_LoadingKey);
    lua_rawset(L, 2);
    lua_pushvalue(L, 1);
    lua_call(L, 1, 1);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_pushboolean(L, 1);
    }
    lua_pushvalue(L, 1);
    lua_pushvalue(L, -2);
    lua_rawset(L, 2);
    return 1;
}

void D2LuaCore_Start(lua_State* L, D2LuaRuntime* pRt) {
    int nTop = lua_gettop(L);
    // The core's environment: the stock functions and libraries of the global table (which no mod
    // reaches), the natives, and the core's require.
    lua_newtable(L);
    int nEnv = lua_gettop(L);
    lua_pushnil(L);
    while (lua_next(L, LUA_GLOBALSINDEX)) {
        lua_pushvalue(L, -2);
        lua_insert(L, -2);
        lua_rawset(L, nEnv);
    }
    lua_pushvalue(L, nEnv);
    lua_setfield(L, nEnv, "_G");
    lua_pushcfunction(L, Lua_CoreRequire);
    lua_setfield(L, nEnv, "require");
    lua_newtable(L);
    int nNatives = lua_gettop(L);
    for (const D2LuaApiDesc* pApi : pRt->tDesc.aApis) {
        if (pApi->pfnNatives) {
            pApi->pfnNatives(L, nNatives, pRt->tDesc.nSide);
        }
    }
    D2LuaUi_Natives(L, nNatives, pRt);
    lua_setfield(L, nEnv, "__native");
    lua_pushlightuserdata(L, &s_CoreEnvKey);
    lua_pushvalue(L, nEnv);
    lua_rawset(L, LUA_REGISTRYINDEX);
    void* const apKeys[] = { &s_CacheKey, &s_InstancesKey, &s_ModEnvsKey };
    for (void* pKey : apKeys) {
        lua_pushlightuserdata(L, pKey);
        lua_newtable(L);
        lua_rawset(L, LUA_REGISTRYINDEX);
    }
    lua_pushcfunction(L, Lua_CoreRequire);
    lua_pushliteral(L, "internal.host");
    lua_call(L, 1, 1);
    lua_pushlightuserdata(L, &s_HostKey);
    lua_insert(L, -2);
    lua_rawset(L, LUA_REGISTRYINDEX);
    lua_settop(L, nTop);
}

void D2LuaCore_ForgetInstances(lua_State* L) {
    lua_pushlightuserdata(L, &s_InstancesKey);
    lua_newtable(L);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

// Upvalue 1: the chunk name. Requires it under the instantiation the caller set up.
static int Lua_Instantiate(lua_State* L) {
    lua_pushcfunction(L, Lua_CoreRequire);
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_call(L, 1, 1);
    return 1;
}

void D2LuaCore_PushModule(lua_State* L, D2LuaRuntime* pRt, D2LuaMod* pMod, const D2LuaCoreModule* pModule) {
    if (strcmp(pModule->pszChunk, "lualib_bundle") == 0) {
        // TypeScriptToLua's runtime for a mod: an instance in the mod's own environment, under a
        // name that is not the core's (it calls the mod's functions and loops for it).
        D2LuaRt_PushEnv(L, pMod);
        int nEnv = lua_gettop(L);
        LoadChunk(L, "lualib_bundle", nEnv, "@redivivus:lualib");
        lua_call(L, 0, 1);
        lua_remove(L, nEnv);
        return;
    }
    pRt->aInstantiating.push_back(pMod->nHostId);
    lua_pushstring(L, pModule->pszChunk);
    lua_pushcclosure(L, Lua_Instantiate, 1);
    int nStatus = lua_pcall(L, 0, 1, 0);
    pRt->aInstantiating.pop_back();
    if (nStatus != 0) {
        lua_error(L);
    }
    int nExports = lua_gettop(L);
    D2LuaCore_PushHost(L, "finishModule");
    lua_pushstring(L, pModule->pszEntry);
    lua_pushvalue(L, nExports);
    lua_pushinteger(L, pMod->nHostId);
    lua_call(L, 3, 1);
    lua_remove(L, nExports);
}

void D2LuaCore_PushHost(lua_State* L, const char* pszName) {
    PushRegistryTable(L, &s_HostKey);
    lua_getfield(L, -1, pszName);
    lua_remove(L, -2);
}

static int Lua_CoreTraceback(lua_State* L) {
    const char* pszMessage = lua_type(L, 1) == LUA_TSTRING ? lua_tostring(L, 1) : nullptr;
    if (!pszMessage) {
        pszMessage = lua_pushfstring(L, "(error object is a %s value)", luaL_typename(L, 1));
    }
    luaL_traceback(L, L, pszMessage, 1);
    return 1;
}

bool D2LuaCore_CallHost(lua_State* L, const char* pszName, int nArgs, int nResults) {
    int nBase = lua_gettop(L) - nArgs;
    D2LuaCore_PushHost(L, pszName);
    lua_insert(L, nBase + 1);
    if (D2LuaHost_InEntry(L)) {
        lua_call(L, nArgs, nResults);
        return true;
    }
    lua_pushcfunction(L, Lua_CoreTraceback);
    lua_insert(L, nBase + 1);
    if (lua_pcall(L, nArgs, nResults, nBase + 1) != 0) {
        lua_pushfstring(L, "[lua] core (%s): %s", pszName, lua_tostring(L, -1));
        D2LuaHost_Log(L, lua_tostring(L, -1));
        lua_settop(L, nBase);
        return false;
    }
    lua_remove(L, nBase + 1);
    return true;
}

void D2LuaCore_PushInternals(lua_State* L) {
    lua_createtable(L, 0, 6);
    static const struct { const char* pszName; void* pKey; } s_aKeys[] = {
        { "env", &s_CoreEnvKey }, { "cache", &s_CacheKey }, { "instances", &s_InstancesKey }, { "modEnvs", &s_ModEnvsKey }, { "host", &s_HostKey },
    };
    for (const auto& t : s_aKeys) {
        PushRegistryTable(L, t.pKey);
        lua_setfield(L, -2, t.pszName);
    }
    lua_getfield(L, -1, "env");
    lua_getfield(L, -1, "__native");
    lua_setfield(L, -3, "natives");
    lua_pop(L, 1);
}

// --- the natives ------------------------------------------------------------------------------------
// Called by the core only; each still checks its arguments' types.

static D2LuaMod* ArgMod(lua_State* L, int i, const char* pszWhere) {
    if (lua_type(L, i) != LUA_TNUMBER) {
        luaL_error(L, "%s: a mod id expected", pszWhere);
    }
    D2LuaMod* pMod = D2LuaRt_ModByHostId(D2LuaRt_FromState(L), (int)lua_tointeger(L, i));
    if (!pMod) {
        luaL_error(L, "%s: no mod %d in this state", pszWhere, (int)lua_tointeger(L, i));
    }
    return pMod;
}

int D2LuaCore_Call(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    if (lua_type(L, 1) != LUA_TNUMBER) {
        return luaL_error(L, "call: a mod id expected");
    }
    int nModId = (int)lua_tointeger(L, 1);
    if (nModId >= 0) {
        ArgMod(L, 1, "call");
    }
    if (!lua_isfunction(L, 2)) {
        return luaL_error(L, "call: a function expected, got %s", luaL_typename(L, 2));
    }
    lua_remove(L, 1);
    int nArgs = lua_gettop(L) - 1;
    int nPushed = 0;
    int nStatus = D2LuaHost_CallAs(L, nModId, nArgs, &nPushed);
    lua_pushinteger(L, nStatus);
    lua_insert(L, -(nPushed + 1));
    return nPushed + 1;
}

int D2LuaCore_Resume(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    lua_State* co = lua_tothread(L, 1);
    if (!co || co == L) {
        return luaL_error(L, "resume: a coroutine expected");
    }
    D2LuaMod* pMod = ArgMod(L, 2, "resume");
    int nArgs = lua_gettop(L) - 2;
    lua_checkstack(co, nArgs + 4);
    lua_xmove(L, co, nArgs);
    int nResults = 0;
    int nStatus = D2LuaHost_ResumeAs(L, co, pMod->nHostId, nArgs, &nResults);
    if (nStatus == D2LUA_CALL_OK || nStatus == D2LUA_CALL_YIELD) {
        luaL_checkstack(L, nResults + 2, "resume");
        lua_pushinteger(L, nStatus);
        lua_xmove(co, L, nResults);
        return nResults + 1;
    }
    lua_pushinteger(L, nStatus);
    lua_insert(L, -(nResults + 1));
    return nResults + 1;
}

int D2LuaCore_Spawn(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    D2LuaMod* pMod = ArgMod(L, 1, "spawn");
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_State* co = lua_newthread(L);
    lua_pushvalue(L, 2);
    lua_xmove(L, co, 1);
    D2LuaHost_SetCoroutineOwner(L, -1, pMod->nHostId);
    return 1;
}

int D2LuaCore_Log(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    D2LuaMod* pMod = ArgMod(L, 1, "log");
    int nArgs = lua_gettop(L);
    luaL_Buffer tBuffer;
    lua_getglobal(L, "tostring");
    int nToString = lua_gettop(L);
    luaL_buffinit(L, &tBuffer);
    for (int i = 2; i <= nArgs; i++) {
        lua_pushvalue(L, nToString);
        lua_pushvalue(L, i);
        lua_call(L, 1, 1);
        if (!lua_isstring(L, -1)) {
            return luaL_error(L, "'tostring' must return a string to 'log'");
        }
        if (i > 2) {
            luaL_addchar(&tBuffer, ' ');
        }
        luaL_addvalue(&tBuffer);
    }
    luaL_pushresult(&tBuffer);
    lua_pushfstring(L, "[lua] [%s] %s", pMod->sName.c_str(), lua_tostring(L, -1));
    D2LuaHost_Log(L, lua_tostring(L, -1));
    return 0;
}

int D2LuaCore_LogError(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    D2LuaMod* pMod = ArgMod(L, 1, "logError");
    if (lua_type(L, 3) == LUA_TSTRING) {
        lua_pushvalue(L, 3);
    } else if (lua_type(L, 2) == LUA_TSTRING) {
        lua_pushvalue(L, 2);
    } else {
        lua_pushfstring(L, "(error object is a %s value)", luaL_typename(L, 2));
    }
    D2LuaHost_LogErrorTop(L, pMod->sName.c_str());
    return 0;
}

int D2LuaCore_LogLine(D2LuaCtx& ctx) {
    D2LuaHost_Log(ctx.L, luaL_checkstring(ctx.L, 1));
    return 0;
}

int D2LuaCore_IsFaulted(D2LuaCtx& ctx) {
    D2LuaMod* pMod = ArgMod(ctx.L, 1, "isFaulted");
    lua_pushboolean(ctx.L, D2LuaRt_ModFaulted(ctx.L, pMod) ? 1 : 0);
    return 1;
}

int D2LuaCore_Mods(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    D2LuaRuntime* pRt = D2LuaRt_FromState(L);
    lua_createtable(L, (int)pRt->aMods.size(), 0);
    for (size_t i = 0; i < pRt->aMods.size(); i++) {
        D2LuaMod* pMod = pRt->aMods[i].get();
        lua_createtable(L, 0, 3);
        lua_pushinteger(L, pMod->nHostId);
        lua_setfield(L, -2, "id");
        lua_pushstring(L, pMod->sName.c_str());
        lua_setfield(L, -2, "name");
        lua_pushinteger(L, pMod->nIndex);
        lua_setfield(L, -2, "index");
        lua_rawseti(L, -2, (int)i + 1);
    }
    return 1;
}

int D2LuaCore_Owner(D2LuaCtx& ctx) {
    lua_pushinteger(ctx.L, D2LuaHost_CurrentOwner(ctx.L));
    return 1;
}

int D2LuaCore_Tick(D2LuaCtx& ctx) {
    D2LuaRuntime* pRt = D2LuaRt_FromState(ctx.L);
    auto it = pRt->tTicks.find(pRt->nGame);
    lua_pushnumber(ctx.L, it == pRt->tTicks.end() ? 0 : (lua_Number)it->second);
    return 1;
}

int D2LuaCore_Side(D2LuaCtx& ctx) {
    lua_pushinteger(ctx.L, D2LuaRt_FromState(ctx.L)->tDesc.nSide);
    return 1;
}

// A copy of a runtime table, nDepth levels deep.
static void PushCopy(lua_State* L, int nIndex, int nDepth) {
    nIndex = nIndex < 0 ? lua_gettop(L) + nIndex + 1 : nIndex;
    lua_newtable(L);
    lua_pushnil(L);
    while (lua_next(L, nIndex)) {
        lua_pushvalue(L, -2);
        if (nDepth > 1 && lua_istable(L, -2)) {
            PushCopy(L, -2, nDepth - 1);
        } else {
            lua_pushvalue(L, -2);
        }
        lua_rawset(L, -5);
        lua_pop(L, 1);
    }
}

static int PushFieldCopy(lua_State* L, const char* pszField, int nDepth) {
    D2LuaRt_PushField(L, pszField);
    PushCopy(L, -1, nDepth);
    lua_remove(L, -2);
    return 1;
}

int D2LuaCore_Events(D2LuaCtx& ctx) {
    return PushFieldCopy(ctx.L, "events", 1);
}

int D2LuaCore_Registries(D2LuaCtx& ctx) {
    return PushFieldCopy(ctx.L, "registries", 3);
}

int D2LuaCore_Enums(D2LuaCtx& ctx) {
    return PushFieldCopy(ctx.L, "enums", 2);
}

int D2LuaCore_HandleIsLive(D2LuaCtx& ctx) {
    lua_pushboolean(ctx.L, D2LuaHandle_IsLive(ctx.L, 1) ? 1 : 0);
    return 1;
}

int D2LuaCore_IsHandle(D2LuaCtx& ctx) {
    const char* pszClass;
    uint64_t nId;
    lua_pushboolean(ctx.L, D2LuaHandle_Inspect(ctx.L, 1, &pszClass, &nId) ? 1 : 0);
    return 1;
}

int D2LuaSettings_Declare(lua_State* L, D2LuaMod* pMod, int nName);
int D2LuaSettings_Get(lua_State* L, D2LuaMod* pMod, int nName);
int D2LuaSettings_List(lua_State* L);

int D2LuaCore_SettingsDeclare(D2LuaCtx& ctx) {
    return D2LuaSettings_Declare(ctx.L, ArgMod(ctx.L, 1, "settingsDeclare"), 2);
}

int D2LuaCore_SettingsGet(D2LuaCtx& ctx) {
    return D2LuaSettings_Get(ctx.L, ArgMod(ctx.L, 1, "settingsGet"), 2);
}

int D2LuaCore_SettingsList(D2LuaCtx& ctx) {
    return D2LuaSettings_List(ctx.L);
}

int D2LuaMessages_Send(lua_State* L, uint32_t nGame, D2LuaMod* pMod, int nPayload, int nPeer);

int D2LuaCore_Send(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    lua_Number nGame = luaL_checknumber(L, 1);
    D2LuaMod* pMod = ArgMod(L, 2, "send");
    return D2LuaMessages_Send(L, (uint32_t)nGame, pMod, 3, 4);
}

int D2LuaCore_WalkKey(D2LuaCtx& ctx) {
    lua_pushlightuserdata(ctx.L, D2LuaRt_WalkKey());
    return 1;
}

int D2LuaCore_Clock(D2LuaCtx& ctx) {
    using namespace std::chrono;
    lua_pushnumber(ctx.L, (lua_Number)duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count() / 1000.0);
    return 1;
}

int D2LuaCore_Require(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    D2LuaMod* pMod = ArgMod(L, 1, "require");
    const char* pszName = luaL_checkstring(L, 2);
    bool bOptional = lua_toboolean(L, 3) != 0;
    D2LuaRt_Require(L, pMod, pszName, bOptional);
    return 1;
}
#endif
