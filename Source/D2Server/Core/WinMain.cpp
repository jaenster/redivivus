// D2Server CORE\WinMain.cpp: start-up and shut-down, the D2Game callback table, the
// MCP service thread, the game worker threads and the status window.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.
//
// The start-up order (docs/engine-internals.md): Fog, the MPQs, the launch switches,
// the window, Fog's error manager, realms.ini, Bnclient, D2Lang, the data tables,
// D2Net, D2Game's module init, the character servers, the callbacks, the game-data
// table, the clock, the MCP thread, one worker per processor, the message loop.

#include "Source/D2Server/Core/WinMain.h"
#include "Source/D2Server/Core/CharServ.h"
#include "Source/D2Server/Core/Console.h"
#include "Source/D2Server/Core/Display.h"
#include "Source/D2Server/Core/FromMCP.h"
#include "Source/D2Server/Core/Inifile.h"
#include "Source/D2Server/Core/Performance.h"
#include "Source/D2Server/Core/ToMCP.h"
#include "Source/D2Server/PlayerToken.h"
#include "Source/D2Server/D2ServerImports.h"

namespace D2Server::Core {

using namespace D2Server::Imports;

static int32_t g_bLadderEnabled = 1;                                  // 1.00 D2Server: 100249d0
static int32_t g_bCharServerEnabled = 1;                              // 1.00 D2Server: 100249d4

// '0' in the launch configuration's server name runs without character servers.
// 1.00 D2Server: 10009270
void __fastcall StaticInit_SetServerName(D2ServerLaunchConfig * pConfig) {
    if (pConfig->szServerName[0] != '0') {
        g_bCharServerEnabled = 1;
        lstrcpyA(WinMain::g_szServerName, pConfig->szServerName);
        return;
    }
    g_bCharServerEnabled = 0;
}

// 1.00 D2Server: 100092b0
void __fastcall StaticInit_SetLadderFlag(D2ServerLaunchConfig * pConfig) {
    g_bLadderEnabled = pConfig->cLadder != '0';
}

// 1.00 D2Server: 100092e0
void __fastcall StaticInit_LoadD2CommonTxt(D2ServerLaunchConfig * pConfig) {
    if (pConfig->cLoadD2CommonTxt != '\0') {
        D2COMMON_10980();
    }
}

// 1.00 D2Server: 10009230
void __fastcall StaticInit_Nop_A(D2ServerLaunchConfig * pConfig) {
}

// 1.00 D2Server: 10009250
void __fastcall StaticInit_Nop_B(D2ServerLaunchConfig * pConfig) {
}

} // namespace D2Server::Core

