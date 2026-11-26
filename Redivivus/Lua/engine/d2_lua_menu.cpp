// The client state and the UI kit in the game's own menus: the drawing backend on D2gfx and D2Win
// (D2LuaUi), the main menu buttons the mods add, and the full screen a mod's button opens. The
// game's build only, with REDIVIVUS_MODS.
//
// The client state is made the first time the main menu comes up (on the thread that runs the
// menus, the only one that ever uses it), over the mods the boot put in load order, and every
// mod's `menu` export runs once then; the built-in redivivus mod adds MODS that way. Each time
// UIMENU_MainMenu builds the menu (D2LUA_MAIN_MENU) the mods' buttons are made with the stock
// short button art and appended to MainMenus.cpp's form list, so FORMS_DestroyLists frees them
// with the stock forms.
//
// A screen takes the menu over as Credits does: FORMS_DestroyLists, then one full-screen control
// of a form type D2Win does not know (so no stock code treats it as one of its own). Its fpDraw is
// the only place the screen draws (inside the engine's BeginScene/EndScene); its fpDrawEx polls
// the mouse position and closes the screen when the mod asked to, returning to the main menu with
// UIMENU_MainMenu, as Credits' timer does from its own fpDrawEx. Mouse buttons come through
// fpPush/fpMouse; the wheel, typed characters and the keys a screen needs through window handlers
// registered while it is open, which run before the stock ones and end the chain.
//
// Drawing rules (paths under Source/):
//  - A DC6 frame drawn at (x, y) has its left edge at x + offsetX and its bottom row at
//    y + offsetY (D2CMP/Src/CelCmp.cpp:2064). The kit places a frame's box top-left at (x, y).
//  - D2gfx has no clip state: the backend keeps the scissor stack. A sprite wholly inside is drawn
//    with DrawImage, one partly inside with DrawSprite6 and the scissor as its clip rect, one
//    outside not at all. A glyph partly inside is drawn clipped only in colour 0 (DrawSprite6
//    takes no palette shift) and dropped otherwise. Rectangles and lines are cut on the CPU.
//  - Colours are palette indices (D2GFX_GetColorFromCurrentPallete, cached; the cache is cleared
//    on every palette load). Fill alpha goes to DrawSolidRectAlpha, which the software renderer
//    rounds to four levels; lines take no alpha there.
//  - Fonts are D2Win's: a handle is an index of aFontNames plus one. Every metric and draw selects
//    the font and restores the one that was selected.
#if D2_FEATURE_MODS
#include <windows.h>

#include <stdio.h>
#include <string.h>

#include <map>
#include <string>
#include <vector>

#include "Source/globals/globals.h"
#include "Source/D2CMP/D2CMP.h"
#include "Source/D2Client/D2ControlStrc.h"
#include "Source/D2Client/D2GfxDataStrc.h"
#include "Source/D2Client/UI/EscMenu.h"
#include "Source/D2Hell/D2Hell.h"
#include "Source/D2Launch/Src/MainMenus.h"
#include "Source/D2Launch/Src/MainMenus.data.h"
#include "Source/D2Launch/D2Launch.globals.h"
#include "Source/D2Win/D2Win.h"
#include "Source/D2Win/Forms.h"
#include "Source/D2Win/D2Win.globals.h"
#include "Source/D2Win/Src/D2WinFont.data.h"
#include "Source/D2Win/Src/D2WinMain.h"
#include "Source/D2gfx/D2gfx.public.h"
#include "Source/D2Lang/D2Lang.h"
#include "Source/Fog/Fog.h"
#include "Source/Storm/Storm.h"
#include "Redivivus/Lua/d2_lua.h"
#include "Redivivus/Lua/d2_lua_hooks.h"
#include "Redivivus/Lua/d2_lua_runtime.h"
#include "Redivivus/Lua/d2_lua_ui.h"
#include "Redivivus/Lua/engine/d2_lua_engine.h"
#include "Redivivus/Lua/generated/d2_lua_api_main.gen.h"
#include "lua.hpp"

