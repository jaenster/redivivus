// Function overrides: every named function of the compiled tree can be replaced from a mod's
// patch(engine), each override getting next() to run what is below it (other mods' overrides in
// dependency order, at the bottom the original).
//
// The registry is generated (Redivivus/Lua/generated/override/, from the tree's own definitions by
// Redivivus/Lua/tools/src/overrides.ts): for each function its engine path, its entry, a thunk with
// the function's own calling convention and the call of the original from Lua.
//
// The mechanism. A mods build compiles every unit with -fpatchable-function-entry=5,0 and functions
// aligned to 16 bytes: five nops at each entry. Overriding a function replaces them with a jump to
// its thunk, in one locked 8-byte store, while every other thread is suspended and none stands
// inside those five bytes (on Windows; the host test elsewhere has no thread running the entry).
// The thunk calls the original five bytes past its entry, so nothing is relocated, and a function
// nobody overrides runs its five nops. Removing the last override stores the nops back, which needs
// no thread to stop. The nops sit after the entry and not before it because GNU as for PE adds a
// symbol's offset inside its COMDAT section to the calls the same object makes to it: an inline
// function whose label did not start its section would be called five bytes in.
//
// Sides and threads. Each override belongs to one Lua state. A thunk asks the state bound to the
// calling thread for the function's side (D2LuaOv_BindThread: the server state of the game the
// thread is processing, the client state on the client's thread); a thread whose state has no
// override of the function runs the original. An override that raises faults its mod: the mod's
// overrides are removed from the state and the call goes on with what is below.
#pragma once

#if D2_FEATURE_MODS
#include <stddef.h>

#include <string>

#include "Redivivus/Lua/d2_lua_override_gen.h"

struct D2LuaRuntime;
struct lua_State;

// The registry of this executable: the generated tables (null-terminated list), set by the generated
// index (the game's) or the host test's own.
extern const D2LuaOvTable* const g_apD2LuaOvTables[];

// How many functions the registry holds, and how many of them can be overridden. Built on first use.
int D2LuaOv_Count(void);
int D2LuaOv_Overridable(void);
const D2LuaOvEntry* D2LuaOv_Entry(int nId);
// The entry at an engine path, or null.
const D2LuaOvEntry* D2LuaOv_Find(const char* pszPath);
// Why the function cannot be overridden, or null when it can.
const char* D2LuaOv_RefusalText(int nRefused);
// Whether this build can patch entries (an x86 target); elsewhere a thunk only runs when called.
bool D2LuaOv_CanPatch(void);

// The state a thread's thunks use for one side (D2LUA_SIDE_SERVER or D2LUA_SIDE_CLIENT). Returns the
// state bound before, so a scope can put it back.
D2LuaRuntime* D2LuaOv_BindThread(int nSide, D2LuaRuntime* pRt);
D2LuaRuntime* D2LuaOv_ThreadRuntime(int nSide);

// The state is made (its patch() exports are about to run) / about to be destroyed: everything its
// mods overrode is removed and the entries nobody else overrides are put back.
void D2LuaOv_StateCreated(D2LuaRuntime* pRt);
void D2LuaOv_StateDestroyed(D2LuaRuntime* pRt);
// Every override of the state (a reload adds them again from patch()).
void D2LuaOv_ClearState(D2LuaRuntime* pRt);
// patch() has run; overrides now fire. Before it nothing a mod registers is called.
void D2LuaOv_StateReady(D2LuaRuntime* pRt, bool bReady);

// Put back every patched entry, whatever overrides it: the process is about to restart or end, or a
// test checks the bytes. Returns how many entries were restored.
int D2LuaOv_UndoAll(void);
// Entries patched now, and every patch and restore made since the start (the journal).
int D2LuaOv_PatchedCount(void);
struct D2LuaOvJournal {
    int nPatches;
    int nRestores;
};
D2LuaOvJournal D2LuaOv_JournalTotals(void);

// For the natives (d2_lua_core.cpp): add an override of function nId for mod nModId in the state of L,
// the function at the top of the stack (popped). bRaw: the mod's raw tier (pointers as cdata).
// Returns null, or the reason it was refused (the caller raises).
const char* D2LuaOv_Add(lua_State* L, int nId, int nModId, bool bRaw);
// What resolves at an engine path in this state's side: 0 nothing, 1 a namespace, 2 a function (*pnId).
int D2LuaOv_Lookup(lua_State* L, const char* pszPath, int* pnId);
// Whether overrides may be added now (inside patch()).
bool D2LuaOv_Patching(lua_State* L);
void D2LuaOv_SetPatching(lua_State* L, bool bPatching);

// A line for the log: the registry's size and what was refused, by reason.
std::string D2LuaOv_Summary(void);
// The functions found without a patch area at run time (the first nMax), with their first bytes.
std::string D2LuaOv_NoEntryList(int nMax);
#endif
