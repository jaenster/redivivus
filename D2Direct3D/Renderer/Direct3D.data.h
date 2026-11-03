/**
 * Data defined in D2Direct3D/Renderer/Direct3D.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "globals/globals.h"
#include "D2Direct3D/Renderer/Direct3D.h"
#include "D2Direct3D/d3dFloorCache.h"
#include "D2Direct3D/d3dSubtileCache.h"
#include "D2Direct3D/d3dTextureCache.h"

// Forward declarations
struct D2D3DDeviceSlotStrc;
struct D3DFloorCacheNodeStrc;
struct D3DFloorCacheStrc;
struct D3DSubtileCacheNodeStrc;
struct D3DSubtileCacheStrc;
struct D3DTextureCacheNodeStrc;
struct D3DTextureCacheSetStrc;
struct IDirect3DDevice;
struct IDirect3DInterface7;
struct IDirect3DTexture2;
struct IDirect3DVertexBuffer;
struct IDirect3DViewport3;
struct IDirectDraw;
struct IDirectDrawGammaControl;
struct IDirectDrawSurface7;

extern GUID D3DGuid;
extern GUID IID_IDirectDraw7;
extern BOOL BOOL_0074c020;
extern GUID gsD3DAdapterGuid;
extern unsigned int gnD3DTextureCacheBlockSize;
extern uint16_t g_Palette16Bit[256];
extern D3DSubtileCacheStrc gD3DSubtileTextureCache;
extern D3DFloorCacheStrc gD3DFloorTextureCache;
extern int gnD3DTextureCacheCount;
extern uint32_t g_Palette32Bit[256];
extern D2D3DDeviceSlotStrc gD3DDeviceBlock0[2];
extern unsigned int g_bDeviceInitialized;
extern BOOL g_bD3DCoreInitialized;
// @data g_nCurrentTextureOpMode
// @wasnamed g_nCurrentBlendMode
// @evidence Caches SetTextureStageState COLOROP/ALPHAOP and ALPHATESTENABLE at
// 006b1ea0. @wasnamed g_nCurrentBlendMode, shared with 00970e58 which caches a different state group.
extern unsigned int g_nCurrentTextureOpMode;
// @data g_nCurrentAlphaBlendMode
// @wasnamed g_nCurrentBlendMode
// @evidence Caches ALPHABLENDENABLE / SRCBLEND / DESTBLEND at 006b1f40.
extern unsigned int g_nCurrentAlphaBlendMode;
extern unsigned int g_nCurrentFilterMode;
extern IDirectDraw * g_pDirectDraw7;
extern IDirect3DInterface7 * g_pDirect3D7;
extern IDirectDrawSurface7 * g_pD3DSurfaceBackbuffer;
extern IDirectDrawSurface7 * D3DSurfaceSecondary;
extern IDirectDrawGammaControl * g_pDDGammaControl;
extern IDirect3DDevice * g_pD3DDevice;
extern IDirect3DViewport3 * g_pD3DViewport;
extern unsigned int g_nBytesPerPixel;
extern D3DTextureCacheSetStrc gD3DGenericTextureCache;
extern IDirect3DVertexBuffer * g_pD3DSpriteQuadVB;
extern int gd3dVertexBufferOffset;
extern uint16_t gpD3DGroundTileVertexBuffer[65];
extern IDirect3DVertexBuffer * g_pD3DGroundTileVB;
extern uint8_t GlobalGreen;
extern uint8_t GlobalBlue;
extern uint8_t GlobalRed;
extern uint8_t g_aLightingTable[65536];
extern int gnPerspectiveScaleY;
extern int gnPerspectiveScaleZ;
extern uint32_t g_aPerspectiveZDivisionTable[8192];
extern uint32_t g_aSmackerSurfaceArray;
extern void * pSmackerDecodeBuffer;

namespace D2Direct3D::Renderer {
extern unsigned int gnD3DTextureCapFlags;
extern unsigned int g_bCutscenePlaying;
} // namespace D2Direct3D::Renderer
