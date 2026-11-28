// Function overrides: the registry, the patcher, and the dispatch a thunk runs. See
// d2_lua_override.h for the mechanism and d2_lua_override_gen.h for what a thunk does.
//
// Dispatch. A thunk that finds an override in its thread's state runs the chain on a coroutine of
// the state's own pool, entered with ResumeAs as the outermost layer's mod: the running thread is
// unknown to the thunk (engine code may be reached from a native that a script's coroutine called),
// and resuming an idle coroutine is the one entry into Lua that is right from any of them. The
// arguments are kept on a second idle thread of the pool entry (the scratch), which also takes the
// errors and the result. A layer's next() is a C closure (function, position); called from the
// layer's code, it runs the next live layer with CallAs on the running thread, or, at the bottom,
// the generated call of the original, which checks every argument it reads.
//
// The typed tier hands a mod no raw memory: units arrive as the world's checked handles (a server
// state) and every other pointer as an opaque value. A pointer a mod passes on to next() or returns
// must be one the call was given or one next() returned, of the same C type (nil only where the call
// itself was given a null one), so a mod can drop or reorder pointers but never make one.
//
// Like everything Lua calls, the functions here hold no object with a destructor across anything
// that may raise.
#if D2_FEATURE_MODS
#include "Redivivus/Lua/d2_lua_override.h"

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#include <tlhelp32.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

#include "Redivivus/Lua/d2_lua_api.h"
#include "Redivivus/Lua/d2_lua_internal.h"
#include "Redivivus/Lua/d2_lua_rt.h"
#include "Redivivus/Lua/d2_lua_world.h"
#include "lua.hpp"