namespace D2Server::Core::WinMain {

using namespace D2Server::Imports;

static const char s_szWinMainFile[] = "C:\\D2\\Source\\D2Server\\CORE\\WinMain.cpp";

typedef void (__fastcall * LaunchSwitchHandler)(D2ServerLaunchConfig * pConfig);

// The launch configuration's handlers, run in order with the configuration; the
// table has 45 slots and five are used.
static const LaunchSwitchHandler g_aStaticInitTable[45] = {          // 1.00 D2Server: 100249d8
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, StaticInit_SetLadderFlag, StaticInit_SetServerName, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, StaticInit_Nop_A, nullptr, nullptr, nullptr, nullptr, nullptr,
    StaticInit_LoadD2CommonTxt, nullptr, nullptr, nullptr, nullptr, StaticInit_Nop_B, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr,
};

// The callbacks D2Game is given; it keeps the pointer, so the table is static.
static D2ServerCallbackFunctions g_serverCallbackTable = {           // 1.00 D2Server: 10024a90
    (void *)ToMCP_SendLeaveGame,                         // 0x00
    (void *)CB_CreateGame,                               // 0x04
    (void *)CB_ValidateGameName_WithToken,               // 0x08
    (void *)CB_AssertSrvDataRecordSize,                  // 0x0C
    (void *)PostStatusMessage,                           // 0x10
    (void *)CB_JoinGame,                                 // 0x14
    (void *)PlayerToken::PlayerToken_ValidateAndConsume, // 0x18
    (void *)CB_SendGameData,                             // 0x1C
    (void *)CB_SendGameNameOnly,                         // 0x20
    (void *)CB_SendPacket,                               // 0x24
    (void *)CB_HandlePacket,                             // 0x28
    (void *)CB_DisconnectClient,                         // 0x2C
    nullptr, nullptr, nullptr, nullptr,                  // 0x30..0x3C
};

HWND g_hMainWnd = nullptr;                                            // 1.00 D2Server: 100294e8
static WNDPROC g_pfnConsoleInputOrigProc = nullptr;                  // 1.00 D2Server: 100294ec
uint32_t g_dServerStartTime = 0;                                      // 1.00 D2Server: 100294f0
volatile LONG g_nActiveWorkerThreads = 0;                             // 1.00 D2Server: 100294f4
LadderThresholds g_tLadderThresholds = {};                            // 1.00 D2Server: 100294f8
char g_szServerName[0x108] = {};                                      // 1.00 D2Server: 10029580
static HINSTANCE g_hInstance = nullptr;                               // 1.00 D2Server: 100296f0
static uint32_t g_gameDataParam2 = 0;                                 // 1.00 D2Server: 100296f8
int32_t g_bShutdownRequested = 0;                                     // 1.00 D2Server: 100296fc
static uint32_t g_dwLastLeaveGameAckTick = 0;                         // 1.00 D2Server: 10029700
static uint32_t g_dwLastGameJoinInfoTick = 0;                         // 1.00 D2Server: 10029704

enum {
    WM_D2SERVER_STATUS = WM_USER + 1,   // lParam: a line for the status log, freed here
    IDC_STATUSLOG = 0x3FB,
    IDC_CONSOLELINE = 0x3FD,
    IDC_UPTIME = 0x3F9,
    IDM_EXIT = 0x3F5,
    IDM_CONSOLE = 0x3F4,
    IDM_PERFGRAPH = 0x3F3,
    IDM_REFRESH = 0x3F2,
    IDM_PERFLOG = 0x3F9,
};

// The launch-switch handlers as the module's initializers; this caller sets no
// configuration.
// 1.00 D2Server: 100091f0
void __stdcall RunModuleInitializers() {
    for (const LaunchSwitchHandler * pfn = g_aStaticInitTable; pfn < g_aStaticInitTable + 45; ++pfn) {
        if (*pfn) {
            (*pfn)(nullptr);
        }
    }
}

// A line for the status log, posted to the window (which frees it); with bIsError it
// also goes to Fog's error log. pFmt is a printf format.
// 1.00 D2Server: 10009310
void __cdecl PostStatusMessage(int bIsError, char * pFmt, ...) {
    char szMsg[0x100];
    va_list args;
    va_start(args, pFmt);
    vsprintf(szMsg, pFmt, args);
    va_end(args);
    if (bIsError) {
        FOG_DisplayError(szMsg);
    }
    char * pLine = (char *)FOG_CrashDeadlockDetected(strlen(szMsg) + 1, s_szWinMainFile, 0x126, 0);
    SStrCopy(pLine, szMsg, 0x7fffffff);
    PostMessageA(g_hMainWnd, WM_D2SERVER_STATUS, 0, (LPARAM)pLine);
}

// Callback slot 0x20.
// 1.00 D2Server: 100093d0
bool __fastcall CB_SendGameNameOnly(char * pszGameName) {
    return CharServ::CB_SendGameNameOnly(pszGameName);
}

// Callback slot 0x14.
// 1.00 D2Server: 100093f0
void __fastcall CB_JoinGame(uint32_t dwToken, char * szText, uint16_t wField1, uint16_t wField2, uint16_t wField3) {
    ToMCP::ToMCP_SendMotd(dwToken, szText, wField1, wField2, wField3);
}

// Callback slot 0x04: the character to the character server (command 7) and to the
// MCP (0x08).
// 1.00 D2Server: 10009420
void __fastcall CB_CreateGame(uint32_t dwToken, uint16_t wClass, uint16_t wLevel, uint32_t dwExperienceLo, uint32_t dwExperienceHi, uint32_t dwFlags, char * szCharName, char * szAccountName) {
    CharServ::CB_SendGameNameOnly(szCharName);
    ToMCP::ToMCP_SendCharacterList(dwToken, wClass, wLevel, dwExperienceLo, dwExperienceHi, dwFlags, szCharName, szAccountName);
}

// Callback slot 0x08.
// 1.00 D2Server: 10009470
bool __fastcall CB_ValidateGameName_WithToken(char * pszGameName, uint32_t dwToken) {
    return CharServ::CB_ValidateGameName_WithToken(pszGameName, dwToken);
}

// Callback slot 0x0C: a record whose first word is not its size is fatal.
// 1.00 D2Server: 10009490
void __fastcall CB_AssertSrvDataRecordSize(char * szAccountName, char * szCharName, uint16_t * pRecord, uint32_t nExpectedSize, uint32_t dwExtra) {
    if (*pRecord != nExpectedSize) {
        FOG_DisplayError("Invalid size of record in SrvDatabaseSaveCharacter for '%s'", szAccountName);
        FOG_DisplayAssert("0", s_szWinMainFile, 0x177);
        _exit(-1);
    }
    CharServ::CB_AssertSrvDataRecordSize(szAccountName, szCharName, pRecord, nExpectedSize, dwExtra);
}

// Callback slot 0x00.
// 1.00 D2Server: 10009500
void __stdcall ToMCP_SendLeaveGame() {
    ToMCP::ToMCP_SendLeaveGame();
}

// Callback slot 0x1C.
// 1.00 D2Server: 10009520
void __fastcall CB_SendGameData(char * pGameName, uint8_t * pData, int nLength) {
    ToMCP::ToMCP_SendCharacterData(pGameName, pData, nLength);
}

// Callback slot 0x24.
// 1.00 D2Server: 10009540
void __fastcall CB_SendPacket(char * pGameName, uint8_t nDifficulty, char * pDescription, uint32_t dwGameFlags) {
    ToMCP::ToMCP_SendGameOpen(pGameName, nDifficulty, pDescription, dwGameFlags);
}

// Callback slot 0x28.
// 1.00 D2Server: 10009570
void __fastcall CB_HandlePacket(char * pCharName, uint16_t wClass, uint32_t dwLevel, uint32_t dwExperienceLo, uint32_t dwExperienceHi, uint32_t dwFlags) {
    ToMCP::ToMCP_SendCharEnter(pCharName, wClass, dwLevel, dwExperienceLo, dwExperienceHi, dwFlags);
}

// Callback slot 0x2C.
// 1.00 D2Server: 100095a0
void __fastcall CB_DisconnectClient(uint32_t dwToken, char * pCharName, uint16_t wClass) {
    ToMCP::ToMCP_SendCharCreate(dwToken, pCharName, wClass);
}

// 1.00 D2Server: 10009710
uint32_t __stdcall GetWorkerStackSize() {
    return 0x400;
}

// 1.00 D2Server: 10009730
uint32_t __stdcall GetWorkerStackSize2() {
    return 0x400;
}

// Seconds since the server started.
// 1.00 D2Server: 10009750
int64_t __stdcall GetCurrentTimeAsTick() {
    time_t tNow;
    tNow = time(&tNow);
    return (int64_t)difftime(tNow, (time_t)g_dServerStartTime);
}

// The status window. The console line is subclassed while the window exists.
// 1.00 D2Server: 10009790
INT_PTR __stdcall ServerWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char szTimeStr[64];
    if (uMsg == WM_D2SERVER_STATUS) {
        SendDlgItemMessageA(g_hMainWnd, IDC_STATUSLOG, LB_INSERTSTRING, (WPARAM)-1, lParam);
        LRESULT nCount = SendDlgItemMessageA(g_hMainWnd, IDC_STATUSLOG, LB_GETCOUNT, 0, 0);
        WPARAM nLast = nCount - 1;
        if ((int)nLast > 99) {
            SendDlgItemMessageA(g_hMainWnd, IDC_STATUSLOG, LB_DELETESTRING, 0, 0);
            nLast = nCount - 2;
        }
        SendDlgItemMessageA(g_hMainWnd, IDC_STATUSLOG, LB_SETCURSEL, nLast, 0);
        FOG_Alloc((void *)lParam, s_szWinMainFile, 0x25E, 0);
        return 0;
    }
    if (uMsg == WM_DESTROY) {
        SetWindowLongA(GetDlgItem(hWnd, IDC_CONSOLELINE), GWL_WNDPROC, (LONG)(intptr_t)g_pfnConsoleInputOrigProc);
        return 0;
    }
    if (uMsg == WM_INITDIALOG) {
        g_pfnConsoleInputOrigProc = (WNDPROC)(intptr_t)SetWindowLongA(GetDlgItem(hWnd, IDC_CONSOLELINE), GWL_WNDPROC, (LONG)(intptr_t)Display::ConsoleInputProc);
        return TRUE;
    }
    if (uMsg != WM_COMMAND) {
        return 0;
    }
    switch (LOWORD(wParam)) {
    case IDCANCEL:
    case IDM_EXIT:
        GAME_CloseAllGames();
        if (Performance::GetPerfGraphDialogHwnd()) {
            Performance::DestroyPerfGraphDialog();
        }
        if (Console::GetConsoleDialogHwnd()) {
            Console::DestroyConsoleDialog();
        }
        PostQuitMessage(0);
        return TRUE;
    case IDM_REFRESH: {
        int nSeconds = (int)GetCurrentTimeAsTick();
        sprintf(szTimeStr, "%d:%02d:%02d", nSeconds / 3600, nSeconds / 60 % 60, nSeconds % 60);
        SendDlgItemMessageA(g_hMainWnd, IDC_UPTIME, WM_SETTEXT, 0, (LPARAM)szTimeStr);
        Display::RefreshDisplay();
        return TRUE;
    }
    case IDM_PERFGRAPH: {
        HMENU hMenu = GetMenu(hWnd);
        if (Performance::GetPerfGraphDialogHwnd()) {
            CheckMenuItem(hMenu, IDM_PERFGRAPH, MF_UNCHECKED);
            Performance::DestroyPerfGraphDialog();
            return 0;
        }
        CheckMenuItem(hMenu, IDM_PERFGRAPH, MF_CHECKED);
        Performance::CreatePerfGraphDialog(hWnd);
        return 0;
    }
    case IDM_CONSOLE: {
        HMENU hMenu = GetMenu(hWnd);
        if (Console::GetConsoleDialogHwnd()) {
            CheckMenuItem(hMenu, IDM_CONSOLE, MF_UNCHECKED);
            Console::DestroyConsoleDialog();
            return 0;
        }
        CheckMenuItem(hMenu, IDM_CONSOLE, MF_CHECKED);
        Console::CreateConsoleDialog(hWnd);
        return 0;
    }
    case IDM_PERFLOG: {
        HMENU hMenu = GetMenu(hWnd);
        if (Performance::GetPerfLogHandle()) {
            CheckMenuItem(hMenu, IDM_PERFLOG, MF_UNCHECKED);
            Performance::SetPerfLogEnabled(FALSE);
            return 0;
        }
        CheckMenuItem(hMenu, IDM_PERFLOG, MF_CHECKED);
        Performance::SetPerfLogEnabled(TRUE);
        return 0;
    }
    }
    return 0;
}