namespace {

namespace Gfx = D2gfx::D2GFX;
namespace CelCmp = D2CMP::SRC::CelCmp;
namespace WinFont = D2Win::Src::D2WinFont;
namespace WinMain = D2Win::Src::D2WinMain;

const char kFile[] = "Redivivus/Lua/engine/d2_lua_menu.cpp";

void Log(const char* pszFormat, const char* pszArg) {
    char sz[512];
    snprintf(sz, sizeof(sz), pszFormat, pszArg);
    D2LUA_LogToGame(sz);
}

struct Rect {
    int x0, y0, x1, y1;  // x1, y1 exclusive
    bool Empty() const { return x0 >= x1 || y0 >= y1; }
};

Rect Intersect(const Rect& a, const Rect& b) {
    return { a.x0 > b.x0 ? a.x0 : b.x0, a.y0 > b.y0 ? a.y0 : b.y0, a.x1 < b.x1 ? a.x1 : b.x1, a.y1 < b.y1 ? a.y1 : b.y1 };
}

bool Inside(const Rect& r, const Rect& in) {
    return r.x0 >= in.x0 && r.y0 >= in.y0 && r.x1 <= in.x1 && r.y1 <= in.y1;
}

// The kit's BlendMode (core/redivivus/enums.ts) to the engine's DRAWMODE.
DRAWMODE DrawMode(int nMode) {
    switch (nMode) {
    case 1: return (DRAWMODE)0;  // Trans25: VERYTRANSPARENT
    case 2: return (DRAWMODE)1;  // Trans50: TRANSPARENT
    case 3: return (DRAWMODE)2;  // Trans75: LESSTRANSPARENT
    case 4: return (DRAWMODE)4;  // AlphaBlack: DARKTRANSPARENT
    case 5: return (DRAWMODE)3;  // Additive: LIGHTTRANSPARENT
    default: return DRAW_SOLID;
    }
}

// A DC6 read from the archives: checked before the engine's loader sees it (IMAGE_LoadDC6 returns
// an uninitialised pointer for a missing file and halts on one that is not a DC6).
bool ValidDc6(const uint8_t* p, uint32_t nSize) {
    if (nSize < 24) {
        return false;
    }
    const int32_t* h = (const int32_t*)p;
    int32_t nDirs = h[4], nFrames = h[5];
    if (h[0] != 6 || !(h[1] & 1) || (h[1] & 2) || nDirs <= 0 || nFrames <= 0 || nDirs > 64 || nFrames > 4096
        || (int64_t)nDirs * nFrames > 8192 || 24 + (int64_t)4 * nDirs * nFrames > nSize) {
        return false;
    }
    const uint32_t* pOffsets = (const uint32_t*)(p + 24);
    for (int32_t i = 0; i < nDirs * nFrames; i++) {
        uint32_t o = pOffsets[i];
        if (o < 24u + 4u * (uint32_t)(nDirs * nFrames) || o > nSize || nSize - o < 32) {
            return false;
        }
        const int32_t* f = (const int32_t*)(p + o);
        if (f[1] < 0 || f[2] < 0 || f[1] > 4096 || f[2] > 4096 || f[7] < 0 || (uint32_t)f[7] > nSize - o - 32) {
            return false;
        }
    }
    return true;
}

DC6* LoadDc6(const char* pszPath) {
    char szFile[300];
    snprintf(szFile, sizeof(szFile), "%s.dc6", pszPath);
    uint32_t nSize = 0;
    void* pData = D2Hell::D2FILE_AllocAndLoadD2File(nullptr, szFile, &nSize, (char*)kFile, __LINE__);
    if (!pData) {
        return nullptr;
    }
    if (!ValidDc6((const uint8_t*)pData, nSize)) {
        Log("[mods] ui: %s is not a DC6 the game can draw", szFile);
        Fog::Memory::FreeClientMemory(pData, (char*)kFile, __LINE__, 0);
        return nullptr;
    }
    DC6* pOut = nullptr;
    CelCmp::CELCMP_FixupPointersAndPrepare((DC6*)pData, &pOut, (char*)kFile, __LINE__, -1, nullptr);
    return pOut;
}

struct Frame {
    D2GfxDataStrc tGfx;
    DC6Block* pBlock;
    int nW, nH, nOffX, nOffY;
};

// Frame f of direction 0, range-checked (the engine lets f == nFrames through).
bool GetFrame(DC6* pDc6, int f, Frame& out) {
    if (!pDc6 || f < 0 || f >= CelCmp::IMAGE_GetFramesCount(pDc6)) {
        return false;
    }
    memset(&out.tGfx, 0, sizeof(out.tGfx));
    out.tGfx.pDC6 = pDc6;
    out.tGfx.nFrameNumber = f;
    out.tGfx.nDirection = 0;
    DC6Block* pBlock = CelCmp::CELCMP_GetDC6BlockFromGfxData(&out.tGfx);
    if (!pBlock) {
        return false;
    }
    out.pBlock = pBlock;
    out.nW = CelCmp::GetDC6Width(pBlock);
    out.nH = CelCmp::GetDC6Height(pBlock);
    out.nOffX = CelCmp::GetDC6OffsetX(pBlock);
    out.nOffY = CelCmp::GetDC6OffsetY(pBlock);
    return true;
}

// The box of a frame's opaque pixels, relative to its top-left, read from its DC6 runs: a byte
// 0x80 ends a line, one with the top bit set skips that many transparent pixels, any other is a
// count of pixels that follow. Lines run bottom up unless the frame is flipped. Empty: x0 >= x1.
Rect InkOf(const Frame& fr) {
    Rect r = { fr.nW, fr.nH, 0, 0 };
    const uint8_t* p = (const uint8_t*)fr.pBlock->data;
    int nLen = fr.pBlock->nLength;
    int x = 0, nLine = 0;
    for (int i = 0; i < nLen && nLine < fr.nH;) {
        uint8_t b = p[i++];
        if (b == 0x80) {
            x = 0;
            nLine++;
        } else if (b & 0x80) {
            x += b & 0x7f;
        } else {
            int y = fr.pBlock->nFlip ? nLine : fr.nH - 1 - nLine;
            if (b) {
                r.x0 = x < r.x0 ? x : r.x0;
                r.x1 = x + b > r.x1 ? x + b : r.x1;
                r.y0 = y < r.y0 ? y : r.y0;
                r.y1 = y + 1 > r.y1 ? y + 1 : r.y1;
            }
            x += b;
            i += b;
        }
    }
    return r;
}

// Draw a frame with its box's top-left at (x, y), cut at rect: whole, clipped, or not at all.
void DrawFrame(Frame& fr, int x, int y, DRAWMODE eMode, const Rect& cut) {
    Rect box = { x, y, x + fr.nW, y + fr.nH };
    Rect vis = Intersect(box, cut);
    if (vis.Empty()) {
        return;
    }
    int nX = x - fr.nOffX;
    int nY = y + fr.nH - 1 - fr.nOffY;
    if (Inside(box, cut)) {
        Gfx::D2GFX_DrawImage(&fr.tGfx, nX, nY, -1, eMode, nullptr);
    } else {
        D2GfxTileRectStrc tClip = { vis.x0, vis.x1, vis.y0, vis.y1 };
        Gfx::D2GFX_DrawSprite6(&fr.tGfx, nX, nY, &tClip, eMode);
    }
}

// --- the backend ----------------------------------------------------------------------------------

class EngineUi : public D2LuaUi {
public:
    struct Sprite {
        std::string sPath;  // lower case
        DC6* pDc6;
    };
    std::vector<Sprite> aSprites;
    std::vector<Rect> aClip;
    std::map<uint32_t, uint8_t> tColours;
    std::map<uint32_t, Rect> tInk;  // a glyph's ink by font and frame (the font's file, whichever copy is loaded)
    int anAscent[14] = {};
    static const size_t kMaxSprites = 256;