namespace {

// --- the registry ----------------------------------------------------------------------------------

struct Registry {
    std::vector<const D2LuaOvEntry*> aById;
    std::vector<uint8_t> aRefused;              // D2LuaOvRefusal, the generated one or found at run time
    std::vector<std::pair<std::string, int>> aPaths;  // sorted by path
    int nOverridable = 0;
};

#if defined(__i386__) || defined(__x86_64__)
const bool kCanPatch = true;
#else
const bool kCanPatch = false;
#endif

// The five bytes at an entry the build leaves (five nops), or a long jump an earlier patch left.
bool HasPatchArea(const void* pEntry) {
    if (!kCanPatch || !pEntry) {
        return false;
    }
    const uint8_t* p = (const uint8_t*)pEntry;
    return (p[0] == 0x90 && p[1] == 0x90 && p[2] == 0x90 && p[3] == 0x90 && p[4] == 0x90) || p[0] == 0xE9;
}

Registry* BuildRegistry() {
    Registry* r = new Registry;
    int nMax = -1;
    for (const D2LuaOvTable* const* pp = g_apD2LuaOvTables; *pp; pp++) {
        for (int i = 0; i < (*pp)->nCount; i++) {
            nMax = std::max(nMax, (*pp)->pEntries[i].nId);
        }
    }
    r->aById.assign((size_t)(nMax + 1), nullptr);
    r->aRefused.assign((size_t)(nMax + 1), (uint8_t)D2LUA_OV_ABSENT);
    for (const D2LuaOvTable* const* pp = g_apD2LuaOvTables; *pp; pp++) {
        for (int i = 0; i < (*pp)->nCount; i++) {
            const D2LuaOvEntry* e = &(*pp)->pEntries[i];
            r->aById[(size_t)e->nId] = e;
            uint8_t nWhy = e->nRefused;
            if (nWhy == D2LUA_OV_OK && !HasPatchArea(e->pEntry)) {
                nWhy = kCanPatch ? D2LUA_OV_NO_ENTRY : D2LUA_OV_OK;  // elsewhere a thunk runs when called directly
            }
            r->aRefused[(size_t)e->nId] = nWhy;
            if (nWhy == D2LUA_OV_OK) {
                r->nOverridable++;
            }
            r->aPaths.emplace_back(e->pszPath, e->nId);
        }
    }
    std::sort(r->aPaths.begin(), r->aPaths.end());
    return r;
}

std::atomic<Registry*> g_pRegistry{ nullptr };
std::mutex g_tRegistryMutex;

Registry& Reg() {
    Registry* r = g_pRegistry.load(std::memory_order_acquire);
    if (r) {
        return *r;
    }
    std::lock_guard<std::mutex> tLock(g_tRegistryMutex);
    r = g_pRegistry.load(std::memory_order_relaxed);
    if (!r) {
        r = BuildRegistry();
        g_pRegistry.store(r, std::memory_order_release);
    }
    return *r;
}

// --- the patcher -------------------------------------------------------------------------------------

std::mutex g_tPatchMutex;
std::vector<int> g_aRefs;        // states overriding each function
int g_nPatched = 0;
D2LuaOvJournal g_tJournal = { 0, 0 };

bool MakeWritable(void* p, size_t n, unsigned long* pnOld) {
#if defined(_WIN32)
    DWORD nOld = 0;
    if (!VirtualProtect(p, n, PAGE_EXECUTE_READWRITE, &nOld)) {
        return false;
    }
    *pnOld = nOld;
    return true;
#else
    (void)pnOld;
    long nPage = sysconf(_SC_PAGESIZE);
    uintptr_t a = (uintptr_t)p & ~(uintptr_t)(nPage - 1);
    uintptr_t b = ((uintptr_t)p + n + (uintptr_t)nPage - 1) & ~(uintptr_t)(nPage - 1);
    return mprotect((void*)a, b - a, PROT_READ | PROT_WRITE | PROT_EXEC) == 0;
#endif
}

void RestoreProtection(void* p, size_t n, unsigned long nOld) {
#if defined(_WIN32)
    DWORD nWas = 0;
    VirtualProtect(p, n, (DWORD)nOld, &nWas);
    FlushInstructionCache(GetCurrentProcess(), p, n);
#else
    (void)nOld;
    long nPage = sysconf(_SC_PAGESIZE);
    uintptr_t a = (uintptr_t)p & ~(uintptr_t)(nPage - 1);
    uintptr_t b = ((uintptr_t)p + n + (uintptr_t)nPage - 1) & ~(uintptr_t)(nPage - 1);
    mprotect((void*)a, b - a, PROT_READ | PROT_EXEC);
    __builtin___clear_cache((char*)p, (char*)p + n);
#endif
}

// The first eight bytes at an entry (entries are on 16 bytes, so one aligned 8-byte block),
// replaced in one locked store: the five nops and the three bytes after them, or a long jump to
// the thunk and the same three bytes.
uint64_t Load8(const uint8_t* p) {
    uint64_t v;
    memcpy(&v, p, 8);
    return v;
}

bool Store8(uint8_t* p, uint64_t nOld, uint64_t nNew) {
    return __atomic_compare_exchange_n((uint64_t*)p, &nOld, nNew, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

// Write the long jump while every other thread of the process stands still and none stands inside
// the five bytes past their first (a thread there would resume in the middle of the jump). A thread
// on the first byte runs the jump; one past the bytes is not affected. Nothing here allocates or
// takes a lock while threads are suspended (one of them may hold the heap's). Elsewhere than
// Windows the store is made as it is: the host test runs no other thread through the entry then.
bool StoreQuiesced(uint8_t* pEntry, uint64_t nOld, uint64_t nNew, int* pnRetries) {
#if defined(_WIN32)
    const DWORD nSelf = GetCurrentThreadId();
    const DWORD nPid = GetCurrentProcessId();
    for (int nTry = 0; nTry < 200; nTry++) {
        std::vector<HANDLE> aThreads;
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (hSnap != INVALID_HANDLE_VALUE) {
            THREADENTRY32 t;
            t.dwSize = sizeof(t);
            for (BOOL b = Thread32First(hSnap, &t); b; b = Thread32Next(hSnap, &t)) {
                if (t.th32OwnerProcessID != nPid || t.th32ThreadID == nSelf) {
                    continue;
                }
                HANDLE h = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, t.th32ThreadID);
                if (h) {
                    aThreads.push_back(h);
                }
            }
            CloseHandle(hSnap);
        }
        std::vector<uint8_t> aSuspended(aThreads.size(), 0);
        bool bInside = false;
        for (size_t i = 0; i < aThreads.size(); i++) {
            if (SuspendThread(aThreads[i]) == (DWORD)-1) {
                continue;
            }
            aSuspended[i] = 1;
            CONTEXT c;
            memset(&c, 0, sizeof(c));
            c.ContextFlags = CONTEXT_CONTROL;
            if (GetThreadContext(aThreads[i], &c)) {
#if defined(_WIN64)
                uintptr_t ip = (uintptr_t)c.Rip;
#else
                uintptr_t ip = (uintptr_t)c.Eip;
#endif
                if (ip > (uintptr_t)pEntry && ip < (uintptr_t)pEntry + 5) {
                    bInside = true;
                }
            }
        }
        bool bStored = !bInside && Store8(pEntry, nOld, nNew);
        for (size_t i = 0; i < aThreads.size(); i++) {
            if (aSuspended[i]) {
                ResumeThread(aThreads[i]);
            }
            CloseHandle(aThreads[i]);
        }
        if (!bInside) {
            return bStored;
        }
        (*pnRetries)++;
        Sleep(0);
    }
    return false;
#else
    (void)pnRetries;
    return Store8(pEntry, nOld, nNew);
#endif
}

int g_nQuiesceRetries = 0;

bool PatchEntry(int nId, const D2LuaOvEntry* e) {
    (void)nId;
    uint8_t* pEntry = (uint8_t*)e->pEntry;
    intptr_t nRel = (intptr_t)e->pThunk - ((intptr_t)pEntry + 5);
    if (nRel != (intptr_t)(int32_t)nRel) {
        return false;
    }
    unsigned long nProtect = 0;
    if (!MakeWritable(pEntry, 8, &nProtect)) {
        return false;
    }
    uint64_t nOld = Load8(pEntry);
    uint8_t aNew[8];
    memcpy(aNew, &nOld, 8);
    int32_t nRel32 = (int32_t)nRel;
    aNew[0] = 0xE9;  // jmp rel32: to the thunk
    memcpy(aNew + 1, &nRel32, 4);
    uint64_t nNew;
    memcpy(&nNew, aNew, 8);
    bool bOk = StoreQuiesced(pEntry, nOld, nNew, &g_nQuiesceRetries);
    RestoreProtection(pEntry, 8, nProtect);
    if (bOk) {
        g_nPatched++;
        g_tJournal.nPatches++;
    }
    return bOk;
}

// Put the five nops back. No thread needs to stop: one that has not reached the entry runs the nops,
// one inside the jump does not exist (a jump is one instruction), one past it is in the thunk, which
// stays.
void RestoreEntry(const D2LuaOvEntry* e) {
    uint8_t* pEntry = (uint8_t*)e->pEntry;
    unsigned long nProtect = 0;
    if (!MakeWritable(pEntry, 8, &nProtect)) {
        return;
    }
    uint64_t nOld = Load8(pEntry);
    uint8_t aNew[8];
    memcpy(aNew, &nOld, 8);
    memset(aNew, 0x90, 5);
    uint64_t nNew;
    memcpy(&nNew, aNew, 8);
    Store8(pEntry, nOld, nNew);
    RestoreProtection(pEntry, 8, nProtect);
    g_nPatched--;
    g_tJournal.nRestores++;
}

// A state starts overriding the function / stops. The entry is patched while any state does.
bool Acquire(int nId) {
    if (!kCanPatch) {
        return true;
    }
    Registry& r = Reg();
    std::lock_guard<std::mutex> tLock(g_tPatchMutex);
    if (g_aRefs.size() < r.aById.size()) {
        g_aRefs.resize(r.aById.size(), 0);
    }
    if (g_aRefs[(size_t)nId]++ == 0 && !PatchEntry(nId, r.aById[(size_t)nId])) {
        g_aRefs[(size_t)nId]--;
        return false;
    }
    return true;
}

void Release(int nId) {
    if (!kCanPatch) {
        return;
    }
    Registry& r = Reg();
    std::lock_guard<std::mutex> tLock(g_tPatchMutex);
    if ((size_t)nId < g_aRefs.size() && g_aRefs[(size_t)nId] > 0 && --g_aRefs[(size_t)nId] == 0) {
        RestoreEntry(r.aById[(size_t)nId]);
    }
}

// --- a state's overrides -------------------------------------------------------------------------------

thread_local D2LuaRuntime* t_pServer = nullptr;
thread_local D2LuaRuntime* t_pClient = nullptr;

}  // namespace

struct D2LuaOvLayer {
    int nMod;      // host mod id
    int nRef;      // the override (registry ref)
    bool bRaw;     // the raw tier
    bool bDead;    // its mod faulted: skipped, dropped when no call is running
};

struct D2LuaOvChain {
    std::vector<D2LuaOvLayer> aLayers;  // outermost first
};

struct D2LuaOvPair {
    lua_State* pScratch;  // the arguments, the errors, the result
    lua_State* pRun;      // the outermost layer runs here
    int nScratchRef, nRunRef;
};

struct D2LuaOvState {
    D2LuaRuntime* pRt = nullptr;
    lua_State* L = nullptr;
    bool bReady = false;
    bool bPatching = false;
    int nCalls = 0;                       // chains running now
    std::vector<D2LuaOvChain*> aChains;   // by id, null when none
    std::vector<int> aIds;                // the functions it overrides
    std::vector<D2LuaOvPair> aPool;       // idle entries at the end
    size_t nPoolUsed = 0;
    int nNextTable = LUA_NOREF;           // { [id * 64 + pos] = next closure }
    D2LuaOvFrame* pTop = nullptr;         // the innermost running call
};

namespace {

D2LuaOvState* StateOf(lua_State* L) {
    D2LuaRuntime* pRt = D2LuaRt_FromState(L);
    return pRt ? pRt->pOv : nullptr;
}

const char* ModName(D2LuaOvState* st, int nMod) {
    const char* p = D2LuaHost_ModName(st->L, nMod);
    return p ? p : "?";
}

void Log(D2LuaOvState* st, const char* pszLine) {
    D2LuaHost_Log(st->L, pszLine);
}

// Make a pool entry, pushing the new threads on a thread that is idle (the state's own while nothing
// runs, a scratch thread otherwise).
bool GrowPool(D2LuaOvState* st, lua_State* pIdle) {
    D2LuaOvPair p;
    p.pScratch = lua_newthread(pIdle);
    p.nScratchRef = luaL_ref(pIdle, LUA_REGISTRYINDEX);
    p.pRun = lua_newthread(pIdle);
    p.nRunRef = luaL_ref(pIdle, LUA_REGISTRYINDEX);
    st->aPool.push_back(p);
    return true;
}

// Mark every layer of the mod dead (its mod faulted).
void KillMod(D2LuaOvState* st, int nMod) {
    for (int nId : st->aIds) {
        D2LuaOvChain* c = st->aChains[(size_t)nId];
        if (!c) {
            continue;
        }
        for (D2LuaOvLayer& l : c->aLayers) {
            if (l.nMod == nMod) {
                l.bDead = true;
            }
        }
    }
}

// Drop dead layers and the chains left empty (no call is running).
void Sweep(D2LuaOvState* st, lua_State* L) {
    std::vector<int> aKeep;
    for (int nId : st->aIds) {
        D2LuaOvChain* c = st->aChains[(size_t)nId];
        if (!c) {
            continue;
        }
        for (size_t i = 0; i < c->aLayers.size();) {
            if (c->aLayers[i].bDead) {
                luaL_unref(L, LUA_REGISTRYINDEX, c->aLayers[i].nRef);
                c->aLayers.erase(c->aLayers.begin() + (long)i);
            } else {
                i++;
            }
        }
        if (c->aLayers.empty()) {
            delete c;
            st->aChains[(size_t)nId] = nullptr;
            Release(nId);
        } else {
            aKeep.push_back(nId);
        }
    }
    st->aIds.swap(aKeep);
}

// The mod of a layer faulted (raised, or ran out of time or memory): the mod is disabled, its
// overrides go, and the call goes on below it.
void FaultLayer(D2LuaOvState* st, lua_State* L, int nId, const D2LuaOvLayer& l, const char* pszWhy) {
    char sz[512];
    const D2LuaOvEntry* e = Reg().aById[(size_t)nId];
    snprintf(sz, sizeof(sz), "[lua] mod '%s': its override of %s %s; the mod is disabled, its overrides are removed and the call goes on without them",
             ModName(st, l.nMod), e->pszPath, pszWhy);
    Log(st, sz);
    int nMod = l.nMod;
    KillMod(st, nMod);
    char szMod[128];
    snprintf(szMod, sizeof(szMod), "%s", ModName(st, nMod));
    D2LuaHost_FaultMod(L, szMod);
}

D2LuaOvFrame* FindFrame(D2LuaOvState* st, int nId) {
    for (D2LuaOvFrame* f = st->pTop; f; f = f->pPrev) {
        if (f->nId == nId) {
            return f;
        }
    }
    return nullptr;
}

int Lua_Next(lua_State* L);

// Push the next() a layer at position nPos - 1 gets: the closure for (nId, nPos).
void PushNext(lua_State* L, D2LuaOvState* st, int nId, int nPos) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, st->nNextTable);
    lua_rawgeti(L, -1, nId * 64 + nPos);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_pushinteger(L, nId);
        lua_pushinteger(L, nPos);
        lua_pushcclosure(L, Lua_Next, 2);
        lua_pushvalue(L, -1);
        lua_rawseti(L, -3, nId * 64 + nPos);
    }
    lua_remove(L, -2);
}

