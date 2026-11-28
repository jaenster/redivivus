// Restoration code, not in any binary: the Mac system calls of the D2OpenGL renderer, on Win32 and WGL.
//
// The renderer is the Mac 1.14d OpenGL code. This header is its Mac SDK: the AGL,
// CoreGraphics, CoreFoundation, Carbon and QuickDraw types it spells, at global
// scope as windows.h declares the Win32 ones, shaped as the Mac binary's Ghidra
// types are; and its calls into those frameworks, the Mac libc and the Mac-only
// Storm helpers, in namespace D2OpenGL, where an unqualified call from the module
// finds them before any Win32 function of the same name (ShowWindow, SetRect).
// On Windows the window is the game's (D2GFX GetHwnd), never one of our own. The 1.14d Mac
// build takes the first branch below: the 10.7 SDK itself. The modern Mac build
// (D2_BACKEND_POSIX) takes the second, as Windows does, with these calls on its own layer:
// MacSdkOnWin32.cpp on Windows, MacSdkOnPosix.cpp (Win32Posix and SDL3) on POSIX.
//
// A structure the Mac returns in memory keeps the i386 Darwin convention the
// decompile shows: the result pointer is the first argument, and is returned.

#pragma once

#include "Source/D2Headers/D2Arch.h"

#if !defined(_WIN32) && D2_MAC_CARBON

// On the Mac the renderer calls the real frameworks. What stays here: the calls whose
// shape in the decompile differs from the 10.7 SDK's (a structure returned through a
// leading result pointer, a const-less result), as inline forwarders in namespace
// D2OpenGL that hide the SDK's name; and the Mac Storm, MacSpecific and Bink entry
// points the renderer calls, which no SDK header declares.

#include <cstdint>
#include <cstddef>
#include <pthread.h>
#include <string.h>
#include <Carbon/Carbon.h>
#include <AGL/agl.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>
#include "Source/MacSpecific/Source/MacShell/QuickDrawMac.h"

typedef int64_t __guard;

struct D2TileLibraryEntryStrc;

// The Bink the Mac links statically; the module keeps a movie as a BINK pointer.
struct BINK { uint32_t Width, Height, Frames, FrameNum; };
typedef BINK *HBINK;
typedef void *BINKSNDSYSOPEN(uint32_t dwParam);

extern "C" {
extern uintptr_t __stack_chk_guard;
void __stack_chk_fail(void) __attribute__((noreturn));
void __bzero(void *pDest, size_t nSize);
int __cxa_guard_acquire(__guard *pGuard);
void __cxa_guard_release(__guard *pGuard);
void __cxa_pure_virtual(void);
}

// Mac Storm's own entry points are global names in the Mac binary (the module imports them
// by those names); the module sees them through the using-declarations in D2OpenGL.
struct FSSpec;
void fp_fatal(const char *szFormat, ...);
void MAC_WarpMouseCursorPosition(Point ptPosition);
int16_t FilepathToFSSpec(FSSpec *pSpec, const char *szPath, int nFlags);
void MACCURSOR_InitCursor(void);
void MACCURSOR_ShowDisplayCursor(void);
void MACCURSOR_HideDisplayCursor(void);
void MACCURSOR_SetCursorProcs(void);
void MACCURSOR_RestoreCursor(void);
CGDisplayModeRef MACDISPLAY_FindBestDisplayMode(CGDirectDisplayID nDisplay, uint32_t nWidth, uint32_t nHeight, uint32_t nBits,
                                                uint32_t nRefreshHz, uint8_t bStretched, uint8_t bAcceptClosest);
void MACDISPLAY_SetGammaRamp(CGDirectDisplayID nDisplay, int nGamma, int nContrast);
void MACDISPLAY_RestoreGammaRamp(CGDirectDisplayID nDisplay);

