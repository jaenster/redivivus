// D2Server: the performance graph and log (no __FILE__ survives; the name is the
// Ghidra program's).
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.

#pragma once

#include "Source/D2Server/D2Server.h"

namespace D2Server::Core::Performance {

HWND __stdcall GetPerfGraphDialogHwnd();                                        // 1.00 D2Server: 10007810
BOOL __fastcall CreatePerfGraphDialog(HWND hParent);                            // 1.00 D2Server: 10007830
void __stdcall DestroyPerfGraphDialog();                                        // 1.00 D2Server: 100079c0
INT_PTR __stdcall PerfGraphDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); // 1.00 D2Server: 100079f0
void __fastcall PerfGraph_SetTitle(char * szText);                              // 1.00 D2Server: 10007b80
void __fastcall PerfGraph_SetSubtitle(char * szText);                           // 1.00 D2Server: 10007bb0
void __fastcall PerfGraph_PlotSample(int nBusyPercent, int nSecondValue);       // 1.00 D2Server: 10007be0
BOOL __stdcall GetPerfLogHandle();                                              // 1.00 D2Server: 10007ed0
void __fastcall SetPerfLogEnabled(BOOL bEnabled);                               // 1.00 D2Server: 10007ef0
void __fastcall PerfLog_SetPath(char * pPathPrefix);                            // 1.00 D2Server: 10008010
void __fastcall PerfLog_Write(uint32_t nBusyPercent, D2GameServerStats * pStats); // 1.00 D2Server: 10008050
void __fastcall PerfLog_WriteRaw(char * pLine);                                 // 1.00 D2Server: 10008170

} // namespace D2Server::Core::Performance
