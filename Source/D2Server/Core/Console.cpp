// D2Server: the console dialog and its commands. No __FILE__ string survives for
// this unit; the name is the Ghidra program's.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.

#include "Source/D2Server/Core/Console.h"
#include "Source/D2Server/Core/Display.h"
#include "Source/D2Server/Core/WinMain.h"
#include "Source/D2Server/D2ServerImports.h"

namespace D2Server::Core::Console {

using namespace D2Server::Imports;

typedef int (__fastcall * ConsoleCommandHandler)(HWND hDlg, char * pszArgs);

#pragma pack(push, 1)
struct ConsoleCommand {
    char szName[0x100];             // 0x000
    ConsoleCommandHandler pfHandler; // 0x100 fastcall(window, arguments); 0 = say "unknown command"
    char szDescription[0x100];      // 0x104
};
#pragma pack(pop)
static_assert(sizeof(ConsoleCommand) == 0x204, "ConsoleCommand is 0x204 bytes");

static HWND HWND_10026c64 = nullptr;                                  // 1.00 D2Server: 10026c64

// The first row is the empty-line placeholder "?"; the search covers rows 0..8.
static ConsoleCommand g_aConsoleCommandTable[10] = {                  // 1.00 D2Server: 10022020
    { "?",             DisplayConsoleHelp,       "Display Console Commands" },
    { "help",          DisplayConsoleHelp,       "Display Console Commands" },
    { "profile",       ConsoleCmd_Profile,       "Restart Profiling" },
    { "globalmessage", ConsoleCmd_GlobalMessage, "Send message to all clients" },
    { "gm",            ConsoleCmd_GlobalMessage, "Send message to all clients" },
    { "localmessage",  ConsoleCmd_LocalMessage,  "Send message to one game" },
    { "perf",          ConsoleCmd_Perf,          "Performance Logging" },
    { "mem",           ConsoleCmd_Mem,           "Memory Dump" },
    { "lmem",          ConsoleCmd_LocalMem,      "Local Memory Dump" },
    { "",              nullptr,                  "" },
};

static const char s_szUnknownCommand[] = "ERROR: Unknown command \"";  // 1.00 D2Server: 100235e8
static const char g_szUnknownCmdSuffix[] = "\"";                      // 1.00 D2Server: 100235e4

// Splits a console line into the command word (lowercased) and the rest, after the
// blanks that follow it. 0x100 bytes each.
static void ConsoleSplitLine(const char * szLine, char * szCommand, char * szArgs) {
    const char szWhitespace[2] = { ' ', '\t' };
    int i = 0;
    szArgs[0] = '\0';
    for (; i < 0x100; ++i) {
        char c = szLine[i];
        if (c == szWhitespace[0] || c == szWhitespace[1] || c == '\0') {
            break;
        }
        szCommand[i] = c;
    }
    szCommand[i] = '\0';
    if (szLine[i] != '\0') {
        while (i < 0x100 && (szLine[i] == szWhitespace[0] || szLine[i] == szWhitespace[1])) {
            ++i;
        }
        int j = 0;
        while (szLine[i] != '\0' && i < 0x100) {
            szArgs[j++] = szLine[i++];
        }
        szArgs[j] = '\0';
    }
    Display::StrToLower(szCommand);
}

// 1.00 D2Server: 10002ce0
HWND __stdcall GetConsoleDialogHwnd() {
    return HWND_10026c64;
}

// 1.00 D2Server: 10002d00
BOOL __fastcall CreateConsoleDialog(HWND hParent) {
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongA(hParent, GWL_HINSTANCE);
    if (HWND_10026c64) {
        return FALSE;
    }
    HWND_10026c64 = CreateDialogParamA(hInstance, MAKEINTRESOURCEA(0x68), hParent, ConsoleDialogProc, 0);
    return HWND_10026c64 != nullptr;
}

// 1.00 D2Server: 10002d50
void __stdcall DestroyConsoleDialog() {
    SendMessageA(HWND_10026c64, WM_CLOSE, 0, 0);
}

// The console dialog: OK runs the line in the edit box (0x3ef), Cancel closes.
// 1.00 D2Server: 10002d80
INT_PTR __stdcall ConsoleDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char szLine[0x100];
    char szCommand[0x200];
    if (uMsg == WM_CLOSE) {
        HWND_10026c64 = nullptr;
        EndDialog(hDlg, 0);
        return TRUE;
    }
    if (uMsg == WM_INITDIALOG) {
        return TRUE;
    }
    if (uMsg != WM_COMMAND) {
        return FALSE;
    }
    if (wParam == IDOK) {
        if (SendDlgItemMessageA(hDlg, 0x3ef, WM_GETTEXT, 0x100, (LPARAM)szLine)) {
            ConsoleSplitLine(szLine, szCommand, szCommand + 0x100);
            int i = 0;
            for (ConsoleCommand * pCommand = g_aConsoleCommandTable; pCommand < g_aConsoleCommandTable + 9; ++pCommand, ++i) {
                if (strcmp(szCommand, pCommand->szName) == 0) {
                    if (g_aConsoleCommandTable[i].pfHandler(hDlg, szCommand + 0x100)) {
                        goto done;
                    }
                    WinMain::PostStatusMessage(0, szLine);
                    goto done;
                }
            }
            strcpy(szLine, s_szUnknownCommand);
            strcat(szLine, szCommand);
            strcat(szLine, g_szUnknownCmdSuffix);
            WinMain::PostStatusMessage(0, szLine);
        }
    done:
        szLine[0] = '\0';
        SendDlgItemMessageA(hDlg, 0x3ef, WM_SETTEXT, 0, (LPARAM)szLine);
        SetFocus(GetDlgItem(hDlg, 0x3ef));
        return TRUE;
    }
    if (wParam == IDCANCEL) {
        HWND_10026c64 = nullptr;
        EndDialog(hDlg, 0);
        return TRUE;
    }
    return TRUE;
}

