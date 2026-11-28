// sdlCore.cpp - D2Uni's SDL3 back end, the Win32Posix build's (D2_BACKEND_POSIX) only one: the
// game draws into the shared 8-bit back buffer as it does under DrawSprocket and QuickDraw, and
// SdlBlit converts it through the palette into an ARGB8888 streaming texture, which the SDL
// renderer scales into the window's client area (Win32Posix_GetViewport: aspect kept, centred)
// with vsync as the settings ask. Windowed and full screen are the same path: Win32Posix sizes
// the window, the viewport follows it.
//
// Ours, not reconstructed: no Mac build has an SDL back end. It keeps the QuickDraw back end's
// (qdCore.cpp) shape - the surface teardown, the 640x480 and 800x600 buffers, the cutscene
// surface at 32 bits, the palette compare - so the shared slots see the same state.
//
// Byte order. The 8-bit buffer is one byte per pixel and the palette entries are bytes, so
// neither has an order. The converted frame is written as 32-bit ARGB words (alpha in the top
// byte), which is what SDL_PIXELFORMAT_ARGB8888 means on either byte order: no pixel is ever
// written byte by byte. The one byte-addressed surface is the Bink cutscene's (BINKSURFACE32:
// B, G, R, A bytes), which reads as those same words only on a little-endian host; that is
// asserted below.
//
// Win32Posix build only.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <SDL3/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Source/D2Uni/Src/uniInternal.h"

static_assert(SDL_BYTEORDER == SDL_LIL_ENDIAN,
              "the Bink cutscene surface (B, G, R, A bytes) is read as ARGB8888 words");

// The Win32 messages the cutscene's quit test takes off the queue (winuser.h).
enum : UINT {
    kSdlWmKeyFirst = 0x0100,  // WM_KEYDOWN
    kSdlWmKeyLast = 0x0109,   // WM_UNICHAR
    kSdlWmKeyDown = 0x0100,
    kSdlWmSysKeyDown = 0x0104,
    kSdlWmMouseFirst = 0x0200, // WM_MOUSEMOVE
    kSdlWmMouseLast = 0x020E,  // WM_MOUSEHWHEEL
    kSdlWmLButtonDown = 0x0201,
    kSdlWmRButtonDown = 0x0204,
    kSdlWmMButtonDown = 0x0207,
};

// Window and back buffer sizes by resolution (0 = 640x480, 1 = 800x600), as QuickDraw's.
static const int gaSdlScreenWidths[2] = {640, 800};
static const int gaSdlScreenHeights[2] = {480, 600};

static HWND ghSdlWindow;               // the game window the surface was made for
static SDL_Renderer * gpSdlRenderer;
static SDL_Texture * gpSdlTexture;
static uint32_t * gpSdlFrame;          // the converted frame, gnUniScreenWidth ARGB words a row
static uint32_t gaSdlPalette[256];     // the colour table as ARGB words
static BOOL gbSdlInCutscene;

// ---- the log and the halt (the Mac platform's LogPrintf and MacFatal) ----

void LogPrintf(const char * szFormat, ...) {
    char szLine[1024];
    va_list va;
    va_start(va, szFormat);
    vsnprintf(szLine, sizeof(szLine), szFormat, va);
    va_end(va);
    fprintf(stderr, "D2Uni: %s\n", szLine);
}

void MacFatal(const char * szMessage) {
    fprintf(stderr, "D2Uni: fatal: %s\n", szMessage);
    abort();
}

// ---- the palette ----

// Rebuilds the ARGB words from the shared colour table (16-bit components, the top byte the
// colour). Gamma is not applied: the Windows windowed renderer (D2Gdi) applies none either, and
// SdlCheckGamma says so.
static void sSdlRebuildPalette() {
    if (!ghUniColorTable) {
        return;
    }
    const ColorSpec * pTable = (*ghUniColorTable)->ctTable;
    for (int i = 0; i < 256; i++) {
        gaSdlPalette[i] = 0xFF000000u | (uint32_t)(pTable[i].rgb.red >> 8) << 16 |
                          (uint32_t)(pTable[i].rgb.green >> 8) << 8 | (uint32_t)(pTable[i].rgb.blue >> 8);
    }
}

// ---- the surface ----

