// uniPosix.h - what the shared D2Uni slots need from the Mac Toolbox and RAD on the Win32Posix
// build (D2_BACKEND_POSIX), where neither exists. uniInternal.h reads this in place of the
// classic stub headers (Redivivus/Platform/MacClassic/MacClassicUni*.h), which stay PowerPC only.
//
// The records are the ones the slots read, natively laid out: every field is a fixed-width
// integer, so the sizes the slots spell (UniInit's 0x808-byte colour table) hold on arm64 and
// x86-64 as on the PowerPC. Nothing here is ever read from or written to a file, so byte order
// does not enter: a colour table entry is built and read by this process alone.
//
// Ours, not an Apple or RAD header. See Source/D2Uni/README.md.
#pragma once

#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char Boolean;
typedef char * Ptr;
typedef Ptr * Handle;

struct Point {
    int16_t v;
    int16_t h;
};

struct Rect {
    int16_t top;
    int16_t left;
    int16_t bottom;
    int16_t right;
};

struct RGBColor {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
};

struct ColorSpec {
    int16_t value;
    RGBColor rgb;
};

struct ColorTable {
    int32_t ctSeed;
    uint16_t ctFlags;
    int16_t ctSize;       // entries - 1
    ColorSpec ctTable[1]; // ctSize + 1 entries
};
typedef ColorTable * CTabPtr;
typedef CTabPtr * CTabHandle;

static_assert(offsetof(ColorTable, ctTable) + 256 * sizeof(ColorSpec) == 0x808,
              "UniInit allocates the 256-entry colour table as 0x808 bytes");

// A graphics device: SetOption 0 and 1 carry one. No device exists on this build; the handle is
// only stored and handed back.
typedef struct UniPosixDevice ** GDHandle;
struct EventRecord;

// The Memory Manager calls UniInit and UniRelease make on the colour table: a handle is a
// pointer to the block's pointer. Nothing here moves a block, so locking is nothing.
static inline Handle NewHandleClear(int32_t nByteCount) {
    Handle hHandle = (Handle)calloc(1, sizeof(Ptr));
    if (hHandle) {
        *hHandle = (Ptr)calloc(1, nByteCount > 0 ? (size_t)nByteCount : 1);
        if (!*hHandle) {
            free(hHandle);
            hHandle = nullptr;
        }
    }
    return hHandle;
}

static inline void DisposeHandle(Handle hHandle) {
    if (hHandle) {
        free(*hHandle);
        free(hHandle);
    }
}

static inline void HLockHi(Handle hHandle) {
    (void)hHandle;
}

// A colour table's seed tells QuickDraw's caches it changed; here it is only a fresh number.
static inline int32_t GetCTSeed() {
    static int32_t s_nSeed = 1024;
    return ++s_nSeed;
}

// The Storm Mac layer's block calls. Lengths are 32-bit, as the slots pass them.
static inline void CopyMemory(void * pDest, const void * pSrc, uint32_t nBytes) {
    memcpy(pDest, pSrc, nBytes);
}

static inline void FillMemory(void * pDest, uint32_t nBytes, BYTE nFill) {
    memset(pDest, nFill, nBytes);
}

static inline void ZeroMemory(void * pDest, uint32_t nBytes) {
    memset(pDest, 0, nBytes);
}

// ---- RAD: the Bink and Smacker record fields the slots read ----
// The calls themselves are declared by d2_platform.h; the 64-bit builds take them from
// Redivivus/x64 (d2_bink.cpp, d2_smack.cpp, on FFmpeg).
//
// That Bink hands back only the public head of RAD's record - Width, Height, Frames, FrameNum -
// with no dirty rectangles and no BinkGetRects, so this record stops there and the Bink player
// presents each frame whole on this build (uniEngine.cpp).
struct BINK {
    /* 0x00 */ uint32_t Width;
    /* 0x04 */ uint32_t Height;
    /* 0x08 */ uint32_t Frames;
    /* 0x0C */ uint32_t FrameNum;
};
typedef BINK * HBINK;

enum {
    BINKFILEHANDLE = 0x00800000,
    BINKSURFACE32 = 3, // 32-bit B, G, R, A bytes: ARGB8888 words on a little-endian host
};

// Smacker: every field up to the ones named is a 32-bit integer in RAD's layout, so the
// offsets hold on 64-bit targets as on the PowerPC.
struct Smack {
    /* 0x000 */ uint32_t Version;
    /* 0x004 */ uint32_t Width;
    /* 0x008 */ uint32_t Height;
    /* 0x00C */ uint32_t Frames;
    /* 0x010 */ uint32_t unknown10[22];
    /* 0x068 */ uint32_t NewPalette;
    /* 0x06C */ uint8_t Palette[772];
    /* 0x370 */ uint32_t PalType;
    /* 0x374 */ uint32_t FrameNum;
};
static_assert(offsetof(Smack, NewPalette) == 0x68 && offsetof(Smack, FrameNum) == 0x374, "RAD's Smack layout");

enum {
    SMACKTRACKS = 0x000FE000,
};

#endif // !defined(_WIN32) && D2_BACKEND_POSIX
