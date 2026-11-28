// uniInternal.h - what the D2Uni objects share: the back-end selector, the 8-bit back buffer, the
// palette, the DrawSprocket and QuickDraw back-end entry points, and the calls D2Uni makes into
// the rest of the game fragment and the Mac shared libraries.
//
// Not an original header name (no PPC string names one); see Source/D2Uni/README.md. Globals are
// listed with their LoD 1.10b classic address. Names are inferred unless a comment says otherwise.
//
// Classic Mac PowerPC, and the Win32Posix build: the guard is
// `#if !defined(_WIN32) && ((D2_MAC_CARBON && D2_ARCH_PPC) || D2_BACKEND_POSIX)`, from the compiler
// flags of the target (never from D2Arch.h). On D2_BACKEND_POSIX the DrawSprocket and QuickDraw
// back ends and the Mac platform calls are left out, and the SDL3 back end (sdlCore.cpp) stands
// in for them; uniPosix.h supplies the Toolbox and RAD records the shared slots read.
#pragma once

#if !defined(_WIN32) && ((D2_MAC_CARBON && D2_ARCH_PPC) || D2_BACKEND_POSIX)

#if !D2_BACKEND_POSIX
// The Mac C library owns FILE; d2_platform.h's MSVC _iobuf stand-in is for the Windows CRT layout.
#ifndef _FILE_DEFINED
#define _FILE_DEFINED
#endif

#include "Redivivus/Platform/MacClassic/MacClassicUniToolbox.h"
#include "Redivivus/Platform/MacClassic/MacClassicUniDrawSprocket.h"
#include "Redivivus/Platform/MacClassic/MacClassicUniRad.h"
#else
#include "Source/D2Uni/Src/uniPosix.h"
// The 1.14d record, whose cel UniGetCel reads (uniSprite.cpp).
#include "Source/D2Client/D2GfxDataStrc.h"
#endif
#include "Source/D2gfx/D2gfx.public.h"
#include "Source/D2Client/D2GfxSettingsStrc.h"
#include "Source/D2Client/Engine/Gfx.h"
#include "Source/D2Common/D2TileLibraryEntryStrc.h"
#include "Source/D2CMP/D2CMP.h"
#include "Source/Storm/Storm.h"
#include "Source/D2Uni/D2Uni.h"

// ---------------------------------------------------------------------------------------------
// The back end in use (gnUniMode). SetOption(8) picks it: full screen DrawSprocket, windowed
// QuickDraw. The Carbon builds add CoreGraphics (3) for Mac OS X.
enum eD2UniMode {
    UNIMODE_NONE = 0,
    UNIMODE_DRAWSPROCKET = 1,
    UNIMODE_QUICKDRAW = 2,
    UNIMODE_COREGRAPHICS = 3,
#if D2_BACKEND_POSIX
    UNIMODE_SDL = 4, // the Win32Posix build's only back end, windowed or full screen
#endif
};

// A display mode record (SetOption 2 and 3 store one each, 8 bytes; index 0 is 640x480, 1 is
// 800x600). The fields are what DSpFindContextEx (1.10b 100a27a0) takes: its log prints
// "%dx%dx%d" and "%.3fHz", and "user provided"/"guessed" - read here as the flag in bit 15 of
// the width. Field names inferred.
struct D2MacDisplayModeStrc {
    /* 0x00 */ uint16_t wWidth;      // bit 15: flag, masked off before use
    /* 0x02 */ int16_t nHeight;
    /* 0x04 */ int32_t fRefreshRate; // Fixed 16.16
};

#if !D2_BACKEND_POSIX
// The Storm Mac "vid" table (a CFM data import): the renderer registers its suspend/resume and
// cursor callbacks here so Storm's Mac event loop can switch it out. Only the slots D2Uni calls
// are named; the offsets are the ones the PEF code loads.
typedef BOOL (*VIDSUSPENDPROC)();
typedef void (*VIDCURSORPROC)();
struct D2MacVidTableStrc {
    /* 0x00 */ void * pUnknown00[0x6C / 4];
    /* 0x6C */ void (*pfnInitCursor)();
    /* 0x70 */ void * pUnknown70[(0xC0 - 0x70) / 4];
    /* 0xC0 */ void (*pfnSetSuspendResumeProcs)(VIDSUSPENDPROC pfnSuspend, VIDSUSPENDPROC pfnResume);
    /* 0xC4 */ void (*pfnSetCursorProcs)(VIDCURSORPROC pfnInit, VIDCURSORPROC pfnShow, VIDCURSORPROC pfnHide);
};
#endif

