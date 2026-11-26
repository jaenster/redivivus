// What a client state draws with and what every state knows about the installed mods: the two
// native tables behind redivivus:ui's engine backend, the main menu, and redivivus:mods /
// redivivus:manager.
//
// D2LuaUi is the drawing backend the UI kit's Backend interface (core/ui/backend.ts) runs on. The
// game implements it on D2gfx and D2Win (engine/d2_lua_menu.cpp); the host test records the calls.
// The natives (table "ui" of the core's natives, a client state with a D2LuaUi only) check their
// arguments and refuse to draw outside a frame the host opened (D2LuaUi_BeginFrame), so nothing a
// mod keeps from a frame can draw later. Units are logical pixels, origin top-left.
//
// D2LuaCatalog is what the boot learnt about the installed mods: every one it found, enabled or
// not and why, its dependencies as they are met, the settings the settings stage declared, and
// the files that hold the player's choices. The natives (table "catalog") read it; writing the
// mod list and restarting are for the built-in mod only (the grant behind redivivus:manager).
#pragma once

#if D2_FEATURE_MODS
#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_api.h"
#include "Redivivus/Lua/d2_lua_mods.h"
#include "Redivivus/Lua/d2_lua_runtime.h"

// --- the drawing backend ---------------------------------------------------------------------------

class D2LuaUi {
public:
    virtual ~D2LuaUi() {}
    // Assets. Handles are positive; 0 is "could not load" and draws nothing.
    virtual int LoadSprite(const char* pszPath) = 0;
    virtual int SpriteFrames(int nSprite) = 0;
    virtual int SpriteWidth(int nSprite, int nFrame) = 0;
    virtual int SpriteHeight(int nSprite, int nFrame) = 0;
    virtual int LoadFont(const char* pszName) = 0;
    virtual int FontLineHeight(int nFont) = 0;
    virtual int FontAscent(int nFont) = 0;
    virtual bool HasGlyph(int nFont, int nCode) = 0;
    virtual int MeasureText(int nFont, const char* pszText, size_t nLen) = 0;
    // A frame: the host opened it (D2LuaUi_BeginFrame); the kit's beginFrame/endFrame only mark it.
    virtual void BeginFrame(int nWidth, int nHeight) = 0;
    virtual void EndFrame() = 0;
    // Drawing, already clipped to the screen; the scissor is the backend's.
    virtual void DrawSprite(int nSprite, int nFrame, int nX, int nY, int nMode) = 0;
    virtual void DrawNineSlice(int nSprite, const int* anFrames, int nX, int nY, int nW, int nH, int nMode) = 0;
    virtual void FillRect(int nX, int nY, int nW, int nH, uint32_t nRgb, int nAlpha) = 0;
    virtual void DrawLine(int nX0, int nY0, int nX1, int nY1, uint32_t nRgb, int nAlpha) = 0;
    virtual void DrawText(int nFont, const char* pszText, size_t nLen, int nX, int nY, int nColour) = 0;
    virtual void PushScissor(int nX, int nY, int nW, int nH) = 0;
    virtual void PopScissor() = 0;
};

// The host opens and closes the frame a mod may draw in (the game: inside a control's fpDraw,
// between the engine's BeginScene and EndScene). Draw calls outside are ignored.
void D2LuaUi_SetDrawing(D2LuaRuntime* pRt, bool bDrawing);

// --- the installed mods -------------------------------------------------------------------------------

// How an installed mod stands this session (the string values of redivivus:enums ModState).
#define D2LUA_MODSTATE_ENABLED "enabled"
#define D2LUA_MODSTATE_DISABLED "disabled"                    // the player turned it off
#define D2LUA_MODSTATE_FAULTED "faulted"                      // it failed while loading
#define D2LUA_MODSTATE_MISSING_DEPENDENCY "missingDependency"  // a dependency is missing, off, or the wrong version

struct D2LuaCatalogDependency {
    std::string sSpec;       // as written in info.json
    std::string sName;
    std::string sState;      // redivivus:enums DependencyState: ok, missing, version, optional, incompatible
    std::string sInstalled;  // the installed version, "" when none
};

struct D2LuaCatalogMod {
    D2LuaModInfo tInfo;
    bool bEnabled = true;  // what the mod list says
    std::string sState;    // D2LUA_MODSTATE_*
    std::string sError;    // why it is not loaded, when it is not
    std::vector<D2LuaCatalogDependency> aDependencies;
};

struct D2LuaCatalog {
    std::vector<D2LuaCatalogMod> aMods;  // load order first, then the rest by name
    std::vector<D2LuaSettingInfo> aSettings;
    std::string sModListFile;  // mods/mod-list.json
    // Restart the game with the mod list as written (the built-in mod's "Apply and restart").
    // False when the host cannot; sError says why.
    bool (*pfnRestart)(void* pUser, std::string& sError) = nullptr;
    void* pUser = nullptr;
};

// mods/mod-list.json: { "mods": [ { "name": "x", "enabled": false }, ... ] }. A mod that is not
// listed is enabled. Read: false only when the file exists and is not a mod list (sError says
// why); a missing file is an empty list.
bool D2LuaModList_Read(const std::string& sFile, std::map<std::string, bool>& tOut, std::string& sError);
// Set the named mods' entries (the others are kept) and write the file.
bool D2LuaModList_Write(const std::string& sFile, const std::map<std::string, bool>& tChanges, std::string& sError);

// Fill aMods from the boot: every mod found, the user-disabled ones, the ones that load (in
// order) and the reasons the rest do not.
void D2LuaCatalog_Fill(D2LuaCatalog& tOut, const std::vector<D2LuaModInfo>& aFound, const std::vector<D2LuaModInfo>& aOrder,
                       const std::map<std::string, bool>& tModList, const std::map<std::string, std::string>& tFaulted,
                       const std::map<std::string, std::string>& tUnresolved);

// The natives, put into the core's natives table at nTable when the state starts: table "ui" in a
// client state with a D2LuaUi, table "catalog" in every state (empty without a D2LuaCatalog).
void D2LuaUi_Natives(lua_State* L, int nTable, D2LuaRuntime* pRt);
#endif
