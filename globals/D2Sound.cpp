/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"


// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

GUID gIID_IDirectSound3DBuffer = {
    /* .Data1 = */ 0x279afa86,
    /* .Data2 = */ 0x4981,
    /* .Data3 = */ 0x11ce,
    /* .Data4 = */ { 0xa5, '!', '\x00', ' ', 0xaf, '\x0b', 0xe5, '`' }
}; D2_GLOBAL_GUARD(gIID_IDirectSound3DBuffer, "gIID_IDirectSound3DBuffer");

GUID gIID_IKsPropertySet = {
    /* .Data1 = */ 0x31efac30,
    /* .Data2 = */ 0x515c,
    /* .Data3 = */ 0x11d0,
    /* .Data4 = */ { 0xa9, 0xaa, '\x00', 0xaa, '\x00', 'a', 0xbe, 0x93 }
}; D2_GLOBAL_GUARD(gIID_IKsPropertySet, "gIID_IKsPropertySet");

uint8_t gEAXPropertyGuid_Reserved2[16] = {
    0xa8,
    0xa6,
    0x6,
    0x3,
    0x24,
    0xb2,
    0xd2,
    0x11,
    0x99,
    0xe5,
    0x0,
    0x0,
    0xe8,
    0xd8,
    0xc7,
    0x22
}; D2_GLOBAL_GUARD(gEAXPropertyGuid_Reserved2, "gEAXPropertyGuid_Reserved2");

uint8_t gEAXPropertyGuid_Obstruction[16] = {
    0xa7,
    0xa6,
    0x6,
    0x3,
    0x24,
    0xb2,
    0xd2,
    0x11,
    0x99,
    0xe5,
    0x0,
    0x0,
    0xe8,
    0xd8,
    0xc7,
    0x22
}; D2_GLOBAL_GUARD(gEAXPropertyGuid_Obstruction, "gEAXPropertyGuid_Obstruction");

uint8_t gEAXPropertyGuid_Reserved1[16] = {
    0xc1,
    0x6f,
    0x4e,
    0x4a,
    0x41,
    0xc3,
    0xd1,
    0x11,
    0xb7,
    0x3a,
    0x44,
    0x45,
    0x53,
    0x54,
    0x0,
    0x0
}; D2_GLOBAL_GUARD(gEAXPropertyGuid_Reserved1, "gEAXPropertyGuid_Reserved1");

uint8_t gaEAXPropertyGuid_ObstructionParams[9] = {
    0xc0,
    0x6f,
    0x4e,
    0x4a,
    0x41,
    0xc3,
    0xd1,
    0x11,
    0xb7
}; D2_GLOBAL_GUARD(gaEAXPropertyGuid_ObstructionParams, "gaEAXPropertyGuid_ObstructionParams");

int gAmbientSoundTable[16] = {
    (int)(uintptr_t)s_data_global_music_common_options__006dea54,
    0x0,
    (int)(uintptr_t)s_data_global_music_act1_caves_wav_006dea30,
    0x0,
    (int)(uintptr_t)s_data_global_music_act1_monastery__006dea08,
    0x0,
    (int)(uintptr_t)s_data_global_music_act1_crypt_wav_006de9e4,
    0x0,
    (int)(uintptr_t)s_data_global_music_act2_harem_wav_006de9c0,
    0x0,
    (int)(uintptr_t)s_data_global_music_act2_tombs_wav_006de99c,
    0x0,
    (int)(uintptr_t)s_data_global_music_act3_spider_wav_006de978,
    0x0,
    (int)(uintptr_t)s_data_global_music_act3_kurastsewe_006de950,
    0x0
}; D2_GLOBAL_GUARD(gAmbientSoundTable, "gAmbientSoundTable");

