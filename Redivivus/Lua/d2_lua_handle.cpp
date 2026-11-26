// Handle classes: see d2_lua_api.h. A handle is a full userdata holding the class descriptor
// and the object's id; every access resolves the id through the class, so a handle to an object
// that is gone answers .valid == false and raises a clean error on any other use.
#if D2_FEATURE_MODS
#include <string.h>

#include "Redivivus/Lua/d2_lua_internal.h"
#include "Redivivus/Lua/d2_lua_rt.h"
#include "lua.hpp"

namespace {

const uint32_t kMagic = 0x44324c48;  // "D2LH"

struct HandleData {
    uint32_t nMagic;
    const D2LuaHandleClass* pClass;
    uint64_t nId;
};

}  // namespace

static HandleData* ToHandle(lua_State* L, int i) {
    if (lua_type(L, i) != LUA_TUSERDATA || lua_objlen(L, i) != sizeof(HandleData)) {
        return nullptr;
    }
    HandleData* pData = (HandleData*)lua_touserdata(L, i);
    return pData && pData->nMagic == kMagic ? pData : nullptr;
}

static int InvalidError(lua_State* L, const HandleData* pData, const char* pszWhere) {
    return luaL_error(L, "%s: this %s is not valid (the object no longer exists)", pszWhere, pData->pClass->pszName);
}

// __index: upvalues 1 methods, 2 getters.
static int Lua_HandleIndex(lua_State* L) {
    HandleData* pData = ToHandle(L, 1);
    if (!pData) {
        return luaL_error(L, "not a handle");
    }
    const char* pszKey = lua_tostring(L, 2);
    if (pszKey && strcmp(pszKey, "valid") == 0) {
        lua_pushboolean(L, pData->pClass->pfnResolve(L, pData->nId) != nullptr);
        return 1;
    }
    if (pszKey && strcmp(pszKey, "objectName") == 0) {
        lua_pushstring(L, pData->pClass->pszName);
        return 1;
    }
    lua_pushvalue(L, 2);
    lua_rawget(L, lua_upvalueindex(1));
    if (!lua_isnil(L, -1)) {
        return 1;
    }
    lua_pop(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, lua_upvalueindex(2));
    if (lua_isnil(L, -1)) {
        return luaL_error(L, "%s has no member '%s'", pData->pClass->pszName, pszKey ? pszKey : luaL_typename(L, 2));
    }
    if (!pData->pClass->pfnResolve(L, pData->nId)) {
        lua_pushfstring(L, "%s.%s", pData->pClass->pszName, pszKey);
        return InvalidError(L, pData, lua_tostring(L, -1));
    }
    lua_pushvalue(L, 1);
    lua_call(L, 1, 1);
    return 1;
}

// __newindex: upvalues 1 setters, 2 getters.
static int Lua_HandleNewIndex(lua_State* L) {
    HandleData* pData = ToHandle(L, 1);
    if (!pData) {
        return luaL_error(L, "not a handle");
    }
    const char* pszKey = lua_tostring(L, 2);
    lua_pushvalue(L, 2);
    lua_rawget(L, lua_upvalueindex(1));
    if (lua_isnil(L, -1)) {
        lua_pushvalue(L, 2);
        lua_rawget(L, lua_upvalueindex(2));
        if (!lua_isnil(L, -1) || (pszKey && strcmp(pszKey, "valid") == 0)) {
            return luaL_error(L, "%s.%s is read-only", pData->pClass->pszName, pszKey);
        }
        return luaL_error(L, "%s has no member '%s'", pData->pClass->pszName, pszKey ? pszKey : luaL_typename(L, 2));
    }
    if (!pData->pClass->pfnResolve(L, pData->nId)) {
        lua_pushfstring(L, "%s.%s", pData->pClass->pszName, pszKey);
        return InvalidError(L, pData, lua_tostring(L, -1));
    }
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 3);
    lua_call(L, 2, 0);
    return 0;
}

static int Lua_HandleToString(lua_State* L) {
    HandleData* pData = ToHandle(L, 1);
    if (!pData) {
        return luaL_error(L, "not a handle");
    }
    lua_pushfstring(L, "%s: %s%f", pData->pClass->pszName,
                    pData->pClass->pfnResolve(L, pData->nId) ? "" : "invalid ", (lua_Number)pData->nId);
    return 1;
}

static int Lua_HandleEq(lua_State* L) {
    HandleData* a = ToHandle(L, 1);
    HandleData* b = ToHandle(L, 2);
    lua_pushboolean(L, a && b && a->pClass == b->pClass && a->nId == b->nId);
    return 1;
}

static void PushRegTable(lua_State* L, const luaL_Reg* aReg) {
    lua_newtable(L);
    for (const luaL_Reg* p = aReg; p && p->name; p++) {
        lua_pushcfunction(L, p->func);
        lua_setfield(L, -2, p->name);
    }
}

