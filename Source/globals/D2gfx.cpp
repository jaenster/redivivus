/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "Source/D2Direct3D/Renderer/Direct3D.h"
#include "Source/D2Direct3D/d3dFloorCache.h"
#include "Source/D2Direct3D/d3dSubtileCache.h"
#include "Source/D2Direct3D/d3dTextureCache.h"
#include "Source/D2Glide/Renderer/Glide.h"
#include "Source/D2Glide/glFloorCache.h"
#include "Source/D2Glide/glSubtileCache.h"
#include "Source/D2Hell/D2Hell.h"
#include "Source/D2Multi/OOGUtilities.h"
#include "Source/D2gfx/D2GFX.h"
#include "Source/D2gfx/D2gfx.globals.h"
#include "Source/Fog/D2MemoryManagerStrc.h"
#include "Source/Fog/D2PoolManagerStrc.h"
#include "Source/Fog/D2QServerStrc.h"
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
#include "Source/D2Client/D2GfxSettingsStrc.h"


// =============================================================================
// Initialized scalars
// =============================================================================


namespace D2Common::Drlg::Maze {

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

} // namespace D2Common::DATATBLS::LvlTbls

namespace D2Client::ClientModeInGame {

} // namespace D2Client::ClientModeInGame

namespace D2Client::UI::Draw {

} // namespace D2Client::UI::Draw

namespace D2Client::UI::Inv {

} // namespace D2Client::UI::Inv

namespace D2Common::DATATBLS::DataTbls {

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

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

uint64_t gaIntensityTableConstant = 0x408f400000000000; D2_GLOBAL_GUARD(gaIntensityTableConstant, "D2gfx::D2GFX::gaIntensityTableConstant");

int gaGroundTileRowOffsets[15] = {
    0xe,
    0xc,
    0xa,
    0x8,
    0x6,
    0x4,
    0x2,
    0x0,
    0x2,
    0x4,
    0x6,
    0x8,
    0xa,
    0xc,
    0xe
}; D2_GLOBAL_GUARD(gaGroundTileRowOffsets, "D2gfx::D2GFX::gaGroundTileRowOffsets");

int gaGroundTileRowWidths[15] = {
    0x4,
    0x8,
    0xc,
    0x10,
    0x14,
    0x18,
    0x1c,
    0x20,
    0x1c,
    0x18,
    0x14,
    0x10,
    0xc,
    0x8,
    0x4
}; D2_GLOBAL_GUARD(gaGroundTileRowWidths, "D2gfx::D2GFX::gaGroundTileRowWidths");

int gaGroundTileSrcPointers[15] = {
    0x0,
    0x4,
    0xc,
    0x18,
    0x28,
    0x3c,
    0x54,
    0x70,
    0x90,
    0xac,
    0xc4,
    0xd8,
    0xe8,
    0xf4,
    0xfc
}; D2_GLOBAL_GUARD(gaGroundTileSrcPointers, "D2gfx::D2GFX::gaGroundTileSrcPointers");

uint8_t gpLineBufferTileArray[32][32][32] = {}; D2_GLOBAL_GUARD(gpLineBufferTileArray, "D2gfx::D2GFX::gpLineBufferTileArray");

// =============================================================================
// Initialized scalars
// =============================================================================

int gnTileInnerClipZMin = 0; D2_GLOBAL_GUARD(gnTileInnerClipZMin, "D2gfx::D2GFX::gnTileInnerClipZMin");
int gnTileClipXMin = 0; D2_GLOBAL_GUARD(gnTileClipXMin, "D2gfx::D2GFX::gnTileClipXMin");
int gnTileInnerXMax = 0; D2_GLOBAL_GUARD(gnTileInnerXMax, "D2gfx::D2GFX::gnTileInnerXMax");
int gnTileClipZMax = 0; D2_GLOBAL_GUARD(gnTileClipZMax, "D2gfx::D2GFX::gnTileClipZMax");
int gnTileViewportXOffset = 0; D2_GLOBAL_GUARD(gnTileViewportXOffset, "D2gfx::D2GFX::gnTileViewportXOffset");
int gnTileInnerClipXMax = 0; D2_GLOBAL_GUARD(gnTileInnerClipXMax, "D2gfx::D2GFX::gnTileInnerClipXMax");
int gnTileClipZMin = 0; D2_GLOBAL_GUARD(gnTileClipZMin, "D2gfx::D2GFX::gnTileClipZMin");
int gnTileClipBottom = 0; D2_GLOBAL_GUARD(gnTileClipBottom, "D2gfx::D2GFX::gnTileClipBottom");

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

} // namespace D2Common::Chat::Ignorelist::ChatIgnoreList

