// The sandboxed Lua state. See d2_lua_host.h and d2_lua_internal.h.
//
// LuaJIT raises errors by unwinding through the C frames between the throw and the pcall, so
// no function here that Lua can call (directly or through a lua_* call that may raise) holds an
// object with a destructor. Plain C style throughout; the C++ objects (the watchdog's thread,
// mutex and condition variable, the mod name list) live in the host context and are only
// touched outside Lua.
//
// The guards:
//   - Environments. Mods never run in the state's global table. Each mod has an environment of
//     its own holding copies of the library tables (copies rather than read-only proxies:
//     a proxy is empty to pairs, next and rawget, which breaks ordinary code, while a copy is a
//     few hundred table slots per mod and a mod may change its own freely). Every route back to
//     the global table is closed: getfenv and setfenv are gone, load and loadstring bind the
//     chunk to the caller's environment (stock load gives it the thread's, i.e. the globals),
//     newproxy is gone (userdata __gc would run mod code at collection time, where hooks are
//     off), and the string metatable is locked (__metatable) with its __index a private copy.
//   - Time. Every entry from the host into a mod goes through GuardedCall, CallAs or ResumeAs,
//     which arm a deadline. A watchdog thread waits for it and, when it passes, sets a count hook
//     with a count of 1 from that thread (lua_sethook is documented safe to call asynchronously;
//     LuaJIT's hook state is global, so it reaches whichever coroutine runs). The hook raises an
//     error on every instruction of mod code until the host disarms it, so a pcall in the mod
//     cannot hold on: the loop around the pcall is itself an instruction. LuaJIT runs an xpcall
//     message handler at the point of the error, which for the hook's error is inside the hook
//     with hooks off, so the mods' xpcall skips the handler once the deadline has passed. Nothing
//     else runs mod code with hooks off: no __gc (newproxy is gone, and tables have no finalizers
//     in LuaJIT), no other message handlers. LuaJIT is built with LUAJIT_ENABLE_CHECKHOOK so
//     compiled traces test the hook mask and fall back to the interpreter. A count hook set for
//     the whole call and reading a clock would be simpler, but a set hook stops trace recording
//     and makes every trace exit, so mods would always run interpreted; CHECKHOOK is meant for
//     hooks that are almost never set.
//     The budget is per entry: a coroutine created in one entry and resumed in a later one gets
//     the later entry's budget, never one for its lifetime.
//   - The core is never cut halfway. The core (Redivivus/Lua/core, compiled TypeScript) runs in
//     this state beside the mods and keeps shared state (registries, scripts, handlers). Its
//     chunks are named "=[core] ...", a name no mod can give a chunk (load refuses it). When the
//     hook fires inside a core function it charges the mod the core is working for (the owner on
//     top) and returns, leaving the hook set: the error is raised at the first instruction of mod
//     code after it, or, when the core returns to the host first, the entry still ends as a fault.
//     So a deadline never lands between two steps of a core update. The cost is that a core
//     function working over a large table a mod handed it finishes before the mod is stopped;
//     such work is bounded by the memory limit.
//   - Memory. The state's allocator counts bytes. Past the limit, while a mod runs, it marks the
//     entry and wakes the watchdog, which sets the same hook: the mod is stopped at its next
//     instruction, the core finishes what it was doing. Past the hard limit (the limit plus a
//     quarter, at least 16 MiB) an allocation is refused outright and LuaJIT raises its
//     out-of-memory error there; either way the mod charged is faulted. Outside a mod call the
//     host's own allocations are not refused, so the host never meets an error where it has no
//     pcall.
//   - Patterns. The mods' string.find, match, gmatch and gsub (their own tables and the string
//     metatable's __index) are d2_lua_pattern.cpp's, which poll the deadline. The global string
//     table keeps the stock functions, for the host.
//   - Faults are charged to the innermost owner (see the owner stack in d2_lua_internal.h).
//
// Known limits (the hook only runs between Lua instructions, so one long C call is not cut):
//   - table.sort with the default comparison on a table as big as the memory limit allows runs
//     for seconds in one call (with a Lua comparison function the hook does cut it).
//   - string.rep, table.concat, string.format and friends are bounded by the memory limit: the
//     result has to be allocated, and the copy is linear in it.
//   - The memory limit is per state, not per mod. JIT machine code (mcode) is mapped outside the
//     allocator and is not counted; LuaJIT caps it itself (the maxmcode parameter).
#if D2_FEATURE_MODS
#include "Redivivus/Lua/d2_lua_host.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <new>
#include <string>
#include <thread>
#include <vector>

#include "Redivivus/Lua/d2_lua_internal.h"
#include "lua.hpp"

namespace {

typedef std::chrono::steady_clock Clock;

const int kMaxOwners = 256;

struct HostContext {
    D2LuaLogFn pfnLog = nullptr;
    lua_State* L = nullptr;

    // Memory, touched on the Lua thread (bMemOver is read by the watchdog too).
    size_t nUsed = 0;
    size_t nLimit = 0;  // past it the mod is stopped at its next instruction
    size_t nHard = 0;   // past it an allocation is refused
    int bEnforce = 0;   // refuse growth past the limits (only while a mod runs)
    int bRefused = 0;   // an allocation was refused during the current call
    std::atomic<bool> bMemOver{false};  // the limit was passed during the current call

    // Time.
    unsigned nBudgetMs = 0;
    int nDepth = 0;  // nesting of guarded entries; only the outermost arms the deadline
    std::mutex tMutex;
    std::condition_variable tWake;
    std::thread tWatchdog;
    bool bStop = false;
    bool bArmed = false;
    bool bHookSet = false;            // the watchdog set the hook for the current call
    std::atomic<bool> bFired{false};  // the deadline passed
    Clock::time_point tDeadline;

    // Mod ids and the owner stack.
    std::vector<std::string> aModNames;
    int aOwner[kMaxOwners];
    lua_State* aOwnerThread[kMaxOwners];
    int nOwners = 0;
    int nFaultOwner = -1;  // charged for the current outer entry's fault, or -1

