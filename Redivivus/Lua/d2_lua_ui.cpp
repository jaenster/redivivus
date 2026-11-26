// See d2_lua_ui.h. The natives here follow the host's rule: no object with a destructor lives in a
// frame across a Lua call that may raise. File work is done by helpers that finish first and leave
// their result in a per-thread scratch string.
#if D2_FEATURE_MODS
#include "Redivivus/Lua/d2_lua_ui.h"

#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <set>

#include "Redivivus/Lua/d2_lua_json.h"
#include "Redivivus/Lua/d2_lua_rt.h"
#include "lua.hpp"

// --- the mod list -----------------------------------------------------------------------------------

bool D2LuaModList_Read(const std::string& sFile, std::map<std::string, bool>& tOut, std::string& sError) {
    tOut.clear();
    std::string sText;
    if (!D2LuaMods_ReadFile(sFile, sText)) {
        return true;  // no file: every mod is enabled
    }
    D2Json tRoot;
    std::string sJsonError;
    if (!D2Json_Parse(sText.data(), sText.size(), tRoot, sJsonError)) {
        sError = "mod-list.json: " + sJsonError;
        return false;
    }
    const D2Json* pMods = tRoot.eType == D2Json::Object ? tRoot.Get("mods") : nullptr;
    if (!pMods || pMods->eType != D2Json::Array) {
        sError = "mod-list.json: expected { \"mods\": [ { \"name\": ..., \"enabled\": ... } ] }";
        return false;
    }
    for (const D2Json& tEntry : pMods->aItems) {
        const D2Json* pName = tEntry.eType == D2Json::Object ? tEntry.Get("name") : nullptr;
        const D2Json* pEnabled = tEntry.eType == D2Json::Object ? tEntry.Get("enabled") : nullptr;
        if (!pName || pName->eType != D2Json::String || !pEnabled || pEnabled->eType != D2Json::Bool) {
            sError = "mod-list.json: every entry needs a string \"name\" and a boolean \"enabled\"";
            return false;
        }
        tOut[pName->sValue] = pEnabled->bValue;
    }
    return true;
}

bool D2LuaModList_Write(const std::string& sFile, const std::map<std::string, bool>& tChanges, std::string& sError) {
    try {
        std::map<std::string, bool> tList;
        if (!D2LuaModList_Read(sFile, tList, sError)) {
            tList.clear();  // a broken file is replaced
        }
        for (const auto& t : tChanges) {
            if (!D2LuaMods_IsValidName(t.first)) {
                sError = "'" + t.first + "' is not a mod name";
                return false;
            }
            tList[t.first] = t.second;
        }
        std::string sText = "{\n  \"mods\": [";
        bool bFirst = true;
        for (const auto& t : tList) {
            sText += bFirst ? "\n" : ",\n";
            sText += "    { \"name\": \"" + t.first + "\", \"enabled\": " + (t.second ? "true" : "false") + " }";
            bFirst = false;
        }
        sText += bFirst ? "]\n}\n" : "\n  ]\n}\n";
        std::string sTemp = sFile + ".tmp";
        FILE* pFile = fopen(sTemp.c_str(), "wb");
        if (!pFile) {
            sError = "cannot write " + sTemp;
            return false;
        }
        bool bOk = fwrite(sText.data(), 1, sText.size(), pFile) == sText.size();
        bOk = fclose(pFile) == 0 && bOk;
        if (bOk) {
            remove(sFile.c_str());
            bOk = rename(sTemp.c_str(), sFile.c_str()) == 0;
        }
        if (!bOk) {
            remove(sTemp.c_str());
            sError = "cannot write " + sFile;
        }
        return bOk;
    } catch (...) {
        sError = "out of memory";
        return false;
    }
}

// --- the catalogue ------------------------------------------------------------------------------------

static const D2LuaModInfo* FindInfo(const std::vector<D2LuaModInfo>& a, const std::string& sName) {
    for (const D2LuaModInfo& t : a) {
        if (t.sName == sName) {
            return &t;
        }
    }
    return nullptr;
}

