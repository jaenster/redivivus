// Win32Posix: the Win32 surface the shared game code is written against, for a build that is
// neither Windows nor Blizzard's Carbon Mac - the modern macOS build (cmake/mac-modern.cmake),
// and later Linux. It stands where Storm's Win32-on-Mac layer (Source/StormMac/Win32Mac) stands
// for the 1.14d Mac build, under the same header names:
//   Win32Posix.h              the types and macros (Win32Mac.h)
//   Win32PosixKernel.h        files, time, threads, modules, locale, strings (Win32MacKernel.h)
//   Win32PosixUser.h          windows, messages, rectangles, keyboard, cursor (Win32MacUser.h)
//   Win32PosixSockets.h       Winsock over BSD sockets (Win32MacSockets.h)
//   Win32PosixDirectSound.h   DirectSound (Win32MacDirectSound.h)
//
// Unlike Win32Mac, it is the Win32 API as Windows declares it - the A-suffixed names, the SDK's
// signatures and layouts - so the shared code takes its Windows path. The scalar and handle
// types are d2_platform.h's non-Windows stand-ins; this layer adds what those do not give, and
// d2_platform.h reads it at its end, after them. Declarations only: the implementations (SDL3
// and POSIX) are to come, and until they do nothing here links.
//
// Every file here compiles to nothing unless D2_BACKEND_POSIX, and each guard is spelled
// `#if !defined(_WIN32) && D2_BACKEND_POSIX`: the VC8 oracle's translator reads every header in
// the tree for its cross-file facts and skips only `!defined(_WIN32)` blocks, and this layer
// declares Win32 entry points that would otherwise change how Windows TUs are translated.
#pragma once

#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#ifndef FALSE
#define FALSE 0
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#define INFINITE 0xFFFFFFFF
#define WAIT_OBJECT_0 0x00000000
#define WAIT_TIMEOUT 0x00000102
#define WAIT_FAILED 0xFFFFFFFF
#define MEM_COMMIT 0x00001000
#define MEM_RESERVE 0x00002000
#define MEM_RELEASE 0x00008000
#define PAGE_NOACCESS 0x01
#define PAGE_READWRITE 0x04
#define PAGE_EXECUTE_READWRITE 0x40

// The Microsoft compiler's own spellings.
typedef long long __int64;
#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif
// <x86intrin.h> (read by d2_platform.h on x86-64) has both as macros.
#ifndef _rotl
static inline uint32_t _rotl(uint32_t dwValue, int nShift) {
    return (dwValue << (nShift & 31)) | (dwValue >> ((32 - nShift) & 31));
}
#endif
#ifndef _rotr
static inline uint32_t _rotr(uint32_t dwValue, int nShift) {
    return (dwValue >> (nShift & 31)) | (dwValue << ((32 - nShift) & 31));
}
#endif

// ---- Win32 data types the shared code declares with, that the stand-ins do not give ----
// The shared headers carry their own non-Windows stand-ins for the records they hold by value
// (RGBQUAD in Draw.h, SYSTEM_INFO and WIN32_FIND_DATAA in Fog's SFile.h, TIME_ZONE_INFORMATION
// in BNGatewayAccess.h, WAVEFORMATEX in Sound.h, the Winsock records in Safesock.h); the
// entry points here name those by their tags.
typedef CRITICAL_SECTION _RTL_CRITICAL_SECTION;
typedef struct HACCEL__* HACCEL;

struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
};
typedef tagBITMAPINFOHEADER BITMAPINFOHEADER;

// The x86 thread context and exception record: no non-Windows build has either (no SEH);
// the shared code names them in its crash handler's prototypes, and Fog/Debug.h defines
// _CONTEXT and _EXCEPTION_RECORD where it needs them whole.
struct _CONTEXT;
struct _EXCEPTION_RECORD;
typedef _EXCEPTION_RECORD EXCEPTION_RECORD;
struct _EXCEPTION_POINTERS;
struct _FLOATING_SAVE_AREA {
    DWORD ControlWord;
    DWORD StatusWord;
    DWORD TagWord;
    DWORD ErrorOffset;
    DWORD ErrorSelector;
    DWORD DataOffset;
    DWORD DataSelector;
    BYTE RegisterArea[80];
    DWORD Cr0NpxState;
};

#include "Source/StormMac/Win32Posix/Win32PosixKernel.h"
#include "Source/StormMac/Win32Posix/Win32PosixUser.h"
#include "Source/StormMac/Win32Posix/Win32PosixSockets.h"
#include "Source/StormMac/Win32Posix/Win32PosixDirectSound.h"

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
