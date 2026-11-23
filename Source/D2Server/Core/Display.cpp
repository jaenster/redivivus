// D2Server CORE\Display.cpp: the status window's game, player and unit lists, and the
// busy-time measurement behind its progress bar.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.

#include "Source/D2Server/Core/Display.h"
#include "Source/D2Server/Core/Performance.h"
#include "Source/D2Server/Core/WinMain.h"
#include "Source/D2Server/D2ServerImports.h"

namespace D2Server::Core::Display {

using namespace D2Server::Imports;

static const char s_szDisplayFile[] = "C:\\D2\\Source\\D2Server\\CORE\\Display.cpp";

// The status window's controls.
enum {
    IDC_GAMES = 1000,               // the game list (names)
    IDC_GAMEDETAIL = 0x3E9,         // the selected game's 25 counters
    IDC_UNITLIST = 0x3EA,           // players or units of the selected row
    IDC_UNITDETAIL = 0x3EB,
    IDC_BUSYBAR = 0x3EC,
    IDC_BUSYTEXT = 0x3ED,
    IDC_STAT_GAMES = 0x3F7,
    IDC_STAT_PLAYERS = 0x3F6,
    IDC_STAT_MONSTERS = 0x3F8,
    IDC_STAT_MONSTERTOTAL = 0x3F1,
    IDC_ITEMLIST = 0x3FA,
};

// Which handler fills the unit list for a row of the game detail list.
typedef void (__fastcall * PlayerActionHandler)(GameInfoNode * pGame, int nSubItemIndex);
#pragma pack(push, 1)
struct PlayerActionEntry {
    int32_t nListboxId;             // 0x00 row of IDC_GAMEDETAIL
    PlayerActionHandler pfHandler;  // 0x04
    int32_t nSubItemIndex;          // 0x08 unit type handed to the handler
};
#pragma pack(pop)

static const PlayerActionEntry g_aPlayerActionTable[5] = {           // 1.00 D2Server: 100237a0
    { 3, GameListAction_PopulatePlayerList, 0 },
    { 4, GameListAction_AddGameEntry, 1 },
    { 5, GameListAction_AddGameEntry, 2 },
    { 6, GameListAction_AddGameEntry, 3 },
    { 7, GameListAction_AddGameEntry, 4 },
};

static LRESULT LRESULT_10023798 = -1;                                 // 1.00 D2Server: 10023798 (last handled detail row)
static uint32_t DAT_1002379c = (uint32_t)-1;                          // 1.00 D2Server: 1002379c (busy value last shown)
static int32_t g_nSelectedListboxItem = -1;                           // 1.00 D2Server: 100237dc
static int32_t g_bPlayerDetailDirty = -1;                             // 1.00 D2Server: 100237e0

static int64_t g_nPerfDivisorTicks = 0;                               // 1.00 D2Server: 10026c68
static int64_t g_nPerfStart = 0;                                      // 1.00 D2Server: 10026c70
uint64_t g_nPerfIdleTicks = 0;                                        // 1.00 D2Server: 10026c78
static int64_t g_nPerfIdleTicks2 = 0;                                 // 1.00 D2Server: 10026c80
static int64_t g_nPerfFrequency = 0;                                  // 1.00 D2Server: 10026c88
static int64_t g_nPerfNextRefresh = 0;                                // 1.00 D2Server: 10026c90
static int32_t g_bPerfAvailable = 0;                                  // 1.00 D2Server: 10026c98
uint32_t g_nBusyPercent = 0;                                          // 1.00 D2Server: 10026c9c
static int64_t g_nPerfMark = 0;                                       // 1.00 D2Server: 10026ca0
static GameInfoNode * g_pGameListHead = nullptr;                      // 1.00 D2Server: 10026ca8
static GameInfoNode * g_pSelectedGame = nullptr;                      // 1.00 D2Server: 10026cac
static char CHAR_00h_10026cb0[16] = {};                               // 1.00 D2Server: 10026cb0 (selected game's name)
static int32_t BOOL_10026cc0 = 0;                                     // 1.00 D2Server: 10026cc0 (redraw the detail list)
static LRESULT DAT_10026cc4 = 0;                                      // 1.00 D2Server: 10026cc4 (unit shown in the item list)

// The refresh period: the counter frequency divided by nDivisor.
// 1.00 D2Server: 100039d0
void __fastcall InitPerfTimerDivisor(uint32_t nDivisor) {
    LARGE_INTEGER tFrequency;
    LARGE_INTEGER tNow;
    QueryPerformanceFrequency(&tFrequency);
    g_nPerfDivisorTicks = tFrequency.QuadPart / (int32_t)nDivisor;
    QueryPerformanceCounter(&tNow);
    g_nPerfNextRefresh = g_nPerfDivisorTicks + tNow.QuadPart;
}

// The game whose name is selected in the game list; noting a new selection marks the
// detail list for a full redraw.
// 1.00 D2Server: 10003a60
GameInfoNode * __stdcall GetSelectedPlayerByAccountName() {
    char szName[16];
    WPARAM nSelected = SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMES, LB_GETCURSEL, 0, 0);
    if (nSelected == (WPARAM)-1) {
        return nullptr;
    }
    SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMES, LB_GETTEXT, nSelected, (LPARAM)szName);
    if (strcmp(CHAR_00h_10026cb0, szName) != 0) {
        SStrCopy(CHAR_00h_10026cb0, szName, 0x7fffffff);
        BOOL_10026cc0 = 1;
    }
    for (GameInfoNode * pNode = g_pGameListHead; pNode; pNode = pNode->pNext) {
        if (strcmp(pNode->szGameName, szName) == 0) {
            return pNode;
        }
    }
    return nullptr;
}