// The MCP thread: the character servers' replies, the MCP's messages, the keep-alive
// every 30 s, the heartbeat every 60 s, and the links' upkeep. Losing every character
// server shuts the server down.
// 1.00 D2Server: 10009b80
DWORD __stdcall McpServiceThread(LPVOID pUnused) {
    uint8_t aRecvBuf[0x400];
    GameJoinInfoFields tJoinInfo;
    if (g_bShutdownRequested) {
        return 1;
    }
    do {
        CharServ::PollCharServerPackets();
        if (FOG_SOCKET_Select(CharServ::g_McpConnection, 0) == 0) {
            uint16_t nReceived;
            while ((nReceived = (uint16_t)FOG_SOCKET_Receive(CharServ::g_McpConnection, aRecvBuf, sizeof(aRecvBuf))) != 0) {
                FromMCP::DispatchClientPacket(aRecvBuf, nReceived);
            }
        }
        uint32_t dwNow = GetTickCount();
        if (g_dwLastLeaveGameAckTick == 0 || dwNow - g_dwLastLeaveGameAckTick > 30000) {
            g_dwLastLeaveGameAckTick = dwNow;
            ToMCP::ToMCP_SendLeaveGameAck();
        }
        if (g_dwLastGameJoinInfoTick == 0 || dwNow - g_dwLastGameJoinInfoTick > 60000) {
            tJoinInfo.nPlayers = (uint8_t)D2GAME_10051();
            tJoinInfo.wPad = 0;
            tJoinInfo.nGames = (uint16_t)GAME_GetGamesCount();
            g_dwLastGameJoinInfoTick = dwNow;
            ToMCP::ToMCP_SendGameJoinInfo(&tJoinInfo);
        }
        CharServ::MaintainCharServerConnection();
        if (!CharServ::TickCharServerConnections()) {
            GAME_CloseAllGames();
            if (Performance::GetPerfGraphDialogHwnd()) {
                Performance::DestroyPerfGraphDialog();
            }
            if (Console::GetConsoleDialogHwnd()) {
                Console::DestroyConsoleDialog();
            }
            FOG_DisplayError("FATAL: Lost connection to character server.  Check character server ip address\n"
                             "in registry or verify that this computer's ip address is listed in the accept\n"
                             "list on the character server in cs.ini.");
            g_bShutdownRequested = 1;
        }
        Sleep(1);
    } while (!g_bShutdownRequested);
    return 1;
}

