// Mods tied to the game: discovered under mods/ beside the executable at boot, with every line the
// runtime logs going to the game's own log (Fog's log manager, the default "D2" log file). See
// d2_lua.h and d2_lua_boot.h. Called from PreInitApplication (Source/Game/Main.cpp) right after
// ERRORMANAGER_InitErrorManager has started the log writer, and once ApplicationMain returns.
#if D2_FEATURE_MODS
#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "Redivivus/Lua/d2_lua.h"
#include "Redivivus/Lua/d2_lua_boot.h"
#include "Redivivus/Lua/d2_lua_json.h"
#include "Redivivus/Lua/d2_lua_mods.h"
#include "Redivivus/Lua/d2_lua_override.h"
#include "Redivivus/Lua/generated/d2_lua_api_main.gen.h"
#include "Source/Fog/Src/LogManager.h"

#include "Redivivus/Lua/engine/d2_lua_engine.h"

static D2LuaBootResult* s_pBoot = nullptr;
static std::string s_sSettingsFile;

void D2LUA_LogToGame(const char* pszLine) {
    Fog::Src::LogManager::LOGMANAGER_TraceToModuleLog((char*)"", (char*)"%s", pszLine);
}

// mods/unsafe-mods.json: the mods the player lets reach engine memory through engine.unsafe.
static void ReadRawGrants(const std::string& sFile, std::vector<std::string>& aOut) {
    std::string sText;
    if (!D2LuaMods_ReadFile(sFile, sText)) {
        return;
    }
    D2Json tRoot;
    std::string sError;
    const D2Json* pMods = nullptr;
    if (D2Json_Parse(sText.data(), sText.size(), tRoot, sError) && tRoot.eType == D2Json::Object) {
        pMods = tRoot.Get("mods");
    }
    if (!pMods || pMods->eType != D2Json::Array) {
        D2LUA_LogToGame("[mods] unsafe-mods.json: expected { \"mods\": [\"name\", ...] }; no mod has the raw tier");
        return;
    }
    for (const D2Json& t : pMods->aItems) {
        if (t.eType == D2Json::String && D2LuaMods_IsValidName(t.sValue)) {
            aOut.push_back(t.sValue);
            std::string sLine = "[mods] mod '" + t.sValue + "' has the raw tier of the overrides (unsafe-mods.json)";
            D2LUA_LogToGame(sLine.c_str());
        }
    }
}

extern "C" void D2LUA_Boot(void) {
    if (s_pBoot) {
        return;
    }
    D2LuaMenu_WaitForPrevious();
    char szPath[MAX_PATH + 32];
    DWORD nLen = GetModuleFileNameA(nullptr, szPath, MAX_PATH);
    if (nLen == 0 || nLen >= MAX_PATH) {
        D2LUA_LogToGame("[mods] cannot find the executable's directory; no mods");
        return;
    }
    char* pSlash = strrchr(szPath, '\\');
    char* pFileName = pSlash ? pSlash + 1 : szPath;
    strcpy(pFileName, "mods");
    DWORD nAttributes = GetFileAttributesA(szPath);
    if (nAttributes == INVALID_FILE_ATTRIBUTES || !(nAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        D2LUA_LogToGame("[mods] no mods directory beside the executable; no mods");
        return;
    }
    try {
        D2LuaBootDesc tDesc;
        tDesc.aRoots.push_back(szPath);
        tDesc.sSettingsFile = std::string(szPath) + "\\settings.json";
        tDesc.sModListFile = std::string(szPath) + "\\mod-list.json";
        s_sSettingsFile = tDesc.sSettingsFile;
        tDesc.aApis.push_back(&g_D2LuaApi_main);
        tDesc.pfnLog = D2LUA_LogToGame;
        s_pBoot = new D2LuaBootResult;
        D2LuaBoot_Run(tDesc, *s_pBoot);
        ReadRawGrants(std::string(szPath) + "\\unsafe-mods.json", s_pBoot->aRawGrant);
        D2LUA_LogToGame(D2LuaOv_Summary().c_str());
        std::string sNoEntry = D2LuaOv_NoEntryList(40);
        if (!sNoEntry.empty()) {
            D2LUA_LogToGame(("[mods] overrides refused for want of a patch area: " + sNoEntry).c_str());
        }
    } catch (...) {
        D2LUA_LogToGame("[mods] out of memory while loading mods; no mods");
    }
}

const D2LuaBootResult* D2LUA_BootResult(void) {
    return s_pBoot;
}

const std::string& D2LUA_SettingsFile(void) {
    return s_sSettingsFile;
}

extern "C" void D2LUA_Shutdown(void) {
    D2LuaMenu_Shutdown();
    // Whatever a state still overrides is put back before the process goes.
    int nRestored = D2LuaOv_UndoAll();
    D2LuaOvJournal tJournal = D2LuaOv_JournalTotals();
    char sz[160];
    snprintf(sz, sizeof(sz), "[mods] overrides at exit: %d entries restored now; %d patched and %d restored in all", nRestored,
             tJournal.nPatches, tJournal.nRestores);
    D2LUA_LogToGame(sz);
    if (s_pBoot) {
        D2LuaBoot_Close(*s_pBoot);
        delete s_pBoot;
        s_pBoot = nullptr;
    }
}
#endif
