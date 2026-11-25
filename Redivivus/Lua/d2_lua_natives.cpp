// The natives behind the declarations in Redivivus/Lua/api (script, remote, registries, storage,
// log, data) and the argument checks generated glue uses. The calling convention is in
// d2_lua_api.h; the glue that calls these is Redivivus/Lua/generated/d2_lua_api_main.gen.cpp.
//
// Registries. An entry is a chain: an implementation at the bottom (a native, or what a mod
// registered last) and wrappers above it, ordered by the wrapping mod's load order (then by the
// order one mod added them), so later mods wrap earlier ones whatever the moment they wrap.
// A call runs the top wrapper with (next, ...args); next runs the level below. Each level runs
// as its owning mod (the owner stack). When a wrapper raises an error of its own, it is removed
// from the chain, logged, and the call continues with the level below and the original
// arguments. An error that comes up out of next (a level below failed) passes through the
// wrappers above unchanged and is not held against them. When the time budget or the memory
// limit runs out inside a wrapper, nothing falls through: the entry ends and the wrapper's mod is
// faulted, which also takes its wrappers out of every chain.
#if D2_FEATURE_MODS
#include <stdio.h>
#include <string.h>

#include "Redivivus/Lua/d2_lua_internal.h"
#include "Redivivus/Lua/d2_lua_rt.h"
#include "lua.hpp"

static char s_ChainErrorKey;  // metatable of the error a failed next() raises through wrappers

// --- context ---------------------------------------------------------------------------------------

D2LuaCtx D2LuaCtx_FromUpvalue(lua_State* L) {
    D2LuaCtx tCtx;
    tCtx.L = L;
    tCtx.pMod = (D2LuaMod*)lua_touserdata(L, lua_upvalueindex(1));
    tCtx.pRuntime = tCtx.pMod ? tCtx.pMod->pRt : D2LuaRt_FromState(L);
    tCtx.nSide = tCtx.pRuntime ? tCtx.pRuntime->tDesc.nSide : 0;
    return tCtx;
}

D2LuaCtx D2LuaCtx_FromOwner(lua_State* L) {
    D2LuaCtx tCtx;
    tCtx.L = L;
    tCtx.pRuntime = D2LuaRt_FromState(L);
    tCtx.pMod = D2LuaRt_ModByHostId(tCtx.pRuntime, D2LuaHost_CurrentOwner(L));
    tCtx.nSide = tCtx.pRuntime ? tCtx.pRuntime->tDesc.nSide : 0;
    return tCtx;
}

D2LuaCtx D2LuaCtx_ForMod(lua_State* L, D2LuaMod* pMod) {
    D2LuaCtx tCtx;
    tCtx.L = L;
    tCtx.pMod = pMod;
    tCtx.pRuntime = pMod ? pMod->pRt : D2LuaRt_FromState(L);
    tCtx.nSide = tCtx.pRuntime ? tCtx.pRuntime->tDesc.nSide : 0;
    return tCtx;
}

const char* D2LuaCtx_ModName(const D2LuaCtx& ctx) {
    return ctx.pMod ? ctx.pMod->sName.c_str() : "(host)";
}

// --- singletons -----------------------------------------------------------------------------------

// __index: upvalues 1 getters (closures over the mod), 2 the global's name.
static int Lua_SingletonIndex(lua_State* L) {
    lua_pushvalue(L, 2);
    lua_rawget(L, lua_upvalueindex(1));
    if (lua_isnil(L, -1)) {
        return luaL_error(L, "%s has no member '%s'", lua_tostring(L, lua_upvalueindex(2)),
                          lua_type(L, 2) == LUA_TSTRING ? lua_tostring(L, 2) : luaL_typename(L, 2));
    }
    lua_call(L, 0, 1);
    return 1;
}

// __newindex: upvalues 1 setters, 2 getters, 3 the global's name.
static int Lua_SingletonNewIndex(lua_State* L) {
    lua_pushvalue(L, 2);
    lua_rawget(L, lua_upvalueindex(1));
    if (lua_isnil(L, -1)) {
        lua_pushvalue(L, 2);
        lua_rawget(L, lua_upvalueindex(2));
        const char* pszKey = lua_type(L, 2) == LUA_TSTRING ? lua_tostring(L, 2) : luaL_typename(L, 2);
        if (!lua_isnil(L, -1)) {
            return luaL_error(L, "%s.%s is read-only", lua_tostring(L, lua_upvalueindex(3)), pszKey);
        }
        return luaL_error(L, "%s has no member '%s'", lua_tostring(L, lua_upvalueindex(3)), pszKey);
    }
    lua_pushvalue(L, 3);
    lua_call(L, 1, 0);
    return 0;
}

static void PushModClosures(lua_State* L, D2LuaMod* pMod, const luaL_Reg* aReg) {
    lua_newtable(L);
    for (const luaL_Reg* p = aReg; p && p->name; p++) {
        lua_pushlightuserdata(L, pMod);
        lua_pushcclosure(L, p->func, 1);
        lua_setfield(L, -2, p->name);
    }
}

void D2LuaApi_PushSingleton(lua_State* L, D2LuaMod* pMod, const char* pszName, const luaL_Reg* aMethods,
                            const luaL_Reg* aGetters, const luaL_Reg* aSetters) {
    PushModClosures(L, pMod, aMethods);
    int nObject = lua_gettop(L);
    lua_newtable(L);
    PushModClosures(L, pMod, aGetters);
    int nGetters = lua_gettop(L);
    lua_pushvalue(L, nGetters);
    lua_pushstring(L, pszName);
    lua_pushcclosure(L, Lua_SingletonIndex, 2);
    lua_setfield(L, nGetters - 1, "__index");
    PushModClosures(L, pMod, aSetters);
    lua_pushvalue(L, nGetters);
    lua_pushstring(L, pszName);
    lua_pushcclosure(L, Lua_SingletonNewIndex, 3);
    lua_setfield(L, nGetters - 1, "__newindex");
    lua_settop(L, nGetters - 1);
    lua_pushboolean(L, 0);
    lua_setfield(L, -2, "__metatable");
    lua_setmetatable(L, nObject);
}