void Remember(D2LuaOvFrame* f, uintptr_t p, const void* pType) {
    for (int i = 0; i < f->nSeen; i++) {
        if (f->aSeen[i] == p && f->aSeenType[i] == pType) {
            return;
        }
    }
    if (f->nSeen < 8) {
        f->aSeen[f->nSeen] = p;
        f->aSeenType[f->nSeen] = pType;
        f->nSeen++;
    }
}

// The unit a handle at index i names: the pointer the call was given for it, else the world's.
uintptr_t HandleUnit(lua_State* L, int i, const D2LuaOvFrame* f) {
    int nType = 0;
    uint32_t nGuid = 0;
    if (!D2LuaWorld_HandleKey(L, i, &nType, &nGuid)) {
        return 0;
    }
    for (int k = 0; k < f->nArgs && k < D2LUA_OV_MAX_ARGS; k++) {
        if (f->aArg[k] && D2LuaWorld_UnitIs(L, (D2UnitStrc*)f->aArg[k], nType, nGuid)) {
            return f->aArg[k];
        }
    }
    return (uintptr_t)D2LuaWorld_FindUnit(L, nType, nGuid);
}

// Push the nArgs arguments at from[nFirst..] onto to, as the layer about to run takes them: a typed
// layer gets units as handles and other pointers as opaque values, a raw layer every pointer as an
// address (the core turns them into cdata). What a raw caller made (a number where a pointer
// belongs) is taken as it is and remembered, so a typed layer below may hand it on.
void PushLayerArgs(lua_State* from, int nFirst, int nArgs, lua_State* to, D2LuaOvFrame* f, const char* pszKinds, bool bCallerRaw,
                   bool bCalleeRaw) {
    for (int k = 0; k < nArgs; k++) {
        int i = nFirst + k;
        char c = pszKinds[k + 1];
        bool bPointer = c == 'w' || c == 'p' || c == 'r';
        int t = lua_type(from, i);
        if (!bPointer || t == LUA_TNIL || t == LUA_TNONE) {
            lua_pushvalue(from, i);
            if (from != to) {
                lua_xmove(from, to, 1);
            }
            continue;
        }
        uintptr_t p = 0;
        bool bHave = true;
        if (t == LUA_TLIGHTUSERDATA) {
            p = (uintptr_t)lua_touserdata(from, i);
            if (bCallerRaw && k < D2LUA_OV_MAX_ARGS) {
                Remember(f, p, f->aArgType[k]);
            }
        } else if (t == LUA_TNUMBER && bCallerRaw) {
            p = (uintptr_t)(uint64_t)lua_tonumber(from, i);
            if (k < D2LUA_OV_MAX_ARGS) {
                Remember(f, p, f->aArgType[k]);
            }
        } else if (t == LUA_TUSERDATA && c == 'w') {
            p = HandleUnit(from, i, f);
            bHave = p != 0 || !bCalleeRaw;
            if (!bCalleeRaw) {
                // a typed layer takes the handle as it is: the bottom checks it
                lua_pushvalue(from, i);
                if (from != to) {
                    lua_xmove(from, to, 1);
                }
                continue;
            }
        } else {
            bHave = false;
        }
        if (!bHave) {
            // something that is no pointer: passed on as it is, the bottom refuses it
            lua_pushvalue(from, i);
            if (from != to) {
                lua_xmove(from, to, 1);
            }
            continue;
        }
        if (!p) {
            lua_pushnil(to);
        } else if (c == 'w' && !bCalleeRaw && D2LuaWorld_PushUnit(to, (D2UnitStrc*)p)) {
        } else {
            lua_pushlightuserdata(to, (void*)p);
        }
    }
}

