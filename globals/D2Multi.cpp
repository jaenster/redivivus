/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"
#include "D2Multi/ChatDlg.data.h"
#include "D2Launch/MainMenus.h"
#include "D2Multi/ChatDlg.h"
#include "D2Multi/ComCallback.h"
#include "D2Multi/D2Multi.h"
#include "D2Multi/OOGUtilities.h"


// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

FORMS_PredefinedFormStrc gaPredefinedForms_b[129] = {
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x1b, 0x1d6, 0x50, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x14b9,
        /* .pDC6 = */ &gpDC6ChatLeftButtons1,
        /* .fpOnPressCallback = */ (OnPress)&D2Multi::OOGUtilities::OOGUTIL_ValidateUsername1,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x6b, 0x1d6, 0x50, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x14ba,
        /* .pDC6 = */ &gpDC6ChatLeftButtons1,
        /* .fpOnPressCallback = */ (OnPress)&D2Multi::OOGUtilities::OOGUTIL_ValidateUsername2,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0xbb, 0x1d6, 0x50, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x14bc,
        /* .pDC6 = */ &gpDC6ChatLeftButtons1,
        /* .fpOnPressCallback = */ &D2Multi::CHATDLG_OpenChatCommandsDialogRestoreFocus,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x1b, 0x1ea, 0x48, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x14bb,
        /* .pDC6 = */ &gpDC6ChatLeftButtons2,
        /* .fpOnPressCallback = */ (OnPress)&D2Multi::ComCallback::COMCALLBACK_HandleGameListError1,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x63, 0x1ea, 0x60, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x14be,
        /* .pDC6 = */ &gpDC6ChatLeftButtons3,
        /* .fpOnPressCallback = */ (OnPress)&D2Multi::ComCallback::COMCALLBACK_HandleGameListError2,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0xc3, 0x1ea, 0x48, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x14bd,
        /* .pDC6 = */ &gpDC6ChatLeftButtons2,
        /* .fpOnPressCallback = */ (OnPress)&D2Multi::OOGUtilities::OOGUTIL_ValidateUsername3,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x215, 0x1d5, 0x78, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x14c0,
        /* .pDC6 = */ &gpDC6ChatRightTopButtons,
        /* .fpOnPressCallback = */ &D2Multi::CHATDLG_DestroyFormsAndOpenCreateGameDialog,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x1
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x217, 0x1ea, 0x50, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x14c2,
        /* .pDC6 = */ &gpDC6ChatRightTopButtonsAlt,
        /* .fpOnPressCallback = */ &D2Multi::CHATDLG_DestroyFormsAndOpenChannelDialog,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x1
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x28c, 0x1d5, 0x78, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x14c1,
        /* .pDC6 = */ &gpDC6ChatRightTopButtons,
        /* .fpOnPressCallback = */ &D2Multi::CHATDLG_DestroyFormsAndJoinGame,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x1
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x266, 0x1ea, 0x50, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x14c3,
        /* .pDC6 = */ &gpDC6ChatRightTopButtonsAlt,
        /* .fpOnPressCallback = */ &D2Multi::CHATDLG_DestroyFormsAndShowLadder,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x1
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2b5, 0x1ea, 0x50, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x14c4,
        /* .pDC6 = */ &gpDC6ChatRightTopButtonsAlt,
        /* .fpOnPressCallback = */ &D2Multi::CHATDLG_ShowGatewaySelectTitle,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2d5, 0x247, 0x19, 0x1e },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6ChatArrows,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressUserListPrevPage,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2ef, 0x247, 0x19, 0x1e },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6ChatArrows,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressUserListNextPage,
        /* .pTypeArg1 = */ (void *)0x00000002,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2d5, 0x210, 0x19, 0x1e },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6ChatArrows,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressUserListPrevLine,
        /* .pTypeArg1 = */ (void *)0x00000004,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2ef, 0x210, 0x19, 0x1e },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6ChatArrows,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressUserListNextLine,
        /* .pTypeArg1 = */ (void *)0x00000006,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Image,
        /* .aParams = */ { 0x91, 0x45, 0x1fe, 0x3c },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Smack,
        /* .aParams = */ { 0x91, 0x45, 0x1fe, 0x3c },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ (OnPress)&D2Multi::OOGUtilities::OOGUTIL_OpenBrowserWithURL,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x1b, 0x1e0, 0x78, 0x14 },
        /* .nExtraParam = */ 0xd,
        /* .nLabelAsLocaleStringId = */ 0x2b76,
        /* .pDC6 = */ &gpDC6ChatRightTopButtons,
        /* .fpOnPressCallback = */ (OnPress)&D2Multi::OOGUtilities::OOGUTIL_JoinDefaultChannel,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x92, 0x1e0, 0x78, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x14bc,
        /* .pDC6 = */ &gpDC6ChatRightTopButtons,
        /* .fpOnPressCallback = */ &D2Multi::ComCallback::CHATDLG_OpenChatCommandsDialogRestoreFocus2,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x8f, 0x24c, 0xe6, 0x57 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_AccountList,
        /* .aParams = */ { 0x28, 0x24f, 0x2d0, 0x4b },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1c, 0x19a, 0x162, 0x12a },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6ChatScrollbar,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)gaMultiTextBoxScrollbarGeom0,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x4
    },
    {
        /* .neD2FormType = */ FORM_EditBox,
        /* .aParams = */ { 0x1c, 0x1c2, 0x14f, 0x20 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)&D2Multi::OOGUtilities::OOGUTIL_SendChatMessageWide,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x16
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1c, 0x8a, 0x162, 0x3c },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_AnimImage,
        /* .aParams = */ { 0x54, 0x34, 0x93, 0x44 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)gaMultiLogoAnimFrames,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Image,
        /* .aParams = */ { 0x19f, 0x1bf, 0x175, 0x175 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6BlankBckg2,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1bf, 0x18e, 0x122, 0x10d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_AnimImage,
        /* .aParams = */ { 0x2d6, 0x34, 0x93, 0x44 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)gaMultiLogoAnimFrames,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x18e, 0x1da, 0xc, 0xd },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6GemButton,
        /* .fpOnPressCallback = */ (OnPress)&D2Multi::ChatDlg::CHATDLG_ToggleLadderNews,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x4
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1b6, 0x12c, 0x146, 0x96 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont2,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_List,
        /* .aParams = */ { 0x1b6, 0xf0, 0x146, 0x32 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)gaFormList30InitialItems,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont2,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1b6, 0xc8, 0x146, 0x32 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont2,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_List,
        /* .aParams = */ { 0x1b6, 0xf0, 0x146, 0x32 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)gaFormList32InitialItems,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont2,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Image,
        /* .aParams = */ { 0x10c, 0x15e, 0x108, 0xb0 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6PopUpOk,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x15f, 0x151, 0x60, 0x20 },
        /* .nExtraParam = */ 0xd,
        /* .nLabelAsLocaleStringId = */ 0x13ee,
        /* .pDC6 = */ &gpDC6CancelButtonBlankAlt,
        /* .fpOnPressCallback = */ (OnPress)&D2Multi::ChatDlg::CHATDLG_InvokeCallbackIfSet,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x15f, 0x151, 0x60, 0x20 },
        /* .nExtraParam = */ 0xd,
        /* .nLabelAsLocaleStringId = */ 0x13ef,
        /* .pDC6 = */ &gpDC6CancelButtonBlankAlt,
        /* .fpOnPressCallback = */ (OnPress)&D2Multi::ChatDlg::CHATDLG_InvokeCallbackIfSet,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x10c, 0x140, 0x108, 0x78 },
        /* .nExtraParam = */ 0xa,
        /* .nLabelAsLocaleStringId = */ 0x8,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont7,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Image,
        /* .aParams = */ { 0x9e, 0x1cc, 0x1e5, 0x122 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6PopUpLargest2,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0xcd, 0x1b8, 0x80, 0x23 },
        /* .nExtraParam = */ 0xd,
        /* .nLabelAsLocaleStringId = */ 0x13ef,
        /* .pDC6 = */ &gpDC6MediumButtonBlank2,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressChatWarningCancel,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x150, 0x1c6, 0x80, 0x23 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x13ee,
        /* .pDC6 = */ &gpDC6MediumButtonBlank2,
        /* .fpOnPressCallback = */ &D2Multi::OOGUtilities::OOGUTIL_OnPressChatWarningOk,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x9e, 0xf0, 0x1e5, 0x28 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont2,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0xac, 0x1a4, 0x1c9, 0xb4 },
        /* .nExtraParam = */ 0xa,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont7,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Image,
        /* .aParams = */ { 0x19f, 0x1bf, 0x175, 0x175 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &DCC_JoinGame,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1f0, 0x6a, 0xc8, 0x16 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont2,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1ac, 0x79, 0x96, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont1,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x25a, 0x79, 0x96, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont1,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1af, 0xd4, 0xc8, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont1,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_EditBox,
        /* .aParams = */ { 0x1b0, 0x94, 0x9b, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)&D2Multi::OOGUtilities::OOGUTIL_ProcessGameListEntry,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_EditBox,
        /* .aParams = */ { 0x25e, 0x94, 0x9b, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)&D2Multi::OOGUtilities::OOGUTIL_ProcessGameListEntry,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x1
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1b0, 0x189, 0xa0, 0xad },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6JoinGameScrollbar,
        /* .fpOnPressCallback = */ (OnPress)&D2Multi::OOGUtilities::OOGUTIL_HandleGameCreation,
        /* .pTypeArg1 = */ (void*)gaMultiTextBoxScrollbarGeom1,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x5
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x252, 0x1b1, 0xac, 0x20 },
        /* .nExtraParam = */ 0xd,
        /* .nLabelAsLocaleStringId = */ 0x141f,
        /* .pDC6 = */ &gpDC6GameButtonBlank,
        /* .fpOnPressCallback = */ (OnPress)&D2Multi::OOGUtilities::OOGUTIL_ProcessGameListEntry,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x1d7, 0xb9, 0xf, 0x10 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6JoinGameClickBox,
        /* .fpOnPressCallback = */ &D2Multi::OOGUtilities::OOGUTIL_OnPressNoAction,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x22a, 0xb9, 0xf, 0x10 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6JoinGameClickBox,
        /* .fpOnPressCallback = */ &D2Multi::OOGUtilities::OOGUTIL_OnPressNoAction,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x26e, 0xb9, 0xf, 0x10 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6JoinGameClickBox,
        /* .fpOnPressCallback = */ &D2Multi::OOGUtilities::OOGUTIL_OnPressNoAction,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2c9, 0xb9, 0xf, 0x10 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6JoinGameClickBox,
        /* .fpOnPressCallback = */ &D2Multi::OOGUtilities::OOGUTIL_OnPressNoAction,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x261, 0x189, 0x8f, 0xc2 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6JoinGameScrollbar,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)gaMultiTextBoxScrollbarGeom2,
        /* .pTypeArg2 = */ (void*)gaChatTextBoxFontParams,
        /* .nFlagsOrType = */ 0x6
    },
    {
        /* .neD2FormType = */ FORM_Image,
        /* .aParams = */ { 0x19f, 0x1bf, 0x175, 0x175 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6CreateJoinBckg,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1f0, 0x72, 0xc8, 0x16 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont2,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1ab, 0x87, 0x96, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont1,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1ab, 0xbb, 0x96, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont1,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_EditBox,
        /* .aParams = */ { 0x1b0, 0xa2, 0x9b, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)&D2Multi::OOGUtilities::OOGUTIL_JoinChannelFromInput,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_EditBox,
        /* .aParams = */ { 0x25e, 0xa2, 0x9e, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1b0, 0x18d, 0x141, 0xc8 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6JoinGameScrollbar,
        /* .fpOnPressCallback = */ (OnPress)&D2Multi::OOGUtilities::OOGUTIL_HandleChannelListDoubleClick,
        /* .pTypeArg1 = */ (void*)gaMultiTextBoxScrollbarGeom3,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x5
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x29f, 0x1b1, 0x60, 0x20 },
        /* .nExtraParam = */ 0xd,
        /* .nLabelAsLocaleStringId = */ 0x13ee,
        /* .pDC6 = */ &gpLauncherImageResource2,
        /* .fpOnPressCallback = */ (OnPress)&D2Multi::OOGUtilities::OOGUTIL_JoinChannelFromInput,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Image,
        /* .aParams = */ { 0x19f, 0x1bf, 0x175, 0x175 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6CreateGameBckg,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1f0, 0x6f, 0xc8, 0x16 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont2,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1ab, 0x86, 0x96, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont1,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1ab, 0xbc, 0x12c, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont1,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1ab, 0xf3, 0x12c, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont1,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x278, 0x132, 0x12c, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont1,
        /* .nFlagsOrType = */ 0x10
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x278, 0x156, 0x12c, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont1,
        /* .nFlagsOrType = */ 0x10
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x2c1, 0x156, 0x96, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont1,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_EditBox,
        /* .aParams = */ { 0x1b0, 0xa2, 0x9e, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)&D2Multi::OOGUtilities::NET_MCP_CLIENT_PrepareGameCreatePacket,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_EditBox,
        /* .aParams = */ { 0x1b0, 0xd9, 0x9e, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)&D2Multi::OOGUtilities::NET_MCP_CLIENT_PrepareGameCreatePacket,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x1
    },
    {
        /* .neD2FormType = */ FORM_EditBox,
        /* .aParams = */ { 0x1b0, 0x10c, 0x14d, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)&D2Multi::OOGUtilities::NET_MCP_CLIENT_PrepareGameCreatePacket,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_EditBox,
        /* .aParams = */ { 0x291, 0x134, 0x1b, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)&D2Multi::OOGUtilities::NET_MCP_CLIENT_PrepareGameCreatePacket,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_EditBox,
        /* .aParams = */ { 0x291, 0x156, 0x1b, 0x14 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)&D2Multi::OOGUtilities::NET_MCP_CLIENT_PrepareGameCreatePacket,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x252, 0x1b1, 0xac, 0x20 },
        /* .nExtraParam = */ 0xd,
        /* .nLabelAsLocaleStringId = */ 0x141e,
        /* .pDC6 = */ &gpDC6GameButtonBlank,
        /* .fpOnPressCallback = */ (OnPress)&D2Multi::OOGUtilities::NET_MCP_CLIENT_PrepareGameCreatePacket,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x1af, 0x155, 0xf, 0x10 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6JoinGameClickBox,
        /* .fpOnPressCallback = */ &D2Multi::OOGUtilities::OOGUTIL_OnPressLevelRestrictionCheckbox,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Image,
        /* .aParams = */ { 0x287, 0x15a, 0x21, 0x1a },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6GreyLevelBox,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x1ae, 0x17d, 0x10, 0x10 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6RadioButton,
        /* .fpOnPressCallback = */ &D2Multi::OOGUtilities::OOGUTIL_OnPressDifficultyNormal,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x22b, 0x17d, 0x10, 0x10 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6RadioButton,
        /* .fpOnPressCallback = */ &D2Multi::OOGUtilities::OOGUTIL_OnPressDifficultyNightmare,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2ba, 0x17d, 0x10, 0x10 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6RadioButton,
        /* .fpOnPressCallback = */ &D2Multi::OOGUtilities::OOGUTIL_OnPressDifficultyHell,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1cb, 0x17c, 0x96, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont1,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x243, 0x17c, 0x96, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont1,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x2d3, 0x17c, 0x96, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont1,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2a9, 0x128, 0xf, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6NumberArrows3,
        /* .fpOnPressCallback = */ &D2Multi::OOGUtilities::OOGUTIL_OnPressMaxPlayersUp,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2a9, 0x136, 0xf, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6NumberArrows4,
        /* .fpOnPressCallback = */ &D2Multi::OOGUtilities::OOGUTIL_OnPressMaxPlayersDown,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2a9, 0x14c, 0xf, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6NumberArrows1,
        /* .fpOnPressCallback = */ &D2Multi::OOGUtilities::OOGUTIL_OnPressLevelRestrictionUp,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2a9, 0x15a, 0xf, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6NumberArrows2,
        /* .fpOnPressCallback = */ &D2Multi::OOGUtilities::OOGUTIL_OnPressLevelRestrictionDown,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2a9, 0x15a, 0xf, 0x1a },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6NumberGreyArrows,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Image,
        /* .aParams = */ { 0x287, 0x137, 0x21, 0x1a },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6GreyLevelBox,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2a9, 0x137, 0xf, 0x1a },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6NumberGreyArrows,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Image,
        /* .aParams = */ { 0x19f, 0x1bf, 0x175, 0x175 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6LadderChoiceEXP,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1f0, 0x70, 0xc8, 0x16 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont2,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x1cf, 0xee, 0x110, 0x20 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x14c6,
        /* .pDC6 = */ &gpDC6WideButtonBlank,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressLadderClassicHardcore,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x1cf, 0xee, 0x110, 0x23 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x678,
        /* .pDC6 = */ &gpDC6WideButtonBlank,
        /* .fpOnPressCallback = */ &D2Multi::CHATDLG_Unused2,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x1cf, 0xbc, 0x110, 0x20 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x14c5,
        /* .pDC6 = */ &gpDC6WideButtonBlank,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressLadderClassicSoftcore,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x1cf, 0x152, 0x110, 0x20 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x276b,
        /* .pDC6 = */ &gpDC6WideButtonBlank,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressLadderExpansionHardcore,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x1cf, 0x120, 0x110, 0x20 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x276a,
        /* .pDC6 = */ &gpDC6WideButtonBlank,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressLadderExpansionSoftcore,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x1b1, 0x1b1, 0x60, 0x20 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x13ef,
        /* .pDC6 = */ &gpLauncherImageResource2,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressCloseDialog,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Image,
        /* .aParams = */ { 0x19f, 0x1bf, 0x175, 0x175 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6HardcoreLadderBckg,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1a5, 0x88, 0x15e, 0x32 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont7,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1b2, 0xa2, 0xd9, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1d4, 0xa2, 0xd9, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x254, 0xa2, 0xd9, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x280, 0xa2, 0xd9, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x2bf, 0xa2, 0xd9, 0xc },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1b2, 0x187, 0x139, 0xda },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6JoinGameScrollbar,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ (void*)gaMultiTextBoxScrollbarGeom4,
        /* .pTypeArg2 = */ (void*)gaMultiFormFontParamsFont4,
        /* .nFlagsOrType = */ 0x4
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x1b1, 0x1b1, 0x60, 0x20 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x13ef,
        /* .pDC6 = */ &gpLauncherImageResource2,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressCloseLadderBoard,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x1ab, 0x95, 0x55, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6OverallTab,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressLadderOverall,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x201, 0x95, 0x24, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6AmazonTab,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressLadderAmazon,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x226, 0x95, 0x24, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6SorceressTab,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressLadderSorceress,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x24b, 0x95, 0x24, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6NecromancerTab,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressLadderNecromancer,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x270, 0x95, 0x24, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6PaladinTab,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressLadderPaladin,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x295, 0x95, 0x24, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6BarbarianTab,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressLadderBarbarian,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2ba, 0x95, 0x24, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6DruidTab,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressLadderDruid,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2df, 0x95, 0x24, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6AssassinTab,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressLadderAssassin,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x1ab, 0x9d, 0x55, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaChatTextBoxFontParams,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x201, 0x9d, 0x24, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaChatTextBoxFontParams,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x226, 0x9d, 0x24, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaChatTextBoxFontParams,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x24b, 0x9d, 0x24, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaChatTextBoxFontParams,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x270, 0x9d, 0x24, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaChatTextBoxFontParams,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x295, 0x9d, 0x24, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaChatTextBoxFontParams,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x2ba, 0x9d, 0x24, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaChatTextBoxFontParams,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_TextBox,
        /* .aParams = */ { 0x2df, 0x9d, 0x24, 0x1d },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ nullptr,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ (void*)gaChatTextBoxFontParams,
        /* .nFlagsOrType = */ 0x2
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x277, 0x1b1, 0x46, 0x20 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6YesNoButtonBlank,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressLadderPageUp,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Button,
        /* .aParams = */ { 0x2bd, 0x1b1, 0x46, 0x20 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ &gpDC6YesNoButtonBlank,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnPressLadderPageDown,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    },
    {
        /* .neD2FormType = */ FORM_Timer,
        /* .aParams = */ { 0x64, 0x0, 0x0, 0x0 },
        /* .nExtraParam = */ 0x0,
        /* .nLabelAsLocaleStringId = */ 0x0,
        /* .pDC6 = */ nullptr,
        /* .fpOnPressCallback = */ &D2Multi::ChatDlg::CHATDLG_OnTimerRequestLadderData,
        /* .pTypeArg1 = */ nullptr,
        /* .pTypeArg2 = */ nullptr,
        /* .nFlagsOrType = */ 0x0
    }
}; D2_GLOBAL_GUARD(gaPredefinedForms_b, "gaPredefinedForms_b");