// --- argument checks -----------------------------------------------------------------------------

static int ArgError(lua_State* L, int i, const char* pszWhere, const char* pszExpected) {
    return luaL_error(L, "bad argument #%d to '%s' (%s expected, got %s)", i, pszWhere, pszExpected,
                      lua_isnone(L, i) ? "no value" : luaL_typename(L, i));
}

const char* D2LuaArg_String(lua_State* L, int i, const char* pszWhere, bool bOptional) {
    if (bOptional && lua_isnoneornil(L, i)) {
        return nullptr;
    }
    if (lua_type(L, i) != LUA_TSTRING) {
        ArgError(L, i, pszWhere, "string");
    }
    return lua_tostring(L, i);
}

double D2LuaArg_Number(lua_State* L, int i, const char* pszWhere) {
    if (lua_type(L, i) != LUA_TNUMBER) {
        ArgError(L, i, pszWhere, "number");
    }
    return lua_tonumber(L, i);
}

D2LuaOpt<double> D2LuaArg_OptNumber(lua_State* L, int i, const char* pszWhere) {
    D2LuaOpt<double> t = { false, 0 };
    if (!lua_isnoneornil(L, i)) {
        t.bHas = true;
        t.tValue = D2LuaArg_Number(L, i, pszWhere);
    }
    return t;
}

lua_Integer D2LuaArg_Integer(lua_State* L, int i, const char* pszWhere) {
    double d = D2LuaArg_Number(L, i, pszWhere);
    lua_Integer n = (lua_Integer)d;
    if ((double)n != d) {
        luaL_error(L, "bad argument #%d to '%s' (integer expected, got %f)", i, pszWhere, d);
    }
    return n;
}

D2LuaOpt<lua_Integer> D2LuaArg_OptInteger(lua_State* L, int i, const char* pszWhere) {
    D2LuaOpt<lua_Integer> t = { false, 0 };
    if (!lua_isnoneornil(L, i)) {
        t.bHas = true;
        t.tValue = D2LuaArg_Integer(L, i, pszWhere);
    }
    return t;
}

bool D2LuaArg_Boolean(lua_State* L, int i, const char* pszWhere) {
    if (lua_type(L, i) != LUA_TBOOLEAN) {
        ArgError(L, i, pszWhere, "boolean");
    }
    return lua_toboolean(L, i) != 0;
}

D2LuaOpt<bool> D2LuaArg_OptBoolean(lua_State* L, int i, const char* pszWhere) {
    D2LuaOpt<bool> t = { false, false };
    if (!lua_isnoneornil(L, i)) {
        t.bHas = true;
        t.tValue = D2LuaArg_Boolean(L, i, pszWhere);
    }
    return t;
}

D2LuaFn D2LuaArg_Function(lua_State* L, int i, const char* pszWhere, bool bOptional) {
    D2LuaFn t = { 0 };
    if (bOptional && lua_isnoneornil(L, i)) {
        return t;
    }
    if (!lua_isfunction(L, i)) {
        ArgError(L, i, pszWhere, "function");
    }
    t.nIndex = i;
    return t;
}

D2LuaTable D2LuaArg_Table(lua_State* L, int i, const char* pszWhere, bool bOptional) {
    D2LuaTable t = { 0 };
    if (bOptional && lua_isnoneornil(L, i)) {
        return t;
    }
    if (!lua_istable(L, i)) {
        ArgError(L, i, pszWhere, "table");
    }
    t.nIndex = i;
    return t;
}

D2LuaAny D2LuaArg_Any(lua_State* L, int i) {
    D2LuaAny t = { lua_isnone(L, i) ? 0 : i };
    return t;
}

const char* D2LuaArg_OneOf(lua_State* L, int i, const char* pszWhere, const char* const* apszAllowed, bool bOptional) {
    const char* psz = D2LuaArg_String(L, i, pszWhere, bOptional);
    if (!psz) {
        return nullptr;
    }
    for (const char* const* pp = apszAllowed; *pp; pp++) {
        if (strcmp(*pp, psz) == 0) {
            return psz;
        }
    }
    luaL_error(L, "bad argument #%d to '%s' ('%s' is not one of the allowed values)", i, pszWhere, psz);
    return nullptr;
}

const char* D2LuaArg_EventName(lua_State* L, int i, const char* pszWhere, const char* pszEventMap, bool bOptional) {
    const char* psz = D2LuaArg_String(L, i, pszWhere, bOptional);
    if (!psz) {
        return nullptr;
    }
    D2LuaRt_PushField(L, "events");
    lua_getfield(L, -1, psz);
    bool bKnown = lua_type(L, -1) == LUA_TSTRING && strcmp(lua_tostring(L, -1), pszEventMap) == 0;
    lua_pop(L, 2);
    if (!bKnown) {
        luaL_error(L, "bad argument #%d to '%s' (no event '%s' on this side)", i, pszWhere, psz);
    }
    return psz;
}

lua_Integer D2LuaArg_Enum(lua_State* L, int i, const char* pszWhere, const char* pszEnum, const lua_Integer* anValues,
                          int nCount) {
    lua_Integer n = D2LuaArg_Integer(L, i, pszWhere);
    for (int k = 0; k < nCount; k++) {
        if (anValues[k] == n) {
            return n;
        }
    }
    luaL_error(L, "bad argument #%d to '%s' (%d is not a %s)", i, pszWhere, (int)n, pszEnum);
    return 0;
}

// --- events and registries the declarations define ---------------------------------------------

void D2LuaApi_DefineEvents(lua_State* L, const char* pszEventMap, int nSides, const char* const* apszNames) {
    D2LuaRuntime* pRt = D2LuaRt_FromState(L);
    if (!pRt || !(pRt->tDesc.nSide & nSides)) {
        return;
    }
    D2LuaRt_PushField(L, "events");
    for (const char* const* pp = apszNames; *pp; pp++) {
        lua_pushstring(L, pszEventMap);
        lua_setfield(L, -2, *pp);
    }
    lua_pop(L, 1);
}

