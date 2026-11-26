// Mod settings (the kit's settings in Redivivus/Lua/kit build on these natives). A mod declares
// each setting once per state (type, default, range or choices, scope); the value comes from the
// settings file under the mods root ({ "<mod>": { "<setting>": value } }), checked against the
// declaration, or is the default when the file has none or a bad one. The host lists every
// declaration with its value (D2LuaRt_Settings), which is what a settings screen needs.
//
// Scopes, by who reads them:
//   startup   every side, data stage included; fixed for the process
//   game      the server control stage; fixed for the game
//   player    the client control stage: the local player's own value
//
// Natives here follow the host's rule: their frames hold no object with a destructor across a Lua
// call that may raise. The C++ bookkeeping is in helpers that finish (and catch) before any.
#if D2_FEATURE_MODS
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "Redivivus/Lua/d2_lua_internal.h"
#include "Redivivus/Lua/d2_lua_json.h"
#include "Redivivus/Lua/d2_lua_rt.h"
#include "lua.hpp"

static const int kMaxChoices = 64;
static const size_t kMaxString = 256;

void D2LuaRt_LoadSettingsFile(D2LuaRuntime* pRt) {
    pRt->tSettingsFile = D2Json();
    if (pRt->tDesc.sSettingsFile.empty()) {
        return;
    }
    std::string sText;
    if (!D2LuaMods_ReadFile(pRt->tDesc.sSettingsFile, sText)) {
        return;  // no file: every setting has its default
    }
    std::string sError;
    D2Json tFile;
    if (!D2Json_Parse(sText.data(), sText.size(), tFile, sError) || tFile.eType != D2Json::Object) {
        if (pRt->tDesc.pfnLog) {
            pRt->tDesc.pfnLog(("[lua] mod settings file " + pRt->tDesc.sSettingsFile + " is not a JSON object"
                               + (sError.empty() ? std::string() : ": " + sError) + "; every setting has its default")
                                  .c_str());
        }
        return;
    }
    pRt->tSettingsFile = tFile;
}

static const char* const s_apszTypes[] = { "bool", "int", "number", "string", "choice", nullptr };
static const char* const s_apszScopes[] = { "startup", "game", "player", nullptr };

static int IndexOf(const char* const* apsz, const char* psz) {
    for (int i = 0; apsz[i]; i++) {
        if (strcmp(apsz[i], psz) == 0) {
            return i;
        }
    }
    return -1;
}

// A declaration as read off the Lua stack: pointers into strings the stack keeps alive.
struct RawDecl {
    const char* pszName;
    const char* pszType;
    const char* pszScope;
    const char* pszTitle;
    const char* pszDescription;
    int nDefaultType;  // LUA_T*
    bool bDefault;
    double nDefault;
    const char* pszDefault;
    bool bHasMin, bHasMax;
    double nMin, nMax;
    int nChoices;
    const char* apszChoices[kMaxChoices];
};

// Why value v does not fit the declaration, or null.
static const char* CheckValue(const D2LuaSettingInfo& t, const D2Json& v, char* pszWhy, size_t nWhy) {
    if (t.sType == "bool") {
        return v.eType == D2Json::Bool ? nullptr : "not a boolean";
    }
    if (t.sType == "int" || t.sType == "number") {
        if (v.eType != D2Json::Number) {
            return "not a number";
        }
        if (t.sType == "int" && v.nValue != floor(v.nValue)) {
            return "not a whole number";
        }
        if ((t.bHasMin && v.nValue < t.nMin) || (t.bHasMax && v.nValue > t.nMax)) {
            snprintf(pszWhy, nWhy, "%.14g is outside %.14g..%.14g", v.nValue, t.bHasMin ? t.nMin : -HUGE_VAL,
                     t.bHasMax ? t.nMax : HUGE_VAL);
            return pszWhy;
        }
        return nullptr;
    }
    if (v.eType != D2Json::String) {
        return "not a string";
    }
    if (t.sType == "string") {
        return v.sValue.size() <= kMaxString ? nullptr : "longer than 256 bytes";
    }
    for (const std::string& c : t.aChoices) {
        if (c == v.sValue) {
            return nullptr;
        }
    }
    snprintf(pszWhy, nWhy, "'%.64s' is not one of the choices", v.sValue.c_str());
    return pszWhy;
}