D2GameListEntryStrc gbLanGameListEntryNamePadding2[1000] = {}; D2_GLOBAL_GUARD(gbLanGameListEntryNamePadding2, "gbLanGameListEntryNamePadding2");

D2GameListEntryStrc gbLanGameListEntryNamePadding[1000] = {}; D2_GLOBAL_GUARD(gbLanGameListEntryNamePadding, "gbLanGameListEntryNamePadding");

char gszOOGRequestedGameName[256] = {}; D2_GLOBAL_GUARD(gszOOGRequestedGameName, "gszOOGRequestedGameName");

// also labelled: gszOOGJoinGamePassword
char gszProfileCharacterName[16] = {}; D2_GLOBAL_GUARD(gszProfileCharacterName, "gszProfileCharacterName");

// =============================================================================
// Initialized scalars
// =============================================================================

D2IniConfigStrc * D2IniData_multiplayer = 0; D2_GLOBAL_GUARD(D2IniData_multiplayer, "D2IniData_multiplayer");
uint16_t LastGameRequestId = 0; D2_GLOBAL_GUARD(LastGameRequestId, "LastGameRequestId");
void * gpChatFormList = 0; D2_GLOBAL_GUARD(gpChatFormList, "gpChatFormList");
unsigned int gdwLastGameListRefreshTick = {}; D2_GLOBAL_GUARD(gdwLastGameListRefreshTick, "gdwLastGameListRefreshTick");
::D2WinTextBox * gpOOGGameListTextBox = 0; D2_GLOBAL_GUARD(gpOOGGameListTextBox, "gpOOGGameListTextBox");
::D2WinTextBox * gpOogGameListDetailsPanelControl = 0; D2_GLOBAL_GUARD(gpOogGameListDetailsPanelControl, "gpOogGameListDetailsPanelControl");
D2ControlStrc * gpOOGJoinGameButton = 0; D2_GLOBAL_GUARD(gpOOGJoinGameButton, "gpOOGJoinGameButton");
D2ControlStrc * gpCtlGameListStatus = 0; D2_GLOBAL_GUARD(gpCtlGameListStatus, "gpCtlGameListStatus");
D2ControlStrc * gpOOGLobbyControl2 = 0; D2_GLOBAL_GUARD(gpOOGLobbyControl2, "gpOOGLobbyControl2");
::D2WinEditBox * gpOogGamePasswordEditControl = 0; D2_GLOBAL_GUARD(gpOogGamePasswordEditControl, "gpOogGamePasswordEditControl");
::D2WinEditBox * gpCtlGameListTop = 0; D2_GLOBAL_GUARD(gpCtlGameListTop, "gpCtlGameListTop");
::D2WinTextBox * gpCtlJoinGameLabel = 0; D2_GLOBAL_GUARD(gpCtlJoinGameLabel, "gpCtlJoinGameLabel");
::D2WinTextBox * pUIGameUsername = 0; D2_GLOBAL_GUARD(pUIGameUsername, "pUIGameUsername");
::D2WinTextBox * pUIGamePassword = 0; D2_GLOBAL_GUARD(pUIGamePassword, "pUIGamePassword");
::D2WinTextBox * pUIGamePlayers = 0; D2_GLOBAL_GUARD(pUIGamePlayers, "pUIGamePlayers");
// also labelled: gpOOGLobbyControl3
D2ControlStrc * gpCtlCreateGameDlgBackground = 0; D2_GLOBAL_GUARD(gpCtlCreateGameDlgBackground, "gpCtlCreateGameDlgBackground");
// also labelled: gpCtlGameListBottom
::D2WinTextBox * gpCtlCreateGameDlgTitle = 0; D2_GLOBAL_GUARD(gpCtlCreateGameDlgTitle, "gpCtlCreateGameDlgTitle");
// also labelled: gpCtlWaitInLineStatus
::D2WinTextBox * gpCtlGameNameLabelOrQueueStatus = 0; D2_GLOBAL_GUARD(gpCtlGameNameLabelOrQueueStatus, "gpCtlGameNameLabelOrQueueStatus");
// also labelled: gpCtlGameListError
::D2WinTextBox * gpCtlCreateGamePasswordLabel = 0; D2_GLOBAL_GUARD(gpCtlCreateGamePasswordLabel, "gpCtlCreateGamePasswordLabel");
::D2WinTextBox * gUIGameListServerMessageText = 0; D2_GLOBAL_GUARD(gUIGameListServerMessageText, "gUIGameListServerMessageText");
// also labelled: gUIGameListExpansionCheckbox
::D2WinTextBox * gUIGameListMaxPlayersLabel = 0; D2_GLOBAL_GUARD(gUIGameListMaxPlayersLabel, "gUIGameListMaxPlayersLabel");
// also labelled: gUIGameListPasswordEditBox
::D2WinTextBox * gUIGameListLevelRestrictionLabel = 0; D2_GLOBAL_GUARD(gUIGameListLevelRestrictionLabel, "gUIGameListLevelRestrictionLabel");
// also labelled: gUIGameListDifficultyLabel
::D2WinTextBox * gUIGameListLevelRestrictionLevelsLabel = 0; D2_GLOBAL_GUARD(gUIGameListLevelRestrictionLevelsLabel, "gUIGameListLevelRestrictionLevelsLabel");
// also labelled: gpOOGLobbyControl1
D2ControlStrc * gUIGameListMaxPlayersInactiveImage = 0; D2_GLOBAL_GUARD(gUIGameListMaxPlayersInactiveImage, "gUIGameListMaxPlayersInactiveImage");
// also labelled: gUIGameListGameTypeLabel
D2ControlStrc * gUIGameListMaxPlayersInactiveArrowButton = 0; D2_GLOBAL_GUARD(gUIGameListMaxPlayersInactiveArrowButton, "gUIGameListMaxPlayersInactiveArrowButton");
// also labelled: gpOOGLobbyControl4
D2ControlStrc * gUIGameListMaxPlayersUpButton = 0; D2_GLOBAL_GUARD(gUIGameListMaxPlayersUpButton, "gUIGameListMaxPlayersUpButton");
// also labelled: gUIGameListPlayerCountLabel
::D2WinButton * gUIGameListMaxPlayersDownButton = 0; D2_GLOBAL_GUARD(gUIGameListMaxPlayersDownButton, "gUIGameListMaxPlayersDownButton");
// also labelled: gUIGameListLevelRestrictionMinEditBox
D2ControlStrc * gUIGameListLevelRestrictionUpButton = 0; D2_GLOBAL_GUARD(gUIGameListLevelRestrictionUpButton, "gUIGameListLevelRestrictionUpButton");
// also labelled: gUIGameListLevelRestrictionMaxEditBox
::D2WinButton * gUIGameListLevelRestrictionDownButton = 0; D2_GLOBAL_GUARD(gUIGameListLevelRestrictionDownButton, "gUIGameListLevelRestrictionDownButton");
// also labelled: gUIGameListRefreshButton
D2ControlStrc * gUIGameListLevelRestrictionInactiveArrowButton = 0; D2_GLOBAL_GUARD(gUIGameListLevelRestrictionInactiveArrowButton, "gUIGameListLevelRestrictionInactiveArrowButton");
D2ControlStrc * gUIGameListCreateButton = 0; D2_GLOBAL_GUARD(gUIGameListCreateButton, "gUIGameListCreateButton");
::D2WinEditBox * gpOogGameNameEditControl = 0; D2_GLOBAL_GUARD(gpOogGameNameEditControl, "gpOogGameNameEditControl");
::D2WinEditBox * gUIGameListGameNameEditBox = 0; D2_GLOBAL_GUARD(gUIGameListGameNameEditBox, "gUIGameListGameNameEditBox");
::D2WinEditBox * gUIGameListGamePasswordEditBox = 0; D2_GLOBAL_GUARD(gUIGameListGamePasswordEditBox, "gUIGameListGamePasswordEditBox");
// also labelled: gpOogSelectedGameListControl
::D2WinEditBox * gUIGameListMaxPlayersEditBox = 0; D2_GLOBAL_GUARD(gUIGameListMaxPlayersEditBox, "gUIGameListMaxPlayersEditBox");
::D2WinEditBox * gUILevelRestrictionEditBox = 0; D2_GLOBAL_GUARD(gUILevelRestrictionEditBox, "gUILevelRestrictionEditBox");
// also labelled: gUIGameListJoinButton
::D2WinButton * gUIGameListLevelRestrictionCheckbox = 0; D2_GLOBAL_GUARD(gUIGameListLevelRestrictionCheckbox, "gUIGameListLevelRestrictionCheckbox");
// also labelled: gUIGameListCancelButton
D2ControlStrc * gUIGameListLevelRestrictionInactiveImage = 0; D2_GLOBAL_GUARD(gUIGameListLevelRestrictionInactiveImage, "gUIGameListLevelRestrictionInactiveImage");
// also labelled: gpOOGLobbyControl6
D2ControlStrc * gpCtlCreateGameDlgCancelButton = 0; D2_GLOBAL_GUARD(gpCtlCreateGameDlgCancelButton, "gpCtlCreateGameDlgCancelButton");
// also labelled: gUIGameListGameListBox
::D2WinButton * gUIGameListDifficultyNormalButton = 0; D2_GLOBAL_GUARD(gUIGameListDifficultyNormalButton, "gUIGameListDifficultyNormalButton");
// also labelled: gUIGameListScrollbar
::D2WinButton * gUIGameListDifficultyNightmareButton = 0; D2_GLOBAL_GUARD(gUIGameListDifficultyNightmareButton, "gUIGameListDifficultyNightmareButton");
// also labelled: gUIGameListExpandedDetailsText
::D2WinButton * gUIGameListDifficultyHellButton = 0; D2_GLOBAL_GUARD(gUIGameListDifficultyHellButton, "gUIGameListDifficultyHellButton");
::D2WinTextBox * gUIGameListStatusLabel = 0; D2_GLOBAL_GUARD(gUIGameListStatusLabel, "gUIGameListStatusLabel");
::D2WinTextBox * gUIGameListPlayerListText = 0; D2_GLOBAL_GUARD(gUIGameListPlayerListText, "gUIGameListPlayerListText");
::D2WinTextBox * gUIGameListActLabel = 0; D2_GLOBAL_GUARD(gUIGameListActLabel, "gUIGameListActLabel");
// also labelled: gpOOGLobbyControl7
D2ControlStrc * gpProfileViewBackground = 0; D2_GLOBAL_GUARD(gpProfileViewBackground, "gpProfileViewBackground");
// also labelled: gpOOGLobbyControl8
::D2WinTextBox * gpProfileViewAccountName = 0; D2_GLOBAL_GUARD(gpProfileViewAccountName, "gpProfileViewAccountName");
// also labelled: gpOOGPasswordEditChName
::D2WinTextBox * gpProfileViewSex = 0; D2_GLOBAL_GUARD(gpProfileViewSex, "gpProfileViewSex");
// also labelled: gpOOGPasswordEditCharacterName
::D2WinTextBox * gpProfileViewLocation = 0; D2_GLOBAL_GUARD(gpProfileViewLocation, "gpProfileViewLocation");
// also labelled: gUIPasswordDialogInputEditBox
::D2WinTextBox * gpProfileViewDescription = 0; D2_GLOBAL_GUARD(gpProfileViewDescription, "gpProfileViewDescription");
// also labelled: gpOOGPasswordEditChPass
D2ControlStrc * gpProfileEditBackground = 0; D2_GLOBAL_GUARD(gpProfileEditBackground, "gpProfileEditBackground");
// also labelled: gpOOGPasswordEditCharacterPass
::D2WinTextBox * gpProfileEditAccountName = 0; D2_GLOBAL_GUARD(gpProfileEditAccountName, "gpProfileEditAccountName");
// also labelled: gpOogBnetLoginAccountNameControl
::D2WinEditBox * gpProfileEditSex = 0; D2_GLOBAL_GUARD(gpProfileEditSex, "gpProfileEditSex");
// also labelled: gpOogBnetLoginGameSelectControl
::D2WinEditBox * gpProfileEditLocation = 0; D2_GLOBAL_GUARD(gpProfileEditLocation, "gpProfileEditLocation");
// also labelled: gpOogBnetLoginPasswordControl
::D2WinEditBox * gpProfileEditDescription = 0; D2_GLOBAL_GUARD(gpProfileEditDescription, "gpProfileEditDescription");
// also labelled: gpOOGPasswordEditChGammaCode
D2ControlStrc * gpProfileCloseButton = 0; D2_GLOBAL_GUARD(gpProfileCloseButton, "gpProfileCloseButton");
::D2WinTextBox * gUIPlayerProfileCharacterNameLabel = 0; D2_GLOBAL_GUARD(gUIPlayerProfileCharacterNameLabel, "gUIPlayerProfileCharacterNameLabel");
::D2WinTextBox * gUIPlayerProfileCharacterLevelLabel = 0; D2_GLOBAL_GUARD(gUIPlayerProfileCharacterLevelLabel, "gUIPlayerProfileCharacterLevelLabel");
::D2WinTextBox * gUIPlayerProfileCharacterClassLabel = 0; D2_GLOBAL_GUARD(gUIPlayerProfileCharacterClassLabel, "gUIPlayerProfileCharacterClassLabel");
::D2WinTextBox * gUIPlayerProfileExperienceLabel = 0; D2_GLOBAL_GUARD(gUIPlayerProfileExperienceLabel, "gUIPlayerProfileExperienceLabel");
// also labelled: gpOOGPasswordButtonOK
D2ControlStrc * gpProfileOKButton = 0; D2_GLOBAL_GUARD(gpProfileOKButton, "gpProfileOKButton");
// also labelled: gpOOGPasswordButtonCancel
D2ControlStrc * gpProfileLadderButton = 0; D2_GLOBAL_GUARD(gpProfileLadderButton, "gpProfileLadderButton");
::D2WinTextBox * gUIPlayerProfilePlaytimeLabel = 0; D2_GLOBAL_GUARD(gUIPlayerProfilePlaytimeLabel, "gUIPlayerProfilePlaytimeLabel");
// also labelled: gnGameListSortMode
int32_t gnProfileDialogMode = 0; D2_GLOBAL_GUARD(gnProfileDialogMode, "gnProfileDialogMode");
// also labelled: gUIChatCommandsScrollOutput
::D2WinTextBox * gpChatCommandsTitle = 0; D2_GLOBAL_GUARD(gpChatCommandsTitle, "gpChatCommandsTitle");
::D2WinTextBox * gpChatCommandsTextBox = 0; D2_GLOBAL_GUARD(gpChatCommandsTextBox, "gpChatCommandsTextBox");
// also labelled: gpOOGPasswordEditChAccount
D2ControlStrc * gpChatCommandsOKButton = 0; D2_GLOBAL_GUARD(gpChatCommandsOKButton, "gpChatCommandsOKButton");
// also labelled: gpOOGLoginEditConnection
D2ControlStrc * gpChatCommandsBackground = 0; D2_GLOBAL_GUARD(gpChatCommandsBackground, "gpChatCommandsBackground");
uint32_t gnGameListFilterState = 0; D2_GLOBAL_GUARD(gnGameListFilterState, "gnGameListFilterState");
uint32_t gnMCPCreateGamePendingFlag = 0; D2_GLOBAL_GUARD(gnMCPCreateGamePendingFlag, "gnMCPCreateGamePendingFlag");
int gnFilteredGameListCount = 0; D2_GLOBAL_GUARD(gnFilteredGameListCount, "gnFilteredGameListCount");
int nGameListDifficultyFilter = 0; D2_GLOBAL_GUARD(nGameListDifficultyFilter, "nGameListDifficultyFilter");
uint32_t CreateGameDifflevel = 0; D2_GLOBAL_GUARD(CreateGameDifflevel, "CreateGameDifflevel");
// also labelled: gdwPasswordControlsPadding2
int gnProfileUserHardcoreFlag = 0; D2_GLOBAL_GUARD(gnProfileUserHardcoreFlag, "gnProfileUserHardcoreFlag");
// also labelled: gdwPasswordControlsPadding1
int gnProfileUserExpansionFlag = 0; D2_GLOBAL_GUARD(gnProfileUserExpansionFlag, "gnProfileUserExpansionFlag");

