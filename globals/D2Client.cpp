/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"
#include "D2Client/D2Client.h"
#include "D2Client/D2GSPacketSrv0x2A.h"
#include "D2Client/Engine/Gfx.h"
#include "D2Client/GAME/Game/Game.h"
#include "D2Client/GAME/Record.h"
#include "D2Client/GAME/SCmd.h"
#include "D2Client/Message.h"
#include "D2Client/Sound/Sound.h"
#include "D2Client/UI/Button/ButtonImplementation.h"
#include "D2Client/UI/Button/ButtonWrapper.h"
#include "D2Client/UI/CmdTbl.h"
#include "D2Client/UI/Editbox/EditBoxImplementation.h"
#include "D2Client/UI/Editbox/EditBoxWrapper.h"
#include "D2Client/UI/EscMenu.h"
#include "D2Client/UI/Inv.h"
#include "D2Client/UI/ListBox/ListBoxImplementation.h"
#include "D2Client/UI/ListBox/ListBoxWrapper.h"
#include "D2Client/UI/Panel.h"
#include "D2Client/UI/QuestLog.h"
#include "D2Client/UI/ShowItems.h"
#include "D2Client/UI/SpellSel.h"
#include "D2Client/UI/Spinner/SpinnerImplementation.h"
#include "D2Client/UI/Spinner/SpinnerWrapper.h"
#include "D2Client/UI/Text.h"
#include "D2Client/UI/UseItem.h"
#include "D2Client/UI/Waypoint.h"
#include "D2Client/UI/npcmenu.h"
#include "D2Client/UI/npcmenu/Merc.h"
#include "D2Client/UI/ui.h"
#include "D2Client/UNIT/Missile.h"
#include "D2Client/UNIT/Monster.h"
#include "D2Client/Warden/WardenClient.h"
#include "D2Common/INVENTORY/Inventory.h"
#include "D2Game/Monster/AI/AI.h"


// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2UIFlagInitStrc g_UIFlagInitDrawMouseDown[4] = {
    {
        /* .eEventType = */ 0x0,
        /* .nMessageOrKeyCode = */ 0x201,
        /* .pfHandler = */ &D2Client::UI::Panel::INPUT_PanelLeftClickDown
    },
    {
        /* .eEventType = */ 0x0,
        /* .nMessageOrKeyCode = */ 0x202,
        /* .pfHandler = */ &D2Client::UI::Panel::INPUT_PanelLeftClickUp
    },
    {
        /* .eEventType = */ 0x0,
        /* .nMessageOrKeyCode = */ 0x200,
        /* .pfHandler = */ &D2Client::UI::Panel::PANEL_UpdatePanelFromMode
    },
    {
        /* .eEventType = */ 0x0,
        /* .nMessageOrKeyCode = */ 0x204,
        /* .pfHandler = */ &D2Client::UI::Panel::INPUT_PanelRightClick
    }
}; D2_GLOBAL_GUARD(g_UIFlagInitDrawMouseDown, "g_UIFlagInitDrawMouseDown");

char param_2_006d6454[3] = { '%', 'i', '\x00' }; D2_GLOBAL_GUARD(param_2_006d6454, "param_2_006d6454");

char param_2_006d9be8[4] = { '%', 'l', 'd', '\x00' }; D2_GLOBAL_GUARD(param_2_006d9be8, "param_2_006d9be8");

char param_2_006da0cc[4] = { '%', '2', 'u', '\x00' }; D2_GLOBAL_GUARD(param_2_006da0cc, "param_2_006da0cc");