// Drops the back buffer, the frame and the texture and every size, and tells D2gfx there is no
// buffer (QuickDraw's QDDeactivateSurface). The renderer stays with the window.
static void sSdlDeactivateSurface() {
    if (gpSdlTexture) {
        SDL_DestroyTexture(gpSdlTexture);
        gpSdlTexture = nullptr;
    }
    free(gpUniBackBuffer);
    gpUniBackBuffer = nullptr;
    free(gpSdlFrame);
    gpSdlFrame = nullptr;
    gnUniBackBufferDepth = 0;
    gnUniScreenHeight = 0;
    gnUniScreenWidth = 0;
    gnUniBackBufferHeight = 0;
    gnUniBackBufferWidth = 0;
    UniEngineReset();
    gpUniHelpers->nfpFillYBufferTable(nullptr, 0, 0, 0);
}

static void sSdlDestroyRenderer() {
    if (gpSdlRenderer) {
        SDL_DestroyRenderer(gpSdlRenderer);
        gpSdlRenderer = nullptr;
    }
}

// Builds the back buffer (8 bits, or 32 for a cutscene) at the resolution, the texture it is
// converted into, and the renderer on the game window when there is none yet.
static BOOL sSdlCreateSurface(int nResolution, int nDepth) {
    if (nResolution < 0 || nResolution > 1) {
        LogPrintf("SDLCreateSurface: *** No %d resolution.", nResolution);
        return 0;
    }

    UniEngineSetResolution();
    UniSpriteSetResolution(nResolution);
    gnUniScreenWidth = gaSdlScreenWidths[nResolution];
    gnUniScreenHeight = gaSdlScreenHeights[nResolution];

    SDL_Window * pWindow = Win32Posix_GetSdlWindow(ghSdlWindow);
    if (!pWindow) {
        LogPrintf("SDLCreateSurface: *** The window has no SDL window.");
        sSdlDeactivateSurface();
        return 0;
    }
    if (!gpSdlRenderer) {
        gpSdlRenderer = SDL_CreateRenderer(pWindow, nullptr);
        if (!gpSdlRenderer) {
            LogPrintf("SDLCreateSurface: *** SDL_CreateRenderer failed: %s", SDL_GetError());
            sSdlDeactivateSurface();
            return 0;
        }
        LogPrintf("SDLCreateSurface: Renderer \"%s\".", SDL_GetRendererName(gpSdlRenderer));
    }
    SDL_SetRenderVSync(gpSdlRenderer, gpUniSettings && gpUniSettings->bVSync ? 1 : 0);

    gpSdlTexture = SDL_CreateTexture(gpSdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                     gnUniScreenWidth, gnUniScreenHeight);
    if (!gpSdlTexture) {
        LogPrintf("SDLCreateSurface: *** SDL_CreateTexture failed: %s", SDL_GetError());
        sSdlDeactivateSurface();
        return 0;
    }
    // Each game pixel a whole block of screen pixels, as D2Gdi's StretchBlt.
    SDL_SetTextureScaleMode(gpSdlTexture, SDL_SCALEMODE_NEAREST);

    size_t nPixels = (size_t)gnUniScreenWidth * (size_t)gnUniScreenHeight;
    gpUniBackBuffer = (BYTE *)calloc(nPixels, (size_t)nDepth / 8);
    gpSdlFrame = (uint32_t *)calloc(nPixels, sizeof(uint32_t));
    if (!gpUniBackBuffer || !gpSdlFrame) {
        LogPrintf("SDLCreateSurface: *** Out of memory for a %dx%dx%d surface.", gnUniScreenWidth, gnUniScreenHeight,
                  nDepth);
        sSdlDeactivateSurface();
        return 0;
    }

    // gnUniBackBufferWidth is the row in pixels, as QuickDraw's (rowBytes * 8 / depth).
    gnUniBackBufferWidth = gnUniScreenWidth;
    gnUniBackBufferHeight = gnUniScreenHeight;
    gnUniBackBufferDepth = nDepth;
    if (nDepth == 8) {
        UniClearScreen(0);
    }
    sSdlRebuildPalette();
    return 1;
}

