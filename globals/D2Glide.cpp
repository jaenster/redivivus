/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"
#include "D2Client/Renderer/Glide.h"
#include "D2Glide/glFloorCache.h"
#include "D2Glide/glSubtileCache.h"


// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

GrTexInfo gTexInfoDebugFontGlide = {
    /* .smallLodLog2 = */ 0x4,
    /* .largeLodLog2 = */ 0x4,
    /* .aspectRatioLog2 = */ (GrAspectRatio_t)-0x1,
    /* .format = */ 0x5,
    /* .data = */ nullptr
}; D2_GLOBAL_GUARD(gTexInfoDebugFontGlide, "gTexInfoDebugFontGlide");

void * gapGlideStripVertexPtrsA[10] = {
    (void*)gaGlidePerspectiveVertices,
    (void*)&gaGlidePerspectiveVertices[1].fX,
    (void*)&gaGlidePerspectiveVertices[3].fX,
    (void*)&gaGlidePerspectiveVertices[4].fX,
    (void*)&gaGlidePerspectiveVertices[6].fX,
    (void*)&gaGlidePerspectiveVertices[7].fX,
    (void*)&gaGlidePerspectiveVertices[9].fX,
    (void*)&gaGlidePerspectiveVertices[10].fX,
    (void*)&gaGlidePerspectiveVertices[12].fX,
    (void*)&gaGlidePerspectiveVertices[13].fX
}; D2_GLOBAL_GUARD(gapGlideStripVertexPtrsA, "gapGlideStripVertexPtrsA");

void * gapGlideStripVertexPtrsB[10] = {
    (void*)&gaGlidePerspectiveVertices[1].fX,
    (void*)&gaGlidePerspectiveVertices[2].fX,
    (void*)&gaGlidePerspectiveVertices[4].fX,
    (void*)&gaGlidePerspectiveVertices[5].fX,
    (void*)&gaGlidePerspectiveVertices[7].fX,
    (void*)&gaGlidePerspectiveVertices[8].fX,
    (void*)&gaGlidePerspectiveVertices[10].fX,
    (void*)&gaGlidePerspectiveVertices[11].fX,
    (void*)&gaGlidePerspectiveVertices[13].fX,
    (void*)&gaGlidePerspectiveVertices[14].fX
}; D2_GLOBAL_GUARD(gapGlideStripVertexPtrsB, "gapGlideStripVertexPtrsB");