    D2LuaEnvHook pfnEnvHook = nullptr;
    D2LuaErrorHook pfnErrorHook = nullptr;
    void* pHookUser = nullptr;
};

const unsigned kDefaultBudgetMs = 200;
const size_t kDefaultMemoryBytes = (size_t)128 * 1024 * 1024;

}  // namespace

// Registry keys: the address of each variable.
static char s_ModsKey;      // mod name -> environment table, or false once faulted
static char s_TemplateKey;  // { base = {...}, string = {...}, ..., load = stock load, wrap = factory }
static char s_CoOwnersKey;  // thread -> mod id, weak keys

static const char* const s_apszBaseNames[] = {
    "assert", "error", "ipairs", "next", "pairs", "pcall", "xpcall", "rawequal", "rawget",
    "rawset", "select", "setmetatable", "getmetatable", "tonumber", "tostring", "type",
    "unpack", "gcinfo", "print", "_VERSION",
};
static const char* const s_apszLibNames[] = { "string", "table", "math", "bit", "coroutine" };

static HostContext* GetContext(lua_State* L) {
    void* pContext = nullptr;
    lua_getallocf(L, &pContext);
    return (HostContext*)pContext;
}

static int TopOwner(HostContext* pContext) {
    if (pContext->nOwners <= 0) {
        return -1;
    }
    int nTop = pContext->nOwners <= kMaxOwners ? pContext->nOwners - 1 : kMaxOwners - 1;
    return pContext->aOwner[nTop];
}

static void* Lua_Alloc(void* pUser, void* pBlock, size_t nOld, size_t nNew) {
    HostContext* pContext = (HostContext*)pUser;
    if (!pBlock) {
        nOld = 0;
    }
    if (nNew == 0) {
        free(pBlock);
        pContext->nUsed -= nOld;
        return nullptr;
    }
    if (nNew > nOld && pContext->bEnforce) {
        size_t nAfter = pContext->nUsed - nOld + nNew;
        if (nAfter > pContext->nHard) {
            if (!pContext->bRefused && pContext->nFaultOwner < 0) {
                pContext->nFaultOwner = TopOwner(pContext);
            }
            pContext->bRefused = 1;
            return nullptr;
        }
        if (nAfter > pContext->nLimit && !pContext->bMemOver.load(std::memory_order_relaxed)) {
            if (pContext->nFaultOwner < 0) {
                pContext->nFaultOwner = TopOwner(pContext);
            }
            pContext->bMemOver = true;
            pContext->tWake.notify_one();
        }
    }
    void* pNew = realloc(pBlock, nNew);
    if (!pNew) {
        // Lua requires a shrink to succeed; the old block is still valid and big enough.
        return nNew <= nOld ? pBlock : nullptr;
    }
    pContext->nUsed = pContext->nUsed - nOld + nNew;
    return pNew;
}

// --- mod ids and owners --------------------------------------------------------------------------

int D2LuaHost_ModId(lua_State* L, const char* pszMod) {
    HostContext* pContext = GetContext(L);
    for (size_t i = 0; i < pContext->aModNames.size(); i++) {
        if (pContext->aModNames[i] == pszMod) {
            return (int)i;
        }
    }
    pContext->aModNames.push_back(pszMod);
    return (int)pContext->aModNames.size() - 1;
}

const char* D2LuaHost_ModName(lua_State* L, int nModId) {
    HostContext* pContext = GetContext(L);
    if (nModId < 0 || (size_t)nModId >= pContext->aModNames.size()) {
        return nullptr;
    }
    return pContext->aModNames[(size_t)nModId].c_str();
}

void D2LuaHost_PushOwner(lua_State* L, int nModId) {
    HostContext* pContext = GetContext(L);
    if (pContext->nOwners < kMaxOwners) {
        pContext->aOwner[pContext->nOwners] = nModId;
        pContext->aOwnerThread[pContext->nOwners] = L;
    }
    pContext->nOwners++;
}

void D2LuaHost_PopOwner(lua_State* L) {
    HostContext* pContext = GetContext(L);
    if (pContext->nOwners > 0) {
        pContext->nOwners--;
    }
}

int D2LuaHost_OwnerDepth(lua_State* L) {
    return GetContext(L)->nOwners;
}

void D2LuaHost_RestoreOwnerDepth(lua_State* L, int nDepth) {
    GetContext(L)->nOwners = nDepth;
}

int D2LuaHost_CurrentOwner(lua_State* L) {
    HostContext* pContext = GetContext(L);
    if (pContext->nOwners == 0) {
        return -1;
    }
    int nTop = pContext->nOwners <= kMaxOwners ? pContext->nOwners - 1 : kMaxOwners - 1;
    return pContext->aOwner[nTop];
}

int D2LuaHost_CoroutineOwner(lua_State* L, lua_State* co) {
    lua_pushlightuserdata(L, &s_CoOwnersKey);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return -1;
    }
    lua_pushthread(co);
    lua_xmove(co, L, 1);
    lua_rawget(L, -2);
    int nOwner = lua_isnumber(L, -1) ? (int)lua_tointeger(L, -1) : -1;
    lua_pop(L, 2);
    return nOwner;
}

void D2LuaHost_SetCoroutineOwner(lua_State* L, int nThreadIndex, int nModId) {
    nThreadIndex = nThreadIndex < 0 ? lua_gettop(L) + nThreadIndex + 1 : nThreadIndex;
    lua_pushlightuserdata(L, &s_CoOwnersKey);
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_pushvalue(L, nThreadIndex);
    lua_pushinteger(L, nModId);
    lua_rawset(L, -3);
    lua_pop(L, 1);
}

// The mod a deadline fault on thread L is charged to (see d2_lua_internal.h).
static int ChargeForDeadline(lua_State* L, HostContext* pContext) {
    if (pContext->nOwners > 0) {
        int nTop = pContext->nOwners <= kMaxOwners ? pContext->nOwners - 1 : kMaxOwners - 1;
        if (pContext->aOwnerThread[nTop] == L) {
            return pContext->aOwner[nTop];
        }
    }
    int nCoOwner = D2LuaHost_CoroutineOwner(L, L);
    if (nCoOwner >= 0) {
        return nCoOwner;
    }
    return D2LuaHost_CurrentOwner(L);
}