    void PaletteLoaded() { tColours.clear(); }

    uint8_t Colour(uint32_t nRgb) {
        auto it = tColours.find(nRgb);
        if (it != tColours.end()) {
            return it->second;
        }
        uint8_t n = D2Win::Src::D2WinPalette::D2GFX_GetColorFromCurrentPallete((uint8_t)(nRgb >> 16), (uint8_t)(nRgb >> 8), (uint8_t)nRgb);
        if (tColours.size() < 4096) {
            tColours[nRgb] = n;
        }
        return n;
    }

    Rect Top() const { return aClip.empty() ? Rect{ 0, 0, 0, 0 } : aClip.back(); }

    DC6* Dc6Of(int h) { return h > 0 && h <= (int)aSprites.size() ? aSprites[h - 1].pDc6 : nullptr; }

    int LoadSprite(const char* pszPath) override {
        std::string sKey = pszPath;
        for (char& c : sKey) {
            c = (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c == '/' ? '\\' : c;
        }
        for (size_t i = 0; i < aSprites.size(); i++) {
            if (aSprites[i].sPath == sKey) {
                return aSprites[i].pDc6 ? (int)i + 1 : 0;
            }
        }
        if (aSprites.size() >= kMaxSprites) {
            return 0;
        }
        DC6* p = LoadDc6(sKey.c_str());
        if (!p) {
            Log("[mods] ui: no sprite %s", sKey.c_str());
        }
        aSprites.push_back({ sKey, p });
        return p ? (int)aSprites.size() : 0;
    }
    int SpriteFrames(int h) override {
        DC6* p = Dc6Of(h);
        return p ? CelCmp::IMAGE_GetFramesCount(p) : 0;
    }
    int SpriteWidth(int h, int f) override {
        Frame fr;
        return GetFrame(Dc6Of(h), f, fr) ? fr.nW : 0;
    }
    int SpriteHeight(int h, int f) override {
        Frame fr;
        return GetFrame(Dc6Of(h), f, fr) ? fr.nH : 0;
    }

    // --- fonts ---
    struct Select {
        int nWas;
        explicit Select(int nFont) : nWas(WinFont::SetAndReturnLastFont(nFont - 1)) {}
        ~Select() { WinFont::SetAndReturnLastFont(nWas); }
    };
    static int Advance(D2FontGlyphStrc* g) { return g ? g->abUnverified[1] : 0; }
    static int FrameOf(D2FontGlyphStrc* g) { return g ? *(const uint16_t*)(g->abUnverified + 6) : -1; }

    int LoadFont(const char* pszName) override {
        for (int i = 0; i < 14; i++) {
            if (_stricmp((const char*)D2Win::_FONT::aFontNames[i], pszName) == 0) {
                return i + 1;
            }
        }
        return 0;
    }
    static bool ValidFont(int nFont) { return nFont >= 1 && nFont <= 14; }

    int FontLineHeight(int nFont) override {
        if (!ValidFont(nFont)) {
            return 0;
        }
        Select s(nFont);
        return (int)WinFont::D2WINFONT_GetCurrentFontHeight() * 16 / 10;
    }
    // The glyph cell: the tallest glyph frame of codes 0..255 (the host renderer's rule).
    int FontAscent(int nFont) override {
        if (!ValidFont(nFont)) {
            return 0;
        }
        if (anAscent[nFont - 1]) {
            return anAscent[nFont - 1];
        }
        Select s(nFont);
        DC6* pDc6 = D2LuaFont_Dc6();
        int nMax = 0;
        for (int c = 0; c < 256; c++) {
            Frame fr;
            if (GetFrame(pDc6, FrameOf(D2LuaFont_Glyph((unsigned short)c)), fr) && fr.nH > nMax) {
                nMax = fr.nH;
            }
        }
        anAscent[nFont - 1] = nMax;
        return nMax;
    }
    bool HasGlyph(int nFont, int nCode) override {
        if (!ValidFont(nFont)) {
            return false;
        }
        Select s(nFont);
        D2FontGlyphStrc* g = D2LuaFont_Glyph((unsigned short)nCode);
        return nCode >= 32 && g && Advance(g) > 0 && (D2LuaFont_ByCode() || g->wCharCode == nCode);
    }
    int MeasureText(int nFont, const char* psz, size_t n) override {
        if (!ValidFont(nFont)) {
            return 0;
        }
        Select s(nFont);
        int w = 0;
        for (size_t i = 0; i < n; i++) {
            w += Advance(D2LuaFont_Glyph((unsigned char)psz[i]));
        }
        return w;
    }

    // --- frame ---
    void BeginFrame(int nW, int nH) override {
        aClip.clear();
        aClip.push_back({ 0, 0, nW, nH });
    }
    void EndFrame() override { aClip.clear(); }
    void PushScissor(int x, int y, int w, int h) override {
        if (aClip.empty() || aClip.size() > 64) {
            return;
        }
        aClip.push_back(Intersect(Top(), { x, y, x + (w > 0 ? w : 0), y + (h > 0 ? h : 0) }));
    }
    void PopScissor() override {
        if (aClip.size() > 1) {
            aClip.pop_back();
        }
    }

    // --- drawing ---
    void DrawSprite(int h, int f, int x, int y, int nMode) override {
        Frame fr;
        if (!aClip.empty() && GetFrame(Dc6Of(h), f, fr)) {
            DrawFrame(fr, x, y, DrawMode(nMode), Top());
        }
    }
    void DrawNineSlice(int h, const int* an, int x, int y, int w, int hh, int nMode) override {
        DC6* p = Dc6Of(h);
        Frame f[9];
        if (aClip.empty() || w <= 0 || hh <= 0) {
            return;
        }
        for (int i = 0; i < 9; i++) {
            if (!GetFrame(p, an[i], f[i]) || f[i].nW <= 0 || f[i].nH <= 0) {
                return;
            }
        }
        DRAWMODE m = DrawMode(nMode);
        Rect box = Intersect(Top(), { x, y, x + w, y + hh });
        int nLeft = f[0].nW, nRight = f[2].nW, nTop = f[0].nH, nBottom = f[6].nH;
        Rect in = Intersect(box, { x + nLeft, y + nTop, x + w - nRight, y + hh - nBottom });
        for (int ty = y + nTop; ty < y + hh - nBottom; ty += f[4].nH) {
            for (int tx = x + nLeft; tx < x + w - nRight; tx += f[4].nW) {
                DrawFrame(f[4], tx, ty, m, in);
            }
        }
        Rect hband = { in.x0, box.y0, in.x1, box.y1 };
        for (int tx = x + nLeft; tx < x + w - nRight; tx += f[1].nW) {
            DrawFrame(f[1], tx, y, m, hband);
            DrawFrame(f[7], tx, y + hh - f[7].nH, m, hband);
        }
        Rect vband = { box.x0, in.y0, box.x1, in.y1 };
        for (int ty = y + nTop; ty < y + hh - nBottom; ty += f[3].nH) {
            DrawFrame(f[3], x, ty, m, vband);
            DrawFrame(f[5], x + w - f[5].nW, ty, m, vband);
        }
        DrawFrame(f[0], x, y, m, box);
        DrawFrame(f[2], x + w - f[2].nW, y, m, box);
        DrawFrame(f[6], x, y + hh - f[6].nH, m, box);
        DrawFrame(f[8], x + w - f[8].nW, y + hh - f[8].nH, m, box);
    }
    void FillRect(int x, int y, int w, int h, uint32_t nRgb, int nAlpha) override {
        if (aClip.empty() || nAlpha <= 0) {
            return;
        }
        Rect r = Intersect(Top(), { x, y, x + w, y + h });
        if (!r.Empty()) {
            Gfx::D2GFX_DrawSolidRectAlpha(r.x0, r.y0, r.x1, r.y1, Colour(nRgb), (DWORD)nAlpha);
        }
    }
    // Liang-Barsky against the scissor's pixels (both ends included).
    void DrawLine(int x0, int y0, int x1, int y1, uint32_t nRgb, int nAlpha) override {
        if (aClip.empty()) {
            return;
        }
        Rect c = Top();
        if (c.Empty()) {
            return;
        }
        double t0 = 0, t1 = 1, dx = x1 - x0, dy = y1 - y0;
        double p[4] = { -dx, dx, -dy, dy };
        double q[4] = { (double)(x0 - c.x0), (double)(c.x1 - 1 - x0), (double)(y0 - c.y0), (double)(c.y1 - 1 - y0) };
        for (int i = 0; i < 4; i++) {
            if (p[i] == 0) {
                if (q[i] < 0) {
                    return;
                }
            } else {
                double t = q[i] / p[i];
                if (p[i] < 0) {
                    if (t > t1) return;
                    if (t > t0) t0 = t;
                } else {
                    if (t < t0) return;
                    if (t < t1) t1 = t;
                }
            }
        }
        int ax = (int)(x0 + t0 * dx + 0.5), ay = (int)(y0 + t0 * dy + 0.5);
        int bx = (int)(x0 + t1 * dx + 0.5), by = (int)(y0 + t1 * dy + 0.5);
        Gfx::D2GFX_DrawLine(ax, ay, bx, by, Colour(nRgb), (DWORD)nAlpha);
    }
    void DrawText(int nFont, const char* psz, size_t n, int x, int y, int nColour) override {
        if (aClip.empty() || !ValidFont(nFont)) {
            return;
        }
        int nAscent = FontAscent(nFont);
        Select s(nFont);
        DC6* pDc6 = D2LuaFont_Dc6();
        Rect c = Top();
        GLOBALPALLETE eColour = (GLOBALPALLETE)(nColour >= 0 && nColour < 13 ? nColour : 0);
        int cx = x;
        int nBottom = y + nAscent - 1;
        for (size_t i = 0; i < n; i++) {
            D2FontGlyphStrc* g = D2LuaFont_Glyph((unsigned char)psz[i]);
            Frame fr;
            if (GetFrame(pDc6, FrameOf(g), fr)) {
                Rect box = { cx + fr.nOffX, nBottom + fr.nOffY - fr.nH + 1, cx + fr.nOffX + fr.nW, nBottom + fr.nOffY + 1 };
                Rect vis = Intersect(box, c);
                // A glyph's cell is wider than its advance: what decides whether it is drawn whole
                // is where its ink is, so a coloured glyph near the scissor is not dropped for the
                // transparent part of its cell.
                uint32_t nKey = (uint32_t)nFont << 16 | (uint32_t)FrameOf(g);
                auto it = tInk.find(nKey);
                Rect ink = it != tInk.end() ? it->second : (tInk[nKey] = InkOf(fr));
                Rect inkBox = { box.x0 + ink.x0, box.y0 + ink.y0, box.x0 + ink.x1, box.y0 + ink.y1 };
                if (ink.Empty()) {
                    // A space: nothing to draw.
                } else if (Inside(inkBox, c)) {
                    Gfx::D2GFX_DrawShiftedImage(&fr.tGfx, cx, nBottom, -1, DRAW_SOLID, eColour);
                } else if (!vis.Empty() && eColour == 0) {
                    D2GfxTileRectStrc tClip = { vis.x0, vis.x1, vis.y0, vis.y1 };
                    Gfx::D2GFX_DrawSprite6(&fr.tGfx, cx, nBottom, &tClip, DRAW_SOLID);
                }
            }
            cx += Advance(g);
        }
    }
};

EngineUi s_tUi;
D2LuaRuntime* s_pClient = nullptr;
bool s_bTried = false;
bool s_bRestart = false;
D2LuaCatalog s_tNoCatalog;  // when the boot found no mods directory

// --- calls into the core ----------------------------------------------------------------------------

lua_State* State() {
    return D2LuaRt_State(s_pClient);
}

bool Host(const char* pszName, int nArgs, int nResults) {
    return D2LuaRt_CallHost(s_pClient, 0, pszName, nArgs, nResults);
}

int ScreenState() {
    if (!s_pClient || !Host("screenState", 0, 1)) {
        return 0;
    }
    int n = (int)lua_tointeger(State(), -1);
    lua_pop(State(), 1);
    return n;
}

void MenuUp(bool bUp) {
    lua_pushboolean(State(), bUp);
    Host("menuUp", 1, 0);
}

bool Restart(void*, std::string& sError) {
    (void)sError;
    s_bRestart = true;
    return true;
}

// Relaunch the executable with the same command line; the new process waits for this one to end
// (D2LuaMenu_WaitForPrevious) before it starts.
void Relaunch() {
    char szExe[MAX_PATH + 1];
    DWORD n = GetModuleFileNameA(nullptr, szExe, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) {
        D2LUA_LogToGame("[mods] restart: cannot find the executable; quit and start the game again");
        return;
    }
    szExe[n] = '\0';
    const char* pszCmd = GetCommandLineA();
    std::vector<char> aCmd(pszCmd, pszCmd + strlen(pszCmd) + 1);
    char szPid[32];
    snprintf(szPid, sizeof(szPid), "%lu", (unsigned long)GetCurrentProcessId());
    SetEnvironmentVariableA("REDIVIVUS_RESTART_AFTER", szPid);
    STARTUPINFOA tStart;
    memset(&tStart, 0, sizeof(tStart));
    tStart.cb = sizeof(tStart);
    PROCESS_INFORMATION tProcess;
    memset(&tProcess, 0, sizeof(tProcess));
    BOOL bOk = CreateProcessA(szExe, aCmd.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &tStart, &tProcess);
    SetEnvironmentVariableA("REDIVIVUS_RESTART_AFTER", nullptr);
    if (!bOk) {
        D2LUA_LogToGame("[mods] restart: the game could not start itself again; quit and start it by hand");
        return;
    }
    CloseHandle(tProcess.hThread);
    CloseHandle(tProcess.hProcess);
    D2LUA_LogToGame("[mods] restart: started again with the new mod list; this process quits");
}

// --- the screen -------------------------------------------------------------------------------------

const eD2FormType kScreenFormType = (eD2FormType)0x40;  // no D2Win form type
D2ControlStrc* s_pScreen = nullptr;
int s_nMouseX = -1, s_nMouseY = -1;

const int s_anKeys[] = { 0x08, 0x09, 0x0d, 0x1b, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x2e };

bool Input(const char* pszType, int x, int y, int nButton, int nDelta, int nKey, const char* pszText) {
    if (!s_pClient) {
        return false;
    }
    lua_State* L = State();
    lua_pushstring(L, pszType);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, nButton);
    lua_pushnumber(L, nDelta);
    lua_pushnumber(L, nKey);
    lua_pushstring(L, pszText);
    lua_pushboolean(L, GetKeyState(VK_SHIFT) < 0);
    lua_pushboolean(L, GetKeyState(VK_CONTROL) < 0);
    if (!Host("screenInput", 9, 1)) {
        return false;
    }
    bool b = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);
    return b;
}

