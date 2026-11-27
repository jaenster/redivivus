// GameSetup.cpp - the Mac shell's game set-up: start-up checks, the CD-key and display
// dialogs, the display-mode switch between windowed and full screen.
//
// 1.14d Intel Mac (DiabloII_macho). Pinned by the 1.14d __FILE__
// ".../Diablo2/Source/DiabloAll/../MacSpecific/Source/Mac Shell/GameSetup.cpp" (0031a4da),
// passed to SMemAlloc/SMemFree by GAMESETUP_Alloc (line 463) and GAMESETUP_Free (line 486),
// and by the video-mode globals 003b8d7c and 00536c50, which the functions below address
// directly (a global another unit defines would go through a non-lazy pointer). Only the
// run 0019c760..0019cc81 and 0019cf50 are reconstructed so far; the file starts earlier (GAMESETUP_Alloc
// is line 463) and where it ends is not settled.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC` (see
// Source/MacSpecific/README.md).
#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <pthread.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>

#include "Source/MacSpecific/Source/MacShell/QuickDrawMac.h"

typedef int BOOL;

// 1.14d mac: 0001eb1c
char* p2cstr(unsigned char* pszString);
// 1.14d mac: 0001eafe
unsigned char* c2pstr(char* pszString);
// 1.14d mac: 00032bcd
void FUN_00032bcd(uint32_t nMilliseconds);
// 1.14d mac: 00034740
void GetDialogItemRect(DialogRef pDialog, DialogItemIndex nItem, Rect* pRect);

typedef Boolean (*MacDialogFilterProc)(DialogRef pDialog, EventRecord* pEvent, DialogItemIndex* pnItemHit);

// What Storm's modal dialog loop keeps while a dialog is up (0x10 bytes of state and the menu
// items it disables); only the part GAMESETUP fills is laid out.
struct MacModalState {
    uint8_t unk00[8];
    volatile int16_t nMenu1;  // 0x08
    volatile int16_t nItem1;
    volatile int16_t nMenu3;
    volatile int16_t nItem3;
    volatile int16_t nItem4;
    volatile int16_t nItem5;
    volatile int16_t nItem6;
    volatile int16_t nItem7;
    volatile int16_t nItem8;
    uint8_t unk1A[10];
};
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
// 1.14d mac: 0006328d
void FUN_0006328d(BOOL bEnable);
// 1.14d mac: 00042c23
BOOL MPQ_LoadAllMediaMpqFiles(void* pfnUserAction1, void* pfnUserAction2, int nFlags, void* pIni);
// 1.14d mac: 00042ab4
void MPQ_UnloadAllMediaMpqFiles(void);
// 1.14d mac: 002edc4a
BOOL D2CheckExpansionGame(void);
// 1.14d mac: 002b692a
void FUN_002b692a(void);

// The application's procedure table (the slots after the shell's own: 0x6c, 0x70 and 0x74 bracket
// a dialog the way the shell wants the app told).
struct MacAppProcs {
    void (*apfn[64])(void);
};
// 1.14d mac: 00397abc
extern MacAppProcs* g_pMacAppProcs;
// 1.14d mac: 000298b2
void MSG_PostKeyMessage(int bKeyDown, uint8_t byKeyCode, uint8_t byChar);
// 1.14d mac: 0002a16f
void* GetModuleHandle(const char* szModuleName);
// 1.14d mac: 000312bf
BOOL InvalidateRect(void* hWnd, const void* pRect, BOOL bErase);

// ---- the SReg, Win32Mac and D2gfx entry points the shell calls ----

// 1.14d mac: 0001ccd5
int SStrPrintf(char* szDest, size_t nDestSize, const char* szFormat, ...);
__attribute__((noinline)) void MAC_DSpShutdown(void);
static char* FUN_0019b1ec(char* szCmdLine);
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
// 1.14d mac: 0002a40b
void PROCESS_SetExitHandler(void (*pfnExitHandler)(void));
// 1.14d mac: 0002c02c
uint32_t GetCurrentThreadId(void);
// 1.14d mac: 00031060
BOOL GetWindowRect(int hWnd, int* pRect);
// 1.14d mac: 000297c3
void FUN_000297c3(uint32_t nMessage, uint32_t nWParam, uint32_t nLParam);
// 1.14d mac: 002b5b6d
void* GetHwnd(void);
// 1.14d mac: 002b5b81
int D2GFX_GetScreenSize(int* pnWidth, int* pnHeight);
// 1.14d mac: 002b5da4
// Hands a Carbon event to the renderer; 0 without one.
uint8_t FUN_002b5da4(EventRecord* pEvent);
// 1.14d mac: 002b5dc3
// Converts a point to the renderer's coordinates; 0 without a renderer.
uint8_t FUN_002b5dc3(Point* pPoint);
// 1.14d mac: 0019ebbe
int ApplicationMain(int nInstance, void* pDisplayState, int nMode);

