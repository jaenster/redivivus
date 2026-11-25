// qdCore.cpp - D2Uni's QuickDraw back end: the windowed software renderer (gnUniMode 2). The
// game draws into an 8-bit MemCGrafPort; QDBlit CopyBits it into a colour window, so in
// Thousands or Millions the 8-to-16/32-bit conversion is QuickDraw's. The monitor depth is
// switched to the requested one (32, falling back to 16) and restored on close.
//
// File name inferred (Source/D2Uni/README.md): "QD*: " is the code's own log prefix. The Windows
// twin is D2Gdi's gdiCore.cpp (GDI = the windowed Windows software renderer). MWCC emits an
// object's functions last to first, so the functions below are in source order, highest address
// first. QDDeactivateSurface is ours: the surface teardown that QDCloseSurface, QDChangeRes,
// QDCreateSurface and the cutscene paths each have inlined (it logs as "QDCloseSurface:").
//
// Bodies from LoD 1.10b classic. Classic 1.13d adds "Unrecoverable internal error" halts to
// several of these (QDChangeRes, QDEndCutScene, QDCreateSurfaceEntry grow); classic 1.00 drew
// into GWorlds and has QDScreenGrab. See d2-surveys/macppc/d2uni/recon.md.
//
// Classic Mac PowerPC only.
#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

#include "Source/D2Uni/Src/uniInternal.h"

static Point gQDWindowOrigin = {60, 20};    // 104344c0, top-left of the window, global
static int gnQDSavedDepth = 16;             // 104344c4, the device depth QDClose restores
static BOOL gbQDDeviceIsColor = 1;          // 104344c8
static BOOL gbQDInCutscene;                 // 104344d0
static Boolean gbQDCursorHidden;            // 104344d4
static Boolean gbQDHideCursorRequested;     // 104344d5, set by Storm's hide-cursor callback
static PaletteHandle ghQDPalette;           // 104344d8
static CTabHandle ghQDColorTable;           // 104344dc, the gamma-corrected copy of ghUniColorTable
static WindowPtr gpQDWindow;                // 104344e0
static GDHandle ghQDPortDevice;             // 104344e4
static CGrafPtr gpQDPort;                   // 104344e8, the 8-bit back buffer port

// Window and back buffer sizes by resolution (0 = 640x480, 1 = 800x600).
static const int gaQDScreenWidths[2] = {640, 800};   // 103f5cc8
static const int gaQDScreenHeights[2] = {480, 600};  // 103f5cd8
static const int gaQDBufferWidths[2] = {640, 800};   // 103f5ce8
static const int gaQDBufferHeights[2] = {480, 600};  // 103f5cf0

void QDForcePaletteUpdate();
static short QDCreateContext(int nWidth, int nHeight, int nDepth);

// The classic way to hand a colour port to CopyBits: its portPixMap overlays portBits.
static inline const BitMap * QDPortBits(CGrafPtr pPort) {
    return (const BitMap *)((Ptr)pPort + 2);
}

// Drops the back buffer port and every size, and tells D2gfx there is no buffer. Inlined.
static inline void QDDeactivateSurface() {
    LogPrintf("QDCloseSurface: Closing QuickDraw context.");
    if (gpQDPort || ghQDPortDevice) {
        DisposeMemCGrafPort(gpUniBackBuffer, gpQDPort, ghQDPortDevice);
        gpUniBackBuffer = nullptr;
        gpQDPort = nullptr;
        ghQDPortDevice = nullptr;
    }
    gnUniBackBufferDepth = 0;
    gnUniScreenHeight = 0;
    gnUniScreenWidth = 0;
    gnUniBackBufferHeight = 0;
    gnUniBackBufferWidth = 0;
    UniEngineReset();
    gpUniHelpers->nfpFillYBufferTable(nullptr, 0, 0, 0);
}

// Builds a 256-entry colour table with value i at entry i. Inlined (UniInit has the same code).
static inline CTabHandle QDNewColorTable() {
    CTabHandle hTable = (CTabHandle)NewHandleClear(0x808);
    if (hTable) {
        HLockHi((Handle)hTable);
        (*hTable)->ctSeed = GetCTSeed();
        (*hTable)->ctFlags = 0xC000;
        (*hTable)->ctSize = 255;
        for (int16_t i = 0; i < 256; i++) {
            (*hTable)->ctTable[i].value = i;
        }
    }
    return hTable;
}