// ---------------------------------------------------------------------------------------------
// Shared state (uniCore.cpp)
extern int gnUniMode;                            // 1055e738
extern D2GfxSettingsStrc * gpUniSettings;        // 1055e600, the InitPerspective settings
extern D2GfxHelperStrc * gpUniHelpers;           // 1055e608, D2gfx's software primitives
extern BYTE * gpUniBackBuffer;                   // 1055e604, the 8-bit back buffer
extern int gnUniBackBufferWidth;                 // 1055e734, pixels per row (the pitch)
extern int gnUniBackBufferHeight;                // 1055e730
extern int gnUniBackBufferDepth;                 // 1055e72c, bits per pixel of the back buffer port
extern int gnUniScreenWidth;                     // 1055e5c8
extern int gnUniScreenHeight;                    // 1055e5c4
extern BOOL gbUniCutsceneQuit;                   // 1055e5cc, ends the Bink loop
extern BOOL gbUniSurfaceReleased;                // 1055e5d0, set while EndCutScene has the surface down
extern D2MacDisplayModeStrc gaUniDisplayModes[2]; // 1055e5d4
extern CTabHandle ghUniColorTable;               // 1055e5f4, the 256-entry palette, built by UniInit
extern GDHandle ghUniDevice;                     // 1055e5f8, SetOption 0
extern BOOL gbUniInScene;                        // 1055e5fc, SetOption 10
extern BYTE * gapUniPaletteTables[72];           // 1055e60c, copied in by SetPaletteTable
extern BOOL gbUniAppActive;                      // 104422a8, the resume flag of the last osEvt
extern int gnUniGamma;                           // 104422ac
extern int gnUniContrast;                        // 104422b0, SetOption 11
extern int gnUniCutsceneDepth;                   // 104422a4, depth QD re-creates the surface at for a cutscene

// uniSmack.cpp
extern BOOL gbUniSmackPaletteReady;              // 1055dc20
extern BYTE gabUniSmackRemap[256];               // 1055dc24, Smacker palette index -> game palette index

// ---------------------------------------------------------------------------------------------
// uniEngine.cpp
void UniPlayBinkFile(char * szFile, int nResolution, BYTE * pDest, int nDestPitch, uint32_t dwSurfaceType,
                     fnBinkFrameCallback pfnFrame);
void UniBinkRenderFrame(BYTE * pDest, int nDestPitch, uint32_t dwSurfaceType);
void UniEngineReset();
void UniEngineSetResolution();
BOOL __stdcall UniAreBlendedShadowsEnabled();

// The table slots defined in uniEngine.cpp, uniSmack.cpp and uniSprite.cpp (uniCore.cpp fills
// the table with them).
void __fastcall UniDrawGroundTile(D2TileLibraryEntryStrc * pTile, D2GfxLightExStrc * pLight, int nPosX, int nPosY,
                                  int nWorldX, int nWorldY, BYTE nAlpha, int nScreenPanels, void * pTileData);
void __fastcall UniDrawSolidRectEx(int nXStart, int nYStart, int nXEnd, int nYEnd, BYTE nColor, DRAWMODE eDrawMode);
void __fastcall UniDrawSolidRectAlpha(int nXStart, int nYStart, int nXEnd, int nYEnd, DWORD dwColor, BYTE nAlpha);
void __fastcall UniDrawLine(int nXStart, int nYStart, int nXEnd, int nYEnd, BYTE dwColor, BYTE nAlpha);
void __fastcall UniDrawRect(RECT * pRect, BYTE nColor);
void __fastcall UniDrawRectEx(RECT * pRect, BYTE nColor);
void __fastcall UniDrawSolidRect(RECT * pRect, BYTE nColor);
void __fastcall UniDrawSolidSquare(POINT * pCenter, BYTE nSize, BYTE nColor);
void __fastcall UniClearScreen(BOOL bPartial);
void __fastcall UniDrawString(int nPosX, int nPosY, char * szFormat, va_list va);
void __fastcall UniDrawLight(DWORD * pLight, DWORD * pPlayerLight, int nPosX, int nPosY);
void __fastcall UniDecodeSmacker(char * szFile, D2SmackerContextStrc * pContext, int nMode);
void __fastcall UniPlayerSmacker(D2SmackerContextStrc * pContext);
void __fastcall UniCloseSmacker(D2SmackerContextStrc * pContext);
BOOL __fastcall UniDrawShadowTile(D2TileLibraryEntryStrc * pTile, int nPosX, int nPosY, int nUnused, int nScreenPanels);
BOOL __fastcall UniDrawTransWallTile(D2TileLibraryEntryStrc * pTile, int nPosX, int nPosY, D2GfxLightStrc * pLight,
                                     int nScreenPanels, BYTE nAlpha);
