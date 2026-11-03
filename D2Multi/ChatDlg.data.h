/**
 * Data defined in D2Multi/ChatDlg.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "D2Multi/OOGUtilities.h"

// Forward declarations
struct D2BNetClientCallbacksStrc;
struct D2ControlStrc;
struct D2LobbyControlsStrc;
struct D2WinButton;
struct D2WinSmack;
struct D2WinTextBox;

extern unsigned short CreateJoinGameRequestId;
extern int gnChatTabCompletionIndex;
// Eleven lobby user-list row buttons. A stray label on element 1 kept it at one pointer, so rows 1..10 were created into, compared against and destroyed through adjacent globals.
extern D2WinButton * gaChatUserListButtonArray[11];
extern uint32_t gbChatOptionButtonState;
extern D2ControlStrc * gpCtlLobbyChat[2];
extern D2LobbyControlsStrc gpLobbyForm0;
extern D2BNetClientCallbacksStrc * pNetworkUnknown;
extern uint32_t ghBannerAdProcess;
extern uint32_t gbLadderNewsExpanded;
extern uint32_t gdwGameJoinRequestTick;
extern uint32_t gnNotifyEnabled;
extern uint32_t gbIsChannelScreen;
extern uint32_t gnMCPConnectionRestoreResult;
// Initial item list of FORM_List record gaPredefinedForms_b[30] (pTypeArg1 -> D2WinList pnItemArray): zero-terminated
// locale string ids. .bss and never written, so the list starts empty. Neither record is referenced by address in 1.14d.
// Provenance: D2WinList 005075e0 reads *pnItemArray until 0; size = extent to the next symbol (0077bd74 / 0077bd84).
extern int gaFormList30InitialItems[4];
// Initial item list of FORM_List record gaPredefinedForms_b[32] (pTypeArg1 -> D2WinList pnItemArray): zero-terminated
// locale string ids. .bss and never written, so the list starts empty. Neither record is referenced by address in 1.14d.
// Provenance: D2WinList 005075e0 reads *pnItemArray until 0; size = extent to the next symbol (0077bd74 / 0077bd84).
extern int gaFormList32InitialItems[4];
extern int32_t gaChatTextBoxFontParams[2];
extern D2WinTextBox * gpChatMessagesTextBox;