// ppc 1.10b: 1005ab80 (1.00 10089e14, 1.13d 10060ed0, carbon 1.10b 103aa010, carbon 1.13d 103d9a90)
static void sQDInitCursorProc() {
    MacInitCursor();
    gbQDCursorHidden = 0;
    gbQDHideCursorRequested = 0;
}

// ppc 1.10b: 1005ab70 (1.00 10089e08, 1.13d 10060ec0, carbon 1.10b 103aa000, carbon 1.13d 103d9a80)
// The cursor is hidden by QDProcessEvent, only while it is over the game.
static void sQDHideCursorProc() {
    gbQDHideCursorRequested = 1;
}

// ppc 1.10b: 1005ab60 (1.00 10089dfc, 1.13d 10060eb0, carbon 1.10b 103a9ff0, carbon 1.13d 103d9a70)
static void sQDShowCursorProc() {
    gbQDHideCursorRequested = 0;
}

// ppc 1.10b: 1005ab50 (1.00 10089df4, 1.13d 10060ea0, carbon 1.10b 103a9fe0, carbon 1.13d 103d9a60)
static BOOL QDSuspendContext() {
    return 1;
}

// ppc 1.10b: 1005ab40 (1.00 10089dec, 1.13d 10060e90, carbon 1.10b 103a9fd0, carbon 1.13d 103d9a50)
static BOOL QDResumeContext() {
    return 1;
}

// ppc 1.10b: 1005a7e0 (1.00 10089c70, 1.13d 10060b60, carbon 1.10b 103a9d40, carbon 1.13d 103d97e0)
// Opens (or resizes) the window, builds the window's colour table and palette once, and switches
// the monitor to nDepth (32 falls back to 16 when the device lacks it). Always returns 0.
static short QDCreateContext(int nWidth, int nHeight, int nDepth) {
    unsigned char szTitle[256];

    LogPrintf("QDCreateContext: Initializing context at %dx%dx%d.", nWidth, nHeight, nDepth);
    Rect tSize = {0, 0, (int16_t)(nHeight - 1), (int16_t)nWidth};
    if (!gpQDWindow) {
        GetIndString(szTitle, 150, 1);
        Rect tBounds = {gQDWindowOrigin.v, gQDWindowOrigin.h, (int16_t)(tSize.bottom + gQDWindowOrigin.v),
                        (int16_t)(tSize.right + gQDWindowOrigin.h)};
        gpQDWindow = NewCWindow(nullptr, &tBounds, szTitle, 1, 0x406, nullptr, 0, 0);
    } else {
        SizeWindow(gpQDWindow, tSize.right, tSize.bottom, 0);
    }

    if (!ghQDColorTable) {
        ghQDColorTable = QDNewColorTable();
    }
    if (!ghQDPalette) {
        ghQDPalette = NewPalette(256, ghQDColorTable, pmTolerant | pmExplicit, 0);
        NSetPalette(gpQDWindow, ghQDPalette, pmAllUpdates);
    }

    if ((*(*ghUniDevice)->gdPMap)->pixelSize != nDepth) {
        BOOL bSwitch = 0;
        if (HasDepth(ghUniDevice, (int16_t)nDepth, 0, 0)) {
            bSwitch = 1;
        } else if (nDepth == 32 && HasDepth(ghUniDevice, 16, 0, 0)) {
            LogPrintf("QDCreateContext: Unable to use 32 bit video. Falling back on 16 bit.");
            nDepth = 16;
            bSwitch = 1;
        } else {
            LogPrintf("QDCreateContext: *** Unable to use %d bit video.", nDepth);
        }
        if (bSwitch && SetDepth(ghUniDevice, (int16_t)nDepth, 1, 1)) {
            LogPrintf("QDCreateContext: *** Couldn't switch monitor depth.");
        }
    }

    SetPort(gpQDWindow);
    Rect tPort = gpQDWindow->portRect;
    PaintRect(&tPort);
    return 0;
}