D2MonMsgAnimModeStrc gaMonMsgAnimModeTable[30] = {
    {
        /* .eMode = */ Walk,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ Walk,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ 16,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ 16,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Cast,
        /* .bCheckMoveFlag = */ 0x1
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Cast,
        /* .bCheckMoveFlag = */ 0x1
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::GetHit,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Neutral,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ Death,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Dead,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Attack1,
        /* .bCheckMoveFlag = */ 0x1
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Attack1,
        /* .bCheckMoveFlag = */ 0x1
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Skill1,
        /* .bCheckMoveFlag = */ 0x1
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Skill1,
        /* .bCheckMoveFlag = */ 0x1
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Skill2,
        /* .bCheckMoveFlag = */ 0x1
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Skill2,
        /* .bCheckMoveFlag = */ 0x1
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Attack2,
        /* .bCheckMoveFlag = */ 0x1
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Attack2,
        /* .bCheckMoveFlag = */ 0x1
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Block,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ 16,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ Knockback,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Sequence,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Sequence,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Run,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Run,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Skill1,
        /* .bCheckMoveFlag = */ 0x0
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Skill3,
        /* .bCheckMoveFlag = */ 0x1
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Skill3,
        /* .bCheckMoveFlag = */ 0x1
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Skill4,
        /* .bCheckMoveFlag = */ 0x1
    },
    {
        /* .eMode = */ eD2MonsterAnimMode_ns::Skill4,
        /* .bCheckMoveFlag = */ 0x1
    }
}; D2_GLOBAL_GUARD(gaMonMsgAnimModeTable, "gaMonMsgAnimModeTable");

D2UIFlagInitStrc gUIFlagMapInit[8] = {
    {
        /* .eEventType = */ 0x0,
        /* .nMessageOrKeyCode = */ 0x202,
        /* .pfHandler = */ &D2Client::UI::ui::INPUT_GambleRepairClick
    },
    {
        /* .eEventType = */ 0x0,
        /* .nMessageOrKeyCode = */ 0x201,
        /* .pfHandler = */ &D2Client::UI::UseItem::INPUT_UseItemClick
    },
    {
        /* .eEventType = */ 0x0,
        /* .nMessageOrKeyCode = */ 0x204,
        /* .pfHandler = */ &D2Client::UI::UseItem::INPUT_UseItemClick
    },
    {
        /* .eEventType = */ 0x0,
        /* .nMessageOrKeyCode = */ 0x205,
        /* .pfHandler = */ &D2Client::UI::ui::INPUT_GambleRepairClick
    },
    {
        /* .eEventType = */ 0x0,
        /* .nMessageOrKeyCode = */ 0x200,
        /* .pfHandler = */ &D2Client::TEXT_SetUnknownFlag
    },
    {
        /* .eEventType = */ 0x3,
        /* .nMessageOrKeyCode = */ 0x1b,
        /* .pfHandler = */ &D2Client::UI::ui::PACKET_SendUnitInteraction0x44
    },
    {
        /* .eEventType = */ 0x3,
        /* .nMessageOrKeyCode = */ 0x20,
        /* .pfHandler = */ &D2Client::UI::ui::PACKET_SendUnitInteraction0x44
    },
    {
        /* .eEventType = */ 0x0,
        /* .nMessageOrKeyCode = */ 0x102,
        /* .pfHandler = */ &D2Client::UI::UseItem::INPUT_UseItemPassthrough
    }
}; D2_GLOBAL_GUARD(gUIFlagMapInit, "gUIFlagMapInit");

// also labelled: lpCriticalSection_QSERVERLoopRelated, lpCriticalSection_007a0468
_RTL_CRITICAL_SECTION gHostGameStateCs = {}; D2_GLOBAL_GUARD(gHostGameStateCs, "gHostGameStateCs");

HACCEL gbLanguageDllLoaded = nullptr; D2_GLOBAL_GUARD(gbLanguageDllLoaded, "gbLanguageDllLoaded");

char lpLibFileName_007a0710[260] = {}; D2_GLOBAL_GUARD(lpLibFileName_007a0710, "lpLibFileName_007a0710");

D2UIFlagStrc GFX_bShowUIFlag = {}; D2_GLOBAL_GUARD(GFX_bShowUIFlag, "GFX_bShowUIFlag");

D2UIFlagDescStrc gaUIFlagDescsClient[3] = {}; D2_GLOBAL_GUARD(gaUIFlagDescsClient, "gaUIFlagDescsClient");

uint16_t gU16ImmunityStringBuffer2[256] = {}; D2_GLOBAL_GUARD(gU16ImmunityStringBuffer2, "gU16ImmunityStringBuffer2");

D2UnitStrc * ClientSideUnitHashTables[6][128] = {}; D2_GLOBAL_GUARD(ClientSideUnitHashTables, "ClientSideUnitHashTables");

D2UnitStrc * ServerSideUnitHashTables[6][128] = {}; D2_GLOBAL_GUARD(ServerSideUnitHashTables, "ServerSideUnitHashTables");

D2ClientLightPulseStrc gRoomLightPulse = {}; D2_GLOBAL_GUARD(gRoomLightPulse, "gRoomLightPulse");

uint16_t gnMonsterUniqueNameBuffer[2] = {}; D2_GLOBAL_GUARD(gnMonsterUniqueNameBuffer, "gnMonsterUniqueNameBuffer");

int gaLevelAmbientMusicPlayedFlags[1024] = {}; D2_GLOBAL_GUARD(gaLevelAmbientMusicPlayedFlags, "gaLevelAmbientMusicPlayedFlags");

// =============================================================================
// Initialized scalars
// =============================================================================

uint32_t gnLastMouseButtonEventType = 0x1; D2_GLOBAL_GUARD(gnLastMouseButtonEventType, "gnLastMouseButtonEventType");
int ScreenSizeX = 0x280; D2_GLOBAL_GUARD(ScreenSizeX, "ScreenSizeX");
int ScreenSizeY = 0x1e0; D2_GLOBAL_GUARD(ScreenSizeY, "ScreenSizeY");
uint32_t gbLightmapColorEnabled = 0x1; D2_GLOBAL_GUARD(gbLightmapColorEnabled, "gbLightmapColorEnabled");
int nSoundVolumeLeftChannel = 0x64; D2_GLOBAL_GUARD(nSoundVolumeLeftChannel, "nSoundVolumeLeftChannel");
int nSoundVolumeRightChannel = 0x64; D2_GLOBAL_GUARD(nSoundVolumeRightChannel, "nSoundVolumeRightChannel");
int nCashDisplayItemCount = 0x2; D2_GLOBAL_GUARD(nCashDisplayItemCount, "nCashDisplayItemCount");
D2DrlgActStrc * sgpDrlgAct = 0; D2_GLOBAL_GUARD(sgpDrlgAct, "sgpDrlgAct");
D2Game_Line0x546 * D2GameGame0x546 = 0; D2_GLOBAL_GUARD(D2GameGame0x546, "D2GameGame0x546");
uint32_t gnWeatherOverlayActId = 0; D2_GLOBAL_GUARD(gnWeatherOverlayActId, "gnWeatherOverlayActId");
D2GameViewStrc * D2GameView = 0; D2_GLOBAL_GUARD(D2GameView, "D2GameView");
HMODULE ghLngLibrary = 0; D2_GLOBAL_GUARD(ghLngLibrary, "ghLngLibrary");
SpinnerWrapper * gpCurrentDialog = 0; D2_GLOBAL_GUARD(gpCurrentDialog, "gpCurrentDialog");
EditBoxWrapper * gpDialogDefaultActionCallback = 0; D2_GLOBAL_GUARD(gpDialogDefaultActionCallback, "gpDialogDefaultActionCallback");
ButtonWrapper * gpUiGoldTransactionDialog = 0; D2_GLOBAL_GUARD(gpUiGoldTransactionDialog, "gpUiGoldTransactionDialog");
ButtonWrapper * gpUiGoldDialogCancelButton = 0; D2_GLOBAL_GUARD(gpUiGoldDialogCancelButton, "gpUiGoldDialogCancelButton");
uint32_t gbKeyConfigModeActiveInDialog = 0; D2_GLOBAL_GUARD(gbKeyConfigModeActiveInDialog, "gbKeyConfigModeActiveInDialog");
// also labelled: g_nMouseX
int gnPanelOffsetX = 0; D2_GLOBAL_GUARD(gnPanelOffsetX, "gnPanelOffsetX");
// also labelled: g_nMouseY
int gnPanelOffsetY = 0; D2_GLOBAL_GUARD(gnPanelOffsetY, "gnPanelOffsetY");
uint32_t nScreenMode = 0; D2_GLOBAL_GUARD(nScreenMode, "nScreenMode");
int WindowSizeY = 0; D2_GLOBAL_GUARD(WindowSizeY, "WindowSizeY");
int WindowSizeX = 0; D2_GLOBAL_GUARD(WindowSizeX, "WindowSizeX");
uint32_t gbCharScreenOkPressed = 0; D2_GLOBAL_GUARD(gbCharScreenOkPressed, "gbCharScreenOkPressed");
uint32_t gbCharScreenOkButtonConfirmed = 0; D2_GLOBAL_GUARD(gbCharScreenOkButtonConfirmed, "gbCharScreenOkButtonConfirmed");
uint32_t gbCharScreenCancelPressed = 0; D2_GLOBAL_GUARD(gbCharScreenCancelPressed, "gbCharScreenCancelPressed");
D2UnitStrc * ClientSideUnitHashTables_1__0_[6][128] = {}; D2_GLOBAL_GUARD(ClientSideUnitHashTables_1__0_, "ClientSideUnitHashTables_1__0_");
D2UnitStrc * ClientSideUnitHashTables_1__1_[6][128] = {}; D2_GLOBAL_GUARD(ClientSideUnitHashTables_1__1_, "ClientSideUnitHashTables_1__1_");
D2UnitStrc * ClientSideUnitHashTables_2__0_[6][128] = {}; D2_GLOBAL_GUARD(ClientSideUnitHashTables_2__0_, "ClientSideUnitHashTables_2__0_");
D2UnitStrc * ClientSideUnitHashTables_3__0_[6][128] = {}; D2_GLOBAL_GUARD(ClientSideUnitHashTables_3__0_, "ClientSideUnitHashTables_3__0_");
D2UnitStrc * ServerSideUnitHashTables_0__1_[6][128] = {}; D2_GLOBAL_GUARD(ServerSideUnitHashTables_0__1_, "ServerSideUnitHashTables_0__1_");
D2UnitStrc * ServerSideUnitHashTables_1__0_[6][128] = {}; D2_GLOBAL_GUARD(ServerSideUnitHashTables_1__0_, "ServerSideUnitHashTables_1__0_");
D2UnitStrc * ServerSideUnitHashTables_1__1_[6][128] = {}; D2_GLOBAL_GUARD(ServerSideUnitHashTables_1__1_, "ServerSideUnitHashTables_1__1_");
D2UnitStrc * ServerSideUnitHashTables_2__0_[6][128] = {}; D2_GLOBAL_GUARD(ServerSideUnitHashTables_2__0_, "ServerSideUnitHashTables_2__0_");
D2UnitStrc * ServerSideUnitHashTables_3__0_[6][128] = {}; D2_GLOBAL_GUARD(ServerSideUnitHashTables_3__0_, "ServerSideUnitHashTables_3__0_");
D2UnitStrc * ServerSideUnitHashTables_4__0_[6][128] = {}; D2_GLOBAL_GUARD(ServerSideUnitHashTables_4__0_, "ServerSideUnitHashTables_4__0_");
D2UnitStrc * pPlayerUnit = 0; D2_GLOBAL_GUARD(pPlayerUnit, "pPlayerUnit");
int gnGfxDirectionCountCache = 0; D2_GLOBAL_GUARD(gnGfxDirectionCountCache, "gnGfxDirectionCountCache");
uint32_t gnNpcMenuMercHireCost1 = 0; D2_GLOBAL_GUARD(gnNpcMenuMercHireCost1, "gnNpcMenuMercHireCost1");
uint32_t gnNpcMenuMercHireCost2 = 0; D2_GLOBAL_GUARD(gnNpcMenuMercHireCost2, "gnNpcMenuMercHireCost2");
uint8_t gbNpcMenuInitialized = 0; D2_GLOBAL_GUARD(gbNpcMenuInitialized, "gbNpcMenuInitialized");
uint8_t gnNpcMenuActiveMode = 0; D2_GLOBAL_GUARD(gnNpcMenuActiveMode, "gnNpcMenuActiveMode");
uint32_t gnTradePlayerGoldAmount = 0; D2_GLOBAL_GUARD(gnTradePlayerGoldAmount, "gnTradePlayerGoldAmount");
uint32_t gdwTradeThrottleTimestamp = 0; D2_GLOBAL_GUARD(gdwTradeThrottleTimestamp, "gdwTradeThrottleTimestamp");
bool gbTradeResetFlag = 0; D2_GLOBAL_GUARD(gbTradeResetFlag, "gbTradeResetFlag");
uint32_t gnTradeAcceptedFlags = 0; D2_GLOBAL_GUARD(gnTradeAcceptedFlags, "gnTradeAcceptedFlags");
int32_t g_nTradeWindowState = 0; D2_GLOBAL_GUARD(g_nTradeWindowState, "g_nTradeWindowState");
DC6 * gpMessageLogDC6 = 0; D2_GLOBAL_GUARD(gpMessageLogDC6, "gpMessageLogDC6");
uint32_t nHoradricCubeUIState = 0; D2_GLOBAL_GUARD(nHoradricCubeUIState, "nHoradricCubeUIState");
int gnPlayerCashDisplay = 0; D2_GLOBAL_GUARD(gnPlayerCashDisplay, "gnPlayerCashDisplay");
int gnNpcShopSecondActiveTab = 0; D2_GLOBAL_GUARD(gnNpcShopSecondActiveTab, "gnNpcShopSecondActiveTab");
uint32_t gnNpcShopActiveTab = 0; D2_GLOBAL_GUARD(gnNpcShopActiveTab, "gnNpcShopActiveTab");
uint32_t gnCursorItemUnitId = 0; D2_GLOBAL_GUARD(gnCursorItemUnitId, "gnCursorItemUnitId");
uint32_t gnTextSystemInitFlag = 0; D2_GLOBAL_GUARD(gnTextSystemInitFlag, "gnTextSystemInitFlag");
DC6 * gpUpgradeMenuDC6 = 0; D2_GLOBAL_GUARD(gpUpgradeMenuDC6, "gpUpgradeMenuDC6");
DC6 * gpHoradricBackgroundDC6 = 0; D2_GLOBAL_GUARD(gpHoradricBackgroundDC6, "gpHoradricBackgroundDC6");
int gnSoundLastInitTick = 0; D2_GLOBAL_GUARD(gnSoundLastInitTick, "gnSoundLastInitTick");
uint32_t gdwSoundChannelIndex = 0; D2_GLOBAL_GUARD(gdwSoundChannelIndex, "gdwSoundChannelIndex");
uint32_t gnSoundInitCooldown = 0; D2_GLOBAL_GUARD(gnSoundInitCooldown, "gnSoundInitCooldown");
unsigned int gnSoundLastPlayerAnimModeTime = {}; D2_GLOBAL_GUARD(gnSoundLastPlayerAnimModeTime, "gnSoundLastPlayerAnimModeTime");
int geSoundLastPlayerAnimMode = 0; D2_GLOBAL_GUARD(geSoundLastPlayerAnimMode, "geSoundLastPlayerAnimMode");
BOOL pAudioMixerInterface = 0; D2_GLOBAL_GUARD(pAudioMixerInterface, "pAudioMixerInterface");
int32_t int32_t_007c8c7c = 0; D2_GLOBAL_GUARD(int32_t_007c8c7c, "int32_t_007c8c7c");
uint32_t gnActiveSoundChannelCount = 0; D2_GLOBAL_GUARD(gnActiveSoundChannelCount, "gnActiveSoundChannelCount");
int gnTileClipXMax = 0; D2_GLOBAL_GUARD(gnTileClipXMax, "gnTileClipXMax");
int gnTileClipYMin = 0; D2_GLOBAL_GUARD(gnTileClipYMin, "gnTileClipYMin");
int gnTileInnerClipYMax = 0; D2_GLOBAL_GUARD(gnTileInnerClipYMax, "gnTileInnerClipYMax");
int gnTileInnerClipXMin = 0; D2_GLOBAL_GUARD(gnTileInnerClipXMin, "gnTileInnerClipXMin");
int gnMinTileScreenY = 0; D2_GLOBAL_GUARD(gnMinTileScreenY, "gnMinTileScreenY");
int gnTileClipLeft = 0; D2_GLOBAL_GUARD(gnTileClipLeft, "gnTileClipLeft");
int gnTileInnerClipYMinRow = 0; D2_GLOBAL_GUARD(gnTileInnerClipYMinRow, "gnTileInnerClipYMinRow");
int gnTileInnerClipYMin = 0; D2_GLOBAL_GUARD(gnTileInnerClipYMin, "gnTileInnerClipYMin");
int gnTileInnerClipXMax_b = 0; D2_GLOBAL_GUARD(gnTileInnerClipXMax_b, "gnTileInnerClipXMax_b");
int gnMinTileDrawY = 0; D2_GLOBAL_GUARD(gnMinTileDrawY, "gnMinTileDrawY");
int gnMinTileScreenY_b = 0; D2_GLOBAL_GUARD(gnMinTileScreenY_b, "gnMinTileScreenY_b");
int gnTileClipRight = 0; D2_GLOBAL_GUARD(gnTileClipRight, "gnTileClipRight");