// Converts the back buffer into gpSdlFrame: through the palette at 8 bits, copied at 32 (the
// Bink surface, ARGB words already).
static void sSdlConvertFrame() {
    int nWidth = gnUniScreenWidth;
    int nHeight = gnUniScreenHeight;
    if (gnUniBackBufferDepth == 8) {
        for (int nRow = 0; nRow < nHeight; nRow++) {
            const BYTE * pSrc = gpUniBackBuffer + (size_t)nRow * (size_t)gnUniBackBufferWidth;
            uint32_t * pDst = gpSdlFrame + (size_t)nRow * (size_t)nWidth;
            for (int nCol = 0; nCol < nWidth; nCol++) {
                pDst[nCol] = gaSdlPalette[pSrc[nCol]];
            }
        }
    } else if (gnUniBackBufferDepth == 32) {
        memcpy(gpSdlFrame, gpUniBackBuffer, (size_t)nWidth * (size_t)nHeight * sizeof(uint32_t));
    }
}

// Uploads the frame and scales it into the client area, black around it.
static BOOL sSdlPresent() {
    if (!gpSdlRenderer || !gpSdlTexture || !gpUniBackBuffer) {
        return 0;
    }
    sSdlConvertFrame();
    SDL_UpdateTexture(gpSdlTexture, nullptr, gpSdlFrame, gnUniScreenWidth * (int)sizeof(uint32_t));

    SDL_SetRenderDrawColor(gpSdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gpSdlRenderer);
    int nX, nY, nWidth, nHeight;
    if (Win32Posix_GetViewport(ghSdlWindow, &nX, &nY, &nWidth, &nHeight) && nWidth > 0 && nHeight > 0) {
        SDL_FRect tDest = {(float)nX, (float)nY, (float)nWidth, (float)nHeight};
        SDL_RenderTexture(gpSdlRenderer, gpSdlTexture, nullptr, &tDest);
    } else {
        SDL_RenderTexture(gpSdlRenderer, gpSdlTexture, nullptr, nullptr);
    }
    SDL_RenderPresent(gpSdlRenderer);
    return 1;
}

// ---- the back-end entry points (uniCore.cpp dispatches to these) ----

BOOL SdlDetect() {
    return SDL_GetNumRenderDrivers() > 0;
}

BOOL SdlInit(D2GfxSettingsStrc * pSettings, D2GfxHelperStrc * pHelpers) {
    gpUniSettings = pSettings;
    gpUniHelpers = pHelpers;
    // The game window is made after this, by D2gfx: no GL context on it, the SDL renderer picks
    // its own driver.
    Win32Posix_SetWindowCreateFlags(0);
    return 1;
}

BOOL SdlRelease() {
    if (gpUniBackBuffer || gpSdlTexture) {
        sSdlDeactivateSurface();
    }
    sSdlDestroyRenderer();
    ghSdlWindow = nullptr;
    return 1;
}

BOOL SdlCreateSurface(HWND hWnd, int nResolution) {
    ghSdlWindow = hWnd;
    return sSdlCreateSurface(nResolution, 8);
}

// The window is about to go (D2gfx destroys it next): the renderer goes with the surface.
BOOL SdlCloseSurface() {
    LogPrintf("SDLCloseSurface: Closing SDL context.");
    sSdlDeactivateSurface();
    sSdlDestroyRenderer();
    return 1;
}

BOOL SdlChangeRes(HWND hWnd, int nResolution) {
    sSdlDeactivateSurface();
    ghSdlWindow = hWnd;
    sSdlCreateSurface(nResolution, 8);
    return 1;
}

// Rebuilds the 8-bit surface after a cutscene, or (bRelease) drops it. QuickDraw first waits for
// the frame in flight on its thread; the game draws on this one.
void SdlEndCutScene(HWND hWnd, int nResolution, BOOL bRelease) {
    if (!bRelease) {
        ghSdlWindow = hWnd;
        sSdlDeactivateSurface();
        sSdlCreateSurface(nResolution, 8);
        gbUniSurfaceReleased = 0;
        return;
    }

    gbUniCutsceneQuit = 1;
    sSdlDeactivateSurface();
    gbUniSurfaceReleased = 1;
}

// BeginCutScene: the surface rebuilt at the Bink depth.
BOOL SdlCutsceneCreateWindow() {
    if (gbUniSurfaceReleased) {
        return 0;
    }
    sSdlDeactivateSurface();
    if (!sSdlCreateSurface(0, gnUniCutsceneDepth)) {
        LogPrintf("SDLPlayCutscene: *** Couldn't build a Bink surface.");
        return 0;
    }
    return 1;
}

