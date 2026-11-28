// raveCore.cpp - the RAVE renderer's core: detection, init and release, the DrawSprocket
// context, surfaces, scene start/end, palette, gamma, cutscene window and the engine pick.
//
// File name attested: the PPC build's __FILE__ in sRaveEngineReport's allocation is
// "raveCore.cpp" (string 10353844). The unit runs 102b1d1c-102b479c and ends with
// __sinit_raveCore_cpp at 102b478c. Log strings are quoted verbatim from 1.09d; the line numbers
// passed to SMemAlloc/SMemFree are the original source lines.

#include "Source/D2Rave/Src/raveInternal.h"
#include "Source/D2Rave/Src/CRaveTextures.h"

#if !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC

namespace D2Rave {

namespace {
const char sgszRaveCoreFile[] = "raveCore.cpp";

// OSStatus DSpContext_SetState may return when activating; D2Rave treats it as success.
const OSStatus kRaveSetStateAccepted = -30448;

// Display modes by resolution mode (0 = 640x480, 1 = 800x600, 2 = 800x600 again).
int gnRaveModeWidth[4] = {640, 800, 800, 0};   // 103f2798
int gnRaveModeHeight[4] = {480, 600, 600, 0};  // 103f27a8
int gnRaveModeFlags[4] = {1, 0, 1, 0};         // 103f27b8, copied to gbRaveModeFlag

// The zeroed DSpContextAttributes templates sInitRaveDrawSprocket and iRaveSwapContext copy.
const DSpContextAttributes sgRaveDSpAttributesInit = {};  // 103f27d8
const DSpContextAttributes sgRaveDSpAttributesSwap = {};  // 103f2820

const char * const sgszRaveErrorNames[9] = {
    nullptr, "kQAError", "kQAOutOfMemory", "kQANotSupported", "kQAOutOfDate", "kQAParamErr",
    "kQAGestaltUnknown", "kQADisplayModeUnsupported", "kQAOutOfVideoMemory",
};

const char * const sgszRaveVendorNames[] = {
    "\tVendor: Apple", "\tVendor: ATI", "\tVendor: Radius", "\tVendor: DesignMark", "\tVendor: Matrox",
    "\tVendor: Yarc", "\tVendor: DiamondMM", "\tVendor: 3DLabs", "\tVendor: D3DAdaptor", "\tVendor: IXMicro",
    "\tVendor: NumberNine", "\tVendor: MicroConversions", "\tVendor: PurpleShark", "\tVendor: VillageTronic",
};

int snRaveTextureFilter;  // 1037c74c
int snRaveBlendMode;      // 1037c750
int snRaveAlphaState;     // 1037c754
Boolean sbRaveCloseAllInit;  // 1037c758
Boolean sbRaveCloseAllDone;  // 1037c759
Point sgRaveOriginZero;      // 1037c744 = {0, 0}
const float sgfRaveColorOne = 1.0f;   // 1037c788
const float sgfRaveZero = 0.0f;       // 1037c79c
const float sgfRaveClipInset = 0.5f;  // 1037c798
uint8_t * sgRavePaletteTableCopy[0x48];  // 1052efd8
static_assert(sizeof(void*) != 4 || sizeof(sgRavePaletteTableCopy) == 0x120, "RaveSetPaletteTable copies the 0x120-byte 32-bit table");
} // namespace

TQADrawContext * gpRaveDrawContext;
TQAEngine * gpRaveEngine;
char gbRaveInDraw;
uint32_t gnRaveBinkSurfaceType;
int gbRavePaused;
int gnRaveScreenHeight;
int gnRaveScreenWidth;
char gbRaveModeFlag;
uint8_t ** gpRavePaletteTable;
void * gpRaveHelpers;
void * gpRaveSettings;
int gbRaveFastTexture;
CRaveTextures * gpRaveTextures;
int gnRaveGamma = 155;
int gnRaveRequestedDepth = 16;
int gnRaveRequestedEngineID = -1;
float gRaveClipRect[4] = {0.5f, 0.5f, 479.5f, 639.5f};
uint8_t gRavePaletteRGB[256][3];
uint8_t gRavePaletteRGBA[256][4];
uint32_t gdwRavePaletteARGB32[256];
uint16_t gwRavePaletteARGB1555[256];
Boolean gbRaveSwapBuffers;
Boolean gbRaveContextOpen;
WindowPtr gpRaveWindow;
int gnRaveDisplayDepth;
DSpContextReference ghRaveDSpContext;
Point gRaveWindowOrigin;
GDHandle ghRaveDevice;
int gbRaveCutscenePlaying;
int gnRaveGammaCurve = 1;         // 1037c740
char gbRaveTextureLimit;          // TOC slot 10375994
char gbRaveWindowNeedsUpdate;     // TOC slot 10375968
RaveDisplayMode gRaveDisplayModes[3];  // 1052efb0, filled through SetOption 2 and 3

namespace {

// Describes the device's current pixel map for sRaveFillDisplay. Inlined at each call site in
// the binary (RaveChangeRes, RaveCutsceneCreateWindow).
inline void sRaveDescribeDevice(RaveFrameBuffer * pBuffer) {
    PixMap * pPixMap = *(*ghRaveDevice)->gdPMap;
    pBuffer->pBase = (uint8_t *)pPixMap->baseAddr;
    pBuffer->nWidth = pPixMap->bounds.right - pPixMap->bounds.left;
    pBuffer->nHeight = pPixMap->bounds.bottom - pPixMap->bounds.top;
    pBuffer->nBytesPerPixel = pPixMap->pixelSize >> 3;
    pBuffer->nRowBytes = pPixMap->rowBytes & 0x3fff;
}

// Deactivates and releases the DrawSprocket context. The log strings name it
// sCloseRaveDrawSprocket; the binary inlines it into RavePauseSurface, RaveCloseSurface,
// RaveRelease and sRaveCloseAll, each followed by the same two outcome lines.
inline bool sCloseRaveDrawSprocket() {
    bool bClosed;
    if (ghRaveDSpContext == nullptr) {
        bClosed = true;
    } else if (OSStatus nError = DSpContext_SetState(ghRaveDSpContext, kDSpContextState_Inactive)) {
        LOGFILE_WriteStringToFile("sCloseRaveDrawSprocket: *** DSpContext_SetState failed.");
        (void)nError;
        bClosed = false;
    } else if ((nError = DSpContext_Release(ghRaveDSpContext)) != 0) {
        LOGFILE_WriteStringToFile("sCloseRaveDrawSprocket: *** DSpContext_Release failed. (%d)", nError);
        bClosed = false;
    } else {
        gnRaveDisplayDepth = 0;
        ghRaveDSpContext = nullptr;
        bClosed = true;
    }
    if (bClosed) {
        LOGFILE_WriteStringToFile("RaveCloseSurface: Closed RAVE context.");
    } else {
        LOGFILE_WriteStringToFile("RaveCloseSurface: *** Closing DrawSprocket failed.");
    }
    return bClosed;
}

inline void sRavePaintWindow() {
    if (gpRaveWindow != nullptr) {
        SetPort(gpRaveWindow);
        PaintRect(&gpRaveWindow->portRect);
    }
}

} // namespace

// Windows slot 12. Options 0-11; 6-8 are not handled.
// mac ppc classic/1.09d: 102b1d1c (1.00: 10077900)
int RaveSetOption(unsigned int nOption, intptr_t nValue) {
    switch (nOption) {
    case 0:  // set the display device
        ghRaveDevice = (GDHandle)nValue;
        return 1;
    case 1:  // get the display device
        *(GDHandle *)nValue = ghRaveDevice;
        return 1;
    case 2:  // the 640x480 DrawSprocket mode
        gRaveDisplayModes[0] = *(const RaveDisplayMode *)nValue;
        return 1;
    case 3:  // the 800x600 DrawSprocket mode, used by modes 1 and 2
        gRaveDisplayModes[2] = *(const RaveDisplayMode *)nValue;
        gRaveDisplayModes[1] = gRaveDisplayModes[2];
        return 1;
    case 4:
        gnRaveRequestedEngineID = nValue;
        return 1;
    case 5:  // millions of colours
        gnRaveRequestedDepth = (nValue & 0xff) ? 32 : 16;
        return 1;
    case 9:
        gbRaveTextureLimit = (char)*(int *)nValue;
        CD2TEXTURES_Flush(gpRaveTextures);
        return 1;
    case 10:
        *(int *)nValue = gbRaveInDraw != 0;
        return 1;
    case 11:
        gnRaveGammaCurve = nValue;
        return 1;
    default:
        return 0;
    }
}

// Mac-only slot 14 (Windows slot 11, ActivateWindow, as D2gfx passes it). RAVE only runs full
// screen.
// mac ppc classic/1.09d: 102b1e40 (1.00: 10077ad4)
int RaveSetFullScreen(int bFullScreen) {
    if (bFullScreen == 0) {
        LOGFILE_WriteStringToFile("RaveSetFullScreen: *** Attempted to set RAVE to windowed mode.");
    }
    return bFullScreen != 0;
}

// Mac-only slot 13; asm only in the database.
// mac ppc classic/1.09d: 102b1e7c (1.00: 10077b10)
int RaveLocalToGlobal(Point * pPoint) {
    if (ghRaveDSpContext == nullptr || gbRavePaused != 0) {
        return 0;
    }
    pPoint->h += gRaveWindowOrigin.h;
    pPoint->v += gRaveWindowOrigin.v;
    return 1;
}

// Mac-only slot 12; asm only in the database.
// mac ppc classic/1.09d: 102b1ecc (1.00: 10077b60)
int RaveGlobalToLocal(Point * pPoint) {
    if (ghRaveDSpContext == nullptr || gbRavePaused != 0) {
        return 0;
    }
    pPoint->h -= gRaveWindowOrigin.h;
    pPoint->v -= gRaveWindowOrigin.v;
    return 1;
}

// Mac-only slot 11: hands the event to DrawSprocket and keeps the mouse inside the game
// window. The scout map called this slot ActivateWindow; it takes an EventRecord.
// mac ppc classic/1.09d: 102b1f1c (1.00: 10077bb0)
Boolean RaveProcessEvent(EventRecord * pEvent) {
    Boolean bHandled = false;
    if (ghRaveDSpContext != nullptr) {
        DSpProcessEvent(pEvent, &bHandled);
        if (gbRavePaused == 0 && pEvent->what < 9) {
            Point where = pEvent->where;
            if (where.h < gRaveWindowOrigin.h) {
                where.h = gRaveWindowOrigin.h;
            }
            if (where.v < gRaveWindowOrigin.v) {
                where.v = gRaveWindowOrigin.v;
            }
            int nMax = (int16_t)(gRaveWindowOrigin.h + (int16_t)gnRaveScreenWidth) - 1;
            if (nMax < where.h) {
                where.h = (int16_t)nMax;
            }
            nMax = (int16_t)(gRaveWindowOrigin.v + (int16_t)gnRaveScreenHeight) - 1;
            if (nMax < where.v) {
                where.v = (int16_t)nMax;
            }
            if (where.h != pEvent->where.h || where.v != pEvent->where.v) {
                SetMousePosition(where);
                pEvent->where = where;
            }
        }
    }
    return bHandled;
}

// asm only in the database.
// mac ppc classic/1.09d: 102b2068 (1.00: 10077cfc)
int * RaveGetRenderStatistics() {
    return nullptr;
}

// mac ppc classic/1.09d: 102b2070 (1.00: 10077d04)
void sRaveSetTextureFilter(int nFilter) {
    if (snRaveTextureFilter == 0 || nFilter != snRaveTextureFilter) {
        if (nFilter == 2) {
            gpRaveDrawContext->setInt(gpRaveDrawContext, kQATag_TextureFilter, 2);
        } else if (nFilter == 1) {
            gpRaveDrawContext->setInt(gpRaveDrawContext, kQATag_TextureFilter, 0);
        }
        snRaveTextureFilter = nFilter;
    }
}

// 1 opaque; 2 additive (ONE, ONE); 3 modulate (ZERO, SRC_COLOR); 4 alpha (SRC_ALPHA,
// ONE_MINUS_SRC_ALPHA). The GL blend factors go through RAVE's GL tags.
// mac ppc classic/1.09d: 102b2100 (1.00: 10077d94)
void sRaveSetBlendMode(int nMode) {
    if (snRaveBlendMode == 0 || nMode != snRaveBlendMode) {
        TQADrawContext * pContext = gpRaveDrawContext;
        if (nMode == 3) {
            pContext->setInt(pContext, kQATag_Blend, 2);
            pContext->setInt(pContext, kQATagGL_BlendSrc, 0);
            pContext->setInt(pContext, kQATagGL_BlendDst, 0x300);
        } else if (nMode < 3) {
            if (nMode == 1) {
                pContext->setInt(pContext, kQATag_Blend, 0);
            } else if (nMode > 0) {
                pContext->setInt(pContext, kQATag_Blend, 2);
                pContext->setInt(pContext, kQATagGL_BlendSrc, 1);
                pContext->setInt(pContext, kQATagGL_BlendDst, 1);
            }
        } else if (nMode < 5) {
            pContext->setInt(pContext, kQATag_Blend, 2);
            pContext->setInt(pContext, kQATagGL_BlendSrc, 0x302);
            pContext->setInt(pContext, kQATagGL_BlendDst, 0x303);
        }
        snRaveBlendMode = nMode;
    }
}

// Tags 31 and 46 behave like an alpha-test function and reference; not verified, so numbers.
// mac ppc classic/1.09d: 102b2270 (1.00: 10077f04)
void sRaveSetAlphaState(int nState) {
    if (snRaveAlphaState == 0 || nState != snRaveAlphaState) {
        TQADrawContext * pContext = gpRaveDrawContext;
        if (nState == 2) {
            pContext->setInt(pContext, kQATag_AlphaTestFunc, kQAAlphaTest_None);
        } else if (nState == 1) {
            pContext->setInt(pContext, kQATag_AlphaTestFunc, kQAAlphaTest_NE);
            pContext->setFloat(pContext, kQATag_AlphaTestRef, sgfRaveZero);
        }
        snRaveAlphaState = nState;
    }
}

// Windows slot 9 (ResizeWindow): switch resolution, blacking the screen around the swap.
// mac ppc classic/1.09d: 102b2320 (1.00: 10077fb4)
int RaveChangeRes(int nUnused, int nMode) {
    (void)nUnused;
    RaveFrameBuffer buffer;
    sCloseRaveContext();
    LOGFILE_WriteStringToFile("RaveChangeRes: Switching to %s.", nMode ? "800x600" : "640x480");
    int nDepth = gnRaveRequestedDepth;
    sRaveDescribeDevice(&buffer);
    sRaveFillDisplay(&buffer, 0, 0, 0, 0);
    DSpContext_FadeGamma(nullptr, 0, nullptr);
    int16_t nError = (int16_t)iRaveSwapContext(nMode, nDepth);
    if (nError != 0) {
        LOGFILE_WriteStringToFile("RaveSwapContext: *** iRaveSwapContext failed. (%d)", (int)nError);
    }
    sRaveDescribeDevice(&buffer);
    sRaveFillDisplay(&buffer, 0, 0, 0, 0);
    DSpContext_FadeGamma(nullptr, 100, nullptr);
    sInitRaveContext(nMode);
    return 1;
}

// mac ppc classic/1.09d: 102b24b0 (1.00: 10078144)
void RaveSetPaletteTable(uint8_t ** pTable) {
    CopyMemory(sgRavePaletteTableCopy, pTable, sizeof(sgRavePaletteTableCopy));
    gpRavePaletteTable = sgRavePaletteTableCopy;
}

// mac ppc classic/1.09d: 102b24f4 (1.00: 10078188)
uint32_t sRaveGetPaletteARGB32(int nIndex) {
    return gdwRavePaletteARGB32[nIndex];
}

// Builds the four palette forms from 256 PALETTEENTRY records (r, g, b, flags). Black is the
// transparent key: it becomes 0 in the ARGB forms and gets alpha 0 in the RGBA form.
// asm only in the database.
// mac ppc classic/1.09d: 102b2504 (1.00: 10078198)
void RaveSetPalette(const uint8_t * pEntries) {
    for (int i = 0; i < 0x100; ++i, pEntries += 4) {
        gRavePaletteRGB[i][0] = pEntries[0];
        gRavePaletteRGB[i][1] = pEntries[1];
        gRavePaletteRGB[i][2] = pEntries[2];
        gRavePaletteRGBA[i][0] = pEntries[0];
        gRavePaletteRGBA[i][1] = pEntries[1];
        gRavePaletteRGBA[i][2] = pEntries[2];
        gdwRavePaletteARGB32[i] = 0xff000000u | (pEntries[0] << 16) | (pEntries[1] << 8) | pEntries[2];
        gwRavePaletteARGB1555[i] =
            (uint16_t)(0x8000 | ((pEntries[0] >> 3) << 10) | ((pEntries[1] >> 3) << 5) | (pEntries[2] >> 3));
        if (pEntries[0] == 0 && pEntries[1] == 0 && pEntries[2] == 0) {
            gdwRavePaletteARGB32[i] = 0;
            gwRavePaletteARGB1555[i] = 0;
            gRavePaletteRGBA[i][3] = 0;
        }
    }
}

// asm only in the database.
// mac ppc classic/1.09d: 102b25ec (1.00: 10078280)
void RaveGetScreenSize(int * pWidth, int * pHeight) {
    *pWidth = gnRaveScreenWidth;
    *pHeight = gnRaveScreenHeight;
}

// Windows slot 10 (GetBackBuffer): copies the DrawSprocket front buffer out as packed RGB24,
// from an 8-, 16- or 32-bit display.
// mac ppc classic/1.09d: 102b2608 (1.00: 1007829c)
int RaveScreenGrab(uint8_t * pDest) {
    CGrafPtr pFront;
    if (DSpContext_GetFrontBuffer(ghRaveDSpContext, &pFront) != 0) {
        return 0;
    }
    int bGrabbed = 0;
    PixMap * pPixMap = *pFront->portPixMap;
    const unsigned int nWidth = gnRaveScreenWidth;
    const unsigned int nDepth = pPixMap->pixelSize;
    const unsigned int nHeight = gnRaveScreenHeight;
    const uint8_t * pSrc = (const uint8_t *)pPixMap->baseAddr;
    const int nSkip = (pPixMap->rowBytes & 0x3fff) - nWidth * (nDepth >> 3);
    if (nDepth == 8) {
        const ColorSpec * pColors = (*pPixMap->pmTable)->ctTable;
        for (unsigned int y = 0; y < nHeight; ++y) {
            for (unsigned int x = 0; x < nWidth; ++x) {
                const RGBColor & rgb = pColors[*pSrc++].rgb;
                pDest[0] = (uint8_t)(rgb.red >> 8);
                pDest[1] = (uint8_t)(rgb.green >> 8);
                pDest[2] = (uint8_t)(rgb.blue >> 8);
                pDest += 3;
            }
            pSrc += nSkip;
        }
        bGrabbed = 1;
    } else if (nDepth == 16) {
        for (unsigned int y = 0; y < nHeight; ++y) {
            for (unsigned int x = 0; x < nWidth; ++x) {
                const uint16_t wPixel = *(const uint16_t *)pSrc;
                pSrc += 2;
                pDest[0] = (uint8_t)(wPixel >> 7) & 0xf8;
                pDest[1] = (uint8_t)(wPixel >> 2) & 0xf8;
                pDest[2] = (uint8_t)(wPixel << 3);
                pDest += 3;
            }
            pSrc += nSkip;
        }
        bGrabbed = 1;
    } else if (nDepth == 32) {
        for (unsigned int y = 0; y < nHeight; ++y) {
            for (unsigned int x = 0; x < nWidth; ++x) {
                const uint32_t dwPixel = *(const uint32_t *)pSrc;
                pSrc += 4;
                pDest[0] = (uint8_t)(dwPixel >> 16);
                pDest[1] = (uint8_t)(dwPixel >> 8);
                pDest[2] = (uint8_t)dwPixel;
                pDest += 3;
            }
            pSrc += nSkip;
        }
        bGrabbed = 1;
    }
    return bGrabbed;
}

// Windows slot 8 (EndScene2).
// mac ppc classic/1.09d: 102b2bcc (1.00: 10078500)
int RaveBlit() {
    EndFrame(gpRaveTextures);
    if (gbRaveSwapBuffers) {
        RaveCheckError(gpRaveDrawContext->swapBuffers(gpRaveDrawContext, nullptr));
    }
    return 1;
}

// asm only in the database.
// mac ppc classic/1.09d: 102b2c24 (1.00: 10078590)
int RaveCheckGamma() {
    return 1;
}

// mac ppc classic/1.09d: 102b2c2c (1.00: 10078598)
void RaveSetGamma(int nGamma) {
    if (nGamma > 0xff) {
        nGamma = 0xff;
    }
    if (nGamma < 0x37) {
        nGamma = 0x37;
    }
    gnRaveGamma = nGamma;
    SetGammaCurve(ghRaveDevice, nGamma, gnRaveGammaCurve);
}

// asm only in the database.
// mac ppc classic/1.09d: 102b2c7c (1.00: 100785e4)
void RaveUpdateScaleFactor(int nScaleFactor) {
    (void)nScaleFactor;
}

// Windows slot 7 (EndScene1).
// mac ppc classic/1.09d: 102b2c80 (1.00: 100785f0)
int RaveEndDraw() {
    if (gbRaveInDraw == 0) {
        LOGFILE_WriteStringToFile("RaveEndDraw: *** Overlapping calls to EndDraw.");
    } else {
        gbRaveInDraw = 0;
        RaveCheckError(gpRaveDrawContext->renderEnd(gpRaveDrawContext, nullptr));
    }
    return 1;
}

// Windows slot 6 (BeginScene). Resets the global light to white for the frame.
// mac ppc classic/1.09d: 102b2ce4 (1.00: 10078664)
int RaveStartDraw() {
    if (gbRavePaused != 0) {
        return 0;
    }
    if (gbRaveInDraw == 0) {
        gpRaveDrawContext->renderStart(gpRaveDrawContext, nullptr, nullptr);
        RaveSetGlobalLight(0xff, 0xff, 0xff);
        gbRaveInDraw = 1;
    } else {
        LOGFILE_WriteStringToFile("RaveStartDraw: *** Overlapping calls to StartDraw.");
    }
    return 1;
}

// asm only in the database.
// mac ppc classic/1.09d: 102b2d7c (1.00: 100786fc)
int RaveCheckCutscene() {
    return gbRaveCutscenePlaying;
}

// Windows slot 14 (PlayCutScene): plays the movie centred in the 640x480 front buffer.
// mac ppc classic/1.09d: 102b2d84 (1.00: 10078704)
void RavePlayCutscene(const char * szFile, int nMode, void (*pfnFrame)()) {
    RaveCutsceneCreateWindow();
    SetGammaCurve(ghRaveDevice, 100, gnRaveGammaCurve);
    if (gbRavePaused != 0) {
        return;
    }
    sRavePaintWindow();
    gbRaveCutscenePlaying = 1;
    CGrafPtr pFront;
    DSpContext_GetFrontBuffer(ghRaveDSpContext, &pFront);
    uint8_t * pBase = (uint8_t *)GetPixBaseAddr(pFront->portPixMap);
    GDevice * pDevice = *ghRaveDevice;
    const int nExtraX = (pDevice->gdRect.right - pDevice->gdRect.left) - 640;
    const int nExtraY = (pDevice->gdRect.bottom - pDevice->gdRect.top) - 480;
    const int nPixelSize = (*pFront->portPixMap)->pixelSize;
    const int nRowBytes = (*pFront->portPixMap)->rowBytes & 0x3fff;
    gnRaveBinkSurfaceType = BinkGDSurfaceType(ghRaveDevice);
    RavePlayBinkFile(szFile, nMode, pBase + (nExtraY / 2) * nRowBytes + (nExtraX / 2) * (nPixelSize / 8), nRowBytes,
                     pfnFrame);
    gbRaveCutscenePlaying = 0;
    sRavePaintWindow();
}

// Windows slot 13 (BeginCutScene): movies need a 640x480x32 context.
// mac ppc classic/1.09d: 102b2eec (1.00: 10078868)
int RaveCutsceneCreateWindow() {
    if (gbRavePaused != 0) {
        return 0;
    }
    if (gnRaveDisplayDepth != 32 || gnRaveScreenWidth != 640 || gnRaveScreenHeight != 480) {
        RaveFrameBuffer buffer;
        sCloseRaveContext();
        sRaveDescribeDevice(&buffer);
        sRaveFillDisplay(&buffer, 0, 0, 0, 0);
        DSpContext_FadeGamma(nullptr, 0, nullptr);
        int16_t nError = (int16_t)iRaveSwapContext(0, 32);
        if (nError != 0) {
            LOGFILE_WriteStringToFile("RaveSwapContext: *** iRaveSwapContext failed. (%d)", (int)nError);
        }
        sRaveDescribeDevice(&buffer);
        sRaveFillDisplay(&buffer, 0, 0, 0, 0);
        DSpContext_FadeGamma(nullptr, 100, nullptr);
        if (nError != 0) {
            LOGFILE_WriteStringToFile("RaveCutsceneCreateWindow: *** RaveSwapContext failed.");
        }
        sInitRaveContext(0);
    }
    return 1;
}

// Replaces the DrawSprocket context with one for nMode at nDepth.
// mac ppc classic/1.09d: 102b30a8 (1.00: 10078a24)
int iRaveSwapContext(int nMode, int nDepth) {
    DSpContextAttributes attributes = sgRaveDSpAttributesSwap;
    int nError = (int16_t)DSpContext_SetState(ghRaveDSpContext, kDSpContextState_Inactive);
    if (nError != 0) {
        LOGFILE_WriteStringToFile("iRaveSwapContext: *** DSpContext_SetState failed.");
        return nError;
    }
    nError = (int16_t)DSpContext_Release(ghRaveDSpContext);
    if (nError != 0) {
        LOGFILE_WriteStringToFile("iRaveSwapContext: *** DSpContext_Release failed.");
        return nError;
    }
    ghRaveDSpContext = nullptr;
    const RaveDisplayMode & mode = gRaveDisplayModes[nMode];
    nError = (int16_t)DSpFindContextEx(ghRaveDevice, mode.nWidth, mode.nHeight, mode.nRefresh, nDepth,
                                       &ghRaveDSpContext, &attributes);
    if (nError != 0) {
        LOGFILE_WriteStringToFile("iRaveSwapContext: *** DSpFindContextEx failed.");
        return nError;
    }
    nError = (int16_t)DSpContext_Reserve(ghRaveDSpContext, &attributes);
    if (nError != 0) {
        LOGFILE_WriteStringToFile("iRaveSwapContext: *** DSpContext_Reserve failed.");
        return nError;
    }
    nError = (int16_t)DSpContext_SetState(ghRaveDSpContext, kDSpContextState_Active);
    if (nError == kRaveSetStateAccepted || nError == 0) {
        gnRaveDisplayDepth = nDepth;
        nError = 0;
    } else {
        LOGFILE_WriteStringToFile("iRaveSwapContext: *** DSpContext_SetState failed.");
    }
    return nError;
}

// Windows slot 5 (EndCutScene as 1.14d labels it): suspend (bPause) or resume the renderer.
// The resume path repeats RaveCreateSurface, the suspend path RaveCloseSurface.
// mac ppc classic/1.09d: 102b3224 (1.00: 10078ba0)
void RavePauseSurface(void * hWindow, int nMode, int bPause) {
    (void)hWindow;
    while (gbRaveInDraw != 0) {
        YieldToThinThread(0);
    }
    if (bPause == 0) {
        if (!sInitRaveDrawSprocket(nMode)) {
            LOGFILE_WriteStringToFile("RaveCreateSurface: *** Initializing DrawSprocket failed.");
        } else if (!sInitRaveContext(nMode)) {
            LOGFILE_WriteStringToFile("RaveCreateSurface: *** Initializing %dx%dx%d RAVE context failed.",
                                      gnRaveScreenWidth, gnRaveScreenHeight, gnRaveRequestedDepth);
        } else {
            LOGFILE_WriteStringToFile("RaveCreateSurface: Created RAVE context at %dx%dx%d.", gnRaveScreenWidth,
                                      gnRaveScreenHeight, gnRaveRequestedDepth);
        }
        gbRavePaused = 0;
        return;
    }
    if (!sCloseRaveContext()) {
        LOGFILE_WriteStringToFile("RaveCloseSurface: *** Closing RAVE context failed.");
    } else {
        SetMouseInputVideoMode(1);
        sCloseRaveDrawSprocket();
    }
    gbRavePaused = 1;
}

// Windows slot 53.
// mac ppc classic/1.09d: 102b33c0 (1.00: 10078d30)
void RaveClearCaches() {
    if (gpRaveTextures != nullptr) {
        delete gpRaveTextures;
        gpRaveTextures = nullptr;
    }
}

// Windows slot 4 (DestroyWindow).
// mac ppc classic/1.09d: 102b3408 (1.00: 10078d78)
int RaveCloseSurface() {
    if (!sCloseRaveContext()) {
        LOGFILE_WriteStringToFile("RaveCloseSurface: *** Closing RAVE context failed.");
        return 0;
    }
    SetMouseInputVideoMode(1);
    return sCloseRaveDrawSprocket();
}

// Windows slot 3 (CreateWindow).
// mac ppc classic/1.09d: 102b34f0 (1.00: 10078e54)
int RaveCreateSurface(void * hWindow, int nMode) {
    (void)hWindow;
    if (!sInitRaveDrawSprocket(nMode)) {
        LOGFILE_WriteStringToFile("RaveCreateSurface: *** Initializing DrawSprocket failed.");
        return 0;
    }
    if (!sInitRaveContext(nMode)) {
        LOGFILE_WriteStringToFile("RaveCreateSurface: *** Initializing %dx%dx%d RAVE context failed.",
                                  gnRaveScreenWidth, gnRaveScreenHeight, gnRaveRequestedDepth);
        return 0;
    }
    LOGFILE_WriteStringToFile("RaveCreateSurface: Created RAVE context at %dx%dx%d.", gnRaveScreenWidth,
                              gnRaveScreenHeight, gnRaveRequestedDepth);
    return 1;
}

// Windows slot 2 (Release).
// mac ppc classic/1.09d: 102b35ac (1.00: 10078f10)
int RaveRelease() {
    if (!sCloseRaveContext()) {
        LOGFILE_WriteStringToFile("RaveCloseSurface: *** Closing RAVE context failed.");
    } else {
        SetMouseInputVideoMode(1);
        sCloseRaveDrawSprocket();
    }
    sRaveBlocksFree();
    sRavePerspectiveFree();
    sRaveVertexFree();
    return 1;
}

// Fog's exit callback (TVector 103845e8, registered by RaveInit): RaveRelease, once.
// mac ppc classic/1.09d: 102b36a4 (1.00: 10078ffc)
void sRaveCloseAll() {
    if (!sbRaveCloseAllInit) {
        sbRaveCloseAllDone = false;
        sbRaveCloseAllInit = true;
    }
    if (sbRaveCloseAllDone) {
        return;
    }
    if (!sCloseRaveContext()) {
        LOGFILE_WriteStringToFile("RaveCloseSurface: *** Closing RAVE context failed.");
    } else {
        SetMouseInputVideoMode(1);
        sCloseRaveDrawSprocket();
    }
    sRaveBlocksFree();
    sRavePerspectiveFree();
    sRaveVertexFree();
    sbRaveCloseAllDone = true;
}

// Windows slot 1 (InitPerspective as 1.14d labels it; D2MOO's pfInit).
// mac ppc classic/1.09d: 102b37c8 (1.00: 10079114)
int RaveInit(void * pSettings, void * pHelpers) {
    gpRaveSettings = pSettings;
    gpRaveHelpers = pHelpers;
    LOGFILE_WriteStringToFile("RaveInit: Initializing for RAVE.");
    sRaveBlocksInit();
    sRavePerspectiveAlloc();
    sRaveVertexAlloc();
    FOG_RegisterExitCallback(sRaveCloseAll);
    sRaveVertexInit();
    sRavePerspectiveInit();
    sRaveWallInit();
    sRaveFloorInit();
    return 1;
}

// Opens the centred game window, picks an engine (the requested ID, else the first usable
// one), creates the draw context and sets the initial draw state.
// mac ppc classic/1.09d: 102b3848 (1.00: 10079194)
int sInitRaveContext(int nMode) {
    gbRaveModeFlag = (char)gnRaveModeFlags[nMode];
    gnRaveScreenWidth = gnRaveModeWidth[nMode];
    gnRaveScreenHeight = gnRaveModeHeight[nMode];

    Point origin = sgRaveOriginZero;
    DSpContext_LocalToGlobal(ghRaveDSpContext, &origin);
    gRaveWindowOrigin = origin;

    Rect bounds;
    GDevice * pDevice = *ghRaveDevice;
    bounds.top = pDevice->gdRect.top;
    bounds.top += (int16_t)((pDevice->gdRect.bottom - pDevice->gdRect.top - gnRaveScreenHeight) / 2);
    pDevice = *ghRaveDevice;
    bounds.left = pDevice->gdRect.left;
    bounds.left += (int16_t)((pDevice->gdRect.right - pDevice->gdRect.left - gnRaveScreenWidth) / 2);
    bounds.bottom = bounds.top + (int16_t)gnRaveScreenHeight;
    bounds.right = bounds.left + (int16_t)gnRaveScreenWidth;
    static const unsigned char sgszEmptyTitle[] = {0};
    gpRaveWindow = NewCWindow(nullptr, &bounds, sgszEmptyTitle, true, 0x410, (WindowPtr)-1, false, 0);
    BringToFront(gpRaveWindow);
    RaveHostWindowCreated();
    gbRaveWindowNeedsUpdate = 0;
    sRavePaintWindow();
    SetGammaCurve(ghRaveDevice, gnRaveGamma, gnRaveGammaCurve);

    TQADevice device;
    device.deviceType = kQADeviceGDevice;
    device.gDevice = ghRaveDevice;
    for (;;) {
        gpRaveEngine = QADeviceGetFirstEngine(&device);
        while (gpRaveEngine != nullptr) {
            if (gnRaveRequestedEngineID == -1) {
                if (sRaveEngineReport(gpRaveEngine, 0)) {
                    gbRaveContextOpen = true;
                    break;
                }
            } else {
                int32_t nEngineID;
                QAEngineGestalt(gpRaveEngine, kQAGestalt_EngineID, &nEngineID);
                if (nEngineID == gnRaveRequestedEngineID) {
                    gbRaveContextOpen = true;
                    break;
                }
            }
            gpRaveEngine = QADeviceGetNextEngine(&device, gpRaveEngine);
        }
        if (gbRaveContextOpen || gnRaveRequestedEngineID == -1) {
            break;
        }
        gnRaveRequestedEngineID = -1;
        LOGFILE_WriteStringToFile("sInitRaveContext: *** Requested engine not found. Will try any available engine.");
    }

    gbRaveContextOpen = (Boolean)sRaveEngineReport(gpRaveEngine, 1);
    if (!gbRaveContextOpen) {
        LOGFILE_WriteStringToFile("sInitRaveContext: *** No usable RAVE engine found.");
        sCloseRaveContext();
        return 0;
    }

    GDevice * pGD = *ghRaveDevice;
    TQARect rect;
    rect.left = bounds.left - pGD->gdRect.left;
    rect.right = bounds.right - pGD->gdRect.left;
    rect.top = bounds.top - pGD->gdRect.top;
    rect.bottom = bounds.bottom - pGD->gdRect.top;
    gRaveClipRect[0] = sgfRaveClipInset;
    gRaveClipRect[1] = sgfRaveClipInset;
    gRaveClipRect[2] = (float)((double)gnRaveScreenHeight - 0.5);
    gRaveClipRect[3] = (float)((double)gnRaveScreenWidth - 0.5);
    RaveCheckError(QADrawContextNew(&device, &rect, nullptr, gpRaveEngine, 0x15, &gpRaveDrawContext));
    sRavePaintWindow();

    gpRaveTextures = new CRaveTextures();
    if (gbRaveModeFlag == 0) {
        sRaveSmackInit();
    }
    D2CMP_SetTileFreeCallback(sRaveFreeTileTextures);

    TQADrawContext * pContext = gpRaveDrawContext;
    pContext->setInt(pContext, kQATag_ZFunction, 0);
    pContext->setInt(pContext, kQATag_PerspectiveZ, 0);
    pContext->setInt(pContext, kQATag_BufferComposite, kQABufferComposite_None);
    pContext->setFloat(pContext, kQATag_ColorBG_a, sgfRaveColorOne);
    pContext->setFloat(pContext, kQATag_ColorBG_r, sgfRaveZero);
    pContext->setFloat(pContext, kQATag_ColorBG_g, sgfRaveZero);
    pContext->setFloat(pContext, kQATag_ColorBG_b, sgfRaveZero);
    pContext->setInt(pContext, kQATag_AlphaTestFunc, kQAAlphaTest_NE);
    pContext->setFloat(pContext, kQATag_AlphaTestRef, sgfRaveZero);
    pContext->setInt(pContext, kQATag_TextureOp, 1);
    if (gbRaveSwapBuffers) {
        pContext->setInt(pContext, kQATag_DontSwap, 1);
    }
    sRaveSpriteInit();

    int nGamma = *(int *)((uint8_t *)gpRaveSettings + 0xc);
    if (nGamma > 0xff) {
        nGamma = 0xff;
    }
    if (nGamma < 0x37) {
        nGamma = 0x37;
    }
    gnRaveGamma = nGamma;
    SetGammaCurve(ghRaveDevice, nGamma, gnRaveGammaCurve);
    if (gbRaveModeFlag != 0) {
        sRaveHookStub1();
        sRaveHookStub3();
    }
    return 1;
}

// mac ppc classic/1.09d: 102b3d98 (1.00: 100796d8)
int sCloseRaveContext() {
    if (gbRaveContextOpen) {
        sRavePaintWindow();
        RestoreGammaCurve(ghRaveDevice);
        if (gpRaveTextures != nullptr) {
            delete gpRaveTextures;
            gpRaveTextures = nullptr;
        }
        if (gbRaveModeFlag == 0) {
            sRaveSmackFree();
        } else {
            sRaveHookStub2();
            sRaveHookStub0();
        }
        sRaveSpriteFree();
        D2CMP_SetTileFreeCallback(nullptr);
        if (gpRaveDrawContext != nullptr) {
            QADrawContextDelete(gpRaveDrawContext);
            gpRaveDrawContext = nullptr;
        }
        if (gpRaveWindow != nullptr) {
            DisposeWindow(gpRaveWindow);
            gpRaveWindow = nullptr;
        }
        gbRaveContextOpen = false;
    }
    return 1;
}

// mac ppc classic/1.09d: 102b3eac (1.00: 10079818)
int sInitRaveDrawSprocket(int nMode) {
    DSpContextAttributes attributes = sgRaveDSpAttributesInit;
    const RaveDisplayMode & mode = gRaveDisplayModes[nMode];
    OSStatus nError = (int16_t)DSpFindContextEx(ghRaveDevice, mode.nWidth, mode.nHeight, mode.nRefresh,
                                                gnRaveRequestedDepth, &ghRaveDSpContext, &attributes);
    if (nError != 0) {
        LOGFILE_WriteStringToFile("sInitRaveDrawSprocket: *** DSpFindContextEx (%s) failed. (%d)",
                                  nMode ? "800x600" : "640x480", nError);
        return 0;
    }
    nError = (int16_t)DSpContext_Reserve(ghRaveDSpContext, &attributes);
    if (nError != 0) {
        LOGFILE_WriteStringToFile("sInitRaveDrawSprocket: *** DSpContext_Reserve failed. (%d)", nError);
        return 0;
    }
    nError = (int16_t)DSpContext_SetState(ghRaveDSpContext, kDSpContextState_Active);
    if (nError != 0 && nError != kRaveSetStateAccepted) {
        LOGFILE_WriteStringToFile("sInitRaveDrawSprocket: *** DSpContext_SetState failed. (%d)", nError);
        return 0;
    }
    gnRaveDisplayDepth = gnRaveRequestedDepth;
    SetMouseInputVideoMode(0);
    return 1;
}

// Checks an engine and, when bReport, logs its gestalt. Usable means: the required optional
// feature (bit 28), and at least 4 MB of fast texture memory.
// mac ppc classic/1.09d: 102b3fec (1.00: 1007994c)
int sRaveEngineReport(TQAEngine * pEngine, int bReport) {
    uint32_t nValue[4];
    uint32_t nResponse;
    QAEngineGestalt(pEngine, kQAGestalt_OptionalFeatures, nValue);
    if ((nValue[0] & 0x10000000) == 0) {
        return 0;
    }
    QAEngineGestalt(pEngine, kQAGestalt_OptionalFeatures2, nValue);
    gbRaveSwapBuffers = (nValue[0] & 0x10) != 0;
    QAEngineGestalt(pEngine, kQAGestalt_FastFeatures, nValue);
    gbRaveFastTexture = (nValue[0] & 4) != 0;
    QAEngineGestalt(pEngine, kQAGestalt_VendorID, &nResponse);
    if (bReport) {
        if (nResponse < sizeof(sgszRaveVendorNames) / sizeof(sgszRaveVendorNames[0])) {
            LOGFILE_WriteStringToFile(sgszRaveVendorNames[nResponse]);
        } else {
            LOGFILE_WriteStringToFile("\tVendor: Unknown (%d)", nResponse);
        }
    }
    QAEngineGestalt(pEngine, kQAGestalt_EngineID, &nResponse);
    if (bReport) {
        LOGFILE_WriteStringToFile("\tEngine ID: %d", nResponse);
    }
    QAEngineGestalt(pEngine, kQAGestalt_Revision, &nResponse);
    if (bReport) {
        LOGFILE_WriteStringToFile("\tRevision: %d", nResponse);
    }
    QAEngineGestalt(pEngine, kQAGestalt_ASCIINameLength, &nResponse);
    char * szName = (char *)SMemAlloc(nResponse + 1, sgszRaveCoreFile, 547, 0);
    QAEngineGestalt(pEngine, kQAGestalt_ASCIIName, szName);
    if (bReport) {
        LOGFILE_WriteStringToFile("\tName: %s", szName);
    }
    SMemFree(szName, sgszRaveCoreFile, 550, 0);
    QAEngineGestalt(pEngine, kQAGestalt_TextureMemory, &nResponse);
    if (bReport) {
        LOGFILE_WriteStringToFile("\tTexture memory: %d", nResponse);
    }
    QAEngineGestalt(pEngine, kQAGestalt_FastTextureMemory, &nResponse);
    if (bReport) {
        LOGFILE_WriteStringToFile("\tTexture memory (fast): %d", nResponse);
    }
    if ((int32_t)nResponse < 0x400000) {
        return 0;
    }
    gbRaveTextureLimit = 0;
    // Queried and discarded: the pixel-type lists.
    QAEngineGestalt(pEngine, kQAGestalt_DrawContextPixelTypesAllowed, &nResponse);
    QAEngineGestalt(pEngine, kQAGestalt_DrawContextPixelTypesPreferred, &nResponse);
    QAEngineGestalt(pEngine, kQAGestalt_TexturePixelTypesAllowed, &nResponse);
    QAEngineGestalt(pEngine, kQAGestalt_TexturePixelTypesPreferred, &nResponse);
    QAEngineGestalt(pEngine, kQAGestalt_BitmapPixelTypesAllowed, &nResponse);
    QAEngineGestalt(pEngine, kQAGestalt_BitmapPixelTypesPreferred, &nResponse);
    return 1;
}

// Windows slot 0 (Initialize; D2MOO's pfDetect): both weak libraries must have loaded.
// mac ppc classic/1.09d: 102b43b4 (1.00: 1007a520)
int RaveDetect() {
    LOGFILE_WriteStringToFile("RaveDetect: Detecting RAVE configuration...");
    if (&DSpGetVersion == nullptr) {
        LOGFILE_WriteStringToFile("RaveDetect: *** DrawSprocket library did not load.");
        return 0;
    }
    if (&QADeviceGetFirstEngine == nullptr) {
        LOGFILE_WriteStringToFile("RaveDetect: *** RAVE library did not load.");
        return 0;
    }
    return 1;
}

// Any RAVE error is fatal: log it with a stack crawl, then stop.
// mac ppc classic/1.09d: 102b4420 (1.00: 1007a58c)
void RaveCheckError(TQAError nError) {
    if (nError == 0) {
        return;
    }
    const char * szName = (uint32_t)nError < 9 ? sgszRaveErrorNames[nError] : "<<unknown>>";
    char szCrawl[2000];
    LOGFILE_WriteStringToFile("RaveCheckError: *** RAVE returned a %s error. Stack crawl:", szName);
    StackCrawl(szCrawl, sizeof(szCrawl), "", "\n", 2, 0x28);
    LOGFILE_WriteStringToFile(szCrawl);
    fp_fatal("RAVE Error: %s", szName);
}

// Fills rows of a frame buffer with one byte value (every other row when bInterlaced; 0 rows
// means to the bottom). The binary unrolls the fill into 64-byte doubleword stores after
// aligning to 8 bytes; the words written are the same.
// mac ppc classic/1.09d: 102b44f4 (1.00: 1007a660)
void sRaveFillDisplay(const RaveFrameBuffer * pBuffer, uint8_t nValue, unsigned int nFirstRow, unsigned int nRows,
                      char bInterlaced) {
    if (bInterlaced && (nFirstRow & 1)) {
        ++nFirstRow;
    }
    int nPitch = pBuffer->nRowBytes;
    const int nWords = (pBuffer->nWidth * pBuffer->nBytesPerPixel) / 4;
    uint8_t * pRow = pBuffer->pBase + nFirstRow * nPitch;
    if (nRows == 0) {
        nRows = pBuffer->nHeight - nFirstRow;
    }
    if (bInterlaced) {
        nRows >>= 1;
        nPitch <<= 1;
    }
    uint32_t dwFill = nValue | (nValue << 8);
    dwFill |= dwFill << 16;
    for (unsigned int nRow = 0; nRow < nRows; ++nRow) {
        uint32_t * pOut = (uint32_t *)pRow;
        for (int n = 0; n < nWords; ++n) {
            pOut[n] = dwFill;
        }
        pRow += nPitch;
    }
}

} // namespace D2Rave

#endif // !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC
