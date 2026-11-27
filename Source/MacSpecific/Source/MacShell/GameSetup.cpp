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

#include <QuickTime/QuickTime.h>
#include <CoreServices/CoreServices.h>
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
// 1.14d mac: 0019784c
void MISC_InitIgnorelist(void);
// 1.14d mac: 002ef085
void* FOG_GetFramePointer(void);
// 1.14d mac: 0002c0ee
uint32_t GetTickCount(void);
// 1.14d mac: 0002c035
void Sleep(uint32_t dwMilliseconds);
// 1.14d mac: 0001e9fb
void MAC_OutputDebugStringF(const char* szFormat, ...);
// 1.14d mac: 00028d7f
void SMemZero(void* pMem, uint32_t nBytes);
// 1.14d mac: 002b98b1
uint32_t MISC_GetLanguageCode(void);
// 1.14d mac: 00029946
void MSG_ExchangeCmdPeriodFlag(int* pnFlag);
// 1.14d mac: 00013bad
uint32_t SFileLoadFile(const char* szFileName, void** ppBuffer, uint32_t* pdwSize, uint32_t dwSearchScope, void* pOverlapped);
// 1.14d mac: 002b8fba
void TABLES_GetLangAbbreviation(char* szOut, int nLanguage);
// 1.14d mac: 002ee3a4
void FUN_002ee3a4(const char* szName);
// 1.14d mac: 0003e2ab
void FUN_0003e2ab(void);
// 1.14d mac: 00018c44
int SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags);
// 1.14d mac: 00396200
extern void (*g_pfnMacAbortHook)(void);
void MAC_DeactivateTSMDocument(void);
void MAC_ActivateTSMDocument(void);
void MAC_ShutdownTSM(void);
void MAC_DisposeTSMDocument(void);
OSErr MAC_HandleTSMUpdateActiveInputArea(const AppleEvent* pAppleEvent);
OSErr MAC_HandleTSMShowHideInputWindow(const AppleEvent* pAppleEvent, AppleEvent* pReply, SRefCon nRefCon);
OSErr MAC_HandleTSMPosToOffset(const AppleEvent* pAppleEvent, AppleEvent* pReply);
OSErr MAC_HandleTSMOffsetToPos(const AppleEvent* pAppleEvent, AppleEvent* pReply);
static void MAC_ShowFatalErrorAndExit(const unsigned char* szMessage);
void FUN_0019db13(void);
__attribute__((noinline)) void FUN_0019be53(int nExitCode);
extern uint8_t g_bMacMenuToggle;
void FUN_0019e432(void);
void MAC_SetWorkingDirectoryToAppFolder(void);
// 1.14d mac: 00036d19
bool MACDISPLAY_SelectDriver(int nDriver);
// 1.14d mac: 00036d8b
void FUN_00036d8b(Boolean (*pfnCallback)(Boolean));
// 1.14d mac: 00036d9f
void MACCURSOR_RegisterSetCursorPosProc(bool (*pfnSetCursorPos)(int16_t*, int16_t*));
// 1.14d mac: 00036db3
void MACCURSOR_RegisterGetCursorPosProc(bool (*pfnGetCursorPos)(int*, int*));
// 1.14d mac: 00036830
void MACCURSOR_ShowDisplayCursor(void);
// 1.14d mac: 00036847
void FUN_00036847(void);
// 1.14d mac: 00036ff5
void MACCURSOR_InitCursor(void);
// 1.14d mac: 0001e550
void FPSetMessageBoxCallback(void (*pfnCallback)(void));
// 1.14d mac: 0001e9bb
void MAC_GetGlobalMouse(Point* pPoint);
// 1.14d mac: 0001e9c4
void MAC_WarpMouseCursorPosition(uint32_t ptCursor);
// 1.14d mac: 002b5de2
// Converts a game point to global coordinates; 0 without a renderer.
int FUN_002b5de2(Point* pPoint);
bool MAC_SetCursorPos(int16_t* pnX, int16_t* pnY);
bool MAC_GetCursorPos(int* pnX, int* pnY);
__attribute__((noinline)) bool MAC_OnApplicationDeactivate(EventRecord* pEvent);
__attribute__((noinline)) bool MAC_OnApplicationActivate(EventRecord* pEvent);
static __attribute__((noinline)) void MAC_HandleKeyDownEvent(EventRecord* pEvent);
static __attribute__((noinline)) uint8_t MAC_HandleSuspendResumeEvent(EventRecord* pEvent);
void FUN_0019db65(void);
static void MAC_SynthesizeKeyEvent(EventRecord* pEvent, int nKeyCode, char bPressed);
static uint8_t MAC_HandleMenuSelection(uint32_t nMenuResult);
// 1.14d mac: 0002a6a3
uint32_t GetTempPathFreeSpace(void);
// 1.14d mac: 00030cb2
void* GetActiveWindow(void);
// 1.14d mac: 000305a7
void* GetFocus(void);
// 1.14d mac: 0002956e
BOOL PostMessage(void* hWnd, uint32_t nMessage, uint32_t nWParam, uint32_t nLParam);
// 1.14d mac: 000298e3
void MSG_PostAltF4Keys(void);
// 1.14d mac: 002eff85
void LOGFILE_WriteStringToFile(const char* szFormat, ...);
// 1.14d mac: 00033f9a
short GetApplicationInformation(void* pUnused1, short* pnVolume, uint32_t* pUnused2, uint64_t* pUnused3);
// 1.14d mac: 002ef3d4
void SFILE_InitializeFileSystem(int nArg1, int nArg2);
// 1.14d mac: 000136ea
int SFileFileExistsEx(void* hArchive, const char* szFileName, uint32_t dwSearchScope);
// 1.14d mac: 00014226
int SFileOpenArchive(const char* szArchiveName, int nPriority, uint32_t dwFlags, void** phArchive);
// 1.14d mac: 0001239c
int SFileCloseArchive(void* hArchive);
// 1.14d mac: 0001e564
int FPMessageBox(const char* szText, const char* szCaption, uint32_t dwType);
// 1.14d mac: 00304096
void BINK_SetMemory(void* (*pfnAlloc)(size_t), void (*pfnFree)(void*));
// 1.14d mac: 0005ad4d
void BNMESSQUEUE_Shutdown(void);
// The Battle.net client's callback table; only the two slots GAMESETUP calls are laid out.
struct BNetClientCallbacks {
    void (*pfnSetLoggingFlag)(void* pFlag);
    uint8_t unk04[0x90];
    int (*pfnDecodeCDKeys)(void);
};
// 1.14d mac: 0005ad31
BNetClientCallbacks* BNCLIENT_GetCallbacks(void);
// 1.14d mac: 0019ced5
void MAC_DisableMenus(void);
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
static __attribute__((noinline)) char* FUN_0019b1ec(char* szCmdLine);
static __attribute__((noinline)) Boolean FUN_0019bed3(void);
static void FUN_0019b732(void);
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
int FUN_002b5dc3(Point* pPoint);
// 1.14d mac: 0019ebbe
int ApplicationMain(int nInstance, void* pDisplayState, int nMode);

#include "Source/StormMac/H/stpl.h"

extern "C" OSStatus DSpShutdown(void);
extern "C" OSStatus DSpStartup(void);
extern "C" uint32_t DSpGetVersion(void);
void* GAMESETUP_Alloc(size_t nSize);
void GAMESETUP_Free(void* pMemory);
int FUN_0019c7d9(void);

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
    uint8_t unk0E[0x211 - 0x0E];
    void* pfnHook __attribute__((packed));  // 0x211
};
static_assert(sizeof(void*) != 4 || offsetof(MacDisplayState, pfnHook) == 0x211, "MacDisplayState::pfnHook");
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
int g_nMacAppInstance;
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
bool g_bMacExiting;
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

