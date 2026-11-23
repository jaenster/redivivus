// D2Server CORE\CharServ.cpp: the MPQs, the MCP link and the character servers.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.

#include "Source/D2Server/Core/CharServ.h"
#include "Source/D2Server/Core/FromMCP.h"
#include "Source/D2Server/Core/Inifile.h"
#include "Source/D2Server/Core/ToMCP.h"
#include "Source/D2Server/Core/WinMain.h"
#include "Source/D2Server/D2ServerImports.h"

namespace D2Server::Core::CharServ {

using namespace D2Server::Imports;

static const char s_szCharServFile[] = "C:\\D2\\Source\\D2Server\\CORE\\CharServ.cpp";

D2Hell::Src::Archive::D2ArchiveHandleStrc * g_hD2DataMpq = nullptr;   // 1.00 D2Server: 10026b90
D2Hell::Src::Archive::D2ArchiveHandleStrc * g_hD2CharMpq = nullptr;   // 1.00 D2Server: 10026b94
void * g_McpConnection = nullptr;                                     // 1.00 D2Server: 10026ba0
static uint32_t g_csLastPingTick = 0;                                 // 1.00 D2Server: 10026ba4
static void * g_pBnClientInterface = nullptr;                         // 1.00 D2Server: 10026ba8
static uint32_t g_csConnRoundRobinIdx = 0;                            // 1.00 D2Server: 10026bb8
CharServerConn g_csConnTable[3] = {};                                 // 1.00 D2Server: 10026bc0
static uint32_t g_csLastTickTime = 0;                                 // 1.00 D2Server: 10026c44 (seconds, time())
static int32_t g_csIsConnected = 1;                                   // 1.00 D2Server: 10021d7c

// The character server's replies, by command byte (PollCharServerPackets).
typedef int (__fastcall * CharServReplyHandler)(uint8_t * pBuf, int nLength);
static const CharServReplyHandler g_csInboundHandlers[9] = {         // 1.00 D2Server: 10021e7c
    nullptr, nullptr, nullptr, CS_OnReply_GameLoad, nullptr,
    CS_OnReply_Stub_Cmd5, nullptr, nullptr, CS_OnReply_Stub_Cmd8,
};

// 1.00 D2Server: 10001770
int __stdcall LoadMPQArchives() {
    char szModulePath[MAX_PATH];
    GetModuleFileNameA(nullptr, szModulePath, MAX_PATH);
    g_hD2DataMpq = D2Hell::Src::Archive::LoadMPQFile(szModulePath, (char *)"d2data.mpq", (char *)"D2DATA", 0, nullptr);
    g_hD2CharMpq = D2Hell::Src::Archive::LoadMPQFile(szModulePath, (char *)"d2char.mpq", (char *)"D2CHAR", 0, nullptr);
    if (g_hD2DataMpq && g_hD2CharMpq) {
        return 1;
    }
    return 0;
}

// 1.00 D2Server: 10001810
int __stdcall Stub_ReturnZero() {
    return 0;
}

// 1.00 D2Server: 10001830
void __stdcall UnloadMPQArchives() {
    if (g_hD2DataMpq) {
        D2Hell::Src::Archive::UnloadMPQFile(g_hD2DataMpq);
    }
    g_hD2DataMpq = nullptr;
    if (g_hD2CharMpq) {
        D2Hell::Src::Archive::UnloadMPQFile(g_hD2CharMpq);
    }
    g_hD2CharMpq = nullptr;
}

// 1.00 D2Server: 10001880
void __cdecl FreeMemory(void * pMem) {
    if (pMem) {
        SMemFree(pMem, "delete", -1, 0);
    }
}

// Opens the MCP link: one handshake byte (2), a ping, then the connected callback.
// 1.00 D2Server: 100018e0
int __stdcall ConnectToCharServer() {
    uint8_t nHello;
    SStrCopy(WinMain::g_szServerName, Inifile::GetMcpIp(), 0x7fffffff);
    g_McpConnection = FOG_SOCKET_Connect(WinMain::g_szServerName, 6112, "GameServer->MCP");
    FOG_SOCKET_SetBlockingMode(g_McpConnection, FromMCP::DispatchClientPacketAlt, 0);
    FOG_10052(g_McpConnection);
    nHello = 2;
    FOG_SOCKET_SendNoLock(g_McpConnection, &nHello, 1);
    ToMCP::ToMCP_SendPing();
    D2NET_10018(OnCharServerConnected);
    WinMain::PostStatusMessage(1, (char *)"[SERVER]  Connected to MCP at %s", WinMain::g_szServerName);
    return 1;
}

// Once a minute: is the MCP link still up; if not, open it again.
// 1.00 D2Server: 100019a0
int __stdcall MaintainCharServerConnection() {
    uint32_t dwNow = GetTickCount();
    if (g_csLastPingTick == 0) {
        g_csLastPingTick = dwNow;
    }
    if (dwNow - g_csLastPingTick < 60000) {
        return 1;
    }
    g_csLastPingTick = dwNow;
    if (FOG_10058(g_McpConnection)) {
        g_csIsConnected = 1;
        return 1;
    }
    if (g_csIsConnected) {
        g_csIsConnected = 0;
    }
    FOG_SOCKET_Destroy(g_McpConnection);
    ConnectToCharServer();
    return 1;
}

// 1.00 D2Server: 10001ae0
void __stdcall DisconnectFromCharServer() {
    if (g_McpConnection) {
        D2NET_10018(nullptr);
        FOG_SOCKET_Destroy(g_McpConnection);
    }
}

// 1.00 D2Server: 10001b30
void __stdcall OnCharServerConnected() {
    WinMain::PostStatusMessage(1, (char *)"[SERVER]  Hack listing IP %s", FOG_GetHostIPAddress());
    ToMCP::ToMCP_SendEnterGame();
}

// Bnclient.dll is loaded by name and asked for its interface; it is not imported.
// 1.00 D2Server: 10001b60
int __stdcall LoadBnClientDll() {
    char szMessage[100];
    if (g_pBnClientInterface) {
        return 0;
    }
    HMODULE hModule = LoadLibraryA("Bnclient.dll");
    if (!hModule) {
        sprintf(szMessage, "Cannot load Bnclient.dll: Error %d", GetLastError());
        MessageBoxA(nullptr, szMessage, "D2Multi", MB_ICONHAND);
        return 0;
    }
    typedef void * (__stdcall * QueryInterfaceFn)();
    QueryInterfaceFn pfnQueryInterface = (QueryInterfaceFn)GetProcAddress(hModule, "QueryInterface");
    if (!pfnQueryInterface) {
        sprintf(szMessage, "Unable to QueryInterface for Bnclient.dll: Error %d", GetLastError());
        MessageBoxA(nullptr, szMessage, "D2Multi", MB_ICONHAND);
        return 0;
    }
    g_pBnClientInterface = pfnQueryInterface();
    return 1;
}

// Command 3: a character the character server sends back for a game.
// 1.00 D2Server: 10001ce0
int __fastcall CS_OnReply_GameLoad(uint8_t * pBuf, int nLength) {
    GAME_ReceiveDatabaseCharacter(*(uint32_t *)(pBuf + 0x05), pBuf + 0x11, *(uint16_t *)(pBuf + 0x09),
                                  *(uint16_t *)(pBuf + 0x0B), *(uint32_t *)(pBuf + 0x0D));
    return 1;
}

// 1.00 D2Server: 10001d20
int __fastcall CS_OnReply_Stub_Cmd5(uint8_t * pBuf, int nLength) {
    return 1;
}

// 1.00 D2Server: 10001d40
int __fastcall CS_OnReply_Stub_Cmd8(uint8_t * pBuf, int nLength) {
    return 1;
}

// Sends to the current character server, moving on to the next one when a send
// fails; true when a server took it.
// 1.00 D2Server: 10001dd0
bool __fastcall SendToCharServer(void * pData, int nLength) {
    uint32_t nIndex = g_csConnRoundRobinIdx;
    int nTried = 0;
    while (nTried < Inifile::GetCharServerCount()) {
        if (g_csConnTable[nIndex].hSocket && FOG_SOCKET_Send(g_csConnTable[nIndex].hSocket, pData, nLength)) {
            break;
        }
        nIndex = (nIndex + 1) % (uint32_t)Inifile::GetCharServerCount();
        ++nTried;
    }
    g_csConnRoundRobinIdx = nIndex;
    return nTried < Inifile::GetCharServerCount();
}

// The rotate-and-add checksum of a save record.
static inline int32_t CharServChecksum(const uint8_t * pData, uint32_t nLength) {
    int32_t nChecksum = 0;
    for (int i = 0; i < (int)nLength; ++i) {
        nChecksum = pData[i] + (nChecksum < 0) + nChecksum * 2;
    }
    return nChecksum;
}

// Command 1: save a character. [0] 1, [1] dwExtra, [5] checksum, [9] account\0 char\0 record.
// 1.00 D2Server: 10001e60 (the copy-and-send tail Ghidra splits off at 10001f20 is part of it)
bool __fastcall CB_SendDataToCharServer(char * szAccountName, char * szCharName, uint16_t * pbData, uint32_t nLength, uint32_t dwExtra) {
    uint8_t aPacket[0x1020];
    aPacket[0] = 1;
    *(uint32_t *)(aPacket + 1) = dwExtra;
    *(int32_t *)(aPacket + 5) = CharServChecksum((const uint8_t *)pbData, nLength);
    size_t nAccount = strlen(szAccountName) + 1;
    memcpy(aPacket + 9, szAccountName, nAccount);
    size_t nChar = strlen(szCharName) + 1;
    memcpy(aPacket + 9 + nAccount, szCharName, nChar);
    if (*pbData != nLength) {
        FOG_DisplayAssert("*(WORD *)pbData == nLength", s_szCharServFile, 0xD0);
        _exit(-1);
    }
    memcpy(aPacket + 9 + nAccount + nChar, pbData, nLength);
    return SendToCharServer(aPacket, (int)(nAccount + nChar + 9 + nLength));
}

// Command 2: as command 1, with FOG_10036's stamp in front of the extra dword.
// 1.00 D2Server: 10002030
bool __fastcall CB_AssertSrvDataRecordSize(char * szAccountName, char * szCharName, uint16_t * pbData, uint32_t nLength, uint32_t dwExtra) {
    uint8_t aPacket[0x1028];
    aPacket[0] = 2;
    *(uint32_t *)(aPacket + 1) = FOG_10036();
    *(uint32_t *)(aPacket + 5) = dwExtra;
    *(int32_t *)(aPacket + 9) = CharServChecksum((const uint8_t *)pbData, nLength);
    size_t nAccount = strlen(szAccountName) + 1;
    memcpy(aPacket + 0x0D, szAccountName, nAccount);
    size_t nChar = strlen(szCharName) + 1;
    memcpy(aPacket + 0x0D + nAccount, szCharName, nChar);
    if (*pbData != nLength) {
        FOG_DisplayAssert("*(WORD *)pbData == nLength", s_szCharServFile, 0xF9);
        _exit(-1);
    }
    memcpy(aPacket + 0x0D + nAccount + nChar, pbData, nLength);
    return SendToCharServer(aPacket, (int)(nAccount + 0x0D + nChar + nLength));
}

// Command 3: [0] 3, [1] 1, [5] dwToken, [9] game name.
// 1.00 D2Server: 10002210
bool __fastcall CB_ValidateGameName_WithToken(char * pszGameName, uint32_t dwToken) {
    uint8_t aPacket[0x1C];
    aPacket[0] = 3;
    *(uint32_t *)(aPacket + 1) = 1;
    *(uint32_t *)(aPacket + 5) = dwToken;
    size_t nName = strlen(pszGameName) + 1;
    memcpy(aPacket + 9, pszGameName, nName);
    return SendToCharServer(aPacket, (int)(nName + 9));
}

// Command 5: [0] 5, [1] dwFlag, [5] game name.
// 1.00 D2Server: 10002300
bool __fastcall CB_ValidateGameName_WithFlag(char * pszGameName, uint32_t dwFlag) {
    uint8_t aPacket[0x18];
    aPacket[0] = 5;
    *(uint32_t *)(aPacket + 1) = dwFlag;
    size_t nName = strlen(pszGameName) + 1;
    memcpy(aPacket + 5, pszGameName, nName);
    return SendToCharServer(aPacket, (int)(nName + 5));
}

// Command 4: [0] 4, [1] wParam, [3] dwExtra, [7] game name.
// 1.00 D2Server: 100023e0
bool __fastcall CB_SendGameNameWithWordAndDword(char * pszGameName, uint16_t wParam, uint32_t dwExtra) {
    uint8_t aPacket[0x18];
    aPacket[0] = 4;
    *(uint16_t *)(aPacket + 1) = wParam;
    *(uint32_t *)(aPacket + 3) = dwExtra;
    size_t nName = strlen(pszGameName) + 1;
    memcpy(aPacket + 7, pszGameName, nName);
    return SendToCharServer(aPacket, (int)(nName + 7));
}

// Command 7: [0] 7, [1] game name.
// 1.00 D2Server: 100024d0
bool __fastcall CB_SendGameNameOnly(char * pszGameName) {
    uint8_t aPacket[0x14];
    aPacket[0] = 7;
    size_t nName = strlen(pszGameName) + 1;
    memcpy(aPacket + 1, pszGameName, nName);
    return SendToCharServer(aPacket, (int)(nName + 1));
}

// The connect thread: every configured character server is tried until one answers,
// then a random live one becomes the first to send to. A slot whose address starts
// with '0' is unused.
// 1.00 D2Server: 10002640
DWORD __stdcall ConnectToCharServers(LPVOID pUnused) {
    char szLabel[0x100];
    uint8_t nHello;
    BOOL bConnected = FALSE;
    for (int i = 0; i < Inifile::GetCharServerCount(); ++i) {
        in_addr tAddr;
        tAddr.s_addr = Inifile::GetCharServerIp(i);
        SStrCopy(g_csConnTable[i].szIpAddr, inet_ntoa(tAddr), 0x7fffffff);
        WinMain::PostStatusMessage(1, (char *)"[CHARACTER SERVER]  Connecting to Character Server at %s", g_csConnTable[i].szIpAddr);
    }
    for (;;) {
        for (int i = 0; i < Inifile::GetCharServerCount(); ++i) {
            CharServerConn * pConn = &g_csConnTable[i];
            if (pConn->szIpAddr[0] == '0') {
                continue;
            }
            sprintf(szLabel, "Game Server->Char Server #%d", i);
            pConn->hSocket = FOG_SOCKET_Connect(pConn->szIpAddr, 6113, szLabel);
            FOG_SOCKET_SetBlockingMode(pConn->hSocket, nullptr, 0);
            FOG_10052(pConn->hSocket);
            nHello = 5;
            FOG_SOCKET_SendNoLock(pConn->hSocket, &nHello, 1);
            if (FOG_10058(pConn->hSocket)) {
                WinMain::PostStatusMessage(1, (char *)"[CHARACTER SERVER]  Connected to Character Server at %s", pConn->szIpAddr);
                bConnected = TRUE;
            }
        }
        if (bConnected) {
            srand(GetTickCount());
            uint32_t nStart = (uint32_t)rand() % (uint32_t)Inifile::GetCharServerCount();
            uint32_t nCount = (uint32_t)Inifile::GetCharServerCount();
            for (g_csConnRoundRobinIdx = (nStart + 1) % nCount;
                 g_csConnRoundRobinIdx != nStart && g_csConnTable[g_csConnRoundRobinIdx].hSocket == nullptr;
                 g_csConnRoundRobinIdx = (g_csConnRoundRobinIdx + 1) % nCount) {
                nCount = (uint32_t)Inifile::GetCharServerCount();
            }
            WinMain::PostStatusMessage(1, (char *)"[CHARACTER SERVER]  Chose Primary Character Server: %s", g_csConnTable[g_csConnRoundRobinIdx].szIpAddr);
            return 1;
        }
        Sleep(100);
    }
}

// Runs ConnectToCharServers on a thread and keeps the dialog alive until it is done.
// 1.00 D2Server: 10002850
BOOL __fastcall WaitForCharServerThread(HWND hDlg) {
    BOOL bAnyConnected = FALSE;
    BOOL bRunning = TRUE;
    DWORD dwThreadId;
    MSG tMsg;
    HANDLE hThread = CreateThread(nullptr, 0, ConnectToCharServers, nullptr, 0, &dwThreadId);
    do {
        if (WaitForSingleObject(hThread, 0) == WAIT_OBJECT_0) {
            break;
        }
        if (!PeekMessageA(&tMsg, nullptr, 0, 0, PM_NOREMOVE)) {
            Sleep(10);
        } else {
            bRunning = GetMessageA(&tMsg, nullptr, 0, 0);
            if (!IsDialogMessageA(hDlg, &tMsg)) {
                TranslateMessage(&tMsg);
                DispatchMessageA(&tMsg);
            }
        }
    } while (bRunning);
    for (int i = 0; i < Inifile::GetCharServerCount(); ++i) {
        if (g_csConnTable[i].szIpAddr[0] != '0' && FOG_10058(g_csConnTable[i].hSocket)) {
            bAnyConnected = TRUE;
        }
    }
    return bAnyConnected;
}

// Once a minute: ping every live character server (command 8) and reconnect the
// dead ones, asynchronously. FALSE when none is alive.
// 1.00 D2Server: 10002980
BOOL __stdcall TickCharServerConnections() {
    char szLabel[0x100];
    uint8_t nCmd;
    BOOL bAlive = FALSE;
    uint32_t dwNow = (uint32_t)time(nullptr);
    if (g_csLastTickTime == 0) {
        g_csLastTickTime = dwNow;
    }
    if (dwNow - g_csLastTickTime < 60) {
        return TRUE;
    }
    g_csLastTickTime = dwNow;
    for (int i = 0; i < Inifile::GetCharServerCount(); ++i) {
        CharServerConn * pConn = &g_csConnTable[i];
        if (pConn->szIpAddr[0] == '\0' || pConn->szIpAddr[0] == '0') {
            continue;
        }
        if (FOG_10058(pConn->hSocket)) {
            nCmd = 8;
            SendToCharServer(&nCmd, 1);
            bAlive = TRUE;
            continue;
        }
        FOG_SOCKET_Destroy(pConn->hSocket);
        pConn->hSocket = nullptr;
        nCmd = 5;
        if (pConn->hConnThread == nullptr) {
            WinMain::PostStatusMessage(1, (char *)"[CHARACTER SERVER]  Lost connection with %s", pConn->szIpAddr);
            SStrPrintf(szLabel, sizeof(szLabel), "Game Server->Char Server #%d", i);
            pConn->hSocket = FOG_SOCKET_Connect(pConn->szIpAddr, 6113, szLabel);
            pConn->hConnThread = FOG_SOCKET_ConnectEx(pConn->hSocket);
            WinMain::PostStatusMessage(1, (char *)"[CHARACTER SERVER]  Connecting to Character Server at %s", pConn->szIpAddr);
        } else if (WaitForSingleObject(pConn->hConnThread, 0) != WAIT_TIMEOUT) {
            CloseHandle(pConn->hConnThread);
            pConn->hConnThread = nullptr;
            if (pConn->hSocket) {
                FOG_SOCKET_SendNoLock(pConn->hSocket, &nCmd, 1);
                WinMain::PostStatusMessage(1, (char *)"[CHARACTER SERVER]  Connected to Character Server at %s", pConn->szIpAddr);
            } else {
                SStrPrintf(szLabel, sizeof(szLabel), "Game Server->Char Server #%d", i);
                pConn->hSocket = FOG_SOCKET_Connect(pConn->szIpAddr, 6113, szLabel);
                pConn->hConnThread = FOG_SOCKET_ConnectEx(pConn->hSocket);
            }
        }
    }
    return bAlive;
}

// 1.00 D2Server: 10002be0
void __stdcall DestroyAllCharServerConnections() {
    for (CharServerConn * pConn = g_csConnTable; pConn < g_csConnTable + 3; ++pConn) {
        if (pConn->hSocket) {
            FOG_SOCKET_Destroy(pConn->hSocket);
        }
        pConn->hSocket = nullptr;
    }
}

// Reads every character server and hands each packet to its command's handler. The
// length is not checked against the handler's needs (a short command 3 reads past
// the packet).
// 1.00 D2Server: 10002c20
BOOL __stdcall PollCharServerPackets() {
    uint8_t aBuffer[0x1020];
    for (int i = 0; i < Inifile::GetCharServerCount(); ++i) {
        if (!g_csConnTable[i].hSocket) {
            continue;
        }
        int16_t nLength = (int16_t)FOG_SOCKET_Receive(g_csConnTable[i].hSocket, aBuffer, sizeof(aBuffer));
        if (nLength != 0 && aBuffer[0] < 9 && g_csInboundHandlers[aBuffer[0]]) {
            g_csInboundHandlers[aBuffer[0]](aBuffer, nLength);
        }
    }
    return TRUE;
}

} // namespace D2Server::Core::CharServ