// --- enums ----------------------------------------------------------------------------------------
// enums[name] = { values = { [member] = value }, mt = proxy metatable }. A mod sees a proxy of its
// own: an empty table whose metatable reads from values and refuses writes, locked with
// __metatable. Only that mod's proxy is reachable from its environment, so a rawset into it
// changes nothing for anyone else.

static char s_EnumValuesKey;  // in a proxy metatable: the values table, for pairs

static int Lua_EnumNewIndex(lua_State* L) {
    return luaL_error(L, "%s is read-only (an enum)", lua_tostring(L, lua_upvalueindex(1)));
}

void D2LuaApi_DefineEnum(lua_State* L, const char* pszEnum, const char* const* apszNames, const lua_Number* anNumbers,
                         const char* const* apszStrings) {
    D2LuaRt_PushField(L, "enums");
    lua_getfield(L, -1, pszEnum);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);  // the record
        lua_newtable(L);
        lua_setfield(L, -2, "values");
        lua_newtable(L);  // the proxy metatable
        lua_getfield(L, -2, "values");
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, "__index");
        lua_pushlightuserdata(L, &s_EnumValuesKey);
        lua_insert(L, -2);
        lua_rawset(L, -3);
        lua_pushstring(L, pszEnum);
        lua_pushcclosure(L, Lua_EnumNewIndex, 1);
        lua_setfield(L, -2, "__newindex");
        lua_pushboolean(L, 0);
        lua_setfield(L, -2, "__metatable");
        lua_setfield(L, -2, "mt");
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, pszEnum);
    }
    lua_getfield(L, -1, "values");
    for (int i = 0; apszNames[i]; i++) {
        if (anNumbers) {
            lua_pushnumber(L, anNumbers[i]);
        } else {
            lua_pushstring(L, apszStrings[i]);
        }
        lua_setfield(L, -2, apszNames[i]);
    }
    lua_pop(L, 3);
}

void D2LuaApi_InstallEnum(lua_State* L, int nEnv, const char* pszEnum) {
    lua_getfield(L, nEnv, pszEnum);
    bool bPresent = !lua_isnil(L, -1);
    lua_pop(L, 1);
    if (bPresent) {
        return;  // a second declaration set merged its members into the same table
    }
    D2LuaRt_PushField(L, "enums");
    lua_getfield(L, -1, pszEnum);
    lua_remove(L, -2);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return;
    }
    lua_newtable(L);
    lua_getfield(L, -2, "mt");
    lua_setmetatable(L, -2);
    lua_setfield(L, nEnv, pszEnum);
    lua_pop(L, 1);
}

// pairs for mods: Lua 5.1's (next, t, nil), except that an enum proxy walks its members.
// Upvalue 1 is next.
int D2LuaRt_EnvPairs(lua_State* L) {
    luaL_checkany(L, 1);
    lua_pushvalue(L, lua_upvalueindex(1));
    if (lua_getmetatable(L, 1)) {
        lua_pushlightuserdata(L, &s_EnumValuesKey);
        lua_rawget(L, -2);
        lua_remove(L, -2);
        if (lua_istable(L, -1)) {
            lua_pushnil(L);
            return 3;
        }
        lua_pop(L, 1);
    }
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    return 3;
}

static void PushRegistry(lua_State* L, const char* pszRegistry) {
    D2LuaRt_PushField(L, "registries");
    lua_getfield(L, -1, pszRegistry);
    lua_remove(L, -2);
}

void D2LuaApi_DefineRegistry(lua_State* L, const char* pszRegistry, int nKind, int nSides) {
    D2LuaRuntime* pRt = D2LuaRt_FromState(L);
    if (!pRt || !(pRt->tDesc.nSide & nSides)) {
        return;
    }
    D2LuaRt_PushField(L, "registries");
    lua_newtable(L);
    lua_pushinteger(L, nKind);
    lua_setfield(L, -2, "kind");
    lua_newtable(L);
    lua_setfield(L, -2, "entries");
    lua_setfield(L, -2, pszRegistry);
    lua_pop(L, 1);
}

// Push entry pszId of the registry table at nRegistry, creating it when bCreate.
static bool PushEntry(lua_State* L, int nRegistry, const char* pszId, bool bCreate) {
    lua_getfield(L, nRegistry, "entries");
    lua_getfield(L, -1, pszId);
    if (lua_istable(L, -1)) {
        lua_remove(L, -2);
        return true;
    }
    lua_pop(L, 1);
    if (!bCreate) {
        lua_pop(L, 1);
        return false;
    }
    lua_newtable(L);
    lua_pushinteger(L, -1);
    lua_setfield(L, -2, "owner");
    lua_newtable(L);
    lua_setfield(L, -2, "wrappers");
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, pszId);
    lua_remove(L, -2);
    return true;
}

void D2LuaApi_RegistryNative(lua_State* L, const char* pszRegistry, const char* pszId, lua_CFunction pfn) {
    PushRegistry(L, pszRegistry);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return;
    }
    PushEntry(L, lua_gettop(L), pszId ? pszId : "", true);
    lua_pushcfunction(L, pfn);
    lua_setfield(L, -2, "impl");
    lua_pushinteger(L, -1);
    lua_setfield(L, -2, "owner");
    lua_pop(L, 2);
}

// --- the chain ------------------------------------------------------------------------------------

static bool IsChainError(lua_State* L, int i) {
    if (!lua_istable(L, i) || !lua_getmetatable(L, i)) {
        return false;
    }
    lua_pushlightuserdata(L, &s_ChainErrorKey);
    lua_rawget(L, LUA_REGISTRYINDEX);
    bool bIs = lua_rawequal(L, -1, -2) != 0;
    lua_pop(L, 2);
    return bIs;
}

static int Lua_ChainErrorToString(lua_State* L) {
    lua_rawgeti(L, 1, 1);
    lua_getglobal(L, "tostring");
    lua_insert(L, -2);
    lua_call(L, 1, 1);
    return 1;
}