#include "Source/StormMac/H/stpl.h"

extern "C" OSStatus DSpShutdown(void);

// 1.14d mac: 0003ad51
int SRegDeleteValue(const char* szKeyName, const char* szValueName, uint32_t dwFlags);
// 1.14d mac: 00034519
void fp_fatal(const char* szFormat, ...);
// 1.14d mac: 002b4d4f
int D2GFX_GetRendererMode(void);
// 1.14d mac: 002b4b3c
// Option 10 reads back whether the renderer is busy; 2 and 3 take a pointer on the Mac.
void D2GFX_SetOption(int nOption, intptr_t nValue);
// 1.14d mac: 002b4b72
int D2GFX_SwitchModule(int nVideoMode, int bFullScreen, int nStage);
// 1.14d mac: 002b4d60
// The display state D2gfx is in: 1 full screen, 0 windowed.
int FUN_002b4d60(void);
// 1.14d mac: 002b5c1c
Boolean FUN_002b5c1c(void);
// Options 2 and 3 are eight-byte values D2gfx takes by address; their fields are unknown.
struct MacDisplayOption {
    int32_t n00;
    int32_t n04;
};
// 1.14d mac: 002b603a
void MACGFX_EnumerateDisplayModes(int* pnDisplayModule, int* pnOption4, int* pnOption5, int* pnOption0,
                                  MacDisplayOption* pOption2, MacDisplayOption* pOption3);
// 1.14d mac: 0005e6ac
void CLIENT_SetRendererMode(int nVideoMode, int bFullScreen);
// 1.14d mac: 0004d278
void D2WINSMACK_SetKeepVideoFile(void);

// The display flags D2gfx and the shell share (defined in another unit; only the bytes
// used here are laid out).
struct MacDisplayState {
    uint8_t unk00[8];
    bool bFullScreen;  // 0x08
    uint8_t unk09;
    uint8_t b0A;  // 0x0A..0x0D reset on every switch
    uint8_t b0B;
    uint8_t b0C;
    uint8_t b0D;
};
// 1.14d mac: 005c8660
extern MacDisplayState g_tMacDisplayState;
// 1.14d mac: 005c8a38
// Set when the display did not end up in the state asked for.
extern int g_bMacDisplayMismatch;

// 1.14d mac: 003b8d7c
// The D2gfx video mode (renderer module) the Mac shell runs.
int g_nMacVideoMode = 2;
// 1.14d mac: 003b8d78
uint32_t g_nMacMainThread = (uint32_t)-1;
// 1.14d mac: 00536c50
// Set while FUN_0019cf50 runs: the switch then does not wait for the renderer to go idle.
static bool s_bSkipRendererIdleWait;

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
    FUN_0006328d(0);
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
    FUN_0006328d(1);
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
    FUN_00032bcd(1000);
    return 0;
}