namespace D2Common::Drlg::Maze {

} // namespace D2Common::Drlg::Maze

namespace D2Client::Warden::NWardenClient {

} // namespace D2Client::Warden::NWardenClient

namespace D2Client::Warden {

} // namespace D2Client::Warden

namespace D2Client::UI {

} // namespace D2Client::UI

namespace D2Client::UI::Button::ButtonImplementation {

} // namespace D2Client::UI::Button::ButtonImplementation

namespace D2Client::UI::Button::ButtonWrapper {

} // namespace D2Client::UI::Button::ButtonWrapper

namespace D2Client::UI::Editbox::EditBoxImplementation {

} // namespace D2Client::UI::Editbox::EditBoxImplementation

namespace D2Client::UI::Editbox::EditBoxWrapper {

} // namespace D2Client::UI::Editbox::EditBoxWrapper

namespace D2Client::UI::Spinner::SpinnerImplementation {

} // namespace D2Client::UI::Spinner::SpinnerImplementation

namespace D2Client::UI::Spinner::SpinnerWrapper {

} // namespace D2Client::UI::Spinner::SpinnerWrapper

namespace D2Client::UI::ListBox::ListBoxWrapper {

} // namespace D2Client::UI::ListBox::ListBoxWrapper

namespace D2Client::UI::ListBox::ListBoxImplementation {

} // namespace D2Client::UI::ListBox::ListBoxImplementation

