// D2Server: the types the host shares between its source files.
//
// Reconstructed from the 1.00 D2Server.dll (2000-05-26). Not part of the 1.14d byte
// match, and not linked into Game.exe yet: the tree builds it as its own object
// library (d2server_objs). Offsets are 1.00 offsets; the layout is 32-bit, pack(1)
// as in the binary.

#pragma once

#include <cstdint>
#include <cstddef>
#include "Source/D2Headers/d2_platform.h"

namespace D2Server {

#pragma pack(push, 1)

// The game-data element D2Game keeps per game. Its RTTI name is SGAMEDATA in 1.00
// and 1.14d alike, but D2Server only ever holds it as an opaque handle: no function
// of the host reads a body field. The body belongs to D2Game.
struct SGAMEDATA {
    void * vtable;                  // 0x00 TSHashObject vtable
    void * pNext;                   // 0x04 TSExplicitList next
    void * pPrev;                   // 0x08 TSExplicitList prev
    void * pHashNext;               // 0x0C hash-chain link
    uint32_t dwHashKey;             // 0x10 hash bucket key
    uint8_t aBody[0x304C];          // 0x14 owned by D2Game
};
static_assert(sizeof(SGAMEDATA) == 0x3060, "SGAMEDATA is 0x3060 bytes in 1.00");

// The table D2Game calls back into (GAME_SetServerCallbackFunctions). D2Game stores
// the pointer, not a copy, so the host keeps it in static storage. 1.13c and 1.14d
// read a slot 0x54 past this 0x40-byte 1.00 layout.
struct D2ServerCallbackFunctions {
    void * pfCloseGame;                 // 0x00
    void * pfLeaveGame;                 // 0x04
    void * pfGetDatabaseCharacter;      // 0x08
    void * pfSaveDatabaseCharacter;     // 0x0C
    void * pfServerLogMessage;          // 0x10 cdecl varargs
    void * pfEnterGame;                 // 0x14
    void * pfFindPlayerToken;           // 0x18
    void * pfSaveDatabaseGuild;         // 0x1C
    void * pfUnlockDatabaseCharacter;   // 0x20
    void * pfUnk0x24;                   // 0x24
    void * pfUpdateCharacterLadder;     // 0x28
    void * pfUpdateGameInformation;     // 0x2C
    void * pfHandlePacket;              // 0x30 null in 1.00
    void * pfSetGameData;               // 0x34 null in 1.00
    void * pfRelockDatabaseCharacter;   // 0x38 null in 1.00
    void * pfLoadComplete;              // 0x3C null in 1.00
};
static_assert(sizeof(D2ServerCallbackFunctions) == 0x40, "D2ServerCallbackFunctions is 0x40 bytes in 1.00");

// One character-server connection; the host keeps three (CharServ.cpp).
struct CharServerConn {
    void * hSocket;                 // 0x00 Fog socket; null when not connected
    char szIpAddr[36];              // 0x04 dotted address from realms.ini; "0..." = unused slot
    HANDLE hConnThread;             // 0x28 pending asynchronous connect (FOG_SOCKET_ConnectEx)
};
static_assert(sizeof(CharServerConn) == 0x2C, "CharServerConn is 0x2C bytes");

// MCP -> server: GTM create game (FromMCP.cpp, message 0x06).
struct MCP_GTM_CreateGameReq {
    uint8_t nCmd;                   // 0x00
    uint32_t dwMCPToken;            // 0x01 echoed in the reply
    uint32_t dwFlags;               // 0x05 passed to D2GAME_10047
    uint8_t nDifficulty;            // 0x09
    uint8_t nGameType;              // 0x0A
    uint8_t nMaxPlayers;            // 0x0B
    char szGameName[16];            // 0x0C name, then the password, packed
};
static_assert(sizeof(MCP_GTM_CreateGameReq) == 28, "MCP_GTM_CreateGameReq is 28 bytes");

// MCP -> server: GTM join game (FromMCP.cpp, message 0x07).
struct MCP_GTM_JoinGameReq {
    uint8_t nCmd;                   // 0x00
    uint32_t dwRequestId;           // 0x01 echoed in the reply
    uint32_t dwGameToken;           // 0x05
    uint32_t dwClientToken;         // 0x09
    uint32_t dwServerIp;            // 0x0D
    char szGameName[16];            // 0x11
};
static_assert(sizeof(MCP_GTM_JoinGameReq) == 33, "MCP_GTM_JoinGameReq is 33 bytes");

// MCP -> server: the hardened create/join forms (messages 0x0C and 0x0D).
struct MCP_GTM_CB_GameReq {
    uint8_t nCmd;                   // 0x00 0x0C create, 0x0D join
    uint32_t dwToken;               // 0x01
    char * pGameName;               // 0x05
    uint16_t wField09;              // 0x09
    uint16_t wField0B;              // 0x0B
    uint16_t wField0D;              // 0x0D
    uint8_t aPayload[1];            // 0x0F
};
static_assert(sizeof(MCP_GTM_CB_GameReq) == 16, "MCP_GTM_CB_GameReq is 16 bytes");

// One row of GAME_GetGameInformation's output (0x8C bytes).
struct D2GameInfoEntry {
    uint8_t unk0x00[8];             // 0x00
    uint8_t nFlags;                 // 0x08
    uint8_t unk0x09[91];            // 0x09
    char szDescription[16];         // 0x64
    char szName[16];                // 0x74
    uint8_t nField84;               // 0x84
    uint8_t nField85;               // 0x85
    uint8_t nField86;               // 0x86
    uint8_t pad0x87;                // 0x87
    uint32_t dwServerIp;            // 0x88 network byte order
};
static_assert(sizeof(D2GameInfoEntry) == 0x8C, "D2GameInfoEntry is 0x8C bytes");

// The display's per-game list node (Display.cpp).
struct GameInfoNode {
    uint16_t wGameId;               // 0x00
    uint8_t unk0x02[98];            // 0x02
    char szGameName[24];            // 0x64
    uint8_t unk0x7C[20];            // 0x7C
    int32_t bActive;                // 0x90 0 = marked for removal
    GameInfoNode * pNext;           // 0x94
};
static_assert(sizeof(GameInfoNode) == 0x98, "GameInfoNode is 0x98 bytes");

// A pending join, registered by the MCP and consumed when the client arrives (PlayerToken.cpp).
struct PlayerTokenNode {
    uint32_t dwClientToken;         // 0x00
    uint32_t dwServerIp;            // 0x04
    uint32_t dwTickStamp;           // 0x08 GetTickCount at registration; 120001 ms life
    char szGameName[16];            // 0x0C
    char szAccountName[16];         // 0x1C
    uint32_t dwGameToken;           // 0x2C
    PlayerTokenNode * pNext;        // 0x30
};
static_assert(sizeof(PlayerTokenNode) == 0x34, "PlayerTokenNode is 0x34 bytes");

// The 5 bytes McpServiceThread hands ToMCP_SendGameJoinInfo once a minute.
struct GameJoinInfoFields {
    uint16_t nGames;                // 0x00 GAME_GetGamesCount
    uint16_t wPad;                  // 0x02 always 0
    uint8_t nPlayers;               // 0x04 D2GAME_10051
};
static_assert(sizeof(GameJoinInfoFields) == 5, "GameJoinInfoFields is 5 bytes");

// The ladder thresholds the MCP sends in message 0x0F (0x88 bytes, copied as is):
// a character is reported in ToMCP_SendCharEnter only when its experience reaches the
// total or its class's entry. Expansion first, then classic.
struct LadderThresholds {
    uint64_t nExpansionTotal;       // 0x00
    uint64_t aExpansionClass[8];    // 0x08
    uint64_t nClassicTotal;         // 0x48
    uint64_t aClassicClass[7];      // 0x50
};
static_assert(sizeof(LadderThresholds) == 0x88, "LadderThresholds is 0x88 bytes");

// The launcher's configuration block WinMain receives in its third argument (the
// MCP's OnServerStart passes it through). Only the bytes the switch handlers read
// are known.
struct D2ServerLaunchConfig {
    uint8_t unk0x000[0x46];         // 0x000
    char cLadder;                   // 0x046 '0' = no ladder
    uint8_t unk0x047[0x17];         // 0x047
    char szServerName[0x190];       // 0x05E '0...' = no character servers
    char cLoadD2CommonTxt;          // 0x1EE nonzero: run D2COMMON_10980
};

// GAME_GetStatistics' output. The first four dwords are shown in the status window
// and logged in the order of the performance log's header; the rest is not read here.
struct D2GameServerStats {
    int32_t nGames;                 // 0x00
    int32_t nActiveMonsters;        // 0x04
    int32_t nInactiveMonsters;      // 0x08
    int32_t nPlayers;               // 0x0C
    uint8_t unk0x10[0x1EC];         // 0x10
};
static_assert(sizeof(D2GameServerStats) == 0x1FC, "D2GameServerStats is 0x1FC bytes");

// One node of the list GAME_GetUnitsDescriptions returns; the first node is the
// caller's buffer, its +0x8C holds the head of the allocated list.
struct D2UnitDescription {
    int32_t nField00;               // 0x00 set to -1 by the caller
    int32_t nActive;                // 0x04
    int32_t nInactive;              // 0x08
    char szName[0x80];              // 0x0C
    D2UnitDescription * pNext;      // 0x8C
};
static_assert(sizeof(D2UnitDescription) == 0x90, "D2UnitDescription is 0x90 bytes");

// One row of D2GAME_10017's player list (0x34 bytes).
struct D2PlayerListEntry {
    uint32_t dwField00;             // 0x00
    int32_t nClass;                 // 0x04
    int32_t nUnitId;                // 0x08 listbox item data
    uint8_t unk0x0C[0x18];          // 0x0C
    char szName[16];                // 0x24 empty: shown as "<class> <id>"
};
static_assert(sizeof(D2PlayerListEntry) == 0x34, "D2PlayerListEntry is 0x34 bytes");

// The game-data table: Storm's TSHashTable<SGAMEDATA,HASHKEY_NONE>, 0x68 bytes. Its
// members are reached as dwords by index, as the template's functions do (GameTable.cpp).
// It is a global C++ object (ToMCP.cpp): constructed before WinMain and destroyed at
// exit by the code the compiler generates for it.
struct GameDataTable {
    uint32_t a[0x1A];               // 0x00 vtable, lists, chunk heap; 0x50 CRITICAL_SECTION
    GameDataTable();
    ~GameDataTable();
};
static_assert(sizeof(GameDataTable) == 0x68, "GameDataTable is 0x68 bytes");

// One entry of realms.ini's accept list.
struct AcceptIpEntry {
    uint32_t dwIpAddr;              // 0x00 network order
    int32_t nPrefixLen;             // 0x04 0..32
};

// Everything ParseIniFile fills, cleared as one block when a realm section starts.
struct RealmIniInfo {
    char szName[16];                // 0x0000
    char szDescription[0x80];       // 0x0010
    char szAdminPass[16];           // 0x0090
    char szPublicPass[16];          // 0x00A0
    char szBnetIp[0x400];           // 0x00B0
    char szMcpIp[0x18];             // 0x04B0
    uint32_t dwMcpIpAddr;           // 0x04C8
    AcceptIpEntry aAcceptIps[256];  // 0x04CC
    int32_t nAcceptIps;             // 0x0CCC
    uint32_t aCharServerIps[3];     // 0x0CD0
    int32_t nCharServers;           // 0x0CDC
    char aAnnouncements[20][0x100]; // 0x0CE0
    int32_t nAnnouncements;         // 0x20E0
    int32_t nMaxMultiClient;        // 0x20E4
};
static_assert(sizeof(RealmIniInfo) == 0x20E8, "RealmIniInfo is 0x20E8 bytes (the 0x83A-dword clear)");

#pragma pack(pop)

} // namespace D2Server