namespace D2Common::Drlg::Maze {

} // namespace D2Common::Drlg::Maze

namespace D2Client::Warden::NWardenClient {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

void * vftable[7] = { (void*)&_purecall, (void*)&_purecall, (void*)&_purecall, (void*)&_purecall, (void*)&_purecall, (void*)&_purecall, (void*)&_purecall }; D2_GLOBAL_GUARD(vftable, "D2Client::Warden::NWardenClient::vftable");

} // namespace D2Client::Warden::NWardenClient

namespace D2Client::Warden {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

void * vftable[7] = { (void*)&D2Client::Warden::SendPacket, (void*)&D2Client::Warden::CheckModule, (void*)&D2Client::Warden::LoadModule, (void*)&D2Client::Warden::AllocateMemory, (void*)&D2Client::Warden::FreeMemory, (void*)&D2Client::Warden::SetRC4Data, (void*)&D2Client::Warden::GetRC4Data }; D2_GLOBAL_GUARD(vftable, "D2Client::Warden::vftable");

} // namespace D2Client::Warden

namespace D2Client::UI {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

void * vftable[12] = {
    (void*)&_purecall,
    (void*)&_purecall,
    (void*)&_purecall,
    (void*)&_purecall,
    (void*)&_purecall,
    (void*)&_purecall,
    (void*)&_purecall,
    (void*)&_purecall,
    (void*)&_purecall,
    (void*)&_purecall,
    (void*)&_purecall,
    (void*)&_purecall
}; D2_GLOBAL_GUARD(vftable, "D2Client::UI::vftable");

} // namespace D2Client::UI