// 1.00 D2Server: 10003ba0
void __stdcall InitPerfTimer() {
    if (QueryPerformanceFrequency((LARGE_INTEGER *)&g_nPerfFrequency)) {
        g_bPerfAvailable = 1;
        g_nPerfIdleTicks2 = 0;
        QueryPerformanceCounter((LARGE_INTEGER *)&g_nPerfStart);
        return;
    }
    g_bPerfAvailable = 0;
}

// 1.00 D2Server: 10003bf0
void __stdcall PerfTimerMark() {
    QueryPerformanceCounter((LARGE_INTEGER *)&g_nPerfMark);
}

// Once a second of counter time has passed since the mark: the busy percentage of
// that span, and a new mark.
// 1.00 D2Server: 10003c10
BOOL __stdcall PerfTimerGetElapsed() {
    LARGE_INTEGER tNow;
    QueryPerformanceCounter(&tNow);
    int64_t nElapsed = tNow.QuadPart - g_nPerfMark;
    if (nElapsed >= g_nPerfFrequency) {
        g_nBusyPercent = (uint32_t)(((int64_t)g_nPerfIdleTicks * 100) / nElapsed);
        g_nPerfIdleTicks2 = 0;
        g_nPerfIdleTicks = 0;
        QueryPerformanceCounter((LARGE_INTEGER *)&g_nPerfMark);
        return TRUE;
    }
    return FALSE;
}

// Detail row 3: the game's players, named, or "<class> <id>" while unnamed.
// 1.00 D2Server: 10003ce0
void __fastcall GameListAction_PopulatePlayerList(GameInfoNode * pGame, int nUnused) {
    char aClassFormats[5][0x40] = { "Amazon %d", "Sorceress %d", "Necromancer %d", "Paladin %d", "Barbarian %d" };
    char szLine[0x58];
    int nPlayers = D2GAME_10016(pGame->wGameId);
    if (nPlayers == 0) {
        return;
    }
    D2PlayerListEntry * pPlayers = (D2PlayerListEntry *)FOG_CrashDeadlockDetected(nPlayers * sizeof(D2PlayerListEntry), s_szDisplayFile, 0x14E, 0);
    if (!pPlayers) {
        return;
    }
    memset(pPlayers, 0, nPlayers * sizeof(D2PlayerListEntry));
    int nCount = D2GAME_10017(pGame->wGameId, pPlayers, nPlayers);
    if (nCount == 0) {
        FOG_Alloc(pPlayers, s_szDisplayFile, 0x156, 0);
        return;
    }
    SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_UNITLIST, LB_RESETCONTENT, 0, 0);
    for (D2PlayerListEntry * pPlayer = pPlayers; nCount > 0; ++pPlayer, --nCount) {
        if (pPlayer->szName[0] == '\0') {
            sprintf(szLine, aClassFormats[pPlayer->nClass], pPlayer->nUnitId);
        } else {
            strcpy(szLine, pPlayer->szName);
        }
        WPARAM nIndex = SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_UNITLIST, LB_ADDSTRING, 0, (LPARAM)szLine);
        SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_UNITLIST, LB_SETITEMDATA, nIndex, pPlayer->nUnitId);
    }
    FOG_Alloc(pPlayers, s_szDisplayFile, 0x168, 0);
}

