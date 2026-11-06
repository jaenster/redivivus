/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "Source/globals/globals.h"
#include "Source/D2CMP/Draw.h"
#include "Source/D2CMP/SRC/Codec.h"
#include "Source/D2CMP/SRC/LRUCache.h"
#include "Source/D2Common/COMPOSIT/Composit.h"
#include "Source/D2Common/D2Common.globals.h"
#include "Source/D2Common/D2SeedStrc.h"
#include "Source/D2Common/DATATBLS/ItemTbls.h"
#include "Source/D2Common/DATATBLS/MonsterTbls.h"
#include "Source/D2Common/DUNGEON/Dungeon.h"
#include "Source/D2Common/Drlg/D2DrlgLevelDataList.h"
#include "Source/D2Common/Drlg/D2DrlgReplaceRoomStrc.h"
#include "Source/D2Common/Drlg/Drlg.h"
#include "Source/D2Common/Drlg/OutSiege.h"
#include "Source/D2Common/INVENTORY/Inventory.h"
#include "Source/D2Common/ITEMS/ItemMods.h"
#include "Source/D2Common/ITEMS/Items.h"
#include "Source/D2Common/Logging/ProfCore.h"
#include "Source/D2Common/Merc.h"
#include "Source/D2Common/Txt/Skills.h"
#include "Source/D2Common/Unit.h"
#include "Source/D2Hell/D2Hell.h"
#include "Source/D2Lang/D2Lang.h"
#include "Source/D2Lang/StrTable/strtable.h"
#include "Source/D2Multi/OOGUtilities.h"
#include "Source/Fog/D2MemoryManagerStrc.h"
#include "Source/Fog/D2PoolManagerStrc.h"
#include "Source/Fog/D2QServerStrc.h"
#include "Source/Fog/Fog.globals.h"
#include "Source/Fog/Memory.h"
#include "Source/Fog/Save.h"
#include "Source/Fog/Source/SFile.h"
#include "Source/Fog/Source/SSComp.h"
#include "Source/Fog/Src/LogManager.h"
#include "3rdparty/Storm/3rdparty/zlib/deflate.h"
#include "3rdparty/Storm/H/stpl.h"
#include "3rdparty/Storm/Source/SCMD.h"
#include "3rdparty/Storm/Source/SComp.h"
#include "3rdparty/Storm/Source/SErr.h"
#include "3rdparty/Storm/Source/SMem.h"
#include "3rdparty/Storm/Storm.h"
#include "Source/D2Headers/d2_platform.h"
#include "Source/D2Client/UI/Automap.h"
#include "Source/D2Common/Chat/Ignorelist.h"
#include "Source/D2Common/Chat/Ignorelist/ChatIgnoreList.h"
#include "Source/D2Common/Chat/Ignorelist/IgnoreList.h"
#include "Source/D2Common/DATATBLS/ItemTbls.h"
#include "Source/D2Common/DATATBLS/MonsterTbls.h"
#include "Source/D2Common/Drlg/Drlg.h"
#include "Source/D2Common/Drlg/DrlgGrid.h"
#include "Source/D2Common/Logging/ProfCore.h"
#include "Source/D2Common/Txt/Skills.h"


// =============================================================================
// Initialized scalars
// =============================================================================


namespace D2Common::Drlg::Maze {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

uint8_t gaMazeLevelTypeCaseTable[40] = {
    0xf4,
    0x39,
    0x67,
    0x0,
    0x0,
    0x1,
    0xf,
    0xf,
    0x2,
    0x3,
    0xf,
    0x4,
    0xf,
    0xf,
    0x5,
    0xf,
    0xf,
    0xf,
    0x6,
    0x7,
    0xf,
    0xf,
    0xf,
    0x8,
    0xf,
    0x9,
    0xa,
    0xf,
    0xf,
    0xb,
    0xf,
    0xf,
    0xf,
    0xf,
    0xc,
    0xd,
    0xe,
    0xcc,
    0xcc,
    0xcc
}; D2_GLOBAL_GUARD(gaMazeLevelTypeCaseTable, "D2Common::Drlg::Maze::gaMazeLevelTypeCaseTable");

} // namespace D2Common::Drlg::Maze

namespace Fog::Src::LogManager {

} // namespace Fog::Src::LogManager

namespace Fog::Src::ErrorManager {

} // namespace Fog::Src::ErrorManager

namespace Storm::Source::SSignature {

} // namespace Storm::Source::SSignature

namespace Fog::Engine::Application {

} // namespace Fog::Engine::Application

namespace Fog::Source::SFile {

} // namespace Fog::Source::SFile

namespace Fog::Src::DataArrays {

} // namespace Fog::Src::DataArrays

namespace Storm::Source::SSTR {

} // namespace Storm::Source::SSTR

namespace Fog::Debug {

} // namespace Fog::Debug

namespace Storm::Source::SFile {

} // namespace Storm::Source::SFile

namespace Fog::BitBuffer {

} // namespace Fog::BitBuffer

namespace Storm::Source::SErr {

} // namespace Storm::Source::SErr

namespace Storm::Source::SLog {

} // namespace Storm::Source::SLog

namespace Storm::_3rdparty::zlib::deflate {

} // namespace Storm::_3rdparty::zlib::deflate

namespace D2Launch::CharSel {

} // namespace D2Launch::CharSel

namespace D2Lang::StrTable::strtable {

} // namespace D2Lang::StrTable::strtable

namespace D2Multi::ChatDlg {

} // namespace D2Multi::ChatDlg

namespace D2Common::DATATBLS::LvlTbls {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char * gaLanguageNames_00744508[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_00744508, "D2Common::DATATBLS::LvlTbls::gaLanguageNames_00744508");

// =============================================================================
// Initialized scalars
// =============================================================================

uint8_t gbExpansionInitialized = 0x30; D2_GLOBAL_GUARD(gbExpansionInitialized, "D2Common::DATATBLS::LvlTbls::gbExpansionInitialized");

} // namespace D2Common::DATATBLS::LvlTbls

namespace D2Client::ClientModeInGame {

} // namespace D2Client::ClientModeInGame

namespace D2Client::UI::Draw {

} // namespace D2Client::UI::Draw

namespace D2Client::UI::Inv {

} // namespace D2Client::UI::Inv

namespace D2Common::DATATBLS::DataTbls {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char gaFreeUnitsOrderData[3] = { 'w', 'b', '\x00' }; D2_GLOBAL_GUARD(gaFreeUnitsOrderData, "D2Common::DATATBLS::DataTbls::gaFreeUnitsOrderData");

} // namespace D2Common::DATATBLS::DataTbls

namespace D2Client::Warden::NWardenClient {

} // namespace D2Client::Warden::NWardenClient