// next() of a layer: run the next live layer below it, or the original. The arguments are this
// call's (1..n). Upvalues: the function id, the position of the layer to run.
int Lua_Next(lua_State* L) {
    int nId = (int)lua_tointeger(L, lua_upvalueindex(1));
    int nPos = (int)lua_tointeger(L, lua_upvalueindex(2));
    D2LuaOvState* st = StateOf(L);
    D2LuaOvFrame* f = st ? FindFrame(st, nId) : nullptr;
    if (!f) {
        return luaL_error(L, "next: called outside the override it was given to");
    }
    const D2LuaOvEntry* e = Reg().aById[(size_t)nId];
    D2LuaOvChain* c = st->aChains[(size_t)nId];
    bool bCallerRaw = c && nPos >= 1 && nPos - 1 < (int)c->aLayers.size() && c->aLayers[(size_t)nPos - 1].bRaw;
    int nArgs = lua_gettop(L);
    int nParams = (int)strlen(e->pszKinds) - 1;
    if (nArgs > nParams) {
        return luaL_error(L, "next: %s takes %d argument(s), got %d", e->pszPath, nParams, nArgs);
    }
    while (c && nPos < (int)c->aLayers.size()) {
        D2LuaOvLayer l = c->aLayers[(size_t)nPos];
        if (l.bDead) {
            nPos++;
            continue;
        }
        if (!lua_checkstack(L, nParams + 8)) {
            return luaL_error(L, "next: stack overflow");
        }
        lua_rawgeti(L, LUA_REGISTRYINDEX, l.nRef);
        PushNext(L, st, nId, nPos + 1);
        PushLayerArgs(L, 1, nArgs, L, f, e->pszKinds, bCallerRaw, l.bRaw);
        int nPushed = 0;
        int nStatus = D2LuaHost_CallAs(L, l.nMod, nArgs + 1, &nPushed);
        if (nStatus == D2LUA_CALL_OK) {
            char r = e->pszKinds[0];
            if (l.bRaw && (r == 'w' || r == 'p' || r == 'r') && lua_type(L, -1) == LUA_TNUMBER) {
                // a raw layer's pointer result, for the layer above
                uintptr_t p = (uintptr_t)(uint64_t)lua_tonumber(L, -1);
                lua_pop(L, 1);
                if (p) {
                    lua_pushlightuserdata(L, (void*)p);
                } else {
                    lua_pushnil(L);
                }
            }
            if (r == 'w' && lua_type(L, -1) == LUA_TLIGHTUSERDATA) {
                // the layer above gets a unit the way its tier takes one
                void* p = lua_touserdata(L, -1);
                if (!bCallerRaw) {
                    lua_pop(L, 1);
                    if (!D2LuaWorld_PushUnit(L, (D2UnitStrc*)p)) {
                        lua_pushlightuserdata(L, p);
                    }
                }
            } else if (r == 'w' && bCallerRaw && lua_type(L, -1) == LUA_TUSERDATA) {
                uintptr_t p = HandleUnit(L, lua_gettop(L), f);
                lua_pop(L, 1);
                lua_pushlightuserdata(L, (void*)p);
            }
            if (bCallerRaw && (r == 'w' || r == 'p' || r == 'r') && lua_type(L, -1) == LUA_TLIGHTUSERDATA) {
                Remember(f, (uintptr_t)lua_touserdata(L, -1), nullptr);
            }
            return 1;
        }
        if (nStatus == D2LUA_CALL_FATAL) {
            return lua_error(L);  // the budget ran out: back to the entry's start
        }
        if (nStatus == D2LUA_CALL_ERROR) {
            // error, traceback
            char sz[1024];
            snprintf(sz, sizeof(sz), "raised: %s", lua_tostring(L, -2) ? lua_tostring(L, -2) : "(error)");
            lua_pop(L, nPushed);
            FaultLayer(st, L, nId, l, sz);
        } else {
            lua_pop(L, nPushed);
            if (nStatus != D2LUA_CALL_DISABLED) {
                FaultLayer(st, L, nId, l, "ran out of time or memory");
            } else {
                KillMod(st, l.nMod);
            }
        }
        nPos++;
    }
    // The bottom: the original. The generated call reads this call's arguments, as the caller's tier
    // allows.
    bool bWas = f->bTrusted;
    f->bTrusted = bCallerRaw;
    if (bCallerRaw) {
        // a raw caller hands addresses: as light userdata for the generated call
        for (int k = 0; k < nArgs; k++) {
            char ch = e->pszKinds[k + 1];
            if ((ch == 'w' || ch == 'p' || ch == 'r') && lua_type(L, k + 1) == LUA_TNUMBER) {
                uintptr_t p = (uintptr_t)(uint64_t)lua_tonumber(L, k + 1);
                if (p) {
                    lua_pushlightuserdata(L, (void*)p);
                } else {
                    lua_pushnil(L);
                }
                lua_replace(L, k + 1);
            }
        }
    }
    int n = e->pfnCall(L);
    f->bTrusted = bWas;
    return n;
}

}  // namespace

// --- the thunk's side -----------------------------------------------------------------------------------