namespace D2OpenGL {

inline CGRect *CGDisplayBounds(CGRect *pResult, CGDirectDisplayID nDisplay)
{
    *pResult = ::CGDisplayBounds(nDisplay);
    return pResult;
}
inline CGRect *CGContextConvertRectToUserSpace(CGRect *pResult, CGContextRef hContext, CGRect rect)
{
    *pResult = ::CGContextConvertRectToUserSpace(hContext, rect);
    return pResult;
}
inline GLubyte *glGetString(GLenum eName)
{
    return (GLubyte *)::glGetString(eName);
}
inline GLubyte *aglErrorString(GLenum nError)
{
    return (GLubyte *)::aglErrorString(nError);
}

// ---- Mac Storm and MacSpecific helpers the renderer calls

// Storm fp_fatal: every renderer halt is fp_fatal("... %08x line %d", address, line)
// BinkOpen on the file HANDLE_GetFileRefNum last saw; the Mac passes the HFS refnum
// Bink's BinkOpen (a third-party symbol): the renderer hands it the HFS refnum.
void *BinkOpenRefNum(void *pRefNum, uint32_t dwFlags) __asm__("_BinkOpen");
void MAC_SettleWait(void) __asm__("__Z14MAC_SettleWaitv");  // MacShell/unknown_util.cpp, 0019f18a
int MAC_GetBinkSurfaceType(CGDirectDisplayID nDisplay) __asm__("__Z22MAC_GetBinkSurfaceTypej");  // oglMac.cpp, 002ffe21
void MAC_OnWindowZoom(void) __asm__("__Z12FUN_0019c953v");  // MacShell/unknown_display.cpp, 0019c953: the zoom event
// Storm's Win32 layer on the Mac: the Win32 calls, with its own handle types (CloseHandle
// and HANDLE_GetFileRefNum are Win32Mac.h's). Bound to the layer's SendMessage and SetRect
// (Win32_Messages.cpp, Win32_Rect.cpp), whose window and rectangle types this header lacks.
intptr_t SendMessageA(void *hWnd, uint32_t nMsg, uintptr_t wParam, intptr_t lParam)
    __asm__("__Z11SendMessageP14Win32MacWindowjji");
int SetRect(void *pRect, int nLeft, int nTop, int nRight, int nBottom) __asm__("__Z7SetRectP4RECTiiii");
// Mac 00134d0f: loads a tile's sprite into the cache. It is the shared SPRITECACHE_AllocEntry,
// whose two further arguments the body never reads and the Mac callers do not pass.
int SPRITECACHE_EnsureSpriteLoaded(::D2TileLibraryEntryStrc *pTile)
    __asm__("__ZN5D2CMP3SRC11SpriteCache22SPRITECACHE_AllocEntryEP22D2TileLibraryEntryStrcjj");
// The shared renderer code passes untyped pointers and a packed point where the layer
// takes records.
inline void MAC_WarpMouseCursorPosition(uint32_t ptPosition)
{
    Point pt;
    memcpy(&pt, &ptPosition, sizeof(pt));
    ::MAC_WarpMouseCursorPosition(pt);
}
inline int16_t FilepathToFSSpec(void *pSpec, const void *szPath, int nFlags)
{
    return ::FilepathToFSSpec(static_cast<FSSpec *>(pSpec), static_cast<const char *>(szPath), nFlags);
}
using ::fp_fatal;
using ::MACCURSOR_InitCursor;
using ::MACCURSOR_ShowDisplayCursor;
using ::MACCURSOR_HideDisplayCursor;
using ::MACCURSOR_SetCursorProcs;
using ::MACCURSOR_RestoreCursor;
using ::MACDISPLAY_FindBestDisplayMode;
using ::MACDISPLAY_SetGammaRamp;
using ::MACDISPLAY_RestoreGammaRamp;
using ::BinkOpenDirectSound;
using ::__stack_chk_guard;
using ::__stack_chk_fail;
using ::__bzero;
using ::__cxa_guard_acquire;
using ::__cxa_guard_release;
using ::__cxa_pure_virtual;
using ::kCGColorBlack;
using ::CGPointZero;
// The Mac application's procedure table
extern void *PTR_DAT_00397abc __asm__("_g_pDisplayDriver");  // StormMac/MacDisplayDriver.cpp, 00397abc

}  // namespace D2OpenGL

#else  // the renderer on Win32 and WGL