// --- the deadline ------------------------------------------------------------------------------

// Whether the function the hook interrupted is the core's (its chunk is named "=[core] ...").
static bool InCore(lua_State* L) {
    lua_Debug tAr;
    if (!lua_getstack(L, 0, &tAr) || !lua_getinfo(L, "S", &tAr) || !tAr.source) {
        return false;
    }
    return strncmp(tAr.source, D2LUA_CORE_CHUNK_PREFIX, sizeof(D2LUA_CORE_CHUNK_PREFIX) - 1) == 0;
}

// Set by the watchdog once the deadline passed or the memory limit did. Raises on every instruction
// of mod code; inside the core it only charges and returns (see the header).
static void Lua_BudgetHook(lua_State* L, lua_Debug* pDebug) {
    (void)pDebug;
    HostContext* pContext = GetContext(L);
    bool bTime = pContext->bFired;
    bool bMemory = pContext->bMemOver;
    if (!bTime && !bMemory) {
        return;
    }
    if (pContext->nFaultOwner < 0) {
        pContext->nFaultOwner = ChargeForDeadline(L, pContext);
    }
    if (InCore(L)) {
        return;
    }
    if (bMemory) {
        lua_pushliteral(L, "not enough memory (the mod's state is over its limit)");
    } else {
        lua_pushliteral(L, "time budget exceeded");
    }
    lua_error(L);
}

// Arming and disarming take the mutex but never wake the watchdog: an idle watchdog looks again a
// quarter of a budget later, which is before any deadline armed meanwhile can pass.
static void Watchdog(HostContext* pContext) {
    std::unique_lock<std::mutex> tLock(pContext->tMutex);
    while (!pContext->bStop) {
        if (pContext->bArmed && !pContext->bHookSet && pContext->bMemOver) {
            pContext->bHookSet = true;
            lua_sethook(pContext->L, Lua_BudgetHook, LUA_MASKCOUNT, 1);
        }
        if (pContext->bArmed && !pContext->bFired) {
            if (Clock::now() >= pContext->tDeadline) {
                pContext->bFired = true;
                if (!pContext->bHookSet) {
                    pContext->bHookSet = true;
                    lua_sethook(pContext->L, Lua_BudgetHook, LUA_MASKCOUNT, 1);
                }
                continue;
            }
            pContext->tWake.wait_until(tLock, pContext->tDeadline);
        } else {
            pContext->tWake.wait_for(tLock, std::chrono::milliseconds(pContext->nBudgetMs / 4 + 1));
        }
    }
}

static void ArmDeadline(HostContext* pContext) {
    std::lock_guard<std::mutex> tLock(pContext->tMutex);
    pContext->tDeadline = Clock::now() + std::chrono::milliseconds(pContext->nBudgetMs);
    pContext->bArmed = true;
    pContext->bFired = false;
    pContext->bHookSet = false;
}

// Returns whether the deadline had passed. After this the watchdog cannot set the hook.
static bool DisarmDeadline(HostContext* pContext) {
    bool bFired;
    bool bHookSet;
    {
        std::lock_guard<std::mutex> tLock(pContext->tMutex);
        pContext->bArmed = false;
        bFired = pContext->bFired;
        bHookSet = pContext->bHookSet;
        pContext->bFired = false;
        pContext->bHookSet = false;
    }
    if (bHookSet) {
        lua_sethook(pContext->L, nullptr, 0, 0);
    }
    return bFired;
}

int D2LuaHost_DeadlinePassed(lua_State* L) {
    HostContext* pContext = GetContext(L);
    if (pContext->bFired || pContext->bMemOver) {
        if (pContext->nFaultOwner < 0) {
            pContext->nFaultOwner = ChargeForDeadline(L, pContext);
        }
        return 1;
    }
    return 0;
}

int D2LuaHost_MemoryRefused(lua_State* L) {
    HostContext* pContext = GetContext(L);
    return pContext->bRefused || pContext->bMemOver;
}

int D2LuaHost_InEntry(lua_State* L) {
    return GetContext(L)->nDepth > 0;
}

// Whether the current call ran out of time or memory. For the xpcall wrapper below.
static int Lua_BudgetSpent(lua_State* L) {
    HostContext* pContext = GetContext(L);
    lua_pushboolean(L, pContext->bFired || pContext->bMemOver ? 1 : 0);
    return 1;
}

void D2LuaHost_LogErrorTop(lua_State* L, const char* pszMod);

// --- logging -----------------------------------------------------------------------------------

static D2LuaLogFn GetLog(lua_State* L) {
    return GetContext(L)->pfnLog;
}

// Hand text to the log one line at a time, each prefixed.
static void LogLines(D2LuaLogFn pfnLog, const char* pszPrefix, const char* pszText, size_t nLen) {
    char szLine[1024];
    size_t nPrefix = strlen(pszPrefix);
    if (!pfnLog) {
        return;
    }
    size_t nPos = 0;
    do {
        size_t nEnd = nPos;
        while (nEnd < nLen && pszText[nEnd] != '\n') {
            nEnd++;
        }
        size_t nPart = nEnd - nPos;
        if (nPart > sizeof(szLine) - nPrefix - 1) {
            nPart = sizeof(szLine) - nPrefix - 1;
        }
        memcpy(szLine, pszPrefix, nPrefix);
        memcpy(szLine + nPrefix, pszText + nPos, nPart);
        szLine[nPrefix + nPart] = '\0';
        pfnLog(szLine);
        nPos = nEnd + 1;
    } while (nPos < nLen);
}

static void LogMessage(lua_State* L, const char* pszPrefix, const char* pszText) {
    LogLines(GetLog(L), pszPrefix, pszText, strlen(pszText));
}

void D2LuaHost_Log(lua_State* L, const char* pszLine) {
    LogMessage(L, "", pszLine);
}

