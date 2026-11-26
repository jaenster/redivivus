// What generated binding glue (Redivivus/Lua/generated/*.gen.cpp, written by
// Redivivus/Lua/tools/src/gen.ts from Redivivus/Lua/core/native/*.d.ts and the core's public modules)
// and the natives it calls build on. Natives are called by the core only (they sit in the table the
// core starts with); handle classes are the native object API mods use directly.
//
// The calling convention of a native, from the declaration's types:
//   function     T Native(D2LuaCtx& ctx, A1 a1, A2 a2, ...)
//   method       T Native(D2LuaCtx& ctx, Object* self, A1 a1, ...)        (handle classes)
//   getter       T Native(D2LuaCtx& ctx, Object* self)  or  T Native(D2LuaCtx& ctx) on a singleton
//   setter       void Native(D2LuaCtx& ctx, Object* self, T value)
//   @raw         int Native(D2LuaCtx& ctx)  - reads its arguments from the Lua stack (1..n) itself
//                and returns the number of results it pushed, as a lua_CFunction does
// with these C++ types per declared type:
//   string -> const char*        number -> double        integer -> lua_Integer (checked integral)
//   boolean -> bool              enum E -> its @native C++ enum type (checked against the members)
//   handle class C -> C's Object* (resolved and checked valid; never null for a required argument)
//   function -> D2LuaFn          table / plain-data object -> D2LuaTable    unknown -> D2LuaAny
//   x?: T or T | undefined -> T may be absent: pointers are null, numbers come as D2LuaOpt<T>,
//                D2LuaFn / D2LuaTable / D2LuaAny carry nIndex 0
//   string-literal union -> const char*, checked against the literals
// Returned: the same types; a returned const char* must stay valid until the native's caller
// pushes it (a static or Lua-owned buffer); an optional handle return of null pushes nil.
//
// Natives run inside Lua's error handling: like everything else Lua calls, they hold no object
// with a destructor across anything that may raise (luaL_error, lua_* that allocate).
#pragma once

#if D2_FEATURE_MODS
#include <stddef.h>
#include <stdint.h>

#include "lua.hpp"

struct D2LuaMod;      // one mod in a runtime (d2_lua_runtime.cpp)
struct D2LuaRuntime;  // one state with its mods

enum D2LuaSide {
    D2LUA_SIDE_DATA = 1,
    D2LUA_SIDE_SERVER = 2,
    D2LUA_SIDE_CLIENT = 4,
};

struct D2LuaCtx {
    lua_State* L;
    D2LuaRuntime* pRuntime;
    D2LuaMod* pMod;  // the mod calling; null when the host calls
    int nSide;
};

struct D2LuaFn { int nIndex; };     // a function argument's stack index, 0 when absent
struct D2LuaTable { int nIndex; };  // a table argument's stack index, 0 when absent
struct D2LuaAny { int nIndex; };    // any value's stack index, 0 when absent

template <typename T>
struct D2LuaOpt {
    bool bHas;
    T tValue;
};

// --- the context ---------------------------------------------------------------------------------
// For a function shared by every mod (handle metatables): the mod whose code is running.
D2LuaCtx D2LuaCtx_FromOwner(lua_State* L);
// For a native taking a mod id from the core: the mod, or null.
D2LuaCtx D2LuaCtx_ForMod(lua_State* L, D2LuaMod* pMod);
const char* D2LuaCtx_ModName(const D2LuaCtx& ctx);

// --- argument checks; pszWhere is "script.on_event" for messages ---------------------------------
const char* D2LuaArg_String(lua_State* L, int i, const char* pszWhere, bool bOptional);
double D2LuaArg_Number(lua_State* L, int i, const char* pszWhere);
D2LuaOpt<double> D2LuaArg_OptNumber(lua_State* L, int i, const char* pszWhere);
lua_Integer D2LuaArg_Integer(lua_State* L, int i, const char* pszWhere);
D2LuaOpt<lua_Integer> D2LuaArg_OptInteger(lua_State* L, int i, const char* pszWhere);
bool D2LuaArg_Boolean(lua_State* L, int i, const char* pszWhere);
D2LuaOpt<bool> D2LuaArg_OptBoolean(lua_State* L, int i, const char* pszWhere);
D2LuaFn D2LuaArg_Function(lua_State* L, int i, const char* pszWhere, bool bOptional);
D2LuaTable D2LuaArg_Table(lua_State* L, int i, const char* pszWhere, bool bOptional);
D2LuaAny D2LuaArg_Any(lua_State* L, int i);
// A string that must be one of apszAllowed (null-terminated list).
const char* D2LuaArg_OneOf(lua_State* L, int i, const char* pszWhere, const char* const* apszAllowed, bool bOptional);
// An event name the state knows for the calling side (events declared in an @events map).
const char* D2LuaArg_EventName(lua_State* L, int i, const char* pszWhere, const char* pszEventMap, bool bOptional);
// An integer that must be one of anValues (nCount of them); for enums.
lua_Integer D2LuaArg_Enum(lua_State* L, int i, const char* pszWhere, const char* pszEnum, const lua_Integer* anValues,
                          int nCount);