namespace D2Client::UI::Button::ButtonImplementation {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

void * vftable[13] = {
    (void*)&D2Client::UI::Button::ButtonImplementation::ScalarDeletingDestructor,
    (void*)&D2Client::UI::Button::ButtonImplementation::Draw,
    (void*)&D2Client::UI::Button::ButtonImplementation::OnMouseWheel,
    (void*)&D2Client::UI::Button::ButtonImplementation::OnMouseDown,
    (void*)&D2Client::UI::Button::ButtonImplementation::OnMouseUp,
    (void*)&D2Client::UI::Button::ButtonImplementation::OnMouseMove,
    (void*)&D2Client::UI::Button::ButtonImplementation::OnChar,
    (void*)&D2Client::UI::Button::ButtonImplementation::OnUpArrow,
    (void*)&D2Client::UI::Button::ButtonImplementation::OnDownArrow,
    (void*)&D2Client::UI::Button::ButtonImplementation::OnRightArrow,
    (void*)&D2Client::UI::Button::ButtonImplementation::OnLeftArrow,
    (void*)&D2Client::UI::Button::ButtonImplementation::GetValue,
    (void*)&D2Client::UI::Button::ButtonImplementation::SetAccelerator
}; D2_GLOBAL_GUARD(vftable, "D2Client::UI::Button::ButtonImplementation::vftable");

} // namespace D2Client::UI::Button::ButtonImplementation

