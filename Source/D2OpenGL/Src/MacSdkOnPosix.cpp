// Restoration code, not in any binary: the Mac system calls of the D2OpenGL renderer, on the
// Win32Posix layer and SDL3 (see MacSdkOnWin32.h). The modern Mac build only; Windows has
// MacSdkOnWin32.cpp, the 1.14d Mac the real frameworks.
//
// The window is the game's (D2GFX GetHwnd): the SDL window Win32Posix made for it with
// SDL_WINDOW_OPENGL. An AGL context is an SDL GL context on that window: OpenGL 2.1 in the
// compatibility profile (the renderer is fixed-function GL), double-buffered, made by
// SDL_GL_CreateContext and presented by SDL_GL_SwapWindow. GL entry points beyond the system
// library's export list come from SDL_GL_GetProcAddress.
//
// The displays are Win32Posix's one reported screen. A display mode is a DEVMODEA record;
// setting one records it (ChangeDisplaySettingsA) and never switches the hardware, and the game
// window over that whole screen becomes a desktop-fullscreen window whose client is scaled to
// fit. Any size scales, so the best mode for a request is the request itself. The renderer
// sets its viewport in client pixels; glViewport below maps it to where the client lands in the
// drawable (Win32Posix_GetViewport: aspect kept, centred, high-DPI pixels).

#if !defined(_WIN32) && D2_BACKEND_POSIX

#include "Source/D2Headers/d2_platform.h"
#include "Source/D2OpenGL/Src/MacSdkOnWin32.h"

#include <SDL3/SDL.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <new>

#include "Source/D2gfx/Src/D2GFX.h"
#include "Source/Fog/Src/ErrorManager.h"
#include "Source/D2CMP/Src/SpriteCache.h"

