// unknown_startup.cpp - the Mac shell's start-up dialogs and launch: the about box, the
// modal-dialog bracket every shell dialog runs in, the game-disc prompt, and the command line
// (the process's own or the one saved in the registry) handed on to ApplicationMain.
//
// 1.14d Intel Mac (DiabloII_macho) 0019a59a..0019b290. No binary records this unit's name.
// It defines the globals it addresses directly: the configuration 005c8660, the instance
// 005c865c and the mode 005c8a30 (the units after it reach them through non-lazy pointers),
// the modal state 00536af4.. and the tokenizer state 00536b1c..; its last function is the
// file-local tokenizer 0019b1ec (register argument), and three bytes of padding then bring the
// next unit to a 16-byte boundary. See Source/MacSpecific/README.md.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC` (see
// Source/MacSpecific/README.md).
#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <pthread.h>
#include <stdlib.h>

#include <QuickTime/QuickTime.h>
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <string.h>

#include "Source/MacSpecific/Source/MacShell/QuickDrawMac.h"

#include "Source/StormMac/H/stpl.h"

typedef int BOOL;
// 1.14d mac: 0001eb1c
char* p2cstr(unsigned char* pszString);
// 1.14d mac: 0001eafe
unsigned char* c2pstr(char* pszString);
// 1.14d mac: 00032bcd
void DirectSound_SetMinVolumeDelay(uint32_t nMilliseconds);
// 1.14d mac: 00034740
void GetDialogItemRect(DialogRef pDialog, DialogItemIndex nItem, Rect* pRect);
typedef Boolean (*MacDialogFilterProc)(DialogRef pDialog, EventRecord* pEvent, DialogItemIndex* pnItemHit);
// What Storm's modal dialog loop keeps while a dialog is up: the menus it disables and their
// saved state (Storm's DlgMenuState, Source/StormMac/unknown_dialog.cpp); only the part the
// shell fills is named. The 32-bit fields give the record 4-byte alignment, which is why
// clang keeps the item stores of MAC_BeginModalDialog as separate 16-bit stores.
struct MacModalState {
    uint8_t unk00[8];
    int16_t nMenuID;          // 0x08
    uint16_t nMenuItems;      // 0x0A
    int16_t nEditMenuID;      // 0x0C
    int16_t n0E;              // 0x0E
    int16_t nCutItem;         // 0x10
    int16_t nCopyItem;        // 0x12
    int16_t nPasteItem;       // 0x14
    int16_t nClearItem;       // 0x16
    int16_t nSelectAllItem;   // 0x18
    uint16_t wSavedBits;      // 0x1A
    uint32_t dwEditSavedBits; // 0x1C
    WindowRef pWindow;        // 0x20
};
static_assert(sizeof(void*) != 4 || sizeof(MacModalState) == 0x24, "MacModalState");
// 1.14d mac: 0003555c
void Dlg_DisableMenusForModal(DialogRef pDialog, int nArg, MacModalState* pState);
// 1.14d mac: 00035ef1
void Dlg_ModalDialog(DialogRef pDialog, MacModalState* pState, MacDialogFilterProc pfnFilter,
                     DialogItemIndex* pnItemHit);
// 1.14d mac: 000358fe
void Dlg_RestoreMenus(DialogRef pDialog, MacModalState* pState);
// 1.14d mac: 0019f2b6
void FLAMINGLOGO_Init(DialogRef pDialog, int nItem);
// 1.14d mac: 0019f551
void FLAMINGLOGO_Shutdown(void);
// 1.14d mac: 0019f61c
void FLAMINGLOGO_DrawFrame(ControlRef pControl);
// 1.14d mac: 0004b543
void D2WINMAIN_SetCursorVisible(BOOL bVisible);
namespace D2Client {
namespace Cursor {
// 1.14d mac: 0006328d
void CURSOR_SetOverrideValue(int nValue);
}
}
// 1.14d mac: 00042c23
BOOL MPQ_LoadAllMediaMpqFiles(void* pfnUserAction1, void* pfnUserAction2, int nFlags, void* pIni);
// 1.14d mac: 00042ab4
void MPQ_UnloadAllMediaMpqFiles(void);
// 1.14d mac: 002edc4a
BOOL D2CheckExpansionGame(void);
// 1.14d mac: 002b692a
void ARCHIVE_UnmountCdVolumes(void);
// The application's procedure table (the slots after the shell's own: 0x6c, 0x70 and 0x74 bracket
// a dialog the way the shell wants the app told).
struct MacAppProcs {
    void (*apfn[64])(void);
};
// 1.14d mac: 00397abc
// The pointer to the display driver table (g_pDisplayDriver, StormMac/MacDisplayDriver.cpp, 00397abc).
extern MacAppProcs* g_pMacAppProcs __asm__("_g_pDisplayDriver");
// ---- the SReg, Win32Mac and D2gfx entry points the shell calls ----