// ppc 1.10b: 1005a4b0 (1.00 10089a48, 1.13d 10060830, carbon 1.10b 103a9a10, carbon 1.13d 103d94b0)
// Rebuilds the window palette from the shared colour table through a gamma ramp (1024 entries
// over the top 10 bits of each component; MathLib pow inside MacBuildGammaTable) and repaints.
void QDForcePaletteUpdate() {
    uint16_t aRamp[0x400];

    MacBuildGammaTable(gnUniGamma * 2 / 3, (double)gnUniContrast, aRamp, 2, 0x400, 0xFFFF);
    const ColorSpec * pSource = (*ghUniColorTable)->ctTable;
    ColorSpec * pDest = (*ghQDColorTable)->ctTable;
    for (int i = 0; i < 256; i++) {
        pDest[i].rgb.red = aRamp[pSource[i].rgb.red >> 6];
        pDest[i].rgb.green = aRamp[pSource[i].rgb.green >> 6];
        pDest[i].rgb.blue = aRamp[pSource[i].rgb.blue >> 6];
    }
    CTabChanged(ghQDColorTable);

    if (!gpQDWindow) {
        LogPrintf("QDForcePaletteUpdate: *** No context.");
        return;
    }
    SetPort(gpQDWindow);
    Rect tPort = gpQDWindow->portRect;
    PaintRect(&tPort);
    CTab2Palette(ghQDColorTable, ghQDPalette, pmTolerant | pmExplicit, 0);
    ActivatePalette(gpQDWindow);
}

// ppc 1.10b: 1005a3f0 (1.00 10089970, 1.13d 10060770, carbon 1.10b 103a9950, carbon 1.13d 103d93f0)
// The window opens 22/24 pixels into the device, below the menu bar on the main screen; the
// device's depth and colour flag are kept for QDClose.
void QDSetGDevice() {
    gQDWindowOrigin.v = (*ghUniDevice)->gdRect.top + 0x16;
    gQDWindowOrigin.h = (*ghUniDevice)->gdRect.left + 0x18;
    if (ghUniDevice == GetMainDevice()) {
        int16_t nMenuBar = GetMBarHeight();
        gQDWindowOrigin.v += nMenuBar ? nMenuBar : 0x14;
    }
    gnQDSavedDepth = (*(*ghUniDevice)->gdPMap)->pixelSize;
    gbQDDeviceIsColor = TestDeviceAttribute(ghUniDevice, gdDevType) == 1;
}

// ppc 1.10b: 1005a3e0 (1.00 10089940, 1.13d 10060760, carbon 1.10b 103a9940, carbon 1.13d 103d93e0)
// QuickDraw is always there (classic 1.00 logs "QDDetect: QuickDraw video driver started up
// successfully." here).
BOOL QDDetect() {
    return 1;
}

// ppc 1.10b: 1005a350 (1.00 100898b8, 1.13d 100606d0, carbon 1.10b 103a98b0, carbon 1.13d 103d9350)
BOOL QDInit(D2GfxSettingsStrc * pSettings, D2GfxHelperStrc * pHelpers) {
    gpUniSettings = pSettings;
    gpUniHelpers = pHelpers;
    vid->pfnSetSuspendResumeProcs(QDSuspendContext, QDResumeContext);
    vid->pfnSetCursorProcs(sQDInitCursorProc, sQDShowCursorProc, sQDHideCursorProc);
    vid->pfnInitCursor();
    gbQDCursorHidden = 0;
    return 1;
}

