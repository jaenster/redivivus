/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"
#include "D2Direct3D/Renderer/Direct3D.h"
#include "D2Direct3D/d3dFloorCache.h"
#include "D2Direct3D/d3dSubtileCache.h"


// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

GUID gIID_IDirect3DSurface = {
    /* .Data1 = */ 0x93281502,
    /* .Data2 = */ 0x8cf8,
    /* .Data3 = */ 0x11d0,
    /* .Data4 = */ { 0x89, 0xab, '\x00', 0xa0, 0xc9, '\x05', 'A', ')' }
}; D2_GLOBAL_GUARD(gIID_IDirect3DSurface, "gIID_IDirect3DSurface");

GUID IID_IDirect3DHALDevice = {
    /* .Data1 = */ 0x84e63de0,
    /* .Data2 = */ 0x46aa,
    /* .Data3 = */ 0x11cf,
    /* .Data4 = */ { 0x81, 'o', '\x00', '\x00', 0xc0, ' ', '\x15', 'n' }
}; D2_GLOBAL_GUARD(IID_IDirect3DHALDevice, "IID_IDirect3DHALDevice");

GUID D3DGuid = {
    /* .Data1 = */ 0xbb223240,
    /* .Data2 = */ 0xe72b,
    /* .Data3 = */ 0x11d0,
    /* .Data4 = */ { 0xa9, 0xb4, '\x00', 0xaa, '\x00', 0xc0, 0x99, '>' }
}; D2_GLOBAL_GUARD(D3DGuid, "D3DGuid");

GUID IID_IDirectDraw7 = {
    /* .Data1 = */ 0x9c59509a,
    /* .Data2 = */ 0x39bd,
    /* .Data3 = */ 0x11d1,
    /* .Data4 = */ { 0x8c, 'J', '\x00', 0xc0, 'O', 0xd9, '0', 0xc5 }
}; D2_GLOBAL_GUARD(IID_IDirectDraw7, "IID_IDirectDraw7");

uint32_t gsD3DTextureDescTemplate[8] = { 0x20, 0x41, 0x0, 0x10, 0x7c00, 0x3e0, 0x1f, 0x8000 }; D2_GLOBAL_GUARD(gsD3DTextureDescTemplate, "gsD3DTextureDescTemplate");

uint16_t gaD3DPerspectiveStripIndices[22] = {
    0x0,
    0x1,
    0x3,
    0x4,
    0x6,
    0x7,
    0x9,
    0xa,
    0xc,
    0xd,
    0xd,
    0x1,
    0x1,
    0x2,
    0x4,
    0x5,
    0x7,
    0x8,
    0xa,
    0xb,
    0xd,
    0xe
}; D2_GLOBAL_GUARD(gaD3DPerspectiveStripIndices, "gaD3DPerspectiveStripIndices");

D2D3DTileCornerStrc gaD3DWallCornerTable[60] = {
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
}; D2_GLOBAL_GUARD(gaD3DWallCornerTable, "gaD3DWallCornerTable");

GUID gsD3DAdapterGuid = {}; D2_GLOBAL_GUARD(gsD3DAdapterGuid, "gsD3DAdapterGuid");

D2D3DDeviceInfoStrc gdwD3DDeviceFlags[4] = {}; D2_GLOBAL_GUARD(gdwD3DDeviceFlags, "gdwD3DDeviceFlags");

D3DSubtileCacheStrc gD3DSubtileTextureCache = {}; D2_GLOBAL_GUARD(gD3DSubtileTextureCache, "gD3DSubtileTextureCache");

D3DFloorCacheStrc gD3DFloorTextureCache = {}; D2_GLOBAL_GUARD(gD3DFloorTextureCache, "gD3DFloorTextureCache");

uint16_t g_OriginalGammaRamp[3][256] = {}; D2_GLOBAL_GUARD(g_OriginalGammaRamp, "g_OriginalGammaRamp");

D3DTextureCacheSetStrc gD3DGenericTextureCache = {}; D2_GLOBAL_GUARD(gD3DGenericTextureCache, "gD3DGenericTextureCache");

void * g_pFontTextureBuffer[96] = {}; D2_GLOBAL_GUARD(g_pFontTextureBuffer, "g_pFontTextureBuffer");

void * g_pFontTextureSurface[96] = {}; D2_GLOBAL_GUARD(g_pFontTextureSurface, "g_pFontTextureSurface");

void * g_pFontTextureHandle[96] = {}; D2_GLOBAL_GUARD(g_pFontTextureHandle, "g_pFontTextureHandle");

