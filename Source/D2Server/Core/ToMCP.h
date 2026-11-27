// D2Server CORE\ToMCP.cpp: the server's messages to the MCP.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.

#pragma once

#include "Source/D2Server/D2Server.h"

namespace D2Server::Core::ToMCP {

void __stdcall ToMCP_SendPing();                                                // 1.00 D2Server: 10008550
void __stdcall ToMCP_SendVersionHandshake();                                    // 1.00 D2Server: 10008580
void __stdcall ToMCP_SendRealmInfo();                                           // 1.00 D2Server: 100085d0
void __fastcall ToMCP_SendSingleGameEntry(D2GameInfoEntry * pGame, int bIsLast); // 1.00 D2Server: 10008620
int __stdcall ToMCP_SendGameListEntries();                                      // 1.00 D2Server: 10008700
void __stdcall ToMCP_SendLeaveGame();                                           // 1.00 D2Server: 10008780
void __stdcall ToMCP_SendCreateGameReply(uint32_t dwGameId, uint32_t dwMcpToken, uint32_t dwError); // 1.00 D2Server: 100087c0
void __fastcall ToMCP_SendJoinGameReply(uint32_t dwRequestId, uint32_t dwGameToken, uint32_t dwClientToken, uint32_t dwError); // 1.00 D2Server: 10008840
int __fastcall ToMCP_SendCharacterList(uint32_t dwToken, uint16_t wClass, uint16_t wLevel, uint32_t dwExperienceLo, uint32_t dwExperienceHi, uint32_t dwFlags, char * szCharName, char * szAccountName); // 1.00 D2Server: 100088c0
int __fastcall ToMCP_SendMotd(uint32_t dwToken, char * szText, uint16_t wField1, uint16_t wField2, uint16_t wField3); // 1.00 D2Server: 100089e0
void __stdcall ToMCP_SendEnterGame();                                           // 1.00 D2Server: 10008a70
void __stdcall ToMCP_SendLeaveGameAck();                                        // 1.00 D2Server: 10008ab0
int __fastcall ToMCP_SendCharacterData(char * szCharName, uint8_t * pData, int nDataLen); // 1.00 D2Server: 10008ae0
int __fastcall ToMCP_SendGameOpen(char * pGameName, uint8_t nDifficulty, char * pDescription, uint32_t dwGameFlags); // 1.00 D2Server: 10008c00
int __fastcall ToMCP_SendCharEnter(char * pCharName, uint16_t wClass, uint32_t dwLevel, uint32_t dwExperienceLo, uint32_t dwExperienceHi, uint32_t dwFlags); // 1.00 D2Server: 10008ca0
int __fastcall ToMCP_SendCharCreate(uint32_t dwToken, char * pCharName, uint16_t wClass); // 1.00 D2Server: 10008e20
void __fastcall ToMCP_SendGameJoinInfo(GameJoinInfoFields * pFields);          // 1.00 D2Server: 10008ec0
void __fastcall ToMCP_Destructor(GameDataTable * pThis);                             // 1.00 D2Server: 10008fe0

// The game-data table GAME_InitGameDataTable fills (a Storm TSHashTable<SGAMEDATA>
// object; GameTable.cpp holds its member functions).
extern GameDataTable g_gameDataParam1;                                          // 1.00 D2Server: 10029688

} // namespace D2Server::Core::ToMCP
