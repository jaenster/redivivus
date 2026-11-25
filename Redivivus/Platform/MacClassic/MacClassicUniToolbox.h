// STUB HEADER - Redivivus-owned, not an Apple header.
//
// The Mac Toolbox names the PowerPC D2Uni module uses (QuickDraw, Color QuickDraw, the Palette,
// Window, Memory and Event Managers, Appearance), written from the public Inside Macintosh
// documentation so the module can be syntax-checked without the Apple SDK. Nothing is copied
// from Apple's Universal Interfaces. Only the record fields D2Uni reads or writes are named; the
// offsets are the 68K/PowerPC (mac68k-aligned) ones the PEF code addresses.
//
// The function list is the D2Uni share of the CFM imports from InterfaceLib and AppearanceLib
// in the classic LoD 1.10b PEF (d2-surveys/macppc/uni/api.md).
//
// Classic Mac PowerPC only. See Source/D2Uni/README.md (the stub set).
#pragma once

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

#include <stdint.h>

typedef int16_t OSErr;
typedef int32_t OSStatus;
typedef unsigned char Boolean;
typedef char * Ptr;
typedef Ptr * Handle;
typedef unsigned char Str255[256];
typedef unsigned char * StringPtr;
typedef int16_t QDErr;

struct Point {
    /* 0x00 */ int16_t v;
    /* 0x02 */ int16_t h;
};

struct Rect {
    /* 0x00 */ int16_t top;
    /* 0x02 */ int16_t left;
    /* 0x04 */ int16_t bottom;
    /* 0x06 */ int16_t right;
};

struct RGBColor {
    /* 0x00 */ uint16_t red;
    /* 0x02 */ uint16_t green;
    /* 0x04 */ uint16_t blue;
};

struct ColorSpec {
    /* 0x00 */ int16_t value;
    /* 0x02 */ RGBColor rgb;
};

struct ColorTable {
    /* 0x00 */ int32_t ctSeed;
    /* 0x04 */ uint16_t ctFlags;
    /* 0x06 */ int16_t ctSize;     // entries - 1
    /* 0x08 */ ColorSpec ctTable[1]; // ctSize + 1 entries
};
typedef ColorTable * CTabPtr;
typedef CTabPtr * CTabHandle;

struct BitMap {
    /* 0x00 */ Ptr baseAddr;
    /* 0x04 */ int16_t rowBytes;
    /* 0x06 */ Rect bounds;
};

struct PixMap {
    /* 0x00 */ Ptr baseAddr;
    /* 0x04 */ int16_t rowBytes;   // top two bits are flags
    /* 0x06 */ Rect bounds;
    /* 0x0E */ int16_t pmVersion;
    /* 0x10 */ int16_t packType;
    /* 0x12 */ int32_t packSize;
    /* 0x16 */ int32_t hRes;
    /* 0x1A */ int32_t vRes;
    /* 0x1E */ int16_t pixelType;
    /* 0x20 */ int16_t pixelSize;
    /* 0x22 */ int16_t cmpCount;
    /* 0x24 */ int16_t cmpSize;
    /* 0x26 */ int32_t planeBytes;
    /* 0x2A */ CTabHandle pmTable;
    /* 0x2E */ int32_t pmReserved;
};
typedef PixMap * PixMapPtr;
typedef PixMapPtr * PixMapHandle;

struct GDevice {
    /* 0x00 */ int16_t gdRefNum;
    /* 0x02 */ int16_t gdID;
    /* 0x04 */ int16_t gdType;
    /* 0x06 */ Handle gdITable;
    /* 0x0A */ int16_t gdResPref;
    /* 0x0C */ Ptr gdSearchProc;
    /* 0x10 */ Ptr gdCompProc;
    /* 0x14 */ int16_t gdFlags;
    /* 0x16 */ PixMapHandle gdPMap;
    /* 0x1A */ int32_t gdRefCon;
    /* 0x1E */ Handle gdNextGD;
    /* 0x22 */ Rect gdRect;
    /* 0x2A */ int32_t gdMode;
};
typedef GDevice * GDPtr;
typedef GDPtr * GDHandle;

