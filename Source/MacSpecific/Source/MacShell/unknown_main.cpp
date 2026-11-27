// unknown_main.cpp - the Mac shell's main and event loop: MAC_Main (process set-up, archives,
// the resource file, menu bar and AppleEvent handlers, the game, the tear-down), the Carbon
// event loop and its key, modifier, suspend/resume and menu handling.
//
// 1.14d Intel Mac (DiabloII_macho) 0019cf88..0019dd07. No binary records this unit's name.
// Starts at the padding to 0019cf88; MAC_ProcessEvents is followed by the file-local handlers
// it calls (register arguments), MAC_Main by its own, and the unit ends with the last of them
// (0019dcd4). It defines 005c8a3c..44, 00536c58..5e, 00536ca4 and 003b8d80, and calls the
// input and text-services units' one-line functions out of line. See
// Source/MacSpecific/README.md.
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
namespace Fog {
// 1.14d mac: 002edc4a
BOOL D2CheckExpansionGame(void);
}
using Fog::D2CheckExpansionGame;
// 1.14d mac: 00029946
uint32_t MSG_ExchangeCmdPeriodFlag(const uint32_t* pnFlag);
namespace D2Lang::StrTable::strtable {
// 1.14d mac: 002b8fba
void TABLES_GetLangAbbreviation(char* szOut, int nLanguage);
}
using D2Lang::StrTable::strtable::TABLES_GetLangAbbreviation;
// 1.14d mac: 002ee3a4
namespace Fog {
namespace Source {
namespace SFile {
void FILETOOLS_SetLogFilePathWithoutDateAndExtension2(char* szSuffix);
}
}
}
// 1.14d mac: 0003e2ab
void FUN_0003e2ab(void);
// 1.14d mac: 00396200
// The cell is the exit hook fp_fatal calls (g_pfnFatalExitHook, unknown_appinfo.cpp, 00554e48).
extern void (*g_pfnMacAbortHook)(void) __asm__("_g_pfnFatalExitHook");
// 1.14d mac: 0019e669
void MAC_DeactivateTSMDocument(void);
// 1.14d mac: 0019e68c
void MAC_ActivateTSMDocument(void);
// 1.14d mac: 0019e736
void MAC_ShutdownTSM(void);
static void MAC_ShowFatalErrorAndExit(const unsigned char* szMessage);
void FUN_0019db13(void);
// 1.14d mac: 0019be53
void FUN_0019be53(int nExitCode);
// 1.14d mac: 0019ba3d
bool MAC_PostKeyDown(EventRecord* pEvent);
// 1.14d mac: 0019b7ae
uint8_t MAC_HandleMouseDownEvent(EventRecord* pEvent);
// 1.14d mac: 0019b7e2
uint8_t MAC_HandleMouseMoved(EventRecord* pEvent);
// 1.14d mac: 0019ba0c
uint8_t MAC_HandleMouseUpEvent(EventRecord* pEvent);
// 1.14d mac: 0019bba4
bool MAC_HandleUpdateEvent(EventRecord* pEvent);
// 1.14d mac: 0019ba81
bool MAC_PostKeyUp(EventRecord* pEvent);
// 1.14d mac: 0019bac5
uint8_t FUN_0019bac5(EventRecord* pEvent);
// 1.14d mac: 0019bbcd
uint8_t FUN_0019bbcd(uint32_t nCommand);
extern uint8_t g_bMacMenuToggle;
// 1.14d mac: 0019e432
void FUN_0019e432(void);
// 1.14d mac: 0019f1c4
void MAC_SetWorkingDirectoryToAppFolder(void);
// 1.14d mac: 00036ff5
void MACCURSOR_InitCursor(void);
// 1.14d mac: 0019bacc
bool MAC_OnApplicationDeactivate(EventRecord* pEvent);
// 1.14d mac: 0019bb55
bool MAC_OnApplicationActivate(EventRecord* pEvent);
// File-local (its argument comes in ecx) yet called out of line by its one caller; clang-425
// inlines a one-caller static unless told not to.
static __attribute__((noinline)) void MAC_HandleKeyDownEvent(EventRecord* pEvent);
static uint8_t MAC_HandleSuspendResumeEvent(EventRecord* pEvent);
void FUN_0019db65(void);
static void MAC_SynthesizeKeyEvent(EventRecord* pEvent, int nKeyCode, char bPressed);
static uint8_t MAC_HandleMenuSelection(uint32_t nMenuResult);
namespace Fog::Source::SFile {
// 1.14d mac: 002ef3d4
void SFILE_InitializeFileSystem(int nArg1, int nArg2);
}
using Fog::Source::SFile::SFILE_InitializeFileSystem;
// ---- the SReg, Win32Mac and D2gfx entry points the shell calls ----
// 1.14d mac: 0019b795
int FUN_0019b795(void);
// 1.14d mac: 0019b74d
uint8_t FUN_0019b74d(EventRecord* pEvent);
// 1.14d mac: 0019b636
bool FUN_0019b636(void);
// 1.14d mac: 0019be79
void FUN_0019be79(void);
// 1.14d mac: 0019b744
void FUN_0019b744(void);
// 1.14d mac: 0019a59a
void MAC_ShowAboutDialog(void);

namespace Fog::Src::ErrorManager {
// 1.14d mac: 002eed78
void FOG_InitErrorManager(void* pUnused, char* szText, int bStoreStartupInfo);
}
using Fog::Src::ErrorManager::FOG_InitErrorManager;