void __stdcall OnChar(void* pData) {
    D2UIMouseMsgStrc* pMsg = (D2UIMouseMsgStrc*)pData;
    unsigned c = (unsigned)pMsg->nWParam & 0xff;
    if (c >= 32 && c != 127) {
        char sz[2] = { (char)c, 0 };
        Input("text", s_nMouseX, s_nMouseY, 0, 0, 0, sz);
    }
    Storm::SMsgBreakHandlerChain(pData);
}

void __stdcall OnWheel(void* pData) {
    D2UIMouseMsgStrc* pMsg = (D2UIMouseMsgStrc*)pData;
    int nNotches = (short)HIWORD(pMsg->nWParam) / WHEEL_DELTA;
    if (nNotches) {
        Input("wheel", WinMain::nPosX, WinMain::nPosY, 0, -nNotches * 3, 0, "");
    }
    Storm::SMsgBreakHandlerChain(pData);
}

void __stdcall OnKeyDown(void* pData) {
    D2UIMouseMsgStrc* pMsg = (D2UIMouseMsgStrc*)pData;
    Input("keydown", WinMain::nPosX, WinMain::nPosY, 0, 0, (int)(pMsg->nWParam & 0xff), "");
    Storm::SMsgBreakHandlerChain(pData);
}

void Handlers(bool bOn) {
    HWND hWnd = Gfx::GetHwnd();
    if (!hWnd) {
        return;
    }
    if (bOn) {
        D2Win::Forms::D2WIN_RegisterWindowHandler(hWnd, 0, 0x102, OnChar);
        D2Win::Forms::D2WIN_RegisterWindowHandler(hWnd, 0, 0x20a, OnWheel);
        for (int k : s_anKeys) {
            D2Win::Forms::D2WIN_RegisterWindowHandler(hWnd, 3, k, OnKeyDown);
        }
    } else {
        D2Win::Forms::D2WIN_UnregisterWindowHandler(hWnd, 0, 0x102, OnChar);
        D2Win::Forms::D2WIN_UnregisterWindowHandler(hWnd, 0, 0x20a, OnWheel);
        for (int k : s_anKeys) {
            D2Win::Forms::D2WIN_UnregisterWindowHandler(hWnd, 3, (uint32_t)k, OnKeyDown);
        }
    }
}

