// StormMac: the modal dialog helpers, 1.14d Intel Mac 0003555c..0003602b: a ModalDialog
// replacement (WaitNextEvent loop with filter, idle and menu callbacks) and the menu
// handling that goes with it: the menus are disabled while a dialog is up, the Edit menu
// follows the dialog's text field.
// The original file name is not recorded: no 1.14d string or static initialiser names it.
#include "Source/StormMac/Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include <Carbon/Carbon.h>

// What the dialog code keeps about the menus, 0x24 bytes. The menu ids and item numbers
// of the Edit menu are the caller's; the saved bits restore what the dialog disabled.
struct DlgMenuState {
    BYTE pad00[8];                // 0x00
    short nMenuID;                // 0x08  first menu (items 0..14 saved and disabled)
    unsigned short nMenuItems;    // 0x0A  its item count
    short nEditMenuID;            // 0x0C  the Edit menu
    BYTE pad0E[2];                // 0x0E
    short nCutItem;               // 0x10
    short nCopyItem;              // 0x12
    short nPasteItem;             // 0x14
    short nClearItem;             // 0x16
    short nSelectAllItem;         // 0x18
    unsigned short wSavedBits;    // 0x1A  first menu's enabled bits (0x3FFF), bit 14: scrap holds text, bit 15: Edit menu enabled
    DWORD dwEditSavedBits;        // 0x1C  the Edit menu's enabled bits
    WindowRef pWindow;            // 0x20
};
WIN32MAC_LAYOUT(DlgMenuState_size, sizeof(DlgMenuState) == 0x24);
WIN32MAC_LAYOUT(DlgMenuState_window, offsetof(DlgMenuState, pWindow) == 0x20);

typedef Boolean (*DlgFilterProc)(DialogPtr pDialog, EventRecord* pEvent, short* pItemHit);
typedef void (*DlgIdleProc)(DialogPtr pDialog, DlgMenuState* pMenuState);
typedef char (*DlgMenuProc)(DialogPtr pDialog, DlgMenuState* pMenuState, long nMenuResult, short* pItemHit);

// The 10.7 SDK headers this layer builds against do not declare these calls.
extern "C" {
void GetPort(GrafPtr* pPort);
void SetPort(GrafPtr pPort);
void SetPortWindowPort(WindowRef pWindow);
RgnHandle GetGrayRgn(void);
Rect* GetRegionBounds(RgnHandle hRgn, Rect* pBounds);
void DragWindow(WindowRef pWindow, Point tStart, const Rect* pBounds);
void BeginUpdate(WindowRef pWindow);
void EndUpdate(WindowRef pWindow);
Boolean StdFilterProc(DialogPtr pDialog, EventRecord* pEvent, short* pItemHit);
Boolean IsDialogEvent(const EventRecord* pEvent);
Boolean DialogSelect(const EventRecord* pEvent, DialogPtr* pDialogHit, short* pItemHit);
void DialogCut(DialogPtr pDialog);
void DialogCopy(DialogPtr pDialog);
void DialogPaste(DialogPtr pDialog);
void DialogDelete(DialogPtr pDialog);
}

// 1.14d mac: 000356df
static void Dlg_UpdateEditMenu(DialogPtr pDialog, DlgMenuState* pMenuState, char bRefreshScrap);
// 1.14d mac: 00035f34
static void Dlg_HandleEditMenuCommand(DialogPtr pDialog, DlgMenuState* pMenuState, long nMenuResult, short* pItemHit);