// The MCP's message 0x13.
// 1.00 D2Server: 10009d10 (Ghidra has no function here; reached from FromMCP's slot 0x13)
void __stdcall OnShutdownRemotely() {
    FOG_DisplayError("Shutdown Remotely by MCP");
    GAME_CloseAllGames();
    if (Performance::GetPerfGraphDialogHwnd()) {
        Performance::DestroyPerfGraphDialog();
    }
    if (Console::GetConsoleDialogHwnd()) {
        Console::DestroyConsoleDialog();
    }
    g_bShutdownRequested = 1;
}

// One pass of a worker: client traffic (every pass without a game, every eighth with
// one), then the game.
// 1.00 D2Server: 10009d60 (Ghidra has no function here; nothing in the binary calls it)
int __fastcall GameWorkerTick(void * pTask, uint32_t * pnCounter) {
    int bHasGame;
    int nWait = D2GAME_10043(pTask, &bHasGame);
    if (!bHasGame || ((*pnCounter)++ & 7) == 0) {
        if (D2NET_10022(nWait < 0 ? 0 : nWait)) {
            GAME_ProcessNetworkMessages();
        }
    }
    if (bHasGame) {
        TASK_ProcessGame(pTask);
    }
    return 1;
}

// A worker thread: GameWorkerTick's pass until shut-down.
// 1.00 D2Server: 10009de0
DWORD __stdcall GameWorkerThread(LPVOID pUnused) {
    void * pTask = TASK_Create();
    uint32_t nCounter = 0;
    while (!g_bShutdownRequested) {
        int bHasGame;
        int nWait = D2GAME_10043(pTask, &bHasGame);
        if (!bHasGame || (nCounter++ & 7) == 0) {
            if (D2NET_10022(nWait < 0 ? 0 : nWait)) {
                GAME_ProcessNetworkMessages();
            }
        }
        if (bHasGame) {
            TASK_ProcessGame(pTask);
        }
    }
    InterlockedDecrement(&g_nActiveWorkerThreads);
    return 1;
}

