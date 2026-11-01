/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"
#include "D2CMP/Draw.h"
#include "D2CMP/SRC/Codec.h"
#include "D2CMP/SRC/FastCmp.h"
#include "D2CMP/SRC/LRUCache.h"
#include "D2Client/D2GfxMemoryStrc.h"


// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char gszExtDc6[5] = { '.', 'd', 'c', '6', '\x00' }; D2_GLOBAL_GUARD(gszExtDc6, "gszExtDc6");

char gszExtDcc[5] = { '.', 'd', 'c', 'c', '\x00' }; D2_GLOBAL_GUARD(gszExtDcc, "gszExtDcc");

fpCelDrawFrameByFormat gaaCelDrawFuncTable[2] = { &D2CMP::SRC::FastCmp::FASTCMP_DecompressRLEDispatcher, nullptr }; D2_GLOBAL_GUARD(gaaCelDrawFuncTable, "gaaCelDrawFuncTable");

uint32_t gaCelCmpDirectionLookupByDirectionAndFrame[512] = {
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x4,
    0x4,
    0x4,
    0x4,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x5,
    0x5,
    0x5,
    0x5,
    0x5,
    0x5,
    0x5,
    0x5,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x6,
    0x6,
    0x6,
    0x6,
    0x6,
    0x6,
    0x6,
    0x6,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x2,
    0x7,
    0x7,
    0x7,
    0x7,
    0x7,
    0x7,
    0x7,
    0x7,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x3,
    0x4,
    0x4,
    0x4,
    0x4,
    0x4,
    0x4,
    0x8,
    0x8,
    0x8,
    0x8,
    0x0,
    0x0,
    0x0,
    0x0,
    0x9,
    0x9,
    0x9,
    0x9,
    0x5,
    0x5,
    0x5,
    0x5,
    0xa,
    0xa,
    0xa,
    0xa,
    0x1,
    0x1,
    0x1,
    0x1,
    0xb,
    0xb,
    0xb,
    0xb,
    0x6,
    0x6,
    0x6,
    0x6,
    0xc,
    0xc,
    0xc,
    0xc,
    0x2,
    0x2,
    0x2,
    0x2,
    0xd,
    0xd,
    0xd,
    0xd,
    0x7,
    0x7,
    0x7,
    0x7,
    0xe,
    0xe,
    0xe,
    0xe,
    0x3,
    0x3,
    0x3,
    0x3,
    0xf,
    0xf,
    0xf,
    0xf,
    0x4,
    0x4,
    0x4,
    0x10,
    0x10,
    0x8,
    0x8,
    0x11,
    0x11,
    0x0,
    0x0,
    0x12,
    0x12,
    0x9,
    0x9,
    0x13,
    0x13,
    0x5,
    0x5,
    0x14,
    0x14,
    0xa,
    0xa,
    0x15,
    0x15,
    0x1,
    0x1,
    0x16,
    0x16,
    0xb,
    0xb,
    0x17,
    0x17,
    0x6,
    0x6,
    0x18,
    0x18,
    0xc,
    0xc,
    0x19,
    0x19,
    0x2,
    0x2,
    0x1a,
    0x1a,
    0xd,
    0xd,
    0x1b,
    0x1b,
    0x7,
    0x7,
    0x1c,
    0x1c,
    0xe,
    0xe,
    0x1d,
    0x1d,
    0x3,
    0x3,
    0x1e,
    0x1e,
    0xf,
    0xf,
    0x1f,
    0x1f,
    0x4,
    0x4,
    0x20,
    0x10,
    0x21,
    0x8,
    0x22,
    0x11,
    0x23,
    0x0,
    0x24,
    0x12,
    0x25,
    0x9,
    0x26,
    0x13,
    0x27,
    0x5,
    0x28,
    0x14,
    0x29,
    0xa,
    0x2a,
    0x15,
    0x2b,
    0x1,
    0x2c,
    0x16,
    0x2d,
    0xb,
    0x2e,
    0x17,
    0x2f,
    0x6,
    0x30,
    0x18,
    0x31,
    0xc,
    0x32,
    0x19,
    0x33,
    0x2,
    0x34,
    0x1a,
    0x35,
    0xd,
    0x36,
    0x1b,
    0x37,
    0x7,
    0x38,
    0x1c,
    0x39,
    0xe,
    0x3a,
    0x1d,
    0x3b,
    0x3,
    0x3c,
    0x1e,
    0x3d,
    0xf,
    0x3e,
    0x1f,
    0x3f
}; D2_GLOBAL_GUARD(gaCelCmpDirectionLookupByDirectionAndFrame, "gaCelCmpDirectionLookupByDirectionAndFrame");

