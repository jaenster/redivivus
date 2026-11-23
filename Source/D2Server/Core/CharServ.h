// D2Server CORE\CharServ.cpp: the MPQs, the MCP link and the character servers.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.

#pragma once

#include "Source/D2Server/D2Server.h"

namespace D2Hell::Src::Archive { struct D2ArchiveHandleStrc; }

namespace D2Server::Core::CharServ {

extern D2Hell::Src::Archive::D2ArchiveHandleStrc * g_hD2DataMpq;   // 1.00 D2Server: 10026b90
extern D2Hell::Src::Archive::D2ArchiveHandleStrc * g_hD2CharMpq;   // 1.00 D2Server: 10026b94
extern void * g_McpConnection;                                     // 1.00 D2Server: 10026ba0
extern CharServerConn g_csConnTable[3];                            // 1.00 D2Server: 10026bc0

int __stdcall LoadMPQArchives();                                    // 1.00 D2Server: 10001770
int __stdcall Stub_ReturnZero();                                    // 1.00 D2Server: 10001810
void __stdcall UnloadMPQArchives();                                 // 1.00 D2Server: 10001830
void __cdecl FreeMemory(void * pMem);                               // 1.00 D2Server: 10001880
int __stdcall ConnectToCharServer();                                // 1.00 D2Server: 100018e0
int __stdcall MaintainCharServerConnection();                       // 1.00 D2Server: 100019a0
void __stdcall DisconnectFromCharServer();                          // 1.00 D2Server: 10001ae0
void __stdcall OnCharServerConnected();                             // 1.00 D2Server: 10001b30
int __stdcall LoadBnClientDll();                                    // 1.00 D2Server: 10001b60
int __fastcall CS_OnReply_GameLoad(uint8_t * pBuf, int nLength);    // 1.00 D2Server: 10001ce0
int __fastcall CS_OnReply_Stub_Cmd5(uint8_t * pBuf, int nLength);   // 1.00 D2Server: 10001d20
int __fastcall CS_OnReply_Stub_Cmd8(uint8_t * pBuf, int nLength);   // 1.00 D2Server: 10001d40
bool __fastcall SendToCharServer(void * pData, int nLength);        // 1.00 D2Server: 10001dd0
bool __fastcall CB_SendDataToCharServer(char * szAccountName, char * szCharName, uint16_t * pbData, uint32_t nLength, uint32_t dwExtra); // 1.00 D2Server: 10001e60
bool __fastcall CB_AssertSrvDataRecordSize(char * szAccountName, char * szCharName, uint16_t * pbData, uint32_t nLength, uint32_t dwExtra); // 1.00 D2Server: 10002030
bool __fastcall CB_ValidateGameName_WithToken(char * pszGameName, uint32_t dwToken); // 1.00 D2Server: 10002210
bool __fastcall CB_ValidateGameName_WithFlag(char * pszGameName, uint32_t dwFlag); // 1.00 D2Server: 10002300
bool __fastcall CB_SendGameNameWithWordAndDword(char * pszGameName, uint16_t wParam, uint32_t dwExtra); // 1.00 D2Server: 100023e0
bool __fastcall CB_SendGameNameOnly(char * pszGameName);            // 1.00 D2Server: 100024d0
DWORD __stdcall ConnectToCharServers(LPVOID pUnused);               // 1.00 D2Server: 10002640
BOOL __fastcall WaitForCharServerThread(HWND hDlg);                 // 1.00 D2Server: 10002850
BOOL __stdcall TickCharServerConnections();                         // 1.00 D2Server: 10002980
void __stdcall DestroyAllCharServerConnections();                   // 1.00 D2Server: 10002be0
BOOL __stdcall PollCharServerPackets();                             // 1.00 D2Server: 10002c20

} // namespace D2Server::Core::CharServ
