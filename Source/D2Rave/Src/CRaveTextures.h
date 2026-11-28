// CRaveTextures - the RAVE texture cache, a subclass of the shared CD2Textures manager.
//
// The class name is attested by its Metrowerks RTTI string "CRaveTextures" (10353a2e); the base
// "CD2Textures" is attested by its RTTI string (10353a3c) and by the base file's __FILE__
// "CD2Textures.cpp". The file name CRaveTextures.cpp/.h is inferred.
//
// CD2Textures is declared here in its PPC form (a C++ class with virtuals, Metrowerks vtable:
// RTTI, this-offset, then the virtuals). The tree reconstructs the same class from the 1.14d
// Intel Mac in Source/MacSpecific/Source/CD2Textures/CD2Textures.h, which does not compile on a
// Mac target yet; the member and method names below are that header's, so the two can merge.
#pragma once

#if !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC

#include <stdint.h>
#include "Redivivus/Platform/MacClassic/MacClassicRave.h"

// One cache entry, 0x30 bytes (CD2Textures allocates 0x800 of them).
struct D2TextureNodeStrc {
    /* 0x00 */ int nIndex;              // 1-based
    /* 0x04 */ TQATexture * pTexture;
    /* 0x08 */ int nUsedThisFrame;
    /* 0x0c */ int nUsedLastFrame;
    /* 0x10 */ int nWidth;
    /* 0x14 */ int nHeight;
    /* 0x18 */ int unknown18[4];
    /* 0x28 */ D2TextureNodeStrc * pNext;
    /* 0x2c */ D2TextureNodeStrc * pPrev;
};

// The base, PPC layout (0x5c bytes). Only what CRaveTextures needs is declared.
class CD2Textures {
public:
    CD2Textures(int nMaxWidth, int nMaxHeight, int nTextureOverhead, int nMemoryBudget);  // 102ce120
    virtual ~CD2Textures();                                                              // 102cdf2c
    virtual int GetFreeMemory(bool bExcludeReserve) = 0;
    virtual bool UploadTexture(D2TextureNodeStrc * pNode, void * pPixels) = 0;
    virtual void ReleaseTexture(D2TextureNodeStrc * pNode) = 0;
    virtual void BindTexture(D2TextureNodeStrc * pNode) = 0;
    virtual bool IsTextureBound(D2TextureNodeStrc * pNode) = 0;
    virtual void ConvertPixels(int nTexWidth, int nTexHeight, int nSrcWidth, int nSrcHeight, uint8_t * pSrc) = 0;

    void Init(int nBytesPerPixel);  // 102ce028 (CD2TEXTURES_Init)
    void ReleaseAll();              // 102cdfc0 (CD2TEXTURES_ReleaseAll)

    /* 0x04 */ int field_0x4;
    /* 0x08 */ int nReserveBytes;
    /* 0x0c */ int nMaxWidth;
    /* 0x10 */ int nMaxHeight;
    /* 0x14 */ int nTextureOverhead;
    /* 0x18 */ int nMemoryBudget;
    /* 0x1c */ int nBytesPerPixel;
    /* 0x20 */ int field_0x20[6];
    /* 0x38 */ void * pStagingAlloc;
    /* 0x3c */ uint8_t * pStaging;      // 32-byte aligned, rows of 0x200 bytes
    /* 0x40 */ int nUsedBytes;
    /* 0x44 */ D2TextureNodeStrc * pNodes;
    /* 0x48 */ int nNodeCount;
    /* 0x4c */ D2TextureNodeStrc * pUsedHead;
    /* 0x50 */ D2TextureNodeStrc * pUsedTail;
    /* 0x54 */ D2TextureNodeStrc * pFreeHead;
    /* 0x58 */ D2TextureNodeStrc * pFreeTail;
};

// Vtable at 103f2c44.
class CRaveTextures : public CD2Textures {
public:
    CRaveTextures();
    ~CRaveTextures() override;
    int GetFreeMemory(bool bExcludeReserve) override;
    bool UploadTexture(D2TextureNodeStrc * pNode, void * pPixels) override;
    void ReleaseTexture(D2TextureNodeStrc * pNode) override;
    void BindTexture(D2TextureNodeStrc * pNode) override;
    bool IsTextureBound(D2TextureNodeStrc * pNode) override;
    void ConvertPixels(int nTexWidth, int nTexHeight, int nSrcWidth, int nSrcHeight, uint8_t * pSrc) override;
};

#endif // !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC
