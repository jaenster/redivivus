/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"
#include "Bnclient/BnSend.h"
#include "Bnclient/SRP.h"
#include "Bnclient/net_sid.h"
#include "D2Client/D2BNetGatewayAccessStr.h"


// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

uint8_t gaBnSendRawBytesHeader[4] = {}; D2_GLOBAL_GUARD(gaBnSendRawBytesHeader, "gaBnSendRawBytesHeader");

char gaCDKeyAuthData[4] = { ' ', ',', ';', '\x00' }; D2_GLOBAL_GUARD(gaCDKeyAuthData, "gaCDKeyAuthData");

char s_ColonSeparator[2] = { ':', '\x00' }; D2_GLOBAL_GUARD(s_ColonSeparator, "s_ColonSeparator");

D2BNetGatewayAccessStr ActualRealmInfo = {}; D2_GLOBAL_GUARD(ActualRealmInfo, "ActualRealmInfo");

D2BnNewsTableStrc g_nNewsMaxCacheSize = {}; D2_GLOBAL_GUARD(g_nNewsMaxCacheSize, "g_nNewsMaxCacheSize");

char gbBnetFtpFileTransferActive[256] = {}; D2_GLOBAL_GUARD(gbBnetFtpFileTransferActive, "gbBnetFtpFileTransferActive");

char gszBNetChatInputBuffer[260] = {}; D2_GLOBAL_GUARD(gszBNetChatInputBuffer, "gszBNetChatInputBuffer");

_RTL_CRITICAL_SECTION lpCriticalSection_00881f64 = {}; D2_GLOBAL_GUARD(lpCriticalSection_00881f64, "lpCriticalSection_00881f64");

char g_szBnetConnectionStateString[128] = {}; D2_GLOBAL_GUARD(g_szBnetConnectionStateString, "g_szBnetConnectionStateString");

_RTL_CRITICAL_SECTION lpCriticalSection_00882004 = {}; D2_GLOBAL_GUARD(lpCriticalSection_00882004, "lpCriticalSection_00882004");

char g_szCurrentChannelName[256] = {}; D2_GLOBAL_GUARD(g_szCurrentChannelName, "g_szCurrentChannelName");

BOOL NET_SID_INCOMING_CLIENT_PacketRequireAnswer[94] = {}; D2_GLOBAL_GUARD(NET_SID_INCOMING_CLIENT_PacketRequireAnswer, "NET_SID_INCOMING_CLIENT_PacketRequireAnswer");

uint32_t g_SIDPacketResponseFlags[94] = {}; D2_GLOBAL_GUARD(g_SIDPacketResponseFlags, "g_SIDPacketResponseFlags");

char gszCDKeyInUseByUsername[256] = {}; D2_GLOBAL_GUARD(gszCDKeyInUseByUsername, "gszCDKeyInUseByUsername");

char g_szStoredUsername[256] = {}; D2_GLOBAL_GUARD(g_szStoredUsername, "g_szStoredUsername");

D2SrpSaltVerifierStrc g_srpSaltVerifier = {}; D2_GLOBAL_GUARD(g_srpSaltVerifier, "g_srpSaltVerifier");

D2SrpCredentialsStrc g_srpCredentials = {}; D2_GLOBAL_GUARD(g_srpCredentials, "g_srpCredentials");

uint8_t g_SHA1Contexts[276] = {}; D2_GLOBAL_GUARD(g_SHA1Contexts, "g_SHA1Contexts");

uint8_t g_IDEAKeySchedules[312] = {}; D2_GLOBAL_GUARD(g_IDEAKeySchedules, "g_IDEAKeySchedules");

// =============================================================================
// Initialized scalars
// =============================================================================

