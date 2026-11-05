/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"
#include "D2Direct3D/Renderer/Direct3D.h"
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

namespace D2Launch::CharSel {

} // namespace D2Launch::CharSel

namespace D2Game::GAME::Rendering {

} // namespace D2Game::GAME::Rendering

namespace D2Client::Renderer {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2RendererFunctionsStrc Direct3DFunctionTable = {
    /* .nfpInitialize = */ &D2Direct3D::Renderer::Initialize,
    /* .nfpInitPerspective = */ &D2Direct3D::Renderer::InitPerspective,
    /* .nfpRelease = */ (fpRelease)&D2Direct3D::Renderer::Release,
    /* .nfpCreateWindow = */ &D2Direct3D::Renderer::CreateWindow,
    /* .nfpDestroyWindow = */ &D2Direct3D::Renderer::RENDERER_D3D_DestroyWindow,
    /* .nfpEndCutScene = */ &D2Direct3D::Renderer::EndCutScene,
    /* .nfpBeginScene = */ &D2Direct3D::Renderer::BeginScene,
    /* .fpEndScene1 = */ &D2Direct3D::Renderer::EndScene1,
    /* .fpEndScene2 = */ &D2Direct3D::Renderer::EndScene2,
    /* .nfpResizeWindow = */ &D2Direct3D::Renderer::ResizeWindow,
    /* .nfpGetBackBuffer = */ &D2Direct3D::Renderer::GetBackBuffer,
    /* .nfpActivateWindow = */ (fpActivateWindow)&D2Direct3D::Renderer::ActivateWindow,
    /* .nfpSetOption = */ &D2Direct3D::Renderer::SetOption,
    /* .nfpBeginCutScene = */ &D2Direct3D::Renderer::BeginCutScene,
    /* .nfpPlayCutScene = */ (fpPlayCutScene)&D2Direct3D::Renderer::PlayCutScene,
    /* .nfpCheckCutScene = */ &D2Direct3D::Renderer::CheckCutScene,
    /* .nfpDecodeSmacker = */ &D2Direct3D::Renderer::DecodeSmacker,
    /* .nfpPlayerSmacker = */ &D2Direct3D::Renderer::PlayerSmacker,
    /* .nfpCloseSmacker = */ &D2Direct3D::Renderer::CloseSmacker,
    /* .nfpGetRenderStatistics = */ &D2Direct3D::Renderer::GetRenderStatistics,
    /* .nfpGetScreenSize = */ &D2Direct3D::Renderer::GetScreenSize,
    /* .nfpUpdateScaleFactor = */ &D2Direct3D::Renderer::UpdateScaleFactor,
    /* .nfpSetGamma = */ &D2Direct3D::Renderer::SetGamma,
    /* .nfpCheckGamma = */ &D2Direct3D::Renderer::CheckGamma,
    /* .nfpSetPerspectiveScale = */ &D2Direct3D::Renderer::SetPerspectiveScale,
    /* .nfpAdjustPerspectivePosition = */ &D2Direct3D::Renderer::AdjustPerspectivePosition,
    /* .nfpPerspectiveScalePosition = */ &D2Direct3D::Renderer::PerspectiveScalePosition,
    /* .nfpSetDefaultPerspectiveFactor = */ &D2Direct3D::Renderer::SetDefaultPerspectiveFactor,
    /* .nfpSetPalette = */ &D2Direct3D::Renderer::SetPalette,
    /* .nfpSetPaletteTable = */ &D2Direct3D::Renderer::SetPaletteTable,
    /* .nfpSetGlobalLight = */ &D2Direct3D::Renderer::SetGlobalLight,
    /* .nfpDrawGroundTile = */ (D2RendererFunctionsStrc_fpDrawGroundTile)&D2Direct3D::Renderer::DrawGroundTile,
    /* .nfpDrawPerspectiveImage = */ &D2Direct3D::Renderer::DrawPerspectiveImage,
    /* .nfpDrawImage = */ &D2Direct3D::Renderer::DrawImage,
    /* .nfpDrawShiftedImage = */ (fpDrawShiftedImage)&D2Direct3D::Renderer::DrawShiftedImage,
    /* .nfpDrawVerticalCropImage = */ &D2Direct3D::Renderer::DrawVerticalCropImage,
    /* .nfpDrawShadow = */ &D2Direct3D::Renderer::DrawShadow,
    /* .nfpDrawImageFast = */ &D2Direct3D::Renderer::DrawImageFast,
    /* .nfpDrawClippedImage = */ &D2Direct3D::Renderer::DrawClippedImage,
    /* .nfpDrawWallTile = */ &D2Direct3D::Renderer::DrawWallTile,
    /* .nfpDrawTransWallTile = */ &D2Direct3D::Renderer::DrawTransWallTile,
    /* .nfpDrawShadowTile = */ &D2Direct3D::Renderer::DrawShadowTile,
    /* .nfpDrawRect = */ (fpDrawRect)&D2Direct3D::Renderer::DrawRect,
    /* .nfpDrawRectEx = */ (fpDrawRectEx)&D2Direct3D::Renderer::DrawRectEx,
    /* .nfpDrawSolidRect = */ &D2Direct3D::Renderer::DrawSolidRect,
    /* .nfpDrawSolidSquare = */ &D2Direct3D::Renderer::DrawSolidSquare,
    /* .nfpDrawSolidRectEx = */ (fpDrawSolidRectEx)&D2Direct3D::Renderer::DrawSolidRectEx,
    /* .nfpDrawSolidRectAlpha = */ &D2Direct3D::Renderer::DrawSolidRectAlpha,
    /* .nfpDrawLine = */ (fpDrawLine)&D2Direct3D::Renderer::DrawLine,
    /* .nfpClearScreen = */ &D2Direct3D::Renderer::ClearScreen,
    /* .nfpDrawString = */ &D2Direct3D::Renderer::DrawString,
    /* .nfpDrawLight = */ &D2Direct3D::Renderer::DrawLight,
    /* .nfpDebugFillBackBuffer = */ &D2Direct3D::Renderer::DebugFillBackBuffer,
    /* .nfpClearCaches = */ &D2Direct3D::Renderer::ClearCaches
}; D2_GLOBAL_GUARD(Direct3DFunctionTable, "D2Client::Renderer::Direct3DFunctionTable");

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

namespace D2Win::Src::D2WinMain {

} // namespace D2Win::Src::D2WinMain

namespace D2Win::D2GFX {

} // namespace D2Win::D2GFX