namespace D2Client::UI::Button::ButtonWrapper {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

void * vftable[13] = {
    (void*)&D2Client::UI::Button::ButtonWrapper::ScalarDeletingDestructor,
    (void*)&D2Client::UI::Button::ButtonWrapper::Draw,
    (void*)&D2Client::UI::Button::ButtonWrapper::OnMouseWheel,
    (void*)&D2Client::UI::Button::ButtonWrapper::OnMouseDown,
    (void*)&D2Client::UI::Button::ButtonWrapper::OnMouseUp,
    (void*)&D2Client::UI::Button::ButtonWrapper::OnMouseMove,
    (void*)&D2Client::UI::Button::ButtonWrapper::OnChar,
    (void*)&D2Client::UI::Button::ButtonWrapper::OnUpArrow,
    (void*)&D2Client::UI::Button::ButtonWrapper::OnDownArrow,
    (void*)&D2Client::UI::Button::ButtonWrapper::OnRightArrow,
    (void*)&D2Client::UI::Button::ButtonWrapper::OnLeftArrow,
    (void*)&D2Client::UI::Button::ButtonWrapper::GetValue,
    (void*)&D2Client::UI::Button::ButtonWrapper::SetAccelerator
}; D2_GLOBAL_GUARD(vftable, "D2Client::UI::Button::ButtonWrapper::vftable");

} // namespace D2Client::UI::Button::ButtonWrapper