BOOL __fastcall ScreenDraw(D2ControlStrc* pControl) {
    (void)pControl;
    if (!s_pClient || s_pScreen != pControl) {
        return 1;
    }
    int w = 0, h = 0;
    Gfx::D2GFX_GetScreenSize(&w, &h);
    D2LuaUi_SetDrawing(s_pClient, true);
    lua_pushnumber(State(), w);
    lua_pushnumber(State(), h);
    Host("screenFrame", 2, 0);
    D2LuaUi_SetDrawing(s_pClient, false);
    s_tUi.EndFrame();
    return 1;
}

void CloseScreen() {
    D2ControlStrc* p = s_pScreen;
    s_pScreen = nullptr;
    Handlers(false);
    if (p) {
        WinMain::FORMS_DestroyControl(p);
    }
    Host("screenDispose", 0, 0);
    D2Launch::MainMenus::UIMENU_MainMenu();
    if (s_bRestart) {
        s_bRestart = false;
        Relaunch();
        D2Launch::MainMenus::MAINMENU_CleanupSoundAndClearMessageLoop(nullptr);
    }
}

// The logic tick, before the frame is drawn: pointer moves, and closing.
BOOL __fastcall ScreenTick(D2ControlStrc* pControl) {
    if (!s_pClient || s_pScreen != pControl) {
        return 1;
    }
    if (WinMain::nPosX != s_nMouseX || WinMain::nPosY != s_nMouseY) {
        s_nMouseX = WinMain::nPosX;
        s_nMouseY = WinMain::nPosY;
        Input("mousemove", s_nMouseX, s_nMouseY, 0, 0, 0, "");
    }
    if (s_bRestart || ScreenState() != 1) {
        CloseScreen();
    }
    return 1;
}