// print(...): the arguments through tostring, tab separated, as Lua 5.1's print does, to the log.
static int Lua_Print(lua_State* L) {
    int nArgs = lua_gettop(L);
    luaL_Buffer tBuffer;
    lua_getglobal(L, "tostring");
    // The buffer may push stack slots of its own, so tostring is reached by its absolute index.
    luaL_buffinit(L, &tBuffer);
    for (int i = 1; i <= nArgs; i++) {
        lua_pushvalue(L, nArgs + 1);
        lua_pushvalue(L, i);
        lua_call(L, 1, 1);
        if (!lua_isstring(L, -1)) {
            return luaL_error(L, "'tostring' must return a string to 'print'");
        }
        if (i > 1) {
            luaL_addchar(&tBuffer, '\t');
        }
        luaL_addvalue(&tBuffer);
    }
    luaL_pushresult(&tBuffer);
    size_t nLen = 0;
    const char* pszText = lua_tolstring(L, -1, &nLen);
    LogLines(GetLog(L), "[lua] ", pszText, nLen);
    return 0;
}

// The message handler for every protected call: the error and a traceback.
static int Lua_Traceback(lua_State* L) {
    const char* pszMessage = lua_tostring(L, 1);
    if (!pszMessage) {
        if (luaL_callmeta(L, 1, "__tostring") && lua_type(L, -1) == LUA_TSTRING) {
            pszMessage = lua_tostring(L, -1);
        } else {
            pszMessage = lua_pushfstring(L, "(error object is a %s value)", luaL_typename(L, 1));
        }
    }
    luaL_traceback(L, L, pszMessage, 1);
    return 1;
}

static char s_TracebackKey;  // the traceback of the last error a CallAs or ResumeAs caught

// The message handler of CallAs: keeps the error value as it is (the core passes it on to the mod
// that called) and stores a traceback beside it. Never calls a __tostring of the mod's.
static int Lua_KeepTraceback(lua_State* L) {
    const char* pszMessage = lua_type(L, 1) == LUA_TSTRING ? lua_tostring(L, 1) : nullptr;
    if (!pszMessage) {
        pszMessage = lua_pushfstring(L, "(error object is a %s value)", luaL_typename(L, 1));
    }
    lua_pushlightuserdata(L, &s_TracebackKey);
    luaL_traceback(L, L, pszMessage, 1);
    lua_rawset(L, LUA_REGISTRYINDEX);
    lua_settop(L, 1);
    return 1;
}

static int Lua_Panic(lua_State* L) {
    const char* pszMessage = lua_tostring(L, -1);
    LogMessage(L, "[lua] PANIC: ", pszMessage ? pszMessage : "(no message)");
    return 0;
}

// Log the error string at the top of the stack as mod pszMod's, through the error hook. Pops it.
static void LogError(lua_State* L, const char* pszMod) {
    HostContext* pContext = GetContext(L);
    const char* pszError = lua_tostring(L, -1);
    if (!pszError) {
        pszError = "(no message)";
    }
    char szPrefix[160];
    snprintf(szPrefix, sizeof(szPrefix), "[lua] %s: ", pszMod);
    if (pContext->pfnErrorHook) {
        static thread_local char s_szMapped[16384];
        pContext->pfnErrorHook(pszError, s_szMapped, sizeof(s_szMapped), pContext->pHookUser);
        LogMessage(L, szPrefix, s_szMapped);
    } else {
        LogMessage(L, szPrefix, pszError);
    }
    lua_pop(L, 1);
}

void D2LuaHost_LogErrorTop(lua_State* L, const char* pszMod) {
    LogError(L, pszMod);
}

// --- the functions a mod environment gets in place of the stock ones ----------------------------

// load and loadstring: the stock load with the mode forced to text, so bytecode is refused,
// and the chunk bound to the caller's environment (upvalue 2) unless a table is passed as the
// fourth argument. Upvalue 1 is the stock load.
static int Lua_EnvLoad(lua_State* L) {
    int nArgs = lua_gettop(L);
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushvalue(L, 1);
    if (nArgs >= 2 && lua_type(L, 2) == LUA_TSTRING
        && strncmp(lua_tostring(L, 2), D2LUA_CORE_CHUNK_PREFIX, sizeof(D2LUA_CORE_CHUNK_PREFIX) - 1) == 0) {
        // The core's chunk names are its own: the watchdog lets a core function finish.
        lua_pushliteral(L, "=(load)");
    } else if (nArgs >= 2) {
        lua_pushvalue(L, 2);
    } else {
        lua_pushnil(L);
    }
    lua_pushliteral(L, "t");
    if (nArgs >= 4 && lua_istable(L, 4)) {
        lua_pushvalue(L, 4);
    } else {
        lua_pushvalue(L, lua_upvalueindex(2));
    }
    lua_call(L, 4, LUA_MULTRET);
    return lua_gettop(L) - nArgs;
}

// collectgarbage: only "count". Stopping the collector or retuning it would change the state
// every mod shares.
static int Lua_CollectGarbage(lua_State* L) {
    const char* pszOption = luaL_optstring(L, 1, "collect");
    if (strcmp(pszOption, "count") != 0) {
        return luaL_error(L, "collectgarbage('%s') is not available to mods", pszOption);
    }
    lua_pushnumber(L, (lua_Number)lua_gc(L, LUA_GCCOUNT, 0) + (lua_Number)lua_gc(L, LUA_GCCOUNTB, 0) / 1024.0);
    return 1;
}

// coroutine.create for a mod: the stock create (upvalue 1), and the new coroutine recorded as
// owned by the mod (upvalue 2, its id).
static int Lua_EnvCoCreate(lua_State* L) {
    lua_settop(L, 1);
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushvalue(L, 1);
    lua_call(L, 1, 1);
    D2LuaHost_SetCoroutineOwner(L, 2, (int)lua_tointeger(L, lua_upvalueindex(2)));
    return 1;
}

// --- building the state ------------------------------------------------------------------------

// Push a shallow copy of the table at index nIndex.
static void PushTableCopy(lua_State* L, int nIndex) {
    nIndex = nIndex < 0 ? lua_gettop(L) + nIndex + 1 : nIndex;
    lua_newtable(L);
    lua_pushnil(L);
    while (lua_next(L, nIndex)) {
        lua_pushvalue(L, -2);
        lua_insert(L, -2);
        lua_rawset(L, -4);
    }
}

