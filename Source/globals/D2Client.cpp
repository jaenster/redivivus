/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "Source/D2Client/D2Client.globals.h"
#include "Source/D2Client/UI/Panel.h"
#include "Source/D2Client/UI/UseItem.h"
#include "Source/D2Client/UI/ui.h"
#include "Source/D2Client/Warden/WardenClient.h"
#include "Source/D2Common/D2Common.globals.h"
#include "Source/D2Headers/d2_platform.h"
#include "Source/D2Client/D2ClientRoot.h"
#include "Source/D2Client/UI/Button/ButtonImplementation.h"
#include "Source/D2Client/UI/Button/ButtonWrapper.h"
#include "Source/D2Client/UI/Editbox/EditBoxImplementation.h"
#include "Source/D2Client/UI/Editbox/EditBoxWrapper.h"
#include "Source/D2Client/UI/ListBox/ListBoxImplementation.h"
#include "Source/D2Client/UI/ListBox/ListBoxWrapper.h"
#include "Source/D2Client/UI/Spinner/SpinnerImplementation.h"
#include "Source/D2Client/UI/Spinner/SpinnerWrapper.h"

namespace Storm::Source::SFile {
extern uint8_t RTTI_Type_Descriptor;
} // namespace Storm::Source::SFile
namespace Class::FILEREC {
extern uint8_t RTTI_Type_Descriptor;
} // namespace Class::FILEREC
namespace Class::EVENTREC {
extern uint8_t RTTI_Type_Descriptor;
} // namespace Class::EVENTREC
namespace Class::REQUEST {
extern uint8_t RTTI_Type_Descriptor;
} // namespace Class::REQUEST
namespace Class::AUDIOSTREAM {
extern uint8_t RTTI_Type_Descriptor;
} // namespace Class::AUDIOSTREAM
namespace Class::HANDLER {
extern uint8_t RTTI_Type_Descriptor;
} // namespace Class::HANDLER

// =============================================================================
// Initialized scalars
// =============================================================================

uint16_t gnNpcMenuScrollCompleteMode = 0; D2_GLOBAL_GUARD(gnNpcMenuScrollCompleteMode, "gnNpcMenuScrollCompleteMode");

namespace D2Common::Drlg::Maze {

} // namespace D2Common::Drlg::Maze

namespace Fog::BitBuffer {

} // namespace Fog::BitBuffer

namespace Storm::Source::SLog {

} // namespace Storm::Source::SLog

namespace Storm::_3rdparty::zlib::deflate {

} // namespace Storm::_3rdparty::zlib::deflate

namespace D2Launch::CharSel {

} // namespace D2Launch::CharSel

namespace D2Common::DATATBLS::LvlTbls {

} // namespace D2Common::DATATBLS::LvlTbls

namespace D2Client::ClientModeInGame {

// =============================================================================
// Initialized scalars
// =============================================================================

char * gpszD2LngFileName = (char*)"D2.LNG"; D2_GLOBAL_GUARD(gpszD2LngFileName, "D2Client::ClientModeInGame::gpszD2LngFileName");
char * gpszLngManagerStartError = (char*)"Unable to start LNG manager."; D2_GLOBAL_GUARD(gpszLngManagerStartError, "D2Client::ClientModeInGame::gpszLngManagerStartError");
char * gpszLngCppError = (char*)"Stupid Lng.cpp error."; D2_GLOBAL_GUARD(gpszLngCppError, "D2Client::ClientModeInGame::gpszLngCppError");

} // namespace D2Client::ClientModeInGame

namespace D2Client::UI::Draw {

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
}; D2_GLOBAL_GUARD(g_UIFlagInitDrawMouseDown, "D2Client::UI::Draw::g_UIFlagInitDrawMouseDown");

D2UIFlagStrc GFX_bShowUIFlag = {}; D2_GLOBAL_GUARD(GFX_bShowUIFlag, "D2Client::UI::Draw::GFX_bShowUIFlag");

D2UIFlagDescStrc gaUIFlagDescsClient[3] = {}; D2_GLOBAL_GUARD(gaUIFlagDescsClient, "D2Client::UI::Draw::gaUIFlagDescsClient");

} // namespace D2Client::UI::Draw

namespace D2Client::UI::Inv {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char param_2_006d6454[3] = { '%', 'i', '\x00' }; D2_GLOBAL_GUARD(param_2_006d6454, "D2Client::UI::Inv::param_2_006d6454");

// =============================================================================
// Initialized scalars
// =============================================================================

int ScreenSizeY = 0x1e0; D2_GLOBAL_GUARD(ScreenSizeY, "D2Client::UI::Inv::ScreenSizeY");
// also labelled: g_nMouseX
int gnPanelOffsetX = 0; D2_GLOBAL_GUARD(gnPanelOffsetX, "D2Client::UI::Inv::gnPanelOffsetX");
// also labelled: g_nMouseY
int gnPanelOffsetY = 0; D2_GLOBAL_GUARD(gnPanelOffsetY, "D2Client::UI::Inv::gnPanelOffsetY");
uint32_t nScreenMode = 0; D2_GLOBAL_GUARD(nScreenMode, "D2Client::UI::Inv::nScreenMode");

} // namespace D2Client::UI::Inv

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

namespace D2gfx::D2GFX {

} // namespace D2gfx::D2GFX

namespace D2Sound::Sound {

} // namespace D2Sound::Sound

namespace Bnclient::BnSend {

} // namespace Bnclient::BnSend

namespace Bnclient::_net_sid {

} // namespace Bnclient::_net_sid

namespace D2Lang::StrTable::strtable {

} // namespace D2Lang::StrTable::strtable

namespace D2Game::GAME::Event::Dispatch {

} // namespace D2Game::GAME::Event::Dispatch

namespace D2Game::PLAYER::PlayerModes {

} // namespace D2Game::PLAYER::PlayerModes