// 1.00 D2Server: 100030f0
int __fastcall ConsoleCmd_Profile(HWND hDlg, char * pszArgs) {
    D2COMMON_10984();
    WinMain::PostStatusMessage(0, (char *)"Profiler Restarted");
    return 1;
}

// 1.00 D2Server: 10003120
int __fastcall ConsoleCmd_GlobalMessage(HWND hDlg, char * pszArgs) {
    char szMessage[0x100];
    strcpy(szMessage, "Diablo: ");
    strcat(szMessage, pszArgs);
    D2GAME_10021(0, 0, szMessage);
    WinMain::PostStatusMessage(0, szMessage);
    return 1;
}

// 1.00 D2Server: 100031e0
int __fastcall ConsoleCmd_LocalMessage(HWND hDlg, char * pszArgs) {
    char szMessage[0x100];
    char * szStatus;
    strcpy(szMessage, "Diablo: ");
    strcat(szMessage, pszArgs);
    GameInfoNode * pGame = Display::GetSelectedPlayerByAccountName();
    if (!pGame) {
        szStatus = (char *)"ERROR: No game selected to send to.";
    } else {
        szStatus = szMessage;
        D2GAME_10022(pGame->wGameId, szMessage);
    }
    WinMain::PostStatusMessage(0, szStatus);
    return 1;
}

// 1.00 D2Server: 100032b0
int __fastcall ConsoleCmd_Perf(HWND hDlg, char * pszSubCmd) {
    Display::StrToLower(pszSubCmd);
    if (*pszSubCmd == 'c') {
        FOG_10037();
        WinMain::PostStatusMessage(0, (char *)"Clear Performance Accumulators.");
        return 1;
    }
    if (*pszSubCmd != 'd') {
        WinMain::PostStatusMessage(0, (char *)"ERROR: Invalid performance param, use (c)lear or (d)ump.");
        return 1;
    }
    FOG_10038();
    WinMain::PostStatusMessage(0, (char *)"Dump Performance Accumulators.");
    return 1;
}

