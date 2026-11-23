// The Mac system calls of the D2OpenGL renderer, on Win32 and WGL (see platform.h).
//
// The pixel format, context and fullscreen handling follow the 1999 D2OpenGL.dll:
// PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER RGBA at the requested
// colour depth, SetLastError(0) before ChoosePixelFormat, a SetPixelFormat failure
// with GetLastError() == 0 tolerated, wglCreateContext / wglMakeCurrent, and the
// dispose order wglMakeCurrent(0), wglDeleteContext, ReleaseDC.

#include "Source/D2Headers/d2_platform.h"
#include "D2OpenGL/platform.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <new>

#include "Source/D2gfx/D2GFX.h"
#include "Source/Fog/Src/DataArrays.h"
#include "Source/Fog/Src/ErrorManager.h"
#include "Source/D2CMP/SRC/SpriteCache.h"

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

// The one renderer a Windows ICD is: the id any requested id matches.
const int kRendererId = 0x00022000;

struct PixelFormat {
    PIXELFORMATDESCRIPTOR pfd;
    bool bAccelerated;
};

struct Context {
    const PixelFormat *pFormat;
    HDC hDC;
    HGLRC hGLRC;
};

struct RendererInfo {
    bool bAccelerated;
};

uint32_t s_nAglError;

HWND GameWindow()
{
    return D2gfx::D2GFX::GetHwnd();
}

bool IsAccelerated(const PIXELFORMATDESCRIPTOR &pfd)
{
    return !(pfd.dwFlags & PFD_GENERIC_FORMAT) || (pfd.dwFlags & PFD_GENERIC_ACCELERATED);
}

// ---- displays: CGDirectDisplayID n is the n-th monitor EnumDisplayMonitors reports

struct MonitorQuery {
    uintptr_t nWanted;
    uintptr_t nIndex;
    HMONITOR hFound;
};

BOOL CALLBACK FindMonitor(HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam)
{
    MonitorQuery *pQuery = reinterpret_cast<MonitorQuery *>(lParam);
    if (pQuery->nIndex++ == pQuery->nWanted) {
        pQuery->hFound = hMonitor;
        return FALSE;
    }
    return TRUE;
}

HMONITOR DisplayMonitor(CGDirectDisplayID nDisplay)
{
    MonitorQuery query = {nDisplay, 0, NULL};
    EnumDisplayMonitors(NULL, NULL, FindMonitor, reinterpret_cast<LPARAM>(&query));
    if (query.hFound != NULL)
        return query.hFound;
    POINT ptOrigin = {0, 0};
    return MonitorFromPoint(ptOrigin, MONITOR_DEFAULTTOPRIMARY);
}

bool DisplayInfo(CGDirectDisplayID nDisplay, MONITORINFOEXA *pInfo)
{
    memset(pInfo, 0, sizeof(*pInfo));
    pInfo->cbSize = sizeof(*pInfo);
    return GetMonitorInfoA(DisplayMonitor(nDisplay), reinterpret_cast<MONITORINFO *>(pInfo)) != 0;
}