BOOL __fastcall UniDrawWallTile(D2TileLibraryEntryStrc * pTile, int nPosX, int nPosY, D2GfxLightStrc * pLight,
                                int nScreenPanels);
void __fastcall UniDrawPerspectiveImage(D2GfxDataStrc * pData, int nPosX, int nPosY, DWORD dwGamma, int nDrawMode,
                                        int nScreenMode, BYTE * pPalette);
BOOL __fastcall UniDrawImage(D2GfxDataStrc * pData, int nPosX, int nPosY, DWORD dwGamma, int nDrawMode, BYTE * pPalette);
void __fastcall UniDrawShiftedImage(D2GfxDataStrc * pData, int nPosX, int nPosY, DWORD dwGamma, DRAWMODE eDrawMode,
                                    GLOBALPALLETE nGlobalPaletteShift);
void __fastcall UniDrawVerticalCropImage(D2GfxDataStrc * pData, int nPosX, int nPosY, int nSkipLines, int nDrawLines,
                                         int nDrawMode);
void __fastcall UniDrawClippedImage(D2GfxDataStrc * pData, int nPosX, int nPosY, void * pCropRect, int nDrawMode);
void __fastcall UniDrawShadow(D2GfxDataStrc * pData, int nPosX, int nPosY);
void __fastcall UniDrawImageFast(D2GfxDataStrc * pData, int nPosX, int nPosY, BYTE nPaletteIndex);
void __fastcall UniDebugFillBackBuffer(int nPosX, int nPosY);

// uniSprite.cpp
void UniSpriteSetResolution(BOOL bLarge);
BYTE * UniGetPaletteTable(int nDrawMode, int * pnTableKind);

// uniCore.cpp
BOOL UniBinkBlit(Rect * pRect);

#if !D2_BACKEND_POSIX
// ---------------------------------------------------------------------------------------------
// dspCore.cpp - the DrawSprocket back end (full screen; the Windows twin is D2DDraw ddCore.cpp)
int * DSpGetRenderStatistics();
BOOL DSpChangeRes(HWND hWnd, int nResolution);
void DSpSetDefaultPerspectiveFactor();
void DSpPerspectiveScalePosition();
void DSpAdjustPerspectivePosition();
void DSpSetPerspectiveScale();
void DSpSetGlobalLight();
int DSpCheckGamma();
void DSpSetGamma(int nGamma);
void DSpUpdateScaleFactor();
void DSpSetPaletteTable(BYTE ** ppTables);
void DSpSetPalette(PALETTEENTRY * pPalette);
void DSpGetScreenSize(int * pWidth, int * pHeight);
BOOL DSpBlit();
BOOL DSpBinkBlit(Rect * pRect);
BOOL DSpEndDraw();
BOOL DSpStartDraw(BOOL bClear);
BOOL DSpCheckCutscene();
void DSpPlayCutsceneGuarded(char * szFile, int nResolution, fnBinkFrameCallback pfnFrame);
BOOL DSpCutsceneCreateWindow();
void DSpPlayCutscene(char * szFile, int nResolution, fnBinkFrameCallback pfnFrame);
void DSpEndCutScene(HWND hWnd, int nResolution, BOOL bRelease);
BOOL DSpCloseSurface();
BOOL DSpCreateSurface(HWND hWnd, int nResolution);
BOOL DSpRelease();
BOOL DSpInit(D2GfxSettingsStrc * pSettings, D2GfxHelperStrc * pHelpers);
BOOL DSpDetect();
void DSpSetGDevice();
BOOL DSpLocalToGlobal(Point * pPoint);
BOOL DSpGlobalToLocal(Point * pPoint);
Boolean DSpProcessEvent(EventRecord * pEvent);

