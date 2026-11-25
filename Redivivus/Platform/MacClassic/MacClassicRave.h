// STUB HEADER - Redivivus-owned, not an Apple header.
//
// The QuickDraw 3D RAVE names the PowerPC D2Rave module uses, written from the public RAVE
// documentation (QuickDraw 3D RAVE programming guide, RAVE 1.6 era) so the module can be
// syntax-checked without the Apple SDK. Nothing here is copied from Apple's RAVE.h.
//
// Scope, from the evidence in d2-surveys/macppc/rave/rave-api.md:
// - the 8 weak CFM imports from "QuickDraw(TM) 3D Accelerator" (identical in every non-Carbon
//   build 1.00-1.13d);
// - TQADrawContext, whose methods D2Rave calls through the context's function pointers; the
//   member order follows the public documentation, and the offsets D2Rave uses are noted;
// - the tag, gestalt, pixel-type and error numbers D2Rave passes. Tags 16, 31, 32 and 46 are
//   NOT verified against the public header and are deliberately left unnamed here: D2Rave spells
//   them as numbers.
//
// Classic Mac PowerPC only. See README.md beside this file.
#pragma once

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

#include <stdint.h>
#include "Redivivus/Platform/MacClassic/MacClassicToolbox.h"

typedef int32_t TQAError;
typedef struct TQAEngine TQAEngine;
typedef struct TQATexture TQATexture;
typedef struct TQAClip TQAClip;
struct TQADrawContext;

enum {
    kQANoErr = 0,
    kQAError = 1,
    kQAOutOfMemory = 2,
    kQANotSupported = 3,
    kQAOutOfDate = 4,
    kQAParamErr = 5,
    kQAGestaltUnknown = 6,
    kQADisplayModeUnsupported = 7,
    kQAOutOfVideoMemory = 8,
};

enum { kQADeviceGDevice = 1 };

struct TQADevice {
    int32_t deviceType;
    GDHandle gDevice;
};

struct TQARect {
    int32_t left;
    int32_t right;
    int32_t top;
    int32_t bottom;
};

struct TQAImage {
    int32_t width;
    int32_t height;
    int32_t rowBytes;
    void * pixmap;
};

// 0x20 bytes: D2Rave's line and rectangle vertices are this far apart.
struct TQAVGouraud {
    float x, y, z, invW;
    float r, g, b, a;
};

// 0x40 bytes: QADrawClippedVTexture walks vertex arrays with this stride.
struct TQAVTexture {
    float x, y, z, invW;
    float r, g, b, a;
    float uOverW, vOverW;
    float kd_r, kd_g, kd_b;
    float ks_r, ks_g, ks_b;
};

enum TQAVertexMode {
    kQAVertexMode_Point = 0,
    kQAVertexMode_Line = 1,
    kQAVertexMode_Polyline = 2,
    kQAVertexMode_Tri = 3,
    kQAVertexMode_Strip = 4,
    kQAVertexMode_Fan = 5,
};

enum {
    kQAPixel_ARGB16 = 2,
};

// Gestalt selectors (the 1.00 engine report prints them in this order).
enum {
    kQAGestalt_OptionalFeatures = 0,
    kQAGestalt_FastFeatures = 1,
    kQAGestalt_VendorID = 2,
    kQAGestalt_EngineID = 3,
    kQAGestalt_Revision = 4,
    kQAGestalt_ASCIINameLength = 5,
    kQAGestalt_ASCIIName = 6,
    kQAGestalt_TextureMemory = 7,
    kQAGestalt_FastTextureMemory = 8,
    kQAGestalt_DrawContextPixelTypesAllowed = 9,
    kQAGestalt_DrawContextPixelTypesPreferred = 10,
    kQAGestalt_TexturePixelTypesAllowed = 11,
    kQAGestalt_TexturePixelTypesPreferred = 12,
    kQAGestalt_BitmapPixelTypesAllowed = 13,
    kQAGestalt_BitmapPixelTypesPreferred = 14,
    kQAGestalt_OptionalFeatures2 = 15,
};

// Draw-context tags whose numbers D2Rave's behaviour confirms.
enum {
    kQATag_ZFunction = 0,
    kQATag_ColorBG_a = 1,
    kQATag_ColorBG_r = 2,
    kQATag_ColorBG_g = 3,
    kQATag_ColorBG_b = 4,
    kQATag_Blend = 9,
    kQATag_PerspectiveZ = 10,
    kQATag_TextureFilter = 11,
    kQATag_TextureOp = 12,
    kQATag_Texture = 13,
    kQATagGL_BlendSrc = 109,
    kQATagGL_BlendDst = 110,
};

