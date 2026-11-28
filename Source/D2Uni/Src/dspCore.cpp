// dspCore.cpp - D2Uni's DrawSprocket back end: the full-screen software renderer (gnUniMode 1).
// The game draws into an 8-bit MemCGrafPort; DSpBlit copies it to the DrawSprocket front buffer
// and uploads the CLUT when the palette changed. Cutscenes switch the context to a 16/32-bit one
// for Bink and decode straight into the front buffer.
//
// File name inferred (Source/D2Uni/README.md): the prefix is the code's own ("DSp*: " log
// strings), the <prefix>Core.cpp pattern is the Windows one. The Windows twin is D2DDraw's
// ddCore.cpp (DirectDraw = the full-screen Windows software renderer). MWCC emits an object's
// functions last to first, so the functions below are in source order, highest address first.
//
// Several helpers exist only inlined: DSpDeactivateSurface, DSpCloseContext, DSpSwapContext and
// DSpCreateContext are named by their own log prefixes; DSpClearScreen is our name for the
// screen clear that precedes each fade.
//
// Bodies from LoD 1.10b classic. Classic 1.00 differs most here (alternate buffers,
// DSpBlit_Fastest timed against CopyBits, DSpScreenGrab); see d2-surveys/macppc/d2uni/recon.md.
//
// Classic Mac PowerPC only.
#if !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC

#include "Source/D2Uni/Src/uniInternal.h"

// The rectangle of the front buffer the game surface is centred in, and the same rectangle in
// global coordinates (DSpProcessEvent clamps the mouse to it).
Rect gUniDSpGlobalRect;                     // 1055e5b4
Rect gUniDSpDestRect;                       // 1055e5bc

static int gnDSpBinkDepth;                  // 104344a0, 32 or 16 for the Bink context, 0 = none
static BOOL gbDSpInCutscene;                // 104344a4
static int16_t gnDSpOriginV;                // 104344a8, surface origin, global
static int16_t gnDSpOriginH;                // 104344aa
static int16_t gnDSpOffsetV;                // 104344ac, surface origin within the front buffer
static int16_t gnDSpOffsetH;                // 104344ae
static Boolean gbDSpClutDirty;              // 104344b0
static GDHandle ghDSpPortDevice;            // 104344b4, the MemCGrafPort's device
static CGrafPtr gpDSpPort;                  // 104344b8, the 8-bit back buffer port
static DSpContextReference ghDSpContext;    // 104344bc

// Back buffer and screen sizes by resolution (0 = 640x480, 1 = 800x600).
static const int gaDSpScreenWidths[2] = {640, 800};      // 103f562c
static const int gaDSpScreenHeights[2] = {480, 600};     // 103f563c
static const int gaDSpBufferWidths[2] = {640, 800};      // 103f564c
static const int gaDSpBufferHeights[2] = {480, 600};     // 103f5654

// What sDSpFillPixMap fills: a PixMap's base, size and row bytes. Built from the device's PixMap
// before every screen clear. Name and layout ours (the binary builds it on the stack).
struct D2DSpFillTargetStrc {
    /* 0x00 */ Ptr pBase;
    /* 0x04 */ int32_t nUnused;
    /* 0x08 */ int16_t nWidth;
    /* 0x0A */ int16_t nHeight;
    /* 0x0C */ int16_t nBytesPerPixel;
    /* 0x0E */ int16_t nRowBytes;
};

static void sDSpFillPixMap(D2DSpFillTargetStrc * pTarget, BYTE nValue, int nStartRow, uint32_t nRows, Boolean bInterlaced);
BOOL DSpSuspendContext();
BOOL DSpResumeContext();
static BOOL DSpActivateSurface(int nResolution, int nDepth);
static OSStatus iDSpSwapContext(int nResolution, int nDepth);

// Clears the whole screen of the current device to index 0. Inlined everywhere it is used.
static inline void DSpClearScreen() {
    PixMapHandle hPixMap = (*ghUniDevice)->gdPMap;
    D2DSpFillTargetStrc tTarget;
    tTarget.pBase = GetPixBaseAddr(hPixMap);
    tTarget.nWidth = (*hPixMap)->bounds.right - (*hPixMap)->bounds.left;
    tTarget.nHeight = (*hPixMap)->bounds.bottom - (*hPixMap)->bounds.top;
    tTarget.nBytesPerPixel = (*hPixMap)->pixelSize >> 3;
    tTarget.nRowBytes = (*hPixMap)->rowBytes & 0x3FFF;
    sDSpFillPixMap(&tTarget, 0, 0, 0, 0);
}

