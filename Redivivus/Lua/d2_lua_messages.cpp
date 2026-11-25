// Mod messages: bytes a mod's half on one side sends to its half on the other.
// The envelope, version 1:
//   byte 0      format version (1)
//   bytes 1-2   the sending mod's index in load order, little endian (both ends run the same
//               mod list in the same order: the server names it and the client refuses to join
//               on a mismatch)
//   bytes 3..   the payload, at most D2LUA_MESSAGE_MAX bytes, opaque here (the kit's channels
//               put their own format inside)
// A message is delivered to the same mod on the receiving side, through its script.on_message
// handler, as a guarded entry of that mod: the time budget and the fault rules apply to it as to
// any other call into a mod. Nothing about the sender is trusted beyond the peer the transport
// names.
#if D2_FEATURE_MODS
#include <string.h>

#include <deque>
#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_internal.h"
#include "Redivivus/Lua/d2_lua_rt.h"
#include "lua.hpp"

static const unsigned char kMessageVersion = 1;
static const size_t kEnvelope = 3;

void D2LuaRt_SetTransport(D2LuaRuntime* pRt, const D2LuaTransport& tTransport) {
    pRt->tTransport = tTransport;
}

// script.send(payload, peer?). On the server peer names the client (0: every client); on the
// client the message goes to the server.
int D2LuaScript_Send(D2LuaCtx& ctx) {
    lua_State* L = ctx.L;
    size_t nPayload = 0;
    const char* pPayload = luaL_checklstring(L, 1, &nPayload);
    lua_Number nPeer = luaL_optnumber(L, 2, 0);
    if (nPayload > D2LUA_MESSAGE_MAX) {
        return luaL_error(L, "script.send: the payload is %d bytes; a message carries at most %d", (int)nPayload, D2LUA_MESSAGE_MAX);
    }
    if (nPeer < 0 || nPeer > 4294967295.0 || nPeer != (lua_Number)(uint32_t)nPeer) {
        return luaL_error(L, "bad argument #2 to 'script.send' (a peer id is a whole number from 0)");
    }
    D2LuaRuntime* pRt = ctx.pRuntime;
    if (!pRt->tTransport.pfnSend) {
        return luaL_error(L, "script.send: mod messages are not connected in this game");
    }
    char* pBuffer = (char*)lua_newuserdata(L, kEnvelope + nPayload);  // freed by the collector
    pBuffer[0] = (char)kMessageVersion;
    pBuffer[1] = (char)(ctx.pMod->nIndex & 0xff);
    pBuffer[2] = (char)((ctx.pMod->nIndex >> 8) & 0xff);
    memcpy(pBuffer + kEnvelope, pPayload, nPayload);
    pRt->tTransport.pfnSend(pRt->tTransport.pUser, pRt, pRt->tDesc.nSide == D2LUA_SIDE_SERVER ? (uint32_t)nPeer : 0, pBuffer,
                            kEnvelope + nPayload);
    lua_pop(L, 1);
    return 0;
}

void D2LuaScript_OnMessage(D2LuaCtx& ctx, D2LuaFn fn) {
    lua_State* L = ctx.L;
    D2LuaRt_PushField(L, "messages");
    if (fn.nIndex) {
        lua_pushvalue(L, fn.nIndex);
    } else {
        lua_pushnil(L);
    }
    lua_setfield(L, -2, ctx.pMod->sName.c_str());
    lua_pop(L, 1);
}

static void LogDrop(D2LuaRuntime* pRt, uint32_t nPeer, const char* pszWhy) {
    if (pRt->tDesc.pfnLog) {
        std::string sLine = "[lua] mod message from peer " + std::to_string(nPeer) + " dropped: " + pszWhy;
        pRt->tDesc.pfnLog(sLine.c_str());
    }
}