namespace D2Client::Warden {

} // namespace D2Client::Warden

namespace D2Client::Sound::SoundHdr {

} // namespace D2Client::Sound::SoundHdr

namespace D2Client::UI::ui {

} // namespace D2Client::UI::ui

namespace D2Client::UI::npcmenu {

} // namespace D2Client::UI::npcmenu

namespace D2Client::UI::QuestLog {

} // namespace D2Client::UI::QuestLog

namespace D2Sound::Src::D2SoundSmp {

} // namespace D2Sound::Src::D2SoundSmp

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

namespace D2gfx::D2GFX {

} // namespace D2gfx::D2GFX

namespace D2Sound::Sound {

} // namespace D2Sound::Sound

namespace Bnclient::BnSend {

} // namespace Bnclient::BnSend

namespace Bnclient::BnDownload {

} // namespace Bnclient::BnDownload

namespace Bnclient::_net_sid {

} // namespace Bnclient::_net_sid

namespace D2Game::GAME::Event::Dispatch {

} // namespace D2Game::GAME::Event::Dispatch

namespace D2Game::UNIT::SUnitDmg {

} // namespace D2Game::UNIT::SUnitDmg

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
    (void*)&D2Common::Chat::Ignorelist::ChatIgnoreList::ScalarDeletingDestructor
}; D2_GLOBAL_GUARD(vftable, "D2Common::Chat::Ignorelist::ChatIgnoreList::vftable");

// =============================================================================
// Initialized scalars
// =============================================================================

void* vftable_meta_ptr = (void*)0x0070056c; D2_GLOBAL_GUARD(vftable_meta_ptr, "D2Common::Chat::Ignorelist::ChatIgnoreList::vftable_meta_ptr");

} // namespace D2Common::Chat::Ignorelist::ChatIgnoreList

namespace D2Common::Chat::Ignorelist::IgnoreList {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

void * vftable[12] = {
    (void*)&D2Common::Chat::Ignorelist::IgnoreList::AddByPattern,
    (void*)&D2Common::Chat::Ignorelist::IgnoreList::RemoveByPattern,
    (void*)&D2Common::Chat::Ignorelist::IgnoreList::CheckIgnored,
    (void*)&D2Common::Chat::Ignorelist::IgnoreList::AddByAccountName,
    (void*)&D2Common::Chat::Ignorelist::IgnoreList::RemoveByAccountName,
    (void*)&D2Common::Chat::Ignorelist::IgnoreList::CheckAccountNameIgnored,
    (void*)&D2Common::Chat::Ignorelist::AddAccountMapping,
    (void*)&D2Common::Chat::Ignorelist::IgnoreList::GetMappedValue,
    (void*)&D2Common::Chat::Ignorelist::IgnoreList::GetPersistentSetting,
    (void*)&D2Common::Chat::Ignorelist::IgnoreList::SavePersistentSetting,
    (void*)&D2Common::Chat::Ignorelist::IgnoreList::ClearAccountMappings,
    (void*)&D2Common::Chat::Ignorelist::IgnoreList::ScalarDeletingDestructor
}; D2_GLOBAL_GUARD(vftable, "D2Common::Chat::Ignorelist::IgnoreList::vftable");

// =============================================================================
// Initialized scalars
// =============================================================================

void* vftable_meta_ptr = (void*)0x00700644; D2_GLOBAL_GUARD(vftable_meta_ptr, "D2Common::Chat::Ignorelist::IgnoreList::vftable_meta_ptr");

} // namespace D2Common::Chat::Ignorelist::IgnoreList

