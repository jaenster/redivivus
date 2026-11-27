// D2Server CORE\ToMCP.cpp: the server's messages to the MCP, and the game-data table
// D2Game keeps its games in.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.
//
// Every message goes out on the MCP link CharServ.cpp opens; byte 0 is the message
// type.

#include "Source/D2Server/Core/ToMCP.h"
#include "Source/D2Server/Core/CharServ.h"
#include "Source/D2Server/Core/Inifile.h"
#include "Source/D2Server/Core/WinMain.h"
#include "Source/D2Server/GameTable.h"
#include "Source/D2Server/D2ServerImports.h"

namespace D2Server::Core::ToMCP {

using namespace D2Server::Imports;
using CharServ::g_McpConnection;

static const char s_szToMCPFile[] = "C:\\D2\\Source\\D2Server\\CORE\\ToMCP.cpp";

// The game-data table handed to GAME_InitGameDataTable (WinMain).
GameDataTable g_gameDataParam1;                                       // 1.00 D2Server: 10029688

// 0x01, one byte.
// 1.00 D2Server: 10008550
void __stdcall ToMCP_SendPing() {
    uint8_t nCmd = 0x01;
    FOG_SOCKET_Send(g_McpConnection, &nCmd, 1);
}

// 0x02: the version words.
// 1.00 D2Server: 10008580
void __stdcall ToMCP_SendVersionHandshake() {
    uint8_t aPacket[0x11];
    aPacket[0] = 0x02;
    *(uint32_t *)(aPacket + 0x01) = 0x49583836;    // "68XI"
    *(uint32_t *)(aPacket + 0x05) = 0x44324456;    // "VD2D"
    *(uint32_t *)(aPacket + 0x09) = 0;
    *(uint32_t *)(aPacket + 0x0D) = 0;
    FOG_SOCKET_Send(g_McpConnection, aPacket, sizeof(aPacket));
}

// 0x03: 0x109 bytes, of which the first ten are set.
// 1.00 D2Server: 100085d0
void __stdcall ToMCP_SendRealmInfo() {
    uint8_t aPacket[0x109];
    aPacket[0] = 0x03;
    *(uint32_t *)(aPacket + 0x01) = 0;
    *(uint32_t *)(aPacket + 0x05) = 0;
    aPacket[0x09] = 0;
    FOG_SOCKET_Send(g_McpConnection, aPacket, sizeof(aPacket));
}

// 0x04: one game of the list. [1] id, [5] address, [9..0xc] three info bytes and the
// flags, [0xd] more to come, [0x11] description\0 name\0 ""\0.
// 1.00 D2Server: 10008620
void __fastcall ToMCP_SendSingleGameEntry(D2GameInfoEntry * pGame, int bIsLast) {
    uint8_t aPacket[0x215];
    aPacket[0] = 0x04;
    *(uint32_t *)(aPacket + 0x01) = *(uint32_t *)pGame->unk0x00;
    *(uint32_t *)(aPacket + 0x05) = pGame->dwServerIp;
    aPacket[0x09] = pGame->nField84;
    aPacket[0x0A] = pGame->nField85;
    aPacket[0x0B] = pGame->nField86;
    aPacket[0x0C] = pGame->nFlags;
    *(uint32_t *)(aPacket + 0x0D) = bIsLast == 0;
    char * pText = (char *)aPacket + 0x11;
    pText += SStrCopy(pText, pGame->szDescription, 0x200) + 1;
    pText += SStrCopy(pText, pGame->szName, 0x200) + 1;
    int nLast = SStrCopy(pText, Inifile::g_szIniWhitespaceChars, 0x200);
    FOG_SOCKET_Send(g_McpConnection, aPacket, (int)((uint8_t *)pText - aPacket) + nLast + 1);
}

// The game list, one message per game. The info passed is a local nothing fills.
// 1.00 D2Server: 10008700
int __stdcall ToMCP_SendGameListEntries() {
    D2GameInfoEntry tGameInfo;
    uint16_t aGameIds[0x400];
    GAME_GetGamesCount();
    int nGames = GAME_GetGameServerGameIds(aGameIds, 0x400);
    for (int i = 0; i < nGames; ++i) {
        ToMCP_SendSingleGameEntry(&tGameInfo, i == nGames - 1);
    }
    return 1;
}

// 0x05, five bytes of which the first is set.
// 1.00 D2Server: 10008780
void __stdcall ToMCP_SendLeaveGame() {
    uint8_t aPacket[5];
    aPacket[0] = 0x05;
    FOG_SOCKET_Send(g_McpConnection, aPacket, sizeof(aPacket));
}

// 0x06: [1] the MCP's token, [5] the game id, [9] the error.
// 1.00 D2Server: 100087c0
void __stdcall ToMCP_SendCreateGameReply(uint32_t dwGameId, uint32_t dwMcpToken, uint32_t dwError) {
    uint8_t aPacket[0x0D];
    WinMain::PostStatusMessage(0, (char *)"[Server->MCP]  (GTM_CREATEGAME)  MCP ID: %d  SERV ID: %d  ERROR: %d", dwMcpToken, dwGameId, dwError);
    aPacket[0] = 0x06;
    *(uint32_t *)(aPacket + 0x01) = dwMcpToken;
    *(uint32_t *)(aPacket + 0x05) = dwGameId;
    *(uint32_t *)(aPacket + 0x09) = dwError;
    FOG_SOCKET_Send(g_McpConnection, aPacket, sizeof(aPacket));
}

// 0x07: [1] request, [5] game token, [9] client token, [0xd] error.
// 1.00 D2Server: 10008840
void __fastcall ToMCP_SendJoinGameReply(uint32_t dwRequestId, uint32_t dwGameToken, uint32_t dwClientToken, uint32_t dwError) {
    uint8_t aPacket[0x11];
    WinMain::PostStatusMessage(0, (char *)"[Server->MCP]  (GTM_JOINGAME)  MCP ID: %d  SERV ID: %d  TOK: %08x  ERR: %08x", dwRequestId, dwGameToken, dwClientToken, dwError);
    aPacket[0] = 0x07;
    *(uint32_t *)(aPacket + 0x01) = dwRequestId;
    *(uint32_t *)(aPacket + 0x05) = dwGameToken;
    *(uint32_t *)(aPacket + 0x09) = dwClientToken;
    *(uint32_t *)(aPacket + 0x0D) = dwError;
    FOG_SOCKET_Send(g_McpConnection, aPacket, sizeof(aPacket));
}

// The character dword of 0x08 and 0x0F: class in bits 0-2, 0x20 expansion, 0x10 its
// second flag, the level in bits 16-23, and the flags' bits 8-12 once they reach 4.
static uint32_t ToMCPPackCharacter(uint16_t wClass, uint32_t dwLevel, uint32_t dwFlags) {
    uint32_t dwPacked = ((dwLevel & 0xFF) << 16) | (wClass & 7);
    if (dwFlags & 4) {
        dwPacked |= 0x20;
        if (dwFlags & 8) {
            dwPacked |= 0x10;
        }
    }
    if ((dwFlags & 0x1F00) >= 0x400) {
        dwPacked = (dwPacked & ~0x1F00u) | (dwFlags & 0x1F00);
    }
    return dwPacked;
}

// 0x08: a character. [1] token, [5] packed class/level/flags, [9] level, [0xb]
// experience low, [0xf] high, [0x13] flags, [0x15] character\0 account\0.
// 1.00 D2Server: 100088c0
int __fastcall ToMCP_SendCharacterList(uint32_t dwToken, uint16_t wClass, uint16_t wLevel, uint32_t dwExperienceLo, uint32_t dwExperienceHi, uint32_t dwFlags, char * szCharName, char * szAccountName) {
    uint8_t aPacket[0x219];
    aPacket[0] = 0x08;
    *(uint32_t *)(aPacket + 0x01) = dwToken;
    *(uint32_t *)(aPacket + 0x05) = ToMCPPackCharacter(wClass, wLevel, dwFlags);
    *(uint16_t *)(aPacket + 0x09) = wLevel;
    *(uint32_t *)(aPacket + 0x0B) = dwExperienceLo;
    *(uint32_t *)(aPacket + 0x0F) = dwExperienceHi;
    *(uint16_t *)(aPacket + 0x13) = (uint16_t)dwFlags;
    int nChar = SStrCopy((char *)aPacket + 0x15, szCharName, 0x200);
    char * pAccount = (char *)aPacket + 0x16 + nChar;
    int nAccount = SStrCopy(pAccount, szAccountName, 0x200);
    return FOG_SOCKET_Send(g_McpConnection, aPacket, nAccount + (int)(pAccount - (char *)aPacket - 0x15) + 0x16);
}

// 0x0A: [1] token, [5..9] three words, [0xb] text.
// 1.00 D2Server: 100089e0
int __fastcall ToMCP_SendMotd(uint32_t dwToken, char * szText, uint16_t wField1, uint16_t wField2, uint16_t wField3) {
    uint8_t aPacket[0x20C];
    aPacket[0] = 0x0A;
    *(uint32_t *)(aPacket + 0x01) = dwToken;
    *(uint16_t *)(aPacket + 0x05) = wField1;
    *(uint16_t *)(aPacket + 0x07) = wField2;
    *(uint16_t *)(aPacket + 0x09) = wField3;
    int nText = SStrCopy((char *)aPacket + 0x0B, szText, 0x200);
    return FOG_SOCKET_Send(g_McpConnection, aPacket, nText + 0x0C);
}

// 0x09, five bytes of which the first is set.
// 1.00 D2Server: 10008a70
void __stdcall ToMCP_SendEnterGame() {
    uint8_t aPacket[5];
    aPacket[0] = 0x09;
    FOG_SOCKET_Send(g_McpConnection, aPacket, sizeof(aPacket));
}

// 0x0B, one byte: the half-minute keep-alive.
// 1.00 D2Server: 10008ab0
void __stdcall ToMCP_SendLeaveGameAck() {
    uint8_t nCmd = 0x0B;
    FOG_SOCKET_Send(g_McpConnection, &nCmd, 1);
}

// 0x0C: a character's data in pieces of at most 400 bytes. [1] the name (the header
// words that follow overwrite all but its first four bytes), [5] offset, [7] size,
// [9] total, [0xb] data. Ghidra's ToMCP_SendCharacterDataChunk (10008b05) is the loop
// of this function, entered in the middle.
// 1.00 D2Server: 10008ae0
int __fastcall ToMCP_SendCharacterData(char * szCharName, uint8_t * pData, int nDataLen) {
    uint8_t aPacket[0x19C];
    int nResult;
    if (nDataLen == 0) {
        return 1;
    }
    aPacket[0] = 0x0C;
    strcpy((char *)aPacket + 1, szCharName);
    *(uint16_t *)(aPacket + 0x09) = (uint16_t)nDataLen;
    uint16_t nOffset = 0;
    *(uint16_t *)(aPacket + 0x05) = 0;
    do {
        uint16_t nChunk = nDataLen < 0x191 ? (uint16_t)nDataLen : 400;
        *(uint16_t *)(aPacket + 0x07) = nChunk;
        memcpy(aPacket + 0x0B, pData + nOffset, nChunk);
        nResult = FOG_SOCKET_Send(g_McpConnection, aPacket, nChunk + 0x0B);
        nOffset += nChunk;
        *(uint16_t *)(aPacket + 0x05) = nOffset;
        nDataLen -= nChunk;
    } while (nDataLen != 0);
    return nResult;
}

// 0x0E: a game opened. [1] difficulty, [2] name (overwrites the flags written first
// when longer than three characters), [6] flags, [0xa] description.
// 1.00 D2Server: 10008c00
int __fastcall ToMCP_SendGameOpen(char * pGameName, uint8_t nDifficulty, char * pDescription, uint32_t dwGameFlags) {
    uint8_t aPacket[0x40];
    *(uint32_t *)(aPacket + 0x06) = dwGameFlags;
    aPacket[0] = 0x0E;
    strcpy((char *)aPacket + 0x02, pGameName);
    strcpy((char *)aPacket + 0x0A, pDescription);
    aPacket[0x01] = nDifficulty;
    return FOG_SOCKET_Send(g_McpConnection, aPacket, 0x1A);
}

// 0x0F: a character entering, sent only when its experience reaches the ladder total
// or its class's threshold (the expansion table when flag 4 is set). [1] packed
// class/level/flags, [5] experience low, [9] high, [0xd] flags, [0xf] name.
// 1.00 D2Server: 10008ca0
int __fastcall ToMCP_SendCharEnter(char * pCharName, uint16_t wClass, uint32_t dwLevel, uint32_t dwExperienceLo, uint32_t dwExperienceHi, uint32_t dwFlags) {
    uint8_t aPacket[0x210];
    if (wClass >= 5) {
        FOG_DisplayAssert("wClass < NUM_PLAYERCLASS", s_szToMCPFile, 0x129);
        _exit(-1);
    }
    uint64_t nExperience = ((uint64_t)dwExperienceHi << 32) | dwExperienceLo;
    const LadderThresholds & tLadder = WinMain::g_tLadderThresholds;
    if ((uint16_t)(dwFlags & 4)) {
        if (nExperience < tLadder.nExpansionTotal && nExperience < tLadder.aExpansionClass[wClass]) {
            return 0;
        }
    } else {
        if (nExperience < tLadder.nClassicTotal && nExperience < tLadder.aClassicClass[wClass]) {
            return 0;
        }
    }
    aPacket[0] = 0x0F;
    *(uint32_t *)(aPacket + 0x01) = ToMCPPackCharacter(wClass, dwLevel, dwFlags);
    *(uint32_t *)(aPacket + 0x05) = dwExperienceLo;
    *(uint32_t *)(aPacket + 0x09) = dwExperienceHi;
    *(uint16_t *)(aPacket + 0x0D) = (uint16_t)dwFlags;
    int nName = SStrCopy((char *)aPacket + 0x0F, pCharName, 0x200);
    return FOG_SOCKET_Send(g_McpConnection, aPacket, nName + 0x10);
}

// 0x10: a character created. [1] token, [5] class, [7] name.
// 1.00 D2Server: 10008e20
int __fastcall ToMCP_SendCharCreate(uint32_t dwToken, char * pCharName, uint16_t wClass) {
    uint8_t aPacket[0x208];
    if (wClass >= 5) {
        FOG_DisplayAssert("wClass < NUM_PLAYERCLASS", s_szToMCPFile, 0x15D);
        _exit(-1);
    }
    aPacket[0] = 0x10;
    *(uint32_t *)(aPacket + 0x01) = dwToken;
    *(uint16_t *)(aPacket + 0x05) = wClass;
    int nName = SStrCopy((char *)aPacket + 0x07, pCharName, 0x200);
    return FOG_SOCKET_Send(g_McpConnection, aPacket, nName + 8);
}

// 0x11: the minute heartbeat. [1] games, [3] 0x400, [5] 0, [7] 0x400, [9] seconds
// up, [0xd] Fog's four counters, [0x1d] Fog's time >> 10 (saturated), [0x21] players.
// 1.00 D2Server: 10008ec0
void __fastcall ToMCP_SendGameJoinInfo(GameJoinInfoFields * pFields) {
    uint8_t aPacket[0x22];
    aPacket[0] = 0x11;
    *(uint16_t *)(aPacket + 0x01) = pFields->nGames;
    *(uint16_t *)(aPacket + 0x03) = (uint16_t)WinMain::GetWorkerStackSize();
    *(uint16_t *)(aPacket + 0x05) = pFields->wPad;
    *(uint16_t *)(aPacket + 0x07) = (uint16_t)WinMain::GetWorkerStackSize2();
    *(uint32_t *)(aPacket + 0x09) = (uint32_t)WinMain::GetCurrentTimeAsTick();
    FOG_10154((uint32_t *)(aPacket + 0x0D), (uint32_t *)(aPacket + 0x11), (uint32_t *)(aPacket + 0x15), (uint32_t *)(aPacket + 0x19));
    uint64_t nTime = FOG_10153() >> 10;
    *(uint32_t *)(aPacket + 0x1D) = (nTime >> 32) != 0 ? 0xFFFFFFFF : (uint32_t)nTime;
    aPacket[0x21] = pFields->nPlayers;
    FOG_SOCKET_Send(g_McpConnection, aPacket, sizeof(aPacket));
}

// The game-data table's destructor, run at exit: its critical section, then the base
// destructor (GameTable_DestructBody, which the binary has inline here).
// 1.00 D2Server: 10008fe0
void __fastcall ToMCP_Destructor(GameDataTable * pThis) {
    DeleteCriticalSection(&pThis->cs);
    GameTable::GameTable_DestructBody(pThis);
}

} // namespace D2Server::Core::ToMCP

// The construction and destruction the compiler emits for the global (its dynamic
// initializer at 10008f90 and the atexit thunk at 10008fc0).
D2Server::GameDataTable::GameDataTable() {
    D2Server::GameTable::GameTable_Construct(this);
}

D2Server::GameDataTable::~GameDataTable() {
    D2Server::Core::ToMCP::ToMCP_Destructor(this);
}