bool D2LuaRt_Deliver(D2LuaRuntime* pRt, uint32_t nPeer, const void* pData, size_t nSize) {
    const unsigned char* p = (const unsigned char*)pData;
    if (nSize < kEnvelope || nSize > kEnvelope + D2LUA_MESSAGE_MAX) {
        LogDrop(pRt, nPeer, "bad size");
        return false;
    }
    if (p[0] != kMessageVersion) {
        LogDrop(pRt, nPeer, "unknown format version");
        return false;
    }
    size_t nIndex = (size_t)p[1] | ((size_t)p[2] << 8);
    if (nIndex >= pRt->aMods.size()) {
        LogDrop(pRt, nPeer, "no such mod");
        return false;
    }
    D2LuaMod* pMod = pRt->aMods[nIndex].get();
    lua_State* L = pRt->L;
    if (D2LuaRt_ModFaulted(L, pMod)) {
        return false;
    }
    D2LuaRt_PushField(L, "messages");
    lua_getfield(L, -1, pMod->sName.c_str());
    lua_remove(L, -2);
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        LogDrop(pRt, nPeer, ("mod '" + pMod->sName + "' does not receive messages on this side").c_str());
        return false;
    }
    lua_pushlstring(L, (const char*)p + kEnvelope, nSize - kEnvelope);
    lua_pushnumber(L, (lua_Number)nPeer);
    return D2LuaHost_GuardedCall(L, pMod->sName.c_str(), 2, 0) == D2LUA_OK;
}

// --- the loopback transport -------------------------------------------------------------------------
// Joins one server runtime and its clients in one process: what one side sends is queued and
// delivered to the other by D2LuaLoopback_Pump, in send order.

struct D2LuaLoopback {
    struct Client {
        D2LuaRuntime* pRt;
        uint32_t nPeer;
    };
    struct Message {
        D2LuaRuntime* pTo;
        uint32_t nFrom;
        std::string sBytes;
    };
    D2LuaRuntime* pServer = nullptr;
    std::vector<Client> aClients;
    std::deque<Message> aQueue;
    size_t nDropped = 0;  // messages that could not be queued
};

static void LoopbackSend(void* pUser, D2LuaRuntime* pFrom, uint32_t nPeer, const void* pData, size_t nSize) {
    D2LuaLoopback* pLoop = (D2LuaLoopback*)pUser;
    try {
        std::string sBytes((const char*)pData, nSize);
        if (pFrom == pLoop->pServer) {
            for (const D2LuaLoopback::Client& c : pLoop->aClients) {
                if (nPeer == 0 || c.nPeer == nPeer) {
                    pLoop->aQueue.push_back({ c.pRt, 0, sBytes });
                }
            }
            return;
        }
        for (const D2LuaLoopback::Client& c : pLoop->aClients) {
            if (c.pRt == pFrom) {
                pLoop->aQueue.push_back({ pLoop->pServer, c.nPeer, sBytes });
            }
        }
    } catch (...) {
        pLoop->nDropped++;
    }
}

D2LuaLoopback* D2LuaLoopback_Create(D2LuaRuntime* pServer) {
    D2LuaLoopback* pLoop = new D2LuaLoopback;
    pLoop->pServer = pServer;
    D2LuaRt_SetTransport(pServer, { LoopbackSend, pLoop });
    return pLoop;
}

void D2LuaLoopback_AddClient(D2LuaLoopback* pLoop, D2LuaRuntime* pClient, uint32_t nPeer) {
    pLoop->aClients.push_back({ pClient, nPeer });
    D2LuaRt_SetTransport(pClient, { LoopbackSend, pLoop });
}

int D2LuaLoopback_Pump(D2LuaLoopback* pLoop) {
    int nDelivered = 0;
    while (!pLoop->aQueue.empty()) {
        D2LuaLoopback::Message tMessage = pLoop->aQueue.front();
        pLoop->aQueue.pop_front();
        D2LuaRt_Deliver(tMessage.pTo, tMessage.nFrom, tMessage.sBytes.data(), tMessage.sBytes.size());
        nDelivered++;
    }
    return nDelivered;
}

void D2LuaLoopback_Destroy(D2LuaLoopback* pLoop) {
    if (!pLoop) {
        return;
    }
    D2LuaRt_SetTransport(pLoop->pServer, { nullptr, nullptr });
    for (const D2LuaLoopback::Client& c : pLoop->aClients) {
        D2LuaRt_SetTransport(c.pRt, { nullptr, nullptr });
    }
    delete pLoop;
}
#endif