// 1.14d mac: 0019b636
// Starts the shell: Storm's exit handler, the Carbon mouse handler for the wheel and the
// extra buttons, the system and archive checks; on success the heap is purged and compacted.
bool FUN_0019b636(void) {
    PROCESS_SetExitHandler(FUN_0019b732);
    static EventHandlerUPP s_pfnMouseHandler = MAC_HandleMouseCarbonEvent;
    EventTypeSpec aEvents[3] = {{kEventClassMouse, kEventMouseDown}, {kEventClassMouse, kEventMouseUp}, {kEventClassMouse, kEventMouseWheelMoved}};
    InstallEventHandler(GetApplicationEventTarget(), s_pfnMouseHandler, 3, aEvents, NULL, NULL);
    Boolean bResult = FUN_0019bed3();
    if (bResult) {
        PurgeMem(0x7FFFFFF0);
        CompactMem(0x7FFFFFF0);
        g_nMacMainThread = GetCurrentThreadId();
    }
    return bResult != 0;
}

// 1.14d mac: 0019b732
// Storm's exit handler: the run is over.
static void FUN_0019b732(void) {
    g_bMacExiting = true;
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
    if (s_bEventsEnabled && (uint8_t)FUN_002b5dc3(&ptMouse)) {
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

__attribute__((noinline)) void FUN_0019be2c(void);

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

// 1.14d mac: 0019bc4b
// A menu item: the edit commands go to the focused window as WM_CUT/COPY/PASTE/CLEAR (and the
// two Storm-specific ones), quit sets the display flag, Alt-F4 is posted for the close item.
uint8_t MAC_HandleMenuCommand(uint32_t nCommand) {
    void* hWnd;
    uint32_t nMessage;
    if (nCommand < 0x81) {
        switch (nCommand) {
        case 4:
            hWnd = GetActiveWindow();
            if (hWnd == NULL) {
                return 0;
            }
            nMessage = 0x3A2;
            break;
        default:
            return 0;
        case 7:
            MSG_PostAltF4Keys();
            return 1;
        case 9:
            hWnd = GetFocus();
            if (hWnd == NULL && (hWnd = GetActiveWindow()) == NULL) {
                return 0;
            }
            nMessage = 0x300;
            break;
        case 10:
            hWnd = GetFocus();
            if (hWnd == NULL && (hWnd = GetActiveWindow()) == NULL) {
                return 0;
            }
            nMessage = 0x301;
            break;
        case 11:
            hWnd = GetFocus();
            if (hWnd == NULL && (hWnd = GetActiveWindow()) == NULL) {
                return 0;
            }
            nMessage = 0x302;
            break;
        case 12:
            hWnd = GetFocus();
            if (hWnd == NULL && (hWnd = GetActiveWindow()) == NULL) {
                return 0;
            }
            nMessage = 0x303;
            break;
        case 13:
            hWnd = GetFocus();
            if (hWnd == NULL && (hWnd = GetActiveWindow()) == NULL) {
                return 0;
            }
            nMessage = 0x3A6;
        }
        PostMessage(hWnd, nMessage, 0, 0);
    } else {
        if (nCommand != 0x81) {
            return 0;
        }
        g_bMacDisplayMismatch = 1;
    }
    return 1;
}

// 1.14d mac: 0019be00
// The display mode the set-up assumes before the game has chosen one.
void MAC_GetDefaultDisplayMode(int* pnWidth, int* pnHeight, int* pnBitDepth) {
    if (pnWidth != NULL) {
        *pnWidth = 800;
    }
    if (pnHeight != NULL) {
        *pnHeight = 600;
    }
    if (pnBitDepth != NULL) {
        *pnBitDepth = 8;
    }
}

// 1.14d mac: 0019be2c
void FUN_0019be2c(void) {
    InvalidateRect(NULL, NULL, 1);
}


// 1.14d mac: 0019be53
void FUN_0019be53(int nExitCode) {
    FUN_0019b2a2();
    g_nMacAppInstance = 0;
    FUN_0019b2b4(nExitCode);
}

// 1.14d mac: 0019be79
// Runs the game: the handlers on, the command line parsed and the application run, then the
// handlers off again.
void FUN_0019be79(void) {
    FUN_0019b290();
    int nExitCode = MAC_ParseCommandLineAndRun((int)FUN_0019bbe8());
    FUN_0019b2a2();
    g_nMacAppInstance = 0;
    FUN_0019b2b4(nExitCode);
}

// 1.14d mac: 0019bed3
// Checks the system (OS version, memory, threads, Sound Manager, DrawSprocket, disk space),
// then the game archives and the CD key, and asks for the game disc. 1 when the game may start.
static Boolean FUN_0019bed3(void) {
    int nWidth;
    int nHeight;
    int nBitDepth;
    Str255 szError = {0};
    long nResponse;
    bool bRequirementsMet;
    MAC_GetDefaultDisplayMode(&nWidth, &nHeight, &nBitDepth);
    if (Gestalt(gestaltSystemVersion, &nResponse) == 0 && (nResponse & 0xFFFF) > 0x80F) {
        if ((nResponse & 0xFFFF) < 0x1012) {
            bRequirementsMet = false;
            if ((nResponse & 0xFFFF) < 0x1000) {
                GetIndString(szError, 201, 1);
            } else {
                GetIndString(szError, 201, 2);
            }
        } else if (Gestalt(gestaltPhysicalRAMSize, &nResponse) == 0 && (uint32_t)nResponse > 0x3FFFFFF) {
            if (Gestalt(gestaltThreadMgrAttr, &nResponse) == 0 && (nResponse & 5) == 5) {
                NumVersion tSndVersion = SndSoundManagerVersion();
                if ((*(uint32_t*)&tSndVersion >> 16) < 0x310) {
                    bRequirementsMet = false;
                    GetIndString(szError, 200, 4);
                } else {
                    uint32_t nDSpVersion = DSpGetVersion();
                    if ((nDSpVersion >> 16) < 0x173) {
                        LOGFILE_WriteStringToFile("CheckSystemRequirements: *** DSpGetVersion returned %.4x.", nDSpVersion >> 16);
                        bRequirementsMet = false;
                        GetIndString(szError, 200, 6);
                    } else {
                        short nErr = DSpStartup();
                        if (nErr == 0) {
                            short nVolume;
                            if (GetApplicationInformation(NULL, &nVolume, NULL, NULL) == 0) {
                                HParamBlockRec tParams;
                                memset(&tParams, 0, sizeof(tParams));
                                tParams.volumeParam.ioVRefNum = nVolume;
                                if (PBHGetVInfoSync(&tParams) == 0 && (tParams.volumeParam.ioVAtrb & 0x80800000) == 0) {
                                    if (GetTempPathFreeSpace() > 0x27FF) {
                                        bRequirementsMet = true;
                                        if ((uint32_t)(tParams.volumeParam.ioVFrBlk * tParams.volumeParam.ioVAlBlkSiz) < 0x2800) {
                                            bRequirementsMet = false;
                                            GetIndString(szError, 200, 10);
                                        }
                                    } else {
                                        bRequirementsMet = false;
                                        GetIndString(szError, 200, 9);
                                    }
                                } else {
                                    bRequirementsMet = false;
                                    GetIndString(szError, 200, 8);
                                }
                            } else {
                                bRequirementsMet = false;
                                GetIndString(szError, 200, 13);
                            }
                        } else {
                            LOGFILE_WriteStringToFile("CheckSystemRequirements: *** DSpStartup returned %d.", (int)nErr);
                            bRequirementsMet = false;
                            GetIndString(szError, 200, 6);
                        }
                    }
                }
            } else {
                bRequirementsMet = false;
                GetIndString(szError, 200, 3);
            }
        } else {
            bRequirementsMet = false;
            GetIndString(szError, 200, 11);
        }
    } else {
        bRequirementsMet = false;
        GetIndString(szError, 200, 1);
    }
    if (szError[0] != '\0') {
        ParamText(szError, "\p", "\p", "\p");
        StopAlert(0x211, NULL);
    }
    if (!bRequirementsMet) {
        return 0;
    }
    EnterMovies();
    SFILE_InitializeFileSystem(0, 0);
    void* hSounds = NULL;
    void* hExpansion = NULL;
    void* hPatch = NULL;
    void* hGraphics = NULL;
    void* hGameData = NULL;
    BOOL bExpansion = D2CheckExpansionGame();
    const char* szReason;
    if (!SFileOpenArchive("Diablo II Sounds", 0, 0, &hSounds)) {
        szReason = "missing Diablo II Sounds";
    fail:
        LOGFILE_WriteStringToFile("CheckCDKey: *** Failed: %s.", szReason);
        GetIndString(szError, 200, 13);
        ParamText(szError, "\p", "\p", "\p");
        StopAlert(0x211, NULL);
    } else {
        if (bExpansion) {
            if (!SFileOpenArchive("Diablo II Expansion Data", 0, 0, &hExpansion)) {
                szReason = "missing Diablo II Expansion Data";
                goto fail;
            }
            if (!SFileOpenArchive("Diablo II Graphics", 0, 0, &hGraphics)) {
                szReason = "missing Diablo II Graphics";
                goto fail;
            }
        }
        SFileOpenArchive("Diablo II Patch", 0, 0, &hPatch);
        SFileOpenArchive("Diablo II Game Data", 0, 0, &hGameData);
        BNetClientCallbacks* pCallbacks = BNCLIENT_GetCallbacks();
        *(BOOL*)&g_tMacDisplayState = bExpansion;
        pCallbacks->pfnSetLoggingFlag(&g_tMacDisplayState);
        if (!pCallbacks->pfnDecodeCDKeys()) {
            szReason = "invalid or missing CD-key";
            goto fail;
        }
        BNMESSQUEUE_Shutdown();
        SFileCloseArchive(hSounds);
        if (hExpansion != NULL) {
            SFileCloseArchive(hExpansion);
        }
        if (hPatch != NULL) {
            SFileCloseArchive(hPatch);
        }
        if (hGraphics != NULL) {
            SFileCloseArchive(hGraphics);
        }
        if (hGameData != NULL) {
            SFileCloseArchive(hGameData);
        }
        BINK_SetMemory((void* (*)(size_t))GAMESETUP_Alloc, GAMESETUP_Free);
        g_nMacAppMode = 4;
        g_tMacDisplayState.b0B = 1;
        g_tMacDisplayState.pfnHook = (void*)FUN_0019c7d9;
        MAC_DisableMenus();
        if (MAC_PromptForGameDisc()) {
            BOOL bGameExpansion = D2CheckExpansionGame();
            void* hPatchArchive = NULL;
            const char* szText;
            if (!SFileOpenArchive("Diablo II Patch", 0, 0, &hPatchArchive)) {
                szText = "The file \"Diablo II Patch\" cannot be opened.";
            } else {
                int nHasMarker = SFileFileExistsEx(hPatchArchive, "Data\\IAmExpansion", 0);
                SFileCloseArchive(hPatchArchive);
                if (bGameExpansion == (nHasMarker == 1)) {
                    return 1;
                }
                szText = "An incompatible Diablo II Patch data file has been detected. You have classic Diablo II installed, "
                         "but the patch data is for use with Diablo II: Lord of Destruction.";
                if (bGameExpansion) {
                    szText = "An incompatible Diablo II Patch data file has been detected. You have Diablo II: Lord of "
                             "Destruction installed, but the patch data is for use with classic Diablo II.";
                }
            }
            LOGFILE_WriteStringToFile("PatchMPQMatchesGameType: *** %s.", szText);
            GetIndString(szError, 200, 13);
            char szErrorText[256];
            CopyPascalStringToC(szError, szErrorText);
            FPMessageBox(szText, szErrorText, 0x20000010);
        }
    }
    return 0;
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

// The two display set-up records the application's display hook takes (their meaning is
// the renderer's; only the fields the shell fills are named).
struct MacDisplayRequest {
    int32_t nWidth;
    int32_t nHeight;
    int32_t nPitch;
    int32_t nBitsPerPixel;
    uint8_t byDepth;
    uint8_t byFlags;
};
struct MacDisplayLimits {
    int32_t nWidth;
    int32_t nHeight;
    int32_t nPitch;
    int32_t nMaxBitsPerPixel;
    int32_t nDepth;
    int32_t nFlags;
};

// 1.14d mac: 00536c54
// The display module the set-up chose (the first output of MACGFX_EnumerateDisplayModes).
static int s_nDisplayModule;

// 1.14d mac: 00331dfa
extern const char g_szMacDisplayName[];

// 1.14d mac: 0019cc81
// Brings the process to the front and sets the display up through the application's display
// hook; the cursor and display-state callbacks go to Storm and the cursor module. False (and
// DrawSprocket shut down again) when the display is not available.
Boolean FUN_0019cc81(int nDisplayModule, int nUnused) {
    ProcessSerialNumber tProcess;
    int nOptionWidth;
    int nOptionHeight;
    int nOptionDepth;
    tProcess.highLongOfPSN = 0;
    tProcess.lowLongOfPSN = kCurrentProcess;
    SetFrontProcess(&tProcess);
    MAC_GetDefaultDisplayMode(&nOptionWidth, &nOptionHeight, &nOptionDepth);
    s_nDisplayModule = nDisplayModule;
    if (MACDISPLAY_SelectDriver(0)) {
        ((void (*)(void*, void*, void*))g_pMacAppProcs->apfn[49])((void*)MACCURSOR_InitCursor, (void*)MACCURSOR_ShowDisplayCursor,
                                                                   (void*)FUN_00036847);
        int nWidth;
        int nHeight;
        int nBitDepth;
        MAC_GetDefaultDisplayMode(&nWidth, &nHeight, &nBitDepth);
        MacDisplayRequest tRequest;
        tRequest.nWidth = nWidth;
        tRequest.nHeight = nHeight;
        tRequest.nPitch = nWidth;
        tRequest.nBitsPerPixel = 8;
        tRequest.byDepth = (uint8_t)nBitDepth;
        tRequest.byFlags = 2;
        MacDisplayLimits tLimits;
        tLimits.nWidth = nWidth;
        tLimits.nHeight = nHeight;
        tLimits.nPitch = nWidth;
        tLimits.nMaxBitsPerPixel = 0x20;
        tLimits.nDepth = nBitDepth;
        tLimits.nFlags = 0;
        if (((char (*)(void*, void*, int, const char*))g_pMacAppProcs->apfn[13])(&tRequest, &tLimits, s_nDisplayModule,
                                                                                 g_szMacDisplayName) == 1) {
            FPSetMessageBoxCallback(FUN_0019cf50);
            FUN_00036d8b(FUN_0019cb40);
            MACCURSOR_RegisterSetCursorPosProc(MAC_SetCursorPos);
            MACCURSOR_RegisterGetCursorPosProc(MAC_GetCursorPos);
            return 1;
        }
    }
    DSpShutdown();
    return 0;
}

// 1.14d mac: 0019cdd3
// Warps the cursor to a game position; false without a renderer.
bool MAC_SetCursorPos(int16_t* pnX, int16_t* pnY) {
    Point ptCursor;
    ptCursor.h = *pnX;
    ptCursor.v = *pnY;
    int bConverted = FUN_002b5de2(&ptCursor);
    if (bConverted != 0) {
        MAC_WarpMouseCursorPosition(*(uint32_t*)&ptCursor);
    }
    return bConverted != 0;
}

// 1.14d mac: 0019ce1b
// The cursor position in game coordinates; false without a renderer.
bool MAC_GetCursorPos(int* pnX, int* pnY) {
    Point ptCursor;
    MAC_GetGlobalMouse(&ptCursor);
    int bConverted = FUN_002b5dc3(&ptCursor);
    if (bConverted != 0) {
        *pnX = ptCursor.h;
        *pnY = ptCursor.v;
    }
    return bConverted != 0;
}

// 1.14d mac: 0019ce5a
void MAC_EnableMenus(void) {
    EnableMenuItem(GetMenuHandle(0x81), 0);
    EnableMenuItem(GetMenuHandle(0x82), 0);
    EnableMenuItem(GetMenuHandle(0x83), 0);
    EnableMenuItem(GetMenuHandle(0x84), 0);
}

// 1.14d mac: 0019ced5
void MAC_DisableMenus(void) {
    DisableMenuItem(GetMenuHandle(0x81), 0);
    DisableMenuItem(GetMenuHandle(0x82), 0);
    DisableMenuItem(GetMenuHandle(0x83), 0);
    DisableMenuItem(GetMenuHandle(0x84), 0);
}

// 1.14d mac: 005c8a44
// Set while a modal dialog has the events.
extern int g_bMacModalActive;

static uint8_t MAC_TrackMouseAndModifierChanges(EventRecord* pEvent);

// 1.14d mac: 0019cf88
// Takes events until a null event comes: mouse, keys, updates, the menu bar, window drags and
// AppleEvents.
void MAC_ProcessEvents(void) {
    EventRecord tEvent;
    do {
        WaitNextEvent(everyEvent, &tEvent, 0, NULL);
        if (!MAC_TrackMouseAndModifierChanges(&tEvent)) {
            if (tEvent.what == osEvt) {
                if (MAC_HandleSuspendResumeEvent(&tEvent)) {
                    MAC_TrackMouseAndModifierChanges(&tEvent);
                }
            } else if (tEvent.what < 0x17) {
                switch (tEvent.what) {
                case mouseDown:
                    if (g_bMacModalActive == 0) {
                        WindowRef pWindow;
                        short nPart = FindWindow(tEvent.where, &pWindow);
                        WindowRef pClicked = pWindow;
                        switch (nPart) {
                        case 0:
                        case 3:
                            goto mouseInGame;
                        case 1:
                            FUN_0019db65();
                            MAC_HandleMenuSelection(MenuSelect(tEvent.where));
                            break;
                        case 4: {
                            Rect rcBounds;
                            GetRegionBounds(GetGrayRgn(), &rcBounds);
                            Point ptOrigin = {0, 0};
                            DragWindow(pClicked, tEvent.where, &rcBounds);
                            GrafPtr pSavedPort;
                            GetPort(&pSavedPort);
                            SetPortWindowPort(pClicked);
                            LocalToGlobal(&ptOrigin);
                            SetPort(pSavedPort);
                            ptOrigin.h = (ptOrigin.h + 3) & 0xFFF8;
                            MoveWindow(pClicked, ptOrigin.h, ptOrigin.v, false);
                            break;
                        }
                        case 7:
                        case 8:
                            if (TrackBox(pWindow, tEvent.where, nPart)) {
                                FUN_0019bbcd(0x81);
                            }
                        }
                    } else {
                    mouseInGame:
                        MAC_HandleMouseDownEvent(&tEvent);
                    }
                    break;
                case mouseUp:
                    MAC_HandleMouseUpEvent(&tEvent);
                    break;
                case keyDown:
                case autoKey:
                    MAC_HandleKeyDownEvent(&tEvent);
                    break;
                case keyUp:
                    MAC_PostKeyUp(&tEvent);
                    break;
                case updateEvt: {
                    GrafPtr pSavedPort;
                    GetPort(&pSavedPort);
                    WindowRef pUpdated = (WindowRef)tEvent.message;
                    SetPortWindowPort(pUpdated);
                    BeginUpdate(pUpdated);
                    if (g_bMacModalActive == 0) {
                        MAC_HandleUpdateEvent(&tEvent);
                    }
                    EndUpdate(pUpdated);
                    SetPort(pSavedPort);
                    break;
                }
                default:
                    FUN_0019bac5();
                case activateEvt:
                    break;
                }
            } else {
                if (tEvent.what != kHighLevelEvent) {
                    FUN_0019bac5();
                } else {
                    AEProcessAppleEvent(&tEvent);
                }
            }
        }
        FUN_0019b795();
    } while (tEvent.what != nullEvent);
}

// 1.14d mac: 003b8d80
// The mouse position of the last event.
static Point s_ptLastMouse = {0x7FFF, 0x7FFF};
// 1.14d mac: 00536ca4
// The modifier keys of the last event.
static uint16_t s_wLastModifiers;

// 1.14d mac: 0019d248
// A position change is a WM_MOUSEMOVE; every modifier key that changed is synthesised as a
// key event. True when the renderer took the event itself.
static uint8_t MAC_TrackMouseAndModifierChanges(EventRecord* pEvent) {
    uint8_t bHandled = FUN_0019b74d(pEvent);
    if (!bHandled && pEvent->what < 0x10 && ((0x81FFu >> (pEvent->what & 0x1F)) & 1)) {
        if (s_ptLastMouse.v != pEvent->where.v || s_ptLastMouse.h != pEvent->where.h) {
            MAC_HandleMouseMoved(pEvent);
            s_ptLastMouse = pEvent->where;
        }
        uint16_t wModifiers = pEvent->modifiers;
        if (s_wLastModifiers != wModifiers) {
            EventRecord tSynth;
            memset(&tSynth, 0, sizeof(tSynth.what) + sizeof(tSynth.message));
            tSynth.message = 0;
            tSynth.when = pEvent->when;
            tSynth.where = pEvent->where;
            tSynth.modifiers = wModifiers;
            s_wLastModifiers ^= wModifiers;
            if (s_wLastModifiers & 0x100) {
                uint32_t nPressed = pEvent->modifiers & 1;
                MAC_SynthesizeKeyEvent(&tSynth, 0x37, (char)nPressed);
            }
            if (s_wLastModifiers & 0x200) {
                uint32_t nPressed = (pEvent->modifiers >> 9) & 1;
                MAC_SynthesizeKeyEvent(&tSynth, 0x38, (char)nPressed);
            }
            if (s_wLastModifiers & 0x400) {
                uint32_t nPressed = (pEvent->modifiers >> 10) & 1;
                MAC_SynthesizeKeyEvent(&tSynth, 0x39, (char)nPressed);
            }
            if (s_wLastModifiers & 0x800) {
                uint32_t nPressed = (pEvent->modifiers >> 11) & 1;
                MAC_SynthesizeKeyEvent(&tSynth, 0x3A, (char)nPressed);
            }
            if (s_wLastModifiers & 0x1000) {
                uint32_t nPressed = (pEvent->modifiers >> 12) & 1;
                MAC_SynthesizeKeyEvent(&tSynth, 0x3B, (char)nPressed);
            }
            s_wLastModifiers = pEvent->modifiers;
        }
    }
    return bHandled;
}

// 1.14d mac: 0019d3c2
// An osEvt that is a suspend or resume: the application is told and the event turned into an
// activate event of the front window. True when it was rewritten.
static uint8_t MAC_HandleSuspendResumeEvent(EventRecord* pEvent) {
    pEvent->what = 0;
    uint8_t bResult = 0;
    if ((pEvent->message & 0xFF000000) == 0x01000000) {
        if ((pEvent->message & 1) == 0) {
            MAC_DeactivateTSMDocument();
            MAC_OnApplicationDeactivate(pEvent);
        } else {
            MAC_OnApplicationActivate(pEvent);
            MAC_ActivateTSMDocument();
        }
        WindowRef pFrontWindow = FrontWindow();
        if (pFrontWindow != NULL) {
            pEvent->what = activateEvt;
            pEvent->modifiers = pEvent->message;
            pEvent->message = (UInt32)pFrontWindow;
            bResult = 1;
        }
    }
    return bResult;
}

// 1.14d mac: 005c8a40
// Set while a command-period is held.
int g_nCmdPeriod;

// 1.14d mac: 0019d42b
// A key down: Cmd-period is noted for Storm, Cmd with a menu key runs the item, anything else
// is posted. A command auto-repeat is dropped.
static void MAC_HandleKeyDownEvent(EventRecord* pEvent) {
    uint16_t nModifiers = pEvent->modifiers;
    UInt32 nMessage = pEvent->message;
    if (pEvent->what != autoKey || (nModifiers & 0x100) == 0) {
        if ((nModifiers & 0x100) == 0) {
            g_nCmdPeriod = 0;
        } else {
            bool bCmdPeriod = (nMessage & 0xFF) == '.';
            g_nCmdPeriod = bCmdPeriod;
            if (bCmdPeriod) {
                MSG_ExchangeCmdPeriodFlag(&g_nCmdPeriod);
                nModifiers = pEvent->modifiers;
            }
        }
        if ((nModifiers & 0x100) != 0) {
            FUN_0019db65();
            nMessage = MenuKey((char)nMessage);
            if ((char)nMessage != 0 && MAC_HandleMenuSelection(nMessage)) {
                return;
            }
        }
        MAC_PostKeyDown(pEvent);
    }
}

OSErr MAC_CheckRequiredAppleEventParams(const AppleEvent* pAppleEvent);

// 1.14d mac: 0019d4ba
// The 'oapp' handler.
OSErr MAC_HandleOpenAppAppleEvent(const AppleEvent* pAppleEvent, AppleEvent* pReply, SRefCon nRefCon) {
    return MAC_CheckRequiredAppleEventParams(pAppleEvent);
}

// 1.14d mac: 0019d4d3
// GotRequiredParams: no missed-keyword attribute is fine, one found is errAEParamMissed.
OSErr MAC_CheckRequiredAppleEventParams(const AppleEvent* pAppleEvent) {
    DescType nReturnedType;
    Size nActualSize;
    OSErr nError = AEGetAttributePtr(pAppleEvent, keyMissedKeywordAttr, typeWildCard, &nReturnedType, NULL, 0, &nActualSize);
    if (nError == errAEDescNotFound) {
        return 0;
    }
    short nResult = errAEParamMissed;
    if (nError != 0) {
        nResult = nError;
    }
    return nResult;
}

// 1.14d mac: 0019d52f
// The 'odoc' handler: nothing to open.
OSErr MAC_HandleOpenDocumentsAppleEvent(const AppleEvent* pAppleEvent, AppleEvent* pReply, SRefCon nRefCon) {
    return 0;
}

// 1.14d mac: 00536c58
// Asks whether quitting is allowed; none means always.
static uint8_t (*s_pfnQuitQuery)(void);

// 1.14d mac: 0019d537
// The 'quit' handler: Quit (menu command 7) unless the application vetoes it.
OSErr MAC_HandleQuitAppleEvent(const AppleEvent* pAppleEvent, AppleEvent* pReply, SRefCon nRefCon) {
    OSErr nError = MAC_CheckRequiredAppleEventParams(pAppleEvent);
    uint8_t bCancelQuit = 0;
    if (s_pfnQuitQuery != NULL) {
        bCancelQuit = s_pfnQuitQuery();
        bCancelQuit = bCancelQuit ^ 1;
    }
    if (nError == 0 && bCancelQuit == 0) {
        FUN_0019bbcd(7);
    }
    return nError;
}

// 1.14d mac: 0019db13
void FUN_0019db13(void) {
    FUN_0019be53(1);
}

// 1.14d mac: 0019db2a
// A start-up failure: the alert, then out.
static void MAC_ShowFatalErrorAndExit(const unsigned char* szMessage) {
    StandardAlert(kAlertStopAlert, szMessage, "\p", NULL, NULL);
    ExitToShell();
}

// 1.14d mac: 0019db65
// Checks item 4 of menu 0x84 from the toggle the menu command keeps.
void FUN_0019db65(void) {
    MenuRef hMenu = GetMenuHandle(0x84);
    if (hMenu != NULL) {
        CheckMenuItem(hMenu, 4, g_bMacMenuToggle);
    }
}

// 1.14d mac: 005c8a3c
// The state of the toggle item of menu 0x84.
uint8_t g_bMacMenuToggle;
// 1.14d mac: 00536c5c
// The reference number of the temporary resource file.
static short s_nResFile;
// 1.14d mac: 00536c5e
// The temporary resource file the game's dialogs and menus come from.
static FSSpec s_tResFileSpec;

// 1.14d mac: 0019d582
// main: process set-up, the game archives, the resource file made from the MPQ's
// D2Resources.rsrc, the menu bar and AppleEvent handlers, then the game itself and the tear-down.
int MAC_Main(void) {
    ProcessSerialNumber tProcess;
    Str255 szFatal = "\pDiablo II has encountered a serious error. Please reinstall Diablo II. (2)";
    char szPath[256];
    char szLang[256];
    char szVersion[260];
    void* hGameData = NULL;
    void* hExpansion = NULL;
    void* hPatch = NULL;
    void* pResourceData;
    uint32_t dwResourceSize;
    short nVRefNum;
    long nDirID;
    FUN_0003e2ab();
    GetCurrentProcess(&tProcess);
    TransformProcessType(&tProcess, kProcessTransformToForegroundApplication);
    MAC_SetWorkingDirectoryToAppFolder();
    SFILE_InitializeFileSystem(0, 0);
    if (SFileOpenArchive("Diablo II Game Data", 100, 0, &hGameData)) {
        BOOL bExpansion = D2CheckExpansionGame();
        if (bExpansion && !SFileOpenArchive("Diablo II Expansion Data", 0x96, 0, &hExpansion)) {
            goto fatal;
        }
        if (hGameData != NULL && (!bExpansion || hExpansion != NULL)) {
            SFileOpenArchive("Diablo II Patch", 200, 0, &hPatch);
            TABLES_GetLangAbbreviation(szLang, 0);
            SStrPrintf(szPath, 0x100, "Data\\Local\\MacUI\\%s\\D2Resources.rsrc", szLang);
            if (SFileLoadFile(szPath, &pResourceData, &dwResourceSize, 0, 0)) {
                SFileCloseArchive(hGameData);
                if (hExpansion != NULL) {
                    SFileCloseArchive(hExpansion);
                }
                if (hPatch != NULL) {
                    SFileCloseArchive(hPatch);
                }
                if (dwResourceSize != 0) {
                    if (FindFolder(kOnSystemDisk, kTemporaryFolderType, true, &nVRefNum, &nDirID) == noErr) {
                        FSMakeFSSpec(nVRefNum, nDirID, "\pDiablo 2 Temporary Data", &s_tResFileSpec);
                        FSpDelete(&s_tResFileSpec);
                        FSpCreateResFile(&s_tResFileSpec, 'Doug', 'RSRC', smSystemScript);
                        short nError = ResError();
                        switch (nError) {
                        case -49:
                        case -48:
                            MAC_ShowFatalErrorAndExit("\pDiablo II cannot be opened multiple times on one computer. (3)");
                            goto done;
                        case -46:
                        case -44:
                            MAC_ShowFatalErrorAndExit("\pDiablo II cannot run from a locked volume. (5)");
                            goto done;
                        case -34:
                        case -33:
                            MAC_ShowFatalErrorAndExit("\pDiablo II requires at least 40MB of free disk space. (4)");
                            FUN_0019be53(1);
                            return 0;
                        }
                        if (nError == 0) {
                            nError = FSpOpenRF(&s_tResFileSpec, fsRdWrPerm, &s_nResFile);
                            if (nError == 0) {
                                long nWritten = dwResourceSize;
                                FSWrite(s_nResFile, &nWritten, pResourceData);
                                bool bFailed = nWritten != (long)dwResourceSize;
                                nError = bFailed;
                                if (!bFailed) {
                                    FSClose(s_nResFile);
                                    s_nResFile = FSpOpenResFile(&s_tResFileSpec, fsRdPerm);
                                    nError = bFailed;
                                    if (s_nResFile == -1) {
                                        nError = 2;
                                    }
                                    SMemFree(pResourceData, NULL, 0, 0);
                                    if (nError == 0) {
                                        Handle hMenuBar = GetNewMBar(0x81);
                                        if (hMenuBar != NULL) {
                                            SetMenuBar(hMenuBar);
                                            DisposeHandle(hMenuBar);
                                            MenuRef hMenu = GetMenuHandle(0x82);
                                            if (hMenu != NULL) {
                                                DeleteMenuItem(hMenu, 3);
                                                DeleteMenuItem(hMenu, 2);
                                            }
                                            DrawMenuBar();
                                        }
                                        AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, MAC_HandleOpenAppAppleEvent, 0, false);
                                        AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, MAC_HandleOpenDocumentsAppleEvent, 0, false);
                                        AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, MAC_HandleQuitAppleEvent, 0, false);
                                        FUN_0019e432();
                                        SStrPrintf(szVersion, 0x104, "v%d.%02d", 1, 14);
                                        FUN_002ee3a4("D2");
                                        FOG_InitErrorManager("Diablo II", szVersion, 1);
                                        g_pfnMacAbortHook = FUN_0019db13;
                                        SetEventMask(0xFFFF);
                                        if (FUN_0019b636()) {
                                            FUN_0019be79();
                                            FUN_0019b744();
                                        }
                                        CloseResFile(s_nResFile);
                                        FSpDelete(&s_tResFileSpec);
                                        MAC_ShutdownTSM();
                                        MACCURSOR_InitCursor();
                                        return 0;
                                    }
                                }
                            }
                        }
                        szFatal[0] = sprintf((char*)szFatal + 1, "Diablo II has encountered a critical error and cannot start. (6, %d)", (int)nError);
                    }
                }
            }
        }
    }
fatal:
    MAC_ShowFatalErrorAndExit(szFatal);
done:
    return 0;
}

// 1.14d mac: 00331f04
// The edit-menu items (3..8) as the shell's menu commands; item 7 is not a command.
static const int s_aEditMenuCommands[9] = {0, 0, 0, 9, 10, 11, 12, -1, 13};

// 1.14d mac: 0019dba3
// A MenuSelect result (menu in the high word, item in the low): About, Quit, the edit
// commands, and the display menu's items.
static uint8_t MAC_HandleMenuSelection(uint32_t nMenuResult) {
    uint8_t bBuffer[973];
    uint32_t nCommand;
    HiliteMenu(0);
    int nMenu = (int)nMenuResult >> 16;
    if (3 < (uint32_t)(nMenu - 0x81)) {
        return 0;
    }
    uint32_t nItem = nMenuResult & 0xFF;
    switch (nMenu) {
    case 0x81:
        if ((short)nItem == 1) {
            MAC_ShowAboutDialog();
        }
        return 0;
    case 0x82:
        nCommand = 4;
        if ((short)nItem != 1) {
            if (nItem != 3) {
                return 0;
            }
            nCommand = 7;
        }
        break;
    case 0x83:
        if (5 < nItem - 3 || nItem - 3 == 4) {
            return 0;
        }
        nCommand = s_aEditMenuCommands[nItem];
        break;
    case 0x84:
        nCommand = 0x81;
        switch (nItem) {
        case 1:
            break;
        case 3:
            SMemZero(bBuffer, sizeof(bBuffer));
            return 0;
        case 4:
            g_bMacMenuToggle = g_bMacMenuToggle == 0;
            return 0;
        case 5:
            nCommand = 0x82;
            break;
        default:
            return 0;
        }
        break;
    }
    return FUN_0019bbcd(nCommand);
}