// 1.14d mac: 0001ccd5
int SStrPrintf(char* szDest, size_t nDestSize, const char* szFormat, ...);
// File-local (its argument comes in ecx) yet called out of line by its one caller; clang-425
// inlines a one-caller static unless told not to.
static __attribute__((noinline)) char* FUN_0019b1ec(char* szCmdLine);
// 1.14d mac: 0003a8e2
int SRegLoadString(char* szSubKey, char* szValueName, int nFlags, char* szOutBuf, uint32_t dwOutBufSize);
// 1.14d mac: 0003a9a0
int SRegLoadValue(char* szSubKey, char* szValueName, int nFlags, int32_t* pnValue);
// 1.14d mac: 0003abad
int SRegSaveString(char* szSubKey, const char* szValueName, int nFlags, char* szValue);
// 1.14d mac: 0003acba
int SRegSaveValue(char* szSubKey, const char* szValueName, int nFlags, int32_t nValue);
// 1.14d mac: 002eed78
void FOG_InitErrorManager(const char* szProduct, const char* szVersion, int nFlags);
// 1.14d mac: 0002ab8e
char* GetCommandLine(void);
// 1.14d mac: 0019ebbe
int ApplicationMain(int nInstance, void* pDisplayState, int nMode);
// Options 2 and 3 are eight-byte values D2gfx takes by address; their fields are unknown.
struct MacDisplayOption {
    int32_t n00;
    int32_t n04;
};
// 1.14d mac: 002b603a
void MACGFX_EnumerateDisplayModes(int* pnDisplayModule, int* pnOption4, int* pnOption5, int* pnOption0,
                                  MacDisplayOption* pOption2, MacDisplayOption* pOption3);
// The configuration Game/Main.cpp parses the command line into (its D2IniConfigStrc, 0x3CD
// bytes); only the bytes the shell uses are laid out.
struct MacDisplayState {
    uint8_t unk00[8];
    bool bFullScreen;
    uint8_t unk09;
    uint8_t b0A;
    uint8_t b0B;
    uint8_t b0C;
    uint8_t b0D;
    uint8_t unk0E[0x211 - 0x0E];
    void* pfnHook __attribute__((packed));
    uint8_t unk215[0x3CD - 0x215];
};
static_assert(sizeof(void*) != 4 || offsetof(MacDisplayState, pfnHook) == 0x211, "MacDisplayState::pfnHook");
static_assert(sizeof(void*) != 4 || sizeof(MacDisplayState) == 0x3CD, "MacDisplayState");
extern MacDisplayState g_tMacDisplayState;
void MAC_BeginModalDialog(DialogRef pDialog, int nArg);
void MAC_ModalDialog(DialogRef pDialog, MacDialogFilterProc pfnFilter, DialogItemIndex* pnItemHit);
void MAC_EndModalDialog(DialogRef pDialog);
Boolean MAC_CanLoadMediaMpqs(void);
Boolean FUN_0019a89c(DialogRef pDialog, EventRecord* pEvent, DialogItemIndex* pnItemHit);
Boolean MAC_GameDiscDialogFilterProc(DialogRef pDialog, EventRecord* pEvent, DialogItemIndex* pnItemHit);