int gnBnetLogFileHandle = 0; D2_GLOBAL_GUARD(gnBnetLogFileHandle, "gnBnetLogFileHandle");
unsigned int gbBnetLogInitialized = {}; D2_GLOBAL_GUARD(gbBnetLogInitialized, "gbBnetLogInitialized");
uint32_t g_nBNetServerToken = 0; D2_GLOBAL_GUARD(g_nBNetServerToken, "g_nBNetServerToken");
uint32_t g_nBNetClientToken = 0; D2_GLOBAL_GUARD(g_nBNetClientToken, "g_nBNetClientToken");
void * g_pBNetChannelListData = 0; D2_GLOBAL_GUARD(g_pBNetChannelListData, "g_pBNetChannelListData");
void * gpRealmQueryData = 0; D2_GLOBAL_GUARD(gpRealmQueryData, "gpRealmQueryData");
void * g_pBNetGameListData = 0; D2_GLOBAL_GUARD(g_pBNetGameListData, "g_pBNetGameListData");
void * gpBNetWriteUserData = 0; D2_GLOBAL_GUARD(gpBNetWriteUserData, "gpBNetWriteUserData");
void * g_pBNetReadUserData = 0; D2_GLOBAL_GUARD(g_pBNetReadUserData, "g_pBNetReadUserData");
void * g_pBNetCDKeyHashData = 0; D2_GLOBAL_GUARD(g_pBNetCDKeyHashData, "g_pBNetCDKeyHashData");
uint32_t g_nBNetConnectionState = 0; D2_GLOBAL_GUARD(g_nBNetConnectionState, "g_nBNetConnectionState");
LONG * g_pBNetSocketData = 0; D2_GLOBAL_GUARD(g_pBNetSocketData, "g_pBNetSocketData");
// also labelled: lpAddend_00881bf4
LONG * Destination_00881bf4 = 0; D2_GLOBAL_GUARD(Destination_00881bf4, "Destination_00881bf4");
uint32_t gbBNetSocketConnected = 0; D2_GLOBAL_GUARD(gbBNetSocketConnected, "gbBNetSocketConnected");
long Target_00881bfc = 0; D2_GLOBAL_GUARD(Target_00881bfc, "Target_00881bfc");
// also labelled: lpAddend_00881c00
LONG * Destination_00881c00 = 0; D2_GLOBAL_GUARD(Destination_00881c00, "Destination_00881c00");
uint32_t g_nBNetThreadInitFlag = 0; D2_GLOBAL_GUARD(g_nBNetThreadInitFlag, "g_nBNetThreadInitFlag");
uint32_t g_pfnNewsChannelCallback = 0; D2_GLOBAL_GUARD(g_pfnNewsChannelCallback, "g_pfnNewsChannelCallback");
pfnNewsEntryCallback g_pfnMotdCallback = (pfnNewsEntryCallback)0; D2_GLOBAL_GUARD(g_pfnMotdCallback, "g_pfnMotdCallback");
pfnVoidNoArgCallback g_pfnNewsCallback = (pfnVoidNoArgCallback)0; D2_GLOBAL_GUARD(g_pfnNewsCallback, "g_pfnNewsCallback");
uint32_t g_nFirstUnreadNewsIndex = 0; D2_GLOBAL_GUARD(g_nFirstUnreadNewsIndex, "g_nFirstUnreadNewsIndex");
uint32_t g_dwNewsTimestamp = 0; D2_GLOBAL_GUARD(g_dwNewsTimestamp, "g_dwNewsTimestamp");
unsigned int gnNetworkSendState = {}; D2_GLOBAL_GUARD(gnNetworkSendState, "gnNetworkSendState");
uint32_t g_nBattleNetConnectionData = 0; D2_GLOBAL_GUARD(g_nBattleNetConnectionData, "g_nBattleNetConnectionData");
uint32_t g_nBattleNetConnectionState = 0; D2_GLOBAL_GUARD(g_nBattleNetConnectionState, "g_nBattleNetConnectionState");
uint32_t g_nSIDResponseCount1 = 0; D2_GLOBAL_GUARD(g_nSIDResponseCount1, "g_nSIDResponseCount1");
uint32_t g_nSIDResponseCount2 = 0; D2_GLOBAL_GUARD(g_nSIDResponseCount2, "g_nSIDResponseCount2");
uint32_t g_bSIDProtocolReady = 0; D2_GLOBAL_GUARD(g_bSIDProtocolReady, "g_bSIDProtocolReady");
uint32_t g_dwLastAdId = 0; D2_GLOBAL_GUARD(g_dwLastAdId, "g_dwLastAdId");
uint32_t g_bSIDFlag2 = 0; D2_GLOBAL_GUARD(g_bSIDFlag2, "g_bSIDFlag2");

