// The mod runtime: one Lua state for one side (data, server or client) running a resolved mod
// list (d2_lua_mods.h) on the sandboxed host (d2_lua_host.h), with the core (d2_lua_core.h) and the
// natives the declarations define (d2_lua_api.h).
//
// A mod is one program. Its files live in its namespace, mods/<name>/ (its folder in folder mode,
// later its archive); what a state runs of it:
//   data      settings.lua, data.lua, data-updates.lua, data-final-fixes.lua (each stage over all
//             mods in load order before the next stage)
//   server    main.lua, whose exports may hold server(game): called once for every game
//   client    main.lua, whose exports may hold client(client): called once for every game
// Both control states load the same main; each calls only its own export, so a side that does not
// run here never has its export called. The states stay separate even in single player.
//
// Modules (require, and import in TypeScript): "<mod>:<path>" names a module of a mod - the
// requiring mod itself, a declared dependency, or redivivus, the mod built into the game; a name
// without a mod ("ai", "shared/ids", TypeScript's compiled relative imports) is the requiring mod's
// own. Both forms of one module are one cached instance. Paths are case-insensitive, as in an MPQ,
// and a mod is only ever served files of its own namespace. redivivus is served only from the copy
// embedded in the exe: its public modules (redivivus:server, ...) resolve in the states they are
// declared for; redivivus:test only for mods the test runner grants it, redivivus:automation only
// for driver scripts. A module of another mod runs in the requiring mod's environment and is
// cached per requiring mod.
//
// Games: everything a mod registers hangs off the object its export got, so off one game, and is
// dropped when the game ends. One state may serve several games.
#pragma once

#if D2_FEATURE_MODS
#include <stdint.h>

#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_api.h"
#include "Redivivus/Lua/d2_lua_host.h"
#include "Redivivus/Lua/d2_lua_json.h"
#include "Redivivus/Lua/d2_lua_mods.h"

class D2LuaWorld;  // d2_lua_world.h
class D2LuaUi;     // d2_lua_ui.h
struct D2LuaCatalog;

struct D2LuaRuntimeDesc {
    int nSide = D2LUA_SIDE_SERVER;
    std::vector<D2LuaModInfo> aMods;           // in load order (D2LuaMods_Resolve); redivivus is added when absent
    std::vector<const D2LuaApiDesc*> aApis;    // the declaration sets bound into this state
    D2LuaLogFn pfnLog = nullptr;
    D2LuaLimits tLimits = { 0, 0 };
    std::string sSettingsFile;  // the mods' settings file; empty or missing: every setting has its default
    bool bSettingsStageOnly = false;  // data side: run settings.lua and none of the data stage files
    std::vector<std::string> aTestGrant;    // mods that may import redivivus:test (the test runner's)
    std::vector<std::string> aDriverGrant;  // scripts that may import redivivus:automation
    // The game world a server state's mods see (d2_lua_world.h), owned by the caller and alive as
    // long as the state; null for a state without one. With a world, add g_D2LuaApi_world to aApis.
    D2LuaWorld* pWorld = nullptr;
    // What a client state draws with (d2_lua_ui.h), owned by the caller; null: nothing is drawn
    // (redivivus:ui still works, as in a server state).
    D2LuaUi* pUi = nullptr;
    // What the boot learnt about the installed mods (d2_lua_ui.h), owned by the caller; null: the
    // state knows only the mods it runs. Read by redivivus:mods.
    const D2LuaCatalog* pCatalog = nullptr;
};

// One declared setting (d2_lua_settings.cpp) with the value this state settled on.
struct D2LuaSettingInfo {
    std::string sMod, sName;
    std::string sType;   // bool, int, number, string, choice
    std::string sScope;  // startup, game, player
    std::string sTitle, sDescription;
    D2Json tDefault, tValue;
    bool bHasMin = false, bHasMax = false;
    double nMin = 0, nMax = 0;
    std::vector<std::string> aChoices;
    std::string sProblem;  // why the file's value was refused, when it was
    int nGeneration = 0;   // the load it was declared in (a reload declares again)
};

// Mod messages (d2_lua_messages.cpp): the transport a runtime sends through. pfnSend gets the whole
// message (envelope and payload) and must copy it; it never throws. nGame is the game it belongs
// to; on the server nPeer names the receiving client (0: every client); from a client it is 0.
#define D2LUA_MESSAGE_MAX 4096
struct D2LuaTransport {
    void (*pfnSend)(void* pUser, D2LuaRuntime* pFrom, uint32_t nGame, uint32_t nPeer, const void* pData, size_t nSize);
    void* pUser;
};