// 1.14d mac: 00536af4
// The modal-dialog state shared by every dialog the shell shows.
static MacModalState s_tModalState;
// 1.14d mac: 00536b18
// The tick of the last disc check.
static uint32_t s_dwLastDiscCheck;

// 1.14d mac: 0019a59a
// Shows DLOG 3400, rewrites the "2000" of its copyright line to "2000 - 2016", shows the
// flaming logo in item 2 and runs the dialog modally until item 1, then releases it.
void MAC_ShowAboutDialog(void) {
    DialogRef pDialog = GetNewDialog(3400, NULL, (WindowRef)-1);
    if (pDialog == NULL) {
        return;
    }
    SetThemeCursor(kThemeArrowCursor);
    SetDialogTracksCursor(pDialog, true);
    SetDialogDefaultItem(pDialog, 1);
    SetDialogCancelItem(pDialog, 1);
    DialogItemType nItemType;
    Handle hItem;
    Rect rcItem;
    GetDialogItem(pDialog, 3, &nItemType, &hItem, &rcItem);
    if ((nItemType & 8) && hItem != NULL) {
        Str255 szItemText;
        char szNewText[256];
        GetDialogItemText(hItem, szItemText);
        p2cstr(szItemText);
        char* pszYear = strstr((char*)szItemText, "2000");
        if (pszYear != NULL) {
            *pszYear = '\0';
            sprintf(szNewText, "%s%s%s", szItemText, "2000 - 2016", pszYear + 4);
            c2pstr(szNewText);
            SetDialogItemText(hItem, (ConstStr255Param)szNewText);
        }
    }
    FLAMINGLOGO_Init(pDialog, 2);
    GrafPtr pSavedPort;
    GetPort(&pSavedPort);
    SetPortWindowPort(GetDialogWindow(pDialog));
    ShowWindow(GetDialogWindow(pDialog));
    D2WINMAIN_SetCursorVisible(0);
    D2Client::Cursor::CURSOR_SetOverrideValue(0);
    MAC_BeginModalDialog(pDialog, 0);
    DialogItemIndex nItemHit;
    do {
        MAC_ModalDialog(pDialog, FUN_0019a89c, &nItemHit);
    } while (nItemHit != 1);
    MAC_EndModalDialog(pDialog);
    FLAMINGLOGO_Shutdown();
    DisposeDialog(pDialog);
    SetPort(pSavedPort);
    Boolean bResLoad = LMGetResLoad();
    SetResLoad(false);
    Handle hRes;
    if ((hRes = GetResource('DLOG', 3400)) != NULL) ReleaseResource(hRes);
    if ((hRes = GetResource('DITL', 3400)) != NULL) ReleaseResource(hRes);
    if ((hRes = GetResource('dlgx', 3400)) != NULL) ReleaseResource(hRes);
    if ((hRes = GetResource('dftb', 3400)) != NULL) ReleaseResource(hRes);
    if ((hRes = GetResource('CTRL', 3203)) != NULL) ReleaseResource(hRes);
    if (bResLoad) {
        SetResLoad(true);
    }
    D2WINMAIN_SetCursorVisible(1);
    D2Client::Cursor::CURSOR_SetOverrideValue(1);
    g_pMacAppProcs->apfn[27]();
}

// 1.14d mac: 0019a89c
// The about box's filter: the next logo frame into item 2, and a Storm timing call; the event
// is never taken.
Boolean FUN_0019a89c(DialogRef pDialog, EventRecord* pEvent, DialogItemIndex* pnItemHit) {
    *pnItemHit = 0;
    ControlRef pControl = NULL;
    GetDialogItemAsControl(pDialog, 2, &pControl);
    FLAMINGLOGO_DrawFrame(pControl);
    DirectSound_SetMinVolumeDelay(1000);
    return 0;
}