#include <cstdint>
#include <cstddef>
#if defined(_WIN32)
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
// The POSIX build: the Win32 types are Win32Posix's (d2_platform.h), the GL headers the
// system's, and these Mac SDK calls its own layer's. Only the 1.14d build has the Mac SDK
// itself: the modern one has no AGL (gone from the SDK), Carbon windows or QuickDraw.
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION 1
#endif
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif

// ---- Mac SDK types (MacTypes, CoreFoundation, CoreGraphics, AGL, HIToolbox, QuickDraw, cxxabi)

typedef uint8_t Boolean;
typedef int32_t OSStatus;
typedef uint32_t OSType;
typedef uint32_t UInt32;
typedef uint32_t ItemCount;
struct Point { int16_t v, h; };
#pragma pack(push, 2)
struct Rect { int16_t top, left, bottom, right; };
#pragma pack(pop)

typedef const void *CFTypeRef;
typedef struct __CFString *CFStringRef;
typedef struct __CFDictionary *CFDictionaryRef;

typedef float CGFloat;
struct CGPoint { CGFloat x, y; };
struct CGSize { CGFloat width, height; };
struct CGRect { CGPoint origin; CGSize size; };
typedef int32_t CGError;
typedef uint32_t CGDirectDisplayID;
typedef struct CGDisplayMode *CGDisplayModeRef;
typedef struct CGContext *CGContextRef;
typedef struct CGColor *CGColorRef;
typedef struct __CGEvent *CGEventRef;
typedef struct __CGEventSource *CGEventSourceRef;

// QuickDraw graphics device record (68k alignment): gdRect at 0x22
#pragma pack(push, 2)
struct GDevice {
    int16_t gdRefNum, gdID, gdType;
    void *gdITable;
    int16_t gdResPref;
    void *gdSearchProc, *gdCompProc;
    int16_t gdFlags;
    void *gdPMap;
    int32_t gdRefCon;
    void *gdNextGD;
    Rect gdRect;
    int32_t gdMode;
    int16_t gdCCBytes, gdCCDepth;
    void *gdCCXData, *gdCCXMask, *gdExt;
};
#pragma pack(pop)
static_assert(sizeof(void*) != 4 || (offsetof(GDevice, gdRect) == 0x22 && sizeof(GDevice) == 0x3e), "QuickDraw GDevice");
typedef GDevice *GDPtr;
typedef GDPtr *GDHandle;

typedef struct __AGLPixelFormatRec *AGLPixelFormat;
typedef struct __AGLContextRec *AGLContext;
typedef struct __AGLRendererInfoRec *AGLRendererInfo;
typedef GDPtr *AGLDevice;

typedef CGRect HIRect;
typedef uint32_t WindowClass;
typedef uint32_t HICoordinateSpace;
typedef uint16_t EventKind;
typedef uint16_t EventMask;
typedef uint16_t EventModifiers;
typedef struct OpaqueWindowPtr *WindowRef;
typedef struct OpaqueEventRef *EventRef;
typedef struct OpaqueEventTargetRef *EventTargetRef;
typedef struct OpaqueEventHandlerRef *EventHandlerRef;
typedef struct OpaqueEventHandlerCallRef *EventHandlerCallRef;
typedef OSStatus (*EventHandlerUPP)(EventHandlerCallRef, EventRef, void *);
struct HIWindowDef;
struct EventTypeSpec { OSType eventClass; UInt32 eventKind; };
#pragma pack(push, 2)
struct EventRecord { EventKind what; UInt32 message; UInt32 when; Point where; EventModifiers modifiers; };
#pragma pack(pop)

typedef int64_t __guard;

struct D2TileLibraryEntryStrc;

// The Bink the Mac links statically; the module keeps a movie as a BINK pointer.
struct BINK { uint32_t Width, Height, Frames, FrameNum; };
typedef BINK *HBINK;
// binkw32's sound-system opener is __stdcall; the Mac's static Bink took a cdecl one
typedef void *__stdcall BINKSNDSYSOPEN(uint32_t dwParam);