namespace D2Game::GAME::Event::Dispatch::Player {

} // namespace D2Game::GAME::Event::Dispatch::Player

namespace D2Game::Objects::ObjectsMode {

} // namespace D2Game::Objects::ObjectsMode

namespace D2Game::GAME::Event::Dispatch::Object {

} // namespace D2Game::GAME::Event::Dispatch::Object

namespace D2Game::GAME::Event::Dispatch::Monster {

} // namespace D2Game::GAME::Event::Dispatch::Monster

namespace D2Game {

} // namespace D2Game

namespace D2CMP::SRC::CelCmp {

} // namespace D2CMP::SRC::CelCmp

namespace D2Common::Chat::Ignorelist::ChatIgnoreList {

} // namespace D2Common::Chat::Ignorelist::ChatIgnoreList

namespace D2Common::Chat::Ignorelist::IgnoreList {

} // namespace D2Common::Chat::Ignorelist::IgnoreList

namespace D2Common::DATATBLS::ItemTbls {

} // namespace D2Common::DATATBLS::ItemTbls

namespace D2Common::Drlg::OutSiege {

} // namespace D2Common::Drlg::OutSiege

namespace D2Common::Drlg::Outdoors {

} // namespace D2Common::Drlg::Outdoors

namespace D2Common::Drlg::OutJung {

} // namespace D2Common::Drlg::OutJung

namespace D2Common::Drlg {

} // namespace D2Common::Drlg

namespace Define_the_symbol__ATL_MIXED {

} // namespace Define_the_symbol__ATL_MIXED

namespace Storm::Source {

} // namespace Storm::Source

namespace Fog::_3rdparty::crashy {

} // namespace Fog::_3rdparty::crashy

namespace Fog::Source::SSComp {

} // namespace Fog::Source::SSComp

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

namespace Storm::Source::SErr {

} // namespace Storm::Source::SErr

namespace Class::HANDLER {

} // namespace Class::HANDLER

namespace D2Launch::MainMenus {

} // namespace D2Launch::MainMenus

namespace D2Client::GAME::Game {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char s___AUSGAMEDATA___0070f56c[16] = {
    '.',
    '?',
    'A',
    'U',
    'S',
    'G',
    'A',
    'M',
    'E',
    'D',
    'A',
    'T',
    'A',
    '@',
    '@',
    '\x00'
}; D2_GLOBAL_GUARD(s___AUSGAMEDATA___0070f56c, "D2Client::GAME::Game::s___AUSGAMEDATA___0070f56c");

// also labelled: lpCriticalSection_QSERVERLoopRelated, lpCriticalSection_007a0468
_RTL_CRITICAL_SECTION gHostGameStateCs = {}; D2_GLOBAL_GUARD(gHostGameStateCs, "D2Client::GAME::Game::gHostGameStateCs");

HACCEL ghAccelTable = nullptr; D2_GLOBAL_GUARD(ghAccelTable, "D2Client::GAME::Game::ghAccelTable");

// =============================================================================
// Initialized scalars
// =============================================================================

D2DrlgActStrc * sgpDrlgAct = 0; D2_GLOBAL_GUARD(sgpDrlgAct, "D2Client::GAME::Game::sgpDrlgAct");
D2Game_Line0x546 * D2GameGame0x546 = 0; D2_GLOBAL_GUARD(D2GameGame0x546, "D2Client::GAME::Game::D2GameGame0x546");
uint32_t gnWeatherOverlayActId = 0; D2_GLOBAL_GUARD(gnWeatherOverlayActId, "D2Client::GAME::Game::gnWeatherOverlayActId");
D2GameViewStrc * D2GameView = 0; D2_GLOBAL_GUARD(D2GameView, "D2Client::GAME::Game::D2GameView");

} // namespace D2Client::GAME::Game

namespace D2Client {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char s___AV__TSExplicitList_USGAMEDATA__0070f584[48] = {
    '.',
    '?',
    'A',
    'V',
    '?',
    '$',
    'T',
    'S',
    'E',
    'x',
    'p',
    'l',
    'i',
    'c',
    'i',
    't',
    'L',
    'i',
    's',
    't',
    '@',
    'U',
    'S',
    'G',
    'A',
    'M',
    'E',
    'D',
    'A',
    'T',
    'A',
    '@',
    '@',
    '$',
    '0',
    '?',
    'C',
    'C',
    'C',
    'C',
    'C',
    'C',
    'C',
    'D',
    '@',
    '@',
    '@',
    '\x00'
}; D2_GLOBAL_GUARD(s___AV__TSExplicitList_USGAMEDATA__0070f584, "D2Client::s___AV__TSExplicitList_USGAMEDATA__0070f584");

D2ClientLightPulseStrc gRoomLightPulse = {}; D2_GLOBAL_GUARD(gRoomLightPulse, "D2Client::gRoomLightPulse");

// =============================================================================
// Initialized scalars
// =============================================================================

uint32_t gnTradePlayerGoldAmount = 0; D2_GLOBAL_GUARD(gnTradePlayerGoldAmount, "D2Client::gnTradePlayerGoldAmount");
uint32_t gnTradeAcceptedFlags = 0; D2_GLOBAL_GUARD(gnTradeAcceptedFlags, "D2Client::gnTradeAcceptedFlags");
BOOL bReceivedQuestAvailabilityPacket = 0; D2_GLOBAL_GUARD(bReceivedQuestAvailabilityPacket, "D2Client::bReceivedQuestAvailabilityPacket");
int gnNpcShopSecondActiveTab = 0; D2_GLOBAL_GUARD(gnNpcShopSecondActiveTab, "D2Client::gnNpcShopSecondActiveTab");

} // namespace D2Client