D2GlideWallCornerStrc gaGlideWallCornerTable[60] = {
    {},
    {},
    {},
    {},
    {},
    {},
    {
        /* .nX = */ 0x0,
        /* .nY = */ 0x5,
        /* .nZ = */ -0x28
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ 0x3,
        /* .nZ = */ -0x38
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ 0x1,
        /* .nZ = */ -0x48
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ -0x1,
        /* .nZ = */ -0x58
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ -0x3,
        /* .nZ = */ -0x68
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ -0x5,
        /* .nZ = */ -0x78
    },
    {
        /* .nX = */ -0x5,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x78
    },
    {
        /* .nX = */ -0x3,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x68
    },
    {
        /* .nX = */ -0x1,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x58
    },
    {
        /* .nX = */ 0x1,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x48
    },
    {
        /* .nX = */ 0x3,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x38
    },
    {
        /* .nX = */ 0x5,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x28
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ 0x5,
        /* .nZ = */ -0x28
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ 0x3,
        /* .nZ = */ -0x38
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ 0x1,
        /* .nZ = */ -0x48
    },
    {
        /* .nX = */ 0x1,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x48
    },
    {
        /* .nX = */ 0x3,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x38
    },
    {
        /* .nX = */ 0x5,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x28
    },
    {
        /* .nX = */ -0x5,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x78
    },
    {
        /* .nX = */ -0x3,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x68
    },
    {
        /* .nX = */ -0x1,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x58
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ -0x1,
        /* .nZ = */ -0x58
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ -0x3,
        /* .nZ = */ -0x68
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ -0x5,
        /* .nZ = */ -0x78
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ 0x5,
        /* .nZ = */ -0x28
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ 0x3,
        /* .nZ = */ -0x38
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ 0x1,
        /* .nZ = */ -0x48
    },
    {
        /* .nX = */ 0x1,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x48
    },
    {
        /* .nX = */ 0x3,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x38
    },
    {
        /* .nX = */ 0x5,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x28
    },
    {
        /* .nX = */ 0x5,
        /* .nY = */ 0xa,
        /* .nZ = */ 0x28
    },
    {
        /* .nX = */ 0x5,
        /* .nY = */ 0x8,
        /* .nZ = */ 0x18
    },
    {
        /* .nX = */ 0x5,
        /* .nY = */ 0x6,
        /* .nZ = */ 0x8
    },
    {
        /* .nX = */ 0x5,
        /* .nY = */ 0x4,
        /* .nZ = */ -0x8
    },
    {
        /* .nX = */ 0x5,
        /* .nY = */ 0x2,
        /* .nZ = */ -0x18
    },
    {
        /* .nX = */ 0x5,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x28
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ 0x5,
        /* .nZ = */ -0x28
    },
    {
        /* .nX = */ 0x2,
        /* .nY = */ 0x5,
        /* .nZ = */ -0x18
    },
    {
        /* .nX = */ 0x4,
        /* .nY = */ 0x5,
        /* .nZ = */ -0x8
    },
    {
        /* .nX = */ 0x6,
        /* .nY = */ 0x5,
        /* .nZ = */ 0x8
    },
    {
        /* .nX = */ 0x8,
        /* .nY = */ 0x5,
        /* .nZ = */ 0x18
    },
    {
        /* .nX = */ 0xa,
        /* .nY = */ 0x5,
        /* .nZ = */ 0x28
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ 0x5,
        /* .nZ = */ -0x28
    },
    {
        /* .nX = */ 0x2,
        /* .nY = */ 0x5,
        /* .nZ = */ -0x18
    },
    {
        /* .nX = */ 0x4,
        /* .nY = */ 0x5,
        /* .nZ = */ -0x8
    },
    {
        /* .nX = */ 0x5,
        /* .nY = */ 0x4,
        /* .nZ = */ -0x8
    },
    {
        /* .nX = */ 0x5,
        /* .nY = */ 0x2,
        /* .nZ = */ -0x18
    },
    {
        /* .nX = */ 0x5,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x28
    },
    {
        /* .nX = */ 0x0,
        /* .nY = */ 0x5,
        /* .nZ = */ -0x28
    },
    {
        /* .nX = */ 0x2,
        /* .nY = */ 0x5,
        /* .nZ = */ -0x18
    },
    {
        /* .nX = */ 0x4,
        /* .nY = */ 0x5,
        /* .nZ = */ -0x8
    },
    {
        /* .nX = */ 0x5,
        /* .nY = */ 0x4,
        /* .nZ = */ -0x8
    },
    {
        /* .nX = */ 0x5,
        /* .nY = */ 0x2,
        /* .nZ = */ -0x18
    },
    {
        /* .nX = */ 0x5,
        /* .nY = */ 0x0,
        /* .nZ = */ -0x28
    }
}; D2_GLOBAL_GUARD(gaGlideWallCornerTable, "gaGlideWallCornerTable");

GrTexInfo gTexInfoBlocks0Glide = {
    /* .smallLodLog2 = */ 0x8,
    /* .largeLodLog2 = */ 0x8,
    /* .aspectRatioLog2 = */ 0x2,
    /* .format = */ 0xa,
    /* .data = */ nullptr
}; D2_GLOBAL_GUARD(gTexInfoBlocks0Glide, "gTexInfoBlocks0Glide");

GrTexInfo gTexInfoBlocks1Glide = {
    /* .smallLodLog2 = */ 0x8,
    /* .largeLodLog2 = */ 0x8,
    /* .aspectRatioLog2 = */ 0x2,
    /* .format = */ 0xa,
    /* .data = */ nullptr
}; D2_GLOBAL_GUARD(gTexInfoBlocks1Glide, "gTexInfoBlocks1Glide");