namespace D2Game::GAME::Event::Dispatch {

} // namespace D2Game::GAME::Event::Dispatch

namespace D2Game::GAME::Event::Dispatch::Player {

} // namespace D2Game::GAME::Event::Dispatch::Player

namespace D2Game::GAME::Event::Dispatch::Object {

} // namespace D2Game::GAME::Event::Dispatch::Object

namespace D2Game::GAME::Event::Dispatch::Monster {

} // namespace D2Game::GAME::Event::Dispatch::Monster

namespace D2Common::Chat::Ignorelist::ChatIgnoreList {

} // namespace D2Common::Chat::Ignorelist::ChatIgnoreList

namespace D2Common::Chat::Ignorelist::IgnoreList {

} // namespace D2Common::Chat::Ignorelist::IgnoreList

namespace D2Common::Drlg::Outdoors {

} // namespace D2Common::Drlg::Outdoors

namespace Define_the_symbol__ATL_MIXED {

} // namespace Define_the_symbol__ATL_MIXED

namespace Fog::_3rdparty::crashy {

} // namespace Fog::_3rdparty::crashy

namespace Storm::Source::SFile {

} // namespace Storm::Source::SFile

namespace Class::FILEREC {

} // namespace Class::FILEREC

namespace Class::EVENTREC {

} // namespace Class::EVENTREC