namespace D2Client::UI::Editbox::EditBoxImplementation {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

void * vftable[19] = {
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::ScalarDeletingDestructor,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::Draw,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::OnMouseWheel,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::OnMouseDown,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::OnMouseUp,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::OnMouseMove,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::OnChar,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::OnUpArrow,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::OnDownArrow,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::OnRightArrow,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::OnLeftArrow,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::GetValue,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::GetText,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::GetRepeatDelta,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::SetText,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::SetNumber,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::SetWidth,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::SetKeyRepeat,
    (void*)&D2Client::UI::Editbox::EditBoxImplementation::SetNumericOnly
}; D2_GLOBAL_GUARD(vftable, "D2Client::UI::Editbox::EditBoxImplementation::vftable");

} // namespace D2Client::UI::Editbox::EditBoxImplementation

namespace D2Client::UI::Editbox::EditBoxWrapper {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

void * vftable[19] = {
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::ScalarDeletingDestructor,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::Draw,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::OnMouseWheel,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::OnMouseDown,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::OnMouseUp,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::OnMouseMove,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::OnChar,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::OnUpArrow,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::OnDownArrow,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::OnRightArrow,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::OnLeftArrow,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::GetValue,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::GetRepeatDelta,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::SetText,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::SetNumber,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::GetText,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::SetWidth,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::SetKeyRepeat,
    (void*)&D2Client::UI::Editbox::EditBoxWrapper::SetNumericOnly
}; D2_GLOBAL_GUARD(vftable, "D2Client::UI::Editbox::EditBoxWrapper::vftable");

} // namespace D2Client::UI::Editbox::EditBoxWrapper

namespace D2Client::UI::Spinner::SpinnerImplementation {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

void * vftable[13] = {
    (void*)&D2Client::UI::Spinner::SpinnerImplementation::ScalarDeletingDestructor,
    (void*)&D2Client::UI::Spinner::SpinnerImplementation::Draw,
    (void*)&D2Client::UI::Spinner::SpinnerImplementation::OnMouseWheel,
    (void*)&D2Client::UI::Spinner::SpinnerImplementation::OnMouseDown,
    (void*)&D2Client::UI::Spinner::SpinnerImplementation::OnMouseUp,
    (void*)&D2Client::UI::Spinner::SpinnerImplementation::OnMouseMove,
    (void*)&D2Client::UI::Spinner::SpinnerImplementation::OnChar,
    (void*)&D2Client::UI::Spinner::SpinnerImplementation::OnUpArrow,
    (void*)&D2Client::UI::Spinner::SpinnerImplementation::OnDownArrow,
    (void*)&D2Client::UI::Spinner::SpinnerImplementation::OnRightArrow,
    (void*)&D2Client::UI::Spinner::SpinnerImplementation::OnLeftArrow,
    (void*)&D2Client::UI::Spinner::SpinnerImplementation::GetValue,
    (void*)&D2Client::UI::Spinner::SpinnerImplementation::GetRepeatDelta
}; D2_GLOBAL_GUARD(vftable, "D2Client::UI::Spinner::SpinnerImplementation::vftable");

} // namespace D2Client::UI::Spinner::SpinnerImplementation