bool D2LuaOv_Begin(D2LuaOvFrame* f, int nId) {
    Registry* r = g_pRegistry.load(std::memory_order_acquire);
    if (!r || (size_t)nId >= r->aById.size()) {
        return false;
    }
    int nSide = r->aById[(size_t)nId]->nSide;
    D2LuaRuntime* pRt = nullptr;
    if ((nSide & D2LUA_SIDE_SERVER) && t_pServer) {
        pRt = t_pServer;
    } else if (nSide & D2LUA_SIDE_CLIENT) {
        pRt = t_pClient;
    }
    D2LuaOvState* st = pRt ? pRt->pOv : nullptr;
    if (!st || !st->bReady || (size_t)nId >= st->aChains.size() || !st->aChains[(size_t)nId]) {
        return false;
    }
    if (st->nPoolUsed == st->aPool.size()) {
        // the scratch of the call running now is idle (it only holds that call's arguments)
        lua_State* pIdle = st->nPoolUsed ? st->aPool[st->nPoolUsed - 1].pScratch : st->L;
        if (!lua_checkstack(pIdle, 4) || !GrowPool(st, pIdle)) {
            return false;
        }
    }
    D2LuaOvPair& p = st->aPool[st->nPoolUsed++];
    f->pState = st;
    f->L = p.pScratch;
    f->nId = nId;
    f->nArgs = 0;
    f->nSeen = 0;
    f->bTrusted = false;
    f->nBase = lua_gettop(p.pScratch);
    f->nDepth = 0;
    f->pPrev = st->pTop;
    st->pTop = f;
    st->nCalls++;
    if (!lua_checkstack(p.pScratch, D2LUA_OV_MAX_ARGS + 8)) {
        D2LuaOv_End(f);
        return false;
    }
    return true;
}

void D2LuaOv_PushNumber(D2LuaOvFrame* f, double n) {
    lua_pushnumber(f->L, n);
    f->nArgs++;
}

void D2LuaOv_PushBool(D2LuaOvFrame* f, bool b) {
    lua_pushboolean(f->L, b ? 1 : 0);
    f->nArgs++;
}

void D2LuaOv_PushPointer(D2LuaOvFrame* f, int i, const void* p, const void* pType, bool bUnit) {
    (void)bUnit;  // a layer gets the handle (PushLayerArgs); the frame keeps the address
    if (i < D2LUA_OV_MAX_ARGS) {
        f->aArg[i] = (uintptr_t)p;
        f->aArgType[i] = pType;
    }
    if (!p) {
        lua_pushnil(f->L);
    } else {
        lua_pushlightuserdata(f->L, (void*)p);
    }
    f->nArgs++;
}

bool D2LuaOv_Invoke(D2LuaOvFrame* f) {
    D2LuaOvState* st = f->pState;
    D2LuaOvPair& p = st->aPool[st->nPoolUsed - 1];
    const D2LuaOvEntry* e = Reg().aById[(size_t)f->nId];
    int nId = f->nId;
    int nArgs = f->nArgs;
    for (int nPos = 0;; nPos++) {
        D2LuaOvChain* c = st->aChains[(size_t)nId];
        if (!c || nPos >= (int)c->aLayers.size()) {
            return false;  // every layer faulted: the thunk runs the original
        }
        D2LuaOvLayer l = c->aLayers[(size_t)nPos];
        if (l.bDead) {
            continue;
        }
        lua_State* co = p.pRun;
        lua_settop(co, 0);
        lua_rawgeti(f->L, LUA_REGISTRYINDEX, l.nRef);
        PushNext(f->L, st, nId, nPos + 1);
        lua_xmove(f->L, co, 2);
        PushLayerArgs(f->L, f->nBase + 1, nArgs, co, f, e->pszKinds, false, l.bRaw);
        int nResults = 0;
        int nStatus = D2LuaHost_ResumeAs(f->L, co, l.nMod, nArgs + 1, &nResults);
        if (nStatus == D2LUA_CALL_OK) {
            if (nResults > 0) {
                lua_settop(co, 1);
                lua_xmove(co, f->L, 1);  // the first result
            } else {
                lua_pushnil(f->L);
            }
            lua_settop(co, 0);
            f->bTrusted = l.bRaw;
            if (l.bRaw && lua_type(f->L, -1) == LUA_TNUMBER && strchr("wpr", e->pszKinds[0])) {
                uintptr_t r = (uintptr_t)(uint64_t)lua_tonumber(f->L, -1);
                lua_pop(f->L, 1);
                if (r) {
                    lua_pushlightuserdata(f->L, (void*)r);
                } else {
                    lua_pushnil(f->L);
                }
            }
            return true;
        }
        // An entry that failed leaves the coroutine unusable: a fresh one takes its place.
        luaL_unref(f->L, LUA_REGISTRYINDEX, p.nRunRef);
        p.pRun = lua_newthread(f->L);
        p.nRunRef = luaL_ref(f->L, LUA_REGISTRYINDEX);
        if (nStatus == D2LUA_CALL_ERROR) {
            // error, traceback on the scratch
            char sz[1024];
            snprintf(sz, sizeof(sz), "raised: %s", lua_tostring(f->L, -2) ? lua_tostring(f->L, -2) : "(error)");
            lua_pop(f->L, nResults);
            FaultLayer(st, f->L, nId, l, sz);
        } else if (nStatus == D2LUA_CALL_YIELD) {
            FaultLayer(st, f->L, nId, l, "yielded (an override runs to its end; it cannot wait)");
        } else if (nStatus == D2LUA_CALL_DISABLED) {
            KillMod(st, l.nMod);
        } else {
            // Faulted (logged by the host, the mod disabled) or fatal.
            lua_pop(f->L, nResults);
            KillMod(st, l.nMod);
            char sz[256];
            snprintf(sz, sizeof(sz), "[lua] mod '%s': its overrides are removed", ModName(st, l.nMod));
            Log(st, sz);
        }
    }
}