namespace D2Client::GAME::Record {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char s___AV__TSHashObjectChunk_USGAMEDA_0070f5f4[56] = {
    '.',
    '?',
    'A',
    'V',
    '?',
    '$',
    'T',
    'S',
    'H',
    'a',
    's',
    'h',
    'O',
    'b',
    'j',
    'e',
    'c',
    't',
    'C',
    'h',
    'u',
    'n',
    'k',
    '@',
    'U',
    'S',
    'G',
    'A',
    'M',
    'E',
    'D',
    'A',
    'T',
    'A',
    '@',
    '@',
    'V',
    'H',
    'A',
    'S',
    'H',
    'K',
    'E',
    'Y',
    '_',
    'N',
    'O',
    'N',
    'E',
    '@',
    '@',
    '@',
    '@',
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(s___AV__TSHashObjectChunk_USGAMEDA_0070f5f4, "D2Client::GAME::Record::s___AV__TSHashObjectChunk_USGAMEDA_0070f5f4");

} // namespace D2Client::GAME::Record

namespace D2Client::_Message {

// =============================================================================
// Initialized scalars
// =============================================================================

uint32_t gnLastMouseButtonEventType = 0x1; D2_GLOBAL_GUARD(gnLastMouseButtonEventType, "D2Client::_Message::gnLastMouseButtonEventType");

} // namespace D2Client::_Message

namespace D2Client::Engine {

// =============================================================================
// Initialized scalars
// =============================================================================

uint32_t gaScreenResolutionTableIndexArray = 0x40; D2_GLOBAL_GUARD(gaScreenResolutionTableIndexArray, "D2Client::Engine::gaScreenResolutionTableIndexArray");
uint32_t gaScreenResolutionModeArray = 0x1; D2_GLOBAL_GUARD(gaScreenResolutionModeArray, "D2Client::Engine::gaScreenResolutionModeArray");
uint32_t gbCharScreenOkPressed = 0; D2_GLOBAL_GUARD(gbCharScreenOkPressed, "D2Client::Engine::gbCharScreenOkPressed");
uint32_t gbCharScreenOkButtonConfirmed = 0; D2_GLOBAL_GUARD(gbCharScreenOkButtonConfirmed, "D2Client::Engine::gbCharScreenOkButtonConfirmed");
uint32_t gbCharScreenCancelPressed = 0; D2_GLOBAL_GUARD(gbCharScreenCancelPressed, "D2Client::Engine::gbCharScreenCancelPressed");

} // namespace D2Client::Engine

namespace D2Client::UI::ui {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

uint16_t gU16ImmunityStringBuffer2[256] = {}; D2_GLOBAL_GUARD(gU16ImmunityStringBuffer2, "D2Client::UI::ui::gU16ImmunityStringBuffer2");

// =============================================================================
// Initialized scalars
// =============================================================================

int ScreenSizeX = 0x280; D2_GLOBAL_GUARD(ScreenSizeX, "D2Client::UI::ui::ScreenSizeX");
::SpinnerWrapper * gpCurrentDialog = 0; D2_GLOBAL_GUARD(gpCurrentDialog, "D2Client::UI::ui::gpCurrentDialog");
::EditBoxWrapper * gpDialogDefaultActionCallback = 0; D2_GLOBAL_GUARD(gpDialogDefaultActionCallback, "D2Client::UI::ui::gpDialogDefaultActionCallback");
::ButtonWrapper * gpUiGoldTransactionDialog = 0; D2_GLOBAL_GUARD(gpUiGoldTransactionDialog, "D2Client::UI::ui::gpUiGoldTransactionDialog");
::ButtonWrapper * gpUiGoldDialogCancelButton = 0; D2_GLOBAL_GUARD(gpUiGoldDialogCancelButton, "D2Client::UI::ui::gpUiGoldDialogCancelButton");
uint32_t gbKeyConfigModeActiveInDialog = 0; D2_GLOBAL_GUARD(gbKeyConfigModeActiveInDialog, "D2Client::UI::ui::gbKeyConfigModeActiveInDialog");
int WindowSizeY = 0; D2_GLOBAL_GUARD(WindowSizeY, "D2Client::UI::ui::WindowSizeY");
int gnPlayerCashDisplay = 0; D2_GLOBAL_GUARD(gnPlayerCashDisplay, "D2Client::UI::ui::gnPlayerCashDisplay");
uint32_t gnNpcShopActiveTab = 0; D2_GLOBAL_GUARD(gnNpcShopActiveTab, "D2Client::UI::ui::gnNpcShopActiveTab");
DC6 * gpUpgradeMenuDC6 = 0; D2_GLOBAL_GUARD(gpUpgradeMenuDC6, "D2Client::UI::ui::gpUpgradeMenuDC6");
DC6 * gpHoradricBackgroundDC6 = 0; D2_GLOBAL_GUARD(gpHoradricBackgroundDC6, "D2Client::UI::ui::gpHoradricBackgroundDC6");

} // namespace D2Client::UI::ui

namespace D2Client::GAME::SCmd {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char * gaLanguageNames_00711d04[15] = {
    (char*)"English",
    (char*)"Spanish",
    (char*)"German",
    (char*)"French",
    (char*)"Portuguese",
    (char*)"Italian",
    (char*)"Japanese",
    (char*)"Korean",
    (char*)"Simplified Chinese",
    (char*)"Traditional Chinese",
    (char*)"Polish",
    (char*)"Russian",
    (char*)"English with Korean Chat",
    (char*)"Unknown",
    (char*)"14"
}; D2_GLOBAL_GUARD(gaLanguageNames_00711d04, "D2Client::GAME::SCmd::gaLanguageNames_00711d04");

} // namespace D2Client::GAME::SCmd

namespace D2Client::Draw::LightMap {

// =============================================================================
// Initialized scalars
// =============================================================================

uint32_t gbLightmapColorEnabled = 0x1; D2_GLOBAL_GUARD(gbLightmapColorEnabled, "D2Client::Draw::LightMap::gbLightmapColorEnabled");
D2LightMapStrc * gpLightMap = 0; D2_GLOBAL_GUARD(gpLightMap, "D2Client::Draw::LightMap::gpLightMap");

} // namespace D2Client::Draw::LightMap

namespace D2Client::GAME::Rendering {

// =============================================================================
// Initialized scalars
// =============================================================================

int CurrentRendingEngine = 0x3; D2_GLOBAL_GUARD(CurrentRendingEngine, "D2Client::GAME::Rendering::CurrentRendingEngine");
uint32_t gnCurrentEngineIndex = 0; D2_GLOBAL_GUARD(gnCurrentEngineIndex, "D2Client::GAME::Rendering::gnCurrentEngineIndex");
uint32_t fps = 0; D2_GLOBAL_GUARD(fps, "D2Client::GAME::Rendering::fps");

} // namespace D2Client::GAME::Rendering

namespace D2Client::UI::Panel {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char * gaLanguageNames_00722414[15] = {
    (char*)"English",
    (char*)"Spanish",
    (char*)"German",
    (char*)"French",
    (char*)"Portuguese",
    (char*)"Italian",
    (char*)"Japanese",
    (char*)"Korean",
    (char*)"Simplified Chinese",
    (char*)"Traditional Chinese",
    (char*)"Polish",
    (char*)"Russian",
    (char*)"English with Korean Chat",
    (char*)"Unknown",
    (char*)"14"
}; D2_GLOBAL_GUARD(gaLanguageNames_00722414, "D2Client::UI::Panel::gaLanguageNames_00722414");

} // namespace D2Client::UI::Panel

namespace D2Client::UI::Party {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char * gaLanguageNames_00722470[15] = {
    (char*)"English",
    (char*)"Spanish",
    (char*)"German",
    (char*)"French",
    (char*)"Portuguese",
    (char*)"Italian",
    (char*)"Japanese",
    (char*)"Korean",
    (char*)"Simplified Chinese",
    (char*)"Traditional Chinese",
    (char*)"Polish",
    (char*)"Russian",
    (char*)"English with Korean Chat",
    (char*)"Unknown",
    (char*)"14"
}; D2_GLOBAL_GUARD(gaLanguageNames_00722470, "D2Client::UI::Party::gaLanguageNames_00722470");

} // namespace D2Client::UI::Party

namespace D2Client::UI::QuestLog {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

uint16_t gaQuestDescData_DenOfEvil[32] = {
    0xe82,
    0x4c,
    0x4,
    0xe97,
    0x40,
    0xe98,
    0x40,
    0xe99,
    0x40,
    0xe9a,
    0x40,
    0xe9c,
    0x40,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe9c,
    0x40,
    0xe90,
    0xe8d,
    0xe8f,
    0xe8d,
    0xe8e,
    0x40,
    0x0,
    0x0,
    0x0
}; D2_GLOBAL_GUARD(gaQuestDescData_DenOfEvil, "D2Client::UI::QuestLog::gaQuestDescData_DenOfEvil");

uint16_t gaQuestDescData_SistersBurialGrounds[32] = {
    0xe83,
    0x5c,
    0x2,
    0xe9d,
    0x51,
    0xe9e,
    0x51,
    0xe9f,
    0x51,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe9f,
    0x51,
    0xe90,
    0xe8d,
    0xe8f,
    0xe8d,
    0xe8e,
    0x51,
    0x0,
    0x0,
    0x0
}; D2_GLOBAL_GUARD(gaQuestDescData_SistersBurialGrounds, "D2Client::UI::QuestLog::gaQuestDescData_SistersBurialGrounds");

uint16_t gaQuestDescData_ToolsOfTheTrade[32] = {
    0xe84,
    0xa3,
    0xffff,
    0xeab,
    0x92,
    0xeac,
    0x92,
    0xe95,
    0x92,
    0xe94,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xead,
    0xa3,
    0xe90,
    0xe8d,
    0xe8f,
    0xe8d,
    0xe8e,
    0x92,
    0x0,
    0x0,
    0x0
}; D2_GLOBAL_GUARD(gaQuestDescData_ToolsOfTheTrade, "D2Client::UI::QuestLog::gaQuestDescData_ToolsOfTheTrade");

uint16_t gaQuestDescData_SearchForCain[32] = {
    0xe85,
    0x7b,
    0xffff,
    0xea0,
    0x61,
    0xea1,
    0x61,
    0xea2,
    0x61,
    0xea3,
    0x61,
    0xea4,
    0x61,
    0xea5,
    0x61,
    0xe96,
    0x61,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xea6,
    0x61,
    0xe90,
    0xe8d,
    0xe8f,
    0xe8d,
    0xe8e,
    0x61,
    0x0,
    0x0,
    0x0
}; D2_GLOBAL_GUARD(gaQuestDescData_SearchForCain, "D2Client::UI::QuestLog::gaQuestDescData_SearchForCain");

uint16_t gaQuestDescData_ForgottenTower[32] = {
    0xe86,
    0x7f,
    0xffff,
    0xea7,
    0x7f,
    0xeaa,
    0x7f,
    0xea8,
    0x7f,
    0xea9,
    0x7f,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe90,
    0xe8d,
    0xe8f,
    0xe8d,
    0xe8e,
    0x7f,
    0x0,
    0x0,
    0x0
}; D2_GLOBAL_GUARD(gaQuestDescData_ForgottenTower, "D2Client::UI::QuestLog::gaQuestDescData_ForgottenTower");

uint16_t gaQuestDescData_SistersToTheSlaughter[32] = {
    0xe87,
    0xb8,
    0x9,
    0xeae,
    0xa6,
    0xeaf,
    0xa6,
    0xeb1,
    0xa6,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xeb0,
    0xa6,
    0xe90,
    0xe8d,
    0xe8f,
    0xe8d,
    0xe8e,
    0xb8,
    0x0,
    0x0,
    0x0
}; D2_GLOBAL_GUARD(gaQuestDescData_SistersToTheSlaughter, "D2Client::UI::QuestLog::gaQuestDescData_SistersToTheSlaughter");

uint16_t gaQuestDescData_RadamentsLair[32] = {
    0x39b,
    0x14e,
    0x2,
    0x3ad,
    0x130,
    0x3ae,
    0x130,
    0x3af,
    0x130,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0x3af,
    0xe8d,
    0xe90,
    0xe8d,
    0xe8f,
    0xe8d,
    0xe8e,
    0x130,
    0x0,
    0x0,
    0x0
}; D2_GLOBAL_GUARD(gaQuestDescData_RadamentsLair, "D2Client::UI::QuestLog::gaQuestDescData_RadamentsLair");

uint16_t gaQuestDescData_HoradricStaff[32] = {
    0x39c,
    0x153,
    0xffff,
    0x3b0,
    0xe8d,
    0x3b1,
    0x14f,
    0x3b2,
    0x152,
    0x3b4,
    0xe8d,
    0x3b3,
    0x153,
    0x3b3,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe92,
    0xe8d,
    0x3b3,
    0xe8d,
    0xe90,
    0xe8d,
    0xe8f,
    0xe8d,
    0xe8e,
    0x153,
    0x0,
    0x0,
    0x0
}; D2_GLOBAL_GUARD(gaQuestDescData_HoradricStaff, "D2Client::UI::QuestLog::gaQuestDescData_HoradricStaff");

uint16_t gaQuestDescData_TaintedSun[32] = {
    0x39d,
    0xe8d,
    0xffff,
    0x3b6,
    0xe8d,
    0x3b7,
    0xe8d,
    0x3b8,
    0x15c,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0x3b8,
    0xe8d,
    0xe90,
    0xe8d,
    0xe8f,
    0xe8d,
    0xe8e,
    0xe8d,
    0x0,
    0x0,
    0x0
}; D2_GLOBAL_GUARD(gaQuestDescData_TaintedSun, "D2Client::UI::QuestLog::gaQuestDescData_TaintedSun");

uint16_t gaQuestDescData_ArcaneSanctuary[32] = {
    0x39e,
    0xe8d,
    0xffff,
    0x3ba,
    0xe8d,
    0x3b9,
    0x175,
    0x3b9,
    0x179,
    0x3bb,
    0x179,
    0x3bc,
    0x18c,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0x3bc,
    0x18c,
    0xe90,
    0x18c,
    0xe8f,
    0xe8d,
    0xe8e,
    0x18c,
    0x0,
    0x0,
    0x0
}; D2_GLOBAL_GUARD(gaQuestDescData_ArcaneSanctuary, "D2Client::UI::QuestLog::gaQuestDescData_ArcaneSanctuary");

uint16_t gaQuestDescData_TheSummoner[32] = {
    0x39f,
    0xe8d,
    0xffff,
    0x3bd,
    0xe8d,
    0x3be,
    0xe8d,
    0xe8e,
    0xe8d,
    0x3bf,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0xe8d,
    0x3bf,
    0xe8d,
    0xe90,
    0xe8d,
    0xe8f,
    0xe8d,
    0xe8e,
    0xe8d,
    0x0,
    0x0,
    0x0
}; D2_GLOBAL_GUARD(gaQuestDescData_TheSummoner, "D2Client::UI::QuestLog::gaQuestDescData_TheSummoner");

} // namespace D2Client::UI::QuestLog

namespace D2Client::UI::npcmenu {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char * gaLanguageNames_007273a8[15] = {
    (char*)"English",
    (char*)"Spanish",
    (char*)"German",
    (char*)"French",
    (char*)"Portuguese",
    (char*)"Italian",
    (char*)"Japanese",
    (char*)"Korean",
    (char*)"Simplified Chinese",
    (char*)"Traditional Chinese",
    (char*)"Polish",
    (char*)"Russian",
    (char*)"English with Korean Chat",
    (char*)"Unknown",
    (char*)"14"
}; D2_GLOBAL_GUARD(gaLanguageNames_007273a8, "D2Client::UI::npcmenu::gaLanguageNames_007273a8");

// =============================================================================
// Initialized scalars
// =============================================================================

uint32_t gnMonsterSpecialType = 0; D2_GLOBAL_GUARD(gnMonsterSpecialType, "D2Client::UI::npcmenu::gnMonsterSpecialType");
uint32_t gnNpcMenuMercHireCost1 = 0; D2_GLOBAL_GUARD(gnNpcMenuMercHireCost1, "D2Client::UI::npcmenu::gnNpcMenuMercHireCost1");
uint32_t gnNpcMenuMercHireCost2 = 0; D2_GLOBAL_GUARD(gnNpcMenuMercHireCost2, "D2Client::UI::npcmenu::gnNpcMenuMercHireCost2");

} // namespace D2Client::UI::npcmenu

namespace D2Client::Sound {

// =============================================================================
// Initialized scalars
// =============================================================================

int nSoundVolumeLeftChannel = 0x64; D2_GLOBAL_GUARD(nSoundVolumeLeftChannel, "D2Client::Sound::nSoundVolumeLeftChannel");
int nSoundVolumeRightChannel = 0x64; D2_GLOBAL_GUARD(nSoundVolumeRightChannel, "D2Client::Sound::nSoundVolumeRightChannel");
int32_t int32_t_007c8c7c = 0; D2_GLOBAL_GUARD(int32_t_007c8c7c, "D2Client::Sound::int32_t_007c8c7c");
uint32_t gnActiveSoundChannelCount = 0; D2_GLOBAL_GUARD(gnActiveSoundChannelCount, "D2Client::Sound::gnActiveSoundChannelCount");

} // namespace D2Client::Sound

namespace D2Client::UI::UseItem {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

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
}; D2_GLOBAL_GUARD(gUIFlagMapInit, "D2Client::UI::UseItem::gUIFlagMapInit");

// =============================================================================
// Initialized scalars
// =============================================================================

int nCashDisplayItemCount = 0x2; D2_GLOBAL_GUARD(nCashDisplayItemCount, "D2Client::UI::UseItem::nCashDisplayItemCount");
uint32_t nHoradricCubeUIState = 0; D2_GLOBAL_GUARD(nHoradricCubeUIState, "D2Client::UI::UseItem::nHoradricCubeUIState");
uint32_t gnCursorItemUnitId = 0; D2_GLOBAL_GUARD(gnCursorItemUnitId, "D2Client::UI::UseItem::gnCursorItemUnitId");
uint32_t gnTextSystemInitFlag = 0; D2_GLOBAL_GUARD(gnTextSystemInitFlag, "D2Client::UI::UseItem::gnTextSystemInitFlag");

} // namespace D2Client::UI::UseItem