namespace D2Client::UI::Spinner::SpinnerWrapper {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

void * vftable[13] = {
    (void*)&D2Client::UI::Spinner::SpinnerWrapper::ScalarDeletingDestructor,
    (void*)&D2Client::UI::Spinner::SpinnerWrapper::Draw,
    (void*)&D2Client::UI::Spinner::SpinnerWrapper::OnMouseWheel,
    (void*)&D2Client::UI::Spinner::SpinnerWrapper::OnMouseDown,
    (void*)&D2Client::UI::Spinner::SpinnerWrapper::OnMouseUp,
    (void*)&D2Client::UI::Spinner::SpinnerWrapper::OnMouseMove,
    (void*)&D2Client::UI::Spinner::SpinnerWrapper::OnChar,
    (void*)&D2Client::UI::Spinner::SpinnerWrapper::OnUpArrow,
    (void*)&D2Client::UI::Spinner::SpinnerWrapper::OnDownArrow,
    (void*)&D2Client::UI::Spinner::SpinnerWrapper::OnRightArrow,
    (void*)&D2Client::UI::Spinner::SpinnerWrapper::OnLeftArrow,
    (void*)&D2Client::UI::Spinner::SpinnerWrapper::GetValue,
    (void*)&D2Client::UI::Spinner::SpinnerWrapper::GetRepeatDelta
}; D2_GLOBAL_GUARD(vftable, "D2Client::UI::Spinner::SpinnerWrapper::vftable");

} // namespace D2Client::UI::Spinner::SpinnerWrapper

namespace D2Client::UI::ListBox::ListBoxWrapper {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

void * vftable[16] = {
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::ScalarDeletingDestructor,
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::Draw,
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::OnMouseWheel,
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::OnMouseDown,
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::OnMouseUp,
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::OnMouseMove,
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::OnChar,
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::OnUpArrow,
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::OnDownArrow,
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::OnRightArrow,
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::OnLeftArrow,
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::GetValue,
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::AddItem,
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::GetItemUserData,
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::GetItemCount,
    (void*)&D2Client::UI::ListBox::ListBoxWrapper::Clear
}; D2_GLOBAL_GUARD(vftable, "D2Client::UI::ListBox::ListBoxWrapper::vftable");

} // namespace D2Client::UI::ListBox::ListBoxWrapper

namespace D2Client::UI::ListBox::ListBoxImplementation {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

void * vftable[16] = {
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::ScalarDeletingDestructor,
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::Draw,
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::OnMouseWheel,
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::OnMouseDown,
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::OnMouseUp,
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::OnMouseMove,
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::OnChar,
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::OnUpArrow,
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::OnDownArrow,
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::OnRightArrow,
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::OnLeftArrow,
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::GetValue,
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::DrawScrollbar,
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::GetItemCount,
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::Clear,
    (void*)&D2Client::UI::ListBox::ListBoxImplementation::AddItem
}; D2_GLOBAL_GUARD(vftable, "D2Client::UI::ListBox::ListBoxImplementation::vftable");

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

namespace D2Client::CharScreen {

} // namespace D2Client::CharScreen

namespace D2Client::UI::ui {

} // namespace D2Client::UI::ui

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

// =============================================================================
// Initialized scalars
// =============================================================================

D2DialogStrc * gpDialog = 0; D2_GLOBAL_GUARD(gpDialog, "D2Client::UI::Dialog::gpDialog");
D2DialogStrc * gpTradeAgreeDialog = 0; D2_GLOBAL_GUARD(gpTradeAgreeDialog, "D2Client::UI::Dialog::gpTradeAgreeDialog");
D2DialogStrc * gpTradeCancelDialog = 0; D2_GLOBAL_GUARD(gpTradeCancelDialog, "D2Client::UI::Dialog::gpTradeCancelDialog");
D2DialogStrc * gpHoradricCubeDialog = 0; D2_GLOBAL_GUARD(gpHoradricCubeDialog, "D2Client::UI::Dialog::gpHoradricCubeDialog");

} // namespace D2Client::UI::Dialog

namespace D2Client::Draw::LightMap {

// =============================================================================
// Initialized scalars
// =============================================================================

D2LightMapStrc * gpLightMap = 0; D2_GLOBAL_GUARD(gpLightMap, "D2Client::Draw::LightMap::gpLightMap");

} // namespace D2Client::Draw::LightMap