namespace D2Common::Chat::Ignorelist::IgnoreList {

} // namespace D2Common::Chat::Ignorelist::IgnoreList

namespace D2Common {

} // namespace D2Common

namespace D2Common::Txt::Cube {

} // namespace D2Common::Txt::Cube

namespace D2Common::DATATBLS::ItemTbls {

} // namespace D2Common::DATATBLS::ItemTbls

namespace D2Common::Drlg::Preset {

} // namespace D2Common::Drlg::Preset

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

} // namespace D2Common::Chat::Ignorelist::MappingEntryNode

namespace D2Common::Chat::Ignorelist::StringEntryNode {

} // namespace D2Common::Chat::Ignorelist::StringEntryNode

namespace D2Common::Chat::Ignorelist {

} // namespace D2Common::Chat::Ignorelist

namespace D2Common::Logging::ProfCore {

} // namespace D2Common::Logging::ProfCore

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

// =============================================================================
// Initialized scalars
// =============================================================================

int gnTileClipXMax = 0; D2_GLOBAL_GUARD(gnTileClipXMax, "D2gfx::gnTileClipXMax");
int gnTileClipYMin = 0; D2_GLOBAL_GUARD(gnTileClipYMin, "D2gfx::gnTileClipYMin");
int gnTileInnerClipYMax = 0; D2_GLOBAL_GUARD(gnTileInnerClipYMax, "D2gfx::gnTileInnerClipYMax");
int gnTileInnerClipXMin = 0; D2_GLOBAL_GUARD(gnTileInnerClipXMin, "D2gfx::gnTileInnerClipXMin");
int gnMinTileScreenY = 0; D2_GLOBAL_GUARD(gnMinTileScreenY, "D2gfx::gnMinTileScreenY");
int gnTileClipLeft = 0; D2_GLOBAL_GUARD(gnTileClipLeft, "D2gfx::gnTileClipLeft");
int gnTileInnerClipYMinRow = 0; D2_GLOBAL_GUARD(gnTileInnerClipYMinRow, "D2gfx::gnTileInnerClipYMinRow");
int gnTileInnerClipYMin = 0; D2_GLOBAL_GUARD(gnTileInnerClipYMin, "D2gfx::gnTileInnerClipYMin");
int gnTileInnerClipXMax_b = 0; D2_GLOBAL_GUARD(gnTileInnerClipXMax_b, "D2gfx::gnTileInnerClipXMax_b");
int gnMinTileDrawY = 0; D2_GLOBAL_GUARD(gnMinTileDrawY, "D2gfx::gnMinTileDrawY");
int gnMinTileScreenY_b = 0; D2_GLOBAL_GUARD(gnMinTileScreenY_b, "D2gfx::gnMinTileScreenY_b");
int gnTileClipRight = 0; D2_GLOBAL_GUARD(gnTileClipRight, "D2gfx::gnTileClipRight");
BYTE * * gpPaletteBlendLookupTable = 0; D2_GLOBAL_GUARD(gpPaletteBlendLookupTable, "D2gfx::gpPaletteBlendLookupTable");

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

} // namespace D2Common::DATATBLS::MonsterTbls

namespace D2Common::Logging {

} // namespace D2Common::Logging

namespace D2Common::DATATBLS::TokTbls {

} // namespace D2Common::DATATBLS::TokTbls
