/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"
#include "D2Gdi/Renderer/Windowed.h"
#include "D2gfx/D2GFX.h"


// =============================================================================
// Initialized scalars
// =============================================================================


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

namespace D2Game::GAME::Rendering {

} // namespace D2Game::GAME::Rendering

namespace D2Client::UI::ui {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2RendererFunctionsStrc WindowedFunctionTable = {
    /* .nfpInitialize = */ &D2Gdi::Renderer::Windowed::Initialize,
    /* .nfpInitPerspective = */ &D2Gdi::Renderer::Windowed::InitPerspective,
    /* .nfpRelease = */ &D2Gdi::Renderer::Windowed::Release,
    /* .nfpCreateWindow = */ &D2Gdi::Renderer::Windowed::CreateWindow,
    /* .nfpDestroyWindow = */ &D2Gdi::Renderer::Windowed::RENDERER_WINDOWED_DestroyWindow,
    /* .nfpEndCutScene = */ &D2Gdi::Renderer::Windowed::EndCutScene,
    /* .nfpBeginScene = */ &D2Gdi::Renderer::Windowed::BeginScene,
    /* .fpEndScene1 = */ &D2Gdi::Renderer::Windowed::EndScene1,
    /* .fpEndScene2 = */ &D2Gdi::Renderer::Windowed::EndScene2,
    /* .nfpResizeWindow = */ &D2Gdi::Renderer::Windowed::ResizeWindow,
    /* .nfpGetBackBuffer = */ &D2Gdi::Renderer::Windowed::GetBackBuffer,
    /* .nfpActivateWindow = */ (fpActivateWindow)&D2Gdi::Renderer::Windowed::ActivateWindow,
    /* .nfpSetOption = */ &D2Gdi::Renderer::Windowed::SetOption,
    /* .nfpBeginCutScene = */ &D2Gdi::Renderer::Windowed::BeginCutScene,
    /* .nfpPlayCutScene = */ (fpPlayCutScene)&D2Gdi::Renderer::Windowed::PlayCutScene,
    /* .nfpCheckCutScene = */ &D2Gdi::Renderer::Windowed::CheckCutScene,
    /* .nfpDecodeSmacker = */ &D2Gdi::Renderer::Windowed::DecodeSmacker,
    /* .nfpPlayerSmacker = */ &D2Gdi::Renderer::Windowed::PlayerSmacker,
    /* .nfpCloseSmacker = */ &D2Gdi::Renderer::Windowed::CloseSmacker,
    /* .nfpGetRenderStatistics = */ &D2Gdi::Renderer::Windowed::GetRendererStatistics,
    /* .nfpGetScreenSize = */ &D2Gdi::Renderer::Windowed::GetScreenSize,
    /* .nfpUpdateScaleFactor = */ &D2Gdi::Renderer::Windowed::UpdateScaleFactor,
    /* .nfpSetGamma = */ &D2Gdi::Renderer::Windowed::SetGamma,
    /* .nfpCheckGamma = */ &D2Gdi::Renderer::Windowed::CheckGamma,
    /* .nfpSetPerspectiveScale = */ &D2Gdi::Renderer::Windowed::SetPerspectiveScale,
    /* .nfpAdjustPerspectivePosition = */ &D2Gdi::Renderer::Windowed::AdjustPerspectivePosition,
    /* .nfpPerspectiveScalePosition = */ &D2Gdi::Renderer::Windowed::PerspecitveScalePosition,
    /* .nfpSetDefaultPerspectiveFactor = */ &D2Gdi::Renderer::Windowed::SetDefaultPerspectiveFactor,
    /* .nfpSetPalette = */ &D2Gdi::Renderer::Windowed::SetPalette,
    /* .nfpSetPaletteTable = */ &D2Gdi::Renderer::Windowed::SetPaletteTable,
    /* .nfpSetGlobalLight = */ &D2Gdi::Renderer::Windowed::SetGlobalLight,
    /* .nfpDrawGroundTile = */ (D2RendererFunctionsStrc_fpDrawGroundTile)&D2Gdi::Renderer::Windowed::DrawGroundTile,
    /* .nfpDrawPerspectiveImage = */ &D2Gdi::Renderer::Windowed::DrawPerspectiveImage,
    /* .nfpDrawImage = */ &D2Gdi::Renderer::Windowed::DrawImage,
    /* .nfpDrawShiftedImage = */ (fpDrawShiftedImage)&D2Gdi::Renderer::Windowed::DrawShiftedImage,
    /* .nfpDrawVerticalCropImage = */ &D2Gdi::Renderer::Windowed::DrawVerticalCropImage,
    /* .nfpDrawShadow = */ &D2Gdi::Renderer::Windowed::DrawShadow,
    /* .nfpDrawImageFast = */ &D2Gdi::Renderer::Windowed::DrawImageFast,
    /* .nfpDrawClippedImage = */ (fpDrawClippedImage)&D2Gdi::Renderer::Windowed::DrawClippedImage,
    /* .nfpDrawWallTile = */ &D2Gdi::Renderer::Windowed::DrawWallTile,
    /* .nfpDrawTransWallTile = */ &D2Gdi::Renderer::Windowed::DrawTransWallTile,
    /* .nfpDrawShadowTile = */ (fpDrawShadowTile)&D2Gdi::Renderer::Windowed::DrawShadowTile,
    /* .nfpDrawRect = */ (fpDrawRect)&D2Gdi::Renderer::Windowed::DrawRect,
    /* .nfpDrawRectEx = */ (fpDrawRectEx)&D2Gdi::Renderer::Windowed::DrawRectEx,
    /* .nfpDrawSolidRect = */ (fpDrawSolidRect)&D2Gdi::Renderer::Windowed::DrawSolidRect,
    /* .nfpDrawSolidSquare = */ &D2Gdi::Renderer::Windowed::DrawSolidSquare,
    /* .nfpDrawSolidRectEx = */ (fpDrawSolidRectEx)&D2Gdi::Renderer::Windowed::DrawSolidRectEx,
    /* .nfpDrawSolidRectAlpha = */ &D2Gdi::Renderer::Windowed::DrawSolidRectAlpha,
    /* .nfpDrawLine = */ (fpDrawLine)&D2Gdi::Renderer::Windowed::DrawLine,
    /* .nfpClearScreen = */ &D2Gdi::Renderer::Windowed::ClearScreen,
    /* .nfpDrawString = */ &D2Gdi::Renderer::Windowed::DrawString,
    /* .nfpDrawLight = */ &D2Gdi::Renderer::Windowed::DrawLight,
    /* .nfpDebugFillBackBuffer = */ &D2Gdi::Renderer::Windowed::DebugFillBackBuffer,
    /* .nfpClearCaches = */ &D2Gdi::Renderer::Windowed::ClearCaches
}; D2_GLOBAL_GUARD(WindowedFunctionTable, "D2Client::Renderer::WindowedFunctionTable");

} // namespace D2Client::Renderer

namespace D2DDraw::Renderer::DirectDraw {

} // namespace D2DDraw::Renderer::DirectDraw

namespace D2Lang::Unicode::UNISYS {

} // namespace D2Lang::Unicode::UNISYS

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

namespace D2Client::Mouse {

} // namespace D2Client::Mouse

namespace D2Client::Forms {

} // namespace D2Client::Forms

namespace D2Client::GAME::Record {

} // namespace D2Client::GAME::Record