RECT DisplayRect(CGDirectDisplayID nDisplay)
{
    MONITORINFOEXA info;
    if (DisplayInfo(nDisplay, &info))
        return info.rcMonitor;
    RECT rc = {0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
    return rc;
}

DEVMODEA *NewMode(const DEVMODEA &dm)
{
    DEVMODEA *pMode = new (std::nothrow) DEVMODEA(dm);
    return pMode;
}

template <typename T> DEVMODEA *Mode(T hMode)
{
    return reinterpret_cast<DEVMODEA *>(const_cast<void *>(static_cast<const void *>(hMode)));
}

// ---- QuickDraw GDevice records, one per monitor: the renderer matches gdRect to a display

const int kMaxDevices = 8;
GDevice s_aDevices[kMaxDevices];
GDevice *s_apDevices[kMaxDevices];
int s_nDevices;

BOOL CALLBACK AddDevice(HMONITOR hMonitor, HDC, LPRECT, LPARAM)
{
    if (s_nDevices >= kMaxDevices)
        return FALSE;
    MONITORINFO info;
    memset(&info, 0, sizeof(info));
    info.cbSize = sizeof(info);
    GetMonitorInfoA(hMonitor, &info);
    GDevice &device = s_aDevices[s_nDevices];
    device.gdRect.top = (int16_t)info.rcMonitor.top;
    device.gdRect.left = (int16_t)info.rcMonitor.left;
    device.gdRect.bottom = (int16_t)info.rcMonitor.bottom;
    device.gdRect.right = (int16_t)info.rcMonitor.right;
    s_apDevices[s_nDevices] = &device;
    s_nDevices++;
    return TRUE;
}

void ScanDevices()
{
    s_nDevices = 0;
    EnumDisplayMonitors(NULL, NULL, AddDevice, 0);
}

// ---- the Carbon event masks as input messages (mDown 0x2, mUp 0x4, keyDown 0x8, keyUp 0x10, autoKey 0x20)

struct EventClass {
    uint16_t wMask;
    UINT nFirst, nLast;
};

const EventClass s_aEventClasses[] = {
    {0x02, WM_LBUTTONDOWN, WM_LBUTTONDOWN}, {0x02, WM_RBUTTONDOWN, WM_RBUTTONDOWN},
    {0x02, WM_MBUTTONDOWN, WM_MBUTTONDOWN}, {0x04, WM_LBUTTONUP, WM_LBUTTONUP},
    {0x04, WM_RBUTTONUP, WM_RBUTTONUP},     {0x04, WM_MBUTTONUP, WM_MBUTTONUP},
    {0x28, WM_KEYDOWN, WM_KEYDOWN},         {0x28, WM_SYSKEYDOWN, WM_SYSKEYDOWN},
    {0x10, WM_KEYUP, WM_KEYUP},             {0x10, WM_SYSKEYUP, WM_SYSKEYUP},
};

// ---- the open file a BinkOpen follows, and the saved gamma ramp

void *s_hBinkFile;
bool s_bGammaSaved;
WORD s_awSavedGammaRamp[3][256];

void NoOperation(void)
{
}

// The Mac application's procedure table (0xc8 bytes and more); every slot does nothing.
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

}  // namespace

void *PTR_DAT_00397abc = s_apfnAppProcs;
uintptr_t __stack_chk_guard = 0xBB40E64E;
const CFStringRef kCGColorBlack = reinterpret_cast<CFStringRef>(1);
const CGPoint CGPointZero = {0.0f, 0.0f};

// ---- AGL

AGLPixelFormat aglChoosePixelFormat(AGLDevice *, GLint, GLint *pAttribs)
{
    PixelFormat format;
    memset(&format, 0, sizeof(format));
    format.pfd.nSize = sizeof(format.pfd);
    format.pfd.nVersion = 1;
    format.pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    format.pfd.iPixelType = PFD_TYPE_RGBA;
    format.pfd.cColorBits = 32;
    format.pfd.iLayerType = PFD_MAIN_PLANE;
    bool bWantAccelerated = false;
    for (const int *p = pAttribs; p != NULL && *p != AGL_NONE; p++) {
        switch (*p) {
        case AGL_DOUBLEBUFFER: format.pfd.dwFlags |= PFD_DOUBLEBUFFER; break;
        case AGL_ACCELERATED: bWantAccelerated = true; break;
        case AGL_PIXEL_SIZE: format.pfd.cColorBits = (BYTE)*++p; break;
        case AGL_RENDERER_ID: ++p; break;
        default: break;
        }
    }

    HDC hDC = GetDC(GameWindow());
    if (hDC == NULL) {
        s_nAglError = GetLastError();
        return 0;
    }
    SetLastError(0);
    int nFormat = ChoosePixelFormat(hDC, &format.pfd);
    if (nFormat != 0)
        DescribePixelFormat(hDC, nFormat, sizeof(format.pfd), &format.pfd);
    ReleaseDC(GameWindow(), hDC);
    if (nFormat == 0) {
        s_nAglError = GetLastError();
        return 0;
    }
    format.bAccelerated = IsAccelerated(format.pfd);
    if (bWantAccelerated && !format.bAccelerated) {
        s_nAglError = ERROR_NOT_SUPPORTED;
        return 0;
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
    case AGL_PIXEL_SIZE: *pValue = pFormat->pfd.cColorBits; return 1;
    default: *pValue = 0; return 1;
    }
}