// Record the declaration and settle its value. False with pszError on a bad declaration. Plain
// C++, catches its own exceptions.
static bool Declare(D2LuaRuntime* pRt, D2LuaMod* pMod, const RawDecl& d, char* pszError, size_t nError) {
    try {
        for (const D2LuaSettingInfo& t : pRt->aSettings) {
            if (t.sMod == pMod->sName && t.sName == d.pszName && t.nGeneration == pRt->nGeneration) {
                snprintf(pszError, nError, "setting '%s' is declared twice", d.pszName);
                return false;
            }
        }
        // A reload declares the mod's settings again: the new declaration replaces the old one.
        for (size_t i = 0; i < pRt->aSettings.size(); i++) {
            if (pRt->aSettings[i].sMod == pMod->sName && pRt->aSettings[i].sName == d.pszName) {
                pRt->aSettings.erase(pRt->aSettings.begin() + (long)i);
                break;
            }
        }
        D2LuaSettingInfo t;
        t.nGeneration = pRt->nGeneration;
        t.sMod = pMod->sName;
        t.sName = d.pszName;
        t.sType = d.pszType;
        t.sScope = d.pszScope;
        t.sTitle = d.pszTitle ? d.pszTitle : "";
        t.sDescription = d.pszDescription ? d.pszDescription : "";
        t.bHasMin = d.bHasMin;
        t.bHasMax = d.bHasMax;
        t.nMin = d.nMin;
        t.nMax = d.nMax;
        for (int i = 0; i < d.nChoices; i++) {
            for (const std::string& c : t.aChoices) {
                if (c == d.apszChoices[i]) {
                    snprintf(pszError, nError, "setting '%s': choice '%s' is listed twice", d.pszName, d.apszChoices[i]);
                    return false;
                }
            }
            t.aChoices.push_back(d.apszChoices[i]);
        }
        if (t.sType == "choice" && t.aChoices.empty()) {
            snprintf(pszError, nError, "setting '%s': a choice setting needs choices", d.pszName);
            return false;
        }
        if (t.sType != "choice" && !t.aChoices.empty()) {
            snprintf(pszError, nError, "setting '%s': only a choice setting has choices", d.pszName);
            return false;
        }
        if ((d.bHasMin || d.bHasMax) && t.sType != "int" && t.sType != "number") {
            snprintf(pszError, nError, "setting '%s': only int and number settings have a range", d.pszName);
            return false;
        }
        if (d.bHasMin && d.bHasMax && d.nMin > d.nMax) {
            snprintf(pszError, nError, "setting '%s': min is above max", d.pszName);
            return false;
        }
        if (d.nDefaultType == LUA_TBOOLEAN) {
            t.tDefault.eType = D2Json::Bool;
            t.tDefault.bValue = d.bDefault;
        } else if (d.nDefaultType == LUA_TNUMBER) {
            t.tDefault.eType = D2Json::Number;
            t.tDefault.nValue = d.nDefault;
        } else if (d.nDefaultType == LUA_TSTRING) {
            t.tDefault.eType = D2Json::String;
            t.tDefault.sValue = d.pszDefault;
        } else {
            snprintf(pszError, nError, "setting '%s' has no default", d.pszName);
            return false;
        }
        char szWhy[160];
        if (const char* pszWhy = CheckValue(t, t.tDefault, szWhy, sizeof(szWhy))) {
            snprintf(pszError, nError, "setting '%s': bad default: %s", d.pszName, pszWhy);
            return false;
        }
        t.tValue = t.tDefault;
        const D2Json* pModValues = pRt->tSettingsFile.Get(pMod->sName.c_str());
        const D2Json* pValue = pModValues ? pModValues->Get(d.pszName) : nullptr;
        if (pValue) {
            if (const char* pszWhy = CheckValue(t, *pValue, szWhy, sizeof(szWhy))) {
                t.sProblem = pszWhy;
                if (pRt->tDesc.pfnLog) {
                    pRt->tDesc.pfnLog(("[lua] " + t.sMod + ": setting '" + t.sName + "' in the settings file: " + pszWhy
                                       + "; the default is used")
                                          .c_str());
                }
            } else {
                t.tValue = *pValue;
            }
        }
        pRt->aSettings.push_back(t);
        return true;
    } catch (...) {
        snprintf(pszError, nError, "out of memory");
        return false;
    }
}

