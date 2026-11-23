// D2Server CORE\Inifile.cpp: realms.ini.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.

#pragma once

#include "Source/D2Server/D2Server.h"

namespace D2Server::Core::Inifile {

void __fastcall GetModuleDir(char * szPath);                                    // 1.00 D2Server: 10005ba0
int __cdecl ParseIniFile();                                                     // 1.00 D2Server: 10005cf0
char * __fastcall SkipWhitespace(char * pStr, char * pDelimiters);              // 1.00 D2Server: 10007390
char * __fastcall ParseAcceptLine(char * pLine, char * pOut, char * pDelimiters); // 1.00 D2Server: 100073e0
char * __stdcall GetMcpIp();                                                    // 1.00 D2Server: 10007560
int __stdcall GetMaxMultiClient();                                              // 1.00 D2Server: 10007580
int __stdcall GetCharServerCount();                                             // 1.00 D2Server: 100075a0
uint32_t __fastcall GetCharServerIp(int nIndex);                                // 1.00 D2Server: 100075c0
BOOL __fastcall StatusLog_InsertLine(int nIndex, char * szLine);                // 1.00 D2Server: 10007770

extern char g_szIniWhitespaceChars[8];                                          // 1.00 D2Server: 10026d40

} // namespace D2Server::Core::Inifile