int ButtonOf(D2ControlMsg* pMsg) {
    return ((D2UIMouseMsgStrc*)pMsg)->uMsg == 0x204 || ((D2UIMouseMsgStrc*)pMsg)->uMsg == 0x205 ? 2 : 0;
}

BOOL __stdcall ScreenPush(D2ControlMsg* pMsg) {
    Input("mousedown", WinMain::nPosX, WinMain::nPosY, ButtonOf(pMsg), 0, 0, "");
    return 1;
}

BOOL __stdcall ScreenMouse(D2ControlMsg* pMsg) {
    Input("mouseup", WinMain::nPosX, WinMain::nPosY, ButtonOf(pMsg), 0, 0, "");
    return 1;
}

void OpenScreen() {
    D2Launch::MainMenus::FORMS_DestroyLists();
    // The stock theme's art is the front end's: draw under its palette, as character select does
    // after FORMS_DestroyLists (which put fechar back). The next FORMS_DestroyLists restores fechar.
    D2Launch::MainMenus::D2GFX_SetPaletteForDraw(D2Launch::CharSel::gaFrontEndPaletteFiles[2], D2Launch::CharSel::gaFrontEndPaletteFiles[3]);
    int w = 800, h = 600;
    Gfx::D2GFX_GetScreenSize(&w, &h);
    MenuUp(false);
    lua_pushnumber(State(), w);
    lua_pushnumber(State(), h);
    bool bMounted = false;
    if (Host("screenMount", 2, 1)) {
        bMounted = lua_toboolean(State(), -1) != 0;
        lua_pop(State(), 1);
    }
    if (!bMounted) {
        D2LUA_LogToGame("[mods] a screen could not be built; back to the main menu");
        D2Launch::MainMenus::UIMENU_MainMenu();
        return;
    }
    D2ControlStrc* p = (D2ControlStrc*)Fog::Memory::AllocClientMemory(sizeof(D2ControlStrc), (char*)kFile, __LINE__, 0);
    memset(p, 0, sizeof(*p));
    p->neD2FormType = kScreenFormType;
    p->dwState |= Fog::BitBuffer::gadwBitMasks[0];
    p->dwState |= Fog::BitBuffer::gadwBitMasks[2];
    p->dwPosX = 0;
    p->dwPosY = h;
    p->dwSizeX = w;
    p->dwSizeY = h;
    p->fpDraw = ScreenDraw;
    p->fpDrawEx = ScreenTick;
    p->fpPush = ScreenPush;
    p->fpMouse = ScreenMouse;
    s_pScreen = p;
    s_nMouseX = s_nMouseY = -1;
    WinMain::FORMS_AddControlToGlobalList(p);
    Handlers(true);
}

