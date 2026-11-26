// Redivivus' own code, not recovered from any Blizzard binary.
//
// The default server host: the minimal public one. It brings the QServer up as a
// dedicated Battle.net-style game server, registers a realm callback table whose slots
// only log, and ticks every game until it is told to stop. It does not talk to a realm,
// so no join token is ever valid and no client gets into a game; a host that serves
// players replaces this file and keeps ServerHost.h.
//
// The bring-up is NET_QServer_StartServer's host branch (0044bc30) minus the local
// player's connection, with SetupAsBnetServer (0052c0e0) ahead of it - the same order the
// d2gs host uses. The tick loop is QSERVER_CooperativeThreadMain's (0044cf20).
#include <windows.h>

#include <cstdarg>
#include <cstdlib>
#include <cstring>

#include "Source/globals/globals.h"
#include "Source/D2Client/D2IniConfigStrc.h"                // D2IniConfigStrc, filed under D2Client
#include "Source/D2Client/D2BattleNetEventCallbackTable.h"  // the realm table, filed under D2Client
#include "Source/D2Client/Game/Game/SGAMEDATA.h"            // the game table's element, filed under D2Client
#include "Source/Storm/H/stpl.h"
#include "Source/D2Game/Game/Server.h"
#include "Source/D2Net/D2Net.h"
#include "Redivivus/Server/ServerData.h"
#include "Redivivus/Server/ServerHost.h"
#include "Redivivus/Server/ServerLog.h"

namespace Redivivus::Server {
namespace {

using GameTable = TSExportTableSync<::SGAMEDATA, HSGAMEDATA__*, HSGAMEDATALOCK__*, CCritSect, 0>;

// ---------------------------------------------------------------------------
// The realm callback table.
//
// The engine dispatches each slot through a null test, so a slot left null is a feature
// the engine skips. A stub is only safe where its stack cleanup is known: the slots are
// __fastcall (callee pops), and a wrong count corrupts the engine's stack where nothing
// checks it. So only the slots whose 1.14d argument count has been counted at the
// dispatch site get a stub (d2up-server packages/d2engine/callbacks.zig, v114d:
// fpLeaveGame 18 stack arguments, fpGetDatabaseCharacter 2, fpSaveDatabaseCharacter 4,
// fpFindPlayerToken 7), plus the cdecl logger, where the caller cleans up. The rest stay
// null until someone counts them.
//
// The tree's D2BattleNetEventCallbackTable types fpLeaveGame and fpFindPlayerToken as
// cdecl with 18 and 7 parameters; the dispatch sites pass ECX and EDX as well and the
// callees pop 0x48 and 0x1c. The stubs below have the counted prototypes and are cast in.
// ---------------------------------------------------------------------------

void ServerLogMessageStub(int nLevel, const char * szFormat, ...) {
    char szLine[1024];
    va_list args;
    va_start(args, szFormat);
    vsnprintf(szLine, sizeof(szLine), szFormat ? szFormat : "", args);
    va_end(args);
    ServerLog("engine[%d]: %s", nLevel, szLine);
}

void __fastcall GetDatabaseCharacterStub(D2ClientStrc *, LPCSTR szCharName, DWORD dwClientId, LPCSTR szAccountName) {
    ServerLog("realm: character %s (account %s, client %lu) requested, no realm to load it from",
        szCharName ? szCharName : "?", szAccountName ? szAccountName : "?", (unsigned long)dwClientId);
}

void __fastcall SaveDatabaseCharacterStub(uint32_t *, char * szCharName, char * szAccountName, void *, int nSize, uint32_t) {
    ServerLog("realm: save of %s (account %s, %d bytes) dropped, no realm to store it",
        szCharName ? szCharName : "?", szAccountName ? szAccountName : "?", nSize);
}

void __fastcall LeaveGameStub(int, int, int, int, int, int, int, int, int, int,
                              int, int, int, int, int, int, int, int, int, int) {
    ServerLog("realm: a client left a game");
}

int __fastcall FindPlayerTokenStub(int, int, int, int, int, int, int, int, int) {
    ServerLog("realm: join token refused, no realm issued it");
    return 0;
}

D2BattleNetEventCallbackTable * BuildCallbackTable() {
    static D2BattleNetEventCallbackTable sTable;
    memset(&sTable, 0, sizeof(sTable));
    sTable.fpServerLogMessage = (uint8_t *)&ServerLogMessageStub;
    sTable.fpGetDatabaseCharacter = &GetDatabaseCharacterStub;
    sTable.fpSaveDatabaseCharacter = &SaveDatabaseCharacterStub;
    sTable.fpLeaveGame = (decltype(sTable.fpLeaveGame))(void *)&LeaveGameStub;
    sTable.fpFindPlayerToken = (decltype(sTable.fpFindPlayerToken))(void *)&FindPlayerTokenStub;
    return &sTable;
}

// ---------------------------------------------------------------------------
// The game table. Game.exe keeps it in a D2Client file-static, built by the C++ static
// initialiser at 006ca9c0 (QSERVER_InitGlobalGameState); the server owns its own and
// builds it the same way.
// ---------------------------------------------------------------------------
GameTable gServerGameTable;
bool gbServerGameTableBuilt = false;

void BuildGameTable() {
    gServerGameTable.TSHashTableReuse<::SGAMEDATA, HASHKEY_NONE, 0>::Constructor();
    gServerGameTable.nSequence = ~(uint32_t)(uintptr_t)&gServerGameTable & 0xfffffff;
    gServerGameTable.bWrapped = 0;
    gServerGameTable.pVtable = (TSHashTable<::SGAMEDATA, HASHKEY_NONE>::Vtable *)(void *)GameTable::vftable;
    InitializeCriticalSection(&gServerGameTable.tSync.m_critsect);
    gbServerGameTableBuilt = true;
}

void DestroyGameTable() {
    if (!gbServerGameTableBuilt) {
        return;
    }
    DeleteCriticalSection(&gServerGameTable.tSync.m_critsect);
    gServerGameTable.TSHashTableReuse<::SGAMEDATA, HASHKEY_NONE, 0>::Destructor();
    gbServerGameTableBuilt = false;
}

// ---------------------------------------------------------------------------
// Stopping. Ctrl-C, Ctrl-Break, closing the console, logoff and shutdown all ask the
// loop to finish its tick and leave; -exitafter <seconds> does the same on a timer.
// ---------------------------------------------------------------------------
volatile LONG gbQuitRequested = 0;

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            InterlockedExchange(&gbQuitRequested, 1);
            return TRUE;
        default:
            return FALSE;
    }
}