static int Lua_OpenSandbox(lua_State* L) {
    // luaopen_base also opens coroutine; luaopen_jit switches the compiler on.
    static const lua_CFunction s_apfnOpen[] = {
        luaopen_base, luaopen_string, luaopen_table, luaopen_math, luaopen_bit, luaopen_jit,
    };
    for (size_t i = 0; i < sizeof(s_apfnOpen) / sizeof(s_apfnOpen[0]); i++) {
        lua_pushcfunction(L, s_apfnOpen[i]);
        lua_call(L, 0, 0);
    }
    lua_pushcfunction(L, Lua_Print);
    lua_setglobal(L, "print");
    lua_getglobal(L, "string");
    lua_pushnil(L);
    lua_setfield(L, -2, "dump");
    lua_pop(L, 1);

    // The template every mod environment is copied from.
    lua_pushlightuserdata(L, &s_TemplateKey);
    lua_newtable(L);
    lua_newtable(L);
    for (size_t i = 0; i < sizeof(s_apszBaseNames) / sizeof(s_apszBaseNames[0]); i++) {
        lua_getglobal(L, s_apszBaseNames[i]);
        lua_setfield(L, -2, s_apszBaseNames[i]);
    }
    lua_setfield(L, -2, "base");
    for (size_t i = 0; i < sizeof(s_apszLibNames) / sizeof(s_apszLibNames[0]); i++) {
        lua_getglobal(L, s_apszLibNames[i]);
        PushTableCopy(L, -1);
        if (strcmp(s_apszLibNames[i], "string") == 0) {
            D2LuaPattern_Install(L, -1);
        }
        lua_setfield(L, -3, s_apszLibNames[i]);
        lua_pop(L, 1);
    }
    lua_getglobal(L, "load");
    lua_setfield(L, -2, "load");
    // xpcall, with the message handler skipped once the deadline has passed. The budget hook
    // raises its error from inside the hook, and LuaJIT runs the message handler right there,
    // before unwinding, with hooks off: a handler that loops would never be stopped. Written in
    // Lua so a coroutine can still yield across it, as across the stock xpcall.
    static const char s_szXpcall[] =
        "local xpcall, spent = ...\n"
        "return function(f, handler, ...)\n"
        "  return xpcall(f, function(e)\n"
        "    if spent() then return e end\n"
        "    return handler(e)\n"
        "  end, ...)\n"
        "end\n";
    lua_getfield(L, -1, "base");
    if (luaL_loadbuffer(L, s_szXpcall, sizeof(s_szXpcall) - 1, "=xpcall") != 0) {
        return lua_error(L);
    }
    lua_getglobal(L, "xpcall");
    lua_pushcfunction(L, Lua_BudgetSpent);
    lua_call(L, 2, 1);
    lua_setfield(L, -2, "xpcall");
    lua_pop(L, 1);
    // coroutine.wrap for a mod, built from the mod's own create so the coroutine is owned by
    // it. A string error is raised again with the caller's position, as the stock wrap does.
    static const char s_szWrap[] =
        "local resume, type, error = ...\n"
        "local function finish(ok, ...)\n"
        "  if ok then return ... end\n"
        "  local e = ...\n"
        "  if type(e) == 'string' then error(e, 2) end\n"
        "  error(e, 0)\n"
        "end\n"
        "return function(create)\n"
        "  return function(f)\n"
        "    local co = create(f)\n"
        "    return function(...) return finish(resume(co, ...)) end\n"
        "  end\n"
        "end\n";
    if (luaL_loadbuffer(L, s_szWrap, sizeof(s_szWrap) - 1, "=coroutine.wrap") != 0) {
        return lua_error(L);
    }
    lua_getglobal(L, "coroutine");
    lua_getfield(L, -1, "resume");
    lua_remove(L, -2);
    lua_getglobal(L, "type");
    lua_getglobal(L, "error");
    lua_call(L, 3, 1);
    lua_setfield(L, -2, "wrap");
    lua_getglobal(L, "coroutine");
    lua_getfield(L, -1, "create");
    lua_remove(L, -2);
    lua_setfield(L, -2, "cocreate");
    lua_rawset(L, LUA_REGISTRYINDEX);

    lua_pushlightuserdata(L, &s_ModsKey);
    lua_newtable(L);
    lua_rawset(L, LUA_REGISTRYINDEX);

    lua_pushlightuserdata(L, &s_CoOwnersKey);
    lua_newtable(L);
    lua_newtable(L);
    lua_pushliteral(L, "k");
    lua_setfield(L, -2, "__mode");
    lua_setmetatable(L, -2);
    lua_rawset(L, LUA_REGISTRYINDEX);

    // No mod reaches the global table, but should one ever leak, it holds nothing that leads
    // further. (jit stays reachable to the host through the registry's _LOADED.)
    static const char* const s_apszRemove[] = { "jit", "dofile", "loadfile", "getfenv", "setfenv", "newproxy" };
    for (size_t i = 0; i < sizeof(s_apszRemove) / sizeof(s_apszRemove[0]); i++) {
        lua_pushnil(L);
        lua_setglobal(L, s_apszRemove[i]);
    }

    // Strings share one metatable. Lock it, so getmetatable("") hands out false and no mod can
    // reach it, and point its __index at a copy of string no mod environment holds, with the
    // step-limited pattern functions.
    lua_pushliteral(L, "");
    lua_getmetatable(L, -1);
    lua_getglobal(L, "string");
    PushTableCopy(L, -1);
    D2LuaPattern_Install(L, -1);
    lua_setfield(L, -3, "__index");
    lua_pop(L, 1);
    lua_pushboolean(L, 0);
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 2);
    return 0;
}