// The draw context: a table of methods the engine fills in. Offsets are the ones D2Rave loads.
struct TQADrawContext {
    /* 0x00 */ void * drawPrivate;
    /* 0x04 */ int32_t version;
    /* 0x08 */ void (*setFloat)(TQADrawContext * ctx, int32_t tag, float value);
    /* 0x0c */ void (*setInt)(TQADrawContext * ctx, int32_t tag, uint32_t value);
    /* 0x10 */ void (*setPtr)(TQADrawContext * ctx, int32_t tag, const void * value);
    /* 0x14 */ float (*getFloat)(const TQADrawContext * ctx, int32_t tag);
    /* 0x18 */ uint32_t (*getInt)(const TQADrawContext * ctx, int32_t tag);
    /* 0x1c */ void * (*getPtr)(const TQADrawContext * ctx, int32_t tag);
    /* 0x20 */ void (*drawPoint)(const TQADrawContext * ctx, const TQAVGouraud * v);
    /* 0x24 */ void (*drawLine)(const TQADrawContext * ctx, const TQAVGouraud * v0, const TQAVGouraud * v1);
    /* 0x28 */ void * drawTriGouraud;
    /* 0x2c */ void * drawTriTexture;
    /* 0x30 */ void (*drawVGouraud)(const TQADrawContext * ctx, uint32_t nVertices, uint32_t vertexMode,
                                    const TQAVGouraud * vertices, const uint32_t * flags);
    /* 0x34 */ void (*drawVTexture)(const TQADrawContext * ctx, uint32_t nVertices, uint32_t vertexMode,
                                    const TQAVTexture * vertices, const uint32_t * flags);
    /* 0x38 */ void * drawBitmap;
    /* 0x3c */ void (*renderStart)(const TQADrawContext * ctx, const TQARect * dirtyRect, const TQADrawContext * initialContext);
    /* 0x40 */ TQAError (*renderEnd)(const TQADrawContext * ctx, const TQARect * modifiedRect);
    /* 0x44 */ void * renderAbort;
    /* 0x48 */ void * flush;
    /* 0x4c */ void * sync;
    /* 0x50 */ void * submitVerticesGouraud;
    /* 0x54 */ void * submitVerticesTexture;
    /* 0x58 */ void * drawTriMeshGouraud;
    /* 0x5c */ void * drawTriMeshTexture;
    /* 0x60 */ void * setNoticeMethod;
    /* 0x64 */ void * getNoticeMethod;
    /* 0x68 */ void * submitMultiTextureParams;
    /* 0x6c */ void * accessDrawBuffer;
    /* 0x70 */ void * accessDrawBufferEnd;
    /* 0x74 */ void * accessZBuffer;
    /* 0x78 */ void * accessZBufferEnd;
    /* 0x7c */ TQAError (*clearDrawBuffer)(const TQADrawContext * ctx, const TQARect * rect, const TQADrawContext * initialContext);
    /* 0x80 */ void * clearZBuffer;
    /* 0x84 */ void * textureNewFromDrawContext;
    /* 0x88 */ void * bitmapNewFromDrawContext;
    /* 0x8c */ void * busy;
    /* 0x90 */ TQAError (*swapBuffers)(const TQADrawContext * ctx, const TQARect * dirtyRect);
};

// Weak CFM imports: the address is null when the library did not load.
extern "C" {
TQAEngine * QADeviceGetFirstEngine(const TQADevice * device) __attribute__((weak));
TQAEngine * QADeviceGetNextEngine(const TQADevice * device, const TQAEngine * currentEngine);
TQAError QAEngineGestalt(const TQAEngine * engine, int32_t selector, void * response);
TQAError QADrawContextNew(const TQADevice * device, const TQARect * rect, const TQAClip * clip,
                          const TQAEngine * engine, uint32_t flags, TQADrawContext ** newDrawContext);
void QADrawContextDelete(TQADrawContext * drawContext);
TQAError QATextureNew(const TQAEngine * engine, uint32_t flags, uint32_t pixelType, const TQAImage images[],
                      TQATexture ** newTexture);
TQAError QATextureDetach(const TQAEngine * engine, TQATexture * texture);
void QATextureDelete(const TQAEngine * engine, TQATexture * texture);
}

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