// qdCore.cpp - the QuickDraw back end (windowed; the Windows twin is D2Gdi gdiCore.cpp)
GDHandle QDGetGDevice();
int * QDGetRenderStatistics();
BOOL QDChangeRes(HWND hWnd, int nResolution);
void QDSetDefaultPerspectiveFactor();
void QDPerspectiveScalePosition(int nX, int nY, int nAngle, int * pX, int * pY, BOOL bOrder);
void QDAdjustPerspectivePosition(int nX, int nY, int nBias, int * pX, int * pY);
void QDSetPerspectiveScale(int nScaleX, int nScaleY);
void QDSetGlobalLight(int nRed, int nGreen, int nBlue);
int QDCheckGamma();
void QDSetGamma(int nGamma);
void QDUpdateScaleFactor(int nScale);
void QDSetPaletteTable(BYTE ** ppTables);
void QDSetPalette(PALETTEENTRY * pPalette);
void QDGetScreenSize(int * pWidth, int * pHeight);
BOOL QDBlit();
BOOL QDBinkBlit(Rect * pRect);
BOOL QDEndDraw();
BOOL QDStartDraw(BOOL bClear);
BOOL QDCloseSurface();
BOOL QDCheckCutscene();
void QDPlayCutscene(char * szFile, int nResolution, fnBinkFrameCallback pfnFrame);
BOOL QDCutsceneCreateWindow();
BOOL QDLocalToGlobal(Point * pPoint);
BOOL QDGlobalToLocal(Point * pPoint);
Boolean QDProcessEvent(EventRecord * pEvent);
void QDEndCutScene(HWND hWnd, int nResolution, BOOL bRelease);
BOOL QDCreateSurfaceEntry(HWND hWnd, int nResolution);
BOOL QDCreateSurface(int nResolution, int nDepth);
BOOL QDClose();
BOOL QDInit(D2GfxSettingsStrc * pSettings, D2GfxHelperStrc * pHelpers);
BOOL QDDetect();
void QDSetGDevice();
#else
// ---------------------------------------------------------------------------------------------
// sdlCore.cpp - the SDL3 back end of the Win32Posix build (windowed and full screen): the 8-bit
// back buffer converted through the palette into an SDL texture, scaled into the window.
BOOL SdlDetect();
BOOL SdlInit(D2GfxSettingsStrc * pSettings, D2GfxHelperStrc * pHelpers);
BOOL SdlRelease();
BOOL SdlCreateSurface(HWND hWnd, int nResolution);
BOOL SdlCloseSurface();
BOOL SdlChangeRes(HWND hWnd, int nResolution);
void SdlEndCutScene(HWND hWnd, int nResolution, BOOL bRelease);
BOOL SdlCutsceneCreateWindow();
void SdlPlayCutscene(char * szFile, int nResolution, fnBinkFrameCallback pfnFrame);
BOOL SdlCheckCutscene();
BOOL SdlStartDraw(BOOL bClear);
BOOL SdlEndDraw();
BOOL SdlBlit();
BOOL SdlBinkBlit(Rect * pRect);
void SdlGetScreenSize(int * pWidth, int * pHeight);
void SdlSetPalette(PALETTEENTRY * pPalette);
void SdlSetPaletteTable(BYTE ** ppTables);
void SdlSetGamma(int nGamma);
int SdlCheckGamma();
int * SdlGetRenderStatistics();
// A key or mouse button press is waiting (it is taken off the queue): the Bink loop's quit test.
BOOL SdlInputPending();
#endif

// ---------------------------------------------------------------------------------------------
// The rest of the game fragment. Named by their Windows twins or their own log strings; the
// address is the LoD 1.10b classic one.

// The Mac platform log (1.10b 1003c160). sdlCore.cpp defines it on D2_BACKEND_POSIX.
void LogPrintf(const char * szFormat, ...);
// Halts with the message (1.10b 10006820: FPvfatal + ExitToShell). sdlCore.cpp defines it on
// D2_BACKEND_POSIX.
void MacFatal(const char * szMessage);
#if !D2_BACKEND_POSIX
// Mac platform gamma helpers. MacSetGammaCurve (1.10b 10012bc0) builds a ramp with
// MacBuildGammaTable and loads it into the device's gamma table (PBStatus/Control on the driver);
// MacRestoreGamma (1.10b 10012a30) puts the saved table back (Control).
void MacSetGammaCurve(GDHandle hDevice, int nGamma, int nContrast);
void MacRestoreGamma(GDHandle hDevice);
// 1.10b 100396c0: a gamma ramp of nCount entries scaled to nMax, curve exponent from nGamma and
// fContrast. The PowerPC argument registers fix the order: int, double (r4/r5 shadow), pointer.
void MacBuildGammaTable(int nGamma, double fContrast, uint16_t * pTable, int nKind, int nCount, int nMax);
// 1.10b 100030a0: suspends (TRUE) or resumes (FALSE) InputSprocket and flushes the cursor device.
void MacSetInputSuspended(BOOL bSuspend);
// The shell's DrawSprocket context search (1.10b 100a27a0, "DSpFindContextEx:" log prefix).
OSStatus DSpFindContextEx(GDHandle hDevice, int nWidth, int nHeight, int32_t fRefreshRate, int nDepth,
                          DSpContextReference * pContext, DSpContextAttributes * pAttributes);