int gaAmbientSoundEntryTable[16] = {
    (int)(uintptr_t)s_data_global_music_introedit_wav_006de930,
    0x0,
    (int)(uintptr_t)s_data_global_music_act5_icecaves_w_006de90c,
    0x0,
    (int)(uintptr_t)s_data_global_music_act5_xtemple_wa_006de8e8,
    0x0,
    (int)(uintptr_t)s_data_global_music_act2_desert_wav_006de8c4,
    0x0,
    (int)(uintptr_t)s_data_global_music_act2_sewer_wav_006de8a0,
    0x0,
    (int)(uintptr_t)s_data_global_music_act3_kurast_wav_006de87c,
    0x0,
    (int)(uintptr_t)s_data_global_music_act3_kurastsewe_006de950,
    0x0,
    (int)(uintptr_t)s_data_global_music_act4_diablo_wav_006de858,
    0x0
}; D2_GLOBAL_GUARD(gaAmbientSoundEntryTable, "gaAmbientSoundEntryTable");

// also labelled: lpCriticalSection_SoundPlay, lpCriticalSection_0088174c
_RTL_CRITICAL_SECTION gSoundSmpCs = {}; D2_GLOBAL_GUARD(gSoundSmpCs, "gSoundSmpCs");

_RTL_CRITICAL_SECTION lpCriticalSection_008817c4 = {}; D2_GLOBAL_GUARD(lpCriticalSection_008817c4, "lpCriticalSection_008817c4");

D2SoundSmpStrc * gpaSoundEffectBuffers[8] = {}; D2_GLOBAL_GUARD(gpaSoundEffectBuffers, "gpaSoundEffectBuffers");

// =============================================================================
// Initialized scalars
// =============================================================================