namespace D2OpenGL {

namespace {

// AGL attribute and renderer property numbers (AGL/agl.h)
enum {
    AGL_NONE = 0,
    AGL_RGBA = 4,
    AGL_DOUBLEBUFFER = 5,
    AGL_PIXEL_SIZE = 50,
    AGL_FULLSCREEN = 54,
    AGL_RENDERER_ID = 70,
    AGL_NO_RECOVERY = 72,
    AGL_ACCELERATED = 73,
};

// The Win32 numbers this file passes to Win32Posix (winuser.h)
enum : UINT {
    kSmCxScreen = 0,
    kSmCyScreen = 1,
    kSwShow = 5,
    kSwpNoZOrder = 0x0004,
    kCdsFullscreen = 0x00000004,
    kEnumCurrentSettings = 0xFFFFFFFFu,
    kDmBitsPerPel = 0x00040000,
    kDmPelsWidth = 0x00080000,
    kDmPelsHeight = 0x00100000,
    kDmDisplayFrequency = 0x00400000,
};

// The one renderer the system GL is: the id any requested id matches.
const int kRendererId = 0x00022000;

struct PixelFormat {
    int nColorBits;
    bool bDoubleBuffer;
    bool bAccelerated;
};

struct Context {
    PixelFormat format;
    SDL_Window *pWindow;
    SDL_GLContext hGL;
};

struct RendererInfo {
    bool bAccelerated;
};

// The last error: an SDL message, since every failing call here is an SDL one.
GLenum s_nAglError;
char s_szAglError[256];

void SetAglError(const char *szWhat)
{
    s_nAglError = 1;
    snprintf(s_szAglError, sizeof(s_szAglError), "%s: %s", szWhat, SDL_GetError());
}

HWND GameWindow()
{
    return D2gfx::D2GFX::GetHwnd();
}

int ScreenWidth()
{
    return GetSystemMetrics(kSmCxScreen);
}

int ScreenHeight()
{
    return GetSystemMetrics(kSmCyScreen);
}

DEVMODEA *NewMode(const DEVMODEA &dm)
{
    return new (std::nothrow) DEVMODEA(dm);
}

template <typename T> DEVMODEA *Mode(T hMode)
{
    return reinterpret_cast<DEVMODEA *>(const_cast<void *>(static_cast<const void *>(hMode)));
}

// ---- the viewport: the renderer's, in client pixels, and where it went in the drawable

struct Viewport {
    GLint nX, nY;
    GLsizei nWidth, nHeight;
};

Context *s_pCurrent;
bool s_bViewportSet;
Viewport s_viewport;
// The client's place in the drawable when the viewport was last mapped (GL's bottom-up rows).
int s_anMapped[5];

// The client's rectangle in the drawable in GL's bottom-up rows, and the client size.
bool ClientInDrawable(SDL_Window *pWindow, int anOut[5], int *pnClientWidth, int *pnClientHeight)
{
    HWND hWnd = GameWindow();
    int nX, nY, nWidth, nHeight;
    RECT rcClient;
    int nDrawableWidth = 0, nDrawableHeight = 0;
    if (!Win32Posix_GetViewport(hWnd, &nX, &nY, &nWidth, &nHeight) || !GetClientRect(hWnd, &rcClient) ||
        !SDL_GetWindowSizeInPixels(pWindow, &nDrawableWidth, &nDrawableHeight))
        return false;
    *pnClientWidth = rcClient.right - rcClient.left;
    *pnClientHeight = rcClient.bottom - rcClient.top;
    if (*pnClientWidth <= 0 || *pnClientHeight <= 0 || nWidth <= 0 || nHeight <= 0)
        return false;
    anOut[0] = nX;
    anOut[1] = nDrawableHeight - (nY + nHeight);
    anOut[2] = nWidth;
    anOut[3] = nHeight;
    anOut[4] = nDrawableHeight;
    return true;
}

void ApplyViewport()
{
    if (!s_bViewportSet)
        return;
    int anClient[5];
    int nClientWidth, nClientHeight;
    if (s_pCurrent == NULL || s_pCurrent->pWindow == NULL ||
        !ClientInDrawable(s_pCurrent->pWindow, anClient, &nClientWidth, &nClientHeight)) {
        ::glViewport(s_viewport.nX, s_viewport.nY, s_viewport.nWidth, s_viewport.nHeight);
        memset(s_anMapped, 0, sizeof(s_anMapped));
        return;
    }
    double fScaleX = (double)anClient[2] / nClientWidth;
    double fScaleY = (double)anClient[3] / nClientHeight;
    ::glViewport(anClient[0] + (GLint)(s_viewport.nX * fScaleX + 0.5), anClient[1] + (GLint)(s_viewport.nY * fScaleY + 0.5),
                 (GLsizei)(s_viewport.nWidth * fScaleX + 0.5), (GLsizei)(s_viewport.nHeight * fScaleY + 0.5));
    memcpy(s_anMapped, anClient, sizeof(s_anMapped));
}

// The window was resized or went fullscreen since the viewport was mapped: map it again.
void FollowWindow()
{
    int anClient[5];
    int nClientWidth, nClientHeight;
    if (!s_bViewportSet || s_pCurrent == NULL || s_pCurrent->pWindow == NULL ||
        !ClientInDrawable(s_pCurrent->pWindow, anClient, &nClientWidth, &nClientHeight))
        return;
    if (memcmp(anClient, s_anMapped, sizeof(s_anMapped)) != 0)
        ApplyViewport();
}

// ---- QuickDraw GDevice records: one, the screen

GDevice s_device;
GDevice *s_pDevice = &s_device;

GDHandle ScreenDevice()
{
    s_device.gdRect.top = 0;
    s_device.gdRect.left = 0;
    s_device.gdRect.bottom = (int16_t)ScreenHeight();
    s_device.gdRect.right = (int16_t)ScreenWidth();
    return &s_pDevice;
}

// ---- the Carbon event masks as input messages (mDown 0x2, mUp 0x4, keyDown 0x8, keyUp 0x10, autoKey 0x20)

struct EventClass {
    uint16_t wMask;
    UINT nFirst, nLast;
};

// WM_LBUTTONDOWN 0x201, WM_RBUTTONDOWN 0x204, WM_MBUTTONDOWN 0x207, the ups one above each;
// WM_KEYDOWN 0x100, WM_KEYUP 0x101, WM_SYSKEYDOWN 0x104, WM_SYSKEYUP 0x105.
const EventClass s_aEventClasses[] = {
    {0x02, 0x201, 0x201}, {0x02, 0x204, 0x204}, {0x02, 0x207, 0x207}, {0x04, 0x202, 0x202}, {0x04, 0x205, 0x205},
    {0x04, 0x208, 0x208}, {0x28, 0x100, 0x100}, {0x28, 0x104, 0x104}, {0x10, 0x101, 0x101}, {0x10, 0x105, 0x105},
};

// ---- the open file a BinkOpen follows

void *s_hBinkFile;

void NoOperation(void)
{
}

// The Mac application's procedure table (0xc8 bytes and more on i386, read by slot); every slot
// does nothing.
void (*s_apfnAppProcs[64])(void) = {
    NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation,
    NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation,
    NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation,
    NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation,
    NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation,
    NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation,
    NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation,
    NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation, NoOperation,
};

// An entry point through SDL_GL_GetProcAddress, looked up once a context is current; NULL
// when the GL has none.
template <typename Fn> Fn GlProc(Fn *ppfn, bool *pbLooked, const char *szName, const char *szAlternative)
{
    if (!*pbLooked && SDL_GL_GetCurrentContext() != NULL) {
        SDL_FunctionPointer pfn = SDL_GL_GetProcAddress(szName);
        if (pfn == NULL && szAlternative != NULL)
            pfn = SDL_GL_GetProcAddress(szAlternative);
        *ppfn = reinterpret_cast<Fn>(pfn);
        *pbLooked = true;
    }
    return *ppfn;
}

}  // namespace

void *PTR_DAT_00397abc = s_apfnAppProcs;
uintptr_t __stack_chk_guard = 0xBB40E64E;
const CFStringRef kCGColorBlack = reinterpret_cast<CFStringRef>(1);
const CGPoint CGPointZero = {0.0f, 0.0f};

void POSIX_PrepareGameWindowForGL(void)
{
    Win32Posix_SetWindowCreateFlags(SDL_WINDOW_OPENGL);
}

// ---- AGL

AGLPixelFormat aglChoosePixelFormat(AGLDevice *, GLint, GLint *pAttribs)
{
    PixelFormat format = {32, false, true};
    for (const GLint *p = pAttribs; p != NULL && *p != AGL_NONE; p++) {
        switch (*p) {
        case AGL_DOUBLEBUFFER: format.bDoubleBuffer = true; break;
        case AGL_PIXEL_SIZE: format.nColorBits = *++p; break;
        case AGL_RENDERER_ID: ++p; break;
        default: break;
        }
    }
    PixelFormat *pFormat = new (std::nothrow) PixelFormat(format);
    return reinterpret_cast<AGLPixelFormat>(pFormat);
}

void aglDestroyPixelFormat(AGLPixelFormat hFormat)
{
    delete reinterpret_cast<PixelFormat *>(hFormat);
}

GLboolean aglDescribePixelFormat(AGLPixelFormat hFormat, GLint nAttrib, GLint *pValue)
{
    const PixelFormat *pFormat = reinterpret_cast<const PixelFormat *>(hFormat);
    if (pFormat == NULL)
        return 0;
    switch (nAttrib) {
    case AGL_RENDERER_ID: *pValue = kRendererId; return 1;
    case AGL_ACCELERATED: *pValue = pFormat->bAccelerated; return 1;
    case AGL_PIXEL_SIZE: *pValue = pFormat->nColorBits; return 1;
    default: *pValue = 0; return 1;
    }
}

// The context is made when it is first attached to the window; the pixel format is copied, as
// the renderer destroys it once the context is current.
AGLContext aglCreateContext(AGLPixelFormat hFormat, AGLContext)
{
    const PixelFormat *pFormat = reinterpret_cast<const PixelFormat *>(hFormat);
    if (pFormat == NULL)
        return 0;
    Context *pContext = new (std::nothrow) Context();
    if (pContext == NULL)
        return 0;
    pContext->format = *pFormat;
    return reinterpret_cast<AGLContext>(pContext);
}

// Attach the context to the game's window: an OpenGL 2.1 compatibility context on its SDL window.
static uint8_t AttachContext(Context *pContext)
{
    if (pContext == NULL)
        return 0;
    if (pContext->hGL != NULL)
        return 1;
    SDL_Window *pWindow = Win32Posix_GetSdlWindow(GameWindow());
    if (pWindow == NULL) {
        s_nAglError = 1;
        snprintf(s_szAglError, sizeof(s_szAglError), "the game window has no SDL window");
        return 0;
    }
    if (!(SDL_GetWindowFlags(pWindow) & SDL_WINDOW_OPENGL)) {
        s_nAglError = 1;
        snprintf(s_szAglError, sizeof(s_szAglError), "the game window was not made for OpenGL");
        return 0;
    }
    int nChannelBits = pContext->format.nColorBits <= 16 ? 5 : 8;
    SDL_GL_ResetAttributes();
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, nChannelBits);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, nChannelBits == 5 ? 6 : 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, nChannelBits);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, pContext->format.bDoubleBuffer ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    pContext->hGL = SDL_GL_CreateContext(pWindow);
    if (pContext->hGL == NULL) {
        SetAglError("SDL_GL_CreateContext");
        return 0;
    }
    pContext->pWindow = pWindow;
    s_pCurrent = pContext;
    return 1;
}