GrTexInfo g_GrTexInfoGlBlocks = {
    /* .smallLodLog2 = */ 0x5,
    /* .largeLodLog2 = */ 0x5,
    /* .aspectRatioLog2 = */ 0x0,
    /* .format = */ 0x5,
    /* .data = */ nullptr
}; D2_GLOBAL_GUARD(g_GrTexInfoGlBlocks, "g_GrTexInfoGlBlocks");

GrTexInfo gTexInfoGlideSurfaceBlocks = {
    /* .smallLodLog2 = */ 0x8,
    /* .largeLodLog2 = */ 0x8,
    /* .aspectRatioLog2 = */ 0x1,
    /* .format = */ 0x5,
    /* .data = */ nullptr
}; D2_GLOBAL_GUARD(gTexInfoGlideSurfaceBlocks, "gTexInfoGlideSurfaceBlocks");

GlSubtileCacheStrc gGlideSubtileTextureCache = {}; D2_GLOBAL_GUARD(gGlideSubtileTextureCache, "gGlideSubtileTextureCache");

GlFloorCacheStrc gGlideFloorTextureCache = {}; D2_GLOBAL_GUARD(gGlideFloorTextureCache, "gGlideFloorTextureCache");

uint32_t gGlidePaletteEntry0[256] = {}; D2_GLOBAL_GUARD(gGlidePaletteEntry0, "gGlidePaletteEntry0");

int gnGlideSecondaryTMUCount[2] = {}; D2_GLOBAL_GUARD(gnGlideSecondaryTMUCount, "gnGlideSecondaryTMUCount");

D2GlideVertexTexCacheBlockStrc gGlideVertex0 = {}; D2_GLOBAL_GUARD(gGlideVertex0, "gGlideVertex0");

uint32_t gaGlideColorDebugGradientIndices[1024] = {}; D2_GLOBAL_GUARD(gaGlideColorDebugGradientIndices, "gaGlideColorDebugGradientIndices");

uint32_t gaGlideDebugFontTexStartAddrs[96] = {}; D2_GLOBAL_GUARD(gaGlideDebugFontTexStartAddrs, "gaGlideDebugFontTexStartAddrs");

void * lpBuffer_008802c8[96] = {}; D2_GLOBAL_GUARD(lpBuffer_008802c8, "lpBuffer_008802c8");

D2GlideVertexStrc gaGlidePerspectiveVertices[15] = {}; D2_GLOBAL_GUARD(gaGlidePerspectiveVertices, "gaGlidePerspectiveVertices");

D2GlideVertexStrc gGlidePerspectiveVertexBuffer[4] = {}; D2_GLOBAL_GUARD(gGlidePerspectiveVertexBuffer, "gGlidePerspectiveVertexBuffer");

D2GlideVertexStrc gaGlideGroundTileVertices[36] = {}; D2_GLOBAL_GUARD(gaGlideGroundTileVertices, "gaGlideGroundTileVertices");

void * gapGlideGroundStripVertexPtrs[60] = {}; D2_GLOBAL_GUARD(gapGlideGroundStripVertexPtrs, "gapGlideGroundStripVertexPtrs");

// =============================================================================
// Initialized scalars
// =============================================================================