namespace {
// The head's result at the top of the scratch: a conversion that fails faults the outermost live layer.
bool ResultFailed(D2LuaOvFrame* f, const char* pszWhat) {
    D2LuaOvState* st = f->pState;
    D2LuaOvChain* c = st->aChains[(size_t)f->nId];
    for (size_t i = 0; c && i < c->aLayers.size(); i++) {
        if (!c->aLayers[i].bDead) {
            D2LuaOvLayer l = c->aLayers[i];
            char sz[256];
            snprintf(sz, sizeof(sz), "returned %s", pszWhat);
            FaultLayer(st, f->L, f->nId, l, sz);
            break;
        }
    }
    return false;
}

bool SeenPointer(const D2LuaOvFrame* f, uintptr_t p, const void* pType, int nArg) {
    if (nArg >= 0 && nArg < D2LUA_OV_MAX_ARGS && nArg < f->nArgs && f->aArg[nArg] == p && f->aArgType[nArg] == pType) {
        return true;
    }
    for (int i = 0; i < D2LUA_OV_MAX_ARGS && i < f->nArgs; i++) {
        if (f->aArg[i] == p && f->aArgType[i] == pType) {
            return true;
        }
    }
    for (int i = 0; i < f->nSeen && i < 8; i++) {
        if (f->aSeen[i] == p && f->aSeenType[i] == pType) {
            return true;
        }
    }
    return false;
}

// A pointer from Lua at index i: a handle (when bUnit and the state has a world), an opaque value, or
// nil. *ppszWhy is set when it is none of these or, for a typed layer, not one the call has seen.
uintptr_t ReadPointer(lua_State* L, int i, const D2LuaOvFrame* f, const void* pType, bool bUnit, int nArg, const char** ppszWhy) {
    if (lua_isnoneornil(L, i)) {
        // nil only where the call was given a null pointer of the type (or next() returned one)
        if (f->bTrusted || SeenPointer(f, 0, pType, nArg)) {
            return 0;
        }
        *ppszWhy = "nil, where the call was given no null pointer";
        return 0;
    }
    if (lua_type(L, i) == LUA_TLIGHTUSERDATA) {
        uintptr_t p = (uintptr_t)lua_touserdata(L, i);
        if (f->bTrusted || SeenPointer(f, p, pType, nArg)) {
            return p;
        }
        *ppszWhy = "a pointer this call was not given";
        return 0;
    }
    if (bUnit && lua_type(L, i) == LUA_TUSERDATA) {
        uintptr_t p = HandleUnit(L, i, f);
        if (!p) {
            *ppszWhy = "not a valid unit";
        }
        return p;
    }
    *ppszWhy = bUnit ? "not a unit" : "not a pointer of the call";
    return 0;
}
}  // namespace

bool D2LuaOv_ResultNumber(D2LuaOvFrame* f, double* pn, double nMin, double nMax, bool bIntegral) {
    lua_State* L = f->L;
    if (lua_type(L, -1) != LUA_TNUMBER) {
        char sz[64];
        snprintf(sz, sizeof(sz), "a %s where a number belongs", luaL_typename(L, -1));
        return ResultFailed(f, sz);
    }
    double n = lua_tonumber(L, -1);
    if ((bIntegral && n != (double)(long long)n) || n < nMin || n > nMax) {
        return ResultFailed(f, "a number out of the result's range");
    }
    *pn = n;
    return true;
}

bool D2LuaOv_ResultBool(D2LuaOvFrame* f, bool* pb) {
    if (lua_type(f->L, -1) != LUA_TBOOLEAN) {
        return ResultFailed(f, "something else where a boolean belongs");
    }
    *pb = lua_toboolean(f->L, -1) != 0;
    return true;
}

bool D2LuaOv_ResultPointer(D2LuaOvFrame* f, void** pp, const void* pType, bool bUnit) {
    const char* pszWhy = nullptr;
    uintptr_t p = ReadPointer(f->L, lua_gettop(f->L), f, pType, bUnit, -1, &pszWhy);
    if (pszWhy) {
        return ResultFailed(f, pszWhy);
    }
    *pp = (void*)p;
    return true;
}

void D2LuaOv_End(D2LuaOvFrame* f) {
    D2LuaOvState* st = f->pState;
    lua_settop(f->L, f->nBase);
    st->pTop = f->pPrev;
    st->nPoolUsed--;
    if (--st->nCalls == 0) {
        Sweep(st, f->L);
    }
}

// --- next() at the bottom --------------------------------------------------------------------------------

D2LuaOvFrame* D2LuaOv_CallFrame(lua_State* L, int nId, int nArgs) {
    D2LuaOvState* st = StateOf(L);
    D2LuaOvFrame* f = st ? FindFrame(st, nId) : nullptr;
    if (!f) {
        luaL_error(L, "next: called outside the override it was given to");
    }
    if (lua_gettop(L) > nArgs) {
        luaL_error(L, "next: %s takes %d argument(s), got %d", Reg().aById[(size_t)nId]->pszPath, nArgs, lua_gettop(L));
    }
    return f;
}

double D2LuaOv_ArgNumber(lua_State* L, D2LuaOvFrame* f, int i, double nMin, double nMax, bool bIntegral) {
    if (lua_type(L, i + 1) != LUA_TNUMBER) {
        luaL_error(L, "next: argument %d of %s must be a number, not a %s", i + 1, Reg().aById[(size_t)f->nId]->pszPath, luaL_typename(L, i + 1));
    }
    double n = lua_tonumber(L, i + 1);
    if ((bIntegral && n != (double)(long long)n) || n < nMin || n > nMax) {
        luaL_error(L, "next: argument %d of %s is out of its type's range (%f)", i + 1, Reg().aById[(size_t)f->nId]->pszPath, n);
    }
    return n;
}

bool D2LuaOv_ArgBool(lua_State* L, D2LuaOvFrame* f, int i) {
    if (lua_type(L, i + 1) != LUA_TBOOLEAN) {
        luaL_error(L, "next: argument %d of %s must be a boolean", i + 1, Reg().aById[(size_t)f->nId]->pszPath);
    }
    return lua_toboolean(L, i + 1) != 0;
}

void* D2LuaOv_ArgPointer(lua_State* L, D2LuaOvFrame* f, int i, const void* pType, bool bUnit) {
    const char* pszWhy = nullptr;
    uintptr_t p = ReadPointer(L, i + 1, f, pType, bUnit, i, &pszWhy);
    if (pszWhy) {
        luaL_error(L, "next: argument %d of %s is %s", i + 1, Reg().aById[(size_t)f->nId]->pszPath, pszWhy);
    }
    return (void*)p;
}

int D2LuaOv_ReturnNumber(lua_State* L, double n) {
    lua_pushnumber(L, n);
    return 1;
}

int D2LuaOv_ReturnBool(lua_State* L, bool b) {
    lua_pushboolean(L, b ? 1 : 0);
    return 1;
}

int D2LuaOv_ReturnPointer(lua_State* L, D2LuaOvFrame* f, const void* p, const void* pType, bool bUnit) {
    Remember(f, (uintptr_t)p, pType);
    if (!p) {
        lua_pushnil(L);
    } else if (!bUnit || f->bTrusted || !D2LuaWorld_PushUnit(L, (D2UnitStrc*)p)) {
        lua_pushlightuserdata(L, (void*)p);
    }
    return 1;
}

int D2LuaOv_ReturnNone(lua_State* L) {
    (void)L;
    return 0;
}

// --- the registry's API ------------------------------------------------------------------------------------

int D2LuaOv_Count(void) {
    return (int)Reg().aById.size();
}

int D2LuaOv_Overridable(void) {
    return Reg().nOverridable;
}