static const char* OptString(lua_State* L, int nTable, const char* pszField) {
    lua_getfield(L, nTable, pszField);
    const char* psz = lua_type(L, -1) == LUA_TSTRING ? lua_tostring(L, -1) : nullptr;
    bool bBad = !psz && !lua_isnil(L, -1);
    lua_pop(L, 1);  // the string stays alive in the table
    if (bBad) {
        luaL_error(L, "settings: '%s' must be a string", pszField);
    }
    return psz;
}

static bool OptNumber(lua_State* L, int nTable, const char* pszField, double* pnOut) {
    lua_getfield(L, nTable, pszField);
    bool bHas = !lua_isnil(L, -1);
    if (bHas && lua_type(L, -1) != LUA_TNUMBER) {
        luaL_error(L, "settings: '%s' must be a number", pszField);
    }
    *pnOut = bHas ? lua_tonumber(L, -1) : 0;
    lua_pop(L, 1);
    return bHas;
}

// declare(name at nName, { type, scope, default, min?, max?, choices?, title?, description? } after it)
// for mod pMod. The core passes a declaration table it built itself.
int D2LuaSettings_Declare(lua_State* L, D2LuaMod* pMod, int nName) {
    D2LuaRuntime* pRt = pMod->pRt;
    lua_pushvalue(L, nName);
    lua_pushvalue(L, nName + 1);
    lua_replace(L, 2);
    lua_replace(L, 1);
    RawDecl d;
    d.pszName = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    lua_settop(L, 2);
    size_t nNameLen = strlen(d.pszName);
    if (nNameLen == 0 || nNameLen > 64 || strspn(d.pszName, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-") != nNameLen) {
        return luaL_error(L, "settings: '%s' is not a setting name (letters, digits, '_' and '-', at most 64)", d.pszName);
    }
    d.pszType = OptString(L, 2, "type");
    d.pszScope = OptString(L, 2, "scope");
    if (!d.pszType || IndexOf(s_apszTypes, d.pszType) < 0) {
        return luaL_error(L, "settings: setting '%s' has no known type (bool, int, number, string, choice)", d.pszName);
    }
    if (!d.pszScope || IndexOf(s_apszScopes, d.pszScope) < 0) {
        return luaL_error(L, "settings: setting '%s' has no known scope (startup, game, player)", d.pszName);
    }
    d.pszTitle = OptString(L, 2, "title");
    d.pszDescription = OptString(L, 2, "description");
    d.bHasMin = OptNumber(L, 2, "min", &d.nMin);
    d.bHasMax = OptNumber(L, 2, "max", &d.nMax);
    lua_getfield(L, 2, "default");  // 3
    d.nDefaultType = lua_type(L, 3);
    d.bDefault = lua_toboolean(L, 3) != 0;
    d.nDefault = lua_tonumber(L, 3);
    d.pszDefault = lua_type(L, 3) == LUA_TSTRING ? lua_tostring(L, 3) : nullptr;
    bool bTypeOk = (strcmp(d.pszType, "bool") == 0 && d.nDefaultType == LUA_TBOOLEAN)
                   || ((strcmp(d.pszType, "int") == 0 || strcmp(d.pszType, "number") == 0) && d.nDefaultType == LUA_TNUMBER)
                   || ((strcmp(d.pszType, "string") == 0 || strcmp(d.pszType, "choice") == 0) && d.nDefaultType == LUA_TSTRING);
    if (!bTypeOk) {
        return luaL_error(L, "settings: setting '%s': the default is a %s, not a %s value", d.pszName,
                          luaL_typename(L, 3), d.pszType);
    }
    lua_getfield(L, 2, "choices");  // 4
    d.nChoices = 0;
    if (!lua_isnil(L, 4)) {
        if (!lua_istable(L, 4)) {
            return luaL_error(L, "settings: setting '%s': choices must be a list of strings", d.pszName);
        }
        int nCount = (int)lua_objlen(L, 4);
        if (nCount > kMaxChoices) {
            return luaL_error(L, "settings: setting '%s' has more than %d choices", d.pszName, kMaxChoices);
        }
        for (int i = 1; i <= nCount; i++) {
            lua_rawgeti(L, 4, i);
            if (lua_type(L, -1) != LUA_TSTRING) {
                return luaL_error(L, "settings: setting '%s': choices must be a list of strings", d.pszName);
            }
            d.apszChoices[d.nChoices++] = lua_tostring(L, -1);  // kept alive by the table at 4
            lua_pop(L, 1);
        }
    }
    char szError[256];
    if (!Declare(pRt, pMod, d, szError, sizeof(szError))) {
        return luaL_error(L, "settings: %s", szError);
    }
    return 0;
}

static bool ScopeReadable(const std::string& sScope, int nSide) {
    if (sScope == "startup") {
        return true;
    }
    if (sScope == "game") {
        return nSide == D2LUA_SIDE_SERVER;
    }
    return nSide == D2LUA_SIDE_CLIENT;
}

static void PushJson(lua_State* L, const D2Json& v) {
    switch (v.eType) {
        case D2Json::Bool: lua_pushboolean(L, v.bValue); break;
        case D2Json::Number: lua_pushnumber(L, v.nValue); break;
        case D2Json::String: lua_pushlstring(L, v.sValue.data(), v.sValue.size()); break;
        default: lua_pushnil(L); break;
    }
}

// get(name at nName): mod pMod's value of the setting.
int D2LuaSettings_Get(lua_State* L, D2LuaMod* pMod, int nName) {
    const char* pszName = luaL_checkstring(L, nName);
    for (const D2LuaSettingInfo& t : pMod->pRt->aSettings) {
        if (t.sMod != pMod->sName || t.sName != pszName) {
            continue;
        }
        if (!ScopeReadable(t.sScope, pMod->pRt->tDesc.nSide)) {
            const char* pszWhere = t.sScope == "game" ? "the server" : "the client";
            return luaL_error(L, "setting '%s' is a %s setting; it is read on %s", pszName, t.sScope.c_str(), pszWhere);
        }
        PushJson(L, t.tValue);
        return 1;
    }
    return luaL_error(L, "setting '%s' is not declared by mod '%s'", pszName, pMod->sName.c_str());
}

// list(): every setting declared in this state, all mods, in declaration order.
int D2LuaSettings_List(lua_State* L) {
    const std::vector<D2LuaSettingInfo>& a = D2LuaRt_FromState(L)->aSettings;
    lua_createtable(L, (int)a.size(), 0);
    for (size_t i = 0; i < a.size(); i++) {
        const D2LuaSettingInfo& t = a[i];
        lua_createtable(L, 0, 10);
        lua_pushstring(L, t.sMod.c_str());
        lua_setfield(L, -2, "mod");
        lua_pushstring(L, t.sName.c_str());
        lua_setfield(L, -2, "name");
        lua_pushstring(L, t.sType.c_str());
        lua_setfield(L, -2, "type");
        lua_pushstring(L, t.sScope.c_str());
        lua_setfield(L, -2, "scope");
        PushJson(L, t.tDefault);
        lua_setfield(L, -2, "default");
        PushJson(L, t.tValue);
        lua_setfield(L, -2, "value");
        if (t.bHasMin) {
            lua_pushnumber(L, t.nMin);
            lua_setfield(L, -2, "min");
        }
        if (t.bHasMax) {
            lua_pushnumber(L, t.nMax);
            lua_setfield(L, -2, "max");
        }
        if (!t.aChoices.empty()) {
            lua_createtable(L, (int)t.aChoices.size(), 0);
            for (size_t c = 0; c < t.aChoices.size(); c++) {
                lua_pushstring(L, t.aChoices[c].c_str());
                lua_rawseti(L, -2, (int)c + 1);
            }
            lua_setfield(L, -2, "choices");
        }
        if (!t.sTitle.empty()) {
            lua_pushstring(L, t.sTitle.c_str());
            lua_setfield(L, -2, "title");
        }
        if (!t.sDescription.empty()) {
            lua_pushstring(L, t.sDescription.c_str());
            lua_setfield(L, -2, "description");
        }
        lua_rawseti(L, -2, (int)i + 1);
    }
    return 1;
}

const std::vector<D2LuaSettingInfo>& D2LuaRt_Settings(D2LuaRuntime* pRt) {
    return pRt->aSettings;
}
#endif