DWORD ExitAfterMilliseconds() {
    const char * szCmdLine = GetCommandLineA();
    const char * p = strstr(szCmdLine, "-exitafter");
    if (!p) {
        return 0;
    }
    const long nSeconds = strtol(p + strlen("-exitafter"), nullptr, 10);
    return nSeconds > 0 ? (DWORD)nSeconds * 1000 : 0;
}

// The APPMODE_server row's run function. Like the client's, it loads the data first and
// returns the next app mode: APPMODE_mode0, which ends ApplicationMain's loop.
int32_t __fastcall RunServerHost(D2IniConfigStrc * pIni) {
    if (!ServerData_Load(pIni)) {
        // Nothing of the server is up yet; leave with a failure code, as the game's own
        // dispatch does on a halt (ApplicationRunCurrentMode, exit(-1)).
        exit(1);
    }

    D2Game::GAME::Server::SetupAsBnetServer(BuildCallbackTable());
    D2Net::Game::Server::QSERVER_CreateAndInit(CONNECTIONTYPE_SERVER, GAMETYPE_BNET);
    D2Net::Game::Server::NET_HACK_SetUseQServerHack(0);
    BuildGameTable();
    static int sInitState = 1;      // any non-null value; the engine only tests it
    D2Game::GAME::Server::QSERVER_SetGlobalInstance(&gServerGameTable, &sInitState);
    D2Game::GAME::Server::QSERVER_InitializeServerState();
    ServerLog("server up (%s), ticking", pIni && pIni->bNOSAVE ? "no saves" : "realm saves");

    const DWORD dwExitAfter = ExitAfterMilliseconds();
    const DWORD dwStart = GetTickCount();
    unsigned long nTicks = 0;
    unsigned long nBusyTicks = 0;
    DWORD dwLastReport = dwStart;
    while (!gbQuitRequested) {
        D2Game::GAME::Server::NET_D2GS_SERVER_HandleAnyIncomingPacket();
        const int32_t bHasActiveGames = D2Game::GAME::Server::SrvProcessAllGames(1);
        if (bHasActiveGames) {
            D2Game::GAME::Server::QSERVER_DispatchAndCleanup(nullptr, 0);
            nBusyTicks++;
        }
        nTicks++;
        Sleep(bHasActiveGames ? 30 : 10);

        const DWORD dwNow = GetTickCount();
        if (dwNow - dwLastReport >= 10000) {
            ServerLog("alive: %lu passes, %lu with games", nTicks, nBusyTicks);
            dwLastReport = dwNow;
        }
        if (dwExitAfter && dwNow - dwStart >= dwExitAfter) {
            ServerLog("-exitafter reached");
            break;
        }
    }
    ServerLog("stopping after %lu passes (%lu with games)", nTicks, nBusyTicks);

    D2Game::GAME::Server::QSERVER_ShutdownAllGames();
    D2Game::GAME::Server::QSERVER_DispatchAndCleanup(nullptr, 1);
    D2Net::Game::Server::NET_QServer_FreeAndSend0xB0ToEveryone();
    D2Game::GAME::Server::SetupAsBnetServer(nullptr);
    ServerData_Free();
    return APPMODE_mode0;
}

fnAppModeRun gpfnServerHostRun = &RunServerHost;

}  // namespace

fnAppModeRun * (__fastcall ServerHost_Init)(HMODULE) {
    SetConsoleCtrlHandler(&ConsoleCtrlHandler, TRUE);
    return &gpfnServerHostRun;
}

void __stdcall ServerHost_Shutdown() {
    DestroyGameTable();
    SetConsoleCtrlHandler(&ConsoleCtrlHandler, FALSE);
}

}  // namespace Redivivus::Server