void D2LuaHandle_Register(lua_State* L, const D2LuaHandleClass* pClass) {
    D2LuaRt_PushField(L, "classes");
    lua_newtable(L);  // the class record
    int nRecord = lua_gettop(L);
    PushRegTable(L, pClass->aMethods);
    lua_setfield(L, nRecord, "methods");
    PushRegTable(L, pClass->aGetters);
    lua_setfield(L, nRecord, "getters");
    PushRegTable(L, pClass->aSetters);
    lua_setfield(L, nRecord, "setters");
    lua_newtable(L);  // the metatable
    lua_getfield(L, nRecord, "methods");
    lua_getfield(L, nRecord, "getters");
    lua_pushcclosure(L, Lua_HandleIndex, 2);
    lua_setfield(L, -2, "__index");
    lua_getfield(L, nRecord, "setters");
    lua_getfield(L, nRecord, "getters");
    lua_pushcclosure(L, Lua_HandleNewIndex, 2);
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, Lua_HandleToString);
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, Lua_HandleEq);
    lua_setfield(L, -2, "__eq");
    lua_pushboolean(L, 0);
    lua_setfield(L, -2, "__metatable");
    lua_setfield(L, nRecord, "mt");
    lua_newtable(L);
    lua_newtable(L);
    lua_pushliteral(L, "v");
    lua_setfield(L, -2, "__mode");
    lua_setmetatable(L, -2);
    lua_setfield(L, nRecord, "intern");
    lua_pushlightuserdata(L, (void*)pClass);
    lua_setfield(L, nRecord, "cls");
    lua_setfield(L, -2, pClass->pszName);
    lua_pop(L, 1);
}

bool D2LuaHandle_PushMembers(lua_State* L, const char* pszClass) {
    D2LuaRt_PushField(L, "classes");
    lua_getfield(L, -1, pszClass);
    lua_remove(L, -2);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    lua_getfield(L, -1, "methods");
    lua_getfield(L, -2, "getters");
    lua_getfield(L, -3, "setters");
    lua_remove(L, -4);
    return true;
}

static bool PushClassRecord(lua_State* L, const char* pszClass) {
    D2LuaRt_PushField(L, "classes");
    lua_getfield(L, -1, pszClass);
    lua_remove(L, -2);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    return true;
}

static void PushIdWithRecord(lua_State* L, const D2LuaHandleClass* pClass, uint64_t nId) {
    int nRecord = lua_gettop(L);
    lua_getfield(L, nRecord, "intern");
    lua_pushnumber(L, (lua_Number)nId);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) {
        lua_replace(L, nRecord);
        lua_settop(L, nRecord);
        return;
    }
    lua_pop(L, 1);
    HandleData* pData = (HandleData*)lua_newuserdata(L, sizeof(HandleData));
    pData->nMagic = kMagic;
    pData->pClass = pClass;
    pData->nId = nId;
    lua_getfield(L, nRecord, "mt");
    lua_setmetatable(L, -2);
    lua_pushnumber(L, (lua_Number)nId);
    lua_pushvalue(L, -2);
    lua_rawset(L, -4);
    lua_replace(L, nRecord);
    lua_settop(L, nRecord);
}

void D2LuaHandle_PushId(lua_State* L, const D2LuaHandleClass* pClass, uint64_t nId) {
    if (!PushClassRecord(L, pClass->pszName)) {
        luaL_error(L, "handle class %s is not registered in this state", pClass->pszName);
    }
    PushIdWithRecord(L, pClass, nId);
}

void D2LuaHandle_PushObject(lua_State* L, const D2LuaHandleClass* pClass, void* pObject) {
    if (!pObject) {
        lua_pushnil(L);
        return;
    }
    D2LuaHandle_PushId(L, pClass, pClass->pfnIdOf(pObject));
}

bool D2LuaHandle_PushByName(lua_State* L, const char* pszClass, uint64_t nId) {
    if (!PushClassRecord(L, pszClass)) {
        return false;
    }
    lua_getfield(L, -1, "cls");
    const D2LuaHandleClass* pClass = (const D2LuaHandleClass*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    PushIdWithRecord(L, pClass, nId);
    return true;
}

void* D2LuaHandle_Check(lua_State* L, int i, const D2LuaHandleClass* pClass, const char* pszWhere, bool bOptional) {
    if (bOptional && lua_isnoneornil(L, i)) {
        return nullptr;
    }
    HandleData* pData = ToHandle(L, i);
    if (!pData || pData->pClass != pClass) {
        luaL_error(L, "bad argument #%d to '%s' (%s expected, got %s)", i, pszWhere, pClass->pszName,
                   pData ? pData->pClass->pszName : luaL_typename(L, i));
        return nullptr;
    }
    void* pObject = pClass->pfnResolve(L, pData->nId);
    if (!pObject) {
        InvalidError(L, pData, pszWhere);
    }
    return pObject;
}

bool D2LuaHandle_IsLive(lua_State* L, int i) {
    HandleData* pData = ToHandle(L, i);
    return pData && pData->pClass->pfnResolve(L, pData->nId) != nullptr;
}

bool D2LuaHandle_Inspect(lua_State* L, int i, const char** ppszClass, uint64_t* pnId) {
    HandleData* pData = ToHandle(L, i);
    if (!pData) {
        return false;
    }
    *ppszClass = pData->pClass->pszName;
    *pnId = pData->nId;
    return true;
}
#endif