AGLContext aglCreateContext(AGLPixelFormat hFormat, AGLContext)
{
    Context *pContext = new (std::nothrow) Context();
    if (pContext == NULL)
        return 0;
    pContext->pFormat = reinterpret_cast<const PixelFormat *>(hFormat);
    return reinterpret_cast<AGLContext>(pContext);
}

// Attach the context to the game's window: its DC, the pixel format (a window takes one for its
// lifetime, so an existing one is kept) and the GL context.
static uint8_t AttachContext(Context *pContext)
{
    if (pContext == NULL || pContext->pFormat == NULL)
        return 0;
    if (pContext->hGLRC != NULL)
        return 1;
    HWND hWnd = GameWindow();
    pContext->hDC = GetDC(hWnd);
    if (pContext->hDC == NULL) {
        s_nAglError = GetLastError();
        return 0;
    }
    if (GetPixelFormat(pContext->hDC) == 0) {
        SetLastError(0);
        int nFormat = ChoosePixelFormat(pContext->hDC, &pContext->pFormat->pfd);
        if (!SetPixelFormat(pContext->hDC, nFormat, &pContext->pFormat->pfd) && GetLastError() != 0) {
            s_nAglError = GetLastError();
            return 0;
        }
    }
    pContext->hGLRC = wglCreateContext(pContext->hDC);
    if (pContext->hGLRC == NULL) {
        s_nAglError = GetLastError();
        return 0;
    }
    return 1;
}

GLboolean aglDestroyContext(AGLContext hContext)
{
    Context *pContext = reinterpret_cast<Context *>(hContext);
    if (pContext == NULL)
        return 0;
    if (pContext->hGLRC != NULL) {
        if (wglGetCurrentContext() == pContext->hGLRC)
            wglMakeCurrent(NULL, NULL);
        wglDeleteContext(pContext->hGLRC);
    }
    if (pContext->hDC != NULL)
        ReleaseDC(GameWindow(), pContext->hDC);
    delete pContext;
    return 1;
}

GLboolean aglSetCurrentContext(AGLContext hContext)
{
    Context *pContext = reinterpret_cast<Context *>(hContext);
    if (pContext == NULL)
        return wglMakeCurrent(NULL, NULL) ? 1 : 0;
    if (!AttachContext(pContext))
        return 0;
    if (!wglMakeCurrent(pContext->hDC, pContext->hGLRC)) {
        s_nAglError = GetLastError();
        return 0;
    }
    return 1;
}

GLboolean aglSetWindowRef(AGLContext hContext, WindowRef)
{
    return AttachContext(reinterpret_cast<Context *>(hContext));
}

