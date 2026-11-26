// D2Server: the performance graph dialog and the performance log. No __FILE__ string
// survives for this unit; the name is the Ghidra program's.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.

#include "Source/D2Server/Core/Performance.h"
#include "Source/D2Server/D2ServerImports.h"

namespace D2Server::Core::Performance {

using namespace D2Server::Imports;

#pragma pack(push, 1)
struct PerfGraphState {
    HWND hDlg;                      // 0x00
    HDC hDC;                        // 0x04 off-screen copy of the graph
    HBITMAP hBitmap;                // 0x08
    BOOL bPerfLogEnabled;           // 0x0C
    int32_t nXPos;                  // 0x10 next column
};
#pragma pack(pop)

static PerfGraphState g_hPerfGraphDlg = {};                           // 1.00 D2Server: 100294cc

// The log's path; PerfLog_SetPath replaces it.
static char g_szPerfLogPath[0x134] = "C:\\D2\\Performace.txt";       // 1.00 D2Server: 100246dc

enum { IDC_GRAPH = 0x3F2, IDC_GRAPH_TITLE = 0x3F1, IDC_GRAPH_SUBTITLE = 0x3F0 };

// 1.00 D2Server: 10007810
HWND __stdcall GetPerfGraphDialogHwnd() {
    return g_hPerfGraphDlg.hDlg;
}

// The dialog, and a black off-screen bitmap the size of its graph control.
// 1.00 D2Server: 10007830
BOOL __fastcall CreatePerfGraphDialog(HWND hParent) {
    RECT tRect;
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtrA(hParent, GWLP_HINSTANCE);
    if (g_hPerfGraphDlg.hDlg) {
        return FALSE;
    }
    g_hPerfGraphDlg.hDlg = CreateDialogParamA(hInstance, MAKEINTRESOURCEA(0x67), hParent, PerfGraphDialogProc, 0);
    if (!g_hPerfGraphDlg.hDlg) {
        return FALSE;
    }
    GetClientRect(GetDlgItem(g_hPerfGraphDlg.hDlg, IDC_GRAPH), &tRect);
    HDC hDC = GetDC(g_hPerfGraphDlg.hDlg);
    g_hPerfGraphDlg.hDC = CreateCompatibleDC(hDC);
    g_hPerfGraphDlg.hBitmap = CreateCompatibleBitmap(hDC, tRect.right - tRect.left, tRect.bottom - tRect.top);
    ReleaseDC(g_hPerfGraphDlg.hDlg, hDC);
    SelectObject(g_hPerfGraphDlg.hDC, g_hPerfGraphDlg.hBitmap);
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HGDIOBJ hOldPen = SelectObject(g_hPerfGraphDlg.hDC, hPen);
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
    HGDIOBJ hOldBrush = SelectObject(g_hPerfGraphDlg.hDC, hBrush);
    Rectangle(g_hPerfGraphDlg.hDC, tRect.left, tRect.top, tRect.right, tRect.bottom);
    SelectObject(g_hPerfGraphDlg.hDC, hOldPen);
    DeleteObject(hPen);
    SelectObject(g_hPerfGraphDlg.hDC, hOldBrush);
    DeleteObject(hBrush);
    return TRUE;
}

// 1.00 D2Server: 100079c0
void __stdcall DestroyPerfGraphDialog() {
    SendMessageA(g_hPerfGraphDlg.hDlg, WM_CLOSE, 0, 0);
}

// 1.00 D2Server: 100079f0
INT_PTR __stdcall PerfGraphDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    RECT tRect;
    PAINTSTRUCT tPaint;
    if (uMsg < WM_COMMAND) {
        if (uMsg == WM_INITDIALOG) {
            return TRUE;
        }
        if (uMsg == WM_PAINT) {
            BeginPaint(hDlg, &tPaint);
            HWND hGraph = GetDlgItem(hDlg, IDC_GRAPH);
            HDC hDC = GetDC(hGraph);
            GetClientRect(hGraph, &tRect);
            BitBlt(hDC, tRect.left, tRect.top, tRect.right - tRect.left, tRect.bottom - tRect.top, g_hPerfGraphDlg.hDC, 0, 0, SRCCOPY);
            ReleaseDC(hGraph, hDC);
            EndPaint(hDlg, &tPaint);
            return TRUE;
        }
        if (uMsg == WM_CLOSE) {
            g_hPerfGraphDlg.hDlg = nullptr;
            DeleteObject(g_hPerfGraphDlg.hBitmap);
            g_hPerfGraphDlg.hBitmap = nullptr;
            DeleteObject(g_hPerfGraphDlg.hDC);
            g_hPerfGraphDlg.hDC = nullptr;
            EndDialog(hDlg, 0);
            return TRUE;
        }
    } else if (uMsg == WM_COMMAND) {
        if (wParam != IDCANCEL) {
            return TRUE;
        }
        g_hPerfGraphDlg.hDlg = nullptr;
        EndDialog(hDlg, 0);
        return TRUE;
    }
    return FALSE;
}

// 1.00 D2Server: 10007b80
void __fastcall PerfGraph_SetTitle(char * szText) {
    if (g_hPerfGraphDlg.hDlg) {
        SendDlgItemMessageA(g_hPerfGraphDlg.hDlg, IDC_GRAPH_TITLE, WM_SETTEXT, 0, (LPARAM)szText);
    }
}