// Wrap the error at the top into a chain error (unless it is one) and raise it.
static int RaiseThroughChain(lua_State* L) {
    if (IsChainError(L, -1)) {
        return lua_error(L);
    }
    lua_createtable(L, 1, 0);
    lua_insert(L, -2);
    lua_rawseti(L, -2, 1);
    lua_pushlightuserdata(L, &s_ChainErrorKey);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushcfunction(L, Lua_ChainErrorToString);
        lua_setfield(L, -2, "__tostring");
        lua_pushlightuserdata(L, &s_ChainErrorKey);
        lua_pushvalue(L, -2);
        lua_rawset(L, LUA_REGISTRYINDEX);
    }
    lua_setmetatable(L, -2);
    return lua_error(L);
}

static int RunLevel(lua_State* L, int nSnapshot, int nEntry, int nLevel, int nArgBase, const char* pszLabel);

// The body of next(...): upvalues snapshot, entry, level, label.
static int Lua_ChainNextInner(lua_State* L) {
    int nArgs = lua_gettop(L);
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushvalue(L, lua_upvalueindex(2));
    int nSnapshot = nArgs + 1;
    int nEntry = nArgs + 2;
    // Args go above the snapshot and entry.
    for (int i = 1; i <= nArgs; i++) {
        lua_pushvalue(L, i);
    }
    int nLevel = (int)lua_tointeger(L, lua_upvalueindex(3));
    int nResults = RunLevel(L, nSnapshot, nEntry, nLevel, nEntry + 1, lua_tostring(L, lua_upvalueindex(4)));
    return nResults;
}

// next(...) for a wrapper. An error from the levels below comes out as a chain error, so the
// wrappers it passes through are not blamed for it.
static int Lua_ChainNext(lua_State* L) {
    for (int i = 1; i <= 4; i++) {
        lua_pushvalue(L, lua_upvalueindex(i));
    }
    lua_pushcclosure(L, Lua_ChainNextInner, 4);
    lua_insert(L, 1);
    int nStatus = lua_pcall(L, lua_gettop(L) - 1, LUA_MULTRET, 0);
    if (nStatus != 0) {
        if (D2LuaHost_DeadlinePassed(L) || D2LuaHost_MemoryRefused(L) || nStatus == LUA_ERRMEM) {
            return lua_error(L);
        }
        return RaiseThroughChain(L);
    }
    return lua_gettop(L);
}
// Run level nLevel of the chain (0 is the implementation, 1.. the snapshot's wrappers from the
// bottom) with the values from nArgBase to the top as arguments. Returns the results' count.
static int RunLevel(lua_State* L, int nSnapshot, int nEntry, int nLevel, int nArgBase, const char* pszLabel) {
    int nArgs = lua_gettop(L) - nArgBase + 1;
    luaL_checkstack(L, nArgs + 8, "registry call");
    int nOwnerDepth = D2LuaHost_OwnerDepth(L);
    if (nLevel == 0) {
        lua_getfield(L, nEntry, "impl");
        if (lua_isnil(L, -1)) {
            return luaL_error(L, "%s has no implementation", pszLabel);
        }
        lua_getfield(L, nEntry, "owner");
        int nOwner = (int)lua_tointeger(L, -1);
        lua_pop(L, 1);
        int nFunction = lua_gettop(L);
        for (int i = 0; i < nArgs; i++) {
            lua_pushvalue(L, nArgBase + i);
        }
        if (nOwner >= 0) {
            D2LuaHost_PushOwner(L, nOwner);
        }
        int nStatus = lua_pcall(L, nArgs, LUA_MULTRET, 0);
        D2LuaHost_RestoreOwnerDepth(L, nOwnerDepth);
        if (nStatus != 0) {
            return lua_error(L);
        }
        return lua_gettop(L) - nFunction + 1;
    }
    lua_rawgeti(L, nSnapshot, nLevel);
    int nWrapper = lua_gettop(L);
    lua_getfield(L, nWrapper, "mod");
    int nOwner = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, nWrapper, "removed");
    bool bRemoved = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);
    const char* pszOwner = D2LuaHost_ModName(L, nOwner);
    if (bRemoved || (pszOwner && D2LuaHost_IsModFaulted(L, pszOwner))) {
        lua_settop(L, nWrapper - 1);
        return RunLevel(L, nSnapshot, nEntry, nLevel - 1, nArgBase, pszLabel);
    }
    int nFunction = nWrapper + 1;
    lua_getfield(L, nWrapper, "fn");
    lua_pushvalue(L, nSnapshot);
    lua_pushvalue(L, nEntry);
    lua_pushinteger(L, nLevel - 1);
    lua_pushstring(L, pszLabel);
    lua_pushcclosure(L, Lua_ChainNext, 4);
    for (int i = 0; i < nArgs; i++) {
        lua_pushvalue(L, nArgBase + i);
    }
    D2LuaHost_PushOwner(L, nOwner);
    int nStatus = lua_pcall(L, nArgs + 1, LUA_MULTRET, 0);
    D2LuaHost_RestoreOwnerDepth(L, nOwnerDepth);
    if (nStatus == 0) {
        return lua_gettop(L) - nFunction + 1;
    }
    if (IsChainError(L, -1) || D2LuaHost_DeadlinePassed(L) || D2LuaHost_MemoryRefused(L) || nStatus == LUA_ERRMEM) {
        return lua_error(L);
    }
    // The wrapper's own error: log it, take it out of the chain, fall through.
    char szPrefix[200];
    snprintf(szPrefix, sizeof(szPrefix), "[lua] %s: a wrapper on %s failed and is removed from the chain: ",
             pszOwner ? pszOwner : "?", pszLabel);
    const char* pszMessage = lua_tostring(L, -1);
    lua_pushfstring(L, "%s%s", szPrefix, pszMessage ? pszMessage : luaL_typename(L, -1));
    D2LuaHost_Log(L, lua_tostring(L, -1));
    lua_pushboolean(L, 1);
    lua_setfield(L, nWrapper, "removed");
    lua_getfield(L, nEntry, "wrappers");
    int nWrappers = lua_gettop(L);
    int nCount = (int)lua_objlen(L, nWrappers);
    for (int i = 1; i <= nCount; i++) {
        lua_rawgeti(L, nWrappers, i);
        bool bThis = lua_rawequal(L, -1, nWrapper) != 0;
        lua_pop(L, 1);
        if (bThis) {
            for (int k = i; k < nCount; k++) {
                lua_rawgeti(L, nWrappers, k + 1);
                lua_rawseti(L, nWrappers, k);
            }
            lua_pushnil(L);
            lua_rawseti(L, nWrappers, nCount);
            break;
        }
    }
    lua_settop(L, nWrapper - 1);
    return RunLevel(L, nSnapshot, nEntry, nLevel - 1, nArgBase, pszLabel);
}