// ppc 1.10b: 1005a210 (1.00 100897b8, 1.13d 10060590, carbon 1.10b 103a97d0, carbon 1.13d 103d9270)
// Mac slot 2 / Windows 2 Release under QuickDraw: remembers where the window was, disposes it,
// its palette and colour table, and restores the monitor depth.
BOOL QDClose() {
    vid->pfnSetSuspendResumeProcs(nullptr, nullptr);
    vid->pfnSetCursorProcs(MacInitCursor, ShowCursor, HideCursor);
    if (gpQDWindow) {
        SetPort(gpQDWindow);
        gQDWindowOrigin.v = 0;
        gQDWindowOrigin.h = 0;
        LocalToGlobal(&gQDWindowOrigin);
        DisposeWindow(gpQDWindow);
        gpQDWindow = nullptr;
    }
    if (ghQDPalette) {
        DisposePalette(ghQDPalette);
        ghQDPalette = nullptr;
    }
    if (ghQDColorTable) {
        DisposeHandle((Handle)ghQDColorTable);
        ghQDColorTable = nullptr;
    }
    if ((*(*ghUniDevice)->gdPMap)->pixelSize != gnQDSavedDepth) {
        if (SetDepth(ghUniDevice, (int16_t)gnQDSavedDepth, 1, gbQDDeviceIsColor ? 1 : 0)) {
            LogPrintf("QDClose: *** Couldn't switch monitor depth.");
        }
    }
    return 1;
}

// ppc 1.10b: 10059fa0 (1.00 10089524, 1.13d 100602e0, carbon 1.10b 103a9560, carbon 1.13d 103d8fc0)
// Opens the window at the resolution, then the 8-bit (or nDepth) back buffer port over the
// window's colour table.
BOOL QDCreateSurface(int nResolution, int nDepth) {
    int nRowBytes;

    UniEngineSetResolution();
    UniSpriteSetResolution(nResolution);
    gnUniScreenWidth = gaQDScreenWidths[nResolution];
    gnUniScreenHeight = gaQDScreenHeights[nResolution];
    if (QDCreateContext(gnUniScreenWidth, gnUniScreenHeight, nDepth) || !gpQDWindow) {
        LogPrintf("QDCreateSurface: *** QDCreateContext failed.");
        QDDeactivateSurface();
        return 0;
    }

    int nBufferHeight = gaQDBufferHeights[nResolution];
    Rect tBounds = {0, 0, (int16_t)nBufferHeight, (int16_t)gaQDBufferWidths[nResolution]};
    if (NewMemCGrafPort(&tBounds, (int16_t)nDepth, ghQDColorTable, &gpUniBackBuffer, &nRowBytes, &gpQDPort,
                        &ghQDPortDevice)) {
        LogPrintf("QDCreateSurface: *** NewMemCGrafPort failed.");
        QDDeactivateSurface();
        return 0;
    }

    gnUniBackBufferHeight = nBufferHeight;
    gnUniBackBufferDepth = nDepth;
    gnUniBackBufferWidth = (nRowBytes << 3) / nDepth;
    UniClearScreen(0);
    QDForcePaletteUpdate();
    return 1;
}

// ppc 1.10b: 10059f70 (1.13d 100601c0, carbon 1.10b 103a9530, carbon 1.13d 103d8ea0)
// Mac slot 3 / Windows 3 CreateWindow under QuickDraw: always an 8-bit back buffer.
BOOL QDCreateSurfaceEntry(HWND hWnd, int nResolution) {
    return QDCreateSurface(nResolution, 8);
}

// ppc 1.10b: 10059e40 (1.00 10089408, 1.13d 1005ffd0, carbon 1.10b 103a9400, carbon 1.13d 103d8cb0)
// Waits for the frame in flight, then rebuilds the surface or (bRelease) closes it - the
// QDCloseSurface body inlined after a request to end any movie.
void QDEndCutScene(HWND hWnd, int nResolution, BOOL bRelease) {
    while (gbUniInScene) {
        YieldToThinThread(0);
    }

    if (!bRelease) {
        QDCreateSurface(nResolution, 8);
        gbUniSurfaceReleased = 0;
        return;
    }

    gbUniCutsceneQuit = 1;
    QDDeactivateSurface();
    gbUniSurfaceReleased = 1;
}