// 1.14d mac: 0019dcd4
// A key event made from a modifier change, sent to the key handlers.
static void MAC_SynthesizeKeyEvent(EventRecord* pEvent, int nKeyCode, char bPressed) {
    pEvent->message = nKeyCode << 8;
    if (bPressed == 0) {
        pEvent->what = keyUp;
        MAC_PostKeyUp(pEvent);
    } else {
        pEvent->what = keyDown;
        MAC_PostKeyDown(pEvent);
    }
}

// 1.14d mac: 0019dd07
void FUN_0019dd07(void) {
}

// 1.14d mac: 0019dd0c
// How many characters a run of bytes is, a double-byte character counting as one.
int MAC_CountMultiByteChars(const char* pText, int nBytes) {
    uint32_t nLanguage = MISC_GetLanguageCode();
    short nScript = 3;
    if (nLanguage != 12 && nLanguage != 7) {
        if (nLanguage == 6) {
            nScript = 1;
        } else {
            nScript = 0;
        }
    }
    int nCount = 0;
    while (0 < nBytes) {
        nCount++;
        if (CharacterByteType((Ptr)pText, 0, nScript) == -1) {
            nBytes -= 2;
            pText += sizeof(uint16_t);
        } else {
            nBytes -= 1;
            pText += 1;
        }
    }
    return nCount;
}

