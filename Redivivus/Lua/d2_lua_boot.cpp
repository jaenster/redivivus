// See d2_lua_boot.h. Plain C++ around the runtime; every line it logs starts with "[mods]".
#if D2_FEATURE_MODS
#include "Redivivus/Lua/d2_lua_boot.h"

#include <stdio.h>

#include <map>

static void Log(const D2LuaBootDesc& tDesc, const std::string& sLine) {
    if (tDesc.pfnLog) {
        tDesc.pfnLog(("[mods] " + sLine).c_str());
    }
}

static std::string JsonText(const D2Json& v) {
    char sz[64];
    switch (v.eType) {
        case D2Json::Bool: return v.bValue ? "true" : "false";
        case D2Json::Number: snprintf(sz, sizeof(sz), "%.14g", v.nValue); return sz;
        case D2Json::String: return "\"" + v.sValue + "\"";
        default: return "nil";
    }
}

void D2LuaBoot_Run(const D2LuaBootDesc& tDesc, D2LuaBootResult& tOut) {
    tOut = D2LuaBootResult();
    Log(tDesc, std::string(D2LuaHost_Version()) + ", API " + D2LUA_REDIVIVUS_VERSION);
    for (const std::string& sRoot : tDesc.aRoots) {
        Log(tDesc, "looking for mods in " + sRoot);
    }
    std::vector<std::string> aDiscovery;
    D2LuaMods_Discover(tDesc.aRoots, tOut.aFound, aDiscovery);
    for (const std::string& sError : aDiscovery) {
        Log(tDesc, "error: " + sError);
        tOut.aErrors.push_back(sError);
    }
    Log(tDesc, std::string(D2LUA_INTRINSIC_MOD) + " " + D2LUA_REDIVIVUS_VERSION + " is built in");
    Log(tDesc, std::to_string(tOut.aFound.size()) + " mod(s) found");
    for (const D2LuaModInfo& t : tOut.aFound) {
        Log(tDesc, "  " + t.sName + " " + t.sVersion + (t.sTitle.empty() ? "" : " \"" + t.sTitle + "\"") + " in " + t.sPath);
    }
    tOut.tCatalog.sModListFile = tDesc.sModListFile;
    std::map<std::string, bool> tModList;
    std::map<std::string, std::string> tFaulted, tUnresolved;
    // The catalogue is filled from whatever the boot got to, on every way out of this function.
    struct Fill {
        D2LuaBootResult& t;
        std::map<std::string, bool>& tList;
        std::map<std::string, std::string>& tFault;
        std::map<std::string, std::string>& tUnres;
        ~Fill() {
            D2LuaCatalog_Fill(t.tCatalog, t.aFound, t.aOrder, tList, tFault, tUnres);
            if (t.pSettings) {
                t.tCatalog.aSettings = D2LuaRt_Settings(t.pSettings);
            }
        }
    } tFill{ tOut, tModList, tFaulted, tUnresolved };
    if (tOut.aFound.empty()) {
        return;
    }
    if (!tDesc.sModListFile.empty()) {
        std::string sError;
        if (!D2LuaModList_Read(tDesc.sModListFile, tModList, sError)) {
            Log(tDesc, "error: " + sError + "; every mod is on");
            tOut.aErrors.push_back(sError);
            tModList.clear();
        }
    }
    std::vector<D2LuaModInfo> aOn;
    for (const D2LuaModInfo& t : tOut.aFound) {
        auto it = tModList.find(t.sName);
        if (it != tModList.end() && !it->second) {
            Log(tDesc, "mod '" + t.sName + "' is off in the mod list");
        } else {
            aOn.push_back(t);
        }
    }
    std::vector<std::string> aDisabled;
    D2LuaMods_ResolveEnabled(aOn, tOut.aOrder, aDisabled);
    for (const std::string& sLine : aDisabled) {
        Log(tDesc, "error: " + sLine);
        tOut.aErrors.push_back(sLine);
        // "mod 'name' is disabled: why"
        size_t nOpen = sLine.find('\'');
        size_t nClose = nOpen == std::string::npos ? nOpen : sLine.find('\'', nOpen + 1);
        size_t nWhy = sLine.find(": ");
        if (nClose != std::string::npos && nWhy != std::string::npos) {
            tUnresolved[sLine.substr(nOpen + 1, nClose - nOpen - 1)] = sLine.substr(nWhy + 2);
        }
    }
    std::string sOrder;
    for (const D2LuaModInfo& t : tOut.aOrder) {
        sOrder += (sOrder.empty() ? "" : ", ") + t.sName;
    }
    Log(tDesc, "load order: " + (sOrder.empty() ? std::string("(none)") : sOrder));
    if (tOut.aOrder.empty()) {
        return;
    }

    D2LuaRuntimeDesc tRt;
    tRt.nSide = D2LUA_SIDE_DATA;
    tRt.aMods = tOut.aOrder;
    tRt.aApis = tDesc.aApis;
    tRt.pfnLog = tDesc.pfnLog;
    tRt.tLimits = tDesc.tLimits;
    tRt.sSettingsFile = tDesc.sSettingsFile;
    tRt.bSettingsStageOnly = true;
    tOut.pSettings = D2LuaRt_Create(tRt);
    if (!tOut.pSettings) {
        Log(tDesc, "error: the settings stage could not start");
        return;
    }
    const std::vector<D2LuaSettingInfo>& aSettings = D2LuaRt_Settings(tOut.pSettings);
    Log(tDesc, "settings stage ran: " + std::to_string(aSettings.size()) + " setting(s) declared");
    for (const D2LuaSettingInfo& t : aSettings) {
        Log(tDesc, "  " + t.sMod + "." + t.sName + " = " + JsonText(t.tValue) + " (" + t.sType + ", " + t.sScope
                       + (t.sProblem.empty() ? std::string() : "; the file's value was refused: " + t.sProblem) + ")");
    }
    // A mod that failed in the settings stage (an error, or out of time or memory) is disabled for
    // the session, as one with a broken dependency is.
    std::vector<D2LuaModInfo> aKept;
    for (const D2LuaModInfo& t : tOut.aOrder) {
        if (t.sName != D2LUA_INTRINSIC_MOD && D2LuaRt_IsFaulted(tOut.pSettings, t.sName.c_str())) {
            std::string sLine = "mod '" + t.sName + "' is disabled: it failed in the settings stage";
            Log(tDesc, "error: " + sLine);
            tOut.aErrors.push_back(sLine);
            tFaulted[t.sName] = "it failed in the settings stage (see the game log)";
        } else {
            aKept.push_back(t);
        }
    }
    tOut.aOrder.swap(aKept);
    Log(tDesc, "not run yet: the data stage (data.lua, data-updates.lua, data-final-fixes.lua) and the server and client "
               "control stages; they wait for their engine hooks");
}

void D2LuaBoot_Close(D2LuaBootResult& tResult) {
    D2LuaRt_Destroy(tResult.pSettings);
    tResult.pSettings = nullptr;
}
#endif