// ppc 1.10b: 10059cb0 (1.00 1008927c, 1.13d 1005fe40, carbon 1.10b 103a9270, carbon 1.13d 103d8b20)
// Mac slot 11 under QuickDraw: tracks suspend/resume and hides the cursor while it is over the
// game area of an active, uncollapsed window (when the game asked for it hidden).
Boolean QDProcessEvent(EventRecord * pEvent) {
    GrafPtr pSaved;
    Point tWhere = pEvent->where;

    if (!gpQDWindow) {
        return 0;
    }

    GetPort(&pSaved);
    SetPort(gpQDWindow);
    GlobalToLocal(&tWhere);
    if (pEvent->what == osEvt && (pEvent->message >> 24) == suspendResumeMessage) {
        gbUniAppActive = pEvent->message & resumeFlag;
    }
    if (!gbUniAppActive || IsWindowCollapsed(gpQDWindow)) {
        tWhere.v = -1;
        tWhere.h = -1;
    }

    BOOL bInside = tWhere.h >= 0 && tWhere.h < gnUniScreenWidth && tWhere.v >= 0 && tWhere.v < gnUniScreenHeight;
    if (!gbQDCursorHidden) {
        if (gbQDHideCursorRequested && bInside) {
            HideCursor();
            gbQDCursorHidden = 1;
        }
    } else if (!gbQDHideCursorRequested || !bInside) {
        ShowCursor();
        gbQDCursorHidden = 0;
    }

    SetPort(pSaved);
    return 0;
}

// ppc 1.10b: 10059c40 (1.00 10089210, 1.13d 1005fda0, carbon 1.10b 103a9200, carbon 1.13d 103d8a80)
BOOL QDGlobalToLocal(Point * pPoint) {
    GrafPtr pSaved;
    if (!gpQDWindow) {
        return 0;
    }
    GetPort(&pSaved);
    SetPort(gpQDWindow);
    GlobalToLocal(pPoint);
    SetPort(pSaved);
    return 1;
}

// ppc 1.10b: 10059bd0 (1.00 100891a4, 1.13d 1005fd00, carbon 1.10b 103a9190, carbon 1.13d 103d89e0)
BOOL QDLocalToGlobal(Point * pPoint) {
    GrafPtr pSaved;
    if (!gpQDWindow) {
        return 0;
    }
    GetPort(&pSaved);
    SetPort(gpQDWindow);
    LocalToGlobal(pPoint);
    SetPort(pSaved);
    return 1;
}

// ppc 1.10b: 10059ac0 (1.00 10088d74, 1.13d 1005fc10, carbon 1.10b 103a9080, carbon 1.13d 103d88f0)
// Mac slot 16 / Windows 13 BeginCutScene: rebuilds the surface at the Bink depth (32).
BOOL QDCutsceneCreateWindow() {
    if (gbUniSurfaceReleased) {
        return 0;
    }
    QDDeactivateSurface();
    if (!QDCreateSurface(0, gnUniCutsceneDepth)) {
        LogPrintf("QDPlayCutscene: *** Couldn't build a Bink surface.");
        return 0;
    }
    return 1;
}

// ppc 1.10b: 100598d0 (1.00 10088ddc, 1.13d 1005f9e0, carbon 1.10b 103a8ea0, carbon 1.13d 103d86d0)
// Rebuilds the surface at 32 bits unless it already is, blanks the window, and decodes the movie
// into the back buffer; QDBinkBlit presents the changed parts.
void QDPlayCutscene(char * szFile, int nResolution, fnBinkFrameCallback pfnFrame) {
    gbQDInCutscene = 1;
    if (gnUniBackBufferDepth != 32 && !gbUniSurfaceReleased) {
        QDDeactivateSurface();
        if (!QDCreateSurface(0, gnUniCutsceneDepth)) {
            LogPrintf("QDPlayCutscene: *** Couldn't build a Bink surface.");
        }
    }

    SetPort(gpQDWindow);
    Rect tPort = gpQDWindow->portRect;
    PaintRect(&tPort);
    uint32_t dwSurfaceType = BinkGDSurfaceType(ghQDPortDevice);
    int nPitch = gnUniBackBufferWidth * (gnUniBackBufferDepth / 8);
    ZeroMemory(gpUniBackBuffer, gnUniBackBufferHeight * nPitch);
    UniPlayBinkFile(szFile, nResolution, gpUniBackBuffer, nPitch, dwSurfaceType, pfnFrame);

    SetPort(gpQDWindow);
    tPort = gpQDWindow->portRect;
    PaintRect(&tPort);
    gbQDInCutscene = 0;
}

// ppc 1.10b: 100598c0 (1.00 10088d3c, 1.13d 1005f9d0, carbon 1.10b 103a8e90, carbon 1.13d 103d86c0)
BOOL QDCheckCutscene() {
    return gbQDInCutscene;
}