// 1.14d mac: 0003555c
// Saves the enabled bits of the two menus into the record, disables their items and
// redraws the menu bar; runs before a modal dialog.
void Dlg_DisableMenusForModal(DialogPtr pDialog, WindowRef pWindow, DlgMenuState* pMenuState) {
    if (pDialog != NULL && pMenuState != NULL) {
        pMenuState->wSavedBits = 0;
        pMenuState->pWindow = pWindow;
        pMenuState->dwEditSavedBits = 0;
        if (pMenuState->nMenuID > 0) {
            MenuRef hMenu = GetMenuHandle(pMenuState->nMenuID);
            if (hMenu != NULL) {
                DWORD dwEnabledBits = 0;
                short nItem = 0;
                int nShift = 0;
                int nStep = 0x10000;
                do {
                    dwEnabledBits |= (IsMenuItemEnabled(hMenu, nItem) & 0xFF) << (nShift & 0x1F);
                    nItem++;
                    nShift = nStep >> 16;
                    nStep += 0x10000;
                } while (nStep != 0x110000);
                pMenuState->wSavedBits |= (unsigned short)dwEnabledBits & 0x3FFF;
                EnableMenuItem(hMenu, 0);
                unsigned short nItems = pMenuState->nMenuItems;
                unsigned short nLastItem = 0xE;
                if (nItems < 0xF) {
                    nLastItem = nItems;
                }
                if ((short)nLastItem > 0) {
                    short nDisable = 1;
                    do {
                        DisableMenuItem(hMenu, nDisable);
                        nDisable++;
                    } while (nDisable <= (short)nLastItem);
                }
            }
        }
        if (pMenuState->nEditMenuID > 0) {
            MenuRef hMenu = GetMenuHandle(pMenuState->nEditMenuID);
            if (hMenu != NULL) {
                short nItem = 0;
                DWORD dwEnabledBits = 0;
                do {
                    dwEnabledBits |= (IsMenuItemEnabled(hMenu, nItem) & 0xFF) << (nItem & 0x1F);
                    nItem++;
                } while (nItem != 0x20);
                pMenuState->dwEditSavedBits = dwEnabledBits;
                unsigned short nItemCount = CountMenuItems(hMenu);
                unsigned short nLastItem = 0x20;
                if (nItemCount < 0x21) {
                    nLastItem = nItemCount;
                }
                if ((short)nLastItem >= 0) {
                    short nDisable = 0;
                    do {
                        DisableMenuItem(hMenu, nDisable);
                        nDisable++;
                    } while (nDisable <= (short)nLastItem);
                }
            }
        }
        DrawMenuBar();
        Dlg_UpdateEditMenu(pDialog, pMenuState, 1);
    }
}

// 1.14d mac: 000356df
// Enables or disables Cut, Copy, Paste, Clear and Select All from the text field's
// selection, the scrap's TEXT flavor and the text length.
static void Dlg_UpdateEditMenu(DialogPtr pDialog, DlgMenuState* pMenuState, char bRefreshScrap) {
    if (pMenuState->nEditMenuID < 1) {
        return;
    }
    MenuRef hMenu = GetMenuHandle(pMenuState->nEditMenuID);
    if (hMenu == NULL) {
        return;
    }
    bool bAnyEnabled = false;
    short nFocus = GetDialogKeyboardFocusItem(pDialog);
    if (nFocus >= 1) {
        TEHandle hTE = GetDialogTextEditHandle(pDialog);
        TEPtr pTE = *hTE;
        short nSelStart = pTE->selStart;
        short nSelEnd = pTE->selEnd;
        short nTextLength = pTE->teLength;
        short nMenuItem = pMenuState->nCutItem;
        if (nMenuItem > 0) {
            if (nSelEnd != nSelStart) {
                EnableMenuItem(hMenu, nMenuItem);
                bAnyEnabled = true;
            } else {
                DisableMenuItem(hMenu, nMenuItem);
            }
        }
        nMenuItem = pMenuState->nCopyItem;
        if (nMenuItem > 0) {
            if (nSelEnd != nSelStart) {
                EnableMenuItem(hMenu, nMenuItem);
                bAnyEnabled = true;
            } else {
                DisableMenuItem(hMenu, nMenuItem);
            }
        }
        nMenuItem = pMenuState->nPasteItem;
        if (nMenuItem > 0) {
            unsigned short wScrapText = pMenuState->wSavedBits & 0x4000;
            if (bRefreshScrap != 0) {
                ScrapRef hScrap;
                Size nScrapSize = 0;
                GetCurrentScrap(&hScrap);
                GetScrapFlavorSize(hScrap, 'TEXT', &nScrapSize);
                wScrapText = (unsigned short)(nScrapSize > 0) << 14;
                pMenuState->wSavedBits = (pMenuState->wSavedBits & 0xBFFF) | wScrapText;
                nMenuItem = pMenuState->nPasteItem;
            }
            if (wScrapText != 0) {
                EnableMenuItem(hMenu, nMenuItem);
                bAnyEnabled = true;
            } else {
                DisableMenuItem(hMenu, nMenuItem);
            }
        }
        nMenuItem = pMenuState->nClearItem;
        if (nMenuItem > 0) {
            if (nSelEnd != nSelStart) {
                EnableMenuItem(hMenu, nMenuItem);
                bAnyEnabled = true;
            } else {
                DisableMenuItem(hMenu, nMenuItem);
            }
        }
        short nSelectAll = pMenuState->nSelectAllItem;
        if (nSelectAll > 0) {
            if (nTextLength >= 1) {
                EnableMenuItem(hMenu, nSelectAll);
                bAnyEnabled = true;
            } else {
                DisableMenuItem(hMenu, nSelectAll);
            }
        }
    }
    bool bSavedEnabled = (short)pMenuState->wSavedBits >= 0;
    if (bAnyEnabled && bSavedEnabled) {
        EnableMenuItem(hMenu, 0);
        pMenuState->wSavedBits |= 0x8000;
        DrawMenuBar();
        return;
    }
    if (bAnyEnabled) {
        return;
    }
    if (bSavedEnabled) {
        return;
    }
    DisableMenuItem(hMenu, 0);
    pMenuState->wSavedBits &= 0x7FFF;
    DrawMenuBar();
}