void SdlPlayCutscene(char * szFile, int nResolution, fnBinkFrameCallback pfnFrame) {
    gbSdlInCutscene = 1;
    if (gnUniBackBufferDepth != 32 && !gbUniSurfaceReleased) {
        sSdlDeactivateSurface();
        if (!sSdlCreateSurface(0, gnUniCutsceneDepth)) {
            LogPrintf("SDLPlayCutscene: *** Couldn't build a Bink surface.");
        }
    }
    if (gpUniBackBuffer) {
        int nPitch = gnUniBackBufferWidth * (gnUniBackBufferDepth / 8);
        ZeroMemory(gpUniBackBuffer, gnUniBackBufferHeight * nPitch);
        UniPlayBinkFile(szFile, nResolution, gpUniBackBuffer, nPitch, BINKSURFACE32, pfnFrame);
    }
    gbSdlInCutscene = 0;
}

BOOL SdlCheckCutscene() {
    return gbSdlInCutscene;
}

BOOL SdlStartDraw(BOOL bClear) {
    gbUniInScene = 1;
    gpUniHelpers->nfpFillYBufferTable(gpUniBackBuffer, gnUniBackBufferWidth, gnUniBackBufferHeight, 0);
    if (bClear) {
        UniClearScreen(1);
    }
    return 1;
}

BOOL SdlEndDraw() {
    gbUniInScene = 0;
    gpUniHelpers->nfpFillYBufferTable(nullptr, 0, 0, 0);
    return 1;
}

BOOL SdlBlit() {
    return sSdlPresent();
}

// A Bink frame changed: the whole frame is presented (the 64-bit Bink keeps no dirty rectangles).
BOOL SdlBinkBlit(Rect * pRect) {
    (void)pRect;
    return sSdlPresent();
}

void SdlGetScreenSize(int * pWidth, int * pHeight) {
    *pWidth = gnUniScreenWidth;
    *pHeight = gnUniScreenHeight;
}

// As QDSetPalette: from the first entry that differs, into the shared colour table (each 8-bit
// component widened to 16 by repeating it), then the ARGB words.
void SdlSetPalette(PALETTEENTRY * pPalette) {
    if (!ghUniColorTable) {
        LogPrintf("SDLSetPalette: *** No colour table.");
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
    sSdlRebuildPalette();
}

// D2gfx's table of palette-table pointers, copied whole: its size is the array's, so the copy is
// pointer-width on a 64-bit target.
void SdlSetPaletteTable(BYTE ** ppTables) {
    CopyMemory(gapUniPaletteTables, ppTables, sizeof(gapUniPaletteTables));
}

// Kept in QuickDraw's range; no colour changes with it (SdlCheckGamma says 0, as D2Gdi's does).
void SdlSetGamma(int nGamma) {
    if (nGamma < 0x37) {
        nGamma = 0x37;
    }
    if (nGamma > 0xFF) {
        nGamma = 0xFF;
    }
    gnUniGamma = nGamma;
}

int SdlCheckGamma() {
    return 0;
}

int * SdlGetRenderStatistics() {
    return nullptr;
}

// The Bink loop's quit test: a key or mouse button press waiting (QuickDraw asks OSEventAvail),
// taken off the queue with every other key and mouse message behind it (FlushEvents).
BOOL SdlInputPending() {
    MSG tMsg;
    BOOL bPressed = 0;
    while (PeekMessageA(&tMsg, nullptr, kSdlWmKeyFirst, kSdlWmKeyLast, PM_REMOVE)) {
        if (tMsg.message == kSdlWmKeyDown || tMsg.message == kSdlWmSysKeyDown) {
            bPressed = 1;
        }
    }
    while (PeekMessageA(&tMsg, nullptr, kSdlWmMouseFirst, kSdlWmMouseLast, PM_REMOVE)) {
        if (tMsg.message == kSdlWmLButtonDown || tMsg.message == kSdlWmRButtonDown ||
            tMsg.message == kSdlWmMButtonDown) {
            bPressed = 1;
        }
    }
    return bPressed;
}

#endif // !defined(_WIN32) && D2_BACKEND_POSIX