// 1.14d mac: 0019dd7d
// The text services 'shiw' handler: nothing to do.
OSErr MAC_HandleTSMShowHideInputWindow(const AppleEvent* pAppleEvent, AppleEvent* pReply, SRefCon nRefCon) {
    return 0;
}

// 1.14d mac: 0019dd84
// The text services 'st2p' handler: a fixed position for the input window.
OSErr MAC_HandleTSMOffsetToPos(const AppleEvent* pAppleEvent, AppleEvent* pReply) {
    short nError = 0;
    Point ptPos;
    Fixed nPointSize;
    DescType nActualType;
    long nOffset;
    long nRefCon;
    Size nActualSize;
    *(uint32_t*)&ptPos = 0x75307530;
    nPointSize = 0x5A0000;
    OSErr nResult = AEGetParamPtr(pAppleEvent, 'ofst', typeLongInteger, &nActualType, &nOffset, 4, &nActualSize);
    if (nResult == 0) {
        nResult = AEGetParamPtr(pAppleEvent, 'refc', typeLongInteger, &nActualType, &nRefCon, 4, &nActualSize);
        if (nResult == 0) {
            nResult = AEPutParamPtr(pReply, 'gpos', typeQDPoint, &ptPos, 4);
            if (nResult == 0) {
                nResult = AEPutParamPtr(pReply, 'ptps', typeFixed, &nPointSize, 4);
                if (nResult == 0) {
                    nResult = AEPutParamPtr(pReply, 'errn', typeShortInteger, &nError, 2);
                }
            }
        }
    }
    return nResult;
}