// 1.14d mac: 0019a8e9
// Fills the modal state, greys the shell's menus and tells Storm.
void MAC_BeginModalDialog(DialogRef pDialog, int nArg) {
    s_tModalState.nMenuID = 0x81;
    s_tModalState.nMenuItems = 2;
    s_tModalState.nEditMenuID = 0x83;
    s_tModalState.n0E = 1;
    s_tModalState.nCutItem = 3;
    s_tModalState.nCopyItem = 4;
    s_tModalState.nPasteItem = 5;
    s_tModalState.nClearItem = 6;
    s_tModalState.nSelectAllItem = 8;
    MenuRef hMenu;
    if ((hMenu = GetMenuHandle(0x81)) != NULL) DisableMenuItem(hMenu, 0);
    if ((hMenu = GetMenuHandle(0x82)) != NULL) DisableMenuItem(hMenu, 0);
    if ((hMenu = GetMenuHandle(0x83)) != NULL) DisableMenuItem(hMenu, 0);
    if ((hMenu = GetMenuHandle(0x84)) != NULL) DisableMenuItem(hMenu, 0);
    DrawMenuBar();
    Dlg_DisableMenusForModal(pDialog, nArg, &s_tModalState);
}

// 1.14d mac: 0019a9f2
void MAC_ModalDialog(DialogRef pDialog, MacDialogFilterProc pfnFilter, DialogItemIndex* pnItemHit) {
    Dlg_ModalDialog(pDialog, &s_tModalState, pfnFilter, pnItemHit);
}

// 1.14d mac: 0019aa26
void MAC_EndModalDialog(DialogRef pDialog) {
    Dlg_RestoreMenus(pDialog, &s_tModalState);
    MenuRef hMenu;
    if ((hMenu = GetMenuHandle(0x81)) != NULL) EnableMenuItem(hMenu, 0);
    if ((hMenu = GetMenuHandle(0x82)) != NULL) EnableMenuItem(hMenu, 0);
    if ((hMenu = GetMenuHandle(0x83)) != NULL) EnableMenuItem(hMenu, 0);
    if ((hMenu = GetMenuHandle(0x84)) != NULL) EnableMenuItem(hMenu, 0);
    DrawMenuBar();
}

// 1.14d mac: 0019aad0
// When the media MPQs cannot be loaded, asks for the game disc (DLOG 301, or 302 for the
// expansion) until Cancel (1) or the disc is found (4). True when the disc is there.
bool MAC_PromptForGameDisc(void) {
    bool bResult = true;
    if (!MAC_CanLoadMediaMpqs()) {
        g_pMacAppProcs->apfn[28]();
        BOOL bExpansion = D2CheckExpansionGame();
        DialogRef pDialog = GetNewDialog(301 + (bExpansion != 0), NULL, (WindowRef)-1);
        GrafPtr pSavedPort;
        GetPort(&pSavedPort);
        SetDialogDefaultItem(pDialog, 0);
        SetDialogCancelItem(pDialog, 1);
        ShowWindow(GetDialogWindow(pDialog));
        SetPortWindowPort(GetDialogWindow(pDialog));
        MAC_BeginModalDialog(pDialog, 0);
        DialogItemIndex nItemHit;
        do {
            Dlg_ModalDialog(pDialog, &s_tModalState, (MacDialogFilterProc)MAC_GameDiscDialogFilterProc, &nItemHit);
            if (nItemHit == 1) {
                break;
            }
        } while (nItemHit != 4);
        MAC_EndModalDialog(pDialog);
        DisposeDialog(pDialog);
        SetPort(pSavedPort);
        Boolean bResLoad = LMGetResLoad();
        SetResLoad(false);
        Handle hRes;
        if ((hRes = GetResource('DLOG', 301)) != NULL) ReleaseResource(hRes);
        if ((hRes = GetResource('DITL', 301)) != NULL) ReleaseResource(hRes);
        if ((hRes = GetResource('dlgx', 301)) != NULL) ReleaseResource(hRes);
        if ((hRes = GetResource('DLOG', 302)) != NULL) ReleaseResource(hRes);
        if ((hRes = GetResource('DITL', 302)) != NULL) ReleaseResource(hRes);
        if ((hRes = GetResource('dlgx', 302)) != NULL) ReleaseResource(hRes);
        if (bResLoad) {
            SetResLoad(true);
        }
        short nHit = nItemHit;
        g_pMacAppProcs->apfn[29]();
        bResult = nHit == 4;
    }
    return bResult;
}