// 1.00 D2Server: 10004030
void __fastcall StrToLower(char * str) {
    int nLength = (int)strlen(str);
    for (int i = 0; i < nLength; ++i) {
        if (str[i] <= 'Z' && str[i] >= 'A') {
            str[i] += ' ';
        }
    }
}

// Detail rows 4..7: the game's units of one type, "<total> - <name> (<active>/<inactive>)".
// 1.00 D2Server: 10004070
void __fastcall GameListAction_AddGameEntry(GameInfoNode * pGame, int nUnitType) {
    char szLine[0x80];
    D2UnitDescription tHead;
    memset(&tHead, 0, sizeof(tHead));
    tHead.nField00 = -1;
    GAME_GetUnitsDescriptions(pGame->wGameId, &tHead, nUnitType);
    SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_UNITLIST, LB_RESETCONTENT, 0, 0);
    for (D2UnitDescription * pUnit = tHead.pNext; pUnit; pUnit = pUnit->pNext) {
        sprintf(szLine, "%3d - %s (%d/%d)", pUnit->nActive + pUnit->nInactive, pUnit->szName, pUnit->nActive, pUnit->nInactive);
        StrToLower(szLine);
        SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_UNITLIST, LB_ADDSTRING, 0, (LPARAM)szLine);
    }
    for (D2UnitDescription * pUnit = tHead.pNext; pUnit;) {
        D2UnitDescription * pNext = pUnit->pNext;
        FOG_Alloc(pUnit, s_szDisplayFile, 0x19C, 0);
        pUnit = pNext;
    }
}