// --- the main menu buttons -----------------------------------------------------------------------

void Press(int i) {
    if (!s_pClient || s_pScreen) {
        return;
    }
    lua_pushnumber(State(), i);
    Host("menuPress", 1, 0);
    if (ScreenState() == 3) {
        OpenScreen();
    }
}

BOOL __stdcall OnPress0(D2ControlMsg*) {
    Press(0);
    return 1;
}
BOOL __stdcall OnPress1(D2ControlMsg*) {
    Press(1);
    return 1;
}
const OnPress s_apfnPress[2] = { OnPress0, OnPress1 };

// Where the buttons go: below Other Multiplayer, above Credits and Cinematics (short buttons, the
// Credits art). One is centred; two take the Credits and Cinematics columns.
void Place(int nCount, int i, int& x, int& yBottom) {
    yBottom = 0x1d8;
    x = nCount == 1 ? 0x14d : i == 0 ? 0x108 : 0x192;
}

bool EnsureClient() {
    if (s_pClient || s_bTried) {
        return s_pClient != nullptr;
    }
    s_bTried = true;
    const D2LuaBootResult* pBoot = D2LUA_BootResult();
    D2LuaRuntimeDesc tDesc;
    tDesc.nSide = D2LUA_SIDE_CLIENT;
    if (pBoot) {
        tDesc.aMods = pBoot->aOrder;
    }
    tDesc.aApis.push_back(&g_D2LuaApi_main);
    tDesc.pfnLog = D2LUA_LogToGame;
    tDesc.sSettingsFile = D2LUA_SettingsFile();
    tDesc.pUi = &s_tUi;
    D2LuaCatalog* pCatalog = pBoot ? const_cast<D2LuaCatalog*>(&pBoot->tCatalog) : &s_tNoCatalog;
    pCatalog->pfnRestart = Restart;
    tDesc.pCatalog = pCatalog;
    s_pClient = D2LuaRt_Create(tDesc);
    if (!s_pClient) {
        D2LUA_LogToGame("[mods] the client state could not start; the main menu has no mod buttons");
        return false;
    }
    Host("runMenu", 0, 0);
    return true;
}

}  // namespace

