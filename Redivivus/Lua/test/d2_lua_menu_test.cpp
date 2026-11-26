// The main menu and the Mods screen without the game: a client state over a recording D2LuaUi and a
// catalogue the boot made from fixture mods. The built-in redivivus mod adds MODS through the same
// menu export a mod has; a mod adds its own button and screen; screens draw only inside a frame
// the host opened; input reaches the kit; Escape and the screen's own buttons close it; Apply goes
// through redivivus:manager, which no mod can import.
#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_boot.h"
#include "Redivivus/Lua/d2_lua_runtime.h"
#include "Redivivus/Lua/d2_lua_ui.h"
#include "Redivivus/Lua/generated/d2_lua_api_main.gen.h"
#include "Redivivus/Lua/test/d2_lua_test_util.h"
#include "lua.hpp"

namespace {

struct Draw {
    std::string sWhat;  // sprite, nine, rect, line, text
    std::string sText;  // the text, or the sprite's path
    int nX, nY;
};

// Stand-in art sizes for the stock theme's sprites (the same as core/ui/themes/stock.ts gives the
// headless metrics): frames, then width and height per frame, or one size for every frame.
struct Art {
    const char* pszTail;
    int nFrames, nW, nH, nW1, nH1;
};
const Art s_aArt[] = {
    { "gameselectscreenEXP", 12, 256, 256, 256, 256 },
    { "TileableDialog", 1, 256, 256, 256, 256 },
    { "WideButtonBlank", 4, 256, 35, 16, 35 },
    { "MediumButtonBlank", 2, 128, 35, 128, 35 },
    { "NarrowButtonBlank", 4, 256, 25, 16, 25 },
    { "ShortButtonBlank", 2, 135, 25, 135, 25 },
    { "CancelButtonBlank", 2, 96, 32, 96, 32 },
    { "clickbox", 2, 15, 16, 15, 16 },
    { "joingamescrollbars", 6, 12, 14, 12, 14 },
    { "textbox2", 1, 169, 26, 169, 26 },
};

class RecordingUi : public D2LuaUi {
public:
    std::vector<std::string> aSprites;
    std::vector<Draw> aDraws;
    int nFrames = 0, nPushes = 0, nPops = 0;

    const Art* ArtOf(int h) {
        if (h <= 0 || h > (int)aSprites.size()) {
            return nullptr;
        }
        const std::string& s = aSprites[h - 1];
        for (const Art& a : s_aArt) {
            size_t n = strlen(a.pszTail);
            if (s.size() >= n && s.compare(s.size() - n, n, a.pszTail) == 0) {
                return &a;
            }
        }
        return nullptr;
    }
    int LoadSprite(const char* pszPath) override {
        for (size_t i = 0; i < aSprites.size(); i++) {
            if (aSprites[i] == pszPath) {
                return (int)i + 1;
            }
        }
        aSprites.push_back(pszPath);
        return (int)aSprites.size();
    }
    int SpriteFrames(int h) override {
        const Art* a = ArtOf(h);
        return a ? a->nFrames : 1;
    }
    int SpriteWidth(int h, int f) override {
        const Art* a = ArtOf(h);
        return !a ? 32 : f < 0 || f >= a->nFrames ? 0 : (f & 1) ? a->nW1 : a->nW;
    }
    int SpriteHeight(int h, int f) override {
        const Art* a = ArtOf(h);
        return !a ? 32 : f < 0 || f >= a->nFrames ? 0 : (f & 1) ? a->nH1 : a->nH;
    }
    int LoadFont(const char* pszName) override { return pszName[0] ? 1 : 0; }
    int FontLineHeight(int) override { return 12; }
    int FontAscent(int) override { return 10; }
    bool HasGlyph(int, int nCode) override { return nCode >= 32; }
    int MeasureText(int, const char*, size_t nLen) override { return (int)nLen * 6; }
    void BeginFrame(int, int) override { nFrames++; }
    void EndFrame() override {}
    void DrawSprite(int h, int, int x, int y, int) override {
        aDraws.push_back({ "sprite", h > 0 && h <= (int)aSprites.size() ? aSprites[h - 1] : "", x, y });
    }
    void DrawNineSlice(int h, const int*, int x, int y, int, int, int) override {
        aDraws.push_back({ "nine", h > 0 && h <= (int)aSprites.size() ? aSprites[h - 1] : "", x, y });
    }
    void FillRect(int x, int y, int, int, uint32_t, int) override { aDraws.push_back({ "rect", "", x, y }); }
    void DrawLine(int x0, int y0, int, int, uint32_t, int) override { aDraws.push_back({ "line", "", x0, y0 }); }
    void DrawText(int, const char* psz, size_t n, int x, int y, int) override { aDraws.push_back({ "text", std::string(psz, n), x, y }); }
    void PushScissor(int, int, int, int) override { nPushes++; }
    void PopScissor() override { nPops++; }

