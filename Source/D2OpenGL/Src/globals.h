/**
 * Auto-generated global data declarations
 * Binary: Game.exe
 * 
 * This file contains extern declarations for global data symbols
 * that are referenced by multiple functions.
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include "Source/D2Headers/d2_platform.h"
#include "Source/D2OpenGL/Src/placeholder_types.h"
#include "Source/D2OpenGL/Src/MacSdkOnWin32.h"
#include "Source/MacSpecific/Source/CD2Textures/CD2Textures.h"
#include "Source/D2OpenGL/Src/COGLAGPTextures.h"
#include "Source/D2OpenGL/Src/COGLTextures.h"
#include "Source/D2OpenGL/Src/oglCore.h"

// Forward declarations
struct CD2Textures;
struct CD2TexturesVtable;
struct COGLAGPTextures;
struct COGLAGPTexturesVtable;
struct COGLTextures;
struct COGLTexturesVtable;
struct D2GfxHelperStrc;
struct D2GfxSettingsStrc;
struct D2OGLColorRGBStrc;
struct D2OGLCutsceneCapStrc;
struct D2OGLDisplayModeStrc;
struct D2OGLVertexStrc;
struct D2OGLWallCornerStrc;
struct D2TextureNodeStrc;
struct D2TileLibraryEntryStrc;
typedef bool (*IsTextureBound_fn)(::CD2Textures * pThis, D2TextureNodeStrc * pNode);
typedef bool (*UploadTexture_fn)(::CD2Textures * pThis, D2TextureNodeStrc * pNode, void * pPixels);
typedef int (*CreateTexture_fn)(::CD2Textures * pThis, void * pPixels, int nWidth, int nHeight, int nOwnerType, void * pOwner, int nOwnerIndex, intptr_t nOwnerKey);
typedef int (*GetFreeMemory_fn)(::CD2Textures * pThis, BYTE bExcludeReserve);
typedef void (*BindTexture_fn)(::CD2Textures * pThis, D2TextureNodeStrc * pNode);
typedef void (*CD2Textures_DeletingDtor_fn)(::CD2Textures * pThis);
typedef void (*CD2Textures_Dtor_fn)(::CD2Textures * pThis);
typedef void (*COGLAGPTextures_DeletingDtor_fn)(::COGLAGPTextures * pThis);
typedef void (*COGLAGPTextures_Dtor_fn)(::COGLAGPTextures * pThis);
typedef void (*COGLTextures_DeletingDtor_fn)(::COGLTextures * pThis);
typedef void (*COGLTextures_Dtor_fn)(::COGLTextures * pThis);
typedef void (*ConvertPixels_fn)(::CD2Textures * pThis, int nTexWidth, int nTexHeight, int nSrcWidth, int nSrcHeight, BYTE * pSrc);
typedef void (__fastcall *D2OGLTileFreeCallbackFn)(D2TileLibraryEntryStrc * pTile);
typedef void (*EndFrame_fn)(::CD2Textures * pThis);
typedef void (*ReleaseTexture_fn)(::CD2Textures * pThis, D2TextureNodeStrc * pNode);

// Full type definitions (used by value in globals)
D2_PACKED_BEGIN struct __attribute__((packed)) D2OGLColorRGBStrc {
    /* 0x00 */ BYTE nRed;
    /* 0x01 */ BYTE nGreen;
    /* 0x02 */ BYTE nBlue;
}; D2_PACKED_END

// =============================================================================
// Global data (referenced by multiple functions)
// =============================================================================