// Arg 1: mod name, arg 2: its id. Returns the mod's environment, creating it, or nil for a
// faulted mod.
static int Lua_ModEnv(lua_State* L) {
    lua_settop(L, 2);
    int nModId = (int)lua_tointeger(L, 2);
    lua_pushlightuserdata(L, &s_ModsKey);
    lua_rawget(L, LUA_REGISTRYINDEX);  // 3: mods
    lua_pushvalue(L, 1);
    lua_rawget(L, 3);  // 4: mods[name]
    if (lua_istable(L, 4)) {
        return 1;
    }
    if (lua_isboolean(L, 4)) {
        lua_pushnil(L);
        return 1;
    }
    lua_pop(L, 1);
    lua_pushlightuserdata(L, &s_TemplateKey);
    lua_rawget(L, LUA_REGISTRYINDEX);  // 4: template
    lua_getfield(L, 4, "base");
    PushTableCopy(L, -1);
    lua_remove(L, -2);  // 5: env
    for (size_t i = 0; i < sizeof(s_apszLibNames) / sizeof(s_apszLibNames[0]); i++) {
        lua_getfield(L, 4, s_apszLibNames[i]);
        PushTableCopy(L, -1);
        lua_setfield(L, 5, s_apszLibNames[i]);
        lua_pop(L, 1);
    }
    lua_getfield(L, 4, "load");
    lua_pushvalue(L, 5);
    lua_pushcclosure(L, Lua_EnvLoad, 2);
    lua_pushvalue(L, -1);
    lua_setfield(L, 5, "load");
    lua_setfield(L, 5, "loadstring");
    lua_pushcfunction(L, Lua_CollectGarbage);
    lua_setfield(L, 5, "collectgarbage");
    // coroutine.create and wrap that record the mod as the owner.
    lua_getfield(L, 5, "coroutine");
    lua_getfield(L, 4, "cocreate");
    lua_pushinteger(L, nModId);
    lua_pushcclosure(L, Lua_EnvCoCreate, 2);
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "create");
    lua_getfield(L, 4, "wrap");
    lua_insert(L, -2);
    lua_call(L, 1, 1);
    lua_setfield(L, -2, "wrap");
    lua_pop(L, 1);
    lua_pushvalue(L, 5);
    lua_setfield(L, 5, "_G");
    HostContext* pContext = GetContext(L);
    if (pContext->pfnEnvHook) {
        pContext->pfnEnvHook(L, 5, nModId, pContext->pHookUser);
    }
    lua_settop(L, 5);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 5);
    lua_rawset(L, 3);
    return 1;
}

// Arg 1: mod name. Marks it faulted and drops its environment, then collects what it held.
static int Lua_FaultMod(lua_State* L) {
    lua_pushlightuserdata(L, &s_ModsKey);
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_pushvalue(L, 1);
    lua_pushboolean(L, 0);
    lua_rawset(L, -3);
    // Several cycles: LuaJIT gives back its string buffer by halves, one halving per cycle, and a
    // mod that ran out of memory leaves it large.
    for (int i = 0; i < 4; i++) {
        lua_gc(L, LUA_GCCOLLECT, 0);
    }
    return 0;
}

lua_State* D2LuaHost_Create(D2LuaLogFn pfnLog, const D2LuaLimits* pLimits) {
    HostContext* pContext = new (std::nothrow) HostContext;
    if (!pContext) {
        if (pfnLog) {
            pfnLog("[lua] cannot create a Lua state (out of memory)");
        }
        return nullptr;
    }
    pContext->pfnLog = pfnLog;
    pContext->nBudgetMs = pLimits && pLimits->nBudgetMs ? pLimits->nBudgetMs : kDefaultBudgetMs;
    pContext->nLimit = pLimits && pLimits->nMemoryBytes ? pLimits->nMemoryBytes : kDefaultMemoryBytes;
    pContext->nHard = pContext->nLimit + (pContext->nLimit / 4 > (size_t)16 * 1024 * 1024 ? pContext->nLimit / 4 : (size_t)16 * 1024 * 1024);

    lua_State* L = lua_newstate(Lua_Alloc, pContext);
    if (!L) {
        if (pfnLog) {
            pfnLog("[lua] cannot create a Lua state (lua_newstate failed)");
        }
        delete pContext;
        return nullptr;
    }
    pContext->L = L;
    lua_atpanic(L, Lua_Panic);

    lua_pushcfunction(L, Lua_Traceback);
    lua_pushcfunction(L, Lua_OpenSandbox);
    if (lua_pcall(L, 0, 0, -2) != 0) {
        LogMessage(L, "[lua] cannot open the libraries: ", lua_tostring(L, -1));
        lua_close(L);
        delete pContext;
        return nullptr;
    }
    lua_pop(L, 1);

    try {
        pContext->tWatchdog = std::thread(Watchdog, pContext);
    } catch (...) {
        LogMessage(L, "", "[lua] cannot start the watchdog thread");
        lua_close(L);
        delete pContext;
        return nullptr;
    }
    return L;
}

void D2LuaHost_SetHooks(lua_State* L, D2LuaEnvHook pfnEnv, D2LuaErrorHook pfnError, void* pUser) {
    HostContext* pContext = GetContext(L);
    pContext->pfnEnvHook = pfnEnv;
    pContext->pfnErrorHook = pfnError;
    pContext->pHookUser = pUser;
}

void* D2LuaHost_HookUser(lua_State* L) {
    return GetContext(L)->pHookUser;
}

// --- running mods ------------------------------------------------------------------------------