namespace D2Client::UNIT::UnitSnd {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

int gaUnitSoundIdsAmazon[11] = {
    0xb3e,
    0xb43,
    0xb89,
    0xb8d,
    0xb8b,
    0xb76,
    0xb78,
    0xb8f,
    0xb79,
    0xb91,
    0xad0
}; D2_GLOBAL_GUARD(gaUnitSoundIdsAmazon, "D2Client::UNIT::UnitSnd::gaUnitSoundIdsAmazon");

char * gaLanguageNames_0072a300[15] = {
    (char*)"English",
    (char*)"Spanish",
    (char*)"German",
    (char*)"French",
    (char*)"Portuguese",
    (char*)"Italian",
    (char*)"Japanese",
    (char*)"Korean",
    (char*)"Simplified Chinese",
    (char*)"Traditional Chinese",
    (char*)"Polish",
    (char*)"Russian",
    (char*)"English with Korean Chat",
    (char*)"Unknown",
    (char*)"14"
}; D2_GLOBAL_GUARD(gaLanguageNames_0072a300, "D2Client::UNIT::UnitSnd::gaLanguageNames_0072a300");

int gaLevelAmbientMusicPlayedFlags[1024] = {}; D2_GLOBAL_GUARD(gaLevelAmbientMusicPlayedFlags, "D2Client::UNIT::UnitSnd::gaLevelAmbientMusicPlayedFlags");

// =============================================================================
// Initialized scalars
// =============================================================================

int gnSoundLastInitTick = 0; D2_GLOBAL_GUARD(gnSoundLastInitTick, "D2Client::UNIT::UnitSnd::gnSoundLastInitTick");
uint32_t gdwSoundChannelIndex = 0; D2_GLOBAL_GUARD(gdwSoundChannelIndex, "D2Client::UNIT::UnitSnd::gdwSoundChannelIndex");
uint32_t gnSoundInitCooldown = 0; D2_GLOBAL_GUARD(gnSoundInitCooldown, "D2Client::UNIT::UnitSnd::gnSoundInitCooldown");
unsigned int gnSoundLastPlayerAnimModeTime = {}; D2_GLOBAL_GUARD(gnSoundLastPlayerAnimModeTime, "D2Client::UNIT::UnitSnd::gnSoundLastPlayerAnimModeTime");
int geSoundLastPlayerAnimMode = 0; D2_GLOBAL_GUARD(geSoundLastPlayerAnimMode, "D2Client::UNIT::UnitSnd::geSoundLastPlayerAnimMode");

} // namespace D2Client::UNIT::UnitSnd