// Drops the back buffer port and every size, and tells D2gfx there is no buffer. Inlined.
static inline void DSpDeactivateSurface() {
    LogPrintf("DSpDeactivateSurface: Closing DrawSprocket context.");
    if (gpDSpPort || ghDSpPortDevice) {
        DisposeMemCGrafPort(gpUniBackBuffer, gpDSpPort, ghDSpPortDevice);
        gpUniBackBuffer = nullptr;
        gpDSpPort = nullptr;
        ghDSpPortDevice = nullptr;
    }
    gnUniScreenHeight = 0;
    gnUniScreenWidth = 0;
    gnUniBackBufferHeight = 0;
    gnUniBackBufferWidth = 0;
    gpUniHelpers->nfpFillYBufferTable(nullptr, 0, 0, 0);
    UniEngineReset();
}

// Deactivates and releases the context. Inlined.
static inline OSStatus DSpCloseContext() {
    OSStatus nErr = 0;
    if (ghDSpContext) {
        nErr = DSpContext_SetState(ghDSpContext, kDSpContextState_Inactive);
        if (nErr) {
            LogPrintf("DSpCloseContext: *** DSpContext_SetState failed.");
        }
        nErr = DSpContext_Release(ghDSpContext);
        if (nErr) {
            LogPrintf("DSpCloseContext: *** DSpContext_Release failed.");
        }
        ghDSpContext = nullptr;
    }
    return nErr;
}

// Switches the context, behind a black screen and a gamma fade. Inlined.
static inline OSStatus DSpSwapContext(int nResolution, int nDepth) {
    DSpClearScreen();
    MacRestoreGamma(ghUniDevice);
    DSpContext_FadeGamma(nullptr, 0, nullptr);
    OSStatus nErr = iDSpSwapContext(nResolution, nDepth);
    if (nErr) {
        LogPrintf("DSpSwapContext: *** iDSpSwapContext failed. (%d)", nErr);
    }
    DSpClearScreen();
    DSpContext_FadeGamma(nullptr, 100, nullptr);
    MacSetGammaCurve(ghUniDevice, gnUniGamma, gnUniContrast);
    return nErr;
}

// ppc 1.10b: 10057f50 (1.00 100872fc, 1.13d 1005dfc0, carbon 1.10b 103a7010, carbon 1.13d 103d67a0)
// Fills rows of a PixMap with one byte value, 32 bits at a time (the binary unrolls to 64-byte
// and 8-byte stores after aligning to 8). nRows 0 means to the bottom; bInterlaced fills every
// other row starting on an even one.
static void sDSpFillPixMap(D2DSpFillTargetStrc * pTarget, BYTE nValue, int nStartRow, uint32_t nRows, Boolean bInterlaced) {
    if (bInterlaced && (nStartRow & 1)) {
        nStartRow++;
    }

    int nStride = pTarget->nRowBytes;
    int nWords = (pTarget->nWidth * pTarget->nBytesPerPixel) / 4;
    BYTE * pRow = (BYTE *)pTarget->pBase + nStartRow * pTarget->nRowBytes;
    if (nRows == 0) {
        nRows = pTarget->nHeight - nStartRow;
    }
    if (bInterlaced) {
        nStride *= 2;
        nRows /= 2;
    }

    uint32_t nFill = nValue | (nValue << 8);
    nFill |= nFill << 16;
    for (uint32_t nRow = 0; nRow < nRows; nRow++) {
        uint32_t * pWord = (uint32_t *)pRow;
        for (int i = 0; i < nWords; i++) {
            pWord[i] = nFill;
        }
        pRow += nStride;
    }
}

// ppc 1.10b: 10057ef0 (1.00 100872a0, 1.13d 1005df60, carbon 1.10b 103a6fa0, carbon 1.13d 103d6730)
// Storm's switch-out callback: pause the context.
BOOL DSpSuspendContext() {
    OSStatus nErr = 0;
    if (ghDSpContext) {
        nErr = DSpContext_SetState(ghDSpContext, kDSpContextState_Paused);
        if (nErr) {
            LogPrintf("DSpSuspendContext: *** DSpContext_SetState failed.");
        }
    }
    return (BYTE)nErr;
}

