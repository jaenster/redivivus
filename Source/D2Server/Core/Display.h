// D2Server CORE\Display.cpp: the status window's game, player and unit lists.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.

#pragma once

#include "Source/D2Server/D2Server.h"

namespace D2Server::Core::Display {

extern uint32_t g_nBusyPercent;                                                 // 1.00 D2Server: 10026c9c
extern uint64_t g_nPerfIdleTicks;                                               // 1.00 D2Server: 10026c78

void __fastcall InitPerfTimerDivisor(uint32_t nDivisor);                        // 1.00 D2Server: 100039d0
GameInfoNode * __stdcall GetSelectedPlayerByAccountName();                      // 1.00 D2Server: 10003a60
void __stdcall InitPerfTimer();                                                 // 1.00 D2Server: 10003ba0
void __stdcall PerfTimerMark();                                                 // 1.00 D2Server: 10003bf0
BOOL __stdcall PerfTimerGetElapsed();                                           // 1.00 D2Server: 10003c10
void __fastcall GameListAction_PopulatePlayerList(GameInfoNode * pGame, int nUnused); // 1.00 D2Server: 10003ce0
void __fastcall StrToLower(char * str);                                         // 1.00 D2Server: 10004030
void __fastcall GameListAction_AddGameEntry(GameInfoNode * pGame, int nUnitType); // 1.00 D2Server: 10004070
void __stdcall RefreshDisplay();                                                // 1.00 D2Server: 100041d0
GameInfoNode * __fastcall FindGameNode(uint32_t gameId);                        // 1.00 D2Server: 100046f0
void __fastcall AddGame(uint32_t gameId);                                       // 1.00 D2Server: 10004720
void __stdcall RemoveStaleGames();                                              // 1.00 D2Server: 100047f0
void __stdcall UpdateBusyBar();                                                 // 1.00 D2Server: 10004980
void __stdcall RefreshGameDetail();                                             // 1.00 D2Server: 10004a20
void __stdcall PurgeAllGames();                                                 // 1.00 D2Server: 100055b0
LRESULT __stdcall ConsoleInputProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // 1.00 D2Server: 100095f0 (defined in WinMain.cpp)

} // namespace D2Server::Core::Display