// 1.14d mac: 0019acd3
// Whether the media MPQs load (they are unloaded again).
Boolean MAC_CanLoadMediaMpqs(void) {
    if (MPQ_LoadAllMediaMpqFiles(NULL, NULL, 1, NULL)) {
        MPQ_UnloadAllMediaMpqFiles();
        return 1;
    }
    return 0;
}

// 1.14d mac: 0019ad14
// The disc prompt's filter: every 30 idle ticks and on disk events it looks for the disc again
// and, with the game in front, sets item 4; on updates it draws the CD icon.
Boolean MAC_GameDiscDialogFilterProc(DialogRef pDialog, EventRecord* pEvent, DialogItemIndex* pnItemHit) {
    bool bDiskEvent = false;
    bool bIdleTimeout = false;
    if (pEvent->what != nullEvent) {
        if (pEvent->what == diskEvt) {
            bDiskEvent = pEvent->message < 0x10000;
        }
    } else {
        bIdleTimeout = pEvent->when > s_dwLastDiscCheck + 30;
    }
    if (bDiskEvent || bIdleTimeout) {
        s_dwLastDiscCheck = pEvent->when;
        if (MAC_CanLoadMediaMpqs()) {
            ProcessSerialNumber psnCurrent;
            ProcessSerialNumber psnFront;
            Boolean bSameProcess;
            GetCurrentProcess(&psnCurrent);
            GetFrontProcess(&psnFront);
            SameProcess(&psnCurrent, &psnFront, &bSameProcess);
            if (bSameProcess) {
                *pnItemHit = 4;
                return 1;
            }
            return 0;
        }
        ARCHIVE_UnmountCdVolumes();
        return 0;
    }
    if (pEvent->what == updateEvt) {
        if (pEvent->message != (UInt32)GetDialogWindow(pDialog)) {
            goto other;
        }
    } else {
    other:
        if (pEvent->what != activateEvt && (pEvent->what != osEvt || (pEvent->message & 0xFF000000) != 0x01000000)) {
            return 0;
        }
    }
    Rect rcIcon;
    GetDialogItemRect(pDialog, 2, &rcIcon);
    EraseRect(&rcIcon);
    Boolean bHilited = IsWindowHilited(GetDialogWindow(pDialog));
    IconRef hIcon;
    if (GetIconRef(kOnSystemDisk, kSystemIconsCreator, kGenericCDROMIcon, &hIcon) == noErr) {
        PlotIconRef(&rcIcon, kAlignNone, bHilited ? kTransformNone : kTransformDisabled, kIconServicesNormalUsageFlag, hIcon);
        ReleaseIconRef(hIcon);
    }
    return 0;
}

// 1.14d mac: 005c865c
// The application instance ApplicationMain was handed.
int g_nMacAppInstance;
// 1.14d mac: 005c8660
// The configuration the command line is parsed into (Game/Main.cpp's D2IniConfigStrc).
MacDisplayState g_tMacDisplayState;
// 1.14d mac: 005c8a30
// The application mode ("-client", "-server", ...) the command line named; the engine default until then.
int g_nMacAppMode;
// 1.14d mac: 00536b1c
static char* s_pCmdLineScan;
// 1.14d mac: 00536b20
static uint32_t s_nCmdLineLeft;
// 1.14d mac: 00536b24
static char s_szCmdLine[1024];
// 1.14d mac: 003e0e9c
const char* gApplicationModeCommandLineArgumentArray[6] = {"modstate0", "client", "server", "multiplayer", "launcher", "expand"};

// 1.14d mac: 0019e8a1
// Game/Main.cpp: clears the configuration, initialises the ignore list and parses the switches.
void MISC_InitD2IniAndIgnorelist(MacDisplayState* pD2IniData, const char* szCommandLine);
// 1.14d mac: 0019cc81
Boolean FUN_0019cc81(int nOption0, int nFullScreen);

