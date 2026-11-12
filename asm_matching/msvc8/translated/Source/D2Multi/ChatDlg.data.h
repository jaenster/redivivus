/**
 * Data defined in D2Multi/ChatDlg.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Multi)
#error "D2Multi header included by a module that may not use D2Multi (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2Multi) && !defined(D2_LOADS_D2Multi)
#error "internal D2Multi header included from another module; include D2Multi's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2BNetClientCallbacksStrc;
struct D2ControlMsg;
struct D2ControlStrc;
struct D2LobbyControlsStrc;
struct D2WinButton;
struct D2WinEditBox;
struct D2WinSmack;
struct D2WinTextBox;
struct DC6;
struct FORMS_AnimFrameStrc;
struct FORMS_PredefinedFormStrc;
typedef BOOL (__stdcall *OnPress)(D2ControlMsg * pControlMsg);

extern unsigned short CreateJoinGameRequestId;
extern int gnChatTabCompletionIndex;
// Eleven lobby user-list row buttons. A stray label on element 1 kept it at one pointer, so rows 1..10 were created into, compared against and destroyed through adjacent globals.
extern ::D2WinButton * gaChatUserListButtonArray[11];
extern uint32_t gbChatOptionButtonState;
extern D2LobbyControlsStrc gpLobbyForm0;
extern D2BNetClientCallbacksStrc * pNetworkUnknown;
extern uint32_t ghBannerAdProcess;
extern char gszOogReplyCommandBuffer[256];
extern uint32_t gdwGameJoinRequestTick;
extern DC6 * gpDC6PopUpLargest2;
extern DC6 * gpDC6MediumButtonBlank2;
extern DC6 * gpDC6CursorOpenHand;
extern DC6 * gpDC6CursorLRotate;
extern DC6 * gpDC6ChatScrollbar;
extern DC6 * gpDC6JoinGameScrollbar;
extern DC6 * gpDC6ChatLeftButtons1;
extern DC6 * gpDC6ChatLeftButtons2;
extern DC6 * gpDC6ChatLeftButtons3;
extern DC6 * gpDC6ChatRightTopButtonsAlt;
extern DC6 * gpDC6ChatRightTopButtons;
extern DC6 * gpDC6ChatArrows;
extern D2ControlStrc * gpCtlLobbyAccountList;
extern ::D2WinTextBox * pGameListCache;
extern DC6 * DCC_JoinGame;
extern DC6 * gpDC6BlankBckg2;
extern DC6 * gpDC6GemButton;
extern DC6 * gpDC6GameButtonBlank;
extern DC6 * gpDC6JoinGameClickBox;
extern DC6 * gpDC6GreyLevelBox;
extern DC6 * gpDC6RadioButton;
extern DC6 * gpDC6PopUpOk;
extern DC6 * gpDC6CancelButtonBlankAlt;
extern DC6 * gpDC6OverallTab;
extern DC6 * gpDC6AmazonTab;
extern DC6 * gpDC6SorceressTab;
extern DC6 * gpDC6NecromancerTab;
extern DC6 * gpDC6PaladinTab;
extern DC6 * gpDC6BarbarianTab;
extern DC6 * gpDC6DruidTab;
extern DC6 * gpDC6AssassinTab;
extern DC6 * gpDC6NumberArrows1;
extern DC6 * gpDC6NumberArrows2;
extern DC6 * gpDC6NumberArrows3;
extern DC6 * gpDC6NumberArrows4;
extern DC6 * gpDC6NumberGreyArrows;
extern DC6 * gpDC6CreateJoinBckg;
extern D2ControlStrc * gpCtlChannelDlgBackground;
extern ::D2WinTextBox * gpCtlChannelDlgTitle;
extern ::D2WinTextBox * gpCtlChannelNameLabel;
extern ::D2WinTextBox * gpCtlChannelListLabel;
extern D2ControlStrc * gpCtlChannelOkButton;
extern D2ControlStrc * gpCtlChannelDlgCancelButton;
extern ::D2WinEditBox * gpCtlChannelNameEditBox;
extern ::D2WinTextBox * gpCtlChannelList;
extern DC6 * gpDC6CreateGameBckg;
extern DC6 * gpDC6WideButtonBlank;
extern DC6 * gpLauncherImageResource2;
extern DC6 * gpDC6YesNoButtonBlank;
extern DC6 * gpDC6LadderChoiceEXP;
extern DC6 * gpDC6HardcoreLadderBckg;
extern DC6 * gpDC6PlayerProfileBckg;
extern DC6 * gpDC6PlayerProfileBckg2;
extern DC6 * gpDC6HelpBckg;
extern DC6 * gpDC6D2LogoBlackBnet;
extern DC6 * gpDC6D2LogoBnet;
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
extern ::D2WinTextBox * gpChatMessagesTextBox;
extern ::D2WinEditBox * gpChatDlgInputEditBox;
extern ::D2WinTextBox * gpControlJoinGameError;
extern uint32_t gnOpenBNetMode;

namespace D2Multi { namespace OOGUtilities {
extern int32_t gaMultiFormFontParamsFont4[2];
extern int32_t gaMultiFormFontParamsFont2[2];
extern int32_t gaMultiFormFontParamsFont1[2];
extern int32_t gaMultiTextBoxScrollbarGeom6[3];
// Form records read by FORMS_CreateAnyForm (48 bytes each). pTypeArg1/pTypeArg2 (+0x24/+0x28) are constructor arguments whose meaning depends on neD2FormType: code, data or a small integer (the Button initial image index 0/2/4/6, e.g. gaPredefinedForms_b[12..14]); the struct field comments list each case. Typed void* for that reason.
// Provenance: FORMS_CreateAnyForm 004f93c0 and a census of every record.
// Index 0x80 (0070e650, FORM_Timer -> CHATDLG_OnTimerRequestLadderData) is the last record: UIMENU_LadderBoard_PrintClassesAndTableHeaders 00440887 calls FORMS_CreateAnyFormFromListEx(0x80).
extern FORMS_PredefinedFormStrc gaPredefinedForms_b[129];
}} // namespace D2Multi::OOGUtilities

namespace D2Multi { namespace ChatDlg {
extern int32_t gaMultiFormFontParamsFont3[2];
extern int32_t gaMultiFormFontParamsFont7[2];
extern int32_t gaMultiTextBoxScrollbarGeom0[3];
extern int32_t gaMultiTextBoxScrollbarGeom1[3];
extern int32_t gaMultiTextBoxScrollbarGeom2[3];
extern int32_t gaMultiTextBoxScrollbarGeom3[3];
extern int32_t gaMultiTextBoxScrollbarGeom4[3];
extern int32_t gaMultiTextBoxScrollbarGeom5[3];
extern FORMS_AnimFrameStrc gaMultiLogoAnimFrames[5];
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0077ae28;
}} // namespace D2Multi::ChatDlg