// 1.14d mac: 0019dea7
// The text services 'p2st' handler: offset 0 in the active region.
OSErr MAC_HandleTSMPosToOffset(const AppleEvent* pAppleEvent, AppleEvent* pReply) {
    DescType nActualType;
    long nRefCon;
    Point ptPos;
    Size nActualSize;
    OSErr nResult = AEGetParamPtr(pAppleEvent, 'refc', typeLongInteger, &nActualType, &nRefCon, 4, &nActualSize);
    if (nResult == 0) {
        nResult = AEGetParamPtr(pAppleEvent, 'cpos', typeQDPoint, &nActualType, &ptPos, 4, &nActualSize);
        if (nResult == 0) {
            long nOffset = 0;
            nResult = AEPutParamPtr(pReply, 'ofst', typeLongInteger, &nOffset, 4);
            if (nResult == 0) {
                short nRegionClass = 2;
                nResult = AEPutParamPtr(pReply, 'rgnc', typeShortInteger, &nRegionClass, 2);
            }
        }
    }
    return nResult;
}

void MAC_ConvertTextToUnicode(const char* pSrc, char* wszDest, int nSrcLength, int nDestSize, int nScript);

// 1.14d mac: 003b8d84
// Where the text services' input goes: the text, whether it is committed, and the highlight range.
static void (*s_pfnTSMInput)(const char*, bool, int*) = (void (*)(const char*, bool, int*))FUN_0019dd07;

