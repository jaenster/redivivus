/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"
#include "D2Client/UI/Automap.h"
#include "D2Common/Chat/Ignorelist.h"
#include "D2Common/Chat/Ignorelist/ChatIgnoreList.h"
#include "D2Common/Chat/Ignorelist/IgnoreList.h"
#include "D2Common/D2SeedStrc.h"
#include "D2Common/DATATBLS/ItemTbls.h"
#include "D2Common/DATATBLS/MonsterTbls.h"
#include "D2Common/Drlg/Drlg.h"
#include "D2Common/Drlg/DrlgGrid.h"
#include "D2Common/Logging/ProfCore.h"
#include "D2Game/PLAYER/PlayerPets.h"
#include "D2Game/SKILLS/Items.h"
#include "D2Game/UNIT/SUNITNPC.h"


// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char gpCubeRecipeItemTypeFlags_7[4] = { "set" }; D2_GLOBAL_GUARD(gpCubeRecipeItemTypeFlags_7, "gpCubeRecipeItemTypeFlags_7");

char gsGameMonsterSpawnContext[4] = { 'm', 'a', 'x', '\x00' }; D2_GLOBAL_GUARD(gsGameMonsterSpawnContext, "gsGameMonsterSpawnContext");

char gsGameMissileTblsContext[4] = { 'm', 'i', 'n', '\x00' }; D2_GLOBAL_GUARD(gsGameMissileTblsContext, "gsGameMissileTblsContext");

char gsGameDataTblsContext[4] = { 'm', 'o', 'd', '\x00' }; D2_GLOBAL_GUARD(gsGameDataTblsContext, "gsGameDataTblsContext");

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
}; D2_GLOBAL_GUARD(gnAct5OuterWallGridOffsetByType, "gnAct5OuterWallGridOffsetByType");

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
}; D2_GLOBAL_GUARD(gaAct5SceneryPresetTable_0, "gaAct5SceneryPresetTable_0");

uint32_t gnTimeRates[3] = { 0x80, 0x4, 0x8 }; D2_GLOBAL_GUARD(gnTimeRates, "gnTimeRates");

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
}; D2_GLOBAL_GUARD(gpsPresetObjectTable, "gpsPresetObjectTable");

D2SeedStrc gsAutoMapSeed = D2SeedStrc(); D2_GLOBAL_GUARD(gsAutoMapSeed, "gsAutoMapSeed");

_RTL_CRITICAL_SECTION lpCriticalSection_0096ca30 = {}; D2_GLOBAL_GUARD(lpCriticalSection_0096ca30, "lpCriticalSection_0096ca30");

// =============================================================================
// Initialized scalars
// =============================================================================

unsigned int gszTxtColName_op = 0x706f; D2_GLOBAL_GUARD(gszTxtColName_op, "gszTxtColName_op");
int gaAct5SceneryPresetTable_0_0__1_[15][7] = {}; D2_GLOBAL_GUARD(gaAct5SceneryPresetTable_0_0__1_, "gaAct5SceneryPresetTable_0_0__1_");
FILE * pLOGFILE = 0; D2_GLOBAL_GUARD(pLOGFILE, "pLOGFILE");
BOOL bLOGFILE_ENABLED = 0; D2_GLOBAL_GUARD(bLOGFILE_ENABLED, "bLOGFILE_ENABLED");
unsigned int gnTreasureClassArraySize = {}; D2_GLOBAL_GUARD(gnTreasureClassArraySize, "gnTreasureClassArraySize");
unsigned int nTxtMonModeSize = {}; D2_GLOBAL_GUARD(nTxtMonModeSize, "nTxtMonModeSize");
unsigned int gdwDrlgLevelPlacementFlags = {}; D2_GLOBAL_GUARD(gdwDrlgLevelPlacementFlags, "gdwDrlgLevelPlacementFlags");

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

namespace D2Game::GAME::Rendering {

} // namespace D2Game::GAME::Rendering

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

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

fpFlagOperations apFlagOperations[6] = { (fpFlagOperations)&D2Common::Drlg::DrlgGrid::OrFlag, (fpFlagOperations)&D2Common::Drlg::DrlgGrid::AndFlag, (fpFlagOperations)&D2Common::Drlg::DrlgGrid::XorFlag, (fpFlagOperations)&D2Common::Drlg::DrlgGrid::OverwriteFlag, (fpFlagOperations)&D2Common::Drlg::DrlgGrid::OverwriteFlagIfZero, (fpFlagOperations)&D2Common::Drlg::DrlgGrid::AndNegatedFlag }; D2_GLOBAL_GUARD(apFlagOperations, "D2Common::Drlg::DrlgGrid::apFlagOperations");

} // namespace D2Common::Drlg::DrlgGrid

namespace D2Client::UI::Dialog {

} // namespace D2Client::UI::Dialog

namespace D2Client::Draw::LightMap {

} // namespace D2Client::Draw::LightMap

namespace D2Client::Mouse {

} // namespace D2Client::Mouse

namespace D2Client::Forms {

} // namespace D2Client::Forms

namespace D2Client::GAME::Record {

} // namespace D2Client::GAME::Record