GLboolean aglDestroyContext(AGLContext hContext)
{
    Context *pContext = reinterpret_cast<Context *>(hContext);
    if (pContext == NULL)
        return 0;
    if (pContext->hGL != NULL) {
        if (SDL_GL_GetCurrentContext() == pContext->hGL)
            SDL_GL_MakeCurrent(pContext->pWindow, NULL);
        SDL_GL_DestroyContext(pContext->hGL);
    }
    if (s_pCurrent == pContext) {
        s_pCurrent = NULL;
        s_bViewportSet = false;
    }
    delete pContext;
    return 1;
}

GLboolean aglSetCurrentContext(AGLContext hContext)
{
    Context *pContext = reinterpret_cast<Context *>(hContext);
    if (pContext == NULL) {
        s_pCurrent = NULL;
        return SDL_GL_MakeCurrent(SDL_GL_GetCurrentWindow(), NULL) ? 1 : 0;
    }
    if (!AttachContext(pContext))
        return 0;
    if (!SDL_GL_MakeCurrent(pContext->pWindow, pContext->hGL)) {
        SetAglError("SDL_GL_MakeCurrent");
        return 0;
    }
    s_pCurrent = pContext;
    return 1;
}

GLboolean aglSetWindowRef(AGLContext hContext, WindowRef)
{
    return AttachContext(reinterpret_cast<Context *>(hContext));
}

