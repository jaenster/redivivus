// oglSdl.cpp - the D2OpenGL renderer's display, window, context and swap on Win32Posix and SDL3:
// the Win32Posix build's (D2_BACKEND_POSIX) bodies of the OpenGLMac* functions that oglMac.cpp
// keeps for the Carbon build and for the Win32/WGL restoration.
//
// Ours, not reconstructed: no Mac build has an SDL renderer. The 1.14d Mac renderer looks its
// resolution up in a display mode table the Mac shell fills through SetOption 2 and 3
// (unknown_display.cpp), switches the display to it, captures the displays and opens a full
// screen AGL context, or its own Carbon window when windowed. None of that has a place here:
// the Mac shell is not part of this build, so that table stays empty (which is why the Carbon
// path, reached from here, asked for a 0x0 mode and failed), and the game window is D2gfx's,
// sized by D2gfx for the resolution (D2GFX_CreateWindow, D2GFX_ResizeWindow).
//
// Windowed (-w, SetOption 8 non-zero) the game window is a plain SDL window at 640x480 or
// 800x600. Full screen (the default) the resolution is recorded as the display mode, as the
// Windows renderers set it: Win32Posix never switches the hardware, it shows the game's popup,
// which now covers that reported screen, as an SDL desktop-fullscreen window, and the client
// keeps the game's size. Either way the GL context is an SDL one on the game window (OpenGL 2.1
// in the compatibility profile, the renderer being fixed-function), and a frame is presented
// with SDL_GL_SwapWindow.
//
// The renderer draws 640x480 or 800x600 under its own gluOrtho2D. The viewport is where
// Win32Posix puts that client in the window's drawable (Win32Posix_GetViewport: centred,
// high-DPI pixels), fitted to the renderer's aspect so a full screen of another shape is
// letterboxed, and follows the window when it is resized or goes full screen.
//
// Win32Posix build only.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <SDL3/SDL.h>
#include <string.h>

#include "Source/D2Headers/d2_platform.h"
#include "Source/D2OpenGL/Src/MacSdkOnWin32.h"
#include "Source/D2OpenGL/Src/globals.h"
#include "Source/D2OpenGL/Src/oglMac.h"
#include "Source/D2gfx/Src/D2GFX.h"
#include "Source/Fog/Src/LogManager.h"