static void FillDependencies(D2LuaCatalogMod& tMod, const std::vector<D2LuaModInfo>& aFound) {
    D2LuaModInfo tIntrinsic = D2LuaMods_Intrinsic();
    for (const D2LuaDependency& tDep : tMod.tInfo.aDependencies) {
        D2LuaCatalogDependency t;
        t.sSpec = tDep.sText.empty() ? tDep.sName : tDep.sText;
        t.sName = tDep.sName;
        const D2LuaModInfo* pHave = tDep.sName == D2LUA_INTRINSIC_MOD ? &tIntrinsic : FindInfo(aFound, tDep.sName);
        t.sInstalled = pHave ? pHave->sVersion : "";
        bool bOptional = tDep.eKind == D2LuaDepKind::Optional || tDep.eKind == D2LuaDepKind::HiddenOptional;
        if (tDep.eKind == D2LuaDepKind::HiddenOptional) {
            continue;  // not shown to players
        }
        if (tDep.eKind == D2LuaDepKind::Incompatible) {
            t.sState = pHave ? "incompatible" : "ok";
        } else if (!pHave) {
            t.sState = bOptional ? "optional" : "missing";
        } else if (!D2LuaMods_Satisfies(tDep, pHave->tVersion)) {
            t.sState = "version";
        } else {
            t.sState = bOptional ? "optional" : "ok";
        }
        tMod.aDependencies.push_back(t);
    }
}

void D2LuaCatalog_Fill(D2LuaCatalog& tOut, const std::vector<D2LuaModInfo>& aFound, const std::vector<D2LuaModInfo>& aOrder,
                       const std::map<std::string, bool>& tModList, const std::map<std::string, std::string>& tFaulted,
                       const std::map<std::string, std::string>& tUnresolved) {
    tOut.aMods.clear();
    std::set<std::string> tDone;
    std::vector<const D2LuaModInfo*> aSorted;
    for (const D2LuaModInfo& t : aOrder) {
        if (!t.bIntrinsic) {
            aSorted.push_back(&t);
            tDone.insert(t.sName);
        }
    }
    std::vector<const D2LuaModInfo*> aRest;
    for (const D2LuaModInfo& t : aFound) {
        if (!t.bIntrinsic && !tDone.count(t.sName)) {
            aRest.push_back(&t);
        }
    }
    for (size_t i = 1; i < aRest.size(); i++) {
        for (size_t j = i; j > 0 && aRest[j]->sName < aRest[j - 1]->sName; j--) {
            const D2LuaModInfo* p = aRest[j];
            aRest[j] = aRest[j - 1];
            aRest[j - 1] = p;
        }
    }
    aSorted.insert(aSorted.end(), aRest.begin(), aRest.end());
    for (const D2LuaModInfo* pInfo : aSorted) {
        D2LuaCatalogMod tMod;
        tMod.tInfo = *pInfo;
        auto itList = tModList.find(pInfo->sName);
        tMod.bEnabled = itList == tModList.end() || itList->second;
        auto itFault = tFaulted.find(pInfo->sName);
        auto itUnresolved = tUnresolved.find(pInfo->sName);
        if (!tMod.bEnabled) {
            tMod.sState = D2LUA_MODSTATE_DISABLED;
        } else if (itFault != tFaulted.end()) {
            tMod.sState = D2LUA_MODSTATE_FAULTED;
            tMod.sError = itFault->second;
        } else if (itUnresolved != tUnresolved.end()) {
            tMod.sState = D2LUA_MODSTATE_MISSING_DEPENDENCY;
            tMod.sError = itUnresolved->second;
        } else if (tDone.count(pInfo->sName)) {
            tMod.sState = D2LUA_MODSTATE_ENABLED;
        } else {
            tMod.sState = D2LUA_MODSTATE_FAULTED;
            tMod.sError = "it did not load";
        }
        FillDependencies(tMod, aFound);
        tOut.aMods.push_back(tMod);
    }
}

// --- the drawing natives ---------------------------------------------------------------------------

void D2LuaUi_SetDrawing(D2LuaRuntime* pRt, bool bDrawing) {
    if (pRt) {
        pRt->bUiDrawing = bDrawing;
    }
}