// 1.14d mac: 0019ae9c
// Builds the command line (the process's own, else the one saved in the registry, plus
// -skiptobnet), picks the application mode and hands over to ApplicationMain.
int MAC_ParseCommandLineAndRun(int nInstance, int nUnused1, int nUnused2, int nUnused3) {
    char szVersion[260];
    char szCmdBuf[2048];
    char szSavedCmdLine[512];
    int nOption0;
    int32_t bUseCmdLine;
    g_nMacAppInstance = nInstance;
    SStrPrintf(szVersion, 0x104, "v%d.%02d", 1, 14);
    FOG_InitErrorManager("Diablo II", szVersion, 1);
    bUseCmdLine = 0;
    char* szCmdLine = GetCommandLine();
    char* pszRest;
    if (szCmdLine != NULL && (pszRest = FUN_0019b1ec(szCmdLine)) != NULL && (szCmdLine = pszRest, *pszRest != '\0')) {
        snprintf(szSavedCmdLine, 0x200, "%s -skiptobnet", pszRest);
        szSavedCmdLine[511] = '\0';
        SRegSaveString((char*)"Diablo II", "CmdLine", 0, szSavedCmdLine);
    } else {
        szCmdLine = NULL;
        SRegLoadValue((char*)"Diablo II", (char*)"UseCmdLine", 0, &bUseCmdLine);
        if (bUseCmdLine == 0) {
            strcpy(szCmdBuf, "-skiptobnet");
            SRegSaveString((char*)"Diablo II", "CmdLine", 0, szCmdBuf);
        } else {
            szCmdLine = s_szCmdLine;
            SRegLoadString((char*)"Diablo II", (char*)"CmdLine", 0, s_szCmdLine, 0x400);
        }
    }
    bUseCmdLine = 0;
    SRegSaveValue((char*)"Diablo II", "UseCmdLine", 0, 0);
    int nMode = g_nMacAppMode;
    strcpy(szCmdBuf, szCmdLine);
    for (char* szArg = strtok(szCmdBuf, "-"); szArg != NULL; szArg = strtok(NULL, "-")) {
        for (int i = 0; i != 6; i++) {
            const char* szModeArg = gApplicationModeCommandLineArgumentArray[i];
            if (strncmp(szArg, szModeArg, strlen(szModeArg)) == 0) {
                nMode = i;
            }
        }
    }
    MISC_InitD2IniAndIgnorelist(&g_tMacDisplayState, szCmdLine);
    MACGFX_EnumerateDisplayModes(NULL, NULL, NULL, &nOption0, NULL, NULL);
    int nResult = 0;
    if (FUN_0019cc81(nOption0, g_tMacDisplayState.bFullScreen)) {
        nResult = ApplicationMain(nInstance, &g_tMacDisplayState, nMode);
    }
    return nResult;
}

// 1.14d mac: 0019b1ec
// A tokenizer that keeps its place in the globals above; a non-NULL string starts it over.
// Skips blanks, cuts one token (up to a quote when it starts with one, else up to a blank) and
// returns what follows it, or NULL when nothing is left.
static char* FUN_0019b1ec(char* szCmdLine) {
    if (szCmdLine != NULL) {
        s_pCmdLineScan = szCmdLine;
        s_nCmdLineLeft = strlen(szCmdLine);
    }
    char* pszRest = NULL;
    if (0 < (int)s_nCmdLineLeft) {
        char ch;
        for (;;) {
            ch = *s_pCmdLineScan;
            if (ch != '\t' && ch != ' ') {
                break;
            }
            s_pCmdLineScan++;
            s_nCmdLineLeft--;
        }
        const char* szDelimiters = (ch == '"') ? "\"" : " \t";
        char* pszToken = strtok(s_pCmdLineScan, szDelimiters);
        char* pszScan = s_pCmdLineScan;
        if (pszToken != NULL) {
            size_t nTokenLen = strlen(s_pCmdLineScan);
            pszRest = pszScan + nTokenLen + 1;
            s_pCmdLineScan = pszRest;
            s_nCmdLineLeft -= nTokenLen + 1;
        }
    }
    return pszRest;
}

#endif  // D2_OS_MAC
