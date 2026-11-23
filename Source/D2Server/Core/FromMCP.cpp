// D2Server CORE\FromMCP.cpp: the MCP's messages to the server, and the interface the
// MCP host loads the DLL through.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.
//
// The 0x06/0x07 handlers read the game name with no length check; the 0x0C/0x0D forms
// check a minimum length first.

#include "Source/D2Server/Core/FromMCP.h"
#include "Source/D2Server/Core/ToMCP.h"
#include "Source/D2Server/Core/WinMain.h"
#include "Source/D2Server/PlayerToken.h"
#include "Source/D2Server/D2ServerImports.h"

namespace D2Server::Core::FromMCP {

using namespace D2Server::Imports;

typedef int (__fastcall * FromMCPHandler)(uint8_t * pPacket, int nLength);

static void * g_pServerInterface = nullptr;                           // 1.00 D2Server: 10026cdc
static FromMCPHandler DAT_10026ce0[20] = {};                          // 1.00 D2Server: 10026ce0 (never filled)

// The interface QueryInterface hands out: slot 0 starts the server, the rest is the
// handler of each message type (DispatchClientPacket reads it from slot 1).
static void * FromMCP_Vtable[21] = {                                  // 1.00 D2Server: 10023ab4
    (void *)OnServerStart,
    (void *)OnUnknown1_Stub,              // 0x00
    (void *)OnVersionHandshake,           // 0x01
    (void *)OnGetRealmList,               // 0x02
    (void *)OnGetGameList,                // 0x03
    (void *)OnGameRecreate_Unsupported,   // 0x04
    (void *)OnUnknown6_Stub,              // 0x05
    (void *)OnCreateGame,                 // 0x06
    (void *)OnJoinGame,                   // 0x07
    nullptr, nullptr, nullptr, nullptr,   // 0x08..0x0B
    (void *)CB_OnPacket_0x0C_CreateGame,  // 0x0C
    (void *)CB_OnPacket_0x0D_JoinGame,    // 0x0D
    nullptr,                              // 0x0E
    (void *)OnLadderThresholds,           // 0x0F
    nullptr, nullptr,                     // 0x10, 0x11
    (void *)OnGlobalMessage,              // 0x12
    (void *)OnShutdown,                   // 0x13
};

// 1.00 D2Server: 100056e0
int __stdcall SetServerInterface(void * pServerInterface) {
    g_pServerInterface = pServerInterface;
    return 1;
}

// 1.00 D2Server: 10005710
void ** __stdcall QueryInterface() {
    return FromMCP_Vtable;
}

// Slot 0: run the server, with the MCP's launch configuration. What the host gave
// SetServerInterface becomes WinMain's instance handle.
// 1.00 D2Server: 10005730
void __fastcall OnServerStart(D2ServerLaunchConfig * pConfig) {
    WinMain::WinMain((HINSTANCE)g_pServerInterface, nullptr, pConfig, SW_SHOW);
}

// 1.00 D2Server: 10005760
int __fastcall OnUnknown1_Stub(uint8_t * pPacket, int nLength) {
    return 1;
}

// 1.00 D2Server: 10005780
int __fastcall OnVersionHandshake(uint8_t * pPacket, int nLength) {
    ToMCP::ToMCP_SendVersionHandshake();
    return 1;
}

// 1.00 D2Server: 100057a0
int __fastcall OnGetRealmList(uint8_t * pPacket, int nLength) {
    ToMCP::ToMCP_SendRealmInfo();
    return 1;
}

// 1.00 D2Server: 100057c0
int __fastcall OnGetGameList(uint8_t * pPacket, int nLength) {
    ToMCP::ToMCP_SendGameListEntries();
    return 1;
}

// 1.00 D2Server: 100057e0
int __fastcall OnGameRecreate_Unsupported(uint8_t * pPacket, int nLength) {
    FOG_10022("GTM_RECREATE shouldn't be sent from MCP to Server!\n", "C:\\D2\\Source\\D2Server\\CORE\\FromMCP.cpp", 0x6B);
    _exit(-1);
}

// 1.00 D2Server: 10005810
int __fastcall OnUnknown6_Stub(uint8_t * pPacket, int nLength) {
    return 1;
}

// GTM 0x06: create a game; the password follows the name.
// 1.00 D2Server: 10005830
int __fastcall OnCreateGame(MCP_GTM_CreateGameReq * pPkt, int nLength) {
    char * pGameName = pPkt->szGameName;
    char * pPassword = pGameName + strlen(pGameName) + 1;
    uint16_t wGameId = 0;
    if (D2GAME_10047(pGameName, pPassword, 0, pPkt->dwFlags, pPkt->nDifficulty, pPkt->nGameType, pPkt->nMaxPlayers, &wGameId)) {
        ToMCP::ToMCP_SendCreateGameReply(wGameId, pPkt->dwMCPToken, 0);
        return 1;
    }
    ToMCP::ToMCP_SendCreateGameReply(0, pPkt->dwMCPToken, 1);
    return 1;
}

// GTM 0x07: a client is on its way; the account name follows the game name.
// 1.00 D2Server: 100058c0
int __fastcall OnJoinGame(MCP_GTM_JoinGameReq * pPkt, int nLength) {
    char * pGameName = pPkt->szGameName;
    PlayerToken::PlayerToken_RegisterOrUpdate(pGameName, pGameName + strlen(pGameName) + 1, pPkt->dwServerIp, pPkt->dwClientToken, pPkt->dwGameToken);
    ToMCP::ToMCP_SendJoinGameReply(pPkt->dwRequestId, pPkt->dwGameToken, pPkt->dwClientToken, 0);
    return 1;
}

// 1.00 D2Server: 10005930
int __fastcall CB_OnPacket_0x0C_CreateGame(MCP_GTM_CB_GameReq * pPkt, uint32_t nLen) {
    if (nLen < 0x0F) {
        return 0;
    }
    if (pPkt->nCmd != 0x0C) {
        return 0;
    }
    D2GAME_10048(pPkt->dwToken, &pPkt->pGameName, pPkt->wField0B, pPkt->wField09, pPkt->wField0D, pPkt->aPayload);
    return 1;
}

// 1.00 D2Server: 10005980
int __fastcall CB_OnPacket_0x0D_JoinGame(MCP_GTM_CB_GameReq * pPkt, uint32_t nLen) {
    if (nLen < 0x0F) {
        return 0;
    }
    if (pPkt->nCmd != 0x0D) {
        return 0;
    }
    D2GAME_10049(pPkt->dwToken, &pPkt->pGameName, pPkt->wField0B, pPkt->wField09, pPkt->wField0D, pPkt->aPayload);
    return 1;
}

// GTM 0x0F (exactly 0x89 bytes): the ladder thresholds, copied as they come.
// 1.00 D2Server: 100059d0 (Ghidra has no function here; reached from slot 0x0F)
int __fastcall OnLadderThresholds(uint8_t * pPacket, uint32_t nLen) {
    if (nLen != 0x89) {
        return 0;
    }
    if (pPacket[0] != 0x0F) {
        return 0;
    }
    memcpy(&WinMain::g_tLadderThresholds, pPacket + 1, sizeof(LadderThresholds));
    return 1;
}

// GTM 0x12 (more than 9 bytes): a message for every game, "Diablo: <text>".
// 1.00 D2Server: 10005a20 (Ghidra has no function here; reached from slot 0x12)
int __fastcall OnGlobalMessage(uint8_t * pPacket, uint32_t nLen) {
    char szMessage[0x100];
    if (nLen <= 9) {
        return 0;
    }
    if (pPacket[0] != 0x12) {
        return 0;
    }
    SStrPrintf(szMessage, sizeof(szMessage), "Diablo: %s", (char *)(pPacket + 9));
    D2GAME_10021(*(uint32_t *)(pPacket + 1), *(uint32_t *)(pPacket + 5), szMessage);
    return 1;
}

// GTM 0x13 (exactly 9 bytes): shut the server down.
// 1.00 D2Server: 10005ab0 (Ghidra has no function here; reached from slot 0x13)
int __fastcall OnShutdown(uint8_t * pPacket, uint32_t nLen) {
    if (nLen != 9) {
        return 0;
    }
    if (pPacket[0] != 0x13) {
        return 0;
    }
    WinMain::OnShutdownRemotely();
    return 1;
}

// 1.00 D2Server: 10005ae0
int __fastcall DispatchClientPacket(uint8_t * pPacket, int nLength) {
    int nResult = 0;
    if (nLength == 0) {
        return 0;
    }
    if (pPacket[0] > 0x13) {
        return 0;
    }
    FromMCPHandler pfHandler = (FromMCPHandler)FromMCP_Vtable[1 + pPacket[0]];
    if (pfHandler) {
        nResult = pfHandler(pPacket, nLength);
    }
    return nResult;
}

// The packet handler the MCP socket is opened with (FOG_SOCKET_SetBlockingMode); its
// table is never filled, so it answers 0.
// 1.00 D2Server: 10005b30
int __fastcall DispatchClientPacketAlt(uint8_t * pPacket, int nLength) {
    int nResult = 0;
    if (nLength == 0) {
        return 0;
    }
    if (pPacket[0] > 0x13) {
        return 0;
    }
    if (DAT_10026ce0[pPacket[0]]) {
        nResult = DAT_10026ce0[pPacket[0]](pPacket, nLength);
    }
    return nResult;
}

} // namespace D2Server::Core::FromMCP