namespace D2Client::UI::SkillDesc {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char * gaLanguageNames_0072d8d8[15] = {
    (char*)"English",
    (char*)"Spanish",
    (char*)"German",
    (char*)"French",
    (char*)"Portuguese",
    (char*)"Italian",
    (char*)"Japanese",
    (char*)"Korean",
    (char*)"Simplified Chinese",
    (char*)"Traditional Chinese",
    (char*)"Polish",
    (char*)"Russian",
    (char*)"English with Korean Chat",
    (char*)"Unknown",
    (char*)"14"
}; D2_GLOBAL_GUARD(gaLanguageNames_0072d8d8, "D2Client::UI::SkillDesc::gaLanguageNames_0072d8d8");

char * gaLanguageNames_0072d914[15] = {
    (char*)"English",
    (char*)"Spanish",
    (char*)"German",
    (char*)"French",
    (char*)"Portuguese",
    (char*)"Italian",
    (char*)"Japanese",
    (char*)"Korean",
    (char*)"Simplified Chinese",
    (char*)"Traditional Chinese",
    (char*)"Polish",
    (char*)"Russian",
    (char*)"English with Korean Chat",
    (char*)"Unknown",
    (char*)"14"
}; D2_GLOBAL_GUARD(gaLanguageNames_0072d914, "D2Client::UI::SkillDesc::gaLanguageNames_0072d914");

char * gaLanguageNames_0072d950[15] = {
    (char*)"English",
    (char*)"Spanish",
    (char*)"German",
    (char*)"French",
    (char*)"Portuguese",
    (char*)"Italian",
    (char*)"Japanese",
    (char*)"Korean",
    (char*)"Simplified Chinese",
    (char*)"Traditional Chinese",
    (char*)"Polish",
    (char*)"Russian",
    (char*)"English with Korean Chat",
    (char*)"Unknown",
    (char*)"14"
}; D2_GLOBAL_GUARD(gaLanguageNames_0072d950, "D2Client::UI::SkillDesc::gaLanguageNames_0072d950");

} // namespace D2Client::UI::SkillDesc