int D2LuaRt_RegistryCallRaw(lua_State* L, const char* pszRegistry, const char* pszId, int nArgBase) {
    int nTop = lua_gettop(L);
    int nArgs = nTop - nArgBase + 1;
    PushRegistry(L, pszRegistry);
    if (!lua_istable(L, -1)) {
        return luaL_error(L, "no registry '%s'", pszRegistry);
    }
    int nRegistry = lua_gettop(L);
    if (pszId[0]) {
        lua_pushfstring(L, "%s '%s'", pszRegistry, pszId);
    } else {
        lua_pushstring(L, pszRegistry);
    }
    int nLabel = lua_gettop(L);
    if (!PushEntry(L, nRegistry, pszId, false)) {
        return luaL_error(L, "registry %s has no entry '%s'", pszRegistry, pszId);
    }
    int nEntry = lua_gettop(L);
    // A snapshot of the wrappers, so removals during the call do not shift the levels.
    lua_getfield(L, nEntry, "wrappers");
    int nWrappers = lua_gettop(L);
    int nCount = (int)lua_objlen(L, nWrappers);
    lua_createtable(L, nCount, 0);
    int nSnapshot = lua_gettop(L);
    for (int i = 1; i <= nCount; i++) {
        lua_rawgeti(L, nWrappers, i);
        lua_rawseti(L, nSnapshot, i);
    }
    luaL_checkstack(L, nArgs + 8, "registry call");
    for (int i = 0; i < nArgs; i++) {
        lua_pushvalue(L, nArgBase + i);
    }
    int nResults = RunLevel(L, nSnapshot, nEntry, nCount, nSnapshot + 1, lua_tostring(L, nLabel));
    // Move the results down to nArgBase and drop everything between.
    int nFirst = lua_gettop(L) - nResults + 1;
    for (int i = 0; i < nResults; i++) {
        lua_pushvalue(L, nFirst + i);
        lua_replace(L, nArgBase + i);
    }
    lua_settop(L, nArgBase + nResults - 1);
    return nResults;
}

// upvalues: registry, id. Every stack value is an argument.
static int Lua_RegistryCallInner(lua_State* L) {
    return D2LuaRt_RegistryCallRaw(L, lua_tostring(L, lua_upvalueindex(1)), lua_tostring(L, lua_upvalueindex(2)), 1);
}

int D2LuaRt_RegistryCallProtected(lua_State* L, const char* pszRegistry, const char* pszId, int nArgBase) {
    lua_pushstring(L, pszRegistry);
    lua_pushstring(L, pszId);
    lua_pushcclosure(L, Lua_RegistryCallInner, 2);
    lua_insert(L, nArgBase);
    int nStatus = lua_pcall(L, lua_gettop(L) - nArgBase, LUA_MULTRET, 0);
    if (nStatus != 0) {
        // A chain error that reaches the caller is unwrapped to the error it carries.
        if (IsChainError(L, -1)) {
            lua_rawgeti(L, -1, 1);
        }
        return lua_error(L);
    }
    return lua_gettop(L) - nArgBase + 1;
}

// --- registries natives -----------------------------------------------------------------------------
// Each registry table in a mod's environment holds closures with upvalues 1 the mod, 2 the
// registry name.

static void AddWrapper(lua_State* L, D2LuaMod* pMod, int nEntry, int nFunction) {
    lua_getfield(L, nEntry, "wrappers");
    int nWrappers = lua_gettop(L);
    int nCount = (int)lua_objlen(L, nWrappers);
    D2LuaRuntime* pRt = pMod->pRt;
    int nSeq = ++pRt->nWrapperSeq;
    // Insert before the first wrapper of a later mod.
    int nAt = nCount + 1;
    for (int i = 1; i <= nCount; i++) {
        lua_rawgeti(L, nWrappers, i);
        lua_getfield(L, -1, "order");
        int nOrder = (int)lua_tointeger(L, -1);
        lua_pop(L, 2);
        if (nOrder > pMod->nIndex) {
            nAt = i;
            break;
        }
    }
    for (int i = nCount; i >= nAt; i--) {
        lua_rawgeti(L, nWrappers, i);
        lua_rawseti(L, nWrappers, i + 1);
    }
    lua_createtable(L, 0, 4);
    lua_pushvalue(L, nFunction);
    lua_setfield(L, -2, "fn");
    lua_pushinteger(L, pMod->nHostId);
    lua_setfield(L, -2, "mod");
    lua_pushinteger(L, pMod->nIndex);
    lua_setfield(L, -2, "order");
    lua_pushinteger(L, nSeq);
    lua_setfield(L, -2, "seq");
    lua_rawseti(L, nWrappers, nAt);
    lua_pop(L, 1);
}

static int RegistryKind(lua_State* L, int nRegistry) {
    lua_getfield(L, nRegistry, "kind");
    int nKind = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);
    return nKind;
}

// register(id, impl) / set(impl)
static int Lua_RegistryRegister(lua_State* L) {
    D2LuaMod* pMod = (D2LuaMod*)lua_touserdata(L, lua_upvalueindex(1));
    const char* pszRegistry = lua_tostring(L, lua_upvalueindex(2));
    PushRegistry(L, pszRegistry);
    int nRegistry = lua_gettop(L);
    bool bSlot = RegistryKind(L, nRegistry) == D2LUA_REGISTRY_SLOT;
    int nFn = bSlot ? 1 : 2;
    const char* pszId = bSlot ? "" : luaL_checkstring(L, 1);
    luaL_checktype(L, nFn, LUA_TFUNCTION);
    PushEntry(L, nRegistry, pszId, true);
    lua_pushvalue(L, nFn);
    lua_setfield(L, -2, "impl");
    lua_pushinteger(L, pMod->nHostId);
    lua_setfield(L, -2, "owner");
    return 0;
}

