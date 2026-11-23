// D2Server PlayerToken.cpp: the joins the MCP announced, waiting for their clients.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.

#pragma once

#include "Source/D2Server/D2Server.h"

namespace D2Server::PlayerToken {

int __fastcall PlayerToken_RegisterOrUpdate(char * pGameName, char * pAccountName, uint32_t dwGameToken, uint32_t dwClientToken, uint32_t dwServerIp); // 1.00 D2Server: 10008250
BOOL __fastcall PlayerToken_ValidateAndConsume(char * pszGameName, uint32_t dwClientToken, uint32_t dwServerIp, char * pszAccountNameOut, uint32_t * pdwGameTokenOut); // 1.00 D2Server: 10008400

} // namespace D2Server::PlayerToken