// The window's periodic refresh: the game list against D2Game's, the selected game's
// detail, the unit and item lists, the totals and the graph.
// 1.00 D2Server: 100041d0
void __stdcall RefreshDisplay() {
    D2GameServerStats tStats;
    SYSTEM_INFO tSystemInfo;
    LARGE_INTEGER tNow;
    char szText[0x100];
    LRESULT aSelection[16];
    LRESULT aItemData[16];
    char aItemText[0x340];

    QueryPerformanceCounter(&tNow);
    if (tNow.QuadPart >= g_nPerfNextRefresh) {
        GetSystemInfo(&tSystemInfo);
        g_nPerfNextRefresh = (g_nPerfDivisorTicks + tNow.QuadPart) / (int64_t)tSystemInfo.dwNumberOfProcessors;

        int nIds = 0;
        uint16_t * pGameIds = nullptr;
        int nGames = GAME_GetGamesCount();
        if (nGames != 0) {
            pGameIds = (uint16_t *)FOG_CrashDeadlockDetected(nGames * sizeof(uint16_t), s_szDisplayFile, 0x48, 0);
            if (!pGameIds) {
                FOG_DisplayAssert("ptwGameIds", s_szDisplayFile, 0x49);
                _exit(-1);
            }
            memset(pGameIds, 0, nGames * sizeof(uint16_t));
            nIds = GAME_GetGameServerGameIds(pGameIds, nGames);
        }
        for (GameInfoNode * pNode = g_pGameListHead; pNode; pNode = pNode->pNext) {
            pNode->bActive = 0;
        }
        for (uint16_t * pId = pGameIds; nIds > 0; --nIds, ++pId) {
            GameInfoNode * pNode = FindGameNode(*pId);
            if (!pNode) {
                AddGame(*pId);
            } else {
                pNode->bActive = 1;
            }
        }
        RemoveStaleGames();
        if (pGameIds) {
            FOG_Alloc(pGameIds, s_szDisplayFile, 0xCD, 0);
        }
        RefreshGameDetail();

        LRESULT nRow = SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMEDETAIL, LB_GETCURSEL, 0, 0);
        if (nRow != -1 && nRow != LRESULT_10023798) {
            for (int i = 0; i < 5; ++i) {
                LRESULT_10023798 = nRow;
                if (g_aPlayerActionTable[i].nListboxId == nRow) {
                    GameInfoNode * pGame = GetSelectedPlayerByAccountName();
                    if (pGame) {
                        g_aPlayerActionTable[i].pfHandler(pGame, g_aPlayerActionTable[i].nSubItemIndex);
                    }
                    break;
                }
            }
        }

        GameInfoNode * pGame = GetSelectedPlayerByAccountName();
        if (pGame) {
            nRow = SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMEDETAIL, LB_GETCURSEL, 0, 0);
            if (nRow != -1) {
                for (int i = 0; i < 5; ++i) {
                    if (g_aPlayerActionTable[i].nListboxId != nRow) {
                        continue;
                    }
                    int nSubItem = g_aPlayerActionTable[i].nSubItemIndex;
                    WPARAM nUnit = SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_UNITLIST, LB_GETCURSEL, 0, 0);
                    if (nUnit != (WPARAM)-1) {
                        LRESULT nUnitData = SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_UNITLIST, LB_GETITEMDATA, nUnit, 0);
                        if (nSubItem == 0 && (*(int32_t *)pGame != g_nSelectedListboxItem || g_bPlayerDetailDirty || nUnitData != DAT_10026cc4)) {
                            g_bPlayerDetailDirty = 0;
                            g_nSelectedListboxItem = *(int32_t *)pGame;
                            DAT_10026cc4 = nUnitData;
                            SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_ITEMLIST, LB_RESETCONTENT, 0, 0);
                            aItemData[0] = 0;
                            aSelection[0] = nUnitData;
                            int nItems = D2GAME_10018(pGame->wGameId, 1, aItemText, aItemData, aSelection);
                            for (int j = 0; j < nItems; ++j) {
                                // Every row is formatted from the same place in the buffer.
                                sprintf(szText, "%s", aItemText + 0x20);
                                SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_ITEMLIST, LB_INSERTSTRING, j, (LPARAM)szText);
                            }
                        }
                    }
                    break;
                }
            }
        }

        GAME_GetStatistics(&tStats);
        sprintf(szText, "%d", tStats.nGames);
        SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_STAT_GAMES, WM_SETTEXT, 0, (LPARAM)szText);
        sprintf(szText, "%d", tStats.nPlayers);
        SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_STAT_PLAYERS, WM_SETTEXT, 0, (LPARAM)szText);
        Performance::PerfGraph_SetSubtitle(szText);
        sprintf(szText, "%d", tStats.nActiveMonsters);
        SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_STAT_MONSTERS, WM_SETTEXT, 0, (LPARAM)szText);
        sprintf(szText, "%d  (%d / %d)", tStats.nActiveMonsters + tStats.nInactiveMonsters, tStats.nActiveMonsters, tStats.nInactiveMonsters);
        Performance::PerfGraph_SetTitle(szText);
        SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_STAT_MONSTERTOTAL, WM_SETTEXT, 0, (LPARAM)szText);
        Performance::PerfGraph_PlotSample(g_nBusyPercent, tStats.nActiveMonsters);
        Performance::PerfLog_Write(g_nBusyPercent, &tStats);
    }
    UpdateBusyBar();
}

// The low word is the game id; the compare reads the whole dword.
// 1.00 D2Server: 100046f0
GameInfoNode * __fastcall FindGameNode(uint32_t gameId) {
    for (GameInfoNode * pNode = g_pGameListHead; pNode; pNode = pNode->pNext) {
        if (*(uint32_t *)pNode == (gameId & 0xFFFF)) {
            return pNode;
        }
    }
    return nullptr;
}