// The Mac platform layer's MemCGrafPort.cpp (1.10b 10016270 / 100161a0, "MemCGrafPort.cpp"
// __FILE__): an offscreen colour port and its own GDevice over a buffer it allocates, with the
// given colour table. Parameter names inferred from the Toolbox calls inside (OpenCPort,
// NewHandle, MakeITable, SetDeviceAttribute; CloseCPort, DisposeGDevice).
OSErr NewMemCGrafPort(const Rect * pBounds, int16_t nDepth, CTabHandle hColors, BYTE ** ppBits, int * pnRowBytes,
                      CGrafPtr * ppPort, GDHandle * phPortDevice);
void DisposeMemCGrafPort(BYTE * pBits, CGrafPtr pPort, GDHandle hPortDevice);

namespace Fog {
BOOL __stdcall GetUseDirectCommand();                // 1.10b 10033370 (1.14d Fog 00406d20)
}
#else
// The 1.14d home of the Fog call the Bink player makes.
namespace Fog::Source::SFile {
BOOL __stdcall GetUseDirectCommand();
}
namespace Fog {
inline BOOL GetUseDirectCommand() {
    return Fog::Source::SFile::GetUseDirectCommand();
}
}
#endif
namespace Fog::Memory {
void * __fastcall AllocClientMemory(size_t nAllocSize, char * szFile, int32_t nLine, int32_t nNull); // 1.10b 10039940
BOOL __fastcall FreeClientMemory(void * pMemoryToFree, char * szFile, int32_t nLine, int32_t nNull);  // 1.10b 10039970
}

// D2Lang and D2Win, for UniDrawString. The D2Lang Unicode character is one 16-bit unit with a
// constructor (UniDrawString builds its buffer with __construct_array).
struct Unicode {
    uint16_t wChar;
    Unicode() : wChar(0) {}
    static Unicode * toUnicode(Unicode * pDest, const char * szSrc, int nMax); // 1.10b 1009b940
};
#if !D2_BACKEND_POSIX
int D2Win_SetTextFont(int nFont);                                                  // 1.10b 1004b630
void D2Win_DrawText(const Unicode * wszText, int nX, int nY, int nColor, BOOL bCentered); // 1.10b 1004a400
#else
// The same three calls in the 1.14d tree: D2Lang's char-to-wide copy, D2Win's font switch and
// its text draw (DRAW_text takes the same colour and centring arguments).
namespace D2Lang::Unicode::UNISYS {
uint16_t * __fastcall STRING_ConvertCharToWchar(uint16_t * pwszBuffer, char * pszBuffer, int32_t nSize);
}
namespace D2Win::Src::D2WinFont {
void __fastcall DRAW_text(uint16_t * wMessage, int32_t nX, int32_t nY, int32_t dwColor, BOOL bMultiLineCenter);
int32_t __fastcall SetAndReturnLastFont(int32_t nFontId);
}
static_assert(sizeof(Unicode) == sizeof(uint16_t), "a D2Lang character is one 16-bit unit");
inline Unicode * Unicode::toUnicode(Unicode * pDest, const char * szSrc, int nMax) {
    D2Lang::Unicode::UNISYS::STRING_ConvertCharToWchar((uint16_t *)pDest, (char *)szSrc, nMax);
    return pDest;
}
inline int D2Win_SetTextFont(int nFont) {
    return D2Win::Src::D2WinFont::SetAndReturnLastFont(nFont);
}
inline void D2Win_DrawText(const Unicode * wszText, int nX, int nY, int nColor, BOOL bCentered) {
    D2Win::Src::D2WinFont::DRAW_text((uint16_t *)wszText, nX, nY, nColor, bCentered);
}
#endif

#if !D2_BACKEND_POSIX
// ---------------------------------------------------------------------------------------------
// The Storm Mac layer (CFM imports from StormLib; the Win32-on-Mac names are Storm's).
extern "C" {
extern D2MacVidTableStrc * vid;
void CopyMemory(void * pDest, const void * pSrc, uint32_t nBytes);
void FillMemory(void * pDest, uint32_t nBytes, BYTE nFill);
void ZeroMemory(void * pDest, uint32_t nBytes);
OSErr FileRefFromHANDLE(HANDLE hFile, int16_t * pRefNum);
void MacInitCursor();
void SetMousePosition(Point tWhere);
void YieldToThinThread(int nFlags);
}
int16_t FilepathToFSSpec(FSSpec * pSpec, const char * szPath, int hBase);
#endif

#endif // !defined(_WIN32) && ((D2_MAC_CARBON && D2_ARCH_PPC) || D2_BACKEND_POSIX)