D2D3DVertexStrc gaD3DPerspectiveStripVertices[15] = {}; D2_GLOBAL_GUARD(gaD3DPerspectiveStripVertices, "gaD3DPerspectiveStripVertices");

D2D3DVertexStrc gaD3DPerspectiveVertices[4] = {}; D2_GLOBAL_GUARD(gaD3DPerspectiveVertices, "gaD3DPerspectiveVertices");

uint8_t g_aLightingTable[65536] = {}; D2_GLOBAL_GUARD(g_aLightingTable, "g_aLightingTable");

uint32_t g_aPerspectiveZDivisionTable[8192] = {}; D2_GLOBAL_GUARD(g_aPerspectiveZDivisionTable, "g_aPerspectiveZDivisionTable");

IDirectDrawSurface7 * gpD3DSmackerSystemSurface0[2] = {}; D2_GLOBAL_GUARD(gpD3DSmackerSystemSurface0, "gpD3DSmackerSystemSurface0");

IDirect3DTexture2 * gpD3DSmackerSystemTexture0[2] = {}; D2_GLOBAL_GUARD(gpD3DSmackerSystemTexture0, "gpD3DSmackerSystemTexture0");

IDirectDrawSurface7 * gpD3DSmackerVideoSurface0[2] = {}; D2_GLOBAL_GUARD(gpD3DSmackerVideoSurface0, "gpD3DSmackerVideoSurface0");

IDirect3DTexture2 * gpD3DSmackerVideoTexture0[2] = {}; D2_GLOBAL_GUARD(gpD3DSmackerVideoTexture0, "gpD3DSmackerVideoTexture0");

// =============================================================================
// Initialized scalars
// =============================================================================