    const Draw* Text(const char* pszText) const {
        for (const Draw& d : aDraws) {
            if (d.sWhat == "text" && d.sText == pszText) {
                return &d;
            }
        }
        return nullptr;
    }
    const Draw* Sprite(const char* pszTail, int nNth = 0) const {
        for (const Draw& d : aDraws) {
            size_t n = strlen(pszTail);
            if (d.sWhat == "sprite" && d.sText.size() >= n && d.sText.compare(d.sText.size() - n, n, pszTail) == 0 && nNth-- == 0) {
                return &d;
            }
        }
        return nullptr;
    }
};

// A screen's first frame lays out a README; under emulation (32-bit Linux on an arm64 Mac) that
// takes longer than the usual test budget. This test checks behaviour, not speed.
const unsigned kMenuBudgetMs = 3000;

int s_nRestarts = 0;
bool TestRestart(void*, std::string& sError) {
    (void)sError;
    s_nRestarts++;
    return true;
}

// Host calls, as engine/d2_lua_menu.cpp makes them.
bool Host(D2LuaRuntime* pRt, const char* pszName, int nArgs, int nResults) {
    return D2LuaRt_CallHost(pRt, 0, pszName, nArgs, nResults);
}

int ScreenState(D2LuaRuntime* pRt) {
    lua_State* L = D2LuaRt_State(pRt);
    if (!Host(pRt, "screenState", 0, 1)) {
        return -1;
    }
    int n = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);
    return n;
}

void Frame(D2LuaRuntime* pRt, RecordingUi& tUi) {
    lua_State* L = D2LuaRt_State(pRt);
    tUi.aDraws.clear();
    D2LuaUi_SetDrawing(pRt, true);
    lua_pushnumber(L, 800);
    lua_pushnumber(L, 600);
    Host(pRt, "screenFrame", 2, 0);
    D2LuaUi_SetDrawing(pRt, false);
}

bool Input(D2LuaRuntime* pRt, const char* pszType, int x, int y, int nKey = 0, const char* pszText = "") {
    lua_State* L = D2LuaRt_State(pRt);
    lua_pushstring(L, pszType);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, 0);
    lua_pushnumber(L, 0);
    lua_pushnumber(L, nKey);
    lua_pushstring(L, pszText);
    lua_pushboolean(L, 0);
    lua_pushboolean(L, 0);
    if (!Host(pRt, "screenInput", 9, 1)) {
        return false;
    }
    bool b = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);
    return b;
}

void Click(D2LuaRuntime* pRt, int x, int y) {
    Input(pRt, "mousemove", x, y);
    Input(pRt, "mousedown", x, y);
    Input(pRt, "mouseup", x, y);
}

bool Mount(D2LuaRuntime* pRt) {
    lua_State* L = D2LuaRt_State(pRt);
    lua_pushnumber(L, 800);
    lua_pushnumber(L, 600);
    if (!Host(pRt, "screenMount", 2, 1)) {
        return false;
    }
    bool b = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);
    return b;
}