namespace D2OpenGL {

// ---- AGL (pixel format, context, renderer info) on WGL

AGLPixelFormat aglChoosePixelFormat(AGLDevice *pDevices, GLint nDevices, GLint *pAttribs);
void aglDestroyPixelFormat(AGLPixelFormat hFormat);
GLboolean aglDescribePixelFormat(AGLPixelFormat hFormat, GLint nAttrib, GLint *pValue);
AGLContext aglCreateContext(AGLPixelFormat hFormat, AGLContext hShare);
GLboolean aglDestroyContext(AGLContext hContext);
GLboolean aglSetCurrentContext(AGLContext hContext);
GLboolean aglSetWindowRef(AGLContext hContext, WindowRef hWindow);
GLboolean aglSetFullScreen(AGLContext hContext, GLsizei nWidth, GLsizei nHeight, GLsizei nFrequency, GLint nDevice);
void aglSwapBuffers(AGLContext hContext);
GLenum aglGetError(void);
GLubyte *aglErrorString(GLenum nError);
AGLRendererInfo aglQueryRendererInfoForCGDirectDisplayIDs(CGDirectDisplayID *pDisplays, GLint nDisplays);
AGLRendererInfo aglNextRendererInfo(AGLRendererInfo hInfo);
GLboolean aglDescribeRenderer(AGLRendererInfo hInfo, GLint nProperty, GLint *pValue);
void aglDestroyRendererInfo(AGLRendererInfo hInfo);

// ---- CoreGraphics displays (CGDirectDisplayID is a monitor index, 0 the primary)

CGRect *CGDisplayBounds(CGRect *pResult, CGDirectDisplayID nDisplay);
size_t CGDisplayPixelsWide(CGDirectDisplayID nDisplay);
size_t CGDisplayPixelsHigh(CGDirectDisplayID nDisplay);
CGDisplayModeRef CGDisplayCopyDisplayMode(CGDirectDisplayID nDisplay);
size_t CGDisplayModeGetWidth(CGDisplayModeRef hMode);
size_t CGDisplayModeGetHeight(CGDisplayModeRef hMode);
void CGDisplayModeRelease(CGDisplayModeRef hMode);
CGError CGDisplaySetDisplayMode(CGDirectDisplayID nDisplay, CGDisplayModeRef hMode, CFDictionaryRef hOptions);
CGError CGCaptureAllDisplays(void);
CGError CGReleaseAllDisplays(void);
CGContextRef CGDisplayGetDrawingContext(CGDirectDisplayID nDisplay);
CGRect *CGContextConvertRectToUserSpace(CGRect *pResult, CGContextRef hContext, CGRect rect);
void CGContextSetFillColorWithColor(CGContextRef hContext, CGColorRef hColor);
void CGContextFillRect(CGContextRef hContext, CGRect rect);
CGColorRef CGColorGetConstantColor(CFStringRef hName);
CGEventRef CGEventCreate(CGEventSourceRef hSource);
CGPoint CGEventGetLocation(CGEventRef hEvent);
Boolean CFEqual(CFTypeRef hA, CFTypeRef hB);
void CFRelease(CFTypeRef hObject);
extern const CFStringRef kCGColorBlack;
extern const CGPoint CGPointZero;

// ---- Carbon window, event and QuickDraw calls

OSStatus HIWindowCreate(WindowClass nClass, int *pAttributes, HIWindowDef *pDef, HICoordinateSpace nSpace, HIRect *pBounds,
                        WindowRef *pWindow);
EventTargetRef GetWindowEventTarget(WindowRef hWindow);
OSStatus InstallStandardEventHandler(EventTargetRef hTarget);
OSStatus InstallEventHandler(EventTargetRef hTarget, EventHandlerUPP pfnHandler, ItemCount nTypes, EventTypeSpec *pTypes,
                             void *pUserData, EventHandlerRef *pOutRef);
UInt32 GetEventClass(EventRef hEvent);
UInt32 GetEventKind(EventRef hEvent);
void ShowWindow(WindowRef hWindow);
OSStatus ActivateWindow(WindowRef hWindow, Boolean bActivate);
void BringToFront(WindowRef hWindow);
void DisposeWindow(WindowRef hWindow);
void SetPortWindowPort(WindowRef hWindow);
void LocalToGlobal(Point *pPoint);
void HideMenuBar(void);
void ShowMenuBar(void);
void FlushEvents(EventMask wMask, EventMask wStopMask);
Boolean EventAvail(EventMask wMask, EventRecord *pEvent);
GDHandle GetMainDevice(void);
GDHandle DMGetFirstScreenDevice(Boolean bActiveOnly);
GDHandle DMGetNextScreenDevice(GDHandle hDevice, Boolean bActiveOnly);

// ---- OpenGL: Apple extensions, and glGetString as the Mac binary types it

void glTextureRangeAPPLE(GLenum eTarget, GLsizei nLength, void *pPointer);
void glFinishObjectAPPLE(GLenum eObject, GLint nName);
void glActiveTexture(GLenum eTexture);
GLubyte *glGetString(GLenum eName);
#if !defined(_WIN32) && D2_BACKEND_POSIX
// The renderer's viewport is in the game window's client pixels; the POSIX layer scales the
// client into the window's drawable (Win32Posix_GetViewport), so the viewport follows it.
void glViewport(GLint nX, GLint nY, GLsizei nWidth, GLsizei nHeight);
// Before D2gfx makes the game window: the window is made for an OpenGL context.
void POSIX_PrepareGameWindowForGL(void);
#endif

// ---- the Mac libc and C++ runtime

void pthread_yield_np(void);
void memset_pattern16(void *pDest, void *pPattern, size_t nSize);
void __bzero(void *pDest, size_t nSize);
int __cxa_guard_acquire(__guard *pGuard);
void __cxa_guard_release(__guard *pGuard);
void __cxa_pure_virtual(void);
extern uintptr_t __stack_chk_guard;
[[noreturn]] void __stack_chk_fail(void);

// ---- Mac Storm and MacSpecific helpers the renderer calls

// Storm fp_fatal: every renderer halt is fp_fatal("... %08x line %d", address, line)
void fp_fatal(char *szFormat, ...);
void MAC_WarpMouseCursorPosition(uint32_t ptPosition);
void HANDLE_GetFileRefNum(void *hFile, int16_t *pRefNum);
int16_t FilepathToFSSpec(void *pSpec, const void *szPath, int nFlags);
// BinkOpen on the file HANDLE_GetFileRefNum last saw; the Mac passes the HFS refnum
void *BinkOpenRefNum(void *pRefNum, uint32_t dwFlags);
void MACCURSOR_InitCursor(void);
void MACCURSOR_ShowDisplayCursor(void);
void MACCURSOR_HideDisplayCursor(void);
void MACCURSOR_SetCursorProcs(void);
void MACCURSOR_RestoreCursor(void);
CGDisplayModeRef MACDISPLAY_FindBestDisplayMode(CGDirectDisplayID nDisplay, uint32_t nWidth, uint32_t nHeight, uint32_t nBits,
                                                uint32_t nRefreshHz, uint8_t bStretched, uint8_t bAcceptClosest);
void MACDISPLAY_SetGammaRamp(CGDirectDisplayID nDisplay, int nGamma, int nContrast);
void MACDISPLAY_RestoreGammaRamp(CGDirectDisplayID nDisplay);
void MAC_SettleWait(void);
int MAC_GetBinkSurfaceType(CGDirectDisplayID nDisplay);
void MAC_OnWindowZoom(void);
// Storm's Win32 layer on the Mac: the Win32 calls, with its own handle types
int CloseHandle(void *hObject);
intptr_t SendMessageA(void *hWnd, uint32_t nMsg, uintptr_t wParam, intptr_t lParam);
int SetRect(void *pRect, int nLeft, int nTop, int nRight, int nBottom);
// Mac 00134d0f: loads a tile's sprite into the cache. Windows' SPRITECACHE_AllocEntry
// (005fdea0) takes two more arguments its body never reads.
BOOL SPRITECACHE_EnsureSpriteLoaded(::D2TileLibraryEntryStrc *pTile);
// Bink's DirectSound opener, which the renderer takes the address of
using ::BinkOpenDirectSound;
// The Mac application's procedure table; its cursor procedures do nothing on Windows
extern void *PTR_DAT_00397abc;

}  // namespace D2OpenGL

#endif  // !defined(_WIN32) && D2_MAC_CARBON
