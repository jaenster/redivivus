// The mod runtime: one Lua state for one side (data, server or client) running a resolved mod
// list (d2_lua_mods.h) on the sandboxed host (d2_lua_host.h), with the API the declarations in
// Redivivus/Lua/api define (d2_lua_api.h).
//
// What each mod gets, per side:
//   data      settings.lua, data.lua, data-updates.lua, data-final-fixes.lua (each stage over all
//             mods in load order before the next stage); require of anything in a mod except
//             server/ and client/.
//   server    server/control.lua; require of server/ and shared/.
//   client    client/control.lua; require of client/ and shared/.
// require("x.y") or require("x/y") names a module relative to the requiring mod's root;
// require("__other__/shared/x") reaches another mod, only one the requiring mod declares a
// dependency on (required or optional). A module runs in the requiring mod's environment and is
// cached per requiring mod, so each mod gets its own instance of a shared module.
//
// The control stage: script (events, on_init, on_load, scripts), remote, registries, storage,
// log. Faults are isolated per mod: a faulted mod is skipped and the others carry on.
#pragma once

#if D2_FEATURE_MODS
#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_api.h"
#include "Redivivus/Lua/d2_lua_host.h"
#include "Redivivus/Lua/d2_lua_json.h"
#include "Redivivus/Lua/d2_lua_mods.h"

struct D2LuaRuntimeDesc {
    int nSide = D2LUA_SIDE_SERVER;
    std::vector<D2LuaModInfo> aMods;           // in load order (D2LuaMods_Resolve)
    std::vector<const D2LuaApiDesc*> aApis;    // the declaration sets bound into this state
    D2LuaLogFn pfnLog = nullptr;
    D2LuaLimits tLimits = { 0, 0 };
    std::string sSettingsFile;  // the mods' settings file; empty or missing: every setting has its default
    bool bSettingsStageOnly = false;  // data side: run settings.lua and none of the data stage files
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
};

// Mod messages (d2_lua_messages.cpp): the transport a runtime sends through. pfnSend gets the whole
// message (envelope and payload) and must copy it; it never throws. On the server nPeer names the
// receiving client (0: every client); from a client it is 0.
#define D2LUA_MESSAGE_MAX 4096
struct D2LuaTransport {
    void (*pfnSend)(void* pUser, D2LuaRuntime* pFrom, uint32_t nPeer, const void* pData, size_t nSize);
    void* pUser;
};

// Create the state and run every mod's entry files for the side, in load order. The control
// stage then waits for D2LuaRt_Start. Returns null (logged) when the state cannot be made; a mod
// whose files fail is logged and the others still run.
D2LuaRuntime* D2LuaRt_Create(const D2LuaRuntimeDesc& tDesc);

// Control stage: a new game (pStorage null: every mod's on_init) or a loaded one (storage
// restored from D2LuaRt_SaveStorage's bytes, then every mod's on_load). False with sError when
// the bytes are not a valid save; the mods then start as a new game.
bool D2LuaRt_Start(D2LuaRuntime* pRt, const std::string* pStorage, std::string& sError);

// Advance one tick: raise on_tick, then resume the scripts that are due, in the order they started.
void D2LuaRt_Tick(D2LuaRuntime* pRt);
long long D2LuaRt_CurrentTick(D2LuaRuntime* pRt);

// Raise an event: the payload table is at the top of the state's stack (popped); name and tick
// are set on it. Every mod's handler runs in load order, each as its own guarded entry, then the
// scripts waiting for the event resume. False when the side has no such event.
bool D2LuaRt_RaiseEvent(D2LuaRuntime* pRt, const char* pszEvent);

// Call registry entry pszId of pszRegistry (pszId null for a slot registry) with the nArgs
// values at the top of the stack; on D2LUA_OK leaves nResults results.
int D2LuaRt_CallRegistry(D2LuaRuntime* pRt, const char* pszRegistry, const char* pszId, int nArgs, int nResults);

// Every mod's storage as bytes (see d2_lua_storage.cpp for the format and the rules).
bool D2LuaRt_SaveStorage(D2LuaRuntime* pRt, std::string& sOut, std::string& sError);

// Tear the control stage down and build it again from the same mods: storage is saved, the
// state destroyed and recreated, the control files run, storage handed back and on_load run.
// Running scripts do not survive (a coroutine cannot be saved); mods restart them in on_load.
bool D2LuaRt_Reload(D2LuaRuntime* pRt, std::string& sError);

lua_State* D2LuaRt_State(D2LuaRuntime* pRt);
int D2LuaRt_Side(D2LuaRuntime* pRt);
// The mods in load order, and whether one has faulted.
const std::vector<D2LuaModInfo>& D2LuaRt_Mods(D2LuaRuntime* pRt);
bool D2LuaRt_IsFaulted(D2LuaRuntime* pRt, const char* pszMod);

// Every setting declared in this state, all mods, with its value.
const std::vector<D2LuaSettingInfo>& D2LuaRt_Settings(D2LuaRuntime* pRt);

void D2LuaRt_SetTransport(D2LuaRuntime* pRt, const D2LuaTransport& tTransport);
// A message that arrived from peer nPeer (on a client: 0, the server): handed to the sending mod's
// half on this side as a guarded entry. False (logged) when it is malformed or nobody receives it.
bool D2LuaRt_Deliver(D2LuaRuntime* pRt, uint32_t nPeer, const void* pData, size_t nSize);

// A transport inside one process: a server runtime and its clients, messages queued and delivered
// in send order by Pump (which returns how many it delivered).
struct D2LuaLoopback;
D2LuaLoopback* D2LuaLoopback_Create(D2LuaRuntime* pServer);
void D2LuaLoopback_AddClient(D2LuaLoopback* pLoop, D2LuaRuntime* pClient, uint32_t nPeer);
int D2LuaLoopback_Pump(D2LuaLoopback* pLoop);
void D2LuaLoopback_Destroy(D2LuaLoopback* pLoop);

void D2LuaRt_Destroy(D2LuaRuntime* pRt);
#endif