const D2LuaOvEntry* D2LuaOv_Entry(int nId) {
    Registry& r = Reg();
    return nId >= 0 && (size_t)nId < r.aById.size() ? r.aById[(size_t)nId] : nullptr;
}

const D2LuaOvEntry* D2LuaOv_Find(const char* pszPath) {
    Registry& r = Reg();
    auto it = std::lower_bound(r.aPaths.begin(), r.aPaths.end(), std::make_pair(std::string(pszPath), -1));
    return it != r.aPaths.end() && it->first == pszPath ? r.aById[(size_t)it->second] : nullptr;
}

const char* D2LuaOv_RefusalText(int nRefused) {
    switch (nRefused) {
    case D2LUA_OV_OK:
        return nullptr;
    case D2LUA_OV_VARIADIC:
        return "it takes a variable argument list (...), which a thunk cannot pass on";
    case D2LUA_OV_BY_VALUE:
        return "a parameter or its result is a record passed by value";
    case D2LUA_OV_INLINED:
        return "it is inlined (always_inline) at its call sites, so an override would miss them";
    case D2LUA_OV_HEADER_STATIC:
        return "it is static in a header: every unit that includes it has its own copy";
    case D2LUA_OV_MEMBER:
        return "it is a member function";
    case D2LUA_OV_NO_ENTRY:
        return "its entry has no patch area in this build";
    case D2LUA_OV_TOO_MANY:
        return "it has more parameters than an override frame holds";
    case D2LUA_OV_ABSENT:
        return "it is not compiled on this target";
    }
    return "it cannot be overridden";
}

bool D2LuaOv_CanPatch(void) {
    return kCanPatch;
}

D2LuaRuntime* D2LuaOv_BindThread(int nSide, D2LuaRuntime* pRt) {
    D2LuaRuntime*& slot = nSide == D2LUA_SIDE_SERVER ? t_pServer : t_pClient;
    D2LuaRuntime* pWas = slot;
    slot = pRt;
    return pWas;
}

D2LuaRuntime* D2LuaOv_ThreadRuntime(int nSide) {
    return nSide == D2LUA_SIDE_SERVER ? t_pServer : t_pClient;
}

void D2LuaOv_StateCreated(D2LuaRuntime* pRt) {
    if (pRt->pOv) {
        return;
    }
    D2LuaOvState* st = new D2LuaOvState;
    st->pRt = pRt;
    st->L = pRt->L;
    st->aChains.assign(Reg().aById.size(), nullptr);
    lua_newtable(st->L);
    st->nNextTable = luaL_ref(st->L, LUA_REGISTRYINDEX);
    for (int i = 0; i < 4; i++) {
        GrowPool(st, st->L);
    }
    pRt->pOv = st;
}

void D2LuaOv_ClearState(D2LuaRuntime* pRt) {
    D2LuaOvState* st = pRt->pOv;
    if (!st) {
        return;
    }
    for (int nId : st->aIds) {
        D2LuaOvChain* c = st->aChains[(size_t)nId];
        if (c) {
            for (D2LuaOvLayer& l : c->aLayers) {
                l.bDead = true;
            }
        }
    }
    if (st->nCalls == 0) {
        Sweep(st, st->L);
    }
}

void D2LuaOv_StateDestroyed(D2LuaRuntime* pRt) {
    D2LuaOvState* st = pRt->pOv;
    if (!st) {
        return;
    }
    st->bReady = false;
    for (int nId : st->aIds) {
        if (st->aChains[(size_t)nId]) {
            delete st->aChains[(size_t)nId];
            st->aChains[(size_t)nId] = nullptr;
            Release(nId);
        }
    }
    st->aIds.clear();
    if (t_pServer == pRt) {
        t_pServer = nullptr;
    }
    if (t_pClient == pRt) {
        t_pClient = nullptr;
    }
    delete st;
    pRt->pOv = nullptr;
}

void D2LuaOv_StateReady(D2LuaRuntime* pRt, bool bReady) {
    if (pRt->pOv) {
        pRt->pOv->bReady = bReady;
    }
}

int D2LuaOv_UndoAll(void) {
    if (!kCanPatch) {
        return 0;
    }
    Registry& r = Reg();
    std::lock_guard<std::mutex> tLock(g_tPatchMutex);
    int n = 0;
    for (size_t i = 0; i < g_aRefs.size(); i++) {
        if (g_aRefs[i] > 0) {
            RestoreEntry(r.aById[i]);
            g_aRefs[i] = 0;
            n++;
        }
    }
    return n;
}

int D2LuaOv_PatchedCount(void) {
    std::lock_guard<std::mutex> tLock(g_tPatchMutex);
    return g_nPatched;
}

D2LuaOvJournal D2LuaOv_JournalTotals(void) {
    std::lock_guard<std::mutex> tLock(g_tPatchMutex);
    return g_tJournal;
}

bool D2LuaOv_Patching(lua_State* L) {
    D2LuaOvState* st = StateOf(L);
    return st && st->bPatching;
}

void D2LuaOv_SetPatching(lua_State* L, bool bPatching) {
    D2LuaOvState* st = StateOf(L);
    if (st) {
        st->bPatching = bPatching;
    }
}

const char* D2LuaOv_Add(lua_State* L, int nId, int nModId, bool bRaw) {
    D2LuaOvState* st = StateOf(L);
    Registry& r = Reg();
    if (!st || nId < 0 || (size_t)nId >= r.aById.size()) {
        lua_pop(L, 1);
        return "no such function";
    }
    if (!st->bPatching) {
        lua_pop(L, 1);
        return "overrides are made in patch(engine), not later";
    }
    const D2LuaOvEntry* e = r.aById[(size_t)nId];
    if (!(e->nSide & st->pRt->tDesc.nSide)) {
        lua_pop(L, 1);
        return "the function does not run on this side";
    }
    if (r.aRefused[(size_t)nId] != D2LUA_OV_OK) {
        lua_pop(L, 1);
        return D2LuaOv_RefusalText(r.aRefused[(size_t)nId]);
    }
    D2LuaOvChain*& c = st->aChains[(size_t)nId];
    if (!c) {
        if (!Acquire(nId)) {
            lua_pop(L, 1);
            return "its entry could not be patched";
        }
        c = new D2LuaOvChain;
        st->aIds.push_back(nId);
    }
    D2LuaOvLayer l;
    l.nMod = nModId;
    l.nRef = luaL_ref(L, LUA_REGISTRYINDEX);
    l.bRaw = bRaw;
    l.bDead = false;
    c->aLayers.insert(c->aLayers.begin(), l);  // the newest is outermost: dependents wrap what they depend on
    return nullptr;
}