uint32_t gnSoundHardwareAcceleration = 0x1; D2_GLOBAL_GUARD(gnSoundHardwareAcceleration, "gnSoundHardwareAcceleration");
int gaAmbientSoundFlags = 0x8; D2_GLOBAL_GUARD(gaAmbientSoundFlags, "gaAmbientSoundFlags");
uint32_t gnAmbientSoundCount = 0x8; D2_GLOBAL_GUARD(gnAmbientSoundCount, "gnAmbientSoundCount");
uint32_t gnSoundGlobalVolume = 0xff; D2_GLOBAL_GUARD(gnSoundGlobalVolume, "gnSoundGlobalVolume");
IKsPropertySet * gpDirectSoundEAXInterface = 0; D2_GLOBAL_GUARD(gpDirectSoundEAXInterface, "gpDirectSoundEAXInterface");
HWND g_hwndSoundWindow = 0; D2_GLOBAL_GUARD(g_hwndSoundWindow, "g_hwndSoundWindow");
IDirectSound3DBuffer * gpEAXInterface1 = 0; D2_GLOBAL_GUARD(gpEAXInterface1, "gpEAXInterface1");
IDirectSoundBuffer * gpEAXInterface2 = 0; D2_GLOBAL_GUARD(gpEAXInterface2, "gpEAXInterface2");
IDirectSoundBuffer * gpDSoundPrimaryBuffer = 0; D2_GLOBAL_GUARD(gpDSoundPrimaryBuffer, "gpDSoundPrimaryBuffer");
uint32_t gnDirectSoundInitialized = 0; D2_GLOBAL_GUARD(gnDirectSoundInitialized, "gnDirectSoundInitialized");
IDirectSound * gnDirectSoundInitStatus = 0; D2_GLOBAL_GUARD(gnDirectSoundInitStatus, "gnDirectSoundInitStatus");
uint32_t gnDSoundPrimaryBufferDelay = 0; D2_GLOBAL_GUARD(gnDSoundPrimaryBufferDelay, "gnDSoundPrimaryBufferDelay");
uint32_t gnSound3DLoudness = 0; D2_GLOBAL_GUARD(gnSound3DLoudness, "gnSound3DLoudness");
int gnSoundQualityMode = 0; D2_GLOBAL_GUARD(gnSoundQualityMode, "gnSoundQualityMode");
uint32_t g_bEAXEnabled = 0; D2_GLOBAL_GUARD(g_bEAXEnabled, "g_bEAXEnabled");
uint32_t gbDirectSoundCritSecInitialized = 0; D2_GLOBAL_GUARD(gbDirectSoundCritSecInitialized, "gbDirectSoundCritSecInitialized");
uint32_t g_bSoundDriverScanComplete = 0; D2_GLOBAL_GUARD(g_bSoundDriverScanComplete, "g_bSoundDriverScanComplete");
uint32_t gbDirectSoundDelayWorkaround = 0; D2_GLOBAL_GUARD(gbDirectSoundDelayWorkaround, "gbDirectSoundDelayWorkaround");
uint32_t gbHasLegacyEMU10K1Driver = 0; D2_GLOBAL_GUARD(gbHasLegacyEMU10K1Driver, "gbHasLegacyEMU10K1Driver");
uint32_t gbHasLegacySoundDriverCrlds3d = 0; D2_GLOBAL_GUARD(gbHasLegacySoundDriverCrlds3d, "gbHasLegacySoundDriverCrlds3d");
uint32_t LanguageCode = 0; D2_GLOBAL_GUARD(LanguageCode, "LanguageCode");
D2SoundSmpStrc * gpMusicStreamBuffer = 0; D2_GLOBAL_GUARD(gpMusicStreamBuffer, "gpMusicStreamBuffer");
uint32_t gnMusicEnabled = 0; D2_GLOBAL_GUARD(gnMusicEnabled, "gnMusicEnabled");
uint32_t gbSoundFastPathActive = 0; D2_GLOBAL_GUARD(gbSoundFastPathActive, "gbSoundFastPathActive");
uint32_t gnAmbientSoundLanguageCode = 0; D2_GLOBAL_GUARD(gnAmbientSoundLanguageCode, "gnAmbientSoundLanguageCode");
uint32_t gbAmbientSoundLangInitialized = 0; D2_GLOBAL_GUARD(gbAmbientSoundLangInitialized, "gbAmbientSoundLangInitialized");
uint32_t g_nSoundMixerType = 0; D2_GLOBAL_GUARD(g_nSoundMixerType, "g_nSoundMixerType");
uint32_t g_nSoundMasterVolume = 0; D2_GLOBAL_GUARD(g_nSoundMasterVolume, "g_nSoundMasterVolume");
uint32_t g_nSoundMusicVolume = 0; D2_GLOBAL_GUARD(g_nSoundMusicVolume, "g_nSoundMusicVolume");
uint32_t g_nSoundPositionalBias = 0; D2_GLOBAL_GUARD(g_nSoundPositionalBias, "g_nSoundPositionalBias");
uint32_t g_nSoundNpcSpeech = 0; D2_GLOBAL_GUARD(g_nSoundNpcSpeech, "g_nSoundNpcSpeech");
uint32_t g_nSoundOptionsMusic = 0; D2_GLOBAL_GUARD(g_nSoundOptionsMusic, "g_nSoundOptionsMusic");
uint32_t gnSoundEffectSampleCount = 0; D2_GLOBAL_GUARD(gnSoundEffectSampleCount, "gnSoundEffectSampleCount");
void * gpWavFileCacheListHead = 0; D2_GLOBAL_GUARD(gpWavFileCacheListHead, "gpWavFileCacheListHead");
uint32_t g_nSoundSampleParam = 0; D2_GLOBAL_GUARD(g_nSoundSampleParam, "g_nSoundSampleParam");
int * gpSoundSampleList = 0; D2_GLOBAL_GUARD(gpSoundSampleList, "gpSoundSampleList");
HANDLE ghSoundThreadStopEvent = 0; D2_GLOBAL_GUARD(ghSoundThreadStopEvent, "ghSoundThreadStopEvent");
HANDLE ghSoundThread = 0; D2_GLOBAL_GUARD(ghSoundThread, "ghSoundThread");

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
