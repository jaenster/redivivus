/**
 * Data defined in D2Client/UI/ui.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "globals/globals.h"
#include "D2Client/UI/ui.h"

// Forward declarations
struct D2ConfigControlDescStrc;
struct D2UIFlagInitStrc;
struct D2UIMouseMsgStrc;
struct DC6;
typedef void (__stdcall *fnD2InitUIFlagFunction)(D2UIMouseMsgStrc * param_1);

extern D2UIFlagInitStrc g_UIFlagInitDrawUnknown[2];
extern D2UIFlagInitStrc gsUIDrawFlagInit0[5];
extern D2UIFlagInitStrc gsUIDrawFlagInit1[2];
extern D2UIFlagInitStrc gaUIFlagInitNpcShopHandlers[8];
extern D2UIFlagInitStrc gUIFlagDrawGoldInit[7];
extern D2UIFlagInitStrc gUIFlagDrawInventoryInit[13];
extern int gnUIFlagCount;
extern D2UIFlagInitStrc gUIFlagDrawInfoInit[8];
extern D2UIFlagInitStrc gaUIFlagInitCubeHandlers[7];
extern D2UIFlagInitStrc g_UIFlagInitDrawPanel1[2];
extern D2UIFlagDescStrc gaUIFlagDescs[38];
extern D2UIFlagDescStrc * gUIFlagDismissModeTable[38];
extern D2ConfigControlDescStrc gaConfigControlBindings[51];
extern DC6 * DC6_EnteredAreaName;
extern DC6 * pDC6_Frame;
extern uint32_t gnLastOpenPanelId;
extern DC6 * gpUiCursorFocus16DC6;
extern DC6 * pUi_Panel_buysellbtn;
extern DC6 * gnLoadedActIndex;
extern uint16_t gU16ImmunityStringBuffer1[256];
extern DC6 * gpUIStatLevelUpPanelImages;
extern DC6 * gpUIStatLevelUpButtonImages;

namespace D2Client::UI::ui {
extern DC6 * gpDC6CursorIcon;
} // namespace D2Client::UI::ui
