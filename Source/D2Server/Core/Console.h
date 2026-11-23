// D2Server: the console dialog and its commands (no __FILE__ survives; the name is
// the Ghidra program's).
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.

#pragma once

#include "Source/D2Server/D2Server.h"

namespace D2Server::Core::Console {

HWND __stdcall GetConsoleDialogHwnd();                                          // 1.00 D2Server: 10002ce0
BOOL __fastcall CreateConsoleDialog(HWND hParent);                              // 1.00 D2Server: 10002d00
void __stdcall DestroyConsoleDialog();                                          // 1.00 D2Server: 10002d50
INT_PTR __stdcall ConsoleDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); // 1.00 D2Server: 10002d80
int __fastcall ConsoleCmd_Profile(HWND hDlg, char * pszArgs);                   // 1.00 D2Server: 100030f0
int __fastcall ConsoleCmd_GlobalMessage(HWND hDlg, char * pszArgs);             // 1.00 D2Server: 10003120
int __fastcall ConsoleCmd_LocalMessage(HWND hDlg, char * pszArgs);              // 1.00 D2Server: 100031e0
int __fastcall ConsoleCmd_Perf(HWND hDlg, char * pszSubCmd);                    // 1.00 D2Server: 100032b0
int __fastcall ConsoleCmd_Mem(HWND hDlg, char * pszSubCmd);                     // 1.00 D2Server: 10003330
int __fastcall ConsoleCmd_LocalMem(HWND hDlg, char * pszSubCmd);                // 1.00 D2Server: 10003410
void __fastcall DispatchConsoleCommand(HWND hWnd, char * pszCommandLine);       // 1.00 D2Server: 10003520
int __fastcall DisplayConsoleHelp(HWND hDlg, char * pszArgs);                   // 1.00 D2Server: 10003740

} // namespace D2Server::Core::Console