// 1.14d mac: 005c8a44
// Set while a modal dialog has the events. Defined here: MAC_ProcessEvents addresses it directly.
int g_bMacModalActive;

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
            } else {
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
                case activateEvt:
                    break;
                case osEvt:
                    MAC_HandleSuspendResumeEvent(&tEvent);
                    break;
                case kHighLevelEvent:
                    AEProcessAppleEvent(&tEvent);
                    break;
                default:
                    FUN_0019bac5(&tEvent);
                    break;
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
    if (pEvent->what != autoKey || (nModifiers & 0x100) == 0) {
        UInt32 nMessage = pEvent->message;
        if ((nModifiers & 0x100) == 0) {
            g_nCmdPeriod = 0;
        } else {
            bool bCmdPeriod = (nMessage & 0xFF) == '.';
            g_nCmdPeriod = bCmdPeriod;
            if (bCmdPeriod) {
                MSG_ExchangeCmdPeriodFlag((const uint32_t*)&g_nCmdPeriod);
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
// GotRequiredParams: no missed-keyword attribute is fine, one found is errAEEventNotHandled.
OSErr MAC_CheckRequiredAppleEventParams(const AppleEvent* pAppleEvent) {
    Size nActualSize;
    DescType nReturnedType;
    OSErr nError = AEGetAttributePtr(pAppleEvent, keyMissedKeywordAttr, typeWildCard, &nReturnedType, NULL, 0, &nActualSize);
    short nResult = 0;
    if (nError != errAEDescNotFound) {
        nResult = errAEEventNotHandled;
        if (nError != 0) {
            nResult = nError;
        }
    }
    return nResult;
}

// 1.14d mac: 0019d52f
// The 'odoc' handler: nothing to open.
OSErr MAC_HandleOpenDocumentsAppleEvent(const AppleEvent* pAppleEvent, AppleEvent* pReply, SRefCon nRefCon) {
    return 0;
}

typedef bool (*MacQuitQueryProc)(void);
// 1.14d mac: 00536c58
// Asks whether quitting is allowed; none means always.
MacQuitQueryProc g_pfnMacQuitQuery;

// 1.14d mac: 0019d537
// The 'quit' handler: Quit (menu command 7) unless the application vetoes it.
OSErr MAC_HandleQuitAppleEvent(const AppleEvent* pAppleEvent, AppleEvent* pReply, SRefCon nRefCon) {
    OSErr nError = MAC_CheckRequiredAppleEventParams(pAppleEvent);
    bool bCancelQuit = false;
    if (g_pfnMacQuitQuery != NULL) {
        bCancelQuit = !g_pfnMacQuitQuery();
    }
    if (nError == 0 && !bCancelQuit) {
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
// crt1's start calls it with argc, argv, envp and the apple vector, none of which it reads.
int main(int argc, char** argv, char** envp, char** apple) {
    void* hGameData;
    void* hExpansion;
    void* hPatch;
    void* pResourceData;
    uint32_t dwResourceSize;
    short nVRefNum;
    long nDirID;
    long nWritten;
    ProcessSerialNumber tProcess;
    FUN_0003e2ab();
    GetCurrentProcess(&tProcess);
    TransformProcessType(&tProcess, kProcessTransformToForegroundApplication);
    MAC_SetWorkingDirectoryToAppFolder();
    Str255 szFatal = "\pDiablo II has encountered a serious error. Please reinstall Diablo II. (2)";
    SFILE_InitializeFileSystem(0, 0);
    hGameData = NULL;
    if (SFileOpenArchive("Diablo II Game Data", 100, 0, &hGameData)) {
        BOOL bExpansion = D2CheckExpansionGame();
        hExpansion = NULL;
        if (bExpansion && !SFileOpenArchive("Diablo II Expansion Data", 0x96, 0, &hExpansion)) {
            goto fatal;
        }
        if (hGameData != NULL && (!bExpansion || hExpansion != NULL)) {
            hPatch = NULL;
            SFileOpenArchive("Diablo II Patch", 200, 0, &hPatch);
            char szPath[256];
            char szLang[256];
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
                    if (FindFolder(kOnSystemDisk, kChewableItemsFolderType, true, &nVRefNum, &nDirID) == noErr) {
                        FSMakeFSSpec(nVRefNum, nDirID, "\pDiablo 2 Temporary Data", &s_tResFileSpec);
                        FSpDelete(&s_tResFileSpec);
                        FSpCreateResFile(&s_tResFileSpec, 'Doug', 'RSRC', smSystemScript);
                        short nError = ResError();
                        switch (nError) {
                        case -49:
                        case -48:
                            MAC_ShowFatalErrorAndExit("\pDiablo II cannot be opened multiple times on one computer. (3)");
                        case -46:
                        case -44:
                            MAC_ShowFatalErrorAndExit("\pDiablo II cannot run from a locked volume. (5)");
                        case -34:
                        case -33:
                            MAC_ShowFatalErrorAndExit("\pDiablo II requires at least 40MB of free disk space. (4)");
                        }
                        if (nError == 0) {
                            nError = FSpOpenRF(&s_tResFileSpec, fsWrPerm, &s_nResFile);
                            if (nError == 0) {
                                nWritten = dwResourceSize;
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
                                        char szVersion[260];
                                        SStrPrintf(szVersion, 0x104, "v%d.%02d", 1, 14);
                                        Fog::Source::SFile::FILETOOLS_SetLogFilePathWithoutDateAndExtension2((char*)"D2");
                                        FOG_InitErrorManager((void*)"Diablo II", szVersion, 1);
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
    default:
        return 0;
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

#endif  // D2_OS_MAC