// ppc 1.10b: 10057e70 (1.00 1008722c, 1.13d 1005dee0, carbon 1.10b 103a6f20, carbon 1.13d 103d66b0)
// Storm's switch-in callback: reactivate the context ("already active" is fine).
BOOL DSpResumeContext() {
    OSStatus nErr = 0;
    if (ghDSpContext) {
        nErr = DSpContext_SetState(ghDSpContext, kDSpContextState_Active);
        if ((uint16_t)nErr == (uint16_t)kDSpStateAlreadySetErr) {
            nErr = 0;
        } else if (nErr) {
            LogPrintf("DSpResumeContext: *** DSpContext_SetState failed.");
        }
    }
    return (BYTE)nErr;
}

// ppc 1.10b: 10057b10 (1.00 10086e84, 1.13d 1005db90, carbon 1.10b 103a6bc0, carbon 1.13d 103d6360)
// Builds the 8-bit back buffer (a MemCGrafPort over the shared colour table), centres the game
// screen in the front buffer, loads a black CLUT with white at 255 when the context is 8-bit, and
// clears the screen.
static BOOL DSpActivateSurface(int nResolution, int nDepth) {
    int nRowBytes;

    UniEngineSetResolution();
    UniSpriteSetResolution(nResolution);
    int nBufferHeight = gaDSpBufferHeights[nResolution];
    int nBufferWidth = gaDSpBufferWidths[nResolution];
    gnUniScreenWidth = gaDSpScreenWidths[nResolution];
    gnUniScreenHeight = gaDSpScreenHeights[nResolution];

    Rect tBounds = {0, 0, (int16_t)nBufferHeight, (int16_t)nBufferWidth};
    OSErr nErr = NewMemCGrafPort(&tBounds, (int16_t)nDepth, ghUniColorTable, &gpUniBackBuffer, &nRowBytes, &gpDSpPort,
                                 &ghDSpPortDevice);
    if (nErr) {
        LogPrintf("DSpActivateSurface: *** NewMemCGrafPort failed.");
        DSpDeactivateSurface();
        return 0;
    }

    gnUniBackBufferHeight = nBufferHeight;
    gnUniBackBufferDepth = nDepth;
    gnUniBackBufferWidth = (nRowBytes << 3) / nDepth;
    UniClearScreen(0);

    const Rect & tDevice = (*ghUniDevice)->gdRect;
    gnDSpOffsetV = (int16_t)(((tDevice.bottom - tDevice.top) - gnUniScreenHeight) / 2);
    gnDSpOffsetH = (int16_t)(((tDevice.right - tDevice.left) - gnUniScreenWidth) / 2);
    gnDSpOriginV = gnDSpOffsetV + tDevice.top;
    gnDSpOriginH = gnDSpOffsetH + tDevice.left;
    gUniDSpDestRect.top = gnDSpOffsetV;
    gUniDSpDestRect.left = gnDSpOffsetH;
    gUniDSpDestRect.bottom = gnDSpOffsetV + (int16_t)gnUniScreenHeight;
    gUniDSpDestRect.right = gnDSpOffsetH + (int16_t)gnUniScreenWidth;
    gUniDSpGlobalRect.top = gnDSpOriginV;
    gUniDSpGlobalRect.left = gnDSpOriginH;
    gUniDSpGlobalRect.bottom = gnDSpOriginV + (int16_t)gnUniScreenHeight;
    gUniDSpGlobalRect.right = gnDSpOriginH + (int16_t)gnUniScreenWidth;
    gpUniHelpers->nfpFillYBufferTable(gpUniBackBuffer, gnUniBackBufferWidth, gnUniBackBufferHeight, 0);

    if (nDepth == 8) {
        ColorSpec aEntries[256] = {};
        aEntries[255].rgb.red = 0xFFFF;
        aEntries[255].rgb.green = 0xFFFF;
        aEntries[255].rgb.blue = 0xFFFF;
        if (DSpContext_SetCLUTEntries(ghDSpContext, aEntries, 0, 255)) {
            LogPrintf("DSpActivateSurface: *** DSpContext_SetCLUTEntries failed.");
        }
        gbDSpClutDirty = 1;
    }

    DSpClearScreen();
    return 1;
}

