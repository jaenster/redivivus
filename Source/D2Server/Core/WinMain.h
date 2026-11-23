// D2Server CORE\WinMain.cpp: start-up, the D2Game callbacks, the threads, the window.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.

#pragma once

#include "Source/D2Server/D2Server.h"

namespace D2Server::Core::WinMain {

extern HWND g_hMainWnd;                                                         // 1.00 D2Server: 100294e8
extern char g_szServerName[0x108];                                              // 1.00 D2Server: 10029580
extern volatile LONG g_nActiveWorkerThreads;                                    // 1.00 D2Server: 100294f4
extern LadderThresholds g_tLadderThresholds;                                    // 1.00 D2Server: 100294f8
extern int32_t g_bShutdownRequested;                                            // 1.00 D2Server: 100296fc
extern uint32_t g_dServerStartTime;                                             // 1.00 D2Server: 100294f0

void __stdcall RunModuleInitializers();                                         // 1.00 D2Server: 100091f0
void __cdecl PostStatusMessage(int bIsError, char * pFmt, ...);                 // 1.00 D2Server: 10009310
bool __fastcall CB_SendGameNameOnly(char * pszGameName);                        // 1.00 D2Server: 100093d0
void __fastcall CB_JoinGame(uint32_t dwToken, char * szText, uint16_t wField1, uint16_t wField2, uint16_t wField3); // 1.00 D2Server: 100093f0
void __fastcall CB_CreateGame(uint32_t dwToken, uint16_t wClass, uint16_t wLevel, uint32_t dwExperienceLo, uint32_t dwExperienceHi, uint32_t dwFlags, char * szCharName, char * szAccountName); // 1.00 D2Server: 10009420
bool __fastcall CB_ValidateGameName_WithToken(char * pszGameName, uint32_t dwToken); // 1.00 D2Server: 10009470
void __fastcall CB_AssertSrvDataRecordSize(char * szAccountName, char * szCharName, uint16_t * pRecord, uint32_t nExpectedSize, uint32_t dwExtra); // 1.00 D2Server: 10009490
void __stdcall ToMCP_SendLeaveGame();                                           // 1.00 D2Server: 10009500
void __fastcall CB_SendGameData(char * pGameName, uint8_t * pData, int nLength); // 1.00 D2Server: 10009520
void __fastcall CB_SendPacket(char * pGameName, uint8_t nDifficulty, char * pDescription, uint32_t dwGameFlags); // 1.00 D2Server: 10009540
void __fastcall CB_HandlePacket(char * pCharName, uint16_t wClass, uint32_t dwLevel, uint32_t dwExperienceLo, uint32_t dwExperienceHi, uint32_t dwFlags); // 1.00 D2Server: 10009570
void __fastcall CB_DisconnectClient(uint32_t dwToken, char * pCharName, uint16_t wClass); // 1.00 D2Server: 100095a0
uint32_t __stdcall GetWorkerStackSize();                                        // 1.00 D2Server: 10009710
uint32_t __stdcall GetWorkerStackSize2();                                       // 1.00 D2Server: 10009730
int64_t __stdcall GetCurrentTimeAsTick();                                       // 1.00 D2Server: 10009750
INT_PTR __stdcall ServerWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // 1.00 D2Server: 10009790
DWORD __stdcall McpServiceThread(LPVOID pUnused);                               // 1.00 D2Server: 10009b80
void __stdcall OnShutdownRemotely();                                            // 1.00 D2Server: 10009d10
int __fastcall GameWorkerTick(void * pTask, uint32_t * pnCounter);              // 1.00 D2Server: 10009d60
DWORD __stdcall GameWorkerThread(LPVOID pUnused);                               // 1.00 D2Server: 10009de0
void __stdcall Noop();                                                          // 1.00 D2Server: 10009e80
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, D2ServerLaunchConfig * pConfig, int nCmdShow); // 1.00 D2Server: 10009ea0

} // namespace D2Server::Core::WinMain

namespace D2Server::Core {
void __fastcall StaticInit_Nop_B(D2ServerLaunchConfig * pConfig);               // 1.00 D2Server: 10009250
void __fastcall StaticInit_Nop_A(D2ServerLaunchConfig * pConfig);               // 1.00 D2Server: 10009230
void __fastcall StaticInit_LoadD2CommonTxt(D2ServerLaunchConfig * pConfig);     // 1.00 D2Server: 100092e0
void __fastcall StaticInit_SetLadderFlag(D2ServerLaunchConfig * pConfig);       // 1.00 D2Server: 100092b0
void __fastcall StaticInit_SetServerName(D2ServerLaunchConfig * pConfig);       // 1.00 D2Server: 10009270
}