// 1.00 D2Server: 10004720
void __fastcall AddGame(uint32_t gameId) {
    GameInfoNode * pNode = (GameInfoNode *)FOG_CrashDeadlockDetected(sizeof(GameInfoNode), s_szDisplayFile, 0x67, 0);
    if (!pNode) {
        FOG_DisplayAssert("ptGameInfo", s_szDisplayFile, 0x68);
        _exit(-1);
    }
    memset(pNode, 0, sizeof(GameInfoNode));
    if (!GAME_GetGameInformation((uint16_t)gameId, pNode)) {
        FOG_Alloc(pNode, s_szDisplayFile, 0x6E, 0);
        return;
    }
    pNode->pNext = g_pGameListHead;
    g_pGameListHead = pNode;
    pNode->bActive = 1;
    SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMES, LB_ADDSTRING, 0, (LPARAM)pNode->szGameName);
}

// Drops every game the last refresh did not see, from the list and the window.
// 1.00 D2Server: 100047f0
void __stdcall RemoveStaleGames() {
    char szItem[16];
    GameInfoNode * pPrev = nullptr;
    GameInfoNode * pNode = g_pGameListHead;
    for (;;) {
        while (pNode && pNode->bActive) {
            pPrev = pNode;
            pNode = pNode->pNext;
        }
        if (!pNode) {
            return;
        }
        LRESULT nCount = SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMES, LB_GETCOUNT, 0, 0);
        for (WPARAM i = 0; (LRESULT)i < nCount; ++i) {
            SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMES, LB_GETTEXT, i, (LPARAM)szItem);
            if (strcmp(szItem, pNode->szGameName) == 0) {
                SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMES, LB_DELETESTRING, i, 0);
                break;
            }
        }
        if (!pPrev) {
            g_pGameListHead = pNode->pNext;
            FOG_Alloc(pNode, s_szDisplayFile, 0xA5, 0);
            pNode = g_pGameListHead;
        } else {
            pPrev->pNext = pNode->pNext;
            FOG_Alloc(pNode, s_szDisplayFile, 0xA1, 0);
            pNode = pPrev->pNext;
        }
    }
}

// 1.00 D2Server: 10004980
void __stdcall UpdateBusyBar() {
    char szBusyText[100];
    if (g_bPerfAvailable && g_nBusyPercent != DAT_1002379c) {
        SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_BUSYBAR, WM_USER + 2 /* PBM_SETPOS */, g_nBusyPercent, 0);
        sprintf(szBusyText, "%d%% Busy", g_nBusyPercent);
        SetWindowTextA(GetDlgItem(WinMain::g_hMainWnd, IDC_BUSYTEXT), szBusyText);
        DAT_1002379c = g_nBusyPercent;
    }
}