// ppc 1.10b: 10057990 (1.00 10086d04, 1.13d 1005da10, carbon 1.10b 103a6a30, carbon 1.13d 103d61d0)
// Releases the context and reserves one at the resolution's display mode and nDepth.
static OSStatus iDSpSwapContext(int nResolution, int nDepth) {
    DSpContextAttributes tAttributes = {};

    OSStatus nErr = DSpContext_SetState(ghDSpContext, kDSpContextState_Inactive);
    if (nErr) {
        LogPrintf("DSpSwapContext: *** DSpContext_SetState failed.");
        return nErr;
    }
    nErr = DSpContext_Release(ghDSpContext);
    if (nErr) {
        LogPrintf("DSpSwapContext: *** DSpContext_Release failed.");
        return nErr;
    }

    ghDSpContext = nullptr;
    const D2MacDisplayModeStrc & tMode = gaUniDisplayModes[nResolution];
    nErr = DSpFindContextEx(ghUniDevice, tMode.wWidth & 0x7FFF, tMode.nHeight, tMode.fRefreshRate, nDepth, &ghDSpContext,
                            &tAttributes);
    if (nErr) {
        LogPrintf("DSpSwapContext: *** DSpFindContextEx failed.");
        return nErr;
    }
    nErr = DSpContext_Reserve(ghDSpContext, &tAttributes);
    if (nErr) {
        LogPrintf("DSpSwapContext: *** DSpContext_Reserve failed.");
        return nErr;
    }
    nErr = DSpContext_SetState(ghDSpContext, kDSpContextState_Active);
    if (nErr == kDSpStateAlreadySetErr || nErr == 0) {
        nErr = 0;
    } else {
        LogPrintf("DSpSwapContext: *** DSpContext_SetState failed.");
    }
    return nErr;
}

// ppc 1.10b: 10057830 (1.00 10086a08, 1.13d 1005d8b0, carbon 1.10b 103a6910, carbon 1.13d 103d60b0)
// Mac slot 11 under DrawSprocket. Overloads the DrawSprocket call of the same name (C++ picks by
// arity); hands the event to DrawSprocket, tracks the suspend/resume state, and keeps the mouse
// inside the game screen while the context is up.
Boolean DSpProcessEvent(EventRecord * pEvent) {
    Boolean bProcessed = 0;
    ::DSpProcessEvent(pEvent, &bProcessed);
    if (pEvent->what == osEvt && (pEvent->message >> 24) == suspendResumeMessage) {
        gbUniAppActive = pEvent->message & resumeFlag;
    }

    // what < 9: null, mouse, key and update events, the ones that carry a meaningful position.
    if (ghDSpContext && !gbUniSurfaceReleased && pEvent->what < 9) {
        Point tWhere = pEvent->where;
        if (tWhere.h < gUniDSpGlobalRect.left) {
            tWhere.h = gUniDSpGlobalRect.left;
        }
        if (tWhere.v < gUniDSpGlobalRect.top) {
            tWhere.v = gUniDSpGlobalRect.top;
        }
        if (tWhere.h > gUniDSpGlobalRect.right - 1) {
            tWhere.h = gUniDSpGlobalRect.right - 1;
        }
        if (tWhere.v > gUniDSpGlobalRect.bottom - 1) {
            tWhere.v = gUniDSpGlobalRect.bottom - 1;
        }
        if (tWhere.h != pEvent->where.h || tWhere.v != pEvent->where.v) {
            SetMousePosition(tWhere);
            pEvent->where = tWhere;
        }
    }
    return bProcessed;
}

// ppc 1.10b: 100577e0 (1.00 100869b8, 1.13d 1005d860, carbon 1.10b 103a68c0, carbon 1.13d 103d6060)
BOOL DSpGlobalToLocal(Point * pPoint) {
    if (!ghDSpContext || gbUniSurfaceReleased) {
        return 0;
    }
    pPoint->h -= gnDSpOriginH;
    pPoint->v -= gnDSpOriginV;
    return 1;
}

// ppc 1.10b: 10057790 (1.00 10086968, 1.13d 1005d810, carbon 1.10b 103a6870, carbon 1.13d 103d6010)
BOOL DSpLocalToGlobal(Point * pPoint) {
    if (!ghDSpContext || gbUniSurfaceReleased) {
        return 0;
    }
    pPoint->h += gnDSpOriginH;
    pPoint->v += gnDSpOriginV;
    return 1;
}

