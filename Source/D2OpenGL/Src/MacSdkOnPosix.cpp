// Restoration code, not in any binary: the Mac system calls of the D2OpenGL renderer, on the
// Win32Posix layer and SDL3 (see MacSdkOnWin32.h). The modern Mac build only; Windows has
// MacSdkOnWin32.cpp, the 1.14d Mac the real frameworks.
//
// The display mode, the window, the context and the swap are not here: in this build the
// renderer's OpenGLMac* functions are oglSdl.cpp's, an SDL GL context on the game window (the
// SDL window Win32Posix made for it with SDL_WINDOW_OPENGL). What stays is what the rest of
// the renderer calls: the displays, which are Win32Posix's one reported screen, the cursor,
// the input queue, the Bink file, and the GL entry points beyond the system library's export
// list, from SDL_GL_GetProcAddress.

#if !defined(_WIN32) && D2_BACKEND_POSIX

#include "Source/D2Headers/d2_platform.h"
#include "Source/D2OpenGL/Src/MacSdkOnWin32.h"

#include <SDL3/SDL.h>
#include <cstdarg>
#include <cstring>

#include "Source/D2gfx/Src/D2GFX.h"
#include "Source/Fog/Src/ErrorManager.h"
#include "Source/D2CMP/Src/SpriteCache.h"

namespace D2OpenGL {

namespace {

// The Win32 numbers this file passes to Win32Posix (winuser.h)
enum : UINT {
    kSmCxScreen = 0,
    kSmCyScreen = 1,
};

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

// The renderer releases its cursor event through CFRelease; events are not allocated.
void CFRelease(CFTypeRef)
{
}

// ---- Carbon: the window is the game's

// QuickDraw Point {v, h} from the window's client area to the screen.
void LocalToGlobal(Point *pPoint)
{
    POINT pt = {pPoint->h, pPoint->v};
    ClientToScreen(GameWindow(), &pt);
    pPoint->h = (int16_t)pt.x;
    pPoint->v = (int16_t)pt.y;
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

// SDL3 has no display gamma ramp: the display keeps its own.
void MACDISPLAY_SetGammaRamp(CGDirectDisplayID, int, int)
{
}

void MACDISPLAY_RestoreGammaRamp(CGDirectDisplayID)
{
}

// Mac 002ffe21: the Bink surface type for the display depth (8 -> 0, 16 -> 9, 32 -> 3). The
// layer's screen is 32 bits.
int MAC_GetBinkSurfaceType(CGDirectDisplayID)
{
    return 3;
}

BOOL SPRITECACHE_EnsureSpriteLoaded(::D2TileLibraryEntryStrc *pTile)
{
    return D2CMP::SRC::SpriteCache::SPRITECACHE_AllocEntry(pTile, 1, 0);
}

int CloseHandle(void *hObject)
{
    return ::CloseHandle(hObject);
}

int SetRect(void *pRect, int nLeft, int nTop, int nRight, int nBottom)
{
    ::SetRect(static_cast<RECT *>(pRect), nLeft, nTop, nRight, nBottom);
    return 1;
}

}  // namespace D2OpenGL

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