namespace Class::REQUEST {

} // namespace Class::REQUEST

namespace Class::AUDIOSTREAM {

} // namespace Class::AUDIOSTREAM

namespace Class::HANDLER {

} // namespace Class::HANDLER

namespace D2Launch::CharSel {

} // namespace D2Launch::CharSel

namespace D2Game::GAME::Rendering {

} // namespace D2Game::GAME::Rendering

namespace D2Client::Renderer {

} // namespace D2Client::Renderer

namespace D2DDraw::Renderer::DirectDraw {

} // namespace D2DDraw::Renderer::DirectDraw

namespace D2Lang::Unicode::UNISYS {

} // namespace D2Lang::Unicode::UNISYS

namespace D2Game::Quests::QUESTS::Quest40 {

} // namespace D2Game::Quests::QUESTS::Quest40

namespace D2Game::Quests::QUESTS::A5Q1 {

} // namespace D2Game::Quests::QUESTS::A5Q1

namespace D2Game::Quests::QUESTS::A5Q2 {

} // namespace D2Game::Quests::QUESTS::A5Q2

namespace D2Game::Quests::QUESTS::A5Q3 {

} // namespace D2Game::Quests::QUESTS::A5Q3

namespace D2Game::Quests::QUESTS::A5Q4 {

} // namespace D2Game::Quests::QUESTS::A5Q4