namespace D2Common {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char gsGameMonsterSpawnContext[4] = { 'm', 'a', 'x', '\x00' }; D2_GLOBAL_GUARD(gsGameMonsterSpawnContext, "D2Common::gsGameMonsterSpawnContext");

char gsGameMissileTblsContext[4] = { 'm', 'i', 'n', '\x00' }; D2_GLOBAL_GUARD(gsGameMissileTblsContext, "D2Common::gsGameMissileTblsContext");

char s___AV__TSExplicitList_UStringEntr_0074366c[56] = {
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
    't',
    'r',
    'i',
    'n',
    'g',
    'E',
    'n',
    't',
    'r',
    'y',
    'N',
    'o',
    'd',
    'e',
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
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(s___AV__TSExplicitList_UStringEntr_0074366c, "D2Common::s___AV__TSExplicitList_UStringEntr_0074366c");

int gpsPresetObjectTable[750] = {
    0xc,
    0x25,
    0x27,
    0x23,
    0x24,
    0x5,
    0x11,
    0x12,
    0x13,
    0x14,
    0x15,
    0x16,
    0x1e,
    0x46,
    0x46,
    0x45,
    0x45,
    0x1d,
    0x1f,
    0x21,
    0x22,
    0x25,
    0x3d,
    0x41,
    0x42,
    0x8,
    0x1a,
    0x1c,
    0x52,
    0x2,
    0x51,
    0x54,
    0x53,
    0x4e,
    0x3d,
    0x67,
    0x6c,
    0x77,
    0x244,
    0x82,
    0x9f,
    0xa3,
    0xa9,
    0xa0,
    0xa1,
    0xa2,
    0x68,
    0x69,
    0x6a,
    0x6b,
    0xb3,
    0xb4,
    0x77,
    0x9d,
    0xf7,
    0xf8,
    0x9b,
    0xae,
    0xaf,
    0x8b,
    0x8c,
    0x8d,
    0x90,
    0x6,
    0xf0,
    0xf1,
    0xf2,
    0x36,
    0x37,
    0x38,
    0x39,
    0x3a,
    0xab,
    0xb2,
    0xef,
    0xf5,
    0xfa,
    0x6f,
    0x8a,
    0x84,
    0xa4,
    0xa5,
    0x4d,
    0x55,
    0x56,
    0x106,
    0x107,
    0x108,
    0x109,
    0x32,
    0x33,
    0x4f,
    0x35,
    0x1,
    0x3,
    0x7,
    0x2e,
    0x26,
    0x100,
    0x101,
    0x102,
    0x81,
    0x10b,
    0x10c,
    0x10d,
    0x245,
    0x15f,
    0x160,
    0x161,
    0x176,
    0x181,
    0x18d,
    0x141,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x4a,
    0x25,
    0xc0,
    0x130,
    0x131,
    0x132,
    0x65,
    0x66,
    0x4e,
    0x67,
    0x9c,
    0x244,
    0x84,
    0x81,
    0x165,
    0x99,
    0x79,
    0x7a,
    0xe5,
    0xe6,
    0xc4,
    0x10b,
    0x105,
    0x95,
    0x10d,
    0x4,
    0x9,
    0x34,
    0x5e,
    0x5f,
    0x8e,
    0x8f,
    0x5,
    0x6,
    0x57,
    0x58,
    0x92,
    0x92,
    0x93,
    0x94,
    0xf0,
    0xf1,
    0xf2,
    0xf3,
    0xb0,
    0xb1,
    0xc6,
    0xf6,
    0x1d,
    0xa0,
    0xa1,
    0xa2,
    0x111,
    0x11b,
    0x55,
    0x56,
    0x6d,
    0x74,
    0x86,
    0x87,
    0x88,
    0x96,
    0x97,
    0xac,
    0xad,
    0x117,
    0x118,
    0x119,
    0x11a,
    0xa6,
    0xa7,
    0x71,
    0x89,
    0x59,
    0x68,
    0x69,
    0x6a,
    0x6b,
    0x9a,
    0xab,
    0xb2,
    0x10e,
    0x10f,
    0x110,
    0x10a,
    0x112,
    0xf4,
    0x11c,
    0x120,
    0x12a,
    0x121,
    0x128,
    0x129,
    0x11f,
    0x11e,
    0x11d,
    0x122,
    0x123,
    0x124,
    0x125,
    0x126,
    0x127,
    0x85,
    0x12f,
    0x12b,
    0x12c,
    0x12d,
    0x12e,
    0x245,
    0x162,
    0x246,
    0x13a,
    0x13b,
    0x13c,
    0x13d,
    0x143,
    0x142,
    0x6e,
    0x70,
    0x72,
    0x163,
    0x164,
    0x165,
    0x15f,
    0x160,
    0x161,
    0x98,
    0x176,
    0x183,
    0x185,
    0x186,
    0x187,
    0x184,
    0x18d,
    0x192,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x75,
    0xed,
    0x244,
    0x82,
    0x66,
    0x25,
    0xa0,
    0xa1,
    0xa2,
    0x68,
    0x69,
    0x6a,
    0x6b,
    0xc2,
    0xc3,
    0xc1,
    0xcf,
    0xd3,
    0xd2,
    0xea,
    0xd6,
    0xd7,
    0xd5,
    0xe4,
    0xd8,
    0xe3,
    0xd9,
    0xeb,
    0xda,
    0xdb,
    0xdc,
    0xdd,
    0xdf,
    0xe0,
    0x10b,
    0x10d,
    0x245,
    0xaa,
    0x145,
    0xb8,
    0xbe,
    0xbf,
    0xc5,
    0xc7,
    0xc8,
    0xc9,
    0xca,
    0xce,
    0x116,
    0x78,
    0x82,
    0x146,
    0x9e,
    0x10f,
    0x110,
    0x147,
    0x148,
    0x149,
    0x14a,
    0x14b,
    0x14c,
    0x14d,
    0x14e,
    0x14f,
    0x150,
    0x5,
    0x6,
    0xb0,
    0xf0,
    0xf1,
    0xb5,
    0xb7,
    0xf6,
    0xb9,
    0xba,
    0xbb,
    0xbc,
    0xcb,
    0xcc,
    0xcd,
    0xd0,
    0xd1,
    0xa9,
    0x143,
    0x144,
    0xc4,
    0xd4,
    0xe1,
    0xf4,
    0x15f,
    0x160,
    0x161,
    0x168,
    0x169,
    0x16a,
    0x16d,
    0xfb,
    0xfc,
    0xd0,
    0x11b,
    0x16f,
    0x16e,
    0x170,
    0x155,
    0x156,
    0x157,
    0x158,
    0x176,
    0x172,
    0x17a,
    0x17b,
    0x182,
    0x18d,
    0x195,
    0x197,
    0x196,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0xee,
    0x244,
    0x10b,
    0x10d,
    0x245,
    0x23d,
    0x23d,
    0x23d,
    0x159,
    0x15a,
    0x15b,
    0x15c,
    0x15d,
    0x15e,
    0x15f,
    0x160,
    0x161,
    0x166,
    0x167,
    0x16b,
    0x103,
    0x175,
    0x174,
    0x176,
    0xec,
    0xf9,
    0xe2,
    0xe7,
    0xe8,
    0x5d,
    0x61,
    0x7b,
    0x7c,
    0x60,
    0xe1,
    0xe9,
    0xde,
    0x7d,
    0x7e,
    0x7f,
    0x80,
    0x177,
    0x178,
    0xfe,
    0xfd,
    0x156,
    0xff,
    0x188,
    0x189,
    0x18a,
    0x18b,
    0x18c,
    0x18e,
    0x18d,
    0x18f,
    0x191,
    0x190,
    0x17c,
    0x17f,
    0x180,
    0x128,
    0x129,
    0x193,
    0x66,
    0x198,
    0x199,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x1c4,
    0x1c5,
    0x152,
    0x151,
    0x10b,
    0x176,
    0x1e2,
    0x27,
    0x23,
    0x24,
    0x21,
    0x22,
    0x26,
    0x66,
    0x19b,
    0x1b6,
    0x19c,
    0x1b3,
    0x1b4,
    0x1b8,
    0x1b9,
    0x1b9,
    0x1ba,
    0x1ad,
    0x1a4,
    0x1af,
    0x1ae,
    0x19d,
    0x1b0,
    0x1b1,
    0x1a2,
    0x1a3,
    0x1a8,
    0x1a9,
    0x1a0,
    0x19e,
    0x19f,
    0x1ab,
    0x1ac,
    0x1a5,
    0x1a6,
    0x1a7,
    0x1aa,
    0x1c3,
    0x10b,
    0x1bb,
    0x1bc,
    0x1bd,
    0x1be,
    0x1bf,
    0x1c0,
    0x1c2,
    0x1c3,
    0x1cb,
    0x1cc,
    0x1cd,
    0x1ce,
    0x1e2,
    0x1d9,
    0x1c7,
    0x1c8,
    0x1c9,
    0x1ca,
    0x1cf,
    0x1d0,
    0x1d1,
    0x1d2,
    0x1d3,
    0x1d4,
    0x1d5,
    0x1d6,
    0x1d7,
    0x1d8,
    0x1dd,
    0x1df,
    0x1e0,
    0x1e1,
    0x1e3,
    0x1e4,
    0x1e5,
    0x1e6,
    0x1e7,
    0x1c6,
    0x1b5,
    0x1fc,
    0x1e8,
    0x1ed,
    0x1ed,
    0x1ef,
    0x1f1,
    0x1f3,
    0x1f7,
    0x1fd,
    0x200,
    0x1e9,
    0x1ea,
    0x202,
    0x203,
    0x1ed,
    0x1f2,
    0x201,
    0x1ee,
    0x1f0,
    0x1ff,
    0x1f4,
    0x1f5,
    0x1f6,
    0x1f8,
    0x1f9,
    0x1fa,
    0x1fb,
    0x1fe,
    0xa0,
    0xa1,
    0xa2,
    0x10d,
    0x20b,
    0x1b2,
    0x1f0,
    0x1f0,
    0x1f0,
    0x20f,
    0x210,
    0x21a,
    0x21b,
    0x21e,
    0x21f,
    0x222,
    0x223,
    0x224,
    0x225,
    0x226,
    0x227,
    0x228,
    0x22b,
    0x229,
    0x22a,
    0x22d,
    0x21d,
    0x220,
    0x22f,
    0x230,
    0x234,
    0x237,
    0x238,
    0x218,
    0x219,
    0x233,
    0x23a,
    0x18d
}; D2_GLOBAL_GUARD(gpsPresetObjectTable, "D2Common::gpsPresetObjectTable");

} // namespace D2Common

namespace D2Common::Txt::Cube {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char gsGameDataTblsContext[4] = { 'm', 'o', 'd', '\x00' }; D2_GLOBAL_GUARD(gsGameDataTblsContext, "D2Common::Txt::Cube::gsGameDataTblsContext");

} // namespace D2Common::Txt::Cube

namespace D2Common::DATATBLS::ItemTbls {

// =============================================================================
// Initialized scalars
// =============================================================================

unsigned int gszTxtColName_op = 0x706f; D2_GLOBAL_GUARD(gszTxtColName_op, "D2Common::DATATBLS::ItemTbls::gszTxtColName_op");

} // namespace D2Common::DATATBLS::ItemTbls

namespace D2Common::Drlg::Preset {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char s_HDM_006ee7c0[4] = { 'H', 'D', 'M', '\x00' }; D2_GLOBAL_GUARD(s_HDM_006ee7c0, "D2Common::Drlg::Preset::s_HDM_006ee7c0");

} // namespace D2Common::Drlg::Preset

namespace D2Common::Drlg::OutSiege {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

int gnAct5OuterWallGridOffsetByType[24] = {
    -0x1,
    0x0,
    0x0,
    -0x1,
    0x1,
    0x0,
    0x0,
    0x1,
    0x0,
    -0x1,
    0x1,
    0x0,
    0x0,
    0x1,
    -0x1,
    0x0,
    -0x1,
    0x0,
    0x0,
    -0x1,
    0x1,
    0x0,
    0x0,
    0x1
}; D2_GLOBAL_GUARD(gnAct5OuterWallGridOffsetByType, "D2Common::Drlg::OutSiege::gnAct5OuterWallGridOffsetByType");

int gaAct5SceneryPresetTable_0[15][7] = {
    { 0x6f, 0x3bb, 0x3bc, 0x0, 0x19, 0x1, 0x1 },
    { 0x70, 0x3bb, 0x3bc, 0x0, 0x19, 0x1, 0x1 },
    { 0x75, 0x3bb, 0x3bc, 0x1, 0x19, 0x1, 0x1 },
    { 0x70, 0x3b9, 0x3b9, -0x1, 0x19, 0x1, 0x1 },
    { 0x75, 0x3ba, 0x3ba, -0x1, 0x19, 0x1, 0x1 },
    { 0x6f, 0x3b0, 0x3b3, -0x1, 0xa, 0x1, 0x0 },
    { 0x6f, 0x3ae, 0x3b1, -0x1, 0x14, 0x4, 0x0 },
    { 0x6f, 0x3af, 0x3b2, -0x1, 0x14, 0x4, 0x0 },
    { 0x70, 0x3ad, 0x3ad, -0x1, 0xa, 0x1, 0x0 },
    { 0x70, 0x3ab, 0x3ab, -0x1, 0x8, 0x1, 0x0 },
    { 0x70, 0x3ac, 0x3ac, -0x1, 0xf, 0x5, 0x0 },
    { 0x75, 0x3b4, 0x3b4, -0x1, 0x9, 0x4, 0x0 },
    { 0x75, 0x3b5, 0x3b5, -0x1, 0x9, 0x4, 0x0 },
    { 0x75, 0x3b6, 0x3b6, -0x1, 0x9, 0x4, 0x0 },
    { 0x75, 0x3b7, 0x3b7, -0x1, 0x5, 0x3, 0x0 }
}; D2_GLOBAL_GUARD(gaAct5SceneryPresetTable_0, "D2Common::Drlg::OutSiege::gaAct5SceneryPresetTable_0");

// =============================================================================
// Initialized scalars
// =============================================================================

int gaAct5SceneryPresetTable_0_0__1_[15][7] = {}; D2_GLOBAL_GUARD(gaAct5SceneryPresetTable_0_0__1_, "D2Common::Drlg::OutSiege::gaAct5SceneryPresetTable_0_0__1_");

} // namespace D2Common::Drlg::OutSiege

namespace D2Common::Drlg::Outdoors {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

int aAct4OutdoorSceneryBaseByLevel[3] = { 0x33c, 0x340, 0x340 }; D2_GLOBAL_GUARD(aAct4OutdoorSceneryBaseByLevel, "D2Common::Drlg::Outdoors::aAct4OutdoorSceneryBaseByLevel");

int aAct4OutdoorPresetBaseByLevel[3] = { 0x32c, 0x331, 0x337 }; D2_GLOBAL_GUARD(aAct4OutdoorPresetBaseByLevel, "D2Common::Drlg::Outdoors::aAct4OutdoorPresetBaseByLevel");

} // namespace D2Common::Drlg::Outdoors

namespace D2Common::Drlg::OutJung {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

int gaOutJungPresetOffsetByLevel[3] = { 0x0, 0xa, 0x14 }; D2_GLOBAL_GUARD(gaOutJungPresetOffsetByLevel, "D2Common::Drlg::OutJung::gaOutJungPresetOffsetByLevel");

} // namespace D2Common::Drlg::OutJung

namespace D2Common::Drlg {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

uint8_t gaWallNeighborOrientTable[256] = {
    0x0,
    0x0,
    0x10,
    0x10,
    0x0,
    0x0,
    0x10,
    0x10,
    0xe,
    0xe,
    0x6,
    0x13,
    0xe,
    0xe,
    0x6,
    0x13,
    0xf,
    0xf,
    0x5,
    0x5,
    0xf,
    0xf,
    0x15,
    0x15,
    0x8,
    0x8,
    0xa,
    0x26,
    0x8,
    0x8,
    0x28,
    0x14,
    0x0,
    0x0,
    0x10,
    0x10,
    0x0,
    0x0,
    0x10,
    0x10,
    0xe,
    0xe,
    0x6,
    0x13,
    0xe,
    0xe,
    0x6,
    0x13,
    0xf,
    0xf,
    0x5,
    0x5,
    0xf,
    0xf,
    0x15,
    0x15,
    0x8,
    0x8,
    0xa,
    0x26,
    0x8,
    0x8,
    0x28,
    0x14,
    0xd,
    0xd,
    0x7,
    0x7,
    0xd,
    0xd,
    0xd,
    0x7,
    0x4,
    0x4,
    0xb,
    0x25,
    0x4,
    0x4,
    0xb,
    0x2b,
    0x3,
    0x3,
    0xc,
    0xc,
    0x3,
    0x3,
    0x27,
    0x27,
    0x9,
    0x9,
    0x2,
    0x2b,
    0x9,
    0x9,
    0x2c,
    0x1a,
    0xd,
    0xd,
    0x7,
    0x7,
    0xd,
    0xd,
    0xd,
    0x7,
    0x17,
    0x17,
    0x29,
    0x11,
    0x17,
    0x17,
    0x29,
    0x11,
    0x3,
    0x3,
    0xc,
    0xc,
    0x3,
    0x3,
    0x27,
    0x27,
    0x2a,
    0x2a,
    0x2e,
    0x2a,
    0x2a,
    0x2a,
    0x21,
    0x1f,
    0x0,
    0x0,
    0x10,
    0x10,
    0x0,
    0x0,
    0x10,
    0x10,
    0xe,
    0xe,
    0x6,
    0x13,
    0xe,
    0xe,
    0x6,
    0x13,
    0xf,
    0xf,
    0x5,
    0x5,
    0xf,
    0xf,
    0x15,
    0x15,
    0x8,
    0x8,
    0xa,
    0x26,
    0x8,
    0x8,
    0x23,
    0x14,
    0x0,
    0x0,
    0x10,
    0x10,
    0x0,
    0x0,
    0x10,
    0x10,
    0xe,
    0xe,
    0x6,
    0x13,
    0xe,
    0xe,
    0x6,
    0x13,
    0xf,
    0xf,
    0x5,
    0x5,
    0xf,
    0xf,
    0x15,
    0x15,
    0x8,
    0x8,
    0xa,
    0x26,
    0x8,
    0x8,
    0x28,
    0x14,
    0xd,
    0xd,
    0x7,
    0x7,
    0xd,
    0xd,
    0xd,
    0x7,
    0x4,
    0x4,
    0xb,
    0x25,
    0x4,
    0x4,
    0xb,
    0x25,
    0x12,
    0x12,
    0x23,
    0x23,
    0x12,
    0x12,
    0x16,
    0x16,
    0x24,
    0x24,
    0x2d,
    0x22,
    0x24,
    0x24,
    0x1c,
    0x1d,
    0xd,
    0xd,
    0x7,
    0x7,
    0xd,
    0xd,
    0xd,
    0x7,
    0x17,
    0x17,
    0x29,
    0x11,
    0x17,
    0x17,
    0x29,
    0x11,
    0x12,
    0x12,
    0x23,
    0x23,
    0x12,
    0x12,
    0x16,
    0x16,
    0x18,
    0x18,
    0x19,
    0x20,
    0x18,
    0x18,
    0x1e,
    0x1
}; D2_GLOBAL_GUARD(gaWallNeighborOrientTable, "D2Common::Drlg::gaWallNeighborOrientTable");

// =============================================================================
// Initialized scalars
// =============================================================================

unsigned int gdwDrlgLevelPlacementFlags = {}; D2_GLOBAL_GUARD(gdwDrlgLevelPlacementFlags, "D2Common::Drlg::gdwDrlgLevelPlacementFlags");

} // namespace D2Common::Drlg

namespace Define_the_symbol__ATL_MIXED {

} // namespace Define_the_symbol__ATL_MIXED

namespace Storm::Source {

} // namespace Storm::Source

namespace Fog::File {

} // namespace Fog::File

namespace Fog::_3rdparty::crashy {

} // namespace Fog::_3rdparty::crashy

namespace D2Hell {

} // namespace D2Hell

namespace Fog::Source::SSComp {

} // namespace Fog::Source::SSComp

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

namespace D2Launch::MainMenus {

} // namespace D2Launch::MainMenus

namespace D2Multi::OOGUtilities {

} // namespace D2Multi::OOGUtilities

namespace D2Client::GAME::Game {

} // namespace D2Client::GAME::Game

namespace D2Client {

} // namespace D2Client

namespace D2Client::GAME::Record {

} // namespace D2Client::GAME::Record

namespace D2Client::_Message {

} // namespace D2Client::_Message

namespace D2Client::Engine {

} // namespace D2Client::Engine

namespace D2Client::GAME::SCmd {

} // namespace D2Client::GAME::SCmd

namespace D2Client::Draw::LightMap {

} // namespace D2Client::Draw::LightMap

namespace D2Client::GAME::Rendering {

} // namespace D2Client::GAME::Rendering

namespace D2Client::UI::Panel {

} // namespace D2Client::UI::Panel

namespace D2Client::UI::Party {

} // namespace D2Client::UI::Party

namespace D2Client::Sound {

} // namespace D2Client::Sound

namespace D2Client::UI::UseItem {

} // namespace D2Client::UI::UseItem

namespace D2Client::UNIT::UnitSnd {

} // namespace D2Client::UNIT::UnitSnd

namespace D2Client::UI::SkillDesc {

} // namespace D2Client::UI::SkillDesc

namespace D2Glide::Renderer::Glide {

} // namespace D2Glide::Renderer::Glide

namespace D2DDraw::Renderer::DirectDraw {

} // namespace D2DDraw::Renderer::DirectDraw

namespace D2Sound::Src::D2SoundFast {

} // namespace D2Sound::Src::D2SoundFast

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

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char s___AUMappingEntryNode___0074362c[24] = {
    '.',
    '?',
    'A',
    'U',
    'M',
    'a',
    'p',
    'p',
    'i',
    'n',
    'g',
    'E',
    'n',
    't',
    'r',
    'y',
    'N',
    'o',
    'd',
    'e',
    '@',
    '@',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(s___AUMappingEntryNode___0074362c, "D2Common::Chat::Ignorelist::MappingEntryNode::s___AUMappingEntryNode___0074362c");

} // namespace D2Common::Chat::Ignorelist::MappingEntryNode

namespace D2Common::Chat::Ignorelist::StringEntryNode {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char s___AUStringEntryNode___0074364c[24] = {
    '.',
    '?',
    'A',
    'U',
    'S',
    't',
    'r',
    'i',
    'n',
    'g',
    'E',
    'n',
    't',
    'r',
    'y',
    'N',
    'o',
    'd',
    'e',
    '@',
    '@',
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(s___AUStringEntryNode___0074364c, "D2Common::Chat::Ignorelist::StringEntryNode::s___AUStringEntryNode___0074364c");

} // namespace D2Common::Chat::Ignorelist::StringEntryNode

namespace D2Common::Chat::Ignorelist {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char s___AV__TSExplicitList_UMappingEnt_007436ac[56] = {
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
    'M',
    'a',
    'p',
    'p',
    'i',
    'n',
    'g',
    'E',
    'n',
    't',
    'r',
    'y',
    'N',
    'o',
    'd',
    'e',
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
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(s___AV__TSExplicitList_UMappingEnt_007436ac, "D2Common::Chat::Ignorelist::s___AV__TSExplicitList_UMappingEnt_007436ac");

} // namespace D2Common::Chat::Ignorelist

namespace D2Common::Logging::ProfCore {

// =============================================================================
// Initialized scalars
// =============================================================================

char * gaLanguageNames_0074430c_11[15] = {}; D2_GLOBAL_GUARD(gaLanguageNames_0074430c_11, "D2Common::Logging::ProfCore::gaLanguageNames_0074430c_11");

} // namespace D2Common::Logging::ProfCore

namespace D2Common::DUNGEON::Dungeon {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char * gaLanguageNames_007443a8[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_007443a8, "D2Common::DUNGEON::Dungeon::gaLanguageNames_007443a8");

} // namespace D2Common::DUNGEON::Dungeon

namespace D2Common::Environment {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

uint32_t gnTimeRates[3] = { 0x80, 0x4, 0x8 }; D2_GLOBAL_GUARD(gnTimeRates, "D2Common::Environment::gnTimeRates");

char * gaLanguageNames_007444c8[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_007444c8, "D2Common::Environment::gaLanguageNames_007444c8");

} // namespace D2Common::Environment

namespace D2Common::Units {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char * gaLanguageNames_00744588[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_00744588, "D2Common::Units::gaLanguageNames_00744588");

char * gaLanguageNames_007445c4[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_007445c4, "D2Common::Units::gaLanguageNames_007445c4");

char * gaLanguageNames_00744600[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_00744600, "D2Common::Units::gaLanguageNames_00744600");

} // namespace D2Common::Units

namespace D2Common::Monsters::MONSTERS {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char * gaLanguageNames_00745648[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_00745648, "D2Common::Monsters::MONSTERS::gaLanguageNames_00745648");

char * gaLanguageNames_00745684[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_00745684, "D2Common::Monsters::MONSTERS::gaLanguageNames_00745684");

char * gaLanguageNames_007456c0[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_007456c0, "D2Common::Monsters::MONSTERS::gaLanguageNames_007456c0");

char * gaLanguageNames_007456fc[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_007456fc, "D2Common::Monsters::MONSTERS::gaLanguageNames_007456fc");

char * gaLanguageNames_00745738[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_00745738, "D2Common::Monsters::MONSTERS::gaLanguageNames_00745738");

} // namespace D2Common::Monsters::MONSTERS

namespace D2Common::Merc {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_007468a0[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::gaMonSeqMonsterTblData_Unknown,
        /* .nCounter1 = */ 0xd,
        /* .nCounter2 = */ 0xd
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::DWORD_007467b0,
        /* .nCounter1 = */ 0x12,
        /* .nCounter2 = */ 0x12
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746820,
        /* .nCounter1 = */ 0x15,
        /* .nCounter2 = */ 0x15
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_007468a0, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_007468a0");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00746978[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::gaMonSeqAnimData,
        /* .nCounter1 = */ 0xe,
        /* .nCounter2 = */ 0xe
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007466dc,
        /* .nCounter1 = */ 0x7,
        /* .nCounter2 = */ 0x7
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746948,
        /* .nCounter1 = */ 0x8,
        /* .nCounter2 = */ 0x8
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_00746978, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_00746978");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00746a80[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::gaMonSeqAnimData,
        /* .nCounter1 = */ 0xe,
        /* .nCounter2 = */ 0xe
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746a20,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_00746a80, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_00746a80");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00746b88[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746b28,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746b28,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746b28,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746b28,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746b28,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746b28,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_00746b88, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_00746b88");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00746c58[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::gaMonSeqAnimData,
        /* .nCounter1 = */ 0xe,
        /* .nCounter2 = */ 0xe
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746c30,
        /* .nCounter1 = */ 0x6,
        /* .nCounter2 = */ 0x6
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_00746c58, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_00746c58");

D2MonSeqMonsterTbls gaMonSeqMonsterTblData[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746d00,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746d00,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746d00,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746d00,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746d00,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746d00,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746d00,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746d00,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746d00,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746d00,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746d00,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746d00,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746d00,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746d00,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    }
}; D2_GLOBAL_GUARD(gaMonSeqMonsterTblData, "D2Common::Merc::gaMonSeqMonsterTblData");

D2MonSeqMonsterTbls gaMonSeqMonsterTblsExtra[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::gaMonSeqMonsterTblData_Unknown,
        /* .nCounter1 = */ 0xd,
        /* .nCounter2 = */ 0xd
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746e08,
        /* .nCounter1 = */ 0xd,
        /* .nCounter2 = */ 0xd
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746e08[13].mode,
        /* .nCounter1 = */ 0x14,
        /* .nCounter2 = */ 0x14
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    }
}; D2_GLOBAL_GUARD(gaMonSeqMonsterTblsExtra, "D2Common::Merc::gaMonSeqMonsterTblsExtra");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747088[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::gaMonSeqMonsterTblData_Unknown,
        /* .nCounter1 = */ 0xd,
        /* .nCounter2 = */ 0xd
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746f78,
        /* .nCounter1 = */ 0x15,
        /* .nCounter2 = */ 0x15
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00746f78[21].mode,
        /* .nCounter1 = */ 0x18,
        /* .nCounter2 = */ 0x18
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_00747088, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_00747088");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_007471f0[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::gaMonSeqMonsterTblData_Unknown,
        /* .nCounter1 = */ 0xd,
        /* .nCounter2 = */ 0xd
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747130,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747130[16].sequence,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_007471f0, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_007471f0");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_007472c8[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747298,
        /* .nCounter1 = */ 0x8,
        /* .nCounter2 = */ 0x8
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747298,
        /* .nCounter1 = */ 0x8,
        /* .nCounter2 = */ 0x8
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747298,
        /* .nCounter1 = */ 0x8,
        /* .nCounter2 = */ 0x8
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747298,
        /* .nCounter1 = */ 0x8,
        /* .nCounter2 = */ 0x8
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747298,
        /* .nCounter1 = */ 0x8,
        /* .nCounter2 = */ 0x8
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747298,
        /* .nCounter1 = */ 0x8,
        /* .nCounter2 = */ 0x8
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747298,
        /* .nCounter1 = */ 0x8,
        /* .nCounter2 = */ 0x8
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747298,
        /* .nCounter1 = */ 0x8,
        /* .nCounter2 = */ 0x8
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747298,
        /* .nCounter1 = */ 0x8,
        /* .nCounter2 = */ 0x8
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747298,
        /* .nCounter1 = */ 0x8,
        /* .nCounter2 = */ 0x8
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747298,
        /* .nCounter1 = */ 0x8,
        /* .nCounter2 = */ 0x8
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747298,
        /* .nCounter1 = */ 0x8,
        /* .nCounter2 = */ 0x8
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_007472c8, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_007472c8");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_007473d8[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747370,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747370,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747370,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747370,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747370,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747370,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747370,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747370,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747370,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747370,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747370,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747370,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747370,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747370,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_007473d8, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_007473d8");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_007474f8[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747480,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747480,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747480,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747480,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747480,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747480,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747480,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747480,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747480,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747480,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747480,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747480,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747480,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747480,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_007474f8, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_007474f8");

D2MonSeqMonsterTbls gaMonSeqMonsterTbls[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007475a0,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007475a0,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007475a0,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007475a0,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007475a0,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007475a0,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007475a0,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007475a0,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007475a0,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007475a0,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007475a0,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007475a0,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007475a0,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007475a0,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    }
}; D2_GLOBAL_GUARD(gaMonSeqMonsterTbls, "D2Common::Merc::gaMonSeqMonsterTbls");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747918[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007476a8,
        /* .nCounter1 = */ 0x16,
        /* .nCounter2 = */ 0x16
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007476a8[22].dir,
        /* .nCounter1 = */ 0x19,
        /* .nCounter2 = */ 0x19
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007476a8[48].sequence,
        /* .nCounter1 = */ 0x1c,
        /* .nCounter2 = */ 0x1c
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007476a8[22].dir,
        /* .nCounter1 = */ 0x19,
        /* .nCounter2 = */ 0x19
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007476a8[76].sequence,
        /* .nCounter1 = */ 0x1b,
        /* .nCounter2 = */ 0x1b
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007476a8[48].sequence,
        /* .nCounter1 = */ 0x1c,
        /* .nCounter2 = */ 0x1c
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007476a8,
        /* .nCounter1 = */ 0x16,
        /* .nCounter2 = */ 0x16
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007476a8,
        /* .nCounter1 = */ 0x16,
        /* .nCounter2 = */ 0x16
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007476a8,
        /* .nCounter1 = */ 0x16,
        /* .nCounter2 = */ 0x16
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007476a8,
        /* .nCounter1 = */ 0x16,
        /* .nCounter2 = */ 0x16
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007476a8,
        /* .nCounter1 = */ 0x16,
        /* .nCounter2 = */ 0x16
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007476a8,
        /* .nCounter1 = */ 0x16,
        /* .nCounter2 = */ 0x16
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_00747918, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_00747918");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747a08[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007479c0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007479c0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007479c0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007479c0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007479c0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007479c0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007479c0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007479c0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007479c0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007479c0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007479c0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007479c0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007479c0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_007479c0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_00747a08, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_00747a08");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747b58[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747ab0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747ab0,
        /* .nCounter1 = */ 0xc,
        /* .nCounter2 = */ 0xc
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747ab0[12].sequence,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_00747b58, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_00747b58");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747c78[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747c00,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747c00,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747c00,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747c00,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747c00,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747c00,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747c00,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747c00,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747c00,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747c00,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_00747c78, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_00747c78");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747d98[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747d20,
        /* .nCounter1 = */ 0x14,
        /* .nCounter2 = */ 0x14
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747d20,
        /* .nCounter1 = */ 0x14,
        /* .nCounter2 = */ 0x14
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747d20,
        /* .nCounter1 = */ 0x14,
        /* .nCounter2 = */ 0x14
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747d20,
        /* .nCounter1 = */ 0x14,
        /* .nCounter2 = */ 0x14
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747d20,
        /* .nCounter1 = */ 0x14,
        /* .nCounter2 = */ 0x14
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747d20,
        /* .nCounter1 = */ 0x14,
        /* .nCounter2 = */ 0x14
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747d20,
        /* .nCounter1 = */ 0x14,
        /* .nCounter2 = */ 0x14
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747d20,
        /* .nCounter1 = */ 0x14,
        /* .nCounter2 = */ 0x14
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747d20,
        /* .nCounter1 = */ 0x14,
        /* .nCounter2 = */ 0x14
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747d20,
        /* .nCounter1 = */ 0x14,
        /* .nCounter2 = */ 0x14
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_00747d98, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_00747d98");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747ea0[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747e40,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747e40,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747e40,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747e40,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747e40,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747e40,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747e40,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747e40,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747e40,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747e40,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747e40,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747e40,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747e40,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747e40,
        /* .nCounter1 = */ 0xf,
        /* .nCounter2 = */ 0xf
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_00747ea0, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_00747ea0");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747fb0[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747f48,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747f48,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747f48,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747f48,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747f48,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747f48,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747f48,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ nullptr,
        /* .nCounter1 = */ 0x0,
        /* .nCounter2 = */ 0x0
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747f48,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00747f48,
        /* .nCounter1 = */ 0x11,
        /* .nCounter2 = */ 0x11
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_00747fb0, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_00747fb0");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_007480e8[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748058,
        /* .nCounter1 = */ 0x17,
        /* .nCounter2 = */ 0x17
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748058,
        /* .nCounter1 = */ 0x17,
        /* .nCounter2 = */ 0x17
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748058,
        /* .nCounter1 = */ 0x17,
        /* .nCounter2 = */ 0x17
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748058,
        /* .nCounter1 = */ 0x17,
        /* .nCounter2 = */ 0x17
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748058,
        /* .nCounter1 = */ 0x17,
        /* .nCounter2 = */ 0x17
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748058,
        /* .nCounter1 = */ 0x17,
        /* .nCounter2 = */ 0x17
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748058,
        /* .nCounter1 = */ 0x17,
        /* .nCounter2 = */ 0x17
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748058,
        /* .nCounter1 = */ 0x17,
        /* .nCounter2 = */ 0x17
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748058,
        /* .nCounter1 = */ 0x17,
        /* .nCounter2 = */ 0x17
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748058,
        /* .nCounter1 = */ 0x17,
        /* .nCounter2 = */ 0x17
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748058,
        /* .nCounter1 = */ 0x17,
        /* .nCounter2 = */ 0x17
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748058,
        /* .nCounter1 = */ 0x17,
        /* .nCounter2 = */ 0x17
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748058,
        /* .nCounter1 = */ 0x17,
        /* .nCounter2 = */ 0x17
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748058,
        /* .nCounter1 = */ 0x17,
        /* .nCounter2 = */ 0x17
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_007480e8, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_007480e8");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_007481f0[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748190,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748190,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748190,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748190,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748190,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748190,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748190,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748190,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748190,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748190,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748190,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748190,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748190,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748190,
        /* .nCounter1 = */ 0x10,
        /* .nCounter2 = */ 0x10
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_007481f0, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_007481f0");

D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00748310[14] = {
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748298,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748298,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748298,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748298,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748298,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748298,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748298,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748298,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748298,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748298,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748298,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748298,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748298,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    },
    {
        /* .pMonSeqTxt = */ (D2MonSeqTxt*)&D2Common::Merc::D2MonSeqTxt_ARRAY_00748298,
        /* .nCounter1 = */ 0x13,
        /* .nCounter2 = */ 0x13
    }
}; D2_GLOBAL_GUARD(D2MonSeqMonsterTbls_ARRAY_00748310, "D2Common::Merc::D2MonSeqMonsterTbls_ARRAY_00748310");

// =============================================================================
// Initialized scalars
// =============================================================================

uint32_t DWORD_007467b0 = 0x5070000; D2_GLOBAL_GUARD(DWORD_007467b0, "D2Common::Merc::DWORD_007467b0");

// =============================================================================
// Uninitialized data (BSS)
// =============================================================================

uint8_t gaMonSeqMonsterTblData_Unknown; D2_GLOBAL_GUARD_BSS(gaMonSeqMonsterTblData_Unknown, "D2Common::Merc::gaMonSeqMonsterTblData_Unknown");
uint8_t gaMonSeqAnimData; D2_GLOBAL_GUARD_BSS(gaMonSeqAnimData, "D2Common::Merc::gaMonSeqAnimData");

} // namespace D2Common::Merc

namespace D2Common::DATATBLS::OverlayTbls {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

uint32_t gaOverlayVariantCodes[43] = {
    0x2074696c,
    0x2074696c,
    0x20736564,
    0x20736564,
    0x20797668,
    0x20797668,
    0x20797668,
    0x2074696c,
    0x20736564,
    0x20797668,
    0x20657861,
    0x20657861,
    0x20616c66,
    0x20616c66,
    0x20786168,
    0x20786168,
    0x2063616d,
    0x2063616d,
    0x206d6373,
    0x206d6373,
    0x0,
    0x20637562,
    0x2067726c,
    0x2074696b,
    0x206c6d73,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x2074696c,
    0x20736564,
    0x20797668,
    0x2074696c,
    0x2064656d,
    0x20797668,
    0x2074696c,
    0x20736564,
    0x20797668
}; D2_GLOBAL_GUARD(gaOverlayVariantCodes, "D2Common::DATATBLS::OverlayTbls::gaOverlayVariantCodes");

char * gaLanguageNames_00748a58[16] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c,
    nullptr
}; D2_GLOBAL_GUARD(gaLanguageNames_00748a58, "D2Common::DATATBLS::OverlayTbls::gaLanguageNames_00748a58");

char * gaLanguageNames_00748a98[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_00748a98, "D2Common::DATATBLS::OverlayTbls::gaLanguageNames_00748a98");

} // namespace D2Common::DATATBLS::OverlayTbls

namespace D2Common::Drlg::DrlgGrid {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

fpFlagOperations apFlagOperations[6] = { (fpFlagOperations)&D2Common::Drlg::DrlgGrid::OrFlag, (fpFlagOperations)&D2Common::Drlg::DrlgGrid::AndFlag, (fpFlagOperations)&D2Common::Drlg::DrlgGrid::XorFlag, (fpFlagOperations)&D2Common::Drlg::DrlgGrid::OverwriteFlag, (fpFlagOperations)&D2Common::Drlg::DrlgGrid::OverwriteFlagIfZero, (fpFlagOperations)&D2Common::Drlg::DrlgGrid::AndNegatedFlag }; D2_GLOBAL_GUARD(apFlagOperations, "D2Common::Drlg::DrlgGrid::apFlagOperations");

} // namespace D2Common::Drlg::DrlgGrid

namespace D2Direct3D::Renderer {

} // namespace D2Direct3D::Renderer

namespace Fog::Src {

} // namespace Fog::Src

namespace D2Gdi::Renderer::Windowed {

} // namespace D2Gdi::Renderer::Windowed

namespace D2Launch::Launcher {

} // namespace D2Launch::Launcher

namespace D2Client::Lng {

} // namespace D2Client::Lng

namespace D2Client::UI::Dialog {

} // namespace D2Client::UI::Dialog

namespace D2Client::UI::SpellSel {

} // namespace D2Client::UI::SpellSel

namespace D2Client::UNIT::CUnit {

} // namespace D2Client::UNIT::CUnit

namespace D2Client::Cursor {

} // namespace D2Client::Cursor

namespace D2Client::Engine::Gfx {

} // namespace D2Client::Engine::Gfx

namespace D2Client::UNIT::Monster {

} // namespace D2Client::UNIT::Monster

namespace D2Client::UI::Trade {

} // namespace D2Client::UI::Trade

namespace D2Client::UNIT::Object {

} // namespace D2Client::UNIT::Object

namespace D2Client::GAME::Sound {

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

namespace Bnclient::Bnet {

} // namespace Bnclient::Bnet

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

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2DataTableTxtStrc sgtDataTable = {}; D2_GLOBAL_GUARD(sgtDataTable, "D2Common::DATATBLS::MonsterTbls::sgtDataTable");

// =============================================================================
// Initialized scalars
// =============================================================================

unsigned int gnTreasureClassArraySize = {}; D2_GLOBAL_GUARD(gnTreasureClassArraySize, "D2Common::DATATBLS::MonsterTbls::gnTreasureClassArraySize");

} // namespace D2Common::DATATBLS::MonsterTbls

namespace D2Common::Logging {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

_RTL_CRITICAL_SECTION lpCriticalSection_0096ca30 = {}; D2_GLOBAL_GUARD(lpCriticalSection_0096ca30, "D2Common::Logging::lpCriticalSection_0096ca30");

// =============================================================================
// Initialized scalars
// =============================================================================

FILE * pLOGFILE = 0; D2_GLOBAL_GUARD(pLOGFILE, "D2Common::Logging::pLOGFILE");
BOOL bLOGFILE_ENABLED = 0; D2_GLOBAL_GUARD(bLOGFILE_ENABLED, "D2Common::Logging::bLOGFILE_ENABLED");

} // namespace D2Common::Logging

namespace D2Common::DATATBLS::TokTbls {

// =============================================================================
// Initialized scalars
// =============================================================================

unsigned int nTxtMonModeSize = {}; D2_GLOBAL_GUARD(nTxtMonModeSize, "D2Common::DATATBLS::TokTbls::nTxtMonModeSize");

} // namespace D2Common::DATATBLS::TokTbls