uint32_t gnGlideTextureMode = -0x1; D2_GLOBAL_GUARD(gnGlideTextureMode, "gnGlideTextureMode");
uint32_t ghGlideSstWindow = 0; D2_GLOBAL_GUARD(ghGlideSstWindow, "ghGlideSstWindow");
int gnGlideTextureMemoryUsed = 0; D2_GLOBAL_GUARD(gnGlideTextureMemoryUsed, "gnGlideTextureMemoryUsed");
uint32_t gbGlideIsDebugBuild = 0; D2_GLOBAL_GUARD(gbGlideIsDebugBuild, "gbGlideIsDebugBuild");
FxI32 gnGlideTextureMemoryMask = 0; D2_GLOBAL_GUARD(gnGlideTextureMemoryMask, "gnGlideTextureMemoryMask");
uint32_t ghGlideWindow = 0; D2_GLOBAL_GUARD(ghGlideWindow, "ghGlideWindow");
uint32_t gbGlideInitialized = 0; D2_GLOBAL_GUARD(gbGlideInitialized, "gbGlideInitialized");
int gbWINDOWED = 0; D2_GLOBAL_GUARD(gbWINDOWED, "gbWINDOWED");
uint32_t gnGlideColorCombineMode = 0; D2_GLOBAL_GUARD(gnGlideColorCombineMode, "gnGlideColorCombineMode");
uint32_t gnGlideAlphaBlendMode = 0; D2_GLOBAL_GUARD(gnGlideAlphaBlendMode, "gnGlideAlphaBlendMode");
uint32_t gnGlideTextureFilteringMode = 0; D2_GLOBAL_GUARD(gnGlideTextureFilteringMode, "gnGlideTextureFilteringMode");
uint32_t gbGlideInScene = 0; D2_GLOBAL_GUARD(gbGlideInScene, "gbGlideInScene");
uint32_t gbGlidePerspectiveMode = 0; D2_GLOBAL_GUARD(gbGlidePerspectiveMode, "gbGlidePerspectiveMode");
uint32_t gnGlideSpriteSecondaryTMUEnabled = 0; D2_GLOBAL_GUARD(gnGlideSpriteSecondaryTMUEnabled, "gnGlideSpriteSecondaryTMUEnabled");
uint32_t gnGlideTMUIndex = 0; D2_GLOBAL_GUARD(gnGlideTMUIndex, "gnGlideTMUIndex");
void * gpGlideSpriteBuffer = 0; D2_GLOBAL_GUARD(gpGlideSpriteBuffer, "gpGlideSpriteBuffer");
void * gpGlideSpritePixelBuffer = 0; D2_GLOBAL_GUARD(gpGlideSpritePixelBuffer, "gpGlideSpritePixelBuffer");
float gfGlideDebugFontVertex0X = 0; D2_GLOBAL_GUARD(gfGlideDebugFontVertex0X, "gfGlideDebugFontVertex0X");
float gfGlideDebugFontVertex0Y = 0; D2_GLOBAL_GUARD(gfGlideDebugFontVertex0Y, "gfGlideDebugFontVertex0Y");
float gfGlideDebugFontVertex1X = 0; D2_GLOBAL_GUARD(gfGlideDebugFontVertex1X, "gfGlideDebugFontVertex1X");
float gfGlideDebugFontVertex1Y = 0; D2_GLOBAL_GUARD(gfGlideDebugFontVertex1Y, "gfGlideDebugFontVertex1Y");
float gfGlideDebugFontVertex3S = 0; D2_GLOBAL_GUARD(gfGlideDebugFontVertex3S, "gfGlideDebugFontVertex3S");
float gfGlideDebugFontVertex2T = 0; D2_GLOBAL_GUARD(gfGlideDebugFontVertex2T, "gfGlideDebugFontVertex2T");
float gfGlideDebugFontVertex3X = 0; D2_GLOBAL_GUARD(gfGlideDebugFontVertex3X, "gfGlideDebugFontVertex3X");
float gfGlideDebugFontVertex3T = 0; D2_GLOBAL_GUARD(gfGlideDebugFontVertex3T, "gfGlideDebugFontVertex3T");
void * gpGlideDebugFontTexture = 0; D2_GLOBAL_GUARD(gpGlideDebugFontTexture, "gpGlideDebugFontTexture");
uint32_t gnGlideDebugFontTMU = 0; D2_GLOBAL_GUARD(gnGlideDebugFontTMU, "gnGlideDebugFontTMU");
int * gpnLightmapRowPointers = 0; D2_GLOBAL_GUARD(gpnLightmapRowPointers, "gpnLightmapRowPointers");
uint8_t gbGlideGlobalLightGreen = 0; D2_GLOBAL_GUARD(gbGlideGlobalLightGreen, "gbGlideGlobalLightGreen");
uint8_t gbGlideGlobalLightBlue = 0; D2_GLOBAL_GUARD(gbGlideGlobalLightBlue, "gbGlideGlobalLightBlue");
uint8_t gbGlideGlobalLightRed = 0; D2_GLOBAL_GUARD(gbGlideGlobalLightRed, "gbGlideGlobalLightRed");
// also labelled: glVertex
uint8_t * gpGlideLightingTable = 0; D2_GLOBAL_GUARD(gpGlideLightingTable, "gpGlideLightingTable");
int gnGlidePerspectiveOriginX = 0; D2_GLOBAL_GUARD(gnGlidePerspectiveOriginX, "gnGlidePerspectiveOriginX");
// also labelled: gpGlideTexturePalette
int32_t gnGlidePerspectiveScale = 0; D2_GLOBAL_GUARD(gnGlidePerspectiveScale, "gnGlidePerspectiveScale");
int gnGlidePerspectiveOriginY = 0; D2_GLOBAL_GUARD(gnGlidePerspectiveOriginY, "gnGlidePerspectiveOriginY");
void * gpPerspectiveLookupTable = 0; D2_GLOBAL_GUARD(gpPerspectiveLookupTable, "gpPerspectiveLookupTable");
uint8_t (*gpGlideSmackerTexBuf0)[32768] = 0; D2_GLOBAL_GUARD(gpGlideSmackerTexBuf0, "gpGlideSmackerTexBuf0");
uint8_t (*gpGlideSmackerTexBuf1)[32768] = 0; D2_GLOBAL_GUARD(gpGlideSmackerTexBuf1, "gpGlideSmackerTexBuf1");
uint32_t gnGlideSmackTex1StartAddr = 0; D2_GLOBAL_GUARD(gnGlideSmackTex1StartAddr, "gnGlideSmackTex1StartAddr");
uint32_t gnGlideSmackTex2StartAddr = 0; D2_GLOBAL_GUARD(gnGlideSmackTex2StartAddr, "gnGlideSmackTex2StartAddr");
uint8_t (*gpGlideSmackerTexBuf2)[32768] = 0; D2_GLOBAL_GUARD(gpGlideSmackerTexBuf2, "gpGlideSmackerTexBuf2");
uint32_t gnGlideTMUHandle = 0; D2_GLOBAL_GUARD(gnGlideTMUHandle, "gnGlideTMUHandle");
uint32_t gnGlideTMUId = 0; D2_GLOBAL_GUARD(gnGlideTMUId, "gnGlideTMUId");
void * gpGlBlocksWallBuffer = 0; D2_GLOBAL_GUARD(gpGlBlocksWallBuffer, "gpGlBlocksWallBuffer");
void * gpGlBlocksFloorBuffer = 0; D2_GLOBAL_GUARD(gpGlBlocksFloorBuffer, "gpGlBlocksFloorBuffer");
uint32_t gnBinkGlideYOffset = 0; D2_GLOBAL_GUARD(gnBinkGlideYOffset, "gnBinkGlideYOffset");
uint32_t gnBinkGlideXOffset = 0; D2_GLOBAL_GUARD(gnBinkGlideXOffset, "gnBinkGlideXOffset");
void * gpBinkVideoGlide = 0; D2_GLOBAL_GUARD(gpBinkVideoGlide, "gpBinkVideoGlide");
uint32_t gnGlideBinkSurfaceType = 0; D2_GLOBAL_GUARD(gnGlideBinkSurfaceType, "gnGlideBinkSurfaceType");
uint32_t g_bGlideBinkVideoQuitRequested = 0; D2_GLOBAL_GUARD(g_bGlideBinkVideoQuitRequested, "g_bGlideBinkVideoQuitRequested");

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