// 1.14d mac: 000358fe
// The inverse of Dlg_DisableMenusForModal: re-enables the items whose saved bit is set.
void Dlg_RestoreMenus(DialogPtr pDialog, DlgMenuState* pMenuState) {
    if (pDialog != NULL && pMenuState != NULL) {
        if (pMenuState->nMenuID > 0) {
            MenuRef hMenu = GetMenuHandle(pMenuState->nMenuID);
            if (hMenu != NULL) {
                unsigned short nLastItem = 0xE;
                if (pMenuState->nMenuItems < 0xF) {
                    nLastItem = pMenuState->nMenuItems;
                }
                if ((short)nLastItem >= 0) {
                    unsigned short wBits = pMenuState->wSavedBits;
                    DWORD dwItem = 0;
                    do {
                        if ((((wBits & 0x3FFF) >> (dwItem & 0x1F)) & 1) != 0) {
                            EnableMenuItem(hMenu, dwItem & 0xFFFF);
                        }
                        dwItem++;
                    } while ((short)dwItem <= (short)nLastItem);
                }
            }
        }
        if (pMenuState->nEditMenuID > 0) {
            MenuRef hMenu = GetMenuHandle(pMenuState->nEditMenuID);
            if (hMenu != NULL) {
                unsigned short nCount = CountMenuItems(hMenu);
                unsigned short nLastItem = 0x20;
                if (nCount < 0x21) {
                    nLastItem = nCount;
                }
                if ((short)nLastItem >= 0) {
                    DWORD dwBits = pMenuState->dwEditSavedBits;
                    DWORD dwItem = 0;
                    do {
                        if (((dwBits >> (dwItem & 0x1F)) & 1) != 0) {
                            EnableMenuItem(hMenu, dwItem & 0xFFFF);
                        }
                        dwItem++;
                    } while ((short)dwItem <= (short)nLastItem);
                }
            }
        }
        DrawMenuBar();
    }
}