namespace D2Game::Quests::QUESTS::A5Q5 {

} // namespace D2Game::Quests::QUESTS::A5Q5

namespace D2Game::Quests::QUESTS::A5Q6 {

} // namespace D2Game::Quests::QUESTS::A5Q6

namespace D2Game::Quests::QUESTS::Quest37 {

} // namespace D2Game::Quests::QUESTS::Quest37

namespace D2Game::Quests::QUESTS::A1Q0 {

} // namespace D2Game::Quests::QUESTS::A1Q0

namespace D2Game::Quests::QUESTS::A1Q1 {

} // namespace D2Game::Quests::QUESTS::A1Q1

namespace D2Game::Quests::QUESTS::A1Q2 {

} // namespace D2Game::Quests::QUESTS::A1Q2

namespace D2Game::Quests::QUESTS::A1Q3 {

} // namespace D2Game::Quests::QUESTS::A1Q3

namespace D2Game::Quests::QUESTS::A1Q4 {

} // namespace D2Game::Quests::QUESTS::A1Q4

namespace D2Game::Quests::QUESTS::A1Q5 {

} // namespace D2Game::Quests::QUESTS::A1Q5

namespace D2Game::Quests::QUESTS::A1Q6 {

} // namespace D2Game::Quests::QUESTS::A1Q6