void D2LuaMenu_MainMenu(D2ControlStrc** apList, int* pnCount) {
    if (!EnsureClient()) {
        return;
    }
    MenuUp(true);
    lua_State* L = State();
    if (!Host("menuButtons", 0, 1)) {
        return;
    }
    std::vector<std::string> aLabels;
    for (int i = 1; i <= 2; i++) {
        lua_rawgeti(L, -1, i);
        if (lua_type(L, -1) == LUA_TSTRING) {
            aLabels.push_back(lua_tostring(L, -1));
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    for (size_t i = 0; i < aLabels.size() && *pnCount < 0x40; i++) {
        int x = 0, yBottom = 0;
        Place((int)aLabels.size(), (int)i, x, yBottom);
        ::D2WinButton* pButton = D2Win::Src::D2WinButton::D2WinButton(
            x, yBottom, 0x87, 0x19, ::DC6_CharSelect_ShortButtonBlank, s_apfnPress[i], 0, 0, 0, 0, nullptr);
        if (!pButton) {
            continue;
        }
        uint16_t wsz[32];
        size_t n = 0;
        for (; n < aLabels[i].size() && n < 31; n++) {
            wsz[n] = (uint8_t)aLabels[i][n];
        }
        wsz[n] = 0;
        D2Win::Src::D2WinButton::D2WINBUTTON_SetTextWithFont(pButton, 9, wsz, 0);
        apList[(*pnCount)++] = (D2ControlStrc*)pButton;
    }
}

void D2LuaMenu_PaletteLoaded(void) {
    s_tUi.PaletteLoaded();
}

// A restart started this process: wait (at most 20 s) until the one that started it has quit.
void D2LuaMenu_WaitForPrevious(void) {
    char sz[32];
    DWORD n = GetEnvironmentVariableA("REDIVIVUS_RESTART_AFTER", sz, sizeof(sz));
    if (n == 0 || n >= sizeof(sz)) {
        return;
    }
    SetEnvironmentVariableA("REDIVIVUS_RESTART_AFTER", nullptr);
    DWORD nPid = (DWORD)strtoul(sz, nullptr, 10);
    HANDLE h = nPid ? OpenProcess(SYNCHRONIZE, FALSE, nPid) : nullptr;
    if (h) {
        WaitForSingleObject(h, 20000);
        CloseHandle(h);
    }
}

void D2LuaMenu_Shutdown(void) {
    if (s_pClient) {
        D2LuaRt_Destroy(s_pClient);
        s_pClient = nullptr;
    }
    // The sprites stay: they are the engine's memory, and the process is ending.
}
#endif