// wrap(id, fn) / wrap(fn)
static int Lua_RegistryWrap(lua_State* L) {
    D2LuaMod* pMod = (D2LuaMod*)lua_touserdata(L, lua_upvalueindex(1));
    const char* pszRegistry = lua_tostring(L, lua_upvalueindex(2));
    PushRegistry(L, pszRegistry);
    int nRegistry = lua_gettop(L);
    bool bSlot = RegistryKind(L, nRegistry) == D2LUA_REGISTRY_SLOT;
    int nFn = bSlot ? 1 : 2;
    const char* pszId = bSlot ? "" : luaL_checkstring(L, 1);
    luaL_checktype(L, nFn, LUA_TFUNCTION);
    if (!PushEntry(L, nRegistry, pszId, false)) {
        return luaL_error(L, "registry %s has no entry '%s' to wrap", pszRegistry, pszId);
    }
    AddWrapper(L, pMod, lua_gettop(L), nFn);
    return 0;
}

// call(id, ...) / call(...)
static int Lua_RegistryCall(lua_State* L) {
    const char* pszRegistry = lua_tostring(L, lua_upvalueindex(2));
    PushRegistry(L, pszRegistry);
    bool bSlot = RegistryKind(L, lua_gettop(L)) == D2LUA_REGISTRY_SLOT;
    lua_pop(L, 1);
    if (bSlot) {
        return D2LuaRt_RegistryCallProtected(L, pszRegistry, "", 1);
    }
    luaL_checkstring(L, 1);
    // The id string stays referenced at index 1 while the call runs above it.
    return D2LuaRt_RegistryCallProtected(L, pszRegistry, lua_tostring(L, 1), 2);
}

static int Lua_RegistryHas(lua_State* L) {
    const char* pszRegistry = lua_tostring(L, lua_upvalueindex(2));
    PushRegistry(L, pszRegistry);
    const char* pszId = luaL_checkstring(L, 1);
    bool bHas = PushEntry(L, lua_gettop(L), pszId, false);
    if (bHas) {
        lua_getfield(L, -1, "impl");
        bHas = !lua_isnil(L, -1);
    }
    lua_pushboolean(L, bHas);
    return 1;
}

int D2LuaRegistries_Push(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    lua_newtable(L);
    int nOut = lua_gettop(L);
    D2LuaRt_PushField(L, "registries");
    int nRegistries = lua_gettop(L);
    lua_pushnil(L);
    while (lua_next(L, nRegistries)) {
        bool bSlot = RegistryKind(L, lua_gettop(L)) == D2LUA_REGISTRY_SLOT;
        lua_pop(L, 1);
        const char* pszName = lua_tostring(L, -1);
        lua_newtable(L);
        static const struct { const char* pszKeyed; const char* pszSlot; lua_CFunction pfn; } s_aFns[] = {
            { "register", "set", Lua_RegistryRegister },
            { "wrap", "wrap", Lua_RegistryWrap },
            { "call", "call", Lua_RegistryCall },
            { "has", nullptr, Lua_RegistryHas },
        };
        for (const auto& tFn : s_aFns) {
            const char* pszField = bSlot ? tFn.pszSlot : tFn.pszKeyed;
            if (!pszField) {
                continue;
            }
            lua_pushlightuserdata(L, ctx.pMod);
            lua_pushstring(L, pszName);
            lua_pushcclosure(L, tFn.pfn, 2);
            lua_setfield(L, -2, pszField);
        }
        lua_setfield(L, nOut, pszName);
    }
    lua_settop(L, nOut);
    return 1;
}

// --- script -----------------------------------------------------------------------------------------

const char* D2LuaScript_ModName(D2LuaCtx& ctx) {
    return D2LuaCtx_ModName(ctx);
}

lua_Integer D2LuaScript_Tick(D2LuaCtx& ctx) {
    return ctx.pRuntime ? (lua_Integer)ctx.pRuntime->nTick : 0;
}

lua_Integer D2LuaScript_Side(D2LuaCtx& ctx) {
    return ctx.nSide;
}

static void SetModFunction(D2LuaCtx& ctx, const char* pszTable, D2LuaFn fn) {
    lua_State* L = ctx.L;
    D2LuaRt_PushField(L, pszTable);
    if (fn.nIndex) {
        lua_pushvalue(L, fn.nIndex);
    } else {
        lua_pushnil(L);
    }
    lua_setfield(L, -2, ctx.pMod->sName.c_str());
    lua_pop(L, 1);
}

void D2LuaScript_OnEvent(D2LuaCtx& ctx, const char* pszEvent, D2LuaFn fn) {
    lua_State* L = ctx.L;
    D2LuaRt_PushField(L, "handlers");
    lua_getfield(L, -1, ctx.pMod->sName.c_str());
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, ctx.pMod->sName.c_str());
    }
    if (fn.nIndex) {
        lua_pushvalue(L, fn.nIndex);
    } else {
        lua_pushnil(L);
    }
    lua_setfield(L, -2, pszEvent);
    lua_pop(L, 2);
}

void D2LuaScript_OnInit(D2LuaCtx& ctx, D2LuaFn fn) {
    SetModFunction(ctx, "init", fn);
}

void D2LuaScript_OnLoad(D2LuaCtx& ctx, D2LuaFn fn) {
    SetModFunction(ctx, "load", fn);
}