// Fullscreen: the game's window over the whole (reported) screen, which Win32Posix shows as a
// desktop-fullscreen window, then the context on it.
GLboolean aglSetFullScreen(AGLContext hContext, GLsizei, GLsizei, GLsizei, GLint)
{
    SetWindowPos(GameWindow(), NULL, 0, 0, ScreenWidth(), ScreenHeight(), kSwpNoZOrder);
    return AttachContext(reinterpret_cast<Context *>(hContext));
}

void aglSwapBuffers(AGLContext hContext)
{
    Context *pContext = reinterpret_cast<Context *>(hContext);
    if (pContext == NULL || pContext->pWindow == NULL)
        return;
    SDL_GL_SwapWindow(pContext->pWindow);
    if (pContext == s_pCurrent)
        FollowWindow();
}

GLenum aglGetError(void)
{
    GLenum nError = s_nAglError;
    s_nAglError = 0;
    return nError;
}

GLubyte *aglErrorString(GLenum nError)
{
    if (nError == 0)
        return reinterpret_cast<GLubyte *>(const_cast<char *>("no error"));
    return reinterpret_cast<GLubyte *>(s_szAglError);
}

AGLRendererInfo aglQueryRendererInfoForCGDirectDisplayIDs(CGDirectDisplayID *, GLint)
{
    RendererInfo *pInfo = new (std::nothrow) RendererInfo();
    if (pInfo != NULL)
        pInfo->bAccelerated = true;
    return reinterpret_cast<AGLRendererInfo>(pInfo);
}

