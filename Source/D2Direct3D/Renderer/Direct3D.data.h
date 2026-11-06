/**
 * Data defined in D2Direct3D/Renderer/Direct3D.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "D2Direct3D/d3dFloorCache.h"
#include "D2Direct3D/d3dSubtileCache.h"

// Forward declarations
struct D3DFloorCacheNodeStrc;
struct D3DFloorCacheStrc;
struct D3DSubtileCacheNodeStrc;
struct D3DSubtileCacheStrc;
struct IDirect3DDevice;
struct IDirectDrawSurface7;

extern unsigned int gnD3DTextureCacheBlockSize;
extern uint16_t g_Palette16Bit[256];
extern D3DSubtileCacheStrc gD3DSubtileTextureCache;
extern D3DFloorCacheStrc gD3DFloorTextureCache;
extern int gnD3DTextureCacheCount;
extern uint32_t g_Palette32Bit[256];
extern IDirectDrawSurface7 * g_pD3DSurfaceBackbuffer;
extern IDirect3DDevice * g_pD3DDevice;
extern unsigned int g_nBytesPerPixel;
extern unsigned int g_bPerspectiveMode;
extern uint8_t g_aLightingTable[65536];
extern uint32_t g_aPerspectiveZDivisionTable[8192];

namespace D2Direct3D::Renderer {
extern unsigned int gnD3DTextureCapFlags;
extern unsigned int g_bCutscenePlaying;
} // namespace D2Direct3D::Renderer