// 1.14d mac: 0019df93
// The text services 'updt' handler: the active input area's text converted to Unicode and
// handed to the input callback with its highlight range.
OSErr MAC_HandleTSMUpdateActiveInputArea(const AppleEvent* pAppleEvent) {
    char wszText[512];
    int aHiliteRange[2];
    DescType nActualType;
    long nRefCon;
    long nFixedLength;
    Size nActualSize;
    TextRange tPinRange;
    AEDesc tTextDesc = {typeNull, NULL};
    AEDesc tHiliteDesc = {typeNull, NULL};
    AEDesc tUpdateDesc = {typeNull, NULL};
    Ptr pText = NULL;
    Ptr pHiliteRanges = NULL;
    aHiliteRange[1] = 0;
    aHiliteRange[0] = 0;
    bzero(wszText, 0x200);
    OSErr nResult = AEGetParamPtr(pAppleEvent, 'refc', typeLongInteger, &nActualType, &nRefCon, 4, &nActualSize);
    if (nResult == 0) {
        nResult = AEGetParamDesc(pAppleEvent, 'kdat', typeText, &tTextDesc);
        if (nResult == 0) {
            AEGetParamPtr(pAppleEvent, 'fixl', typeLongInteger, &nActualType, &nFixedLength, 4, &nActualSize);
            AEGetParamDesc(pAppleEvent, 'hrng', typeTextRangeArray, &tHiliteDesc);
            AEGetParamDesc(pAppleEvent, 'udng', typeTextRangeArray, &tUpdateDesc);
            AEGetParamPtr(pAppleEvent, 'pnrg', typeTextRange, &nActualType, &tPinRange, 4, &nActualSize);
            Size nTextSize = AEGetDescDataSize(&tTextDesc);
            pText = NewPtr(nTextSize);
            nResult = MemError();
            if (nResult == 0 && (nResult = AEGetDescData(&tTextDesc, pText, nTextSize)) == 0) {
                if (nFixedLength == -1) {
                    nFixedLength = nTextSize;
                }
                if (tHiliteDesc.dataHandle != NULL) {
                    Size nRangesSize = AEGetDescDataSize(&tHiliteDesc);
                    pHiliteRanges = NewPtr(nRangesSize);
                    nResult = MemError();
                    if (nResult != 0 || (nResult = AEGetDescData(&tHiliteDesc, pHiliteRanges, nRangesSize)) != 0) {
                        goto cleanup;
                    }
                    TextRangeArray* pArray = (TextRangeArray*)pHiliteRanges;
                    TextRange* pRange = pArray->fRange;
                    for (int i = pArray->fNumOfRanges; 0 < i; i--) {
                        if (pRange->fHiliteStyle == 5) {
                            aHiliteRange[0] = MAC_CountMultiByteChars(pText, pRange->fStart);
                            aHiliteRange[1] = MAC_CountMultiByteChars(pText, pRange->fEnd);
                            break;
                        }
                        pRange++;
                    }
                }
                int nConvert = 0xFF;
                if (nTextSize < 0x100) {
                    nConvert = nTextSize;
                }
                MAC_ConvertTextToUnicode(pText, wszText, nConvert, 0x200, -0x80);
                s_pfnTSMInput(wszText, nFixedLength == nTextSize, aHiliteRange);
                nResult = 0;
                goto cleanup;
            }
        }
    }
    pText = NULL;
    pHiliteRanges = NULL;
cleanup:
    AEDisposeDesc(&tTextDesc);
    AEDisposeDesc(&tHiliteDesc);
    AEDisposeDesc(&tUpdateDesc);
    DisposePtr(pHiliteRanges);
    DisposePtr(pText);
    return nResult;
}

// 1.14d mac: 003b8d8c
// The script the TEC converter below was made for; none until the first conversion.
static int s_nTECScript = -0x80;
// 1.14d mac: 00536cf0
static TECObjectRef s_hTECConverter;

// 1.14d mac: 0019e2fb
// Converts text in a script (the language's by default, -0x80) to Unicode, with a terminator.
void MAC_ConvertTextToUnicode(const char* pSrc, char* wszDest, int nSrcLength, int nDestSize, int nScript) {
    ByteCount nOutLength;
    ByteCount nSrcRead;
    TextEncoding nEncoding;
    if (nScript == -0x80) {
        uint32_t nLanguage = MISC_GetLanguageCode();
        nScript = 3;
        if (nLanguage != 12 && nLanguage != 7) {
            if (nLanguage == 6) {
                nScript = 1;
            } else {
                nScript = 0;
            }
        }
    }
    if (nScript != s_nTECScript) {
        if (s_hTECConverter != NULL) {
            TECDisposeConverter(s_hTECConverter);
            s_hTECConverter = NULL;
        }
        s_nTECScript = nScript;
        UpgradeScriptInfoToTextEncoding((short)nScript, kTextLanguageDontCare, kTextRegionDontCare, NULL, &nEncoding);
        TextEncoding nUnicodeEncoding = CreateTextEncoding(kTextEncodingUnicodeDefault, 0, 0);
        TECCreateConverter(&s_hTECConverter, nEncoding, nUnicodeEncoding);
    }
    TECConvertText(s_hTECConverter, (ConstTextPtr)pSrc, nSrcLength, &nSrcRead, (TextPtr)wszDest, nDestSize - 2, &nOutLength);
    ByteCount nConverted = nOutLength;
    TECFlushText(s_hTECConverter, (TextPtr)wszDest + nOutLength, (nDestSize - 2) - nOutLength, &nOutLength);
    *(uint16_t*)(wszDest + nConverted + nOutLength) = 0;
}

// 1.14d mac: 00536ca8
// The Script Manager variable 8 as the shell found it.
static long s_nSavedScriptVariable;
// 1.14d mac: 00536ce8
// The text services document, while there is one.
static TSMDocumentID s_hTSMDocument;

// 1.14d mac: 0019e432
// Takes the Script Manager's variable 8 down to 0, installs the four text services AppleEvent
// handlers and drops any text services document.
void FUN_0019e432(void) {
    s_nSavedScriptVariable = GetScriptManagerVariable(8);
    SetScriptManagerVariable(8, 0);
    static AEEventHandlerUPP s_pfnUpdate = (AEEventHandlerUPP)MAC_HandleTSMUpdateActiveInputArea;
    static AEEventHandlerUPP s_pfnShowHide = (AEEventHandlerUPP)MAC_HandleTSMShowHideInputWindow;
    static AEEventHandlerUPP s_pfnPosToOffset = (AEEventHandlerUPP)MAC_HandleTSMPosToOffset;
    static AEEventHandlerUPP s_pfnOffsetToPos = (AEEventHandlerUPP)MAC_HandleTSMOffsetToPos;
    AEInstallEventHandler('tsvc', 'updt', s_pfnUpdate, 0, false);
    AEInstallEventHandler('tsvc', 'shiw', s_pfnShowHide, 0, false);
    AEInstallEventHandler('tsvc', 'p2st', s_pfnPosToOffset, 0, false);
    AEInstallEventHandler('tsvc', 'st2p', s_pfnOffsetToPos, 0, false);
    MAC_DisposeTSMDocument();
}

// 1.14d mac: 0019e5e9
// Fixes, deactivates and deletes the text services document; the keyboard script goes back to
// the system's.
void MAC_DisposeTSMDocument(void) {
    if (s_hTSMDocument != 0) {
        FixTSMDocument(s_hTSMDocument);
        DeactivateTSMDocument(s_hTSMDocument);
        DeleteTSMDocument(s_hTSMDocument);
        s_hTSMDocument = 0;
    }
    KeyScript(0);
    KeyScript(-12);
}

// 1.14d mac: 0019e64a
// Where the text services' input goes; the empty handler for none.
void MAC_SetTSMInputCallback(void (*pfnCallback)(const char*, bool, int*)) {
    void (*pfnInput)(const char*, bool, int*) = (void (*)(const char*, bool, int*))FUN_0019dd07;
    if (pfnCallback != NULL) {
        pfnInput = pfnCallback;
    }
    s_pfnTSMInput = pfnInput;
}

// 1.14d mac: 0019e669
void MAC_DeactivateTSMDocument(void) {
    if (s_hTSMDocument != 0) {
        DeactivateTSMDocument(s_hTSMDocument);
    }
}

// 1.14d mac: 0019e68c
void MAC_ActivateTSMDocument(void) {
    if (s_hTSMDocument != 0) {
        ActivateTSMDocument(s_hTSMDocument);
    }
}

// 1.14d mac: 0019e6af
// Makes the text services document, active and without the input window, and turns the
// keyboard scripts on.
void MAC_CreateTSMDocument(void) {
    if (s_hTSMDocument == 0) {
        OSType nInterfaceType = 'tsvc';
        NewTSMDocument(1, &nInterfaceType, &s_hTSMDocument, 'TSM!');
        ActivateTSMDocument(s_hTSMDocument);
        UseInputWindow(s_hTSMDocument, false);
    }
    KeyScript(-7);
    KeyScript(-3);
}

// 1.14d mac: 0019e736
void MAC_ShutdownTSM(void) {
    MAC_DisposeTSMDocument();
    SetScriptManagerVariable(8, s_nSavedScriptVariable);
}

// 1.14d mac: 003b8d88
// The script the converter below was made for.
static int s_nTECScriptToText = -0x80;
// 1.14d mac: 00536cec
static TECObjectRef s_hTECConverterToText;