AGLRendererInfo aglNextRendererInfo(AGLRendererInfo)
{
    return 0;
}

GLboolean aglDescribeRenderer(AGLRendererInfo hInfo, GLint nProperty, GLint *pValue)
{
    const RendererInfo *pInfo = reinterpret_cast<const RendererInfo *>(hInfo);
    if (pInfo == NULL)
        return 0;
    switch (nProperty) {
    case AGL_RENDERER_ID: *pValue = kRendererId; return 1;
    case AGL_ACCELERATED: *pValue = pInfo->bAccelerated; return 1;
    default: *pValue = 0; return 1;
    }
}

void aglDestroyRendererInfo(AGLRendererInfo hInfo)
{
    delete reinterpret_cast<RendererInfo *>(hInfo);
}

// ---- CoreGraphics: every display is the reported screen

CGRect *CGDisplayBounds(CGRect *pResult, CGDirectDisplayID)
{
    *pResult = CGRect{{0.0f, 0.0f}, {(CGFloat)ScreenWidth(), (CGFloat)ScreenHeight()}};
    return pResult;
}

size_t CGDisplayPixelsWide(CGDirectDisplayID)
{
    return (size_t)ScreenWidth();
}

size_t CGDisplayPixelsHigh(CGDirectDisplayID)
{
    return (size_t)ScreenHeight();
}

CGDisplayModeRef CGDisplayCopyDisplayMode(CGDirectDisplayID)
{
    DEVMODEA dm;
    memset(&dm, 0, sizeof(dm));
    dm.dmSize = sizeof(dm);
    if (!EnumDisplaySettingsA(NULL, kEnumCurrentSettings, &dm))
        return 0;
    return reinterpret_cast<CGDisplayModeRef>(NewMode(dm));
}

size_t CGDisplayModeGetWidth(CGDisplayModeRef hMode)
{
    return Mode(hMode) ? Mode(hMode)->dmPelsWidth : 0;
}

size_t CGDisplayModeGetHeight(CGDisplayModeRef hMode)
{
    return Mode(hMode) ? Mode(hMode)->dmPelsHeight : 0;
}

void CGDisplayModeRelease(CGDisplayModeRef hMode)
{
    delete Mode(hMode);
}

// Recorded, not switched: Win32Posix reports the mode as the screen from then on.
CGError CGDisplaySetDisplayMode(CGDirectDisplayID, CGDisplayModeRef hMode, CFDictionaryRef)
{
    if (Mode(hMode) == NULL)
        return -1;
    DEVMODEA dm = *Mode(hMode);
    dm.dmSize = sizeof(dm);
    dm.dmFields = kDmBitsPerPel | kDmPelsWidth | kDmPelsHeight | kDmDisplayFrequency;
    LONG nResult = ChangeDisplaySettingsA(&dm, kCdsFullscreen);
    return nResult == 0 ? 0 : (CGError)nResult;
}

// Nothing to capture: the game owns only its window.
CGError CGCaptureAllDisplays(void)
{
    return 0;
}

CGError CGReleaseAllDisplays(void)
{
    return 0;
}

// A display's drawing context is the display number plus one. The renderer fills the captured
// display black with it; the window's own frames are cleared by GL.
CGContextRef CGDisplayGetDrawingContext(CGDirectDisplayID nDisplay)
{
    return reinterpret_cast<CGContextRef>(static_cast<uintptr_t>(nDisplay) + 1);
}

CGRect *CGContextConvertRectToUserSpace(CGRect *pResult, CGContextRef, CGRect rect)
{
    *pResult = rect;
    return pResult;
}

void CGContextSetFillColorWithColor(CGContextRef, CGColorRef)
{
}

void CGContextFillRect(CGContextRef, CGRect)
{
}

CGColorRef CGColorGetConstantColor(CFStringRef hName)
{
    return reinterpret_cast<CGColorRef>(const_cast<__CFString *>(hName));
}

// Events are not allocated: the one event the renderer makes reads the cursor.
CGEventRef CGEventCreate(CGEventSourceRef)
{
    return reinterpret_cast<CGEventRef>(1);
}

