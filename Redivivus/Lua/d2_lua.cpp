// Mods tied to the game: discovered under mods/ beside the executable at boot, with every line the
// runtime logs going to the game's own log (Fog's log manager, the default "D2" log file). See
// d2_lua.h and d2_lua_boot.h. Called from PreInitApplication (Source/Game/Main.cpp) right after
// ERRORMANAGER_InitErrorManager has started the log writer, and once ApplicationMain returns.
#if D2_FEATURE_MODS
#include <windows.h>
#include <string.h>

#include "Redivivus/Lua/d2_lua.h"
#include "Redivivus/Lua/d2_lua_boot.h"
#include "Redivivus/Lua/generated/d2_lua_api_main.gen.h"
#include "Source/Fog/Src/LogManager.h"

static D2LuaBootResult* s_pBoot = nullptr;

static void D2LUA_LogToGame(const char* pszLine) {
    Fog::Src::LogManager::LOGMANAGER_TraceToModuleLog((char*)"", (char*)"%s", pszLine);
}

extern "C" void D2LUA_Boot(void) {
    if (s_pBoot) {
        return;
    }
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
        tDesc.aApis.push_back(&g_D2LuaApi_main);
        tDesc.pfnLog = D2LUA_LogToGame;
        s_pBoot = new D2LuaBootResult;
        D2LuaBoot_Run(tDesc, *s_pBoot);
    } catch (...) {
        D2LUA_LogToGame("[mods] out of memory while loading mods; no mods");
    }
}

extern "C" void D2LUA_Shutdown(void) {
    if (s_pBoot) {
        D2LuaBoot_Close(*s_pBoot);
        delete s_pBoot;
        s_pBoot = nullptr;
    }
}
#endif
