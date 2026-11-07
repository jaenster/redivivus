/**
 * Data defined in D2Client/UI/npcmenu.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2Client) && !defined(D2_LOADS_D2Client)
#error "internal D2Client header included from another module; include D2Client's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2BitBufferStrc;
struct D2DialogStrc;
struct D2MercForHire;
struct D2NPCSpeechStrc;
struct D2SpeechBankStrc;
struct DC6;
struct ListBoxWrapper;

extern D2SpeechBankStrc gaNpcSpeechData_725498[12];
extern D2SpeechBankStrc gaNpcSpeechBank_725550[11];
extern D2SpeechBankStrc gaNpcDialogOptionData_7255f8[9];
extern D2SpeechBankStrc gaNpcDialogOptionData_725680[7];
extern D2SpeechBankStrc gaNpcDialogOptionData_7256f0[7];
extern D2SpeechBankStrc gaNpcDialogOptionData_725760[8];
extern D2SpeechBankStrc gaNpcDialogOptionData_7257d8[9];
extern D2SpeechBankStrc gaNpcDialogOptionData_725860[13];
extern D2SpeechBankStrc gaNpcDialogOptionData_725928[10];
extern D2SpeechBankStrc gaNpcDialogOptionData_7259c0[12];
extern D2SpeechBankStrc gaNpcDialogOptionData_725a78[14];
extern D2SpeechBankStrc gaNpcDialogOptionData_725b50[12];
extern D2SpeechBankStrc gaNpcDialogOptionData_725c08[11];
extern D2SpeechBankStrc gaNpcSpeechBank_725cb0[10];
extern D2SpeechBankStrc gaNpcSpriteFrameData_725d48[12];
extern D2SpeechBankStrc gaNpcSpriteFrameData_725e00[11];
extern D2SpeechBankStrc gaNpcDialogOptionData_725ea8[12];
extern D2SpeechBankStrc gaNpcDialogOptionData_725f60[7];
extern D2SpeechBankStrc gaNpcDialogOptionData_725fd0[11];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_00726078[12];
extern D2SpeechBankStrc gaNpcSpeechBank_726130[9];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_007261b8[11];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_00726260[12];
extern D2SpeechBankStrc gaNpcSpriteFrameData_726314[3];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_00726344[3];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_00726378[6];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_007263d8[12];
extern D2SpeechBankStrc gaNpcSpriteFrameData_726490[12];
extern D2SpeechBankStrc gaNpcSpeechBank_726548[12];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_00726600[16];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_007266f0[12];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_007267a8[11];
extern D2NPCSpeechStrc D2NPCSpeech[46];
extern uint16_t gnNpcMenuRepairCostGold;
extern int16_t gnNpcMenuTradeActionGold;
extern D2BitBufferStrc * gpNpcMenuDialogOptions;
extern ::ListBoxWrapper * gpNpcMenuMercListBox;
extern uint16_t gnNpcMenuScrollMessageHead;
extern uint32_t gnNpcMenuShopMode;
extern DC6 * gpNpcMenuShopInventoryDC6;

namespace D2Client::UI::npcmenu {
extern short nMercNameId;
} // namespace D2Client::UI::npcmenu

namespace D2Client::_Interaction {
extern eD2NpcUiState geNpcUiState;
extern BOOL bInteractingWithNpc;
extern int nCurrentInteractedNpcClassId;
} // namespace D2Client::_Interaction

namespace D2Client::Merc {
extern struct D2MercForHire D2MercForHire[10];
} // namespace D2Client::Merc

namespace D2Client::UI::Dialog {
extern D2DialogStrc * gpNpcTransactionConfirmDialog;
extern D2DialogStrc * gpNpcTransactionWaitDialog;
extern D2DialogStrc * gpNpcInteractionDialog;
extern D2DialogStrc * gpNpcGossipDialog;
extern D2DialogStrc * gpNpcActionConfirmDialog;
extern D2DialogStrc * gpNpcMercListDialog;
extern D2DialogStrc * gpNpcMainMenuDialog;
} // namespace D2Client::UI::Dialog