// script.start(body, owner?): the script's id. An owner is a handle; the script is dropped once
// the owner's object no longer exists.
int D2LuaScript_Start(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    luaL_checktype(L, 1, LUA_TFUNCTION);
    bool bOwner = !lua_isnoneornil(L, 2);
    if (bOwner) {
        const char* pszClass;
        uint64_t nId;
        if (!D2LuaHandle_Inspect(L, 2, &pszClass, &nId)) {
            return luaL_error(L, "bad argument #2 to 'script.start' (a handle expected as the owner, got %s)", luaL_typename(L, 2));
        }
        if (!D2LuaHandle_IsLive(L, 2)) {
            return luaL_error(L, "bad argument #2 to 'script.start' (the owner %s is not valid)", pszClass);
        }
    }
    lua_settop(L, 2);
    D2LuaFn body = { 1 };
    D2LuaRt_PushField(L, "scripts");
    int nScripts = lua_gettop(L);
    lua_createtable(L, 0, 4);
    int nRecord = lua_gettop(L);
    lua_State* co = lua_newthread(L);
    lua_pushvalue(L, body.nIndex);
    lua_xmove(L, co, 1);
    D2LuaHost_SetCoroutineOwner(L, -1, ctx.pMod->nHostId);
    D2LuaRt_PushField(L, "script_threads");
    lua_pushvalue(L, -2);
    lua_pushboolean(L, 1);
    lua_rawset(L, -3);
    lua_pop(L, 1);
    lua_setfield(L, nRecord, "co");
    lua_pushstring(L, ctx.pMod->sName.c_str());
    lua_setfield(L, nRecord, "owner");
    lua_pushnumber(L, (lua_Number)(ctx.pRuntime->nTick + 1));
    lua_setfield(L, nRecord, "due");
    lua_Integer nId = ++ctx.pRuntime->nScriptSeq;
    lua_pushnumber(L, (lua_Number)nId);
    lua_setfield(L, nRecord, "id");
    if (bOwner) {
        lua_pushvalue(L, 2);
        lua_setfield(L, nRecord, "holder");
    }
    lua_rawseti(L, nScripts, (int)lua_objlen(L, nScripts) + 1);
    lua_pop(L, 1);
    lua_pushnumber(L, (lua_Number)nId);
    return 1;
}

// The live record of script nId started by the calling mod, pushed; false (nothing pushed) otherwise.
static bool PushOwnScript(D2LuaCtx& ctx, lua_Integer nId) {
    lua_State* L = ctx.L;
    D2LuaRt_PushField(L, "scripts");
    int nCount = (int)lua_objlen(L, -1);
    for (int i = 1; i <= nCount; i++) {
        lua_rawgeti(L, -1, i);
        lua_getfield(L, -1, "id");
        lua_getfield(L, -2, "owner");
        lua_getfield(L, -3, "done");
        bool bHit = (lua_Integer)lua_tonumber(L, -3) == nId && lua_type(L, -2) == LUA_TSTRING
                    && strcmp(lua_tostring(L, -2), ctx.pMod->sName.c_str()) == 0 && !lua_toboolean(L, -1);
        lua_pop(L, 3);
        if (bHit) {
            lua_remove(L, -2);
            return true;
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return false;
}

bool D2LuaScript_Cancel(D2LuaCtx& ctx, lua_Integer nId) {
    if (!PushOwnScript(ctx, nId)) {
        return false;
    }
    lua_pushboolean(ctx.L, 1);
    lua_setfield(ctx.L, -2, "done");
    lua_pop(ctx.L, 1);
    return true;
}

bool D2LuaScript_Running(D2LuaCtx& ctx, lua_Integer nId) {
    if (!PushOwnScript(ctx, nId)) {
        return false;
    }
    lua_pop(ctx.L, 1);
    return true;
}

// Whether the running thread is a host script (started with script.start).
static bool InScript(lua_State* L) {
    if (lua_pushthread(L)) {
        lua_pop(L, 1);
        return false;  // the main thread
    }
    D2LuaRt_PushField(L, "script_threads");
    lua_pushvalue(L, -2);
    lua_rawget(L, -2);
    bool bIs = lua_toboolean(L, -1) != 0;
    lua_pop(L, 3);
    return bIs;
}

int D2LuaScript_Wait(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    lua_Number nTicks = luaL_optnumber(L, 1, 1);
    if (!InScript(L)) {
        return luaL_error(L, "script.wait can only be used inside a script (started with script.start)");
    }
    lua_settop(L, 0);
    lua_createtable(L, 0, 1);
    lua_pushnumber(L, nTicks);
    lua_setfield(L, -2, "ticks");
    return lua_yield(L, 1);
}

int D2LuaScript_WaitEvent(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    const char* pszEvent = luaL_checkstring(L, 1);
    D2LuaRt_PushField(L, "events");
    lua_getfield(L, -1, pszEvent);
    bool bKnown = !lua_isnil(L, -1);
    lua_pop(L, 2);
    if (!bKnown) {
        return luaL_error(L, "bad argument #1 to 'script.wait_event' (no event '%s' on this side)", pszEvent);
    }
    if (!InScript(L)) {
        return luaL_error(L, "script.wait_event can only be used inside a script (started with script.start)");
    }
    lua_settop(L, 1);
    lua_createtable(L, 0, 1);
    lua_insert(L, 1);
    lua_setfield(L, 1, "event");
    return lua_yield(L, 1);
}

// --- remote ---------------------------------------------------------------------------------------

void D2LuaRemote_AddInterface(D2LuaCtx& ctx, const char* pszName, D2LuaTable tFunctions) {
    lua_State* L = ctx.L;
    D2LuaRt_PushField(L, "remote");
    lua_getfield(L, -1, pszName);
    if (!lua_isnil(L, -1)) {
        lua_getfield(L, -1, "owner");
        luaL_error(L, "remote interface '%s' already exists (added by mod '%s')", pszName, lua_tostring(L, -1));
    }
    lua_pop(L, 1);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, D2LuaCtx_ModName(ctx));
    lua_setfield(L, -2, "owner");
    lua_newtable(L);
    lua_pushnil(L);
    while (lua_next(L, tFunctions.nIndex)) {
        if (lua_type(L, -2) != LUA_TSTRING || !lua_isfunction(L, -1)) {
            luaL_error(L, "remote.add_interface: interface '%s' must map names to functions", pszName);
        }
        lua_pushvalue(L, -2);
        lua_insert(L, -2);
        lua_rawset(L, -4);
    }
    lua_setfield(L, -2, "fns");
    lua_setfield(L, -2, pszName);
    lua_pop(L, 1);
}

bool D2LuaRemote_RemoveInterface(D2LuaCtx& ctx, const char* pszName) {
    lua_State* L = ctx.L;
    D2LuaRt_PushField(L, "remote");
    lua_getfield(L, -1, pszName);
    bool bHad = !lua_isnil(L, -1);
    lua_pop(L, 1);
    lua_pushnil(L);
    lua_setfield(L, -2, pszName);
    lua_pop(L, 1);
    return bHad;
}

int D2LuaRemote_Call(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    const char* pszInterface = luaL_checkstring(L, 1);
    const char* pszFunction = luaL_checkstring(L, 2);
    D2LuaRt_PushField(L, "remote");
    lua_getfield(L, -1, pszInterface);
    if (lua_isnil(L, -1)) {
        return luaL_error(L, "remote.call: unknown interface '%s'", pszInterface);
    }
    lua_getfield(L, -1, "owner");
    D2LuaMod* pOwner = D2LuaRt_ModByName(ctx.pRuntime, lua_tostring(L, -1));
    lua_pop(L, 1);
    lua_getfield(L, -1, "fns");
    lua_getfield(L, -1, pszFunction);
    if (!lua_isfunction(L, -1)) {
        return luaL_error(L, "remote.call: interface '%s' has no function '%s'", pszInterface, pszFunction);
    }
    if (pOwner && D2LuaRt_ModFaulted(L, pOwner)) {
        return luaL_error(L, "remote.call: interface '%s' belongs to mod '%s', which is disabled", pszInterface,
                          pOwner->sName.c_str());
    }
    int nFunction = lua_gettop(L);
    int nArgs = nFunction - 3 - 2;  // below: remote, interface, fns
    for (int i = 3; i < 3 + nArgs; i++) {
        lua_pushvalue(L, i);
    }
    int nOwnerDepth = D2LuaHost_OwnerDepth(L);
    if (pOwner) {
        D2LuaHost_PushOwner(L, pOwner->nHostId);
    }
    int nStatus = lua_pcall(L, nArgs, LUA_MULTRET, 0);
    D2LuaHost_RestoreOwnerDepth(L, nOwnerDepth);
    if (nStatus != 0) {
        return lua_error(L);
    }
    return lua_gettop(L) - nFunction + 1;
}

int D2LuaRemote_Interfaces(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    lua_newtable(L);
    int nOut = lua_gettop(L);
    D2LuaRt_PushField(L, "remote");
    lua_pushnil(L);
    while (lua_next(L, -2)) {
        lua_newtable(L);
        lua_getfield(L, -2, "fns");
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            lua_pop(L, 1);
            lua_pushvalue(L, -1);
            lua_pushboolean(L, 1);
            lua_rawset(L, -5);
        }
        lua_pop(L, 1);
        lua_pushvalue(L, -3);
        lua_insert(L, -2);
        lua_rawset(L, nOut);
        lua_pop(L, 1);
    }
    lua_settop(L, nOut);
    return 1;
}