// A colour graphics port. Only the two fields D2Uni reads are placed.
struct CGrafPort {
    /* 0x00 */ int16_t device;
    /* 0x02 */ PixMapHandle portPixMap;
    /* 0x06 */ int16_t portVersion;
    /* 0x08 */ Handle grafVars;
    /* 0x0C */ int16_t chExtra;
    /* 0x0E */ int16_t pnLocHFrac;
    /* 0x10 */ Rect portRect;
    /* 0x18 */ uint8_t rest[0x6C - 0x18];
};
typedef CGrafPort * CGrafPtr;
typedef CGrafPtr GrafPtr;
typedef CGrafPtr WindowPtr;
typedef WindowPtr WindowRef;

typedef struct OpaquePaletteHandle * PaletteHandle;

struct EventRecord {
    /* 0x00 */ uint16_t what;
    /* 0x02 */ uint32_t message;
    /* 0x06 */ uint32_t when;
    /* 0x0A */ Point where;
    /* 0x0E */ uint16_t modifiers;
} __attribute__((packed));

enum {
    osEvt = 15,
    suspendResumeMessage = 1,
    resumeFlag = 1,
    everyEvent = 0xFFFF,
    mUpMask = 0x0004,
    keyUpMask = 0x0010,
    pmTolerant = 0x0002,
    pmExplicit = 0x0008,
    pmAllUpdates = (int32_t)0xFFFFE000,
    srcCopy = 0,
    documentProc = 0,
    gdDevType = 0,
};

struct FSSpec {
    /* 0x00 */ int16_t vRefNum;
    /* 0x02 */ int32_t parID;
    /* 0x06 */ Str255 name;
} __attribute__((packed));

extern "C" {
// QuickDraw
void SetPort(GrafPtr port);
void GetPort(GrafPtr * port);
void CopyBits(const BitMap * srcBits, const BitMap * dstBits, const Rect * srcRect, const Rect * dstRect,
              int16_t mode, void * maskRgn);
void PaintRect(const Rect * r);
void OffsetRect(Rect * r, int16_t dh, int16_t dv);
Boolean SectRect(const Rect * src1, const Rect * src2, Rect * dstRect);
void LocalToGlobal(Point * pt);
void GlobalToLocal(Point * pt);
void HideCursor(void);
void ShowCursor(void);
int16_t GetMBarHeight(void);
// Color QuickDraw and the Graphics Device Manager
Ptr GetPixBaseAddr(PixMapHandle pm);
int32_t GetCTSeed(void);
void CTabChanged(CTabHandle ctab);
GDHandle GetDeviceList(void);
GDHandle GetNextDevice(GDHandle curDevice);
GDHandle GetMainDevice(void);
Boolean TestDeviceAttribute(GDHandle gdh, int16_t attribute);
int16_t HasDepth(GDHandle gd, int16_t depth, int16_t whichFlags, int16_t flags);
OSErr SetDepth(GDHandle gd, int16_t depth, int16_t whichFlags, int16_t flags);
// Palette Manager
PaletteHandle NewPalette(int16_t entries, CTabHandle srcColors, int16_t srcUsage, int16_t srcTolerance);
void DisposePalette(PaletteHandle srcPalette);
void NSetPalette(WindowRef dstWindow, PaletteHandle srcPalette, int32_t nCUpdates);
void ActivatePalette(WindowRef srcWindow);
void CTab2Palette(CTabHandle srcCTab, PaletteHandle dstPalette, int16_t srcUsage, int16_t srcTolerance);
// Window Manager and Appearance
WindowRef NewCWindow(void * wStorage, const Rect * boundsRect, const unsigned char * title, Boolean visible,
                     int16_t procID, WindowRef behind, Boolean goAwayFlag, int32_t refCon);
void SizeWindow(WindowRef theWindow, int16_t w, int16_t h, Boolean fUpdate);
void DisposeWindow(WindowRef theWindow);
Boolean IsWindowCollapsed(WindowRef window);
// Memory Manager
Handle NewHandleClear(int32_t byteCount);
void DisposeHandle(Handle h);
void HLock(Handle h);
void HLockHi(Handle h);
int8_t HGetState(Handle h);
void HSetState(Handle h, int8_t flags);
// Event Manager and resources
void FlushEvents(uint16_t whichMask, uint16_t stopMask);
Boolean OSEventAvail(uint16_t mask, EventRecord * theEvent);
void GetIndString(unsigned char * theString, int16_t strListID, int16_t index);
}

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