// ppc 1.10b: 100597f0 (1.00 10088c88, 1.13d 1005f910, carbon 1.10b 103a8dc0, carbon 1.13d 103d8600)
BOOL QDCloseSurface() {
    QDDeactivateSurface();
    return 1;
}

// ppc 1.10b: 10059770 (1.00 10088c20, 1.13d 1005f890, carbon 1.10b 103a8d40, carbon 1.13d 103d8580)
BOOL QDStartDraw(BOOL bClear) {
    gbUniInScene = 1;
    gpUniHelpers->nfpFillYBufferTable(gpUniBackBuffer, gnUniBackBufferWidth, gnUniBackBufferHeight, 0);
    if (bClear) {
        UniClearScreen(1);
    }
    return 1;
}

// ppc 1.10b: 10059720 (1.00 10088bd0, 1.13d 1005f840, carbon 1.10b 103a8cf0, carbon 1.13d 103d8530)
BOOL QDEndDraw() {
    gbUniInScene = 0;
    gpUniHelpers->nfpFillYBufferTable(nullptr, 0, 0, 0);
    return 1;
}

// ppc 1.10b: 10059690 (1.00 10088b44, 1.13d 1005f7b0, carbon 1.10b 103a8c40, carbon 1.13d 103d8480)
// Copies the changed rectangle of a Bink frame into the window. As in QDBlit, the source starts
// one row below the destination.
BOOL QDBinkBlit(Rect * pRect) {
    Rect tDest = {pRect->top, pRect->left, (int16_t)(pRect->bottom - 1), pRect->right};
    Rect tSource = {(int16_t)(pRect->top + 1), pRect->left, pRect->bottom, pRect->right};
    SetPort(gpQDWindow);
    CopyBits(QDPortBits(gpQDPort), QDPortBits(gpQDWindow), &tSource, &tDest, srcCopy, nullptr);
    return 1;
}

// ppc 1.10b: 100595a0 (1.00 10088ab4, 1.13d 1005f6c0, carbon 1.10b 103a8b70, carbon 1.13d 103d83b0)
// Windows 8 EndScene2 under QuickDraw: CopyBits of the back buffer port into the window, its
// PixMap locked for the copy. The source rectangle is the port's moved down one row and the
// destination the port's less its last row, as the binary builds them.
BOOL QDBlit() {
    Rect tPort = gpQDPort->portRect;
    Rect tDest = {tPort.top, tPort.left, (int16_t)(tPort.bottom - 1), tPort.right};
    Rect tSource = {(int16_t)(tPort.top + 1), tPort.left, tPort.bottom, tPort.right};
    SetPort(gpQDWindow);
    Handle hPixMap = (Handle)gpQDPort->portPixMap;
    int8_t nState = HGetState(hPixMap);
    HLock(hPixMap);
    CopyBits((const BitMap *)*gpQDPort->portPixMap, QDPortBits(gpQDWindow), &tSource, &tDest, srcCopy, nullptr);
    HSetState(hPixMap, nState);
    return 1;
}

// ppc 1.10b: 10059580 (1.00 10088a98, 1.13d 1005f6a0, carbon 1.10b 103a8b50, carbon 1.13d 103d8390)
void QDGetScreenSize(int * pWidth, int * pHeight) {
    *pWidth = gnUniScreenWidth;
    *pHeight = gnUniScreenHeight;
}

// ppc 1.10b: 100593b0 (1.00 10088800, 1.13d 1005f4d0, carbon 1.10b 103a8980, carbon 1.13d 103d81c0)
// As DSpSetPalette, into the shared colour table; then the window palette is rebuilt.
void QDSetPalette(PALETTEENTRY * pPalette) {
    if (!gpQDWindow) {
        LogPrintf("QDSetPalette: *** Surface does not exist.");
        return;
    }

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
    QDForcePaletteUpdate();
}

// ppc 1.10b: 10059380 (1.00 100887d0, 1.13d 1005f4a0, carbon 1.10b 103a8950, carbon 1.13d 103d8190)
void QDSetPaletteTable(BYTE ** ppTables) {
    CopyMemory(gapUniPaletteTables, ppTables, sizeof(gapUniPaletteTables));
}