extern uint32_t DAT_00326eac;
extern uint32_t DAT_0032718c;
extern uint32_t DAT_00327b60;
extern uint32_t DAT_00327b64;
extern float gfConst128;
extern uint32_t DAT_00334b68;
extern uint32_t DAT_00334b6c;
extern uint64_t DAT_00334b80;
extern uint32_t DAT_00334bd0;
extern uint32_t DAT_00334bd4;
extern uint32_t DAT_00334bd8;
extern float FLOAT_ARRAY_00334be0[2];
extern int INT_ARRAY_ARRAY_00334be8[2][10];
extern int INT_ARRAY_ARRAY_00334be8_0__1_[2][10];
extern int INT_ARRAY_ARRAY_00334be8_1__0_[2][10];
extern int INT_ARRAY_ARRAY_00334be8_1__1_[2][10];
extern int INT_ARRAY_00334c40[6];
extern int INT_ARRAY_00334c58[6];
extern int INT_ARRAY_00334c70[6];
extern uint64_t DAT_00335278;
extern D2OGLCutsceneCapStrc D2OGLCutsceneCapStrc_ARRAY_003ce7d0[3];
extern uint32_t DAT_003ce7e8;
extern uint32_t DAT_003ce7ec;
extern uint32_t DAT_003ce7f0;
extern uint32_t DAT_003ce7f4;
extern uint32_t DAT_003ce7f8;
extern D2OGLWallCornerStrc D2OGLWallCornerStrc_ARRAY_ARRAY_003ce7fc[10][6];
extern CD2TexturesVtable CD2TexturesVtable_003e26d8;
extern COGLTexturesVtable COGLTexturesVtable_003e2878;
extern COGLAGPTexturesVtable COGLAGPTexturesVtable_003e28b8;
extern uint32_t DAT_0055256c;
extern uint32_t DAT_00552570;
extern uint32_t DAT_00552574;
extern uint32_t DAT_00552578;
extern uint32_t DAT_0055257c;
extern uint8_t * DAT_00552590;
extern uint8_t * DAT_00552594;
extern BYTE * PTR_00552598;
extern uint8_t DAT_0055259c;
extern uint32_t DAT_005525a0;
extern uint32_t DAT_005525a4;
extern uint8_t DAT_005526cc;
extern uint8_t DAT_00552b20;
extern uint32_t DAT_00552b24;
extern CGDisplayModeRef CGDisplayModeRef_00552b28;
extern CGDirectDisplayID CGDirectDisplayID_00552b2c;
extern WindowRef WindowRef_00552b30;
extern AGLContext AGLContext_00552b34;
extern short SHORT_ARRAY_00552b38[2];
extern uint8_t DAT_00552b3c;
extern uint8_t * DAT_00552b40;
extern D2OGLDisplayModeStrc D2OGLDisplayModeStrc_ARRAY_00552b50[3];
extern int * PTR_00552b70;
extern int DAT_00552b74;
extern int DAT_00552b78;
extern BYTE * PTR_00552b7c;
extern uint32_t * PTR_00552b80;
extern uint32_t * PTR_00552b84;
extern uint32_t DAT_00552b88;
extern uint32_t DAT_00552b8c;
extern uint8_t * DAT_00552bc0;
extern uint8_t * DAT_00552bc4;
extern uint8_t * DAT_00552bc8;
extern D2OGLVertexStrc D2OGLVertexStrc_ARRAY_00552bcc[4];
extern D2OGLColorRGBStrc D2OGLColorRGBStrc_00552c2c;
extern uint8_t DAT_00552c30;
extern D2OGLVertexStrc D2OGLVertexStrc_ARRAY_00552c34[4];
extern D2OGLVertexStrc D2OGLVertexStrc_ARRAY_ARRAY_00552c94[5][3];
extern BOOL BOOL_005c8a64;
extern BOOL BOOL_005c8a68;
extern D2GfxSettingsStrc * PTR_005e9e14;
extern BYTE * * PTR_005e9e20;
extern int INT_005e9e24;
extern int INT_005e9e28;
extern BOOL BOOL_005e9e2c;
extern BOOL BOOL_005e9e30;
extern CD2Textures * PTR_005e9edc;
extern uint32_t UINT_ARRAY_005e9ee0[256];
extern uint16_t USHORT_ARRAY_005ea2e0[256];
extern int INT_005eabe0;
extern int INT_005eabe4;
extern BYTE * PTR_005eabe8;
extern void* PTR_PTR_003961e0;
extern void* PTR_LAB_003961ec;
extern void* PTR_MACCURSOR_ShowDisplayCursor_003961f0;
extern void* PTR_LAB_003961f4;
extern void* PTR____stack_chk_guard_003961fc;
extern uint16_t (*PTR_USHORT_ARRAY_0039636c)[256];
extern uint32_t (*PTR_UINT_ARRAY_00396370)[256];
extern BINKSNDSYSOPEN * PTR_BINK_OpenSoundManager_00396374;
extern BOOL * PTR_BOOL_00396380;
extern D2GfxSettingsStrc * * PTR_PTR_00396384;
extern BOOL * PTR_BOOL_00396390;
extern BOOL * PTR_BOOL_0039639c;
extern BYTE * * * PTR_PTR_003963a0;
extern int * PTR_INT_003963a4;
extern int * PTR_INT_003963a8;
extern int * PTR_INT_003963ac;
extern CD2Textures * * PTR_PTR_003963b0;
