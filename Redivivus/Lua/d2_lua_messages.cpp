// Mod messages: bytes a mod's half on one side sends to its half on the other. The envelope, version 1:
//   byte 0      format version (1)
//   bytes 1-2   the sending mod's index in load order, little endian (both ends run the same
//               mod list in the same order: the server names it and the client refuses to join
//               on a mismatch)
//   bytes 3..   the payload, at most D2LUA_MESSAGE_MAX bytes, opaque here (the kit's channels
//               put their own format inside)
// A message belongs to a game (the transport carries its id) and is delivered to the same mod on the
// receiving side, in that game: the core decodes it for the channels the mod opened there and runs
// the handler as a call of that mod, under the time budget and the fault rules. Nothing about the
// sender is trusted beyond the peer the transport names.
#if D2_FEATURE_MODS
#include <string.h>

#include <deque>
#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_core.h"
#include "Redivivus/Lua/d2_lua_internal.h"
#include "Redivivus/Lua/d2_lua_rt.h"
#include "lua.hpp"

static const unsigned char kMessageVersion = 1;
static const size_t kEnvelope = 3;

void D2LuaRt_SetTransport(D2LuaRuntime* pRt, const D2LuaTransport& tTransport) {
    pRt->tTransport = tTransport;
}

// Send the payload at nPayload as mod pMod in game nGame. On the server the peer at nPeerIndex names
// the client (0: every client); on the client the message goes to the server.
int D2LuaMessages_Send(lua_State* L, uint32_t nGame, D2LuaMod* pMod, int nPayloadIndex, int nPeerIndex) {
    size_t nPayload = 0;
    const char* pPayload = luaL_checklstring(L, nPayloadIndex, &nPayload);
    lua_Number nPeer = luaL_optnumber(L, nPeerIndex, 0);
    if (nPayload > D2LUA_MESSAGE_MAX) {
        return luaL_error(L, "a channel message is %d bytes; a message carries at most %d", (int)nPayload, D2LUA_MESSAGE_MAX);
    }
    if (nPeer < 0 || nPeer > 4294967295.0 || nPeer != (lua_Number)(uint32_t)nPeer) {
        return luaL_error(L, "a peer id is a whole number from 0");
    }
    D2LuaRuntime* pRt = pMod->pRt;
    if (!pRt->tTransport.pfnSend) {
        return luaL_error(L, "mod messages are not connected in this game");
    }
    char* pBuffer = (char*)lua_newuserdata(L, kEnvelope + nPayload);  // freed by the collector
    pBuffer[0] = (char)kMessageVersion;
    pBuffer[1] = (char)(pMod->nIndex & 0xff);
    pBuffer[2] = (char)((pMod->nIndex >> 8) & 0xff);
    memcpy(pBuffer + kEnvelope, pPayload, nPayload);
    pRt->tTransport.pfnSend(pRt->tTransport.pUser, pRt, nGame, pRt->tDesc.nSide == D2LUA_SIDE_SERVER ? (uint32_t)nPeer : 0, pBuffer,
                            kEnvelope + nPayload);
    lua_pop(L, 1);
    return 0;
}

static void LogDrop(D2LuaRuntime* pRt, uint32_t nPeer, const char* pszWhy) {
    if (pRt->tDesc.pfnLog) {
        std::string sLine = "[lua] mod message from peer " + std::to_string(nPeer) + " dropped: " + pszWhy;
        pRt->tDesc.pfnLog(sLine.c_str());
    }
}

bool D2LuaRt_Deliver(D2LuaRuntime* pRt, uint32_t nGame, uint32_t nPeer, const void* pData, size_t nSize) {
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
    if (pMod->bIntrinsic || D2LuaRt_ModFaulted(L, pMod)) {
        return false;
    }
    int nTop = lua_gettop(L);
    uint32_t nWas = pRt->nGame;
    pRt->nGame = nGame;
    lua_pushnumber(L, (lua_Number)nGame);
    lua_pushinteger(L, pMod->nHostId);
    lua_pushlstring(L, (const char*)p + kEnvelope, nSize - kEnvelope);
    lua_pushnumber(L, (lua_Number)nPeer);
    bool bDelivered = D2LuaCore_CallHost(L, "deliver", 4, 1) && lua_toboolean(L, -1);
    lua_settop(L, nTop);
    pRt->nGame = nWas;
    if (!bDelivered) {
        LogDrop(pRt, nPeer, ("mod '" + pMod->sName + "' opened no channel in this game on this side").c_str());
    }
    return bDelivered;
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
        uint32_t nGame;
        uint32_t nFrom;
        std::string sBytes;
    };
    D2LuaRuntime* pServer = nullptr;
    std::vector<Client> aClients;
    std::deque<Message> aQueue;
    size_t nDropped = 0;  // messages that could not be queued
};

static void LoopbackSend(void* pUser, D2LuaRuntime* pFrom, uint32_t nGame, uint32_t nPeer, const void* pData, size_t nSize) {
    D2LuaLoopback* pLoop = (D2LuaLoopback*)pUser;
    try {
        std::string sBytes((const char*)pData, nSize);
        if (pFrom == pLoop->pServer) {
            for (const D2LuaLoopback::Client& c : pLoop->aClients) {
                if (nPeer == 0 || c.nPeer == nPeer) {
                    pLoop->aQueue.push_back({ c.pRt, nGame, 0, sBytes });
                }
            }
            return;
        }
        for (const D2LuaLoopback::Client& c : pLoop->aClients) {
            if (c.pRt == pFrom) {
                pLoop->aQueue.push_back({ pLoop->pServer, nGame, c.nPeer, sBytes });
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
        D2LuaRt_Deliver(tMessage.pTo, tMessage.nGame, tMessage.nFrom, tMessage.sBytes.data(), tMessage.sBytes.size());
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