// ppc 1.10b: 10059370 (1.00 100887cc, 1.13d 1005f490, carbon 1.10b 103a8940, carbon 1.13d 103d8180)
void QDUpdateScaleFactor(int nScale) {
}

// ppc 1.10b: 10059330 (1.00 1008878c, 1.13d 1005f450, carbon 1.10b 103a8900, carbon 1.13d 103d8140)
void QDSetGamma(int nGamma) {
    if (nGamma < 0x37) {
        nGamma = 0x37;
    }
    if (nGamma > 0xFF) {
        nGamma = 0xFF;
    }
    gnUniGamma = nGamma;
    QDForcePaletteUpdate();
}

// ppc 1.10b: 10059320 (1.00 10088784, 1.13d 1005f440, carbon 1.10b 103a88f0, carbon 1.13d 103d8130)
int QDCheckGamma() {
    return 1;
}

// ppc 1.10b: 10059310 (1.00 10088780, 1.13d 1005f430, carbon 1.10b 103a88e0, carbon 1.13d 103d8120)
void QDSetGlobalLight(int nRed, int nGreen, int nBlue) {
}

// ppc 1.10b: 10059300 (1.00 1008877c, 1.13d 1005f420, carbon 1.10b 103a88d0, carbon 1.13d 103d8110)
void QDSetPerspectiveScale(int nScaleX, int nScaleY) {
}

// ppc 1.10b: 100592f0 (1.00 10088778, 1.13d 1005f410, carbon 1.10b 103a88c0, carbon 1.13d 103d8100)
void QDAdjustPerspectivePosition(int nX, int nY, int nBias, int * pX, int * pY) {
}

// ppc 1.10b: 100592e0 (1.00 10088774, 1.13d 1005f400, carbon 1.10b 103a88b0, carbon 1.13d 103d80f0)
void QDPerspectiveScalePosition(int nX, int nY, int nAngle, int * pX, int * pY, BOOL bOrder) {
}

// ppc 1.10b: 100592d0 (1.00 10088770, 1.13d 1005f3f0, carbon 1.10b 103a88a0, carbon 1.13d 103d80e0)
void QDSetDefaultPerspectiveFactor() {
}

// ppc 1.10b: 100591f0 (1.00 10088698, 1.13d 1005f250, carbon 1.10b 103a87c0, carbon 1.13d 103d7f40)
// Windows 9 ResizeWindow under QuickDraw: the surface torn down and built again at 8 bits.
BOOL QDChangeRes(HWND hWnd, int nResolution) {
    QDDeactivateSurface();
    QDCreateSurface(nResolution, 8);
    return 1;
}

// ppc 1.10b: 100591e0 (1.00 10088690, 1.13d 1005f240, carbon 1.10b 103a87b0, carbon 1.13d 103d7f30)
int * QDGetRenderStatistics() {
    return nullptr;
}

// ppc 1.10b: 10059090 (1.00 1008856c, 1.13d 1005f0f0, carbon 1.10b 103a8670, carbon 1.13d 103d7df0)
// SetOption 1 under QuickDraw: the device holding the largest part of the window.
GDHandle QDGetGDevice() {
    GrafPtr pSaved;
    Rect tWindow = gpQDWindow->portRect;
    GetPort(&pSaved);
    SetPort(gpQDWindow);
    Point tOrigin = {0, 0};
    LocalToGlobal(&tOrigin);
    OffsetRect(&tWindow, tOrigin.h, tOrigin.v);
    SetPort(pSaved);

    int nBestArea = 0;
    GDHandle hBest = GetDeviceList();
    for (GDHandle hDevice = hBest; hDevice; hDevice = GetNextDevice(hDevice)) {
        Rect tDevice = (*hDevice)->gdRect;
        Rect tShared;
        if (SectRect(&tWindow, &tDevice, &tShared)) {
            int nArea = (tShared.right - tShared.left) * (tShared.bottom - tShared.top);
            if (nArea > nBestArea) {
                nBestArea = nArea;
                hBest = hDevice;
            }
        }
    }
    return hBest;
}

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