CGPoint CGEventGetLocation(CGEventRef)
{
    POINT pt = {0, 0};
    GetCursorPos(&pt);
    return CGPoint{(CGFloat)pt.x, (CGFloat)pt.y};
}

Boolean CFEqual(CFTypeRef hA, CFTypeRef hB)
{
    const DEVMODEA *pA = Mode(hA);
    const DEVMODEA *pB = Mode(hB);
    if (pA == NULL || pB == NULL)
        return pA == pB;
    return pA->dmPelsWidth == pB->dmPelsWidth && pA->dmPelsHeight == pB->dmPelsHeight &&
           pA->dmBitsPerPel == pB->dmBitsPerPel && pA->dmDisplayFrequency == pB->dmDisplayFrequency;
}

// The renderer releases display modes and events through CFRelease; events are not allocated.
void CFRelease(CFTypeRef hObject)
{
    if (reinterpret_cast<uintptr_t>(hObject) > 1)
        delete Mode(hObject);
}

// ---- Carbon: the window is the game's

OSStatus HIWindowCreate(WindowClass, int *, HIWindowDef *, HICoordinateSpace, HIRect *, WindowRef *pWindow)
{
    *pWindow = reinterpret_cast<WindowRef>(GameWindow());
    return 0;
}

EventTargetRef GetWindowEventTarget(WindowRef hWindow)
{
    return reinterpret_cast<EventTargetRef>(hWindow);
}

OSStatus InstallStandardEventHandler(EventTargetRef)
{
    return 0;
}

// The game's window procedure gets its input from Win32Posix: no Carbon handler is called.
OSStatus InstallEventHandler(EventTargetRef, EventHandlerUPP, ItemCount, EventTypeSpec *, void *, EventHandlerRef *)
{
    return 0;
}

UInt32 GetEventClass(EventRef)
{
    return 0;
}

UInt32 GetEventKind(EventRef)
{
    return 0;
}

void ShowWindow(WindowRef hWindow)
{
    ::ShowWindow(reinterpret_cast<HWND>(hWindow), kSwShow);
}

OSStatus ActivateWindow(WindowRef hWindow, Boolean bActivate)
{
    if (bActivate)
        SetActiveWindow(reinterpret_cast<HWND>(hWindow));
    return 0;
}

void BringToFront(WindowRef hWindow)
{
    SDL_Window *pWindow = Win32Posix_GetSdlWindow(reinterpret_cast<HWND>(hWindow));
    if (pWindow != NULL)
        SDL_RaiseWindow(pWindow);
}

// The game owns its window: disposing it is the game's business.
void DisposeWindow(WindowRef)
{
}

void SetPortWindowPort(WindowRef)
{
}

// QuickDraw Point {v, h} from the window's client area to the screen.
void LocalToGlobal(Point *pPoint)
{
    POINT pt = {pPoint->h, pPoint->v};
    ClientToScreen(GameWindow(), &pt);
    pPoint->h = (int16_t)pt.x;
    pPoint->v = (int16_t)pt.y;
}

// A desktop-fullscreen window hides the menu bar itself.
void HideMenuBar(void)
{
}

void ShowMenuBar(void)
{
}

// Discard the pending input messages of the mask's classes; never other messages.
void FlushEvents(EventMask wMask, EventMask)
{
    MSG msg;
    for (const EventClass &eventClass : s_aEventClasses) {
        if (eventClass.wMask & wMask) {
            while (PeekMessageA(&msg, NULL, eventClass.nFirst, eventClass.nLast, PM_REMOVE)) {
            }
        }
    }
}

Boolean EventAvail(EventMask wMask, EventRecord *)
{
    MSG msg;
    for (const EventClass &eventClass : s_aEventClasses) {
        if ((eventClass.wMask & wMask) && PeekMessageA(&msg, NULL, eventClass.nFirst, eventClass.nLast, PM_NOREMOVE))
            return 1;
    }
    return 0;
}

GDHandle GetMainDevice(void)
{
    return ScreenDevice();
}

GDHandle DMGetFirstScreenDevice(Boolean)
{
    return ScreenDevice();
}