uint32_t g_aCodecBitMaskLookupTable[48] = {
    0x0,
    0x1,
    0x2,
    0x4,
    0x6,
    0x8,
    0xa,
    0xc,
    0xe,
    0x10,
    0x14,
    0x18,
    0x1a,
    0x1c,
    0x1e,
    0x20,
    0x1,
    0x2,
    0x4,
    0x8,
    0x10,
    0x20,
    0x40,
    0x80,
    0x100,
    0x200,
    0x400,
    0x800,
    0x1000,
    0x2000,
    0x4000,
    0x8000,
    0x10000,
    0x20000,
    0x40000,
    0x80000,
    0x100000,
    0x200000,
    0x400000,
    0x800000,
    0x1000000,
    0x2000000,
    0x4000000,
    0x8000000,
    0x10000000,
    0x20000000,
    0x40000000,
    0x80000000
}; D2_GLOBAL_GUARD(g_aCodecBitMaskLookupTable, "g_aCodecBitMaskLookupTable");

int32_t gaCodecLookupTable[33] = {
    0x0,
    0x1,
    0x3,
    0x7,
    0xf,
    0x1f,
    0x3f,
    0x7f,
    0xff,
    0x1ff,
    0x3ff,
    0x7ff,
    0xfff,
    0x1fff,
    0x3fff,
    0x7fff,
    0xffff,
    0x1ffff,
    0x3ffff,
    0x7ffff,
    0xfffff,
    0x1fffff,
    0x3fffff,
    0x7fffff,
    0xffffff,
    0x1ffffff,
    0x3ffffff,
    0x7ffffff,
    0xfffffff,
    0x1fffffff,
    0x3fffffff,
    0x7fffffff,
    -0x1
}; D2_GLOBAL_GUARD(gaCodecLookupTable, "gaCodecLookupTable");

uint32_t gdwD2CMPCodecTab1[32] = {
    0xffffffff,
    0xfffffffe,
    0xfffffffc,
    0xfffffff8,
    0xfffffff0,
    0xffffffe0,
    0xffffffc0,
    0xffffff80,
    0xffffff00,
    0xfffffe00,
    0xfffffc00,
    0xfffff800,
    0xfffff000,
    0xffffe000,
    0xffffc000,
    0xffff8000,
    0xffff0000,
    0xfffe0000,
    0xfffc0000,
    0xfff80000,
    0xfff00000,
    0xffe00000,
    0xffc00000,
    0xff800000,
    0xff000000,
    0xfe000000,
    0xfc000000,
    0xf8000000,
    0xf0000000,
    0xe0000000,
    0xc0000000,
    0x80000000
}; D2_GLOBAL_GUARD(gdwD2CMPCodecTab1, "gdwD2CMPCodecTab1");

D2LRUCacheStrc gSpriteCacheLru = {}; D2_GLOBAL_GUARD(gSpriteCacheLru, "gSpriteCacheLru");

BYTE gaCelDataHashTable[4096] = {}; D2_GLOBAL_GUARD(gaCelDataHashTable, "gaCelDataHashTable");

D2LRUCacheStrc gCelDataCacheLru = {}; D2_GLOBAL_GUARD(gCelDataCacheLru, "gCelDataCacheLru");

BYTE gaGfxHashTable[65536] = {}; D2_GLOBAL_GUARD(gaGfxHashTable, "gaGfxHashTable");

D2GfxMemoryStrc D2GfxMemoryStrc_0089db60 = {}; D2_GLOBAL_GUARD(D2GfxMemoryStrc_0089db60, "D2GfxMemoryStrc_0089db60");

TSExportTableSync<CELLIST, HCELLIST__*, HLOCKEDCELLIST__*, CCritSect, 0> gCelCmpCacheHandler; D2_GLOBAL_GUARD(gCelCmpCacheHandler, "gCelCmpCacheHandler");

uint32_t gaColorDistanceLookupTable[512] = {}; D2_GLOBAL_GUARD(gaColorDistanceLookupTable, "gaColorDistanceLookupTable");

uint8_t gaPaletteMaskData[256] = {}; D2_GLOBAL_GUARD(gaPaletteMaskData, "gaPaletteMaskData");

D2PaletteEntryStrc gaGlobalPaletteRed[256] = {}; D2_GLOBAL_GUARD(gaGlobalPaletteRed, "gaGlobalPaletteRed");