// 1.00 D2Server: 10009e80
void __stdcall Noop() {
}

// The server's main. The MCP host calls it through FromMCP's OnServerStart with its
// launch configuration in the third argument.
// 1.00 D2Server: 10009ea0
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, D2ServerLaunchConfig * pConfig, int nCmdShow) {
    MSG tMsg;
    DWORD dwThreadId;
    g_hInstance = hInstance;
    FOG_10139(1);
    CharServ::LoadMPQArchives();
    for (const LaunchSwitchHandler * pfn = g_aStaticInitTable; pfn < g_aStaticInitTable + 45; ++pfn) {
        if (*pfn) {
            (*pfn)(pConfig);
        }
    }
    g_hMainWnd = CreateDialogParamA(g_hInstance, MAKEINTRESOURCEA(0x66), nullptr, ServerWindowProc, 0);
    ShowWindow(g_hMainWnd, nCmdShow);
    UpdateWindow(g_hMainWnd);
    FOG_InitErrorMgr("D2SDebug");
    if (Inifile::ParseIniFile()) {
        if (g_bCharServerEnabled) {
            CharServ::LoadBnClientDll();
        }
        if (D2LANG_10000(CharServ::Stub_ReturnZero())) {
            DATATBLS_LoadAllTxts(CharServ::Stub_ReturnZero(), TRUE, 0);
            D2NET_SERVER_Initialize(0, 0);
            D2NET_SERVER_SetMaxClientsPerGame(Inifile::GetMaxMultiClient());
            D2NET_SERVER_SetHackListEnabled(TRUE);
            D2GAME_10046();
            if (g_bCharServerEnabled) {
                CharServ::ConnectToCharServer();
                CharServ::WaitForCharServerThread(g_hMainWnd);
                GAME_SetServerCallbackFunctions(&g_serverCallbackTable);
            }
            GAME_InitGameDataTable(&ToMCP::g_gameDataParam1, &g_gameDataParam2);
            g_dServerStartTime = (uint32_t)time((time_t *)&g_dServerStartTime);
            TASK_InitializeClock();
            if (g_bCharServerEnabled) {
                CloseHandle(CreateThread(nullptr, 0, McpServiceThread, nullptr, 0, &dwThreadId));
            }
            for (int nWorkers = FOG_GetSystemInfo()->nProcessors; nWorkers != 0; --nWorkers) {
                InterlockedIncrement(&g_nActiveWorkerThreads);
                HANDLE hThread = CreateThread(nullptr, 0, GameWorkerThread, nullptr, 0, &dwThreadId);
                if (!hThread) {
                    InterlockedDecrement(&g_nActiveWorkerThreads);
                } else {
                    CloseHandle(hThread);
                }
            }
            Display::InitPerfTimer();
            Display::InitPerfTimerDivisor(2);
            Display::PerfTimerMark();
            PostStatusMessage(1, (char *)"[SERVER]  Finished Loading...");
            BOOL bMsgLoopActive = TRUE;
            Display::g_nPerfIdleTicks = 0;
            do {
                if (g_bShutdownRequested) {
                    break;
                }
                if (!PeekMessageA(&tMsg, nullptr, 0, 0, PM_NOREMOVE)) {
                    Sleep(1);
                    continue;
                }
                bMsgLoopActive = GetMessageA(&tMsg, nullptr, 0, 0);
                if (!IsDialogMessageA(Performance::GetPerfGraphDialogHwnd(), &tMsg) ||
                    !IsDialogMessageA(Console::GetConsoleDialogHwnd(), &tMsg)) {
                    TranslateMessage(&tMsg);
                    DispatchMessageA(&tMsg);
                }
            } while (bMsgLoopActive);
        }
    }
    g_bShutdownRequested = 1;
    while (g_nActiveWorkerThreads > 0) {
        Sleep(100);
    }
    g_bShutdownRequested = 0;
    Display::PurgeAllGames();
    TASK_FreeAllQueueSlots();
    D2NET_SERVER_Release();
    if (g_bCharServerEnabled) {
        CharServ::DisconnectFromCharServer();
        CharServ::DestroyAllCharServerConnections();
    }
    D2GAME_10050();
    DATATBLS_UnloadAllBins();
    D2LANG_10001();
    D2COMMON_10983();
    CharServ::UnloadMPQArchives();
    return 0;
}

} // namespace D2Server::Core::WinMain

namespace D2Server::Core::Display {

using namespace D2Server::Imports;

// The status window's console line: Enter runs the line as a console command.
// 1.00 D2Server: 100095f0 (in WinMain.cpp's code; Ghidra files it under Display)
LRESULT __stdcall ConsoleInputProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char szLine[0x100];
    if (uMsg == WM_GETDLGCODE) {
        return DLGC_WANTALLKEYS;
    }
    if (uMsg == WM_CHAR && wParam == VK_RETURN) {
        if (SendDlgItemMessageA(WinMain::g_hMainWnd, 0x3FD, WM_GETTEXT, 0x100, (LPARAM)szLine)) {
            Console::DispatchConsoleCommand(WinMain::g_hMainWnd, szLine);
        }
        szLine[0] = '\0';
        SendDlgItemMessageA(WinMain::g_hMainWnd, 0x3FD, WM_SETTEXT, 0, (LPARAM)szLine);
        SetFocus(GetDlgItem(WinMain::g_hMainWnd, 0x3FD));
        return 1;
    }
    return CallWindowProcA(WinMain::g_pfnConsoleInputOrigProc, hWnd, uMsg, wParam, lParam);
}

} // namespace D2Server::Core::Display