static D2LuaUi* Ui(lua_State* L) {
    D2LuaRuntime* pRt = D2LuaRt_FromState(L);
    return pRt ? pRt->tDesc.pUi : nullptr;
}

static bool Drawing(lua_State* L) {
    D2LuaRuntime* pRt = D2LuaRt_FromState(L);
    return pRt && pRt->tDesc.pUi && pRt->bUiDrawing;
}

// An integer argument: a finite number, rounded toward minus infinity, clamped to a range no
// layout reaches, so no arithmetic on it overflows.
static int IntArg(lua_State* L, int i) {
    double n = luaL_checknumber(L, i);
    if (!(n == n)) {
        luaL_argerror(L, i, "not a number");
    }
    n = floor(n);
    if (n > 1e6) {
        n = 1e6;
    } else if (n < -1e6) {
        n = -1e6;
    }
    return (int)n;
}

static int N_loadSprite(lua_State* L) {
    size_t nLen = 0;
    const char* pszPath = luaL_checklstring(L, 1, &nLen);
    if (nLen == 0 || nLen > 200 || strlen(pszPath) != nLen) {
        lua_pushinteger(L, 0);
        return 1;
    }
    D2LuaUi* p = Ui(L);
    lua_pushinteger(L, p ? p->LoadSprite(pszPath) : 0);
    return 1;
}

static int N_spriteFrames(lua_State* L) {
    int h = IntArg(L, 1);
    D2LuaUi* p = Ui(L);
    lua_pushinteger(L, p && h > 0 ? p->SpriteFrames(h) : 0);
    return 1;
}

static int N_spriteWidth(lua_State* L) {
    int h = IntArg(L, 1);
    int f = IntArg(L, 2);
    D2LuaUi* p = Ui(L);
    lua_pushinteger(L, p && h > 0 ? p->SpriteWidth(h, f) : 0);
    return 1;
}

static int N_spriteHeight(lua_State* L) {
    int h = IntArg(L, 1);
    int f = IntArg(L, 2);
    D2LuaUi* p = Ui(L);
    lua_pushinteger(L, p && h > 0 ? p->SpriteHeight(h, f) : 0);
    return 1;
}

static int N_loadFont(lua_State* L) {
    size_t nLen = 0;
    const char* pszName = luaL_checklstring(L, 1, &nLen);
    D2LuaUi* p = Ui(L);
    lua_pushinteger(L, p && nLen > 0 && nLen < 64 ? p->LoadFont(pszName) : 0);
    return 1;
}

static int N_fontLineHeight(lua_State* L) {
    int f = IntArg(L, 1);
    D2LuaUi* p = Ui(L);
    lua_pushinteger(L, p && f > 0 ? p->FontLineHeight(f) : 0);
    return 1;
}

static int N_fontAscent(lua_State* L) {
    int f = IntArg(L, 1);
    D2LuaUi* p = Ui(L);
    lua_pushinteger(L, p && f > 0 ? p->FontAscent(f) : 0);
    return 1;
}

static int N_hasGlyph(lua_State* L) {
    int f = IntArg(L, 1);
    int c = IntArg(L, 2);
    D2LuaUi* p = Ui(L);
    lua_pushboolean(L, p && f > 0 && c >= 0 && c < 256 && p->HasGlyph(f, c));
    return 1;
}

static int N_measureText(lua_State* L) {
    int f = IntArg(L, 1);
    size_t nLen = 0;
    const char* psz = luaL_checklstring(L, 2, &nLen);
    D2LuaUi* p = Ui(L);
    lua_pushinteger(L, p && f > 0 ? p->MeasureText(f, psz, nLen) : 0);
    return 1;
}

static int N_beginFrame(lua_State* L) {
    int w = IntArg(L, 1);
    int h = IntArg(L, 2);
    if (Drawing(L)) {
        Ui(L)->BeginFrame(w, h);
    }
    return 0;
}

static int N_endFrame(lua_State* L) {
    if (Drawing(L)) {
        Ui(L)->EndFrame();
    }
    return 0;
}

static int N_drawSprite(lua_State* L) {
    int h = IntArg(L, 1);
    int f = IntArg(L, 2);
    int x = IntArg(L, 3);
    int y = IntArg(L, 4);
    int m = IntArg(L, 5);
    if (Drawing(L) && h > 0) {
        Ui(L)->DrawSprite(h, f, x, y, m);
    }
    return 0;
}