// Fullscreen: the game's window over the whole display, then the context on it.
GLboolean aglSetFullScreen(AGLContext hContext, GLsizei, GLsizei, GLsizei, GLint)
{
    RECT rc = DisplayRect(0);
    MoveWindow(GameWindow(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
    return AttachContext(reinterpret_cast<Context *>(hContext));
}

void aglSwapBuffers(AGLContext hContext)
{
    Context *pContext = reinterpret_cast<Context *>(hContext);
    if (pContext != NULL && pContext->hDC != NULL)
        SwapBuffers(pContext->hDC);
}

GLenum aglGetError(void)
{
    GLenum nError = s_nAglError;
    s_nAglError = 0;
    return nError;
}

GLubyte *aglErrorString(GLenum nError)
{
    static char szMessage[64];
    snprintf(szMessage, sizeof(szMessage), "Win32 error %lu", (unsigned long)nError);
    return reinterpret_cast<GLubyte *>(szMessage);
}

AGLRendererInfo aglQueryRendererInfoForCGDirectDisplayIDs(CGDirectDisplayID *, GLint)
{
    PixelFormat *pFormat = reinterpret_cast<PixelFormat *>(aglChoosePixelFormat(NULL, 0, NULL));
    RendererInfo *pInfo = new (std::nothrow) RendererInfo();
    if (pInfo != NULL)
        pInfo->bAccelerated = pFormat != NULL && pFormat->bAccelerated;
    delete pFormat;
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

// ---- CoreGraphics

CGRect *CGDisplayBounds(CGRect *pResult, CGDirectDisplayID nDisplay)
{
    RECT rc = DisplayRect(nDisplay);
    *pResult = CGRect{{(CGFloat)rc.left, (CGFloat)rc.top}, {(CGFloat)(rc.right - rc.left), (CGFloat)(rc.bottom - rc.top)}};
    return pResult;
}

size_t CGDisplayPixelsWide(CGDirectDisplayID nDisplay)
{
    RECT rc = DisplayRect(nDisplay);
    return (uint32_t)(rc.right - rc.left);
}

size_t CGDisplayPixelsHigh(CGDirectDisplayID nDisplay)
{
    RECT rc = DisplayRect(nDisplay);
    return (uint32_t)(rc.bottom - rc.top);
}

CGDisplayModeRef CGDisplayCopyDisplayMode(CGDirectDisplayID nDisplay)
{
    MONITORINFOEXA info;
    DEVMODEA dm;
    memset(&dm, 0, sizeof(dm));
    dm.dmSize = sizeof(dm);
    const char *szDevice = DisplayInfo(nDisplay, &info) ? info.szDevice : NULL;
    if (!EnumDisplaySettingsA(szDevice, ENUM_CURRENT_SETTINGS, &dm))
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

CGError CGDisplaySetDisplayMode(CGDirectDisplayID nDisplay, CGDisplayModeRef hMode, CFDictionaryRef)
{
    if (Mode(hMode) == NULL)
        return -1;
    MONITORINFOEXA info;
    const char *szDevice = DisplayInfo(nDisplay, &info) ? info.szDevice : NULL;
    DEVMODEA dm = *Mode(hMode);
    dm.dmSize = sizeof(dm);
    dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
    LONG nResult = ChangeDisplaySettingsExA(szDevice, &dm, NULL, CDS_FULLSCREEN, NULL);
    return nResult == DISP_CHANGE_SUCCESSFUL ? 0 : (CGError)nResult;
}

// Capturing the displays has no Windows counterpart.
CGError CGCaptureAllDisplays(void)
{
    return 0;
}

CGError CGReleaseAllDisplays(void)
{
    return 0;
}

// A display's drawing context is its DC, filled through GDI.
// A display's drawing context is the display number plus one; filled through GDI.
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

void CGContextFillRect(CGContextRef hContext, CGRect rect)
{
    CGDirectDisplayID nDisplay = (CGDirectDisplayID)(reinterpret_cast<uintptr_t>(hContext) - 1);
    RECT rcDisplay = DisplayRect(nDisplay);
    RECT rc = {rcDisplay.left + (LONG)rect.origin.x, rcDisplay.top + (LONG)rect.origin.y,
               rcDisplay.left + (LONG)(rect.origin.x + rect.size.width), rcDisplay.top + (LONG)(rect.origin.y + rect.size.height)};
    HDC hDC = GetDC(NULL);
    if (hDC == NULL)
        return;
    FillRect(hDC, &rc, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
    ReleaseDC(NULL, hDC);
}

CGColorRef CGColorGetConstantColor(CFStringRef hName)
{
    return reinterpret_cast<CGColorRef>(hName);
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
    ::ShowWindow(reinterpret_cast<HWND>(hWindow), SW_SHOW);
}

OSStatus ActivateWindow(WindowRef hWindow, Boolean bActivate)
{
    if (bActivate)
        SetActiveWindow(reinterpret_cast<HWND>(hWindow));
    return 0;
}

void BringToFront(WindowRef hWindow)
{
    BringWindowToTop(reinterpret_cast<HWND>(hWindow));
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
    ScanDevices();
    return s_nDevices > 0 ? &s_apDevices[0] : NULL;
}

GDHandle DMGetFirstScreenDevice(Boolean)
{
    ScanDevices();
    return s_nDevices > 0 ? &s_apDevices[0] : NULL;
}

GDHandle DMGetNextScreenDevice(GDHandle hDevice, Boolean)
{
    GDevice **ppDevice = hDevice;
    if (ppDevice < s_apDevices || ppDevice + 1 >= s_apDevices + s_nDevices)
        return NULL;
    return ppDevice + 1;
}

// ---- Apple GL extensions and the Mac libc

// Client storage hints have no Windows counterpart.
void glTextureRangeAPPLE(GLenum, GLsizei, void *)
{
}

// No per-object finish in GL 1.1.
void glFinishObjectAPPLE(GLenum, GLint)
{
    glFinish();
}

typedef void(APIENTRY *ActiveTextureFn)(GLenum eTexture);

// glActiveTexture through wglGetProcAddress (needs a current context); nothing when absent.
void glActiveTexture(GLenum eTexture)
{
    static ActiveTextureFn s_pfnActiveTexture;
    if (s_pfnActiveTexture == NULL) {
        PROC pfn = wglGetProcAddress("glActiveTexture");
        if (pfn == NULL)
            pfn = wglGetProcAddress("glActiveTextureARB");
        uintptr_t nAddress = reinterpret_cast<uintptr_t>(pfn);
        if (nAddress <= 3 || nAddress == (uintptr_t)-1)
            return;
        s_pfnActiveTexture = reinterpret_cast<ActiveTextureFn>(reinterpret_cast<void (*)(void)>(pfn));
    }
    s_pfnActiveTexture(eTexture);
}

// The Mac binary's glGetString is typed without the const.
GLubyte *glGetString(GLenum eName)
{
    return const_cast<GLubyte *>(::glGetString(eName));
}

void pthread_yield_np(void)
{
    SwitchToThread();
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

// Windows opens a smacker through Storm; there is no file spec to make.
int16_t FilepathToFSSpec(void *, const void *, int)
{
    return 0;
}

void *BinkOpenRefNum(void *, uint32_t dwFlags)
{
    return ::BinkOpen(s_hBinkFile, dwFlags);
}

// The Mac cursor procedures: Windows draws its own cursor.
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

// Mac 00036ffe: the mode with the smallest (area over the request + 100 for a stretch mismatch +
// refresh difference) among the modes at least nWidth x nHeight with nBits, when exact or accepted.
// Windows modes carry no stretched flag.
CGDisplayModeRef MACDISPLAY_FindBestDisplayMode(CGDirectDisplayID nDisplay, uint32_t nWidth, uint32_t nHeight, uint32_t nBits,
                                                uint32_t nRefreshHz, uint8_t bStretched, uint8_t bAcceptClosest)
{
    MONITORINFOEXA info;
    const char *szDevice = DisplayInfo(nDisplay, &info) ? info.szDevice : NULL;
    int nBestScore = 0x7fffffff;
    bool bFound = false;
    DEVMODEA best;
    DEVMODEA dm;
    memset(&best, 0, sizeof(best));
    memset(&dm, 0, sizeof(dm));
    dm.dmSize = sizeof(dm);
    for (DWORD i = 0; EnumDisplaySettingsA(szDevice, i, &dm); i++) {
        if (dm.dmBitsPerPel == nBits && dm.dmPelsWidth >= nWidth && dm.dmPelsHeight >= nHeight) {
            int nRefreshDiff = (int)nRefreshHz - (int)dm.dmDisplayFrequency;
            if (nRefreshDiff < 0)
                nRefreshDiff = -nRefreshDiff;
            int nScore = (int)(dm.dmPelsWidth * dm.dmPelsHeight - nWidth * nHeight) + (bStretched ? 100 : 0) + nRefreshDiff;
            if (nScore < nBestScore) {
                nBestScore = nScore;
                best = dm;
                bFound = true;
            }
        }
        memset(&dm, 0, sizeof(dm));
        dm.dmSize = sizeof(dm);
    }
    if (!bFound || !(bAcceptClosest || nBestScore == 0))
        return 0;
    return reinterpret_cast<CGDisplayModeRef>(NewMode(best));
}

// Mac 002f7324: the Fog ramp (as the Windows D3D SetGamma 006b3570 builds it: 2-byte entries,
// 256 of them, up to 0xffff) into the display; the display's own ramp is saved first.
void MACDISPLAY_SetGammaRamp(CGDirectDisplayID nDisplay, int nGamma, int nContrast)
{
    MONITORINFOEXA info;
    HDC hDC = DisplayInfo(nDisplay, &info) ? CreateDCA(info.szDevice, NULL, NULL, NULL) : NULL;
    if (hDC == NULL)
        return;
    if (!s_bGammaSaved && GetDeviceGammaRamp(hDC, s_awSavedGammaRamp))
        s_bGammaSaved = true;
    WORD awRamp[3][256];
    Fog::Src::DataArrays::FOG_FillArrayWithRandomInts(nGamma, reinterpret_cast<uint32_t *>(awRamp[0]), (double)nContrast,
                                                      2, 0x100, 0xffff);
    memcpy(awRamp[1], awRamp[0], sizeof(awRamp[0]));
    memcpy(awRamp[2], awRamp[0], sizeof(awRamp[0]));
    SetDeviceGammaRamp(hDC, awRamp);
    DeleteDC(hDC);
}

// Mac 002f73f8
void MACDISPLAY_RestoreGammaRamp(CGDirectDisplayID nDisplay)
{
    if (!s_bGammaSaved)
        return;
    MONITORINFOEXA info;
    HDC hDC = DisplayInfo(nDisplay, &info) ? CreateDCA(info.szDevice, NULL, NULL, NULL) : NULL;
    if (hDC == NULL)
        return;
    SetDeviceGammaRamp(hDC, s_awSavedGammaRamp);
    DeleteDC(hDC);
    s_bGammaSaved = false;
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

// Mac 002ffe21: the Bink surface type for the display depth (8 -> 0, 16 -> 9, 32 -> 3).
int MAC_GetBinkSurfaceType(CGDirectDisplayID nDisplay)
{
    MONITORINFOEXA info;
    DEVMODEA dm;
    memset(&dm, 0, sizeof(dm));
    dm.dmSize = sizeof(dm);
    if (!EnumDisplaySettingsA(DisplayInfo(nDisplay, &info) ? info.szDevice : NULL, ENUM_CURRENT_SETTINGS, &dm))
        return -1;
    switch (dm.dmBitsPerPel) {
    case 8: return 0;
    case 16: return 9;
    case 32: return 3;
    default: return -1;
    }
}

// The Carbon window handler's zoom event: Windows sends the game no such event.
void MAC_OnWindowZoom(void)
{
}

BOOL SPRITECACHE_EnsureSpriteLoaded(::D2TileLibraryEntryStrc *pTile)
{
    return D2CMP::SRC::SpriteCache::SPRITECACHE_AllocEntry(pTile, 1, 0);
}

int CloseHandle(uintptr_t hObject)
{
    return ::CloseHandle(reinterpret_cast<HANDLE>(hObject));
}

intptr_t SendMessageA(void *hWnd, uint32_t nMsg, uintptr_t wParam, intptr_t lParam)
{
    return ::SendMessageA(static_cast<HWND>(hWnd), nMsg, wParam, lParam);
}

int SetRect(void *pRect, int nLeft, int nTop, int nRight, int nBottom)
{
    return ::SetRect(static_cast<RECT *>(pRect), nLeft, nTop, nRight, nBottom);
}

}  // namespace D2OpenGL