GDHandle DMGetNextScreenDevice(GDHandle, Boolean)
{
    return NULL;
}

// ---- OpenGL: the Apple extensions and glActiveTexture through SDL_GL_GetProcAddress

typedef void (*TextureRangeFn)(GLenum eTarget, GLsizei nLength, const void *pPointer);
typedef void (*FinishObjectFn)(GLenum eObject, GLint nName);
typedef void (*ActiveTextureFn)(GLenum eTexture);

// GL_APPLE_texture_range: a client-storage hint; nothing when absent.
void glTextureRangeAPPLE(GLenum eTarget, GLsizei nLength, void *pPointer)
{
    static TextureRangeFn s_pfn;
    static bool s_bLooked;
    if (GlProc(&s_pfn, &s_bLooked, "glTextureRangeAPPLE", NULL) != NULL)
        s_pfn(eTarget, nLength, pPointer);
}

// GL_APPLE_fence: finish one object's commands; glFinish when absent.
void glFinishObjectAPPLE(GLenum eObject, GLint nName)
{
    static FinishObjectFn s_pfn;
    static bool s_bLooked;
    if (GlProc(&s_pfn, &s_bLooked, "glFinishObjectAPPLE", NULL) != NULL)
        s_pfn(eObject, nName);
    else
        glFinish();
}

// Core since GL 1.3, ARB_multitexture before; nothing when absent.
void glActiveTexture(GLenum eTexture)
{
    static ActiveTextureFn s_pfn;
    static bool s_bLooked;
    if (GlProc(&s_pfn, &s_bLooked, "glActiveTexture", "glActiveTextureARB") != NULL)
        s_pfn(eTexture);
}

// The Mac binary's glGetString is typed without the const.
GLubyte *glGetString(GLenum eName)
{
    return const_cast<GLubyte *>(::glGetString(eName));
}

void glViewport(GLint nX, GLint nY, GLsizei nWidth, GLsizei nHeight)
{
    s_viewport = Viewport{nX, nY, nWidth, nHeight};
    s_bViewportSet = true;
    ApplyViewport();
}

// ---- the Mac libc and C++ runtime

void pthread_yield_np(void)
{
    SDL_Delay(0);
}

void memset_pattern16(void *pDest, void *pPattern, size_t nSize)
{
    uint8_t *pOut = static_cast<uint8_t *>(pDest);
    const uint8_t *pIn = static_cast<const uint8_t *>(pPattern);
    for (size_t i = 0; i < nSize; i++)
        pOut[i] = pIn[i & 15];
}

void __bzero(void *pDest, size_t nSize)
{
    memset(pDest, 0, nSize);
}

// The renderer's one guarded static is initialised on the game thread.
int __cxa_guard_acquire(__guard *pGuard)
{
    return *reinterpret_cast<uint8_t *>(pGuard) == 0;
}

void __cxa_guard_release(__guard *pGuard)
{
    *reinterpret_cast<uint8_t *>(pGuard) = 1;
}

// A pure virtual slot of an abstract CD2Textures: called only through a broken object.
void __cxa_pure_virtual(void)
{
    Fog::Src::ErrorManager::ERROR_UnrecoverableInternalError_Halt(const_cast<char *>(""), __builtin_return_address(0), 0);
    ExitProcess((UINT)-1);
}

void __stack_chk_fail(void)
{
    Fog::Src::ErrorManager::ERROR_UnrecoverableInternalError_Halt(const_cast<char *>(""), __builtin_return_address(0), 0);
    ExitProcess((UINT)-1);
    // Win32Posix's ExitProcess is not declared noreturn; it ends the process.
    __builtin_unreachable();
}

// ---- Mac Storm and MacSpecific helpers

// fp_fatal("Unrecoverable internal error %08x line %d", address, line) is the Mac's halt.
void fp_fatal(char *szFormat, ...)
{
    va_list args;
    va_start(args, szFormat);
    void *pAddress = va_arg(args, void *);
    int nLine = va_arg(args, int);
    va_end(args);
    Fog::Src::ErrorManager::ERROR_UnrecoverableInternalError_Halt(const_cast<char *>(""), pAddress, nLine);
}