// 1.14d mac: 0019e764
// The reverse of MAC_ConvertTextToUnicode: UTF-16 to a script, with a one-byte terminator.
void MAC_ConvertUnicodeToText(const char* wszSrc, char* szDest, int nSrcChars, int nDestSize, int nScript) {
    ByteCount nOutLength;
    ByteCount nSrcRead;
    TextEncoding nEncoding;
    if (nScript == -0x80) {
        uint32_t nLanguage = MISC_GetLanguageCode();
        nScript = 3;
        if (nLanguage != 12 && nLanguage != 7) {
            if (nLanguage == 6) {
                nScript = 1;
            } else {
                nScript = 0;
            }
        }
    }
    if (nScript != s_nTECScriptToText) {
        if (s_hTECConverterToText != NULL) {
            TECDisposeConverter(s_hTECConverterToText);
            s_hTECConverterToText = NULL;
        }
        s_nTECScriptToText = nScript;
        TextEncoding nUnicodeEncoding = CreateTextEncoding(kTextEncodingUnicodeDefault, 0, 0);
        UpgradeScriptInfoToTextEncoding((short)s_nTECScriptToText, kTextLanguageDontCare, kTextRegionDontCare, NULL, &nEncoding);
        TECCreateConverter(&s_hTECConverterToText, nUnicodeEncoding, nEncoding);
    }
    TECConvertText(s_hTECConverterToText, (ConstTextPtr)wszSrc, nSrcChars * 2, &nSrcRead, (TextPtr)szDest, nDestSize - 1, &nOutLength);
    ByteCount nConverted = nOutLength;
    TECFlushText(s_hTECConverterToText, (TextPtr)szDest + nOutLength, (nDestSize - 1) - nOutLength, &nOutLength);
    szDest[nConverted + nOutLength] = 0;
}

// One command-line switch of the table below: its name, what it takes (0 a flag, 1 a number,
// 2 a string) and where its value goes in the configuration.
struct __attribute__((packed)) MacSwitchEntry {
    char szName[0x1B];
    uint8_t byType;
    uint16_t unk1C;
    int32_t nFieldOffset;
    uint8_t unk22[0x3A];
};
// 1.14d mac: 003b8dc6
extern MacSwitchEntry g_aMacSwitches[58];

// 1.14d mac: 0019e8a1
// The switches of a command line ("-name value") into the configuration: each is lowercased,
// matched against the switch table (a prefix of it, shortest first), and its value stored.
void MAC_ParseCommandLine(MacDisplayState* pConfig, const char* szCommandLine) {
    char szValue[256];
    char szSwitchName[256];
    char szSwitch[256];
    char szValueCopy[256];
    MISC_InitIgnorelist();
    int nLength = strlen(szCommandLine);
    if (0 < nLength) {
        int nPos = 0;
        do {
            int nNext = nPos;
            if (szCommandLine[nPos] == '-' && (nNext = nPos + 1) < nLength) {
                int nRemaining = strlen(szCommandLine + nNext);
                int nCopied;
                if (nRemaining < 0x100) {
                    nCopied = 0;
                    if (0 < nRemaining) {
                        goto copy;
                    }
                } else {
                    fp_fatal("Unrecoverable internal error %08x line %d", FOG_GetFramePointer(), 0x103);
                copy:
                    nCopied = 0;
                    do {
                        char cChar = szCommandLine[nCopied + nPos + 1];
                        if (cChar == '\0' || cChar == '-') {
                            break;
                        }
                        szSwitch[nCopied] = cChar;
                        nCopied++;
                    } while (nCopied < nRemaining);
                }
                szSwitch[nCopied] = '\0';
                szSwitch[nCopied + 1] = '\0';
                strcpy(szSwitchName, szSwitch);
                size_t nNameLength = strlen(szSwitchName);
                char* pChar = szSwitchName;
                char cChar = szSwitchName[0];
                while (pChar++, cChar != '\0') {
                    if ((uint8_t)(cChar - 'A') < 26) {
                        pChar[-1] = cChar + ' ';
                    }
                    cChar = *pChar;
                }
                bool bSearching = true;
                uint32_t nSwitchIndex = 0xFFFFFFFF;
                if (nNameLength != 0) {
                    do {
                        szSwitchName[nNameLength] = '\0';
                        nSwitchIndex = 0xFFFFFFFF;
                        const char* pszName = g_aMacSwitches[0].szName;
                        int nCompare;
                        do {
                            nSwitchIndex++;
                            if (0x39 < nSwitchIndex) {
                                nSwitchIndex = 0xFFFFFFFF;
                                break;
                            }
                            nCompare = strcmp(pszName, szSwitchName);
                            pszName += sizeof(g_aMacSwitches[0]);
                        } while (nCompare != 0);
                        if (nSwitchIndex != 0xFFFFFFFF) {
                            bSearching = false;
                        }
                        nNameLength--;
                    } while (nNameLength != 0 && bSearching);
                }
                nNameLength = strlen(szSwitchName);
                int nValuePos = 0;
                int nValueLength = 0;
                if (szSwitch[0] != '\0') {
                    nValueLength = 0;
                    do {
                        szValue[nValueLength] = szSwitch[nValueLength + nNameLength];
                        int nNextChar = nValueLength + 1;
                        nValueLength++;
                        if (szSwitch[nNextChar] == '\0') {
                            break;
                        }
                    } while (true);
                }
                szValue[nValueLength] = '\0';
                strcpy(szValueCopy, szValue);
                size_t nTotal = strlen(szValue);
                if (0 < (int)nTotal) {
                    do {
                        int nSep = 0;
                        const char* pszSeparators = " \t\n:";
                        do {
                            if (3 < nSep) {
                                goto trimmed;
                            }
                            nSep++;
                            cChar = *pszSeparators++;
                        } while (cChar != szValue[nValuePos]);
                        nValuePos++;
                    } while (nValuePos < (int)nTotal);
                }
            trimmed:
                nValueLength = 0;
                for (; nValuePos < (int)nTotal; nValuePos++) {
                    int nSep = 0;
                    const char* pszSeparators = " \t\n:";
                    while (nSep < 4) {
                        nSep++;
                        cChar = *pszSeparators++;
                        if (cChar == szValueCopy[nValuePos]) {
                            goto end;
                        }
                    }
                    szValue[nValueLength] = szValueCopy[nValuePos];
                    nValueLength++;
                }
            end:
                szValue[nValueLength] = '\0';
                if (nSwitchIndex != 0xFFFFFFFF) {
                    int* pField = (int*)((char*)pConfig + g_aMacSwitches[nSwitchIndex].nFieldOffset);
                    uint8_t byType = g_aMacSwitches[nSwitchIndex].byType;
                    if (byType == 2) {
                        strcpy((char*)pField, szValue);
                    } else if (byType == 1) {
                        *pField = atoi(szValue);
                    } else if (byType == 0) {
                        *(uint8_t*)pField = 1;
                    }
                }
            }
            nPos = nNext + 1;
        } while (nPos < nLength);
    }
}

// 1.14d mac: 0019f18a
// Sleeps in 10 ms steps, at least ten of them and at least a quarter of a second.
void FUN_0019f18a(void) {
    uint32_t dwStartTick = GetTickCount();
    uint32_t nSleepCount = 0;
    do {
        if (9 < nSleepCount) {
            uint32_t dwTick = GetTickCount();
            if (-1 < (int)(dwTick + (-0xFA - dwStartTick))) {
                return;
            }
        }
        nSleepCount++;
        Sleep(10);
    } while (true);
}

// 1.14d mac: 0019f1c4
// Makes the folder the application lives in the working directory.
void MAC_SetWorkingDirectoryToAppFolder(void) {
    struct {
        short nVRefNum;
        long nDirID;
    } tFolder;
    FSSpec tSpec;
    FSRef tRef;
    char szPath[1024];
    GetApplicationInformation(NULL, &tFolder.nVRefNum, NULL, NULL);
    FSMakeFSSpec(tFolder.nVRefNum, tFolder.nDirID, "\p", &tSpec);
    FSpMakeFSRef(&tSpec, &tRef);
    CFURLRef pUrl = CFURLCreateFromFSRef(NULL, &tRef);
    CFURLGetFileSystemRepresentation(pUrl, true, (UInt8*)szPath, 0x400);
    if (chdir(szPath) == -1) {
        MAC_OutputDebugStringF("Couldn't set the working directory!");
    }
}

#endif  // D2_OS_MAC