namespace D2Common::Drlg::Maze {

} // namespace D2Common::Drlg::Maze

namespace Fog::Src::Excel {

} // namespace Fog::Src::Excel

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

namespace D2Client::UNIT::Object {

} // namespace D2Client::UNIT::Object

namespace D2Client::UI::ListBox::ListBoxWrapper {

} // namespace D2Client::UI::ListBox::ListBoxWrapper

namespace D2Client::UI::ListBox::ListBoxImplementation {

} // namespace D2Client::UI::ListBox::ListBoxImplementation

namespace D2Win::_FONT {

} // namespace D2Win::_FONT

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

namespace D2Client::UI::Automap {

} // namespace D2Client::UI::Automap

namespace D2Game::GAME::Rendering {

} // namespace D2Game::GAME::Rendering

namespace D2Client::UI::Help {

} // namespace D2Client::UI::Help

namespace D2Client::UI::npcmenu {

} // namespace D2Client::UI::npcmenu

namespace D2Client::NPC {

} // namespace D2Client::NPC

namespace D2Client::UNIT::UnitSnd {

} // namespace D2Client::UNIT::UnitSnd

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

namespace D2Common::Monsters::MONSTERS::Umod {

} // namespace D2Common::Monsters::MONSTERS::Umod

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

namespace D2Client::Waypoint {

} // namespace D2Client::Waypoint

namespace D2Common::Drlg::DrlgGrid {

} // namespace D2Common::Drlg::DrlgGrid

namespace Fog::Memory {

} // namespace Fog::Memory

namespace Fog::Debug {

} // namespace Fog::Debug

namespace D2Client::MCP {

} // namespace D2Client::MCP

namespace D2Client::Bnet {

} // namespace D2Client::Bnet

namespace D2Game::Save {

} // namespace D2Game::Save

namespace D2Game::GameData {

} // namespace D2Game::GameData

namespace D2Client::GAME::Record {

} // namespace D2Client::GAME::Record

namespace D2Client::UI::Dialog {

} // namespace D2Client::UI::Dialog

namespace D2Client::_Selection {

} // namespace D2Client::_Selection

namespace D2Client::Cursor {

} // namespace D2Client::Cursor

namespace D2Client::Draw::Weather {

} // namespace D2Client::Draw::Weather

namespace D2Client::Draw::LightMap {

} // namespace D2Client::Draw::LightMap

namespace D2Client::GAME::View {

} // namespace D2Client::GAME::View

namespace D2Client::GAME::RosterPets {

} // namespace D2Client::GAME::RosterPets

namespace D2Client::GAME::Roster {

} // namespace D2Client::GAME::Roster

namespace D2Client::UI::Inv {

} // namespace D2Client::UI::Inv

namespace D2Client::UI::Hireables {

} // namespace D2Client::UI::Hireables

namespace D2Client::UI::Panel {

} // namespace D2Client::UI::Panel

namespace D2Client::UI::Party {

} // namespace D2Client::UI::Party

namespace D2Client::_Interaction {

} // namespace D2Client::_Interaction

namespace D2Client::Merc {

} // namespace D2Client::Merc

namespace D2Client::Mouse {

} // namespace D2Client::Mouse

namespace D2Client::Forms {

} // namespace D2Client::Forms

namespace D2Client::_CdKey {

// =============================================================================
// Initialized scalars
// =============================================================================

char * KeyClassic = 0; D2_GLOBAL_GUARD(KeyClassic, "D2Client::_CdKey::KeyClassic");
char * KeyExpansion = 0; D2_GLOBAL_GUARD(KeyExpansion, "D2Client::_CdKey::KeyExpansion");
char * KeyOwner = 0; D2_GLOBAL_GUARD(KeyOwner, "D2Client::_CdKey::KeyOwner");

} // namespace D2Client::_CdKey

namespace D2Direct3D::Renderer {

} // namespace D2Direct3D::Renderer