// 1.00 D2Server: 10007bb0
void __fastcall PerfGraph_SetSubtitle(char * szText) {
    if (g_hPerfGraphDlg.hDlg) {
        SendDlgItemMessageA(g_hPerfGraphDlg.hDlg, IDC_GRAPH_SUBTITLE, WM_SETTEXT, 0, (LPARAM)szText);
    }
}

// One 2-pixel column: the busy percentage in green (of 100), the second value in red
// (of 600), wrapping at the right edge.
// 1.00 D2Server: 10007be0
void __fastcall PerfGraph_PlotSample(int nBusyPercent, int nSecondValue) {
    POINT tOld;
    RECT tRect;
    if (!g_hPerfGraphDlg.hDlg) {
        return;
    }
    HPEN hBlack = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HPEN hGreen = CreatePen(PS_SOLID, 1, RGB(0, 0xFF, 0));
    HPEN hRed = CreatePen(PS_SOLID, 1, RGB(0xFF, 0, 0));
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
    HWND hGraph = GetDlgItem(g_hPerfGraphDlg.hDlg, IDC_GRAPH);
    GetClientRect(hGraph, &tRect);
    int nHeight = tRect.bottom - tRect.top;
    int nLeft = g_hPerfGraphDlg.nXPos + tRect.left;
    HGDIOBJ hOldBrush = SelectObject(g_hPerfGraphDlg.hDC, hBrush);
    HGDIOBJ hOldPen = SelectObject(g_hPerfGraphDlg.hDC, hBlack);
    int nRight = nLeft + 2;
    Rectangle(g_hPerfGraphDlg.hDC, nLeft, tRect.top, nRight, tRect.bottom);
    int nY = (nHeight * nBusyPercent) / 100;
    SelectObject(g_hPerfGraphDlg.hDC, hGreen);
    MoveToEx(g_hPerfGraphDlg.hDC, nLeft, tRect.bottom - nY, &tOld);
    LineTo(g_hPerfGraphDlg.hDC, nRight, tRect.bottom - nY);
    nY = (nHeight * nSecondValue) / 600;
    SelectObject(g_hPerfGraphDlg.hDC, hRed);
    MoveToEx(g_hPerfGraphDlg.hDC, nLeft, tRect.bottom - nY, &tOld);
    LineTo(g_hPerfGraphDlg.hDC, nRight, tRect.bottom - nY);
    g_hPerfGraphDlg.nXPos += 2;
    if ((tRect.right - tRect.left) - 2 <= g_hPerfGraphDlg.nXPos) {
        g_hPerfGraphDlg.nXPos = 0;
    }
    SelectObject(g_hPerfGraphDlg.hDC, hOldPen);
    SelectObject(g_hPerfGraphDlg.hDC, hOldBrush);
    DeleteObject(hBlack);
    DeleteObject(hGreen);
    DeleteObject(hRed);
    DeleteObject(hBrush);
    HDC hDC = GetDC(hGraph);
    GetClientRect(hGraph, &tRect);
    BitBlt(hDC, tRect.left, tRect.top, tRect.right - tRect.left, tRect.bottom - tRect.top, g_hPerfGraphDlg.hDC, 0, 0, SRCCOPY);
    ReleaseDC(hGraph, hDC);
}

// 1.00 D2Server: 10007ed0
BOOL __stdcall GetPerfLogHandle() {
    return g_hPerfGraphDlg.bPerfLogEnabled;
}

// Appends a line to the log (a lone CR ends each line).
static void PerfLogAppend(const char * szLine) {
    uint8_t nCarriageReturn = '\r';
    DWORD dwWritten;
    HANDLE hFile = CreateFileA(g_szPerfLogPath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == nullptr) {
        MessageBoxA(nullptr, "Error creating performance logfile!", "Error", MB_ICONHAND);
        g_hPerfGraphDlg.bPerfLogEnabled = (BOOL)(intptr_t)hFile;
        return;
    }
    SetFilePointer(hFile, 0, nullptr, FILE_END);
    WriteFile(hFile, szLine, (DWORD)strlen(szLine), &dwWritten, nullptr);
    WriteFile(hFile, &nCarriageReturn, 1, &dwWritten, nullptr);
    CloseHandle(hFile);
}

// Switching the log on writes its column header.
// 1.00 D2Server: 10007ef0
void __fastcall SetPerfLogEnabled(BOOL bEnabled) {
    g_hPerfGraphDlg.bPerfLogEnabled = bEnabled;
    if (bEnabled) {
        PerfLogAppend("Busy\tGames\tPlayers\tActiveMonsters\tInActiveMonsters");
    }
}

// 1.00 D2Server: 10008010
void __fastcall PerfLog_SetPath(char * pPathPrefix) {
    strcpy(g_szPerfLogPath, pPathPrefix);
}

// 1.00 D2Server: 10008050
void __fastcall PerfLog_Write(uint32_t nBusyPercent, D2GameServerStats * pStats) {
    char szLine[0x100];
    sprintf(szLine, "%d\t%d\t%d\t%d\t%d", nBusyPercent, pStats->nGames, pStats->nPlayers, pStats->nActiveMonsters, pStats->nInactiveMonsters);
    if (g_hPerfGraphDlg.bPerfLogEnabled) {
        PerfLogAppend(szLine);
    }
}

// 1.00 D2Server: 10008170
void __fastcall PerfLog_WriteRaw(char * pLine) {
    if (g_hPerfGraphDlg.bPerfLogEnabled) {
        PerfLogAppend(pLine);
    }
}

} // namespace D2Server::Core::Performance
