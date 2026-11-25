// STUB HEADER - Redivivus-owned, not an Apple header.
//
// The minimum of the classic Mac OS Toolbox (QuickDraw, Window, Event, Memory Manager) that
// the PowerPC D2Rave module calls, so it can be syntax-checked without an Apple SDK. Every name
// here is one the module uses; the set comes from the CFM import list of the classic 1.09d
// Diablo II PEF (InterfaceLib). Layouts and signatures are written from the public Inside
// Macintosh documentation of the calls, not copied from any Apple header. Only the fields the
// module reads are named; the struct offsets in comments are the classic 68K/PPC (mac68k
// alignment) offsets D2Rave dereferences.
//
// Classic Mac PowerPC only (the CodeWarrior/CFM builds). See README.md beside this file.
#pragma once

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

#include <stdint.h>

#pragma pack(push, 2)

typedef unsigned char Boolean;
typedef int16_t OSErr;
typedef int32_t OSStatus;
typedef char * Ptr;
typedef Ptr * Handle;
typedef const unsigned char * ConstStr255Param;

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
    uint16_t red;
    uint16_t green;
    uint16_t blue;
};

struct ColorSpec {
    /* 0x00 */ int16_t value;
    /* 0x02 */ RGBColor rgb;
};

struct ColorTable {
    /* 0x00 */ int32_t ctSeed;
    /* 0x04 */ int16_t ctFlags;
    /* 0x06 */ int16_t ctSize;
    /* 0x08 */ ColorSpec ctTable[1];
};
typedef ColorTable ** CTabHandle;

struct PixMap {
    /* 0x00 */ Ptr baseAddr;
    /* 0x04 */ int16_t rowBytes;    // high two bits are flags
    /* 0x06 */ Rect bounds;
    /* 0x0e */ int16_t pmVersion;
    /* 0x10 */ int16_t packType;
    /* 0x12 */ int32_t packSize;
    /* 0x16 */ int32_t hRes;
    /* 0x1a */ int32_t vRes;
    /* 0x1e */ int16_t pixelType;
    /* 0x20 */ int16_t pixelSize;
    /* 0x22 */ int16_t cmpCount;
    /* 0x24 */ int16_t cmpSize;
    /* 0x26 */ int32_t planeBytes;
    /* 0x2a */ CTabHandle pmTable;
    /* 0x2e */ int32_t pmReserved;
};
typedef PixMap * PixMapPtr;
typedef PixMap ** PixMapHandle;

struct GDevice {
    /* 0x00 */ int16_t gdRefNum;
    /* 0x02 */ int16_t gdID;
    /* 0x04 */ int16_t gdType;
    /* 0x06 */ Handle gdITable;
    /* 0x0a */ int16_t gdResPref;
    /* 0x0c */ Handle gdSearchProc;
    /* 0x10 */ Handle gdCompProc;
    /* 0x14 */ int16_t gdFlags;
    /* 0x16 */ PixMapHandle gdPMap;
    /* 0x1a */ int32_t gdRefCon;
    /* 0x1e */ Handle gdNextGD;
    /* 0x22 */ Rect gdRect;
};
typedef GDevice * GDPtr;
typedef GDevice ** GDHandle;

// A colour graphics port; D2Rave reads only portPixMap and portRect.
struct CGrafPort {
    /* 0x00 */ int16_t device;
    /* 0x02 */ PixMapHandle portPixMap;
    /* 0x06 */ int16_t portVersion;
    /* 0x08 */ Handle grafVars;
    /* 0x0c */ int16_t chExtra;
    /* 0x0e */ int16_t pnLocHFrac;
    /* 0x10 */ Rect portRect;
};
typedef CGrafPort * CGrafPtr;
typedef CGrafPort * GrafPtr;
typedef CGrafPort * WindowPtr;

struct EventRecord {
    /* 0x00 */ uint16_t what;
    /* 0x02 */ uint32_t message;
    /* 0x06 */ uint32_t when;
    /* 0x0a */ Point where;
    /* 0x0e */ uint16_t modifiers;
};

typedef void * GWorldPtr;

#pragma pack(pop)

enum {
    mDownMask = 0x0002,
    mUpMask = 0x0004,
    keyDownMask = 0x0008,
    keyUpMask = 0x0010,
    autoKeyMask = 0x0020,
    everyEvent = 0xffff,
};

extern "C" {
// Event Manager
void FlushEvents(uint16_t whichMask, uint16_t stopMask);
Boolean OSEventAvail(uint16_t mask, EventRecord * theEvent);
// Window Manager
WindowPtr NewCWindow(void * wStorage, const Rect * boundsRect, ConstStr255Param title, Boolean visible,
                     int16_t procID, WindowPtr behind, Boolean goAwayFlag, int32_t refCon);
void BringToFront(WindowPtr theWindow);
void DisposeWindow(WindowPtr theWindow);
// QuickDraw
void SetPort(GrafPtr port);
void PaintRect(const Rect * r);
Ptr GetPixBaseAddr(PixMapHandle pm);
// Memory Manager
void BlockMoveData(const void * srcPtr, void * destPtr, int32_t byteCount);
}

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
