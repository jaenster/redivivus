// What generated glue builds on (d2_lua_api.h): the context of a native, the argument checks, the
// tables the declarations fill (events, registries and their native entries, enums), the natives
// tables the core gets, and pairs for mod environments. The example registry's native entries are
// here too. Registries, events and the rest of the API's behaviour are the core's
// (Redivivus/Lua/core); the native layer only records what the declarations define.
#if D2_FEATURE_MODS
#include <stdio.h>
#include <string.h>

#include "Redivivus/Lua/d2_lua_internal.h"
#include "Redivivus/Lua/d2_lua_rt.h"
#include "lua.hpp"

// --- context ---------------------------------------------------------------------------------------

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

void D2LuaApi_PushNatives(lua_State* L, const luaL_Reg* aNatives) {
    lua_newtable(L);
    for (const luaL_Reg* p = aNatives; p && p->name; p++) {
        lua_pushcfunction(L, p->func);
        lua_setfield(L, -2, p->name);
    }
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

// --- enums ----------------------------------------------------------------------------------------
// enums[name] = { [member] = value }. The core builds the read-only tables mods see from these.

void D2LuaApi_DefineEnum(lua_State* L, const char* pszEnum, const char* const* apszNames, const lua_Number* anNumbers,
                         const char* const* apszStrings) {
    D2LuaRt_PushField(L, "enums");
    lua_getfield(L, -1, pszEnum);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, pszEnum);
    }
    for (int i = 0; apszNames[i]; i++) {
        if (anNumbers) {
            lua_pushnumber(L, anNumbers[i]);
        } else {
            lua_pushstring(L, apszStrings[i]);
        }
        lua_setfield(L, -2, apszNames[i]);
    }
    lua_pop(L, 2);
}

// --- read-only proxies in mod environments ------------------------------------------------------

static char s_WalkKey;

void* D2LuaRt_WalkKey() {
    return &s_WalkKey;
}

// pairs for mods: Lua 5.1's (next, t, nil), except that a read-only proxy of the core (a module's
// exports, an enum, a facade object) walks its values. Upvalue 1 is next. The metatable is read
// raw, so nothing of the mod's runs here.
int D2LuaRt_EnvPairs(lua_State* L) {
    luaL_checkany(L, 1);
    lua_pushvalue(L, lua_upvalueindex(1));
    if (lua_type(L, 1) == LUA_TTABLE && lua_getmetatable(L, 1)) {
        lua_pushlightuserdata(L, &s_WalkKey);
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

// --- events and registries ---------------------------------------------------------------------------

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

void D2LuaApi_DefineRegistry(lua_State* L, const char* pszRegistry, int nKind, int nSides) {
    D2LuaRuntime* pRt = D2LuaRt_FromState(L);
    if (!pRt || !(pRt->tDesc.nSide & nSides)) {
        return;
    }
    D2LuaRt_PushField(L, "registries");
    lua_createtable(L, 0, 2);
    lua_pushinteger(L, nKind);
    lua_setfield(L, -2, "kind");
    lua_newtable(L);
    lua_setfield(L, -2, "natives");
    lua_setfield(L, -2, pszRegistry);
    lua_pop(L, 1);
}

void D2LuaApi_RegistryNative(lua_State* L, const char* pszRegistry, const char* pszId, lua_CFunction pfn) {
    D2LuaRt_PushField(L, "registries");
    lua_getfield(L, -1, pszRegistry);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 2);
        return;
    }
    lua_getfield(L, -1, "natives");
    lua_pushcfunction(L, pfn);
    lua_setfield(L, -2, pszId ? pszId : "");
    lua_pop(L, 3);
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
