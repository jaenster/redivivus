// D2Server PlayerToken.cpp: the joins the MCP announced, waiting for their clients.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.
//
// The MCP registers a join (FromMCP's OnJoinGame) before the client reaches the game
// server; D2Game asks for it back through the pfFindPlayerToken callback when the
// client arrives. A token lives 120001 ms and is consumed by the lookup.

#include "Source/D2Server/PlayerToken.h"
#include "Source/D2Server/D2ServerImports.h"

namespace D2Server::PlayerToken {

using namespace D2Server::Imports;

static const char s_szPlayerTokenFile[] = "C:\\D2\\Source\\D2Server\\PlayerToken.cpp";

static PlayerTokenNode * PTR_100294e4 = nullptr;                      // 1.00 D2Server: 100294e4

// Prunes the expired tokens, then updates the game's token or adds a new one.
// 1.00 D2Server: 10008250
int __fastcall PlayerToken_RegisterOrUpdate(char * pGameName, char * pAccountName, uint32_t dwGameToken, uint32_t dwClientToken, uint32_t dwServerIp) {
    DWORD dwNow = GetTickCount();
    PlayerTokenNode * pNode = PTR_100294e4;
    PlayerTokenNode ** ppLink = &PTR_100294e4;
    while (pNode) {
        if (dwNow - pNode->dwTickStamp <= 120000) {
            ppLink = &pNode->pNext;
        } else {
            *ppLink = pNode->pNext;
            FOG_Alloc(pNode, s_szPlayerTokenFile, 0x96, 0);
        }
        pNode = *ppLink;
    }
    for (pNode = PTR_100294e4; pNode; pNode = pNode->pNext) {
        if (SStrCmp(pNode->szGameName, pGameName, 0x7fffffff) == 0) {
            goto update;
        }
    }
    pNode = (PlayerTokenNode *)FOG_CrashDeadlockDetected(sizeof(PlayerTokenNode), s_szPlayerTokenFile, 0x34, 0);
    if (pNode) {
        memset(pNode, 0, sizeof(PlayerTokenNode));
        pNode->dwServerIp = dwServerIp;
        pNode->dwClientToken = dwClientToken;
        pNode->dwTickStamp = GetTickCount();
        SStrCopy(pNode->szGameName, pGameName, 0x10);
        SStrCopy(pNode->szAccountName, pAccountName, 0x10);
        pNode->dwGameToken = dwGameToken;
        pNode->pNext = PTR_100294e4;
        PTR_100294e4 = pNode;
        return 1;
    }
    FOG_DisplayAssert("ptToken", s_szPlayerTokenFile, 0x35);
    _exit(-1);
update:
    pNode->dwServerIp = dwServerIp;
    pNode->dwClientToken = dwClientToken;
    pNode->dwTickStamp = GetTickCount();
    SStrCopy(pNode->szAccountName, pAccountName, 0x10);
    pNode->dwGameToken = dwGameToken;
    return 1;
}

// D2Game's pfFindPlayerToken: the game's token is removed whether or not it matches;
// TRUE when the client token and address match and it has not expired.
// 1.00 D2Server: 10008400
BOOL __fastcall PlayerToken_ValidateAndConsume(char * pszGameName, uint32_t dwClientToken, uint32_t dwServerIp, char * pszAccountNameOut, uint32_t * pdwGameTokenOut) {
    DWORD dwNow = GetTickCount();
    PlayerTokenNode ** ppLink = &PTR_100294e4;
    PlayerTokenNode * pNode = PTR_100294e4;
    for (;;) {
        if (!pNode) {
            return FALSE;
        }
        if (SStrCmp(pNode->szGameName, pszGameName, 0x7fffffff) == 0) {
            break;
        }
        ppLink = &pNode->pNext;
        pNode = pNode->pNext;
    }
    *ppLink = pNode->pNext;
    BOOL bMatched = pNode->dwClientToken == dwClientToken && pNode->dwServerIp == dwServerIp && dwNow - pNode->dwTickStamp <= 120000;
    SStrCopy(pszAccountNameOut, pNode->szAccountName, 0x10);
    *pdwGameTokenOut = pNode->dwGameToken;
    FOG_Alloc(pNode, s_szPlayerTokenFile, 99, 0);
    return bMatched;
}

} // namespace D2Server::PlayerToken
