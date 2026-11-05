/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"
#include "D2DDraw/Renderer/DirectDraw.h"
#include "D2gfx/D2GFX.h"


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

namespace D2Client::Renderer {

} // namespace D2Client::Renderer

namespace D2DDraw::Renderer::DirectDraw {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2RendererFunctionsStrc DirectDrawFunctionTable = {
    /* .nfpInitialize = */ &D2DDraw::Renderer::DirectDraw::Initialize,
    /* .nfpInitPerspective = */ &D2DDraw::Renderer::DirectDraw::InitPerspective,
    /* .nfpRelease = */ (fpRelease)&D2DDraw::Renderer::DirectDraw::Release,
    /* .nfpCreateWindow = */ (fpCreateWindow)&D2DDraw::Renderer::DirectDraw::CreateWindow,
    /* .nfpDestroyWindow = */ &D2DDraw::Renderer::DirectDraw::RENDERER_DDRAW_DestroyWindow,
    /* .nfpEndCutScene = */ (fpEndCutScene)&D2DDraw::Renderer::DirectDraw::EndCutScene,
    /* .nfpBeginScene = */ &D2DDraw::Renderer::DirectDraw::BeginScene,
    /* .fpEndScene1 = */ &D2DDraw::Renderer::DirectDraw::EndScene1,
    /* .fpEndScene2 = */ &D2DDraw::Renderer::DirectDraw::EndScene2,
    /* .nfpResizeWindow = */ &D2DDraw::Renderer::DirectDraw::ResizeWindow,
    /* .nfpGetBackBuffer = */ &D2DDraw::Renderer::DirectDraw::GetBackBuffer,
    /* .nfpActivateWindow = */ (fpActivateWindow)&D2DDraw::Renderer::DirectDraw::ActivateWindow,
    /* .nfpSetOption = */ (fpSetOption)&D2DDraw::Renderer::DirectDraw::SetOption,
    /* .nfpBeginCutScene = */ &D2DDraw::Renderer::DirectDraw::BeginCutScene,
    /* .nfpPlayCutScene = */ (fpPlayCutScene)&D2DDraw::Renderer::DirectDraw::PlayCutScene,
    /* .nfpCheckCutScene = */ &D2DDraw::Renderer::DirectDraw::CheckCutScene,
    /* .nfpDecodeSmacker = */ (fpDecodeSmacker)&D2DDraw::Renderer::DirectDraw::DecodeSmacker,
    /* .nfpPlayerSmacker = */ &D2DDraw::Renderer::DirectDraw::PlayerSmacker,
    /* .nfpCloseSmacker = */ &D2DDraw::Renderer::DirectDraw::CloseSmacker,
    /* .nfpGetRenderStatistics = */ &D2DDraw::Renderer::DirectDraw::GetRenderStatistics,
    /* .nfpGetScreenSize = */ (fpGetScreenSize)&D2DDraw::Renderer::DirectDraw::GetScreenSize,
    /* .nfpUpdateScaleFactor = */ (fpUpdateScaleFactor)&D2DDraw::Renderer::DirectDraw::UpdateScaleFactor,
    /* .nfpSetGamma = */ (fpSetGamma)&D2DDraw::Renderer::DirectDraw::SetGamma,
    /* .nfpCheckGamma = */ (fpCheckGamma)&D2DDraw::Renderer::DirectDraw::CheckGamma,
    /* .nfpSetPerspectiveScale = */ (fpSetPerspectiveScale)&D2DDraw::Renderer::DirectDraw::SetPerspectiveScale,
    /* .nfpAdjustPerspectivePosition = */ (fpAdjustPerspectivePosition)&D2DDraw::Renderer::DirectDraw::AdjustPerspectivePosition,
    /* .nfpPerspectiveScalePosition = */ (fpPerspectiveScalePosition)&D2DDraw::Renderer::DirectDraw::PerspectiveScalePosition,
    /* .nfpSetDefaultPerspectiveFactor = */ &D2DDraw::Renderer::DirectDraw::SetDefaultPerspectiveFactor,
    /* .nfpSetPalette = */ (fpSetPalette)&D2DDraw::Renderer::DirectDraw::SetPalette,
    /* .nfpSetPaletteTable = */ &D2DDraw::Renderer::DirectDraw::SetPaletteTable,
    /* .nfpSetGlobalLight = */ &D2DDraw::Renderer::DirectDraw::SetGlobalLight,
    /* .nfpDrawGroundTile = */ (D2RendererFunctionsStrc_fpDrawGroundTile)&D2DDraw::Renderer::DirectDraw::DrawGroundTile,
    /* .nfpDrawPerspectiveImage = */ (fpDrawPerspectiveImage)&D2DDraw::Renderer::DirectDraw::DrawPerspectiveImage,
    /* .nfpDrawImage = */ (fpDrawImage)&D2DDraw::Renderer::DirectDraw::DrawImage,
    /* .nfpDrawShiftedImage = */ (fpDrawShiftedImage)&D2DDraw::Renderer::DirectDraw::DrawShiftedImage,
    /* .nfpDrawVerticalCropImage = */ (fpDrawVerticalCropImage)&D2DDraw::Renderer::DirectDraw::DrawVerticalCropImage,
    /* .nfpDrawShadow = */ (fpDrawShadow)&D2DDraw::Renderer::DirectDraw::DrawShadow,
    /* .nfpDrawImageFast = */ (fpDrawImageFast)&D2DDraw::Renderer::DirectDraw::DrawImageFast,
    /* .nfpDrawClippedImage = */ (fpDrawClippedImage)&D2DDraw::Renderer::DirectDraw::DrawClippedImage,
    /* .nfpDrawWallTile = */ (D2RendererFunctionsStrc_fpDrawWallTile)&D2DDraw::Renderer::DirectDraw::DrawWallTile,
    /* .nfpDrawTransWallTile = */ (fpDrawTransWallTile)&D2DDraw::Renderer::DirectDraw::DrawTransWallTile,
    /* .nfpDrawShadowTile = */ (fpDrawShadowTile)&D2DDraw::Renderer::DirectDraw::DrawShadowTile,
    /* .nfpDrawRect = */ &D2DDraw::Renderer::DirectDraw::DrawRect,
    /* .nfpDrawRectEx = */ &D2DDraw::Renderer::DirectDraw::DrawRectEx,
    /* .nfpDrawSolidRect = */ &D2DDraw::Renderer::DirectDraw::DrawSolidRect,
    /* .nfpDrawSolidSquare = */ &D2DDraw::Renderer::DirectDraw::DrawSolidSquare,
    /* .nfpDrawSolidRectEx = */ (fpDrawSolidRectEx)&D2DDraw::Renderer::DirectDraw::DrawSolidRectEx,
    /* .nfpDrawSolidRectAlpha = */ &D2DDraw::Renderer::DirectDraw::DrawSolidRectAlpha,
    /* .nfpDrawLine = */ (fpDrawLine)&D2DDraw::Renderer::DirectDraw::DrawLine,
    /* .nfpClearScreen = */ &D2DDraw::Renderer::DirectDraw::ClearScreen,
    /* .nfpDrawString = */ (fpDrawString)&D2DDraw::Renderer::DirectDraw::DrawString,
    /* .nfpDrawLight = */ (fpDrawLight)&D2DDraw::Renderer::DirectDraw::DrawLight,
    /* .nfpDebugFillBackBuffer = */ (fpDebugFillBackBuffer)&D2DDraw::Renderer::DirectDraw::DebugFillBackBuffer,
    /* .nfpClearCaches = */ &D2DDraw::Renderer::DirectDraw::ClearCaches
}; D2_GLOBAL_GUARD(DirectDrawFunctionTable, "D2DDraw::Renderer::DirectDraw::DirectDrawFunctionTable");

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
