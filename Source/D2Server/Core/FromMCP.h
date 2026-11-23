// D2Server CORE\FromMCP.cpp: the MCP's messages to the server.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.

#pragma once

#include "Source/D2Server/D2Server.h"

namespace D2Server::Core::FromMCP {

int __stdcall SetServerInterface(void * pServerInterface);                      // 1.00 D2Server: 100056e0
void ** __stdcall QueryInterface();                                             // 1.00 D2Server: 10005710
void __fastcall OnServerStart(D2ServerLaunchConfig * pConfig);                  // 1.00 D2Server: 10005730
int __fastcall OnUnknown1_Stub(uint8_t * pPacket, int nLength);                 // 1.00 D2Server: 10005760
int __fastcall OnVersionHandshake(uint8_t * pPacket, int nLength);              // 1.00 D2Server: 10005780
int __fastcall OnGetRealmList(uint8_t * pPacket, int nLength);                  // 1.00 D2Server: 100057a0
int __fastcall OnGetGameList(uint8_t * pPacket, int nLength);                   // 1.00 D2Server: 100057c0
int __fastcall OnGameRecreate_Unsupported(uint8_t * pPacket, int nLength);      // 1.00 D2Server: 100057e0
int __fastcall OnUnknown6_Stub(uint8_t * pPacket, int nLength);                 // 1.00 D2Server: 10005810
int __fastcall OnCreateGame(MCP_GTM_CreateGameReq * pPkt, int nLength);         // 1.00 D2Server: 10005830
int __fastcall OnJoinGame(MCP_GTM_JoinGameReq * pPkt, int nLength);             // 1.00 D2Server: 100058c0
int __fastcall CB_OnPacket_0x0C_CreateGame(MCP_GTM_CB_GameReq * pPkt, uint32_t nLen); // 1.00 D2Server: 10005930
int __fastcall CB_OnPacket_0x0D_JoinGame(MCP_GTM_CB_GameReq * pPkt, uint32_t nLen); // 1.00 D2Server: 10005980
int __fastcall OnLadderThresholds(uint8_t * pPacket, uint32_t nLen);            // 1.00 D2Server: 100059d0
int __fastcall OnGlobalMessage(uint8_t * pPacket, uint32_t nLen);               // 1.00 D2Server: 10005a20
int __fastcall OnShutdown(uint8_t * pPacket, uint32_t nLen);                    // 1.00 D2Server: 10005ab0
int __fastcall DispatchClientPacket(uint8_t * pPacket, int nLength);            // 1.00 D2Server: 10005ae0
int __fastcall DispatchClientPacketAlt(uint8_t * pPacket, int nLength);         // 1.00 D2Server: 10005b30

} // namespace D2Server::Core::FromMCP