namespace D2Glide::Renderer::Glide {

} // namespace D2Glide::Renderer::Glide

namespace D2DDraw::Renderer::DirectDraw {

} // namespace D2DDraw::Renderer::DirectDraw

namespace D2Lang::Unicode::UNISYS {

} // namespace D2Lang::Unicode::UNISYS

namespace D2Net::SRC::Client {

} // namespace D2Net::SRC::Client

namespace D2Game::PLAYER::PlayerSave {

} // namespace D2Game::PLAYER::PlayerSave

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

namespace D2Game::Monster::Create {

} // namespace D2Game::Monster::Create

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

namespace D2Game::SKILLS::SkillNec {

} // namespace D2Game::SKILLS::SkillNec

namespace D2CMP::SRC::CelCmp::CELLIST {

} // namespace D2CMP::SRC::CelCmp::CELLIST

namespace D2CMP {

} // namespace D2CMP

namespace D2CMP::Draw {

} // namespace D2CMP::Draw

namespace D2Common::Chat::Ignorelist::MappingEntryNode {

} // namespace D2Common::Chat::Ignorelist::MappingEntryNode

namespace D2Common::Chat::Ignorelist::StringEntryNode {

} // namespace D2Common::Chat::Ignorelist::StringEntryNode

namespace D2Common {

} // namespace D2Common