// 1.14d mac: 0019a8e9
// Fills the modal state, greys the shell's menus and tells Storm.
void MAC_BeginModalDialog(DialogRef pDialog, int nArg) {
    s_tModalState.nMenu1 = 0x81;
    s_tModalState.nItem1 = 2;
    s_tModalState.nMenu3 = 0x83;
    s_tModalState.nItem3 = 1;
    s_tModalState.nItem4 = 3;
    s_tModalState.nItem5 = 4;
    s_tModalState.nItem6 = 5;
    s_tModalState.nItem7 = 6;
    s_tModalState.nItem8 = 8;
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
    bool bIdleTimeout = false;
    bool bDiskEvent;
    if (pEvent->what == nullEvent) {
        bIdleTimeout = s_dwLastDiscCheck + 30 < pEvent->when;
        bDiskEvent = false;
    } else if (pEvent->what == diskEvt) {
        bDiskEvent = pEvent->message < 0x10000;
        bIdleTimeout = false;
    } else {
        bDiskEvent = false;
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
        FUN_002b692a();
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

// The shell's input state: the Carbon handlers and the Storm callbacks work on these.
// 1.14d mac: 005c865c
// The application instance ApplicationMain was handed.
static int g_nMacAppInstance;
// 1.14d mac: 005c8a30
// The application mode ("-client", "-server", ...) the command line named; the engine default until then.
int g_nMacAppMode;
// 1.14d mac: 005c8a34
uint8_t g_bMacQuitRequested;
// 1.14d mac: 005c8a35
uint8_t g_bMacCapsLock;
// 1.14d mac: 005c8a36
uint16_t g_wMacModifiers;
// 1.14d mac: 00536b1c
static char* s_pCmdLineScan;
// 1.14d mac: 00536b20
static uint32_t s_nCmdLineLeft;
// 1.14d mac: 00536b24
static char s_szCmdLine[1024];
// 1.14d mac: 00536c28
// Set while the Carbon handlers may post: mouse and key messages, activation, updates.
static uint8_t s_bEventsEnabled;
// 1.14d mac: 00536c29
static bool s_bExiting;
// 1.14d mac: 00536c2c
static OSStatus (*s_pfnMouseHandler)(EventHandlerCallRef, EventRef, void*);
// 1.14d mac: 00536c37
// Pressed state of the mouse buttons 0..5 (1 left, 2 right, 3 middle, 4 and 5 the extra ones).
static bool s_abMouseButtons[6];
// 1.14d mac: 00536c3d
// Set while the right button is emulated by a command-click.
static bool s_bCommandClick;
// 1.14d mac: 003e0e9c
extern const char* gApplicationModeCommandLineArgumentArray[6];

void MAC_ParseCommandLine(MacDisplayState* pState, const char* szCmdLine);
Boolean FUN_0019cc81(int nOption0, int nFullScreen);

// 1.14d mac: 0019ae9c
// Builds the command line (the process's own, else the one saved in the registry, plus
// -skiptobnet), picks the application mode and hands over to ApplicationMain.
int MAC_ParseCommandLineAndRun(int nInstance) {
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
    MAC_ParseCommandLine(&g_tMacDisplayState, szCmdLine);
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
        const char* szDelimiters = " \t";
        if (ch == '"') {
            szDelimiters = "\"";
        }
        char* pszToken = strtok(s_pCmdLineScan, szDelimiters);
        char* pszScan = s_pCmdLineScan;
        if (pszToken != NULL) {
            size_t nTokenLen = strlen(s_pCmdLineScan);
            pszRest = pszScan + nTokenLen + 1;
            s_nCmdLineLeft -= nTokenLen + 1;
            s_pCmdLineScan = pszRest;
        }
    }
    return pszRest;
}

// 1.14d mac: 0019b290
// Lets the handlers below post.
void FUN_0019b290(void) {
    s_bEventsEnabled = 1;
}

// 1.14d mac: 0019b2a2
void FUN_0019b2a2(void) {
    s_bEventsEnabled = 0;
}

// 1.14d mac: 0019b2b4
void FUN_0019b2b4(int nExitCode) {
    g_bMacQuitRequested = 1;
    g_nMacMainThread = (uint32_t)-1;
}

// 1.14d mac: 0019b2d1
void FUN_0019b2d1(int nMouseSensitivity) {
}

static uint8_t MAC_PostMouseMessage(uint32_t nMessage, uint32_t nWParamHigh, uint16_t nModifiers, Point ptMouse);

// 1.14d mac: 0019b2d9
// Whether a point (in window coordinates) is on the game's screen.
bool MAC_IsMousePointInGameScreen(Point ptMouse) {
    if (s_bEventsEnabled == 1) {
        Point ptLocal = ptMouse;
        if (FUN_002b5dc3(&ptLocal) != 0) {
            int nScreenWidth;
            int nScreenHeight;
            D2GFX_GetScreenSize(&nScreenWidth, &nScreenHeight);
            if (-1 < ptLocal.h && ptLocal.h <= nScreenWidth) {
                if (-1 < ptLocal.v) {
                    return ptLocal.v <= nScreenHeight;
                }
            }
        }
    }
    return false;
}

static uint8_t FUN_0019b51a(uint32_t nButton, bool bPressed, uint16_t nModifiers, Point ptMouse);

// 1.14d mac: 0019b33e
// The Carbon 'mous' handler: the wheel is posted as WM_MOUSEWHEEL, and the buttons from the
// third on as button presses; the first two come with the classic events.
OSStatus MAC_HandleMouseCarbonEvent(EventHandlerCallRef hCallRef, EventRef hEvent, void* pUserData) {
    OSStatus nStatus = eventNotHandledErr;
    if (GetEventClass(hEvent) == kEventClassMouse) {
        Point ptMouse;
        GetEventParameter(hEvent, kEventParamMouseLocation, typeQDPoint, NULL, 4, NULL, &ptMouse);
        if (MAC_IsMousePointInGameScreen(ptMouse)) {
            UInt32 nKind = GetEventKind(hEvent);
            if (nKind == kEventMouseWheelMoved) {
                int32_t nWheelDelta;
                UInt32 nModifiers;
                GetEventParameter(hEvent, kEventParamMouseWheelDelta, typeLongInteger, NULL, 4, NULL, &nWheelDelta);
                GetEventParameter(hEvent, kEventParamKeyModifiers, typeUInt32, NULL, 4, NULL, &nModifiers);
                uint32_t nDelta = (uint32_t)(nWheelDelta * 0x78000) >> 12;
                MAC_PostMouseMessage(0x20A, nDelta & 0xFFFF, (uint16_t)nModifiers, ptMouse);
            } else {
                if (1 < nKind - 1) {
                    return eventNotHandledErr;
                }
                EventMouseButton nButton;
                UInt32 nModifiers;
                GetEventParameter(hEvent, kEventParamMouseButton, typeMouseButton, NULL, 2, NULL, &nButton);
                if (nButton < 2) {
                    return eventNotHandledErr;
                }
                GetEventParameter(hEvent, kEventParamKeyModifiers, typeUInt32, NULL, 4, NULL, &nModifiers);
                FUN_0019b51a(nButton, nKind == 1, (uint16_t)nModifiers, ptMouse);
            }
            nStatus = noErr;
        }
    }
    return nStatus;
}

// 1.14d mac: 0019b51a
// A new pressed state of a mouse button: posted as the matching WM_*BUTTON message when it
// changed. A command-click on the left button counts as the right one.
static uint8_t FUN_0019b51a(uint32_t nButton, bool bPressed, uint16_t nModifiers, Point ptMouse) {
    uint32_t nMessage;
    uint32_t nWParamHigh;
    if (nButton > 5 || s_abMouseButtons[nButton] == bPressed) {
        return 0;
    }
    s_abMouseButtons[nButton] = bPressed;
    if (nButton == 1) {
        if (!bPressed) {
            if (s_bCommandClick == 1) {
                s_bCommandClick = 0;
                goto emulate;
            }
        } else if (nModifiers & 0x100) {
            s_bCommandClick = 1;
        emulate:
            if (s_abMouseButtons[2] != 0) {
                return 0;
            }
            nButton = 2;
            goto dispatch;
        }
        nMessage = 0x201 + (bPressed == 0);
        nWParamHigh = 0;
    } else {
        if (!((nButton == 2) & s_bCommandClick) == 0) {
            return 0;
        }
    dispatch:
        switch (nButton) {
        case 1:
            nMessage = 0x201 + (bPressed == 0);
            nWParamHigh = 0;
            break;
        case 2:
            nMessage = (bPressed == 0) | 0x204;
            nWParamHigh = 0;
            break;
        case 3:
            nMessage = 0x207 + (bPressed == 0);
            nWParamHigh = 0;
            break;
        case 4:
            nMessage = (bPressed == 0) + 0x20B;
            nWParamHigh = 1;
            break;
        case 5:
            nMessage = (bPressed == 0) + 0x20B;
            nWParamHigh = 2;
            break;
        default:
            return 0;
        }
    }
    return MAC_PostMouseMessage(nMessage, nWParamHigh, nModifiers, ptMouse);
}

// 1.14d mac: 0019b732
// Storm's exit handler: the run is over.
static void FUN_0019b732(void) {
    s_bExiting = true;
}

// 1.14d mac: 0019b744
void FUN_0019b744(void) {
    MAC_DSpShutdown();
}

// 1.14d mac: 0019b74d
// Keeps the modifier keys of an event, and passes the event on while posting is on.
uint8_t FUN_0019b74d(EventRecord* pEvent) {
    g_wMacModifiers = pEvent->modifiers;
    g_bMacCapsLock = (pEvent->modifiers >> 10) & 1;
    if (!s_bEventsEnabled) {
        return 0;
    }
    return FUN_002b5da4(pEvent);
}

// 1.14d mac: 0019b795
int FUN_0019b795(void) {
    FUN_00032bcd(1500);
    return 0;
}

// 1.14d mac: 0019b7ae
uint8_t MAC_HandleMouseDownEvent(EventRecord* pEvent) {
    return FUN_0019b51a(1, true, pEvent->modifiers, pEvent->where);
}

// 1.14d mac: 0019b7e2
uint8_t MAC_HandleMouseMoved(EventRecord* pEvent) {
    return MAC_PostMouseMessage(0x200, 0, pEvent->modifiers, pEvent->where);
}

// 1.14d mac: 0019b813
// Posts a mouse message to Storm: the position clamped to the screen, the buttons and
// modifier keys as the wParam, or a non-client move when the point is outside the window.
static uint8_t MAC_PostMouseMessage(uint32_t nMessage, uint32_t nWParamHigh, uint16_t nModifiers, Point ptMouse) {
    int aWindowRect[4];
    int nScreenWidth;
    int nScreenHeight;
    uint32_t nPoint = *(uint32_t*)&ptMouse;
    GetWindowRect((int)GetHwnd(), aWindowRect);
    Point ptOrigin = {0, 0};
    LocalToGlobal(&ptOrigin);
    aWindowRect[0] = ptOrigin.h;
    aWindowRect[2] = aWindowRect[2] + aWindowRect[0];
    aWindowRect[1] = ptOrigin.v;
    aWindowRect[3] = aWindowRect[3] + aWindowRect[1];
    int nX = (int)nPoint >> 16;
    uint8_t bPosted = 0;
    bool bInWindow = false;
    if (nX < aWindowRect[2] && aWindowRect[0] <= nX) {
        bInWindow = (short)nPoint < aWindowRect[3] && aWindowRect[1] <= (short)nPoint;
    }
    if (s_bEventsEnabled && FUN_002b5dc3(&ptMouse)) {
        D2GFX_GetScreenSize(&nScreenWidth, &nScreenHeight);
        if (ptMouse.h < 0) {
            ptMouse.h = 0;
        }
        if (nScreenWidth <= ptMouse.h) {
            ptMouse.h = nScreenWidth - 1;
        }
        if (ptMouse.v < 0) {
            ptMouse.v = 0;
        }
        if (nScreenHeight <= ptMouse.v) {
            ptMouse.v = nScreenHeight - 1;
        }
        uint32_t nWParam;
        uint32_t nLParam;
        if (bInWindow) {
            nWParam = (s_abMouseButtons[1] != 0) | (nWParamHigh << 16);
            if (s_abMouseButtons[2]) nWParam += 2;
            if (s_abMouseButtons[3]) nWParam |= 0x10;
            if (s_abMouseButtons[4]) nWParam |= 0x20;
            if (s_abMouseButtons[5]) nWParam |= 0x40;
            if (nModifiers & 0x200) nWParam |= 4;
            if (!s_bEventsEnabled) {
                return 0;
            }
            nLParam = (uint32_t)ptMouse.h | ((uint32_t)(uint16_t)ptMouse.v << 16);
            if (nModifiers & 0x1000) nWParam |= 8;
        } else {
            if (!s_bEventsEnabled) {
                return 0;
            }
            nLParam = (nPoint << 16) | nX;
            nMessage = 0xA0;
            nWParam = 2;
        }
        FUN_000297c3(nMessage, nWParam, nLParam);
        bPosted = 1;
    }
    return bPosted;
}

// 1.14d mac: 0019ba0c
uint8_t MAC_HandleMouseUpEvent(EventRecord* pEvent) {
    return FUN_0019b51a(1, false, pEvent->modifiers, pEvent->where);
}

// 1.14d mac: 0019ba3d
// A key went down: posted to Storm while the handlers are on.
bool MAC_PostKeyDown(EventRecord* pEvent) {
    if (s_bEventsEnabled) {
        MSG_PostKeyMessage(1, (uint8_t)(pEvent->message >> 8), (uint8_t)pEvent->message);
        return true;
    }
    return false;
}

// 1.14d mac: 0019ba81
bool MAC_PostKeyUp(EventRecord* pEvent) {
    if (s_bEventsEnabled) {
        MSG_PostKeyMessage(0, (uint8_t)(pEvent->message >> 8), (uint8_t)pEvent->message);
        return true;
    }
    return false;
}

// 1.14d mac: 0019bac5
int FUN_0019bac5(void) {
    return 0;
}

// 1.14d mac: 0019bacc
// The application was suspended: lets go of every held mouse button, posts WM_ACTIVATEAPP
// (inactive) and tells the application.
bool MAC_OnApplicationDeactivate(EventRecord* pEvent) {
    Point ptZero = {0, 0};
    for (uint32_t nButton = 1; nButton != 6; nButton++) {
        if (s_abMouseButtons[nButton]) {
            FUN_0019b51a(nButton, false, 0, ptZero);
        }
    }
    bool bPosted;
    if (s_bEventsEnabled) {
        FUN_000297c3(0x1C, 0, 0);
        bPosted = true;
    } else {
        bPosted = false;
    }
    g_pMacAppProcs->apfn[28]();
    return bPosted;
}

// 1.14d mac: 0019bb55
// The application was resumed.
bool MAC_OnApplicationActivate(EventRecord* pEvent) {
    bool bPosted;
    if (s_bEventsEnabled) {
        FUN_000297c3(0x1C, 1, 0);
        bPosted = true;
    } else {
        bPosted = false;
    }
    g_pMacAppProcs->apfn[29]();
    return bPosted;
}

void FUN_0019be2c(void);

// 1.14d mac: 0019bba4
// An update event: the whole window is redrawn.
bool MAC_HandleUpdateEvent(EventRecord* pEvent) {
    if (s_bEventsEnabled) {
        FUN_0019be2c();
        return true;
    }
    return false;
}

uint8_t MAC_HandleMenuCommand(uint32_t nCommand);

// 1.14d mac: 0019bbcd
uint8_t FUN_0019bbcd(uint32_t nCommand) {
    return MAC_HandleMenuCommand(nCommand);
}

// 1.14d mac: 0019bbe8
// The application's module handle, asked for once.
void* FUN_0019bbe8(void) {
    static void* s_hModule = GetModuleHandle(NULL);
    return s_hModule;
}

// 1.14d mac: 0019c760
// The allocator handed to the set-up code: never NULL, -1 on failure.
void* GAMESETUP_Alloc(size_t nSize) {
    void* pMemory = SMemAlloc(nSize, __FILE__, 463, 0);
    return pMemory ? pMemory : (void*)-1;
}

// 1.14d mac: 0019c79f
void GAMESETUP_Free(void* pMemory) {
    if (pMemory)
        SMemFree(pMemory, __FILE__, 486, 0);
}

// 1.14d mac: 0019c7d9
int FUN_0019c7d9(void) {
    return 1;
}

// 1.14d mac: 0019c7e4
// A display switch failed: forget every video setting, tell the user (STR# 200, 15 and 16)
// and quit.
void MAC_ResetVideoSettingsAndExit(void) {
    Str255 szError;
    Str255 szExplanation;
    DialogItemIndex nItemHit;
    SRegDeleteValue("Diablo II Mac", "Video Mode", 0);
    SRegDeleteValue("Diablo II Mac", "Video Engine", 0);
    SRegDeleteValue("Diablo II Mac", "Display ID", 0);
    SRegDeleteValue("Diablo II Mac", "Video Detect", 0);
    SRegDeleteValue("Diablo II Mac", "Engine Names", 0);
    SRegDeleteValue("Diablo II Mac", "32 Bit Video", 0);
    SRegDeleteValue("Diablo II Mac", "1-Resolution", 0);
    SRegDeleteValue("Diablo II Mac", "1-Expert", 0);
    SRegDeleteValue("Diablo II Mac", "Dont Show Again", 0);
    GetIndString(szError, 200, 15);
    GetIndString(szExplanation, 200, 16);
    StandardAlert(kAlertStopAlert, szError, szExplanation, 0, &nItemHit);
    exit(1);
}

int FUN_0019c96e(Boolean bFullScreen);
void MAC_ApplyDisplayModuleSettings(void);

// 1.14d mac: 0019c953
void FUN_0019c953(void) {
    FUN_0019c96e(FUN_002b5c1c());
}

// 1.14d mac: 0019c96e
// Switches D2gfx between windowed and full screen, unless it is already there; without a
// renderer only the flag is recorded. Waits for the renderer to go idle first.
int FUN_0019c96e(Boolean bFullScreen) {
    int nRendererMode = D2GFX_GetRendererMode();
    int nState = FUN_002b4d60();
    if (!nRendererMode) {
        if (bFullScreen) {
            g_tMacDisplayState.b0A = 0;
            g_tMacDisplayState.b0B = 0;
            g_tMacDisplayState.b0C = 0;
            g_tMacDisplayState.b0D = 0;
            g_tMacDisplayState.bFullScreen = true;
        } else {
            g_tMacDisplayState.bFullScreen = false;
        }
        return 1;
    }
    if (bFullScreen && nState == 1)
        return 1;
    if (!bFullScreen && nState == 0)
        return 1;
    if (!s_bSkipRendererIdleWait) {
        int bBusy = 0;
        do {
            D2GFX_SetOption(10, (intptr_t)&bBusy);
            if (!bBusy)
                break;
            pthread_yield_np();
        } while (bBusy);
    }
    if (bFullScreen) {
        int nVideoMode = g_nMacVideoMode;
        if (!D2GFX_SwitchModule(nVideoMode, 1, 1))
            MAC_ResetVideoSettingsAndExit();
        CLIENT_SetRendererMode(nVideoMode, 1);
        g_tMacDisplayState.b0A = 0;
        g_tMacDisplayState.b0B = 1;
        g_tMacDisplayState.b0C = 0;
        g_tMacDisplayState.b0D = 0;
        g_tMacDisplayState.bFullScreen = 1;
        MAC_ApplyDisplayModuleSettings();
        if (!D2GFX_SwitchModule(nVideoMode, 1, 2))
            MAC_ResetVideoSettingsAndExit();
    } else {
        if (!D2GFX_SwitchModule(g_nMacVideoMode, 0, 1)) {
            g_nMacVideoMode = 2;
            if (!D2GFX_SwitchModule(2, 0, 1))
                MAC_ResetVideoSettingsAndExit();
        }
        CLIENT_SetRendererMode(g_nMacVideoMode, 0);
        g_tMacDisplayState.b0A = 0;
        g_tMacDisplayState.b0B = 1;
        g_tMacDisplayState.b0C = 0;
        g_tMacDisplayState.b0D = 0;
        g_tMacDisplayState.bFullScreen = 0;
        MAC_ApplyDisplayModuleSettings();
        if (!D2GFX_SwitchModule(g_nMacVideoMode, 0, 2))
            MAC_ResetVideoSettingsAndExit();
    }
    D2WINSMACK_SetKeepVideoFile();
    return 1;
}

// 1.14d mac: 0019cb40
// Flags g_bMacDisplayMismatch when D2gfx is not in the display state asked for.
Boolean FUN_0019cb40(Boolean bFullScreen) {
    int nState = FUN_002b4d60();
    if (bFullScreen) {
        if (nState == 1)
            return 1;
    } else if (nState == 0) {
        return 1;
    }
    g_bMacDisplayMismatch = 1;
    return 1;
}

// 1.14d mac: 0019cb78
// The display module the set-up chose, as a D2gfx video mode, and its options.
void MAC_ApplyDisplayModuleSettings(void) {
    int nDisplayModule;
    int nOption4;
    int nOption5;
    MacDisplayOption tOption2;
    MacDisplayOption tOption3;
    int nOption0;
    MACGFX_EnumerateDisplayModes(&nDisplayModule, &nOption4, &nOption5, &nOption0, &tOption2, &tOption3);
    switch (nDisplayModule) {
    case 1:
        g_nMacVideoMode = 2;
        break;
    case 2:
        g_nMacVideoMode = 4;
        break;
    case 3:
        g_nMacVideoMode = 5;
        break;
    case 4:
        g_nMacVideoMode = 7;
        break;
    default:
        fp_fatal("Unrecognized display module!");
    }
    D2GFX_SetOption(5, nOption5);
    D2GFX_SetOption(0, nOption0);
    D2GFX_SetOption(2, (intptr_t)&tOption2);
    D2GFX_SetOption(3, (intptr_t)&tOption3);
    D2GFX_SetOption(4, nOption4);
}

// 1.14d mac: 0019cc78
void MAC_DSpShutdown(void) {
    DSpShutdown();
}

// 1.14d mac: 0019cf50
// Checks that D2gfx is full screen, without waiting for the renderer.
void FUN_0019cf50(void) {
    s_bSkipRendererIdleWait = true;
    FUN_0019cb40(1);
    s_bSkipRendererIdleWait = false;
}

#endif  // D2_OS_MAC