// ppc 1.10b: 10057780 (1.00 10086964, 1.13d 1005d800, carbon 1.10b 103a6860, carbon 1.13d 103d6000)
void DSpSetGDevice() {
}

// ppc 1.10b: 10057740 (1.00 10086920, 1.13d 1005d7c0, carbon 1.10b 103a6820, carbon 1.13d 103d5fc0)
// DrawSprocket is a weak import: present only when the library loaded.
BOOL DSpDetect() {
    BOOL bLoaded = (void *)DSpGetVersion != nullptr;
    if (!bLoaded) {
        LogPrintf("DSpDetect: *** DrawSprocket 1.7 did not load.");
    }
    return bLoaded;
}

// ppc 1.10b: 100576c0 (1.00 100868a0, 1.13d 1005d740, carbon 1.10b 103a67a0, carbon 1.13d 103d5f40)
// Keeps D2gfx's settings and helpers and registers the switch-out and cursor callbacks with Storm.
BOOL DSpInit(D2GfxSettingsStrc * pSettings, D2GfxHelperStrc * pHelpers) {
    gpUniSettings = pSettings;
    gpUniHelpers = pHelpers;
    vid->pfnSetSuspendResumeProcs(DSpSuspendContext, DSpResumeContext);
    vid->pfnSetCursorProcs(MacInitCursor, ShowCursor, HideCursor);
    vid->pfnInitCursor();
    return 1;
}

// ppc 1.10b: 10057680 (1.00 10086840, 1.13d 1005d700, carbon 1.10b 103a6760, carbon 1.13d 103d5f00)
BOOL DSpRelease() {
    vid->pfnSetSuspendResumeProcs(nullptr, nullptr);
    return 1;
}

// ppc 1.10b: 100573b0 (1.00 1008660c, 1.13d 1005d450, carbon 1.10b 103a6470, carbon 1.13d 103d5c30)
// Finds the deepest context Bink can use (32, else 16 bits, at 640x480), then reserves and
// activates the 8-bit game context at the requested resolution.
BOOL DSpCreateSurface(HWND hWnd, int nResolution) {
    DSpContextReference hProbe = nullptr;
    DSpContextAttributes tAttributes = {};

    const D2MacDisplayModeStrc & tBinkMode = gaUniDisplayModes[0];
    gnDSpBinkDepth = 32;
    OSStatus nErr = DSpFindContextEx(ghUniDevice, tBinkMode.wWidth & 0x7FFF, tBinkMode.nHeight, tBinkMode.fRefreshRate,
                                     32, &hProbe, &tAttributes);
    if (nErr) {
        gnDSpBinkDepth = 16;
        nErr = DSpFindContextEx(ghUniDevice, tBinkMode.wWidth & 0x7FFF, tBinkMode.nHeight, tBinkMode.fRefreshRate, 16,
                                &hProbe, &tAttributes);
    }
    if (nErr) {
        gnDSpBinkDepth = 0;
        LogPrintf("DSpCreateSurface: Couldn't find a suitable depth for Bink.");
    }

    const D2MacDisplayModeStrc & tMode = gaUniDisplayModes[nResolution];
    nErr = DSpFindContextEx(ghUniDevice, tMode.wWidth & 0x7FFF, tMode.nHeight, tMode.fRefreshRate, 8, &ghDSpContext,
                            &tAttributes);
    if (nErr || !ghDSpContext) {
        LogPrintf("DSpCreateSurface: *** DSpFindContextEx (%s) failed.", nResolution ? "800x600" : "640x480");
        return 0;
    }

    // DSpCreateContext, inlined.
    nErr = DSpContext_Reserve(ghDSpContext, &tAttributes);
    if (nErr) {
        LogPrintf("DSpCreateContext: *** DSpContext_Reserve failed.");
        DSpCloseContext();
        return 0;
    }

    nErr = DSpContext_SetState(ghDSpContext, kDSpContextState_Active);
    if (nErr != kDSpStateAlreadySetErr && nErr) {
        LogPrintf("DSpCreateSurface: *** DSpContext_SetState failed. (%d)", nErr);
        DSpCloseContext();
        return 0;
    }

    MacSetGammaCurve(ghUniDevice, gnUniGamma, gnUniContrast);
    if (!DSpActivateSurface(nResolution, 8)) {
        LogPrintf("DSpCreateSurface: *** DSpActivateSurface failed.");
        return 0;
    }

    MacSetInputSuspended(0);
    LogPrintf("DSpCreateSurface: DrawSprocket initialized successfully.");
    return 1;
}