void Press(D2LuaRuntime* pRt, int i) {
    lua_pushnumber(D2LuaRt_State(pRt), i);
    Host(pRt, "menuPress", 1, 0);
}

void MenuUp(D2LuaRuntime* pRt, bool bUp) {
    lua_pushboolean(D2LuaRt_State(pRt), bUp);
    Host(pRt, "menuUp", 1, 0);
}

std::string Buttons(D2LuaRuntime* pRt) {
    lua_State* L = D2LuaRt_State(pRt);
    std::string s;
    if (!Host(pRt, "menuButtons", 0, 1)) {
        return "(failed)";
    }
    for (int i = 1;; i++) {
        lua_rawgeti(L, -1, i);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            break;
        }
        s += (s.empty() ? "" : ",") + std::string(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return s;
}

std::string TempFile(const char* pszName) {
    const char* pszDir = getenv("TMPDIR");
    if (!pszDir || !*pszDir) {
        pszDir = getenv("TEMP");
    }
    if (!pszDir || !*pszDir) {
        pszDir = getenv("TMP");
    }
#ifndef _WIN32
    if (!pszDir || !*pszDir) {
        pszDir = "/tmp";
    }
#endif
    if (!pszDir || !*pszDir) {
        pszDir = ".";
    }
    return std::string(pszDir) + "/" + pszName;
}

}  // namespace

void RunMenuTests(const char* pszRoot) {
    printf("--- the main menu and the Mods screen\n");
    std::string sRoot = std::string(pszRoot) + "/Redivivus/Lua/test/menu-mods";
    std::string sModList = TempFile("d2_lua_menu_test-mod-list.json");
    remove(sModList.c_str());
    {
        FILE* f = fopen(sModList.c_str(), "wb");
        Check(f != nullptr, ("a mod list to write: " + sModList).c_str());
        if (!f) {
            return;
        }
        fputs("{ \"mods\": [ { \"name\": \"menubad\", \"enabled\": true } ] }\n", f);
        fclose(f);
    }

    // The boot over the fixture mods: the catalogue.
    D2LuaBootDesc tBoot;
    tBoot.aRoots.push_back(sRoot);
    tBoot.sModListFile = sModList;
    tBoot.aApis.push_back(&g_D2LuaApi_main);
    tBoot.pfnLog = TestLog;
    tBoot.tLimits = { kMenuBudgetMs, (size_t)64 * 1024 * 1024 };
    D2LuaBootResult tResult;
    D2LuaBoot_Run(tBoot, tResult);
    tResult.tCatalog.pfnRestart = TestRestart;
    const D2LuaCatalog& tCat = tResult.tCatalog;
    std::string sStates;
    for (const D2LuaCatalogMod& t : tCat.aMods) {
        sStates += t.tInfo.sName + "=" + t.sState + " ";
    }
    Check(sStates == "menubad=enabled menumod=enabled needsgone=missingDependency ",
          ("the catalogue has every installed mod and how it stands: " + sStates).c_str());
    bool bDepOk = false;
    for (const D2LuaCatalogMod& t : tCat.aMods) {
        if (t.tInfo.sName == "menumod") {
            bDepOk = t.aDependencies.size() == 2 && t.aDependencies[0].sState == "ok" && t.aDependencies[0].sInstalled == "0.1.0"
                && t.aDependencies[1].sState == "optional" && t.tInfo.sLicence == "MIT";
        }
        if (t.tInfo.sName == "needsgone") {
            bDepOk = bDepOk && t.aDependencies.size() == 2 && t.aDependencies[0].sSpec == "redivivus (implicit)"
                && t.aDependencies[1].sState == "missing"
                && t.sError.find("gone-mod") != std::string::npos;
        }
    }
    Check(bDepOk, "  with each dependency as it is met, the licence, and why a mod does not load");

    RecordingUi tUi;
    D2LuaRuntimeDesc tDesc;
    tDesc.nSide = D2LUA_SIDE_CLIENT;
    for (const D2LuaModInfo& t : tResult.aOrder) {
        if (t.sName != "menubad") {
            tDesc.aMods.push_back(t);  // menubad gets a state of its own below
        }
    }
    tDesc.aApis.push_back(&g_D2LuaApi_main);
    tDesc.pfnLog = TestLog;
    tDesc.tLimits = { kMenuBudgetMs, (size_t)64 * 1024 * 1024 };
    tDesc.pUi = &tUi;
    tDesc.pCatalog = &tCat;
    ClearLog();
    D2LuaRuntime* pRt = D2LuaRt_Create(tDesc);
    Check(pRt != nullptr, "a client state with a renderer and the catalogue");
    if (!pRt) {
        D2LuaBoot_Close(tResult);
        return;
    }

    // The menu exports run once, in load order: redivivus (the built-in mod) first.
    ClearLog();
    Host(pRt, "runMenu", 0, 0);
    Host(pRt, "runMenu", 0, 0);
    Check(Buttons(pRt) == "MODS,Menu Mod", ("the built-in mod adds MODS through the menu export, a mod adds its own: " + Buttons(pRt)).c_str());
    CheckLogged("menumod menu ran table false", "  a mod's menu export gets the Menu once");
    CheckLogged("the main menu has room for 2 buttons; 'Third' is left out", "  a button past the room is left out and logged");
    Check(tUi.nFrames == 0 && tUi.aDraws.empty(), "  nothing is drawn while no screen is open");

    // A mod's screen.
    MenuUp(pRt, true);
    ClearLog();
    Press(pRt, 1);
    CheckLogged("menumod pressed", "a button press runs its handler as its mod");
    Check(ScreenState(pRt) == 3, "  the screen asks to open");
    MenuUp(pRt, false);
    Check(Mount(pRt) && ScreenState(pRt) == 1, "  and mounts over the engine backend");
    CheckLogged("menumod view built true", "  its view gets the screen");
    // Outside a frame the host opened nothing is drawn, whatever the screen does.
    lua_State* L = D2LuaRt_State(pRt);
    lua_pushnumber(L, 800);
    lua_pushnumber(L, 600);
    Host(pRt, "screenFrame", 2, 0);
    Check(tUi.nFrames == 0 && tUi.aDraws.empty(), "  a frame outside the host's draw is not drawn");
    Frame(pRt, tUi);
    Check(tUi.nFrames == 1 && tUi.Text("Menu Mod screen") != nullptr && tUi.nPushes == tUi.nPops,
          "  inside it the screen draws with the stock theme, every scissor popped");
    const Draw* pClose = tUi.Text("Close");
    if (pClose) {
        Click(pRt, pClose->nX + 2, pClose->nY + 2);
    }
    Check(pClose && ScreenState(pRt) == 2, "  its own button closes it");
    Host(pRt, "screenDispose", 0, 0);
    Check(ScreenState(pRt) == 0, "  and the host disposes of it");

    // The Mods screen: the built-in mod's, on the public API.
    MenuUp(pRt, true);
    Press(pRt, 0);
    MenuUp(pRt, false);
    Check(ScreenState(pRt) == 3 && Mount(pRt), "MODS opens the Mods screen");
    Frame(pRt, tUi);
    Check(tUi.Text("Mods") && tUi.Text("Menu Bad") && tUi.Text("Menu Mod") && tUi.Text("Needs Gone"),
          "  it lists the installed mods from the catalogue");
    Check(tUi.Text("missing dependency") != nullptr && tUi.Text("enabled") != nullptr, "  with how each stands");
    Check(tUi.Text("Menu Bad") && tUi.Text("Name") && tUi.Text("menubad"), "  and the selected mod's details");
    // Select Menu Mod: its README is read from its folder.
    const Draw* pRow = tUi.Text("Menu Mod");
    if (pRow) {
        Click(pRt, pRow->nX + 2, pRow->nY + 2);
    }
    Frame(pRt, tUi);
    Check(tUi.Text("Menu Mod") && tUi.Text("Adds a ") != nullptr, "  clicking a row shows that mod's README");
    // Keys and text reach the kit: typing into the search field filters the list.
    const Draw* pSearch = tUi.Text("Search mods");
    if (pSearch) {
        Click(pRt, pSearch->nX + 2, pSearch->nY + 2);
    }
    Input(pRt, "text", 0, 0, 0, "need");
    Frame(pRt, tUi);
    Check(tUi.Text("need") && tUi.Text("Needs Gone") && !tUi.Text("Menu Bad"), "  typed text reaches the focused field and filters the list");
    Input(pRt, "keydown", 0, 0, 8);
    Input(pRt, "keydown", 0, 0, 8);
    Input(pRt, "keydown", 0, 0, 8);
    Input(pRt, "keydown", 0, 0, 8);
    Frame(pRt, tUi);
    Check(tUi.Text("Menu Bad") != nullptr && ScreenState(pRt) == 1, "  Backspace edits it; the screen stays open");
    // Turn Menu Bad off (its checkbox is the first), then Apply and restart.
    const Draw* pBox = tUi.Sprite("clickbox");
    if (pBox) {
        Click(pRt, pBox->nX + 4, pBox->nY + 4);
    }
    Frame(pRt, tUi);
    Check(tUi.Text("1 pending change: restart to apply.") != nullptr, "  toggling a mod is a pending change");
    const Draw* pApply = tUi.Text("Apply and restart");
    if (pApply) {
        Click(pRt, pApply->nX + 2, pApply->nY + 2);
    }
    std::string sList;
    D2LuaMods_ReadFile(sModList, sList);
    Check(s_nRestarts == 1 && sList.find("\"name\": \"menubad\", \"enabled\": false") != std::string::npos,
          ("  Apply writes the mod list through redivivus:manager and restarts: " + sList).c_str());
    ClearLog();
    Check(Input(pRt, "keydown", 0, 0, 27) && ScreenState(pRt) == 2, "  Escape that nothing took closes the screen");
    Host(pRt, "screenDispose", 0, 0);

    D2LuaRt_Destroy(pRt);

    // A menu export that fails after adding a button, and a screen whose view fails.
    std::vector<D2LuaModInfo> aAll = tDesc.aMods;
    tDesc.aMods.clear();
    for (const D2LuaModInfo& t : tResult.aOrder) {
        if (t.sName == "menubad") {
            tDesc.aMods.push_back(t);
        }
    }
    ClearLog();
    D2LuaRuntime* pBad = D2LuaRt_Create(tDesc);
    if (pBad) {
        Host(pBad, "runMenu", 0, 0);
        Check(Logged("menubad's menu fails") && Buttons(pBad) == "MODS,Bad", "a menu export that fails is logged; what it added stays");
        MenuUp(pBad, true);
        Press(pBad, 1);
        MenuUp(pBad, false);
        ClearLog();
        Check(ScreenState(pBad) == 3 && !Mount(pBad) && Logged("the view fails") && ScreenState(pBad) == 0,
              "  a screen whose view fails does not open (the host returns to the menu)");
        D2LuaRt_Destroy(pBad);
    }
    tDesc.aMods = aAll;

    // A server state never calls menu; UI built there is never drawn and raises nothing.
    tDesc.nSide = D2LUA_SIDE_SERVER;
    tDesc.pUi = nullptr;
    ClearLog();
    D2LuaRuntime* pServer = D2LuaRt_Create(tDesc);
    if (pServer) {
        Host(pServer, "runMenu", 0, 0);
        std::string sButtons = Buttons(pServer);
        Check(sButtons.empty() && !Logged("menumod menu ran"), "a server state never runs menu exports");
        Check(TestRun(pServer, 0, "menumod", "local m = require('redivivus:mods') assert(#m.installedMods() == 3)"),
              "  redivivus:mods reads the catalogue in every state");
        D2LuaRt_Destroy(pServer);
    }
    D2LuaBoot_Close(tResult);
    remove(sModList.c_str());
}
#endif