// Create the state, start the core, and run the side's entries of every mod in load order (the
// data stage files, or every main). Returns null (logged) when the state cannot be made; a mod
// whose files fail is logged and the others still run.
D2LuaRuntime* D2LuaRt_Create(const D2LuaRuntimeDesc& tDesc);

// A game starts: every mod's export for this side is called with its object for the game. With
// pStorage the mods' storage for it is restored first (D2LuaRt_SaveStorage's bytes) and
// game.loaded is true. False with sError when the bytes are not a valid save; the mods then start
// with empty storage.
bool D2LuaRt_StartGame(D2LuaRuntime* pRt, uint32_t nGame, const std::string* pStorage, std::string& sError);
// The game ends: everything registered through it is dropped. False when it was not running.
bool D2LuaRt_EndGame(D2LuaRuntime* pRt, uint32_t nGame);

// Advance a game one tick: raise Event.Tick, then resume the game's scripts that are due.
void D2LuaRt_Tick(D2LuaRuntime* pRt, uint32_t nGame);
long long D2LuaRt_CurrentTick(D2LuaRuntime* pRt, uint32_t nGame);

// Raise an event of a game: the payload table is at the top of the stack (popped); name and tick
// are set on it. False when the side has no such event or the game is not running.
bool D2LuaRt_RaiseEvent(D2LuaRuntime* pRt, uint32_t nGame, const char* pszEvent);

// Call registry entry pszId of pszRegistry (pszId null for a slot registry) in a game with the
// nArgs values at the top of the stack; on D2LUA_OK leaves one result.
int D2LuaRt_CallRegistry(D2LuaRuntime* pRt, uint32_t nGame, const char* pszRegistry, const char* pszId, int nArgs);

// Call the core's host function pszName (core/internal/host.ts) for game nGame with the nArgs values
// on top, leaving nResults results; false (logged, nothing left) when it failed.
bool D2LuaRt_CallHost(D2LuaRuntime* pRt, uint32_t nGame, const char* pszName, int nArgs, int nResults);

// Every mod's storage for a game as bytes (see d2_lua_storage.cpp for the format and the rules).
bool D2LuaRt_SaveStorage(D2LuaRuntime* pRt, uint32_t nGame, std::string& sOut, std::string& sError);

// Reload a game's mods: storage is saved, what was registered through the game is dropped, every
// mod's modules are loaded afresh, each export is called again with the same game and storage is
// handed back (game.loaded is true). Other games keep what they had.
bool D2LuaRt_ReloadGame(D2LuaRuntime* pRt, uint32_t nGame, std::string& sError);

lua_State* D2LuaRt_State(D2LuaRuntime* pRt);
int D2LuaRt_Side(D2LuaRuntime* pRt);
// The mods in load order (redivivus first), and whether one has faulted.
const std::vector<D2LuaModInfo>& D2LuaRt_Mods(D2LuaRuntime* pRt);
bool D2LuaRt_IsFaulted(D2LuaRuntime* pRt, const char* pszMod);
// Run a chunk as mod pszMod with the game nGame current (what a test does from outside).
int D2LuaRt_Run(D2LuaRuntime* pRt, uint32_t nGame, const char* pszMod, const char* pszSource);

// Every setting declared in this state, all mods, with its value.
const std::vector<D2LuaSettingInfo>& D2LuaRt_Settings(D2LuaRuntime* pRt);

void D2LuaRt_SetTransport(D2LuaRuntime* pRt, const D2LuaTransport& tTransport);
// A message of game nGame that arrived from peer nPeer (on a client: 0, the server): handed to the
// sending mod's half on this side. False (logged) when it is malformed or nobody receives it.
bool D2LuaRt_Deliver(D2LuaRuntime* pRt, uint32_t nGame, uint32_t nPeer, const void* pData, size_t nSize);

// A transport inside one process: a server runtime and its clients, messages queued and delivered
// in send order by Pump (which returns how many it delivered).
struct D2LuaLoopback;
D2LuaLoopback* D2LuaLoopback_Create(D2LuaRuntime* pServer);
void D2LuaLoopback_AddClient(D2LuaLoopback* pLoop, D2LuaRuntime* pClient, uint32_t nPeer);
int D2LuaLoopback_Pump(D2LuaLoopback* pLoop);
void D2LuaLoopback_Destroy(D2LuaLoopback* pLoop);

void D2LuaRt_Destroy(D2LuaRuntime* pRt);
#endif