namespace D2Game::Quests::QUESTS::A1Q7 {

} // namespace D2Game::Quests::QUESTS::A1Q7

namespace D2Game::Quests::QUESTS::Quest38 {

} // namespace D2Game::Quests::QUESTS::Quest38

namespace D2Game::Quests::QUESTS::A2Q0 {

} // namespace D2Game::Quests::QUESTS::A2Q0

namespace D2Game::Quests::QUESTS::A2Q1 {

} // namespace D2Game::Quests::QUESTS::A2Q1

namespace D2Game::Quests::QUESTS::A2Q2 {

} // namespace D2Game::Quests::QUESTS::A2Q2

namespace D2Game::Quests::QUESTS::A2Q3 {

} // namespace D2Game::Quests::QUESTS::A2Q3

namespace D2Game::Quests::QUESTS::A2Q4 {

} // namespace D2Game::Quests::QUESTS::A2Q4

namespace D2Game::Quests::QUESTS::A2Q5 {

} // namespace D2Game::Quests::QUESTS::A2Q5

namespace D2Game::Quests::QUESTS::A2Q6 {

} // namespace D2Game::Quests::QUESTS::A2Q6

namespace D2Game::Quests::QUESTS::A2Q7 {

} // namespace D2Game::Quests::QUESTS::A2Q7