int D2LuaOv_Lookup(lua_State* L, const char* pszPath, int* pnId) {
    D2LuaOvState* st = StateOf(L);
    int nSide = st ? st->pRt->tDesc.nSide : 0;
    Registry& r = Reg();
    std::string sPath(pszPath);
    auto it = std::lower_bound(r.aPaths.begin(), r.aPaths.end(), std::make_pair(sPath, -1));
    if (it != r.aPaths.end() && it->first == sPath && (r.aById[(size_t)it->second]->nSide & nSide)) {
        *pnId = it->second;
        return 2;
    }
    std::string sPrefix = sPath + ".";
    for (auto jt = std::lower_bound(r.aPaths.begin(), r.aPaths.end(), std::make_pair(sPrefix, -1));
         jt != r.aPaths.end() && jt->first.compare(0, sPrefix.size(), sPrefix) == 0; ++jt) {
        if (r.aById[(size_t)jt->second]->nSide & nSide) {
            return 1;
        }
    }
    return 0;
}

std::string D2LuaOv_NoEntryList(int nMax) {
    Registry& r = Reg();
    std::string s;
    int n = 0;
    for (size_t i = 0; i < r.aById.size(); i++) {
        if (r.aRefused[i] == D2LUA_OV_NO_ENTRY && r.aById[i] && r.aById[i]->nRefused == D2LUA_OV_OK) {
            if (n++ < nMax) {
                char sz[48];
                const uint8_t* p = (const uint8_t*)r.aById[i]->pEntry;
                snprintf(sz, sizeof(sz), " (%02x %02x %02x)", p ? p[0] : 0, p ? p[1] : 0, p ? p[2] : 0);
                s += (s.empty() ? "" : ", ") + std::string(r.aById[i]->pszPath) + sz;
            }
        }
    }
    return s;
}

std::string D2LuaOv_Summary(void) {
    Registry& r = Reg();
    int aWhy[9] = {};
    for (uint8_t n : r.aRefused) {
        aWhy[n < 9 ? n : 0]++;
    }
    char sz[512];
    snprintf(sz, sizeof(sz),
             "[mods] overrides: %d functions, %d overridable; refused: %d variadic, %d by-value records, %d inlined, %d static in a header, "
             "%d members, %d without a patch area, %d with too many parameters",
             (int)r.aById.size() - aWhy[D2LUA_OV_ABSENT], r.nOverridable, aWhy[D2LUA_OV_VARIADIC], aWhy[D2LUA_OV_BY_VALUE], aWhy[D2LUA_OV_INLINED],
             aWhy[D2LUA_OV_HEADER_STATIC], aWhy[D2LUA_OV_MEMBER], aWhy[D2LUA_OV_NO_ENTRY], aWhy[D2LUA_OV_TOO_MANY]);
    return sz;
}

// --- the natives the core's engine object is built on (core/native/core.d.ts) ---------------------------

namespace {
D2LuaMod* OvMod(lua_State* L, int i, const char* pszWhere) {
    if (lua_type(L, i) != LUA_TNUMBER) {
        luaL_error(L, "%s: a mod id expected", pszWhere);
    }
    D2LuaMod* pMod = D2LuaRt_ModByHostId(D2LuaRt_FromState(L), (int)lua_tointeger(L, i));
    if (!pMod) {
        luaL_error(L, "%s: no mod %d in this state", pszWhere, (int)lua_tointeger(L, i));
    }
    return pMod;
}

char s_FfiKey;
}  // namespace

int D2LuaCore_OvLookup(D2LuaCtx& ctx) {
    int nId = -1;
    int nKind = D2LuaOv_Lookup(ctx.L, luaL_checkstring(ctx.L, 1), &nId);
    lua_pushinteger(ctx.L, nKind);
    if (nKind == 2) {
        lua_pushinteger(ctx.L, nId);
        return 2;
    }
    return 1;
}

int D2LuaCore_OvInfo(D2LuaCtx& ctx) {
    int nId = (int)luaL_checkinteger(ctx.L, 1);
    const D2LuaOvEntry* e = D2LuaOv_Entry(nId);
    if (!e) {
        return luaL_error(ctx.L, "ovInfo: no function %d", nId);
    }
    lua_pushstring(ctx.L, e->pszPath);
    lua_pushstring(ctx.L, e->pszKinds ? e->pszKinds : "");
    const char* pszWhy = D2LuaOv_RefusalText(Reg().aRefused[(size_t)nId]);
    if (pszWhy) {
        lua_pushstring(ctx.L, pszWhy);
    } else {
        lua_pushnil(ctx.L);
    }
    lua_pushstring(ctx.L, e->pszPointees ? e->pszPointees : "");
    return 4;
}

int D2LuaCore_OvAdd(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    D2LuaMod* pMod = OvMod(L, 1, "ovAdd");
    int nId = (int)luaL_checkinteger(L, 2);
    luaL_checktype(L, 3, LUA_TFUNCTION);
    bool bRaw = lua_toboolean(L, 4) != 0;
    if (bRaw && !pMod->bRawGrant) {
        return luaL_error(L, "ovAdd: mod '%s' has no raw tier", pMod->sName.c_str());
    }
    lua_pushvalue(L, 3);
    const char* pszWhy = D2LuaOv_Add(L, nId, pMod->nHostId, bRaw);
    if (pszWhy) {
        lua_pushstring(L, pszWhy);
        return 1;
    }
    return 0;
}

int D2LuaCore_OvDropMod(D2LuaCtx& ctx) {
    D2LuaMod* pMod = OvMod(ctx.L, 1, "ovDropMod");
    D2LuaOvState* st = StateOf(ctx.L);
    if (st) {
        KillMod(st, pMod->nHostId);
        if (st->nCalls == 0) {
            Sweep(st, ctx.L);
        }
    }
    return 0;
}

int D2LuaCore_OvRawGranted(D2LuaCtx& ctx) {
    lua_pushboolean(ctx.L, OvMod(ctx.L, 1, "ovRawGranted")->bRawGrant ? 1 : 0);
    return 1;
}

int D2LuaCore_OvPatching(D2LuaCtx& ctx) {
    D2LuaOv_SetPatching(ctx.L, lua_toboolean(ctx.L, 1) != 0);
    return 0;
}

int D2LuaCore_OvFfi(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    lua_pushlightuserdata(L, &s_FfiKey);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (!lua_isnil(L, -1)) {
        return 1;
    }
    lua_pop(L, 1);
    // ffi is opened for the raw tier on first use and kept in the registry, out of every mod's reach.
    lua_pushcfunction(L, luaopen_ffi);
    lua_call(L, 0, 1);
    lua_pushlightuserdata(L, &s_FfiKey);
    lua_pushvalue(L, -2);
    lua_rawset(L, LUA_REGISTRYINDEX);
    return 1;
}
#endif