// 1.14d mac: 000359e3
// WaitNextEvent loop with filter, idle and menu callbacks, StdFilterProc, MenuSelect and
// MenuKey, window drag and update handling; runs until an item is hit.
void Dlg_ModalDialogLoop(DialogPtr pDialog, DlgMenuState* pMenuState, DlgFilterProc pfnFilter, short* pItemHit,
                         DlgIdleProc pfnIdle, DlgMenuProc pfnMenu, long nSleepTicks) {
    EventRecord tEvent;
    short nItemHit;
    GrafPtr pSavedPort;
    if (pItemHit == NULL) {
        return;
    }
    *pItemHit = 0;
    if (pDialog == NULL || pMenuState == NULL) {
        return;
    }
    GetPort(&pSavedPort);
    SetPortWindowPort(GetDialogWindow(pDialog));
    nItemHit = 0;
    if (GetDialogWindow(pDialog) == FrontWindow()) {
        char cResult = 0;
        do {
            Boolean bGotEvent = WaitNextEvent(0xFFFF, &tEvent, nSleepTicks, NULL);
            Dlg_UpdateEditMenu(pDialog, pMenuState, bGotEvent);
            if (pfnIdle != NULL) {
                pfnIdle(pDialog, pMenuState);
            }
            if (pfnFilter != NULL) {
                cResult = pfnFilter(pDialog, &tEvent, &nItemHit);
            }
            if (cResult != 0 || (cResult = StdFilterProc(pDialog, &tEvent, &nItemHit)) != 0) {
                continue;
            }
            WindowRef pEventWindow = (WindowRef)tEvent.message;
            bool bSelectWindow = false;
            bool bHandled = false;
            if (tEvent.what > 0x16) {
                if (tEvent.what == 0x17) {
                    AEProcessAppleEvent(&tEvent);
                    bHandled = true;
                }
            } else if (tEvent.what < 0xF) {
                switch (tEvent.what) {
                case mouseDown: {
                    WindowRef pHit;
                    short nPart = FindWindow(tEvent.where, &pHit);
                    if (nPart != inMenuBar) {
                        if (nPart == inContent) {
                            if (pHit == GetDialogWindow(pDialog)) {
                                break;
                            }
                        } else if (nPart == inDrag && (pHit == GetDialogWindow(pDialog) || pHit == pMenuState->pWindow)) {
                            Rect tBounds;
                            GetRegionBounds(GetGrayRgn(), &tBounds);
                            DragWindow(pHit, tEvent.where, &tBounds);
                            bHandled = true;
                            break;
                        }
                        SysBeep(10);
                        bHandled = true;
                        break;
                    }
                    long nMenuResult = MenuSelect(tEvent.where);
                    cResult = 1;
                    if ((unsigned long)nMenuResult < 0x10000) {
                        continue;
                    }
                    if (pfnMenu == NULL || (cResult = pfnMenu(pDialog, pMenuState, nMenuResult, &nItemHit)) == 0) {
                        Dlg_HandleEditMenuCommand(pDialog, pMenuState, nMenuResult, &nItemHit);
                    }
                    cResult = 1;
                    HiliteMenu(0);
                    continue;
                }
                case keyDown:
                    if ((tEvent.modifiers & 0x100) != 0) {
                        long nMenuResult = MenuKey(tEvent.message & 0xFF);
                        cResult = 1;
                        if ((unsigned long)nMenuResult < 0x10000) {
                            continue;
                        }
                        UInt32 nTicks = TickCount();
                        if ((pfnMenu == NULL || (cResult = pfnMenu(pDialog, pMenuState, nMenuResult, &nItemHit)) == 0) &&
                            (nMenuResult >> 16) != 0 && (nMenuResult >> 16) == pMenuState->nEditMenuID &&
                            (short)nMenuResult > 0) {
                            MenuRef hMenu = GetMenuHandle(pMenuState->nEditMenuID);
                            if (hMenu != NULL && IsMenuItemEnabled(hMenu, 0) && GetDialogKeyboardFocusItem(pDialog) > 0) {
                                short nItem = (short)nMenuResult;
                                bool bEnabled;
                                bool bKnown = true;
                                if (nItem == pMenuState->nCutItem || nItem == pMenuState->nCopyItem ||
                                    nItem == pMenuState->nClearItem) {
                                    TEHandle hTE = GetDialogTextEditHandle(pDialog);
                                    bEnabled = (*hTE)->selEnd != (*hTE)->selStart;
                                } else if (nItem == pMenuState->nPasteItem) {
                                    bEnabled = (pMenuState->wSavedBits >> 14) & 1;
                                } else if (nItem == pMenuState->nSelectAllItem) {
                                    TEHandle hTE = GetDialogTextEditHandle(pDialog);
                                    bEnabled = (*hTE)->teLength > 0;
                                } else {
                                    bKnown = false;
                                    bEnabled = false;
                                }
                                if (bKnown && bEnabled) {
                                    Dlg_HandleEditMenuCommand(pDialog, pMenuState, nMenuResult, &nItemHit);
                                }
                            }
                        }
                        long nDelay = nTicks - TickCount() + 8;
                        unsigned long nFinal;
                        Delay(nDelay, &nFinal);
                        cResult = 1;
                        HiliteMenu(0);
                        continue;
                    }
                    break;
                case updateEvt:
                    if (pEventWindow != GetDialogWindow(pDialog)) {
                        GrafPtr pUpdatePort;
                        GetPort(&pUpdatePort);
                        SetPortWindowPort(pEventWindow);
                        BeginUpdate(pEventWindow);
                        EndUpdate(pEventWindow);
                        SetPort(pUpdatePort);
                        bHandled = true;
                    }
                    break;
                case activateEvt:
                    if (pEventWindow == pMenuState->pWindow) {
                        bSelectWindow = true;
                    }
                    break;
                }
            } else if (tEvent.what == 0xF && (tEvent.message & 0xFF000001) == 0x1000001 && pMenuState->pWindow != NULL) {
                bSelectWindow = true;
            }
            if (bSelectWindow) {
                SelectWindow(GetDialogWindow(pDialog));
                bHandled = true;
            }
            if (bHandled) {
                cResult = 1;
            } else {
                cResult = 0;
                if (IsDialogEvent(&tEvent)) {
                    DialogPtr pDummy;
                    cResult = DialogSelect(&tEvent, &pDummy, &nItemHit);
                }
            }
        } while (nItemHit == 0);
        *pItemHit = nItemHit;
        SetPort(pSavedPort);
    } else {
        memset(&tEvent, 0, sizeof(tEvent));
        WaitNextEvent(0, &tEvent, 0, NULL);
        if (pfnFilter != NULL) {
            pfnFilter(pDialog, &tEvent, &nItemHit);
        }
    }
}