static int N_drawNineSlice(lua_State* L) {
    int h = IntArg(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int anFrames[9];
    for (int i = 0; i < 9; i++) {
        lua_rawgeti(L, 2, i + 1);
        anFrames[i] = lua_isnumber(L, -1) ? (int)floor(lua_tonumber(L, -1)) : -1;
        lua_pop(L, 1);
    }
    int x = IntArg(L, 3);
    int y = IntArg(L, 4);
    int w = IntArg(L, 5);
    int hh = IntArg(L, 6);
    int m = IntArg(L, 7);
    if (Drawing(L) && h > 0) {
        Ui(L)->DrawNineSlice(h, anFrames, x, y, w, hh, m);
    }
    return 0;
}

static uint32_t RgbArg(lua_State* L, int i) {
    double n = luaL_checknumber(L, i);
    if (!(n >= 0 && n <= 16777215)) {
        return 0;
    }
    return (uint32_t)n;
}

static int AlphaArg(lua_State* L, int i) {
    int a = IntArg(L, i);
    return a < 0 ? 0 : a > 255 ? 255 : a;
}

static int N_fillRect(lua_State* L) {
    int x = IntArg(L, 1);
    int y = IntArg(L, 2);
    int w = IntArg(L, 3);
    int h = IntArg(L, 4);
    uint32_t c = RgbArg(L, 5);
    int a = AlphaArg(L, 6);
    if (Drawing(L) && w > 0 && h > 0) {
        Ui(L)->FillRect(x, y, w, h, c, a);
    }
    return 0;
}

static int N_drawLine(lua_State* L) {
    int x0 = IntArg(L, 1);
    int y0 = IntArg(L, 2);
    int x1 = IntArg(L, 3);
    int y1 = IntArg(L, 4);
    uint32_t c = RgbArg(L, 5);
    int a = AlphaArg(L, 6);
    if (Drawing(L)) {
        Ui(L)->DrawLine(x0, y0, x1, y1, c, a);
    }
    return 0;
}

static int N_drawText(lua_State* L) {
    int f = IntArg(L, 1);
    size_t nLen = 0;
    const char* psz = luaL_checklstring(L, 2, &nLen);
    int x = IntArg(L, 3);
    int y = IntArg(L, 4);
    int c = IntArg(L, 5);
    if (Drawing(L) && f > 0 && nLen > 0) {
        Ui(L)->DrawText(f, psz, nLen > 1024 ? 1024 : nLen, x, y, c);
    }
    return 0;
}

static int N_pushScissor(lua_State* L) {
    int x = IntArg(L, 1);
    int y = IntArg(L, 2);
    int w = IntArg(L, 3);
    int h = IntArg(L, 4);
    if (Drawing(L)) {
        Ui(L)->PushScissor(x, y, w, h);
    }
    return 0;
}

static int N_popScissor(lua_State* L) {
    if (Drawing(L)) {
        Ui(L)->PopScissor();
    }
    return 0;
}

static const luaL_Reg s_aUiNatives[] = {
    { "loadSprite", N_loadSprite },
    { "spriteFrames", N_spriteFrames },
    { "spriteWidth", N_spriteWidth },
    { "spriteHeight", N_spriteHeight },
    { "loadFont", N_loadFont },
    { "fontLineHeight", N_fontLineHeight },
    { "fontAscent", N_fontAscent },
    { "hasGlyph", N_hasGlyph },
    { "measureText", N_measureText },
    { "beginFrame", N_beginFrame },
    { "endFrame", N_endFrame },
    { "drawSprite", N_drawSprite },
    { "drawNineSlice", N_drawNineSlice },
    { "fillRect", N_fillRect },
    { "drawLine", N_drawLine },
    { "drawText", N_drawText },
    { "pushScissor", N_pushScissor },
    { "popScissor", N_popScissor },
    { nullptr, nullptr },
};

// --- the catalogue natives -----------------------------------------------------------------------------

static const D2LuaCatalog* Catalog(lua_State* L) {
    D2LuaRuntime* pRt = D2LuaRt_FromState(L);
    return pRt ? pRt->tDesc.pCatalog : nullptr;
}

static void SetString(lua_State* L, const char* pszKey, const std::string& s) {
    lua_pushlstring(L, s.data(), s.size());
    lua_setfield(L, -2, pszKey);
}

// mods(): every installed mod, as plain tables.
static int N_mods(lua_State* L) {
    const D2LuaCatalog* pCat = Catalog(L);
    lua_newtable(L);
    if (!pCat) {
        return 1;
    }
    int n = 0;
    for (const D2LuaCatalogMod& t : pCat->aMods) {
        lua_createtable(L, 0, 10);
        SetString(L, "name", t.tInfo.sName);
        SetString(L, "title", t.tInfo.sTitle.empty() ? t.tInfo.sName : t.tInfo.sTitle);
        SetString(L, "version", t.tInfo.sVersion);
        SetString(L, "author", t.tInfo.sAuthor);
        SetString(L, "licence", t.tInfo.sLicence);
        lua_pushboolean(L, t.bEnabled);
        lua_setfield(L, -2, "enabled");
        SetString(L, "state", t.sState);
        SetString(L, "error", t.sError);
        lua_createtable(L, (int)t.aDependencies.size(), 0);
        int d = 0;
        for (const D2LuaCatalogDependency& tDep : t.aDependencies) {
            lua_createtable(L, 0, 4);
            SetString(L, "spec", tDep.sSpec);
            SetString(L, "name", tDep.sName);
            SetString(L, "state", tDep.sState);
            SetString(L, "installed", tDep.sInstalled);
            lua_rawseti(L, -2, ++d);
        }
        lua_setfield(L, -2, "dependencies");
        lua_rawseti(L, -2, ++n);
    }
    return 1;
}

static void PushJsonValue(lua_State* L, const D2Json& v) {
    switch (v.eType) {
    case D2Json::Bool: lua_pushboolean(L, v.bValue); break;
    case D2Json::Number: lua_pushnumber(L, v.nValue); break;
    case D2Json::String: lua_pushlstring(L, v.sValue.data(), v.sValue.size()); break;
    default: lua_pushnil(L); break;
    }
}

// settings(): every setting the settings stage declared, with its value.
static int N_settings(lua_State* L) {
    const D2LuaCatalog* pCat = Catalog(L);
    lua_newtable(L);
    if (!pCat) {
        return 1;
    }
    int n = 0;
    for (const D2LuaSettingInfo& t : pCat->aSettings) {
        lua_createtable(L, 0, 6);
        SetString(L, "mod", t.sMod);
        SetString(L, "name", t.sName);
        SetString(L, "title", t.sTitle.empty() ? t.sName : t.sTitle);
        SetString(L, "type", t.sType);
        SetString(L, "scope", t.sScope);
        PushJsonValue(L, t.tValue);
        lua_setfield(L, -2, "value");
        lua_rawseti(L, -2, ++n);
    }
    return 1;
}

static bool IsFile(const std::string& sPath) {
    struct stat tStat;
    return stat(sPath.c_str(), &tStat) == 0 && S_ISREG(tStat.st_mode);
}

// A file directly in an installed mod's folder, its name matched without case, read into the
// scratch string (at most 256 KiB). Plain C++; catches its own exceptions.
static thread_local std::string t_sScratch;

static bool ReadModFile(const D2LuaCatalog* pCat, const char* pszMod, const char* pszFile) {
    try {
        t_sScratch.clear();
        if (!pCat) {
            return false;
        }
        const D2LuaCatalogMod* pMod = nullptr;
        for (const D2LuaCatalogMod& t : pCat->aMods) {
            if (t.tInfo.sName == pszMod) {
                pMod = &t;
            }
        }
        if (!pMod || pMod->tInfo.sPath.empty()) {
            return false;
        }
        std::string sFound;
        if (DIR* pDir = opendir(pMod->tInfo.sPath.c_str())) {
            while (struct dirent* pEntry = readdir(pDir)) {
                const char* a = pEntry->d_name;
                const char* b = pszFile;
                while (*a && *b && ((*a | 0x20) == (*b | 0x20))) {
                    a++;
                    b++;
                }
                if (!*a && !*b) {
                    sFound = pEntry->d_name;
                    break;
                }
            }
            closedir(pDir);
        }
        std::string sPath = pMod->tInfo.sPath + "/" + sFound;
        if (sFound.empty() || !IsFile(sPath) || !D2LuaMods_ReadFile(sPath, t_sScratch)) {
            t_sScratch.clear();
            return false;
        }
        if (t_sScratch.size() > 256 * 1024) {
            t_sScratch.resize(256 * 1024);
        }
        return true;
    } catch (...) {
        t_sScratch.clear();
        return false;
    }
}

static int PushModFile(lua_State* L, const char* pszFile) {
    const char* pszMod = luaL_checkstring(L, 1);
    ReadModFile(Catalog(L), pszMod, pszFile);
    lua_pushlstring(L, t_sScratch.data(), t_sScratch.size());
    t_sScratch.clear();
    return 1;
}

static int N_readme(lua_State* L) {
    return PushModFile(L, "README.md");
}

static int N_changelog(lua_State* L) {
    return PushModFile(L, "CHANGELOG.md");
}

// writeModList({ [name] = enabled }): true, or false and why.
static thread_local std::map<std::string, bool> t_tChanges;
static thread_local std::string t_sError;

static bool WriteModList(const D2LuaCatalog* pCat) {
    try {
        t_sError.clear();
        if (!pCat || pCat->sModListFile.empty()) {
            t_sError = "this game has no mod list";
            return false;
        }
        bool bOk = D2LuaModList_Write(pCat->sModListFile, t_tChanges, t_sError);
        t_tChanges.clear();
        return bOk;
    } catch (...) {
        t_sError = "out of memory";
        return false;
    }
}

static bool CollectChange(const char* pszName, bool bEnabled) {
    try {
        t_tChanges[pszName] = bEnabled;
        return true;
    } catch (...) {
        return false;
    }
}

static int PushResult(lua_State* L, bool bOk) {
    lua_pushboolean(L, bOk);
    if (bOk) {
        t_sError.clear();
        return 1;
    }
    lua_pushlstring(L, t_sError.data(), t_sError.size());
    t_sError.clear();
    return 2;
}

static int N_writeModList(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    t_tChanges.clear();
    lua_pushnil(L);
    while (lua_next(L, 1) != 0) {
        if (lua_type(L, -2) != LUA_TSTRING || lua_type(L, -1) != LUA_TBOOLEAN) {
            t_tChanges.clear();
            return luaL_error(L, "the changes map mod names to booleans");
        }
        if (!CollectChange(lua_tostring(L, -2), lua_toboolean(L, -1) != 0)) {
            t_tChanges.clear();
            return luaL_error(L, "out of memory");
        }
        lua_pop(L, 1);
    }
    return PushResult(L, WriteModList(Catalog(L)));
}

static bool Restart(const D2LuaCatalog* pCat) {
    try {
        t_sError.clear();
        if (!pCat || !pCat->pfnRestart) {
            t_sError = "this game cannot restart itself; restart it by hand";
            return false;
        }
        return pCat->pfnRestart(pCat->pUser, t_sError);
    } catch (...) {
        t_sError = "out of memory";
        return false;
    }
}

static int N_restart(lua_State* L) {
    return PushResult(L, Restart(Catalog(L)));
}

static const luaL_Reg s_aCatalogNatives[] = {
    { "mods", N_mods },
    { "settings", N_settings },
    { "readme", N_readme },
    { "changelog", N_changelog },
    { "writeModList", N_writeModList },
    { "restart", N_restart },
    { nullptr, nullptr },
};

void D2LuaUi_Natives(lua_State* L, int nTable, D2LuaRuntime* pRt) {
    if ((pRt->tDesc.nSide & D2LUA_SIDE_CLIENT) && pRt->tDesc.pUi) {
        D2LuaApi_PushNatives(L, s_aUiNatives);
        lua_setfield(L, nTable, "ui");
    }
    D2LuaApi_PushNatives(L, s_aCatalogNatives);
    lua_setfield(L, nTable, "catalog");
}
#endif