void MAC_WarpMouseCursorPosition(uint32_t ptPosition)
{
    // QuickDraw Point {v, h} packed into one register: v in the low half
    SetCursorPos((int16_t)(ptPosition >> 16), (int16_t)ptPosition);
}

void HANDLE_GetFileRefNum(void *hFile, int16_t *pRefNum)
{
    s_hBinkFile = hFile;
    *pRefNum = 0;
}

// The smacker is opened through Storm; there is no file spec to make.
int16_t FilepathToFSSpec(void *, const void *, int)
{
    return 0;
}

void *BinkOpenRefNum(void *, uint32_t dwFlags)
{
    return ::BinkOpen(s_hBinkFile, dwFlags);
}

// The Mac cursor procedures: the layer shows the system cursor.
void MACCURSOR_InitCursor(void)
{
}

void MACCURSOR_ShowDisplayCursor(void)
{
}

void MACCURSOR_HideDisplayCursor(void)
{
}

void MACCURSOR_SetCursorProcs(void)
{
}

void MACCURSOR_RestoreCursor(void)
{
}

// Mac 00036ffe picks the closest hardware mode at least nWidth x nHeight. Win32Posix accepts
// every mode, since the client is scaled into the window, so the closest is the request itself.
CGDisplayModeRef MACDISPLAY_FindBestDisplayMode(CGDirectDisplayID, uint32_t nWidth, uint32_t nHeight, uint32_t nBits,
                                                uint32_t nRefreshHz, uint8_t, uint8_t)
{
    if (nWidth == 0 || nHeight == 0)
        return 0;
    DEVMODEA dm;
    memset(&dm, 0, sizeof(dm));
    dm.dmSize = sizeof(dm);
    if (!EnumDisplaySettingsA(NULL, kEnumCurrentSettings, &dm))
        return 0;
    dm.dmFields = kDmBitsPerPel | kDmPelsWidth | kDmPelsHeight | kDmDisplayFrequency;
    dm.dmPelsWidth = nWidth;
    dm.dmPelsHeight = nHeight;
    if (nBits != 0)
        dm.dmBitsPerPel = nBits;
    if (nRefreshHz != 0)
        dm.dmDisplayFrequency = nRefreshHz;
    return reinterpret_cast<CGDisplayModeRef>(NewMode(dm));
}

// SDL3 has no display gamma ramp: the display keeps its own.
void MACDISPLAY_SetGammaRamp(CGDirectDisplayID, int, int)
{
}

void MACDISPLAY_RestoreGammaRamp(CGDirectDisplayID)
{
}

// Mac 0019f18a: at least ten 10 ms sleeps and at least 250 ms.
void MAC_SettleWait(void)
{
    DWORD dwStart = GetTickCount();
    for (unsigned i = 0;; i++) {
        if (i > 9 && GetTickCount() - dwStart >= 250)
            return;
        Sleep(10);
    }
}

// Mac 002ffe21: the Bink surface type for the display depth (8 -> 0, 16 -> 9, 32 -> 3). The
// layer's screen is 32 bits.
int MAC_GetBinkSurfaceType(CGDirectDisplayID)
{
    return 3;
}

// The Carbon window handler's zoom event: the layer sends the game no such event.
void MAC_OnWindowZoom(void)
{
}

BOOL SPRITECACHE_EnsureSpriteLoaded(::D2TileLibraryEntryStrc *pTile)
{
    return D2CMP::SRC::SpriteCache::SPRITECACHE_AllocEntry(pTile, 1, 0);
}

int CloseHandle(void *hObject)
{
    return ::CloseHandle(hObject);
}

intptr_t SendMessageA(void *hWnd, uint32_t nMsg, uintptr_t wParam, intptr_t lParam)
{
    return ::SendMessageA(static_cast<HWND>(hWnd), nMsg, wParam, lParam);
}

int SetRect(void *pRect, int nLeft, int nTop, int nRight, int nBottom)
{
    ::SetRect(static_cast<RECT *>(pRect), nLeft, nTop, nRight, nBottom);
    return 1;
}

}  // namespace D2OpenGL

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
