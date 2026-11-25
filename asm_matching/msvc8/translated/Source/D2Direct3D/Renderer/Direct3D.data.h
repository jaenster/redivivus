/**
 * Data defined in D2Direct3D/Renderer/Direct3D.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Direct3D)
#error "D2Direct3D header included by a module that may not use D2Direct3D (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2Direct3D) && !defined(D2_LOADS_D2Direct3D)
#error "internal D2Direct3D header included from another module; include D2Direct3D's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D3DFloorCacheNodeStrc;
struct D3DFloorCacheStrc;
struct D3DSubtileCacheNodeStrc;
struct D3DSubtileCacheStrc;
struct D3DTextureCacheSetStrc;
struct IDirect3DDevice;
struct IDirectDrawSurface7;

extern unsigned int gnD3DTextureCacheBlockSize;
extern uint16_t g_Palette16Bit[256];
extern D3DSubtileCacheStrc gD3DSubtileTextureCache;
extern D3DFloorCacheStrc gD3DFloorTextureCache;
extern int gnD3DTextureCacheCount;
extern IDirectDrawSurface7 * g_pD3DSurfaceBackbuffer;
extern IDirect3DDevice * g_pD3DDevice;
extern unsigned int g_nBytesPerPixel;
extern uint8_t g_aLightingTable[65536];
extern uint32_t g_aPerspectiveZDivisionTable[8192];
extern GUID D3DGuid;
extern GUID IID_IDirectDraw7;
extern GUID gIID_IDirect3DSurface;
extern unsigned int g_bDeviceInitialized;
extern unsigned int g_bSupportsTextureCachingInternal;
extern unsigned int g_nCurrentAlphaBlendMode;
extern unsigned int g_nCurrentFilterMode;
extern unsigned int g_nCurrentTextureOpMode;
extern IDirect3DInterface7 * g_pDirect3D7;
extern IDirectDraw * g_pDirectDraw7;
extern GUID gsD3DAdapterGuid;
extern uint32_t gsD3DTextureDescTemplate[8];
extern IDirectDrawSurface7 * D3DSurfaceSecondary;
extern GUID IID_IDirect3DHALDevice;
extern uint16_t g_OriginalGammaRamp[3][256];
extern int g_bCutsceneModeActive;
extern unsigned int g_bPerspectiveMode;
extern unsigned int g_nVideoMemoryFree;
extern unsigned int g_nVideoMemoryTotal;
extern IDirect3DViewport3 * g_pD3DViewport;
extern IDirectDrawGammaControl * g_pDDGammaControl;
extern D3DTextureCacheSetStrc gD3DGenericTextureCache;
extern int g_aSmackerSurfaceArray;
extern void * g_pFontTextureHandle[96];
extern void * g_pFontTextureSurface[96];

namespace D2Direct3D { namespace Renderer {
extern unsigned int gnD3DTextureCapFlags;
extern unsigned int g_bCutscenePlaying;
}} // namespace D2Direct3D::Renderer