namespace D2Game::Quests::QUESTS::A2Q8 {

} // namespace D2Game::Quests::QUESTS::A2Q8

namespace D2Game::Quests::QUESTS::A4Q0 {

} // namespace D2Game::Quests::QUESTS::A4Q0

namespace D2Game::Quests::QUESTS::A4Q1 {

} // namespace D2Game::Quests::QUESTS::A4Q1

namespace D2Game::Quests::QUESTS::A4Q2 {

} // namespace D2Game::Quests::QUESTS::A4Q2

namespace D2Game::Quests::QUESTS::A4Q3 {

} // namespace D2Game::Quests::QUESTS::A4Q3

namespace D2Game::Quests::QUESTS::A4Q4 {

} // namespace D2Game::Quests::QUESTS::A4Q4

namespace D2Game::Quests::QUESTS::Quest39 {

} // namespace D2Game::Quests::QUESTS::Quest39

namespace D2Game::Quests::QUESTS::A3Q0 {

} // namespace D2Game::Quests::QUESTS::A3Q0

namespace D2Game::Quests::QUESTS::A3Q1 {

} // namespace D2Game::Quests::QUESTS::A3Q1

namespace D2Game::Quests::QUESTS::A3Q2 {

} // namespace D2Game::Quests::QUESTS::A3Q2

namespace D2Game::Quests::QUESTS::A3Q3 {

} // namespace D2Game::Quests::QUESTS::A3Q3

namespace D2Game::Quests::QUESTS::A3Q4 {

} // namespace D2Game::Quests::QUESTS::A3Q4

namespace D2Game::Quests::QUESTS::A3Q5 {

} // namespace D2Game::Quests::QUESTS::A3Q5

namespace D2Game::Quests::QUESTS::A3Q6 {

} // namespace D2Game::Quests::QUESTS::A3Q6

namespace D2Common::Drlg::DrlgGrid {

} // namespace D2Common::Drlg::DrlgGrid

namespace D2Client::UI::Dialog {

} // namespace D2Client::UI::Dialog

namespace D2Client::Draw::LightMap {

} // namespace D2Client::Draw::LightMap

namespace D2Win::Src::D2WinMain {

} // namespace D2Win::Src::D2WinMain

namespace D2Win::D2GFX {

} // namespace D2Win::D2GFX