// ppc 1.10b: 10057250 (1.00 100864c4, 1.13d 1005d300, carbon 1.10b 103a6300, carbon 1.13d 103d5ae0)
BOOL DSpCloseSurface() {
    LogPrintf("DSpCloseSurface: Closing DrawSprocket context.");
    MacSetInputSuspended(1);
    MacRestoreGamma(ghUniDevice);
    DSpDeactivateSurface();
    OSStatus nErr = DSpCloseContext();
    if (nErr) {
        LogPrintf("DSpCloseSurface: *** Couldn't close the context.");
    }
    return nErr == 0;
}

// ppc 1.10b: 10057090 (1.00 10086324, 1.13d 1005d160, carbon 1.10b 103a6130, carbon 1.13d 103d5930)
// Waits for the frame in flight, then either rebuilds the surface or (bRelease) closes it and
// marks it released. The close is DSpCloseSurface inlined, after a request to end any movie.
void DSpEndCutScene(HWND hWnd, int nResolution, BOOL bRelease) {
    while (gbUniInScene) {
        YieldToThinThread(0);
    }

    if (!bRelease) {
        DSpCreateSurface(hWnd, nResolution);
        gbUniSurfaceReleased = 0;
        return;
    }

    gbUniCutsceneQuit = 1;
    DSpCloseSurface();
    gbUniSurfaceReleased = 1;
}

// ppc 1.10b: 10056c80 (1.00 10086194, 1.13d 1005cd70, carbon 1.10b 103a5d30, carbon 1.13d 103d5550)
// Switches to the Bink context if the surface is not already at its depth, then decodes the movie
// straight into the front buffer at the centred origin.
void DSpPlayCutscene(char * szFile, int nResolution, fnBinkFrameCallback pfnFrame) {
    CGrafPtr pFront;

    if (!gnDSpBinkDepth) {
        return;
    }
    if (gnUniBackBufferDepth != gnDSpBinkDepth && !DSpCutsceneCreateWindow()) {
        LogPrintf("DSpPlayCutscene: *** DSpCutsceneCreateWindow failed.");
        return;
    }

    DSpClearScreen();
    MacSetGammaCurve(ghUniDevice, 100, gnUniContrast);
    OSStatus nErr = DSpContext_GetFrontBuffer(ghDSpContext, &pFront);
    if (nErr || !pFront) {
        LogPrintf("DSpPlayCutscene: *** DSpContext_GetFrontBuffer failed. (%d)", nErr);
        return;
    }

    BYTE * pBase = (BYTE *)GetPixBaseAddr(pFront->portPixMap);
    int nRowBytes = (*pFront->portPixMap)->rowBytes & 0x3FFF;
    BYTE * pDest = pBase + gnDSpOffsetV * nRowBytes + gnDSpOffsetH * (gnDSpBinkDepth / 8);
    UniPlayBinkFile(szFile, nResolution, pDest, nRowBytes, BinkGDSurfaceType(ghUniDevice), pfnFrame);
}

// ppc 1.10b: 100569c0 (1.00 10086078, 1.13d 1005cad0, carbon 1.10b 103a5a90, carbon 1.13d 103d52d0)
// Mac slot 16 / Windows 13 BeginCutScene: moves to the 640x480 Bink context.
BOOL DSpCutsceneCreateWindow() {
    if (gbUniSurfaceReleased || !gnDSpBinkDepth) {
        return 0;
    }

    LogPrintf("DSpCutsceneCreateWindow: Switching to 640x480x%d for Bink.", gnDSpBinkDepth);
    DSpDeactivateSurface();
    if (DSpSwapContext(0, gnDSpBinkDepth)) {
        LogPrintf("DSpCutsceneCreateWindow: *** DSpSwapContext failed.");
    }
    if (!DSpActivateSurface(0, gnDSpBinkDepth)) {
        LogPrintf("DSpCutsceneCreateWindow: *** DSpActivateSurface failed.");
        return 0;
    }
    return 1;
}