int D2LuaHost_NewModEnv(lua_State* L, const char* pszMod) {
    int nModId = D2LuaHost_ModId(L, pszMod);
    lua_pushcfunction(L, Lua_ModEnv);
    lua_pushstring(L, pszMod);
    lua_pushinteger(L, nModId);
    if (lua_pcall(L, 2, 1, 0) != 0) {
        LogMessage(L, "[lua] cannot create a mod environment: ", lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pushnil(L);
        return -1;
    }
    return lua_istable(L, -1) ? 1 : 0;
}

int D2LuaHost_IsModFaulted(lua_State* L, const char* pszMod) {
    lua_pushlightuserdata(L, &s_ModsKey);
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_pushstring(L, pszMod);
    lua_rawget(L, -2);
    int bFaulted = lua_isboolean(L, -1);
    lua_pop(L, 2);
    return bFaulted;
}

void D2LuaHost_FaultMod(lua_State* L, const char* pszMod) {
    lua_pushcfunction(L, Lua_FaultMod);
    lua_pushstring(L, pszMod);
    if (lua_pcall(L, 1, 0, 0) != 0) {
        lua_pop(L, 1);
    }
}

// The start and end of one guarded entry. Begin returns whether it is the outermost.
static bool BeginEntry(HostContext* pContext) {
    bool bOuter = pContext->nDepth == 0;
    if (bOuter) {
        pContext->bRefused = 0;
        pContext->bMemOver = false;
        pContext->bEnforce = 1;
        pContext->nFaultOwner = -1;
        ArmDeadline(pContext);
    }
    pContext->nDepth++;
    return bOuter;
}

// Ends the entry; when it was the outermost and it ran out of time or memory, logs it, faults the
// mod charged for it and returns D2LUA_FAULT. Otherwise returns nResult. A deadline that passed
// counts even when the entry then finished: it passed inside the core, which was let finish.
static int EndEntry(lua_State* L, HostContext* pContext, bool bOuter, int nStatus, const char* pszMod, int nResult) {
    pContext->nDepth--;
    if (!bOuter) {
        return nResult;
    }
    bool bTimedOut = DisarmDeadline(pContext);
    bool bRefused = pContext->bRefused != 0 || pContext->bMemOver || nStatus == LUA_ERRMEM;
    pContext->bEnforce = 0;
    pContext->bRefused = 0;
    pContext->bMemOver = false;
    int nCharged = pContext->nFaultOwner;
    pContext->nFaultOwner = -1;
    if (!bTimedOut && !bRefused) {
        return nResult;
    }
    const char* pszCharged = D2LuaHost_ModName(L, nCharged);
    if (!pszCharged) {
        pszCharged = pszMod;
    }
    // Copy the name: faulting may drop the last reference the caller's pointer came from.
    char szCharged[128];
    snprintf(szCharged, sizeof(szCharged), "%s", pszCharged);
    char szWhy[320];
    // Memory first: a mod that swallowed its out-of-memory error and spun until the deadline
    // ran out of memory.
    if (bRefused) {
        snprintf(szWhy, sizeof(szWhy), "[lua] mod '%s' ran out of memory (limit %lu bytes) and is disabled", szCharged,
                 (unsigned long)pContext->nLimit);
    } else {
        snprintf(szWhy, sizeof(szWhy), "[lua] mod '%s' ran past its time budget of %u ms and is disabled", szCharged,
                 pContext->nBudgetMs);
    }
    LogMessage(L, "", szWhy);
    if (strcmp(szCharged, pszMod) != 0) {
        snprintf(szWhy, sizeof(szWhy), "[lua]   (in a call that entered through mod '%s', which stays enabled)", pszMod);
        LogMessage(L, "", szWhy);
    }
    if (D2LuaHost_ModName(L, nCharged)) {
        D2LuaHost_FaultMod(L, szCharged);
    }
    return D2LUA_FAULT;
}

int D2LuaHost_GuardedCall(lua_State* L, const char* pszMod, int nArgs, int nResults) {
    HostContext* pContext = GetContext(L);
    int nFunction = lua_gettop(L) - nArgs;
    lua_pushcfunction(L, Lua_Traceback);
    lua_insert(L, nFunction);
    int nOwnerDepth = pContext->nOwners;
    D2LuaHost_PushOwner(L, D2LuaHost_ModId(L, pszMod));
    bool bOuter = BeginEntry(pContext);
    int nStatus = lua_pcall(L, nArgs, nResults, nFunction);
    pContext->nOwners = nOwnerDepth;
    if (nStatus != 0) {
        LogError(L, pszMod);
        lua_settop(L, nFunction - 1);
    } else {
        lua_remove(L, nFunction);
    }
    int nResult = EndEntry(L, pContext, bOuter, nStatus, pszMod, nStatus == 0 ? D2LUA_OK : D2LUA_ERROR);
    if (nResult == D2LUA_FAULT && nStatus == 0) {
        // Finished, but its time or memory ran out on the way (inside the core, or caught by the mod).
        lua_pop(L, nResults);
    }
    return nResult;
}

// Push the traceback CallAs stored, or nil.
static void PushStoredTraceback(lua_State* L) {
    lua_pushlightuserdata(L, &s_TracebackKey);
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_pushlightuserdata(L, &s_TracebackKey);
    lua_pushnil(L);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

int D2LuaHost_CallAs(lua_State* L, int nModId, int nArgs, int* pnPushed) {
    HostContext* pContext = GetContext(L);
    int nFunction = lua_gettop(L) - nArgs;
    const char* pszMod = nModId >= 0 ? D2LuaHost_ModName(L, nModId) : nullptr;
    char szMod[128];
    snprintf(szMod, sizeof(szMod), "%s", pszMod ? pszMod : "(core)");
    if (pszMod && D2LuaHost_IsModFaulted(L, szMod)) {
        lua_settop(L, nFunction - 1);
        *pnPushed = 0;
        return D2LUA_CALL_DISABLED;
    }
    lua_pushcfunction(L, Lua_KeepTraceback);
    lua_insert(L, nFunction);
    int nOwnerDepth = pContext->nOwners;
    if (nModId >= 0) {
        D2LuaHost_PushOwner(L, nModId);
    }
    bool bOuter = BeginEntry(pContext);
    int nStatus = lua_pcall(L, nArgs, 1, nFunction);
    pContext->nOwners = nOwnerDepth;
    bool bSpent = pContext->bFired || pContext->bMemOver || pContext->bRefused || nStatus == LUA_ERRMEM;
    lua_remove(L, nFunction);  // the handler; the result or the error is on top
    int nResult = EndEntry(L, pContext, bOuter, nStatus, szMod, D2LUA_OK);
    if (nResult == D2LUA_FAULT) {
        lua_pop(L, 1);
        PushStoredTraceback(L);
        lua_pop(L, 1);
        lua_pushliteral(L, "ran out of time or memory; the mod charged is disabled");
        *pnPushed = 1;
        return D2LUA_CALL_FAULTED;
    }
    if (nStatus == 0) {
        *pnPushed = 1;
        return D2LUA_CALL_OK;
    }
    if (!bOuter && bSpent) {
        PushStoredTraceback(L);
        lua_pop(L, 1);
        *pnPushed = 1;
        return D2LUA_CALL_FATAL;
    }
    PushStoredTraceback(L);
    *pnPushed = 2;
    return D2LUA_CALL_ERROR;
}

int D2LuaHost_ResumeAs(lua_State* L, lua_State* co, int nModId, int nArgs, int* pnResults) {
    HostContext* pContext = GetContext(L);
    const char* pszMod = D2LuaHost_ModName(L, nModId);
    char szMod[128];
    snprintf(szMod, sizeof(szMod), "%s", pszMod ? pszMod : "(core)");
    *pnResults = 0;
    if (!pszMod || D2LuaHost_IsModFaulted(L, szMod)) {
        lua_settop(co, 0);
        return D2LUA_CALL_DISABLED;
    }
    int nOwnerDepth = pContext->nOwners;
    D2LuaHost_PushOwner(co, nModId);
    bool bOuter = BeginEntry(pContext);
    int nStatus = lua_resume(co, nArgs);
    pContext->nOwners = nOwnerDepth;
    bool bSpent = pContext->bFired || pContext->bMemOver || pContext->bRefused || nStatus == LUA_ERRMEM;
    int nResult = EndEntry(L, pContext, bOuter, nStatus == LUA_YIELD ? 0 : nStatus, szMod, D2LUA_OK);
    if (nResult == D2LUA_FAULT) {
        lua_settop(co, 0);
        return D2LUA_CALL_FAULTED;
    }
    if (nStatus == LUA_YIELD || nStatus == 0) {
        *pnResults = lua_gettop(co);
        return nStatus == LUA_YIELD ? D2LUA_CALL_YIELD : D2LUA_CALL_OK;
    }
    // The error value on co, and a traceback of co.
    const char* pszMessage = lua_type(co, -1) == LUA_TSTRING ? lua_tostring(co, -1) : nullptr;
    char szType[64];
    if (!pszMessage) {
        snprintf(szType, sizeof(szType), "(error object is a %s value)", luaL_typename(co, -1));
        pszMessage = szType;
    }
    luaL_traceback(L, co, pszMessage, 0);
    lua_xmove(co, L, 1);
    lua_insert(L, -2);  // error, traceback
    lua_settop(co, 0);
    if (!bOuter && bSpent) {
        lua_pop(L, 1);
        *pnResults = 1;
        return D2LUA_CALL_FATAL;
    }
    *pnResults = 2;
    return D2LUA_CALL_ERROR;
}

static int RunBuffer(lua_State* L, const char* pszMod, const char* pBuffer, size_t nSize, const char* pszChunkName) {
    if (!L) {
        return D2LUA_ERROR;
    }
    if (D2LuaHost_IsModFaulted(L, pszMod)) {
        return D2LUA_DISABLED;
    }
    int nBase = lua_gettop(L);
    if (luaL_loadbufferx(L, pBuffer, nSize, pszChunkName, "t") != 0) {
        LogError(L, pszMod);
        lua_settop(L, nBase);
        return D2LUA_ERROR;
    }
    int nEnv = D2LuaHost_NewModEnv(L, pszMod);
    if (nEnv != 1) {
        lua_settop(L, nBase);
        return nEnv == 0 ? D2LUA_DISABLED : D2LUA_ERROR;
    }
    lua_setfenv(L, -2);
    return D2LuaHost_GuardedCall(L, pszMod, 0, 0);
}

int D2LuaHost_RunString(lua_State* L, const char* pszMod, const char* pszSource, const char* pszChunkName) {
    return RunBuffer(L, pszMod, pszSource, pszSource ? strlen(pszSource) : 0, pszChunkName);
}

int D2LuaHost_RunFile(lua_State* L, const char* pszMod, const char* pszPath, const char* pszChunkName) {
    if (!L) {
        return D2LUA_ERROR;
    }
    FILE* pFile = fopen(pszPath, "rb");
    if (!pFile) {
        return D2LUA_MISSING;
    }
    char* pBuffer = nullptr;
    long nSize = -1;
    if (fseek(pFile, 0, SEEK_END) == 0) {
        nSize = ftell(pFile);
    }
    if (nSize >= 0 && fseek(pFile, 0, SEEK_SET) == 0) {
        pBuffer = (char*)malloc((size_t)nSize + 1);
    }
    if (!pBuffer || fread(pBuffer, 1, (size_t)nSize, pFile) != (size_t)nSize) {
        fclose(pFile);
        free(pBuffer);
        char szError[600];
        snprintf(szError, sizeof(szError), "[lua] cannot read %s", pszPath);
        LogMessage(L, "", szError);
        return D2LUA_ERROR;
    }
    fclose(pFile);
    int nResult = RunBuffer(L, pszMod, pBuffer, (size_t)nSize, pszChunkName);
    free(pBuffer);
    return nResult;
}

int D2LuaHost_CallMod(lua_State* L, const char* pszMod, const char* pszFunction) {
    if (!L) {
        return D2LUA_ERROR;
    }
    int nBase = lua_gettop(L);
    int nEnv = D2LuaHost_NewModEnv(L, pszMod);
    if (nEnv != 1) {
        lua_settop(L, nBase);
        return nEnv == 0 ? D2LUA_DISABLED : D2LUA_ERROR;
    }
    // rawget: a metatable the mod set on its environment must not run outside the guard.
    lua_pushstring(L, pszFunction);
    lua_rawget(L, -2);
    if (!lua_isfunction(L, -1)) {
        lua_settop(L, nBase);
        return D2LUA_MISSING;
    }
    lua_remove(L, -2);
    return D2LuaHost_GuardedCall(L, pszMod, 0, 0);
}

size_t D2LuaHost_MemoryUsed(lua_State* L) {
    return L ? GetContext(L)->nUsed : 0;
}

const char* D2LuaHost_Version(void) {
    return LUAJIT_VERSION;
}

void D2LuaHost_Destroy(lua_State* L) {
    if (!L) {
        return;
    }
    HostContext* pContext = GetContext(L);
    lua_close(L);
    {
        std::lock_guard<std::mutex> tLock(pContext->tMutex);
        pContext->bStop = true;
    }
    pContext->tWake.notify_one();
    if (pContext->tWatchdog.joinable()) {
        pContext->tWatchdog.join();
    }
    delete pContext;
}
#endif
