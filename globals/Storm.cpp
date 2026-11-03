/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"
#include "Fog/Source/SSComp.h"
#include "Storm/Source/SErr.h"
#include "Storm/Storm.h"


// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2StormListHead gMpqArchiveList = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gMpqArchiveList.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gMpqArchiveList, "gMpqArchiveList");

D2StormListHead gSFileAsyncReqQueue = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gSFileAsyncReqQueue.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gSFileAsyncReqQueue, "gSFileAsyncReqQueue");

D2StormListHead gSFileOpenFileList = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gSFileOpenFileList.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gSFileOpenFileList, "gSFileOpenFileList");

D2StormListHead gSFileAudioStreamList = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gSFileAudioStreamList.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gSFileAudioStreamList, "gSFileAudioStreamList");

D2StormListHead gSFileStreamRecordList = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gSFileStreamRecordList.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gSFileStreamRecordList, "gSFileStreamRecordList");

char sRTTI_ARCHIVEREC_AllocTag[32] = {
    '.',
    '?',
    'A',
    'U',
    'A',
    'R',
    'C',
    'H',
    'I',
    'V',
    'E',
    'R',
    'E',
    'C',
    '@',
    'S',
    'F',
    'i',
    'l',
    'e',
    '@',
    'S',
    't',
    'o',
    'r',
    'm',
    '@',
    '@',
    '\x00',
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(sRTTI_ARCHIVEREC_AllocTag, "sRTTI_ARCHIVEREC_AllocTag");

char sRTTI_EVENTREC_AllocTag[16] = {
    '.',
    '?',
    'A',
    'U',
    'E',
    'V',
    'E',
    'N',
    'T',
    'R',
    'E',
    'C',
    '@',
    '@',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(sRTTI_EVENTREC_AllocTag, "sRTTI_EVENTREC_AllocTag");

char sRTTI_REQUEST_AllocTag[16] = {
    '.',
    '?',
    'A',
    'U',
    'R',
    'E',
    'Q',
    'U',
    'E',
    'S',
    'T',
    '@',
    '@',
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(sRTTI_REQUEST_AllocTag, "sRTTI_REQUEST_AllocTag");

char sRTTI_AUDIOSTREAM_AllocTag[20] = {
    '.',
    '?',
    'A',
    'U',
    'A',
    'U',
    'D',
    'I',
    'O',
    'S',
    'T',
    'R',
    'E',
    'A',
    'M',
    '@',
    '@',
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(sRTTI_AUDIOSTREAM_AllocTag, "sRTTI_AUDIOSTREAM_AllocTag");

D2StormListHead gnStormErrorCallbackCount = {
    /* .nLinkOffset = */ 0x0,
    /* .pPrevLink = */ (StormLinkStrc*)&gnStormErrorCallbackCount.pPrevLink,
    /* .pNext = */ 0x0
}; D2_GLOBAL_GUARD(gnStormErrorCallbackCount, "gnStormErrorCallbackCount");

char gszSSystemListAllocTag[14] = {
    '.',
    '?',
    'A',
    'U',
    'H',
    'A',
    'N',
    'D',
    'L',
    'E',
    'R',
    '@',
    '@',
    '\x00'
}; D2_GLOBAL_GUARD(gszSSystemListAllocTag, "gszSSystemListAllocTag");

// =============================================================================
// Initialized scalars
// =============================================================================

uint32_t gnSFileDataChunkSize = 0x20000; D2_GLOBAL_GUARD(gnSFileDataChunkSize, "gnSFileDataChunkSize");
uint32_t gnSFileReadErrorMode = 0x2; D2_GLOBAL_GUARD(gnSFileReadErrorMode, "gnSFileReadErrorMode");
uint32_t gbSFileSeekOptimization = 0x1; D2_GLOBAL_GUARD(gbSFileSeekOptimization, "gbSFileSeekOptimization");

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

} // namespace D2Client::UI::Dialog

namespace D2Client::Draw::LightMap {

} // namespace D2Client::Draw::LightMap