// The selected game's 25 counters, each row rewritten only when its value changed
// (all of them after a new selection), and the unit list for a newly selected game.
// The previous values live in the game's own node, over the info D2Game gave AddGame.
// 1.00 D2Server: 10004a20
void __stdcall RefreshGameDetail() {
    char szSelectedGameName[16];
    char szSelectedGameName2[16];
    char aFormatStrings[25][64] = {
        "Game Id: %d", "Seed: %d", "Clients: %d", "Players: %d", "Monsters: %d",
        "Objects: %d", "Items: %d", "Missiles: %d", "Unique Items: %d", "NPCs: %d",
        "Next Player Id: %d", "Next Monster Id: %d", "Next Object Id: %d", "Next Item Id: %d",
        "Next Missile Id: %d", "Next Warp Id: %d", "Path Toward Pct: %d", "Path Clock  Pct: %d",
        "Path Countr Pct: %d", "Path FoWall Pct: %d", "Path Astar  Pct: %d", "Path Total Call: %d",
        "Frames(h): %d", "Time (min): %d", "Frame Rate: %d",
    };
    CHAR szRow[64];
    GameInfoNode tGameInfo;

    GameInfoNode * pGame = nullptr;
    WPARAM nSelected = SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMES, LB_GETCURSEL, 0, 0);
    if (nSelected != (WPARAM)-1) {
        SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMES, LB_GETTEXT, nSelected, (LPARAM)szSelectedGameName);
        if (strcmp(CHAR_00h_10026cb0, szSelectedGameName) != 0) {
            SStrCopy(CHAR_00h_10026cb0, szSelectedGameName, 0x7fffffff);
            BOOL_10026cc0 = 1;
        }
        for (pGame = g_pGameListHead; pGame; pGame = pGame->pNext) {
            if (strcmp(pGame->szGameName, szSelectedGameName) == 0) {
                break;
            }
        }
    }
    if (!pGame) {
        g_pSelectedGame = nullptr;
        SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMEDETAIL, LB_RESETCONTENT, 0, 0);
        SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_UNITLIST, LB_RESETCONTENT, 0, 0);
        SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_UNITDETAIL, LB_RESETCONTENT, 0, 0);
        BOOL_10026cc0 = 1;
        return;
    }

    memset(&tGameInfo, 0, sizeof(tGameInfo));
    GAME_GetGameInformation(pGame->wGameId, &tGameInfo);
    const int32_t * pNew = (const int32_t *)&tGameInfo;
    int32_t * pOld = (int32_t *)pGame;
    for (WPARAM i = 0; (int)i < 25; ++i) {
        if (pNew[i] != pOld[i] || BOOL_10026cc0) {
            wsprintfA(szRow, aFormatStrings[i], pNew[i]);
            WPARAM nCurrent = SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMEDETAIL, LB_GETCURSEL, 0, 0);
            SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMEDETAIL, LB_DELETESTRING, i, 0);
            SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMEDETAIL, LB_INSERTSTRING, i, (LPARAM)szRow);
            if (nCurrent != (WPARAM)-1) {
                SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMEDETAIL, LB_SETCURSEL, nCurrent, 0);
            }
            pOld[i] = pNew[i];
        }
    }

    if (g_pSelectedGame == pGame) {
        BOOL_10026cc0 = 0;
        return;
    }
    g_pSelectedGame = pGame;
    LRESULT nRow = SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMEDETAIL, LB_GETCURSEL, 0, 0);
    if (nRow == -1) {
        BOOL_10026cc0 = 0;
        return;
    }
    int nAction = 0;
    for (; nAction < 5; ++nAction) {
        if (g_aPlayerActionTable[nAction].nListboxId == nRow) {
            break;
        }
    }
    LRESULT_10023798 = nRow;
    if (nAction == 5) {
        BOOL_10026cc0 = 0;
        return;
    }
    nSelected = SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMES, LB_GETCURSEL, 0, 0);
    if (nSelected == (WPARAM)-1) {
        BOOL_10026cc0 = 0;
        return;
    }
    SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMES, LB_GETTEXT, nSelected, (LPARAM)szSelectedGameName2);
    if (strcmp(CHAR_00h_10026cb0, szSelectedGameName2) != 0) {
        SStrCopy(CHAR_00h_10026cb0, szSelectedGameName2, 0x7fffffff);
        BOOL_10026cc0 = 1;
    }
    for (GameInfoNode * pNode = g_pGameListHead; pNode; pNode = pNode->pNext) {
        if (strcmp(pNode->szGameName, szSelectedGameName2) == 0) {
            g_aPlayerActionTable[nAction].pfHandler(pNode, g_aPlayerActionTable[nAction].nSubItemIndex);
            break;
        }
    }
    BOOL_10026cc0 = 0;
}

// 1.00 D2Server: 100055b0
void __stdcall PurgeAllGames() {
    char szItem[16];
    GameInfoNode * pNode = g_pGameListHead;
    while (pNode) {
        g_pGameListHead = pNode;
        LRESULT nCount = SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMES, LB_GETCOUNT, 0, 0);
        for (WPARAM i = 0; (LRESULT)i < nCount; ++i) {
            SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMES, LB_GETTEXT, i, (LPARAM)szItem);
            if (strcmp(szItem, pNode->szGameName) == 0) {
                SendDlgItemMessageA(WinMain::g_hMainWnd, IDC_GAMES, LB_DELETESTRING, i, 0);
                break;
            }
        }
        g_pGameListHead = pNode->pNext;
        FOG_Alloc(pNode, s_szDisplayFile, 0x2B6, 0);
        pNode = g_pGameListHead;
    }
    g_pGameListHead = pNode;
}

} // namespace D2Server::Core::Display