namespace D2OpenGL::oglMac {

namespace {

using Fog::Src::LogManager::LOGFILE_WriteStringToFile;

// ChangeDisplaySettingsA's flag and the DEVMODEA fields set (winuser.h, wingdi.h).
enum : DWORD {
    kCdsFullscreen = 0x00000004,
    kDmPelsWidth = 0x00080000,
    kDmPelsHeight = 0x00100000,
};

SDL_Window * gpSdlGlWindow;   // the game window the context is on
SDL_GLContext ghSdlGlContext;
// The viewport last set, in the drawable's bottom-up pixels: x, y, width, height.
int gaSdlGlViewport[4];

// Where the renderer's client lands in the drawable, in GL's bottom-up rows; the client itself
// (the renderer's size from the origin) while Win32Posix has no answer.
void sSdlGlClientInDrawable(int anViewport[4]) {
    int nX, nY, nWidth, nHeight;
    int nDrawableWidth = 0, nDrawableHeight = 0;
    if (Win32Posix_GetViewport(D2gfx::D2GFX::GetHwnd(), &nX, &nY, &nWidth, &nHeight) && nWidth > 0 && nHeight > 0 &&
        SDL_GetWindowSizeInPixels(gpSdlGlWindow, &nDrawableWidth, &nDrawableHeight) && nDrawableHeight > 0) {
        // The renderer's aspect inside that rectangle, centred.
        int nFitWidth = nWidth;
        int nFitHeight = (int)((int64_t)nWidth * INT_005e9e28 / INT_005e9e24);
        if (nFitHeight > nHeight) {
            nFitHeight = nHeight;
            nFitWidth = (int)((int64_t)nHeight * INT_005e9e24 / INT_005e9e28);
        }
        nX += (nWidth - nFitWidth) / 2;
        nY += (nHeight - nFitHeight) / 2;
        anViewport[0] = nX;
        anViewport[1] = nDrawableHeight - (nY + nFitHeight);
        anViewport[2] = nFitWidth;
        anViewport[3] = nFitHeight;
        return;
    }
    anViewport[0] = 0;
    anViewport[1] = 0;
    anViewport[2] = INT_005e9e24;
    anViewport[3] = INT_005e9e28;
}

// Sets the viewport to the client's place in the drawable when that moved (or always).
void sSdlGlFollowWindow(BOOL bAlways) {
    int anViewport[4];
    sSdlGlClientInDrawable(anViewport);
    if (!bAlways && memcmp(anViewport, gaSdlGlViewport, sizeof(anViewport)) == 0) {
        return;
    }
    ::glViewport(anViewport[0], anViewport[1], anViewport[2], anViewport[3]);
    memcpy(gaSdlGlViewport, anViewport, sizeof(gaSdlGlViewport));
}

// A "shot" step of the run's script (D2_POSIX_SCRIPT): the back buffer's client area, before
// the swap, as a BMP at the renderer's resolution.
void sSdlGlTakeShot() {
    char szShot[1024];
    if (!Win32Posix_TakeShotRequest(szShot, sizeof(szShot))) {
        return;
    }
    const int nWidth = gaSdlGlViewport[2];
    const int nHeight = gaSdlGlViewport[3];
    SDL_Surface * pFrame = SDL_CreateSurface(nWidth, nHeight, SDL_PIXELFORMAT_ARGB8888);
    SDL_Surface * pShot = nullptr;
    if (pFrame) {
        // GL rows run bottom-up: read each into its top-down place. A row is whole words, so
        // the pack alignment the renderer set does not matter.
        for (int nRow = 0; nRow < nHeight; nRow++) {
            glReadPixels(gaSdlGlViewport[0], gaSdlGlViewport[1] + nHeight - 1 - nRow, nWidth, 1, GL_BGRA,
                         GL_UNSIGNED_INT_8_8_8_8_REV, (uint8_t *)pFrame->pixels + (size_t)nRow * pFrame->pitch);
        }
        pShot = (nWidth == INT_005e9e24 && nHeight == INT_005e9e28)
                    ? pFrame
                    : SDL_ScaleSurface(pFrame, INT_005e9e24, INT_005e9e28, SDL_SCALEMODE_NEAREST);
    }
    if (!pShot || !SDL_SaveBMP(pShot, szShot)) {
        LOGFILE_WriteStringToFile((char *)"OpenGLSDLSwapBuffers: *** screenshot %s failed. (%s)", szShot, SDL_GetError());
    }
    if (pShot && pShot != pFrame) {
        SDL_DestroySurface(pShot);
    }
    if (pFrame) {
        SDL_DestroySurface(pFrame);
    }
}

// Full screen: the resolution (0 640x480, otherwise 800x600) as the reported display mode.
// Win32Posix records it and never switches the hardware; the game's popup of that size then
// covers the reported screen and is shown desktop-fullscreen.
BOOL sSdlGlSetFullscreenMode(int nResolution) {
    DEVMODEA tMode;
    memset(&tMode, 0, sizeof(tMode));
    tMode.dmSize = sizeof(tMode);
    tMode.dmFields = kDmPelsWidth | kDmPelsHeight;
    tMode.dmPelsWidth = nResolution == 0 ? 640 : 800;
    tMode.dmPelsHeight = nResolution == 0 ? 480 : 600;
    LONG nResult = ChangeDisplaySettingsA(&tMode, kCdsFullscreen);
    if (nResult) {
        LOGFILE_WriteStringToFile((char *)"OpenGLSDLSetMode: *** ChangeDisplaySettings (%dx%d) failed. (%d)",
                                  (int)tMode.dmPelsWidth, (int)tMode.dmPelsHeight, (int)nResult);
        return 0;
    }
    return 1;
}

}  // namespace

// Full screen: the desktop's mode reported again, which ends the desktop-fullscreen window.
BOOL OGL_RestoreDisplayMode() {
    if (DAT_00552b24 == 1) {
        ChangeDisplaySettingsA(nullptr, 0);
    }
    return 1;
}

// Full screen: the resolution as the reported mode. Windowed: nothing, D2gfx sized the window.
BOOL OGL_MacCreateSurface(int nResolution) {
    if (DAT_00552b24 != 1) {
        return 1;
    }
    return sSdlGlSetFullscreenMode(nResolution);
}

// The resolution changed (after D2gfx's SetWindowPos): full screen, the new resolution as the
// reported mode. The Carbon path's failure value is -50 (paramErr).
short OGL_MacSwapContext(int nResolution, int nColorBits, int nUnused) {
    if (DAT_00552b24 != 1) {
        return 0;
    }
    return sSdlGlSetFullscreenMode(nResolution) ? 0 : -50;
}

// The context on the game window, made current, and the viewport on the client. The colour
// depth is SetOption 5's (16 or 32 bits).
BOOL OGL_MacOpenWindow(int nResolution) {
    SDL_Window * pWindow = Win32Posix_GetSdlWindow(D2gfx::D2GFX::GetHwnd());
    if (!pWindow) {
        LOGFILE_WriteStringToFile((char *)"OpenGLSDLOpenWindow: *** The game window has no SDL window.");
        return 0;
    }
    if (!(SDL_GetWindowFlags(pWindow) & SDL_WINDOW_OPENGL)) {
        LOGFILE_WriteStringToFile((char *)"OpenGLSDLOpenWindow: *** The game window was not made for OpenGL.");
        return 0;
    }
    OGL_MacCloseWindow();

    const int nChannelBits = DAT_003ce7ec <= 16 ? 5 : 8;
    SDL_GL_ResetAttributes();
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, nChannelBits);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, nChannelBits == 5 ? 6 : 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, nChannelBits);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GLContext hContext = SDL_GL_CreateContext(pWindow);
    if (!hContext) {
        LOGFILE_WriteStringToFile((char *)"OpenGLSDLOpenWindow: *** SDL_GL_CreateContext failed. (%s)", SDL_GetError());
        return 0;
    }
    if (!SDL_GL_MakeCurrent(pWindow, hContext)) {
        LOGFILE_WriteStringToFile((char *)"OpenGLSDLOpenWindow: *** SDL_GL_MakeCurrent failed. (%s)", SDL_GetError());
        SDL_GL_DestroyContext(hContext);
        return 0;
    }
    gpSdlGlWindow = pWindow;
    ghSdlGlContext = hContext;

    // Win32Posix delivers the mouse in client coordinates: the client's origin is 0, 0.
    SHORT_ARRAY_00552b38[0] = 0;
    SHORT_ARRAY_00552b38[1] = 0;
    sSdlGlFollowWindow(1);
    int nDrawableWidth = 0, nDrawableHeight = 0;
    SDL_GetWindowSizeInPixels(pWindow, &nDrawableWidth, &nDrawableHeight);
    LOGFILE_WriteStringToFile((char *)"OpenGLSDLOpenWindow: %dx%d %s, viewport %dx%d at %d,%d of %dx%d.", INT_005e9e24,
                              INT_005e9e28, (SDL_GetWindowFlags(pWindow) & SDL_WINDOW_FULLSCREEN) ? "full screen" : "windowed",
                              gaSdlGlViewport[2], gaSdlGlViewport[3], gaSdlGlViewport[0], gaSdlGlViewport[1],
                              nDrawableWidth, nDrawableHeight);
    // What OGL_MacSetRenderer records for an accelerated renderer, and the open resolution.
    DAT_00552b20 = 1;
    DAT_003ce7e8 = nResolution;
    return 1;
}

void OGL_MacCloseWindow() {
    if (!ghSdlGlContext) {
        return;
    }
    if (SDL_GL_GetCurrentContext() == ghSdlGlContext) {
        SDL_GL_MakeCurrent(gpSdlGlWindow, nullptr);
    }
    SDL_GL_DestroyContext(ghSdlGlContext);
    ghSdlGlContext = nullptr;
    gpSdlGlWindow = nullptr;
}

void OGL_MacSwapBuffers() {
    if (!ghSdlGlContext) {
        return;
    }
    sSdlGlTakeShot();
    SDL_GL_SwapWindow(gpSdlGlWindow);
    sSdlGlFollowWindow(0);
}

}  // namespace D2OpenGL::oglMac

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