unsigned int g_nVideoMemoryTotal = {}; D2_GLOBAL_GUARD(g_nVideoMemoryTotal, "g_nVideoMemoryTotal");
unsigned int g_nVideoMemoryFree = {}; D2_GLOBAL_GUARD(g_nVideoMemoryFree, "g_nVideoMemoryFree");
unsigned int gnD3DTextureCacheBlockSize = {}; D2_GLOBAL_GUARD(gnD3DTextureCacheBlockSize, "gnD3DTextureCacheBlockSize");
int gnD3DTextureCacheCount = 0; D2_GLOBAL_GUARD(gnD3DTextureCacheCount, "gnD3DTextureCacheCount");
unsigned int g_bDeviceInitialized = {}; D2_GLOBAL_GUARD(g_bDeviceInitialized, "g_bDeviceInitialized");
int g_bCutsceneModeActive = 0; D2_GLOBAL_GUARD(g_bCutsceneModeActive, "g_bCutsceneModeActive");
// also labelled: g_pD3DPerspectiveSurface
BOOL g_bD3DCoreInitialized = 0; D2_GLOBAL_GUARD(g_bD3DCoreInitialized, "g_bD3DCoreInitialized");
unsigned int g_bSupportsTextureCachingInternal = {}; D2_GLOBAL_GUARD(g_bSupportsTextureCachingInternal, "g_bSupportsTextureCachingInternal");
unsigned int g_nCurrentTextureOpMode = {}; D2_GLOBAL_GUARD(g_nCurrentTextureOpMode, "g_nCurrentTextureOpMode");
unsigned int g_nCurrentAlphaBlendMode = {}; D2_GLOBAL_GUARD(g_nCurrentAlphaBlendMode, "g_nCurrentAlphaBlendMode");
unsigned int g_nCurrentFilterMode = {}; D2_GLOBAL_GUARD(g_nCurrentFilterMode, "g_nCurrentFilterMode");
IDirectDraw * g_pDirectDraw7 = 0; D2_GLOBAL_GUARD(g_pDirectDraw7, "g_pDirectDraw7");
IDirect3DInterface7 * g_pDirect3D7 = 0; D2_GLOBAL_GUARD(g_pDirect3D7, "g_pDirect3D7");
IDirectDrawSurface7 * D3DSurfaceSecondary = 0; D2_GLOBAL_GUARD(D3DSurfaceSecondary, "D3DSurfaceSecondary");
IDirectDrawGammaControl * g_pDDGammaControl = 0; D2_GLOBAL_GUARD(g_pDDGammaControl, "g_pDDGammaControl");
IDirect3DDevice * g_pD3DDevice = 0; D2_GLOBAL_GUARD(g_pD3DDevice, "g_pD3DDevice");
IDirect3DViewport3 * g_pD3DViewport = 0; D2_GLOBAL_GUARD(g_pD3DViewport, "g_pD3DViewport");
unsigned int g_nBytesPerPixel = {}; D2_GLOBAL_GUARD(g_nBytesPerPixel, "g_nBytesPerPixel");
HWND ghD3DEnumWindowHandle = 0; D2_GLOBAL_GUARD(ghD3DEnumWindowHandle, "ghD3DEnumWindowHandle");
int gnDirect3DDeviceCount = 0; D2_GLOBAL_GUARD(gnDirect3DDeviceCount, "gnDirect3DDeviceCount");
unsigned int g_bPerspectiveMode = {}; D2_GLOBAL_GUARD(g_bPerspectiveMode, "g_bPerspectiveMode");
void * gpD3DTextureSpritePoolMemory0 = 0; D2_GLOBAL_GUARD(gpD3DTextureSpritePoolMemory0, "gpD3DTextureSpritePoolMemory0");
void * gpD3DTextureSpritePoolMemory1 = 0; D2_GLOBAL_GUARD(gpD3DTextureSpritePoolMemory1, "gpD3DTextureSpritePoolMemory1");
IDirect3DVertexBuffer * g_pD3DSpriteQuadVB = 0; D2_GLOBAL_GUARD(g_pD3DSpriteQuadVB, "g_pD3DSpriteQuadVB");
int gd3dVertexBufferOffset = 0; D2_GLOBAL_GUARD(gd3dVertexBufferOffset, "gd3dVertexBufferOffset");
int gD3DFontVertex0 = 0; D2_GLOBAL_GUARD(gD3DFontVertex0, "gD3DFontVertex0");
float gfD3DFontVertex0X = 0; D2_GLOBAL_GUARD(gfD3DFontVertex0X, "gfD3DFontVertex0X");
float gfD3DFontVertex0Z = 0; D2_GLOBAL_GUARD(gfD3DFontVertex0Z, "gfD3DFontVertex0Z");
float gfD3DFontVertex0Y = 0; D2_GLOBAL_GUARD(gfD3DFontVertex0Y, "gfD3DFontVertex0Y");
float gfD3DFontVertex2Y = 0; D2_GLOBAL_GUARD(gfD3DFontVertex2Y, "gfD3DFontVertex2Y");
float gfD3DFontVertex1Y = 0; D2_GLOBAL_GUARD(gfD3DFontVertex1Y, "gfD3DFontVertex1Y");
float gfD3DFontVertex2X = 0; D2_GLOBAL_GUARD(gfD3DFontVertex2X, "gfD3DFontVertex2X");
float gfD3DFontVertex3X = 0; D2_GLOBAL_GUARD(gfD3DFontVertex3X, "gfD3DFontVertex3X");
IDirect3DVertexBuffer * g_pD3DGroundTileVB = 0; D2_GLOBAL_GUARD(g_pD3DGroundTileVB, "g_pD3DGroundTileVB");
uint8_t GlobalGreen = 0; D2_GLOBAL_GUARD(GlobalGreen, "GlobalGreen");
uint8_t GlobalBlue = 0; D2_GLOBAL_GUARD(GlobalBlue, "GlobalBlue");
uint8_t GlobalRed = 0; D2_GLOBAL_GUARD(GlobalRed, "GlobalRed");
int gnPerspectiveScaleY = 0; D2_GLOBAL_GUARD(gnPerspectiveScaleY, "gnPerspectiveScaleY");
int gnPerspectiveScaleZ = 0; D2_GLOBAL_GUARD(gnPerspectiveScaleZ, "gnPerspectiveScaleZ");
uint32_t g_aSmackerSurfaceArray = 0; D2_GLOBAL_GUARD(g_aSmackerSurfaceArray, "g_aSmackerSurfaceArray");
void * pSmackerDecodeBuffer = 0; D2_GLOBAL_GUARD(pSmackerDecodeBuffer, "pSmackerDecodeBuffer");
IDirectDrawSurface7 * gpD3DFloorTileSystemSurface = 0; D2_GLOBAL_GUARD(gpD3DFloorTileSystemSurface, "gpD3DFloorTileSystemSurface");
IDirect3DTexture2 * gpD3DFloorTileMemoryTexture = 0; D2_GLOBAL_GUARD(gpD3DFloorTileMemoryTexture, "gpD3DFloorTileMemoryTexture");
IDirectDrawSurface7 * gpD3DSubtileSystemSurface = 0; D2_GLOBAL_GUARD(gpD3DSubtileSystemSurface, "gpD3DSubtileSystemSurface");
IDirect3DTexture2 * gpD3DSubtileMemoryTexture = 0; D2_GLOBAL_GUARD(gpD3DSubtileMemoryTexture, "gpD3DSubtileMemoryTexture");

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