int32_t gnFastCmpEffectMapMaxRow[80] = {}; D2_GLOBAL_GUARD(gnFastCmpEffectMapMaxRow, "gnFastCmpEffectMapMaxRow");

int32_t gnFastCmpEffectMapMinRow[80] = {}; D2_GLOBAL_GUARD(gnFastCmpEffectMapMinRow, "gnFastCmpEffectMapMinRow");

uint32_t gaFastCmpEffectPixelOffsetTable[6400] = {}; D2_GLOBAL_GUARD(gaFastCmpEffectPixelOffsetTable, "gaFastCmpEffectPixelOffsetTable");

_RTL_CRITICAL_SECTION lpCriticalSection_008f03cc = {}; D2_GLOBAL_GUARD(lpCriticalSection_008f03cc, "lpCriticalSection_008f03cc");

// =============================================================================
// Initialized scalars
// =============================================================================

uint32_t bUseCompressedData = 0x1; D2_GLOBAL_GUARD(bUseCompressedData, "bUseCompressedData");
void* gpszExtDcc = (void*)&gszExtDcc; D2_GLOBAL_GUARD(gpszExtDcc, "gpszExtDcc");
void* gpszExtDc6 = (void*)&gszExtDc6; D2_GLOBAL_GUARD(gpszExtDc6, "gpszExtDc6");
int gbSpriteCacheLruInitialized = 0; D2_GLOBAL_GUARD(gbSpriteCacheLruInitialized, "gbSpriteCacheLruInitialized");
void * gpSpriteCachePoolManager = 0; D2_GLOBAL_GUARD(gpSpriteCachePoolManager, "gpSpriteCachePoolManager");
pfnLruFreeEntry gpfnSpriteCacheFreeEntry = (pfnLruFreeEntry)0; D2_GLOBAL_GUARD(gpfnSpriteCacheFreeEntry, "gpfnSpriteCacheFreeEntry");
uint32_t gbSpriteCacheConversionEnabled = 0; D2_GLOBAL_GUARD(gbSpriteCacheConversionEnabled, "gbSpriteCacheConversionEnabled");
// also labelled: gpPaletteCacheHead
D2TileProjectStrc * gpTileProjectListHead = 0; D2_GLOBAL_GUARD(gpTileProjectListHead, "gpTileProjectListHead");
uint8_t gaItemPaletteData[241920] = {}; D2_GLOBAL_GUARD(gaItemPaletteData, "gaItemPaletteData");
pfnCelPixelDataFree gpCelPixelDataFreeCallback = (pfnCelPixelDataFree)0; D2_GLOBAL_GUARD(gpCelPixelDataFreeCallback, "gpCelPixelDataFreeCallback");
uint32_t gbTransparentColorEnabled = 0; D2_GLOBAL_GUARD(gbTransparentColorEnabled, "gbTransparentColorEnabled");
uint32_t gnPaletteMaxIndex = 0; D2_GLOBAL_GUARD(gnPaletteMaxIndex, "gnPaletteMaxIndex");
uint32_t gnPaletteColorDepth = 0; D2_GLOBAL_GUARD(gnPaletteColorDepth, "gnPaletteColorDepth");
uint32_t gbPaletteLoaded = 0; D2_GLOBAL_GUARD(gbPaletteLoaded, "gbPaletteLoaded");
uint32_t gnDrawTransparentColorRGB = 0; D2_GLOBAL_GUARD(gnDrawTransparentColorRGB, "gnDrawTransparentColorRGB");
uint32_t gbBlackColorIsTransparent = 0; D2_GLOBAL_GUARD(gbBlackColorIsTransparent, "gbBlackColorIsTransparent");
uint32_t gbPaletteMaskEnabled = 0; D2_GLOBAL_GUARD(gbPaletteMaskEnabled, "gbPaletteMaskEnabled");
LONG * lpAddend_008f03e4 = 0; D2_GLOBAL_GUARD(lpAddend_008f03e4, "lpAddend_008f03e4");
uint8_t gaCodecFrameWorkBuffer2[262144] = {}; D2_GLOBAL_GUARD(gaCodecFrameWorkBuffer2, "gaCodecFrameWorkBuffer2");
D2CodecPixelBufferEntryStrc gaCodecFrameDecodeBuffer[5625] = {}; D2_GLOBAL_GUARD(gaCodecFrameDecodeBuffer, "gaCodecFrameDecodeBuffer");

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

} // namespace D2Client::_CdKey

namespace D2Direct3D::Renderer {

} // namespace D2Direct3D::Renderer