// ppc 1.10b: 10056990 (1.00 10086048, 1.13d 1005caa0, carbon 1.10b 103a5a60, carbon 1.13d 103d52a0)
// Mac slot 17 / Windows 14 PlayCutScene: DSpPlayCutscene inside the in-cutscene flag.
void DSpPlayCutsceneGuarded(char * szFile, int nResolution, fnBinkFrameCallback pfnFrame) {
    gbDSpInCutscene = 1;
    DSpPlayCutscene(szFile, nResolution, pfnFrame);
    gbDSpInCutscene = 0;
}

// ppc 1.10b: 10056980 (1.00 10086040, 1.13d 1005ca90, carbon 1.10b 103a5a50, carbon 1.13d 103d5290)
BOOL DSpCheckCutscene() {
    return gbDSpInCutscene;
}

// ppc 1.10b: 10056900 (1.00 10085fd8, 1.13d 1005ca10, carbon 1.10b 103a59d0, carbon 1.13d 103d5210)
BOOL DSpStartDraw(BOOL bClear) {
    gbUniInScene = 1;
    gpUniHelpers->nfpFillYBufferTable(gpUniBackBuffer, gnUniBackBufferWidth, gnUniBackBufferHeight, 0);
    if (bClear) {
        UniClearScreen(1);
    }
    return 1;
}

// ppc 1.10b: 100568b0 (1.00 10085f88, 1.13d 1005c9c0, carbon 1.10b 103a5980, carbon 1.13d 103d51c0)
BOOL DSpEndDraw() {
    gbUniInScene = 0;
    gpUniHelpers->nfpFillYBufferTable(nullptr, 0, 0, 0);
    return 1;
}

// ppc 1.10b: 100568a0 (1.00 10085f80, 1.13d 1005c9b0, carbon 1.10b 103a5970, carbon 1.13d 103d51b0)
// Bink decodes straight into the front buffer, so there is nothing to present.
BOOL DSpBinkBlit(Rect * pRect) {
    return 1;
}

// ppc 1.10b: 10056760 (1.00 100857dc, 1.13d 1005c870, carbon 1.10b 103a5740, carbon 1.13d 103d4f80)
// Windows 8 EndScene2: one CopyBits of the 8-bit port into the front buffer (the colour table
// seeds are made equal first, so CopyBits copies indices instead of remapping), then the deferred
// CLUT upload.
BOOL DSpBlit() {
    CGrafPtr pFront;

    if (!ghDSpContext) {
        LogPrintf("DSpBlit: *** Context does not exist.");
        return 0;
    }

    DSpContext_GetFrontBuffer(ghDSpContext, &pFront);
    Rect tDest = gUniDSpDestRect;
    Rect tSource = {0, 0, (int16_t)gnUniScreenHeight, (int16_t)gnUniScreenWidth};
    (*(*pFront->portPixMap)->pmTable)->ctSeed = (*(*gpDSpPort->portPixMap)->pmTable)->ctSeed;
    SetPort(pFront);
    CopyBits((const BitMap *)*gpDSpPort->portPixMap, (const BitMap *)*pFront->portPixMap, &tSource, &tDest, srcCopy,
             nullptr);
    if (gbDSpClutDirty) {
        if (DSpContext_SetCLUTEntries(ghDSpContext, (*ghUniColorTable)->ctTable, 0, 255)) {
            LogPrintf("DSpBlit: *** DSpContext_SetCLUTEntries failed.");
        }
        gbDSpClutDirty = 0;
    }
    return 1;
}

// ppc 1.10b: 10056740 (1.00 10085554, 1.13d 1005c850, carbon 1.10b 103a5720, carbon 1.13d 103d4f60)
void DSpGetScreenSize(int * pWidth, int * pHeight) {
    *pWidth = gnUniScreenWidth;
    *pHeight = gnUniScreenHeight;
}