namespace D2Common::Chat::Ignorelist {

} // namespace D2Common::Chat::Ignorelist

namespace D2Common::DUNGEON::Dungeon {

} // namespace D2Common::DUNGEON::Dungeon

namespace D2Common::Environment {

} // namespace D2Common::Environment

namespace D2Common::Units {

} // namespace D2Common::Units

namespace D2Common::Monsters::MONSTERS {

} // namespace D2Common::Monsters::MONSTERS

namespace D2Common::Merc {

} // namespace D2Common::Merc

namespace D2Common::DATATBLS::OverlayTbls {

} // namespace D2Common::DATATBLS::OverlayTbls

namespace D2Common::Drlg::DrlgGrid {

} // namespace D2Common::Drlg::DrlgGrid

namespace D2Direct3D::Renderer {

} // namespace D2Direct3D::Renderer

namespace Fog::Src {

} // namespace Fog::Src

namespace D2Gdi::Renderer::Windowed {

} // namespace D2Gdi::Renderer::Windowed

namespace D2Launch::Launcher {

} // namespace D2Launch::Launcher

namespace D2Multi::OOGUtilities {

} // namespace D2Multi::OOGUtilities

namespace D2Client::Lng {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char lpLibFileName_007a0710[260] = {}; D2_GLOBAL_GUARD(lpLibFileName_007a0710, "D2Client::Lng::lpLibFileName_007a0710");

// =============================================================================
// Initialized scalars
// =============================================================================

HMODULE ghLngLibrary = 0; D2_GLOBAL_GUARD(ghLngLibrary, "D2Client::Lng::ghLngLibrary");

} // namespace D2Client::Lng

namespace D2Client::UI::Dialog {

// =============================================================================
// Initialized scalars
// =============================================================================

D2DialogStrc * gpDialog = 0; D2_GLOBAL_GUARD(gpDialog, "D2Client::UI::Dialog::gpDialog");
D2DialogStrc * gpTradeAgreeDialog = 0; D2_GLOBAL_GUARD(gpTradeAgreeDialog, "D2Client::UI::Dialog::gpTradeAgreeDialog");
D2DialogStrc * gpTradeCancelDialog = 0; D2_GLOBAL_GUARD(gpTradeCancelDialog, "D2Client::UI::Dialog::gpTradeCancelDialog");
D2DialogStrc * gpHoradricCubeDialog = 0; D2_GLOBAL_GUARD(gpHoradricCubeDialog, "D2Client::UI::Dialog::gpHoradricCubeDialog");

} // namespace D2Client::UI::Dialog

namespace D2Client::UI::SpellSel {

// =============================================================================
// Initialized scalars
// =============================================================================

int WindowSizeX = 0; D2_GLOBAL_GUARD(WindowSizeX, "D2Client::UI::SpellSel::WindowSizeX");

} // namespace D2Client::UI::SpellSel

namespace D2Client::UNIT::CUnit {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2UnitStrc * ClientSideUnitHashTables[6][128] = {}; D2_GLOBAL_GUARD(ClientSideUnitHashTables, "D2Client::UNIT::CUnit::ClientSideUnitHashTables");

D2UnitStrc * ServerSideUnitHashTables[6][128] = {}; D2_GLOBAL_GUARD(ServerSideUnitHashTables, "D2Client::UNIT::CUnit::ServerSideUnitHashTables");

// =============================================================================
// Initialized scalars
// =============================================================================

D2UnitStrc * ClientSideUnitHashTables_1__0_[6][128] = {}; D2_GLOBAL_GUARD(ClientSideUnitHashTables_1__0_, "D2Client::UNIT::CUnit::ClientSideUnitHashTables_1__0_");
D2UnitStrc * ClientSideUnitHashTables_1__1_[6][128] = {}; D2_GLOBAL_GUARD(ClientSideUnitHashTables_1__1_, "D2Client::UNIT::CUnit::ClientSideUnitHashTables_1__1_");
D2UnitStrc * ClientSideUnitHashTables_2__0_[6][128] = {}; D2_GLOBAL_GUARD(ClientSideUnitHashTables_2__0_, "D2Client::UNIT::CUnit::ClientSideUnitHashTables_2__0_");
D2UnitStrc * ClientSideUnitHashTables_3__0_[6][128] = {}; D2_GLOBAL_GUARD(ClientSideUnitHashTables_3__0_, "D2Client::UNIT::CUnit::ClientSideUnitHashTables_3__0_");
D2UnitStrc * ServerSideUnitHashTables_0__1_[6][128] = {}; D2_GLOBAL_GUARD(ServerSideUnitHashTables_0__1_, "D2Client::UNIT::CUnit::ServerSideUnitHashTables_0__1_");
D2UnitStrc * ServerSideUnitHashTables_1__0_[6][128] = {}; D2_GLOBAL_GUARD(ServerSideUnitHashTables_1__0_, "D2Client::UNIT::CUnit::ServerSideUnitHashTables_1__0_");
D2UnitStrc * ServerSideUnitHashTables_1__1_[6][128] = {}; D2_GLOBAL_GUARD(ServerSideUnitHashTables_1__1_, "D2Client::UNIT::CUnit::ServerSideUnitHashTables_1__1_");
D2UnitStrc * ServerSideUnitHashTables_2__0_[6][128] = {}; D2_GLOBAL_GUARD(ServerSideUnitHashTables_2__0_, "D2Client::UNIT::CUnit::ServerSideUnitHashTables_2__0_");
D2UnitStrc * ServerSideUnitHashTables_3__0_[6][128] = {}; D2_GLOBAL_GUARD(ServerSideUnitHashTables_3__0_, "D2Client::UNIT::CUnit::ServerSideUnitHashTables_3__0_");
D2UnitStrc * ServerSideUnitHashTables_4__0_[6][128] = {}; D2_GLOBAL_GUARD(ServerSideUnitHashTables_4__0_, "D2Client::UNIT::CUnit::ServerSideUnitHashTables_4__0_");
D2UnitStrc * pPlayerUnit = 0; D2_GLOBAL_GUARD(pPlayerUnit, "D2Client::UNIT::CUnit::pPlayerUnit");

} // namespace D2Client::UNIT::CUnit

namespace D2Client::Cursor {

// =============================================================================
// Initialized scalars
// =============================================================================

int gnCursorInitialized = 0; D2_GLOBAL_GUARD(gnCursorInitialized, "D2Client::Cursor::gnCursorInitialized");

} // namespace D2Client::Cursor

namespace D2Client::Engine::Gfx {

// =============================================================================
// Initialized scalars
// =============================================================================

int gnGfxDirectionCountCache = 0; D2_GLOBAL_GUARD(gnGfxDirectionCountCache, "D2Client::Engine::Gfx::gnGfxDirectionCountCache");

} // namespace D2Client::Engine::Gfx

namespace D2Client::UNIT::Monster {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

uint16_t gnMonsterUniqueNameBuffer[2] = {}; D2_GLOBAL_GUARD(gnMonsterUniqueNameBuffer, "D2Client::UNIT::Monster::gnMonsterUniqueNameBuffer");

} // namespace D2Client::UNIT::Monster

namespace D2Client::UI::Trade {

// =============================================================================
// Initialized scalars
// =============================================================================

uint32_t gdwTradeThrottleTimestamp = 0; D2_GLOBAL_GUARD(gdwTradeThrottleTimestamp, "D2Client::UI::Trade::gdwTradeThrottleTimestamp");
bool gbTradeResetFlag = 0; D2_GLOBAL_GUARD(gbTradeResetFlag, "D2Client::UI::Trade::gbTradeResetFlag");
int32_t g_nTradeWindowState = 0; D2_GLOBAL_GUARD(g_nTradeWindowState, "D2Client::UI::Trade::g_nTradeWindowState");

} // namespace D2Client::UI::Trade

namespace D2Client::UNIT::Object {

// =============================================================================
// Initialized scalars
// =============================================================================

DC6 * gpMessageLogDC6 = 0; D2_GLOBAL_GUARD(gpMessageLogDC6, "D2Client::UNIT::Object::gpMessageLogDC6");

} // namespace D2Client::UNIT::Object

namespace D2Client::GAME::Sound {

// =============================================================================
// Initialized scalars
// =============================================================================

BOOL pAudioMixerInterface = 0; D2_GLOBAL_GUARD(pAudioMixerInterface, "D2Client::GAME::Sound::pAudioMixerInterface");

} // namespace D2Client::GAME::Sound

namespace D2gfx {

} // namespace D2gfx

namespace D2Win::D2GFX {

} // namespace D2Win::D2GFX

namespace D2Win::Src::D2WinMain {

} // namespace D2Win::Src::D2WinMain

namespace D2Win::Src::D2WinButton {

} // namespace D2Win::Src::D2WinButton

namespace D2Win::Src::D2WinArchive {

} // namespace D2Win::Src::D2WinArchive

namespace D2Win::Src::D2WinTextBox {

} // namespace D2Win::Src::D2WinTextBox

namespace D2Sound {

} // namespace D2Sound

namespace D2Sound::Src::D2SoundFast {

} // namespace D2Sound::Src::D2SoundFast

namespace Bnclient::Bnet {

} // namespace Bnclient::Bnet

namespace Bnclient::BnDownload {

} // namespace Bnclient::BnDownload

namespace Bnclient {

} // namespace Bnclient

namespace D2Net::Game::Server {

} // namespace D2Net::Game::Server

namespace D2Game::UNIT::SUNITNPC {

} // namespace D2Game::UNIT::SUNITNPC

namespace D2Game::UNIT::sunitproxy {

} // namespace D2Game::UNIT::sunitproxy

namespace D2Game::GAME::Clients {

} // namespace D2Game::GAME::Clients

namespace D2Game::Quests::QUESTS {

} // namespace D2Game::Quests::QUESTS

namespace D2Common::DATATBLS::MonsterTbls {

} // namespace D2Common::DATATBLS::MonsterTbls

namespace D2Common::Logging {

} // namespace D2Common::Logging

namespace D2Common::DATATBLS::TokTbls {

} // namespace D2Common::DATATBLS::TokTbls

namespace Fog::Debug {

} // namespace Fog::Debug
