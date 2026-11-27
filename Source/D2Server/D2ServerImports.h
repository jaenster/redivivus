// D2Server's imports from the 1.00 DLLs, declared by the names the D2Server Ghidra
// program gives them, with the ordinal each one is imported by.
//
// Reconstructed from the 1.00 D2Server.dll (2000-05-26). Not part of the 1.14d byte
// match, and not linked into Game.exe yet: nothing here is bound. Where a 1.14d tree
// function has the same meaning, its name is noted for the linking step; where the
// D2Server program's name does not match what the call sites do, the behaviour the
// call sites show is noted too (those names are listed in NOTES for a Ghidra fix).
//
// Fog, D2Game and D2Common ordinals moved between 1.00 and the later versions; Storm
// and D2Net ordinals did not. Storm, D2Net, D2Common, D2Lang and most of Fog are
// __stdcall; the Fog allocator pair and a few others are __fastcall, as noted.

#pragma once

#include <cstdint>
#include <cstddef>
#include "Source/D2Headers/d2_platform.h"
#include "Source/D2Server/D2Server.h"

namespace D2Server::Imports {

// ---- Storm.dll (ordinals stable across versions)
BOOL __stdcall SFileCloseArchive(HANDLE hArchive);                                              // Storm 252
BOOL __stdcall SFileOpenArchive(const char * szName, DWORD dwPriority, DWORD dwFlags, HANDLE * phArchive); // Storm 266
void * __stdcall SMemAlloc(size_t nSize, const char * szFile, int nLine, DWORD dwFlags);         // Storm 401
char * __stdcall SStrNCat(char * szDest, const char * szSrc, int nMax);                          // Storm 503
BOOL __stdcall SFileGetFileName(HANDLE hFile, char * szBuffer, DWORD dwBufferSize);              // Storm 276
int __stdcall SStrCopy(char * szDest, const char * szSrc, int nMax);                              // Storm 501
int __cdecl SStrPrintf(char * szDest, size_t nMax, const char * szFormat, ...);                  // Storm 578
char * __stdcall SStrChr(const char * szString, char cChar);                                      // Storm 571
int __stdcall SStrCmp(const char * szString1, const char * szString2, size_t nMax);              // Storm 508
void * __stdcall SMemReAlloc(void * pMemory, size_t nSize, const char * szFile, int nLine, DWORD dwFlags); // Storm 405
BOOL __stdcall SMemFree(void * pMemory, const char * szFile, int nLine, DWORD dwFlags);          // Storm 403

// ---- D2Net.dll (ordinals stable across versions)
typedef void (__stdcall * D2NetConnectCallback)();
void __stdcall D2NET_10018(D2NetConnectCallback pfnOnConnected);  // D2Net 10018: registers (null: clears) the MCP-connected callback
int __stdcall D2NET_10022(int nWaitMs);                           // D2Net 10022: waits up to nWaitMs for client traffic; nonzero when there is some
void __stdcall D2NET_SERVER_Release();                            // D2Net 10004
void __stdcall D2NET_SERVER_Initialize(int nUnused1, int nUnused2); // D2Net 10003
void __stdcall D2NET_SERVER_SetMaxClientsPerGame(int nMaxClients); // D2Net 10026
void __stdcall D2NET_SERVER_SetHackListEnabled(BOOL bEnabled);    // D2Net 10023

// ---- D2Common.dll (ordinals shifted since 1.00)
void __stdcall DATATBLS_LoadAllTxts(int nLanguage, BOOL bLoadAll, int nFlags); // D2Common 10554
void __stdcall DATATBLS_UnloadAllBins();                          // D2Common 10553
void __stdcall D2COMMON_10980();                                  // D2Common 10980: run by the +0x1ee launch switch
void __stdcall D2COMMON_10984();                                  // D2Common 10984: the console's "profile" (restarts the profiler)
void __stdcall D2COMMON_10983();                                  // D2Common 10983: module shutdown, last in WinMain

// ---- D2Game.dll (ordinals shifted since 1.00; the host-facing ones are stable 1.00 -> 1.10f)
void __stdcall GAME_InitGameDataTable(void * ptGameDataTbl, void * phGameList); // D2Game 10002
void __stdcall GAME_SetServerCallbackFunctions(void * pCallbacks);  // D2Game 10023: stores the pointer, not a copy
void __stdcall D2GAME_10046();                                      // D2Game 10046: module init (GAME_InitServerModule); must run first
void __stdcall GAME_ReceiveDatabaseCharacter(uint32_t dwToken, uint8_t * pData, uint16_t wField09, uint16_t wField0B, uint32_t dwField0D); // D2Game 10007
void __stdcall GAME_GetStatistics(D2GameServerStats * pStats);      // D2Game 10020
void __stdcall GAME_GetUnitsDescriptions(uint16_t wGameId, D2UnitDescription * pOut, int nUnitType); // D2Game 10019
void __stdcall TASK_FreeAllQueueSlots();                            // D2Game 10040
void __stdcall D2GAME_10050();                                      // D2Game 10050: module shutdown
void * __stdcall TASK_Create();                                     // D2Game 10041
int __fastcall D2GAME_10043(void * pTask, int * pbHasGame);         // D2Game 10043: next game slot for this worker; returns the wait in ms
void __stdcall GAME_ProcessNetworkMessages();                       // D2Game 10003
void __fastcall TASK_ProcessGame(void * pTask);                     // D2Game 10045
uint16_t __stdcall D2GAME_10051();                                  // D2Game 10051: player count reported in the MCP heartbeat
void __stdcall GAME_CloseAllGames();                                // D2Game 10006
void __stdcall TASK_InitializeClock();                              // D2Game 10039
void __stdcall D2GAME_10021(uint32_t dwUnused1, uint32_t dwUnused2, const char * szMessage); // D2Game 10021: message to every game
void __stdcall D2GAME_10022(uint16_t wGameId, const char * szMessage); // D2Game 10022: message to one game
int __stdcall D2GAME_10017(uint16_t wGameId, D2PlayerListEntry * pOut, int nMax); // D2Game 10017: fills the player list of a game
int __stdcall D2GAME_10016(uint16_t wGameId);                       // D2Game 10016: player count of a game
void __stdcall D2GAME_10048(uint32_t dwToken, uint32_t * pGameNameRef, uint16_t wField0B, uint16_t wField09, uint16_t wField0D, uint8_t * pPayload); // D2Game 10048: GTM 0x0C create
void __stdcall D2GAME_10049(uint32_t dwToken, uint32_t * pGameNameRef, uint16_t wField0B, uint16_t wField09, uint16_t wField0D, uint8_t * pPayload); // D2Game 10049: GTM 0x0D join
int __stdcall D2GAME_10018(uint16_t wGameId, int nOne, void * pItemText, void * pItemData, void * pSelection); // D2Game 10018: item list of a selected unit
int __stdcall GAME_GetGameServerGameIds(uint16_t * pGameIds, int nMax); // D2Game 10015
int __stdcall GAME_GetGamesCount();                                 // D2Game 10012
int __stdcall GAME_GetGameInformation(uint16_t wGameId, void * pInfo); // D2Game 10014
int __stdcall D2GAME_10047(char * szGameName, char * szPassword, int nZero, uint32_t dwFlags, uint8_t nDifficulty, uint8_t nGameType, uint8_t nMaxPlayers, uint16_t * pGameId); // D2Game 10047: GAME_CreateNewEmptyGame

// ---- D2Lang.dll (ordinals stable)
BOOL __fastcall D2LANG_10000(int nLanguage);                        // D2Lang 10000: string tables in
void __stdcall D2LANG_10001();                                      // D2Lang 10001: string tables out

// ---- Fog.dll (ordinals shifted since 1.00)
BOOL __stdcall FOG_10058(void * hSocket);                           // Fog 10058: is the socket still connected
void * __stdcall FOG_FOpenFile(const char * szFile, DWORD dwFlags); // Fog 10075 (used by D2Hell Archive.cpp)
void __stdcall FOG_FCloseFile(void * hFile);                        // Fog 10076 (used by D2Hell Archive.cpp)
void __stdcall FOG_10038();                                         // Fog 10038: dump the performance accumulators
void __stdcall FOG_GetSavePath(int nPool, int nMode);               // Fog 10113: the call sites use it as a memory dump (pool, 0 pool/1 line/2 clear/3 num)
BOOL __stdcall FOG_FReadFile(void * hFile, void * pBuffer, DWORD dwBytes); // Fog 10077 (used by D2Hell Archive.cpp)
void __stdcall FOG_10037();                                         // Fog 10037: clear the performance accumulators
void __stdcall FOG_SOCKET_SendNoLock(void * hSocket, const void * pData, int nLength); // Fog 10050
void __stdcall FOG_10052(void * hSocket);                           // Fog 10052: run after a connect, before the first send
typedef int (__fastcall * FogPacketHandler)(uint8_t * pPacket, int nLength);
void __stdcall FOG_SOCKET_SetBlockingMode(void * hSocket, FogPacketHandler pfnHandler, int nZero); // Fog 10056: the second argument is the packet handler
void * __stdcall FOG_SOCKET_Connect(const char * szHost, int nPort, const char * szLabel); // Fog 10047
void __stdcall FOG_SOCKET_Destroy(void * hSocket);                  // Fog 10048
void __fastcall FOG_Alloc(void * pMemory, const char * szFile, int nLine, int nZero); // Fog 10034: the call sites use it as the free (Fog::Memory::FreeClientMemory in 1.14d)
const char * __stdcall FOG_GetHostIPAddress();                      // Fog 10013
int __stdcall FOG_SOCKET_Send(void * hSocket, const void * pData, int nLength); // Fog 10049
void __cdecl FOG_DisplayAssert(const char * szExpression, const char * szFile, int nLine); // Fog 10021
uint32_t __stdcall FOG_10036();                                     // Fog 10036: stamp prefixed to a cmd-2 save record
HANDLE __stdcall FOG_SOCKET_ConnectEx(void * hSocket);              // Fog 10053: finishes a connect on a thread; returns the thread
int __stdcall FOG_SOCKET_Receive(void * hSocket, void * pBuffer, int nSize); // Fog 10051
void __cdecl FOG_DisplayHalt(const char * szMessage, const char * szFile, int nLine); // Fog 10024
DWORD __stdcall FOG_FSetFilePointer(void * hFile, LONG nDistance, LONG * pnHigh, DWORD dwMethod); // Fog 10079 (used by D2Hell Archive.cpp)
DWORD __stdcall FOG_FGetFileSize(void * hFile, DWORD * pnHigh);     // Fog 10078 (used by D2Hell Archive.cpp)
void __stdcall FOG_10154(uint32_t * p1, uint32_t * p2, uint32_t * p3, uint32_t * p4); // Fog 10154: four counters for the MCP heartbeat
uint64_t __stdcall FOG_10153();                                     // Fog 10153: a 64-bit time, shifted right 10 for the heartbeat
void * __fastcall FOG_CrashDeadlockDetected(size_t nSize, const char * szFile, int nLine, int nZero); // Fog 10033: the call sites use it as the allocator (Fog::Memory::AllocClientMemory in 1.14d)
void __cdecl FOG_10022(const char * szMessage, const char * szFile, int nLine); // Fog 10022: fatal message, followed by _exit
const char * __stdcall FOG_SOCKET_GetPeerName();                    // Fog 10012: the call site reads it as this machine's address
void __cdecl FOG_DisplayError(const char * szFormat, ...);          // Fog 10026
int __stdcall FOG_SOCKET_Select(void * hSocket, int nTimeout);      // Fog 10055: 0 when data is waiting
struct FogSystemInfo;
FogSystemInfo * __stdcall FOG_GetSystemInfo();                      // Fog 10020: +0x28 = processor count
void __fastcall FOG_InitErrorMgr(const char * szName);              // Fog 10019
void __fastcall FOG_10139(int nOne);                                // Fog 10139: first call of WinMain

// The processor count FOG_GetSystemInfo reports; the rest of the block is not read.
struct FogSystemInfo {
    uint8_t unk0x00[0x28];          // 0x00
    int32_t nProcessors;            // 0x28
};

} // namespace D2Server::Imports

// The 1.00 form of D2Hell's Archive.cpp. The tree already carries D2Hell's archive code
// in its 1.14d form (Source/D2Hell/D2HellRoot.cpp: D2FILE_GetFileSize, D2FILE_SeekFile,
// D2FILE_ReadFile, D2FILE_AllocAndLoadD2File, ARCHIVE_LoadMPQ, FindMpqFromDiskOrCd,
// FindMPQOnCd, ARCHIVE_FreeMpqFile), whose signatures changed after 1.00 (a pool
// argument, a different loader). D2Server is not given a second copy: these are
// declarations only, bound to the 1.14d functions in the linking step.
namespace D2Hell::Src::Archive {
struct D2ArchiveHandleStrc;
D2ArchiveHandleStrc * __fastcall LoadMPQFile(char * szModuleName, char * szFileName, char * szLabel, int nPriority, void * pfShowMessage); // 1.00 D2Server: 1000bdfe (1.14d: D2Hell::ARCHIVE_LoadMPQ)
void __fastcall UnloadMPQFile(D2ArchiveHandleStrc * pMPQHandle); // 1.00 D2Server: 1000bf9d (1.14d: D2Hell::ARCHIVE_FreeMpqFile)
}