// --- storage, log, data ---------------------------------------------------------------------------

int D2LuaStorage_Push(D2LuaCtx& ctx) {
    lua_newtable(ctx.L);
    return 1;
}

int D2LuaNative_Log(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    int nArgs = lua_gettop(L);
    luaL_Buffer tBuffer;
    lua_getglobal(L, "tostring");
    luaL_buffinit(L, &tBuffer);
    for (int i = 1; i <= nArgs; i++) {
        lua_pushvalue(L, nArgs + 1);
        lua_pushvalue(L, i);
        lua_call(L, 1, 1);
        if (!lua_isstring(L, -1)) {
            return luaL_error(L, "'tostring' must return a string to 'log'");
        }
        if (i > 1) {
            luaL_addchar(&tBuffer, ' ');
        }
        luaL_addvalue(&tBuffer);
    }
    luaL_pushresult(&tBuffer);
    lua_pushfstring(L, "[lua] [%s] %s", D2LuaCtx_ModName(ctx), lua_tostring(L, -1));
    D2LuaHost_Log(L, lua_tostring(L, -1));
    return 0;
}

// data.extend(prototypes): each { type = t, name = n, ... } lands in data.raw[t][n].
static int Lua_DataExtend(lua_State* L) {
    // Accepts data.extend(list) and data:extend(list).
    int nList = lua_gettop(L) >= 2 ? 2 : 1;
    luaL_checktype(L, nList, LUA_TTABLE);
    lua_getfield(L, lua_upvalueindex(1), "raw");
    int nRaw = lua_gettop(L);
    int nCount = (int)lua_objlen(L, nList);
    for (int i = 1; i <= nCount; i++) {
        lua_rawgeti(L, nList, i);
        int nProto = lua_gettop(L);
        if (!lua_istable(L, nProto)) {
            return luaL_error(L, "data.extend: entry %d is not a table", i);
        }
        lua_getfield(L, nProto, "type");
        lua_getfield(L, nProto, "name");
        if (lua_type(L, -2) != LUA_TSTRING || lua_type(L, -1) != LUA_TSTRING) {
            return luaL_error(L, "data.extend: entry %d needs string fields type and name", i);
        }
        lua_pushvalue(L, -2);
        lua_rawget(L, nRaw);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushvalue(L, -3);
            lua_pushvalue(L, -2);
            lua_rawset(L, nRaw);
        }
        lua_pushvalue(L, -2);
        lua_pushvalue(L, nProto);
        lua_rawset(L, -3);
        lua_settop(L, nProto - 1);
    }
    return 0;
}

int D2LuaData_Push(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    D2LuaRt_PushTable(L);
    lua_getfield(L, -1, "data");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_newtable(L);
        lua_setfield(L, -2, "raw");
        lua_pushvalue(L, -1);
        lua_pushcclosure(L, Lua_DataExtend, 1);
        lua_setfield(L, -2, "extend");
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, "data");
    }
    lua_remove(L, -2);
    return 1;
}

// --- the example registry's native entries -------------------------------------------------------------

const char* D2LuaExample_Greet(D2LuaCtx& ctx, const char* pszName) {
    (void)ctx;
    static thread_local char s_szBuf[256];
    snprintf(s_szBuf, sizeof(s_szBuf), "hello, %s", pszName);
    return s_szBuf;
}

double D2LuaExample_Slot(D2LuaCtx& ctx, double nValue) {
    (void)ctx;
    return nValue * 2;
}
#endif