// --- handle classes ------------------------------------------------------------------------------
// A handle names an engine object by (class, id) and is looked up on every access; it never holds
// the object's address. Ids are below 2^53 (they live in Lua numbers). Handles are interned per
// state: the same (class, id) is the same userdata while anything holds it, so handles work as
// table keys and compare with ==.
struct D2LuaHandleClass {
    const char* pszName;
    // The object for id in the state L belongs to, or null when it no longer exists (or is not
    // usable now: a monster that died, a unit in a room that was frozen).
    void* (*pfnResolve)(lua_State* L, uint64_t nId);
    // The id of a live object.
    uint64_t (*pfnIdOf)(void* pObject);
    const luaL_Reg* aMethods;  // lua_CFunctions; self at index 1
    const luaL_Reg* aGetters;  // lua_CFunctions called with the handle at index 1
    const luaL_Reg* aSetters;  // lua_CFunctions called with the handle at 1 and the value at 2
};

// Make the class known to the state (metatable, intern table). Once per state per class.
void D2LuaHandle_Register(lua_State* L, const D2LuaHandleClass* pClass);
void D2LuaHandle_PushId(lua_State* L, const D2LuaHandleClass* pClass, uint64_t nId);
// Push a handle for a live object, or nil for null.
void D2LuaHandle_PushObject(lua_State* L, const D2LuaHandleClass* pClass, void* pObject);
// The object behind the handle at index i: raises a clean error when i is not a handle of this
// class, or when the object is gone. Null only when bOptional and the value is nil.
void* D2LuaHandle_Check(lua_State* L, int i, const D2LuaHandleClass* pClass, const char* pszWhere, bool bOptional);
// Push the member tables of handle class pszClass: methods, getters, setters (what __index and
// __newindex read; a function added to one is a member of every handle of the class). For the
// core, which implements the members of the engine's classes in TypeScript. False when the state
// has no such class (nothing pushed).
bool D2LuaHandle_PushMembers(lua_State* L, const char* pszClass);
// Whether the value at i is a handle whose object exists.
bool D2LuaHandle_IsLive(lua_State* L, int i);
// Whether the value at i is a handle (of any class); class name and id out.
bool D2LuaHandle_Inspect(lua_State* L, int i, const char** ppszClass, uint64_t* pnId);
// Push the handle of class pszClass with id nId; false when the state has no such class.
bool D2LuaHandle_PushByName(lua_State* L, const char* pszClass, uint64_t nId);

// --- events and registries the declarations define ---------------------------------------------
// The events of one @events map, for the sides in nSides.
void D2LuaApi_DefineEvents(lua_State* L, const char* pszEventMap, int nSides, const char* const* apszNames);
enum D2LuaRegistryKind { D2LUA_REGISTRY_KEYED = 0, D2LUA_REGISTRY_SLOT = 1 };
void D2LuaApi_DefineRegistry(lua_State* L, const char* pszRegistry, int nKind, int nSides);
// An enum's members and values (anNumbers or apszStrings, parallel to the null-terminated
// apszNames); a second call for the same enum adds members. Once per state per declaration set.
void D2LuaApi_DefineEnum(lua_State* L, const char* pszEnum, const char* const* apszNames, const lua_Number* anNumbers,
                         const char* const* apszStrings);
// A native implementation at the bottom of a registry entry's chain (pszId null for a slot).
void D2LuaApi_RegistryNative(lua_State* L, const char* pszRegistry, const char* pszId, lua_CFunction pfn);
// Raise an event whose payload table is at the top of the stack (popped), in the game the host is
// working on (the one whose tick, event or call is running); see D2LuaRt_RaiseEvent.
int D2LuaApi_RaiseEvent(lua_State* L, const char* pszEvent);

// A table of the natives in aNatives, pushed (for the core's natives table).
void D2LuaApi_PushNatives(lua_State* L, const luaL_Reg* aNatives);

// --- one declaration file set ------------------------------------------------------------------
struct D2LuaApiDesc {
    const char* pszName;
    // Once per state: handle classes, events, registries, native registry entries.
    void (*pfnRegister)(lua_State* L, int nSide);
    // Once per state: its native tables into the core's natives table at nTable.
    void (*pfnNatives)(lua_State* L, int nTable, int nSide);
};
#endif
