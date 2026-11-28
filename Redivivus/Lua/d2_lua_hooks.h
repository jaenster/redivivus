// The engine's hooks into the mod runtime, one macro per call site in Source/ (see
// engine/d2_lua_engine.cpp for what each does). With the REDIVIVUS_MODS option off every macro
// expands to nothing, so a hooked unit's code is exactly what it was. Macros, not empty inline
// functions: at -O0 GCC leaves a nop for an inlined empty call. This header adds no type to the
// unit that includes it: only forward declarations, under the option.
//
//   D2LUA_AI_OVERLAY(pUnit, nAiIndex)  MONSTERAI_GetAiFunctionFromTable, before the general AI index
//                                      is used: returns the host's AI entry for an AI id a mod
//                                      claimed in this game
//   D2LUA_TICK(pGame)                  ServerGameLoop, after the timers and before the clients are
//                                      updated: the game's mod state (made on the first tick) ticks
//   D2LUA_GAME_END(pGame)              GAME_DestroyGame, before anything is freed: the state closes
//   D2LUA_UNIT_FREED(pGame, pUnit)     FreeServerUnit, while the unit is whole: a monster's AI
//                                      script ends (freed, or frozen with its room)
//   D2LUA_MONSTER_DIED(pGame, pUnit)   MONSTERMODE_OnDeath, before the AI timers stop: the script ends;
//                                      a revived monster starts it over
//   D2LUA_MAIN_MENU(apList, nCount)    UIMENU_MainMenu, after the stock buttons: the mods' main menu
//                                      buttons (engine/d2_lua_menu.cpp), appended to MainMenus.cpp's
//                                      form list so FORMS_DestroyLists frees them with the rest
//   D2LUA_PALETTE_LOADED()             D2WINPAL_LoadPaletteFiles, once the new palette is set: the UI
//                                      backend's RGB-to-palette-index cache is cleared
//   D2LUA_GAME_SCOPE_BEGIN(pGame), D2LUA_GAME_SCOPE_END()
//                                      around ServerGameLoop's frame and a client's in-game command:
//                                      the thread works for that game, so function overrides on it
//                                      run in the game's server state (d2_lua_override.h)
//   D2LUA_WINFONT_ACCESS()             at the end of D2WinFont.cpp: defines the accessors the UI backend
//                                      reads the selected font through (its DC6 and glyph lookup are
//                                      file statics there)
#pragma once

#if D2_FEATURE_MODS
struct SGAMEDATA;
struct D2UnitStrc;
struct D2AiListStrc;
struct D2ControlStrc;
struct DC6;
struct D2FontGlyphStrc;

D2AiListStrc* D2LuaAi_Overlay(D2UnitStrc* pUnit, int nAiIndex);
void D2LuaEngine_Tick(SGAMEDATA* pGame);
void D2LuaEngine_GameEnd(SGAMEDATA* pGame);
void D2LuaEngine_UnitFreed(SGAMEDATA* pGame, D2UnitStrc* pUnit);
void D2LuaEngine_MonsterDied(SGAMEDATA* pGame, D2UnitStrc* pUnit);
void* D2LuaEngine_ScopeBegin(SGAMEDATA* pGame);
void D2LuaEngine_ScopeEnd(void* pWas);

void D2LuaMenu_MainMenu(D2ControlStrc** apList, int* pnCount);
void D2LuaMenu_PaletteLoaded(void);
// Defined by D2LUA_WINFONT_ACCESS in D2WinFont.cpp: the selected font's DC6, the glyph entry of a
// character in it, and whether glyphs are indexed by code (the Latin fonts) or looked up.
DC6* D2LuaFont_Dc6(void);
D2FontGlyphStrc* D2LuaFont_Glyph(unsigned short nCode);
int D2LuaFont_ByCode(void);

#define D2LUA_AI_OVERLAY(pUnit, nAiIndex)                                \
    do {                                                                  \
        D2AiListStrc* pLuaAiEntry_ = D2LuaAi_Overlay((pUnit), (nAiIndex)); \
        if (pLuaAiEntry_) {                                               \
            return pLuaAiEntry_;                                          \
        }                                                                 \
    } while (0)
#define D2LUA_TICK(pGame) D2LuaEngine_Tick(pGame)
#define D2LUA_GAME_END(pGame) D2LuaEngine_GameEnd(pGame)
#define D2LUA_UNIT_FREED(pGame, pUnit) D2LuaEngine_UnitFreed((pGame), (pUnit))
#define D2LUA_MONSTER_DIED(pGame, pUnit) D2LuaEngine_MonsterDied((pGame), (pUnit))
#define D2LUA_GAME_SCOPE_BEGIN(pGame) void* pLuaGameScope_ = D2LuaEngine_ScopeBegin(pGame)
#define D2LUA_GAME_SCOPE_END() D2LuaEngine_ScopeEnd(pLuaGameScope_)
#define D2LUA_MAIN_MENU(apList, nCount) D2LuaMenu_MainMenu((apList), &(nCount))
#define D2LUA_PALETTE_LOADED() D2LuaMenu_PaletteLoaded()
#define D2LUA_WINFONT_ACCESS()                                                                      \
    DC6* D2LuaFont_Dc6(void) {                                                                      \
        return gppDC6FontCache ? *gppDC6FontCache : nullptr;                                         \
    }                                                                                               \
    D2FontGlyphStrc* D2LuaFont_Glyph(unsigned short nCode) {                                        \
        return gpfnFontGetCharWidth ? gpfnFontGetCharWidth(nCode) : nullptr;                         \
    }                                                                                               \
    int D2LuaFont_ByCode(void) {                                                                    \
        return gpfnFontGetCharWidth == &D2Win::Src::D2WinFont::FONT_GetCharWidthCallback;             \
    }
#else
#define D2LUA_AI_OVERLAY(pUnit, nAiIndex) ((void)0)
#define D2LUA_TICK(pGame) ((void)0)
#define D2LUA_GAME_END(pGame) ((void)0)
#define D2LUA_UNIT_FREED(pGame, pUnit) ((void)0)
#define D2LUA_MONSTER_DIED(pGame, pUnit) ((void)0)
#define D2LUA_GAME_SCOPE_BEGIN(pGame) ((void)0)
#define D2LUA_GAME_SCOPE_END() ((void)0)
#define D2LUA_MAIN_MENU(apList, nCount) ((void)0)
#define D2LUA_PALETTE_LOADED() ((void)0)
#define D2LUA_WINFONT_ACCESS()
#endif
