// See d2_lua_boot.h. Plain C++ around the runtime; every line it logs starts with "[mods]".
#if D2_FEATURE_MODS
#include "Redivivus/Lua/d2_lua_boot.h"

#include <stdio.h>

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
    Log(tDesc, std::to_string(tOut.aFound.size()) + " mod(s) found");
    for (const D2LuaModInfo& t : tOut.aFound) {
        Log(tDesc, "  " + t.sName + " " + t.sVersion + (t.sTitle.empty() ? "" : " \"" + t.sTitle + "\"") + " in " + t.sPath);
    }
    if (tOut.aFound.empty()) {
        return;
    }
    std::vector<std::string> aDisabled;
    D2LuaMods_ResolveEnabled(tOut.aFound, tOut.aOrder, aDisabled);
    for (const std::string& sLine : aDisabled) {
        Log(tDesc, "error: " + sLine);
        tOut.aErrors.push_back(sLine);
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
    for (const D2LuaModInfo& t : tOut.aOrder) {
        if (D2LuaRt_IsFaulted(tOut.pSettings, t.sName.c_str())) {
            Log(tDesc, "error: mod '" + t.sName + "' faulted in the settings stage");
        }
    }
    Log(tDesc, "not run yet: the data stage (data.lua, data-updates.lua, data-final-fixes.lua) and the server and client "
               "control stages; they wait for their engine hooks");
}

void D2LuaBoot_Close(D2LuaBootResult& tResult) {
    D2LuaRt_Destroy(tResult.pSettings);
    tResult.pSettings = nullptr;
}
#endif