// The switch cases Ghidra lists as DispatchHandler_A0..A4 (10003355..1000339d) are
// this function's.
// 1.00 D2Server: 10003330
int __fastcall ConsoleCmd_Mem(HWND hDlg, char * pszSubCmd) {
    Display::StrToLower(pszSubCmd);
    switch (*pszSubCmd) {
    case 'c':
        FOG_GetSavePath(0, 2);
        return 1;
    case 'l':
        FOG_GetSavePath(0, 1);
        return 1;
    case 'n':
        FOG_GetSavePath(0, 3);
        return 1;
    case 'p':
        FOG_GetSavePath(0, 0);
        return 1;
    }
    WinMain::PostStatusMessage(0, (char *)"ERROR: Invalid memory param, use (p)ool, (l)ine, (c)lear, or (n)um.");
    return 1;
}

// The switch cases Ghidra lists as DispatchHandler_B0..B4 (10003443..1000348b) are
// this function's.
// 1.00 D2Server: 10003410
int __fastcall ConsoleCmd_LocalMem(HWND hDlg, char * pszSubCmd) {
    Display::StrToLower(pszSubCmd);
    GameInfoNode * pGame = Display::GetSelectedPlayerByAccountName();
    if (!pGame) {
        WinMain::PostStatusMessage(0, (char *)"ERROR: No game selected to memory dump.");
        return 0;
    }
    int nPool = *(int32_t *)((uint8_t *)pGame + 0x8C);
    switch (*pszSubCmd) {
    case 'c':
        FOG_GetSavePath(nPool, 2);
        return 1;
    case 'l':
        FOG_GetSavePath(nPool, 1);
        return 1;
    case 'n':
        FOG_GetSavePath(nPool, 3);
        return 1;
    case 'p':
        FOG_GetSavePath(nPool, 0);
        return 1;
    }
    WinMain::PostStatusMessage(0, (char *)"ERROR: Invalid memory param, use (p)ool, (l)ine, (c)lear, or (n)um.");
    return 1;
}

// The main window's console line (Display's ConsoleInputProc) runs commands here; an
// unknown command is echoed back in place, in pszCommandLine.
// 1.00 D2Server: 10003520
void __fastcall DispatchConsoleCommand(HWND hWnd, char * pszCommandLine) {
    char szCommand[0x200];
    ConsoleSplitLine(pszCommandLine, szCommand, szCommand + 0x100);
    int i = 0;
    for (ConsoleCommand * pCommand = g_aConsoleCommandTable; pCommand < g_aConsoleCommandTable + 9; ++pCommand, ++i) {
        if (strcmp(szCommand, pCommand->szName) == 0) {
            if (g_aConsoleCommandTable[i].pfHandler(hWnd, szCommand + 0x100)) {
                return;
            }
            WinMain::PostStatusMessage(0, pszCommandLine);
            return;
        }
    }
    strcpy(pszCommandLine, s_szUnknownCommand);
    strcat(pszCommandLine, szCommand);
    strcat(pszCommandLine, g_szUnknownCmdSuffix);
    WinMain::PostStatusMessage(0, pszCommandLine);
}

// "help": the command list, into the dialog's list (0x3ee, kept to 64 lines) and the
// status log.
// 1.00 D2Server: 10003740
int __fastcall DisplayConsoleHelp(HWND hDlg, char * pszArgs) {
    char szLine[0x100];
    if (HWND_10026c64) {
        if (SendDlgItemMessageA(HWND_10026c64, 0x3ee, LB_GETCOUNT, 0, 0) > 0x40) {
            SendDlgItemMessageA(HWND_10026c64, 0x3ee, LB_DELETESTRING, 0, 0);
        }
        SendDlgItemMessageA(HWND_10026c64, 0x3ee, LB_ADDSTRING, 0, (LPARAM)"Console Commands:");
        LRESULT nCount = SendDlgItemMessageA(HWND_10026c64, 0x3ee, LB_GETCOUNT, 0, 0);
        SendDlgItemMessageA(HWND_10026c64, 0x3ee, LB_SETCURSEL, nCount - 1, 0);
    }
    for (ConsoleCommand * pCommand = g_aConsoleCommandTable; pCommand < g_aConsoleCommandTable + 9; ++pCommand) {
        strcpy(szLine, "    ");
        strcat(szLine, pCommand->szName);
        strcat(szLine, " - ");
        strcat(szLine, pCommand->szDescription);
        WinMain::PostStatusMessage(0, szLine);
    }
    return 1;
}

} // namespace D2Server::Core::Console