// 1.14d mac: 00035ef1
// ModalDialog with no idle or menu callbacks.
void Dlg_ModalDialog(DialogPtr pDialog, DlgMenuState* pMenuState, DlgFilterProc pfnFilter, short* pItemHit) {
    Dlg_ModalDialogLoop(pDialog, pMenuState, pfnFilter, pItemHit, NULL, NULL, 0);
}

// 1.14d mac: 00035f34
// Dispatches an Edit menu selection to the dialog's text field and returns the focus item.
static void Dlg_HandleEditMenuCommand(DialogPtr pDialog, DlgMenuState* pMenuState, long nMenuResult, short* pItemHit) {
    short nFocusItem = 0;
    if ((nMenuResult >> 16) == pMenuState->nMenuID) {
        GetMenuHandle(pMenuState->nMenuID);
    } else if ((nMenuResult >> 16) == pMenuState->nEditMenuID) {
        MenuRef hMenu = GetMenuHandle(pMenuState->nEditMenuID);
        if (hMenu != NULL) {
            short nItem = (short)nMenuResult;
            if (nItem == pMenuState->nCutItem || nItem == pMenuState->nCopyItem) {
                if (nItem == pMenuState->nCutItem) {
                    DialogCut(pDialog);
                } else {
                    DialogCopy(pDialog);
                }
                TEToScrap();
            } else if (nItem == pMenuState->nPasteItem) {
                TEFromScrap();
                DialogPaste(pDialog);
            } else if (nItem == pMenuState->nClearItem) {
                DialogDelete(pDialog);
            } else {
                if (nItem != pMenuState->nSelectAllItem) {
                    goto done;
                }
                if (GetDialogKeyboardFocusItem(pDialog) < 1) {
                    goto done;
                }
                SelectDialogItemText(pDialog, GetDialogKeyboardFocusItem(pDialog), 0, 0x7FFF);
            }
            nFocusItem = GetDialogKeyboardFocusItem(pDialog);
        }
    }
done:
    if (pItemHit != NULL) {
        *pItemHit = nFocusItem;
    }
}

// 1.14d mac: 0003602b
BOOL Storm_ReturnTrue(void) {
    return 1;
}

#endif  // D2_OS_MAC