// ppc 1.10b: 100565a0 (1.00 100852ec, 1.13d 1005c6b0, carbon 1.10b 103a5580, carbon 1.13d 103d4dc0)
// Copies the engine palette into the colour table from the first entry that differs (8-bit
// components widened as c<<8|c), and marks the CLUT for upload. No change, no upload.
void DSpSetPalette(PALETTEENTRY * pPalette) {
    ColorSpec * pTable = (*ghUniColorTable)->ctTable;
    int nFirst = 0;
    while (nFirst < 256 && (pTable[nFirst].rgb.red >> 8) == pPalette[nFirst].peRed &&
           (pTable[nFirst].rgb.green >> 8) == pPalette[nFirst].peGreen &&
           (pTable[nFirst].rgb.blue >> 8) == pPalette[nFirst].peBlue) {
        nFirst++;
    }
    if (nFirst == 256) {
        return;
    }

    for (int i = nFirst; i < 256; i++) {
        pTable[i].rgb.red = (uint16_t)(pPalette[i].peRed << 8 | pPalette[i].peRed);
        pTable[i].rgb.green = (uint16_t)(pPalette[i].peGreen << 8 | pPalette[i].peGreen);
        pTable[i].rgb.blue = (uint16_t)(pPalette[i].peBlue << 8 | pPalette[i].peBlue);
    }
    gbDSpClutDirty = 1;
}

// ppc 1.10b: 10056570 (1.00 100852bc, 1.13d 1005c680, carbon 1.10b 103a5550, carbon 1.13d 103d4d90)
// The 72 table pointers are copied, not referenced (as in the RAVE renderer).
void DSpSetPaletteTable(BYTE ** ppTables) {
    CopyMemory(gapUniPaletteTables, ppTables, sizeof(gapUniPaletteTables));
}

// ppc 1.10b: 10056560 (1.00 100852b8, 1.13d 1005c670, carbon 1.10b 103a5540, carbon 1.13d 103d4d80)
void DSpUpdateScaleFactor() {
}

// ppc 1.10b: 10056500 (1.00 10085268, 1.13d 1005c610, carbon 1.10b 103a5490, carbon 1.13d 103d4cd0)
// Clamped to [0x37, 0xFF], loaded into the device's gamma table, CLUT re-uploaded.
void DSpSetGamma(int nGamma) {
    if (nGamma < 0x37) {
        nGamma = 0x37;
    }
    if (nGamma > 0xFF) {
        nGamma = 0xFF;
    }
    gnUniGamma = nGamma;
    MacSetGammaCurve(ghUniDevice, nGamma, gnUniContrast);
    gbDSpClutDirty = 1;
}

// ppc 1.10b: 100564f0 (1.00 10085260, 1.13d 1005c600, carbon 1.10b 103a5480, carbon 1.13d 103d4cc0)
int DSpCheckGamma() {
    return 1;
}

// ppc 1.10b: 100564e0 (1.00 1008525c, 1.13d 1005c5f0, carbon 1.10b 103a5470, carbon 1.13d 103d4cb0)
void DSpSetGlobalLight() {
}

// ppc 1.10b: 100564d0 (1.00 10085258, 1.13d 1005c5e0, carbon 1.10b 103a5460, carbon 1.13d 103d4ca0)
void DSpSetPerspectiveScale() {
}

// ppc 1.10b: 100564c0 (1.00 10085254, 1.13d 1005c5d0, carbon 1.10b 103a5450, carbon 1.13d 103d4c90)
void DSpAdjustPerspectivePosition() {
}

// ppc 1.10b: 100564b0 (1.00 10085250, 1.13d 1005c5c0, carbon 1.10b 103a5440, carbon 1.13d 103d4c80)
void DSpPerspectiveScalePosition() {
}

// ppc 1.10b: 100564a0 (1.00 1008524c, 1.13d 1005c5b0, carbon 1.10b 103a5430, carbon 1.13d 103d4c70)
void DSpSetDefaultPerspectiveFactor() {
}

// ppc 1.10b: 10056220 (1.00 10085148, 1.13d 1005c360, carbon 1.10b 103a51d0, carbon 1.13d 103d4a40)
// Windows 9 ResizeWindow: tear the surface down, swap the context to the new resolution, build
// the surface again.
BOOL DSpChangeRes(HWND hWnd, int nResolution) {
    DSpDeactivateSurface();
    LogPrintf("DSpChangeRes: Switching to %s.", nResolution ? "800x600" : "640x480");
    DSpSwapContext(nResolution, 8);
    if (!DSpActivateSurface(nResolution, 8)) {
        LogPrintf("DSpChangeRes: *** DSpActivateSurface failed.");
        return 0;
    }
    return 1;
}

// ppc 1.10b: 10056210 (1.00 10085140, 1.13d 1005c350, carbon 1.10b 103a51c0, carbon 1.13d 103d4a30)
int * DSpGetRenderStatistics() {
    return nullptr;
}

#endif // !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC
