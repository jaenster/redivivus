// Win32Posix: the Win32 surface the shared game code is written against, for a build that is
// neither Windows nor Blizzard's Carbon Mac - the modern macOS build (cmake/mac-modern.cmake),
// and later Linux. It stands where Storm's Win32-on-Mac layer (Source/StormMac/Win32Mac) stands
// for the 1.14d Mac build, under the same header names:
//   Win32Posix.h              the types and macros (Win32Mac.h)
//   Win32PosixKernel.h        files, time, threads, modules, locale, strings (Win32MacKernel.h)
//   Win32PosixUser.h          windows, messages, rectangles, keyboard, cursor (Win32MacUser.h)
//   Win32PosixSockets.h       Winsock over BSD sockets (Win32MacSockets.h)
//   Win32PosixDirectSound.h   DirectSound (Win32MacDirectSound.h)
//   Win32PosixShell.h         the process entry, the crash path, the taskbar, URLs
//
// Unlike Win32Mac, it is the Win32 API as Windows declares it - the A-suffixed names, the SDK's
// signatures and layouts - so the shared code takes its Windows path. It owns every Win32 type,
// as windows.h does on Windows and Win32Mac.h on the 1.14d Mac: d2_platform.h reads it in place
// of its non-Windows stand-ins, and the module headers' stand-ins (Fog/Debug.h, Safesock.h,
// SFile.h, Draw.h, Sound.h, BNGatewayAccess.h, d2_enums.h) are skipped under it. The scalars are
// the SDK's fixed widths - DWORD, LONG, ULONG and BOOL are 32 bits on an LP64 target too - and
// every record holds the layout Windows gives it at the same pointer width (checked below).
// Declarations only: the implementations (SDL3 and POSIX) are to come, and until they do
// nothing here links.
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
#include <stdarg.h>

// The Windows size of a record at this target's pointer width: x86 on a 32-bit target, x64
// (LLP64, which an LP64 target matches once DWORD and LONG are 32 bits) on a 64-bit one.
#if UINTPTR_MAX == 0xFFFFFFFFu
#  define WIN32POSIX_SIZE(type, nX86, nX64) static_assert(sizeof(type) == (nX86), #type ": the Win32 x86 size")
#else
#  define WIN32POSIX_SIZE(type, nX86, nX64) static_assert(sizeof(type) == (nX64), #type ": the Win32 x64 size")
#endif

// ---- scalars (windef.h, winnt.h, basetsd.h) ----
typedef int32_t BOOL;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;
typedef char CHAR;
typedef uint16_t WCHAR;
typedef char TCHAR;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef int16_t SHORT;
typedef uint16_t USHORT;
typedef int32_t INT;
typedef uint32_t UINT;
typedef unsigned char UCHAR;
typedef float FLOAT;
typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;
typedef uint64_t DWORD64;
typedef uint64_t DWORDLONG;
typedef int64_t LONG64;
typedef uint64_t ULONG64;
typedef intptr_t INT_PTR;
typedef uintptr_t UINT_PTR;
typedef intptr_t LONG_PTR;
typedef uintptr_t ULONG_PTR;
typedef uintptr_t DWORD_PTR;
typedef size_t SIZE_T;
typedef intptr_t SSIZE_T;
typedef WORD LANGID;
typedef DWORD LCID;
typedef DWORD LCTYPE;
typedef LONG HRESULT;
typedef LONG LSTATUS;
typedef WORD ATOM;
typedef DWORD COLORREF;
typedef uint16_t u_short;
typedef uint32_t u_int;
typedef uint32_t u_long;
typedef unsigned char u_char;
static_assert(sizeof(DWORD) == 4 && sizeof(LONG) == 4 && sizeof(ULONG) == 4 && sizeof(BOOL) == 4,
              "the Win32 scalars are 32 bits on every target");

// ---- pointers and handles ----
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef void* PVOID;
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef const char* LPCCH;
typedef WCHAR* LPWSTR;
typedef const WCHAR* LPCWSTR;
typedef const WCHAR* LPCWCH;
typedef BYTE* LPBYTE;
typedef BYTE* PBYTE;
typedef WORD* LPWORD;
typedef DWORD* LPDWORD;
typedef DWORD* PDWORD;
typedef LONG* LPLONG;
typedef LONG* PLONG;
typedef int* LPINT;
typedef BOOL* LPBOOL;
typedef void* HANDLE;
typedef HANDLE* LPHANDLE;
typedef HANDLE HGLOBAL;
typedef HANDLE HLOCAL;
typedef void* HGDIOBJ;
// The other handles are plain pointers, as the stand-ins and Win32Mac.h give them to the
// shared Mac code; the ones the module headers declare as distinct types (D2WinMain.h,
// Message.h, Chat.h) take windows.h's STRICT spelling.
typedef void* HINSTANCE;
typedef void* HMODULE;
typedef void* HWND;
typedef void* HDC;
typedef void* HBITMAP;
typedef void* HFONT;
typedef void* HBRUSH;
typedef void* HPEN;
typedef void* HKEY;
typedef void* HICON;
typedef void* HCURSOR;
typedef void* HMENU;
typedef void* HPALETTE;
typedef void* HRGN;
typedef void* HRSRC;
typedef struct HMONITOR__* HMONITOR;
typedef struct HACCEL__* HACCEL;
typedef struct HIMC__* HIMC;
typedef struct HKL__* HKL;
typedef void* HCERTSTORE;
typedef void* HCRYPTMSG;
typedef HKEY* PHKEY;
typedef int32_t REGSAM;
typedef intptr_t LRESULT;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef intptr_t (*FARPROC)();
typedef LRESULT (*WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef DWORD (*PTHREAD_START_ROUTINE)(LPVOID);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;
typedef void* LPSERVICE_STATUS;
typedef uintptr_t SOCKET;

// ---- constants ----
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
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

// ---- records (the SDK's layouts) ----
struct tagPOINT {
    LONG x;
    LONG y;
};
typedef tagPOINT POINT;
typedef POINT* LPPOINT;
WIN32POSIX_SIZE(POINT, 0x08, 0x08);

struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};
typedef tagRECT RECT;
typedef RECT* LPRECT;
typedef const RECT* LPCRECT;
WIN32POSIX_SIZE(RECT, 0x10, 0x10);

struct tagSIZE {
    LONG cx;
    LONG cy;
};
typedef tagSIZE SIZE;

struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
};
typedef _FILETIME FILETIME;
typedef FILETIME* LPFILETIME;
WIN32POSIX_SIZE(FILETIME, 0x08, 0x08);

struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
};
typedef _SYSTEMTIME SYSTEMTIME;
typedef SYSTEMTIME* LPSYSTEMTIME;
WIN32POSIX_SIZE(SYSTEMTIME, 0x10, 0x10);

struct _TIME_ZONE_INFORMATION {
    LONG Bias;
    WCHAR StandardName[32];
    SYSTEMTIME StandardDate;
    LONG StandardBias;
    WCHAR DaylightName[32];
    SYSTEMTIME DaylightDate;
    LONG DaylightBias;
};
typedef _TIME_ZONE_INFORMATION TIME_ZONE_INFORMATION;
WIN32POSIX_SIZE(TIME_ZONE_INFORMATION, 0xAC, 0xAC);

union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    };
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
};
typedef _LARGE_INTEGER LARGE_INTEGER;
typedef LARGE_INTEGER* PLARGE_INTEGER;
WIN32POSIX_SIZE(LARGE_INTEGER, 0x08, 0x08);

union _ULARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    };
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    ULONGLONG QuadPart;
};
typedef _ULARGE_INTEGER ULARGE_INTEGER;
typedef ULARGE_INTEGER* PULARGE_INTEGER;
WIN32POSIX_SIZE(ULARGE_INTEGER, 0x08, 0x08);

struct _GUID {
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[8];
};
typedef _GUID GUID;
WIN32POSIX_SIZE(GUID, 0x10, 0x10);

// The critical section keeps the SDK's record; the implementation hangs its lock off it.
struct _RTL_CRITICAL_SECTION_DEBUG;
struct _RTL_CRITICAL_SECTION {
    _RTL_CRITICAL_SECTION_DEBUG* DebugInfo;
    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;
    HANDLE LockSemaphore;
    ULONG_PTR SpinCount;
};
typedef _RTL_CRITICAL_SECTION CRITICAL_SECTION;
typedef CRITICAL_SECTION* LPCRITICAL_SECTION;
WIN32POSIX_SIZE(CRITICAL_SECTION, 0x18, 0x28);

struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union {
        struct {
            DWORD Offset;
            DWORD OffsetHigh;
        };
        PVOID Pointer;
    };
    HANDLE hEvent;
};
typedef _OVERLAPPED OVERLAPPED;
typedef OVERLAPPED* LPOVERLAPPED;
WIN32POSIX_SIZE(OVERLAPPED, 0x14, 0x20);

struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
};
typedef _SECURITY_ATTRIBUTES SECURITY_ATTRIBUTES;
typedef SECURITY_ATTRIBUTES* LPSECURITY_ATTRIBUTES;
WIN32POSIX_SIZE(SECURITY_ATTRIBUTES, 0x0C, 0x18);

struct _MEMORYSTATUS {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    SIZE_T dwTotalPhys;
    SIZE_T dwAvailPhys;
    SIZE_T dwTotalPageFile;
    SIZE_T dwAvailPageFile;
    SIZE_T dwTotalVirtual;
    SIZE_T dwAvailVirtual;
};
typedef _MEMORYSTATUS MEMORYSTATUS;
typedef MEMORYSTATUS* LPMEMORYSTATUS;
WIN32POSIX_SIZE(MEMORYSTATUS, 0x20, 0x38);

struct _MEMORY_BASIC_INFORMATION {
    PVOID BaseAddress;
    PVOID AllocationBase;
    DWORD AllocationProtect;
    SIZE_T RegionSize;
    DWORD State;
    DWORD Protect;
    DWORD Type;
};
typedef _MEMORY_BASIC_INFORMATION MEMORY_BASIC_INFORMATION;
typedef MEMORY_BASIC_INFORMATION* PMEMORY_BASIC_INFORMATION;
WIN32POSIX_SIZE(MEMORY_BASIC_INFORMATION, 0x1C, 0x30);

struct _SYSTEM_INFO {
    union {
        DWORD dwOemId;
        struct {
            WORD wProcessorArchitecture;
            WORD wReserved;
        };
    };
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
};
typedef _SYSTEM_INFO SYSTEM_INFO;
typedef SYSTEM_INFO* LPSYSTEM_INFO;
WIN32POSIX_SIZE(SYSTEM_INFO, 0x24, 0x30);

struct _WIN32_FIND_DATAA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR cFileName[MAX_PATH];
    CHAR cAlternateFileName[14];
};
typedef _WIN32_FIND_DATAA WIN32_FIND_DATAA;
typedef WIN32_FIND_DATAA* LPWIN32_FIND_DATAA;
WIN32POSIX_SIZE(WIN32_FIND_DATAA, 0x140, 0x140);

struct tagRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
};
typedef tagRGBQUAD RGBQUAD;
WIN32POSIX_SIZE(RGBQUAD, 0x04, 0x04);

struct tagPALETTEENTRY {
    BYTE peRed;
    BYTE peGreen;
    BYTE peBlue;
    BYTE peFlags;
};
typedef tagPALETTEENTRY PALETTEENTRY;
typedef PALETTEENTRY* LPPALETTEENTRY;
WIN32POSIX_SIZE(PALETTEENTRY, 0x04, 0x04);

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
WIN32POSIX_SIZE(BITMAPINFOHEADER, 0x28, 0x28);

// mmsystem.h packs it to 1: 0x12 bytes.
#pragma pack(push, 1)
struct tWAVEFORMATEX {
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
    WORD cbSize;
};
#pragma pack(pop)
typedef tWAVEFORMATEX WAVEFORMATEX;
typedef WAVEFORMATEX* LPWAVEFORMATEX;
WIN32POSIX_SIZE(WAVEFORMATEX, 0x12, 0x12);

struct tagMSG {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
};
typedef tagMSG MSG;
typedef MSG* LPMSG;
WIN32POSIX_SIZE(MSG, 0x1C, 0x30);

// The display mode record ChangeDisplaySettingsA takes (wingdi.h), the display arm of its
// union: 0x9C bytes, which the shared code writes into dmSize.
struct _devicemodeA {
    BYTE dmDeviceName[32];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;
    struct {
        POINT dmPosition;
        DWORD dmDisplayOrientation;
        DWORD dmDisplayFixedOutput;
    };
    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    BYTE dmFormName[32];
    WORD dmLogPixels;
    DWORD dmBitsPerPel;
    DWORD dmPelsWidth;
    DWORD dmPelsHeight;
    DWORD dmDisplayFlags;
    DWORD dmDisplayFrequency;
    DWORD dmICMMethod;
    DWORD dmICMIntent;
    DWORD dmMediaType;
    DWORD dmDitherType;
    DWORD dmReserved1;
    DWORD dmReserved2;
    DWORD dmPanningWidth;
    DWORD dmPanningHeight;
};
typedef _devicemodeA DEVMODEA;
typedef DEVMODEA* LPDEVMODEA;
WIN32POSIX_SIZE(DEVMODEA, 0x9C, 0x9C);

// The IME candidate list (imm.h).
struct tagCANDIDATELIST {
    DWORD dwSize;
    DWORD dwStyle;
    DWORD dwCount;
    DWORD dwSelection;
    DWORD dwPageStart;
    DWORD dwPageSize;
    DWORD dwOffset[1];
};
typedef tagCANDIDATELIST CANDIDATELIST;
typedef CANDIDATELIST* LPCANDIDATELIST;
WIN32POSIX_SIZE(CANDIDATELIST, 0x1C, 0x1C);

// The exception record and the thread context: no POSIX build raises SEH, but the shared
// crash handler (Fog's ErrorManager and Debug.h) names them. The record keeps the SDK layout;
// the context is the target's own and stays opaque.
struct _EXCEPTION_RECORD {
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    _EXCEPTION_RECORD* ExceptionRecord;
    PVOID ExceptionAddress;
    DWORD NumberParameters;
    ULONG_PTR ExceptionInformation[15];
};
typedef _EXCEPTION_RECORD EXCEPTION_RECORD;
typedef EXCEPTION_RECORD* PEXCEPTION_RECORD;
WIN32POSIX_SIZE(EXCEPTION_RECORD, 0x50, 0x98);
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
typedef _FLOATING_SAVE_AREA FLOATING_SAVE_AREA;
WIN32POSIX_SIZE(FLOATING_SAVE_AREA, 0x70, 0x70);
struct _CONTEXT;
typedef _CONTEXT CONTEXT;
typedef CONTEXT* PCONTEXT;
typedef CONTEXT* LPCONTEXT;
struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;
};
typedef _EXCEPTION_POINTERS EXCEPTION_POINTERS;
typedef EXCEPTION_POINTERS* PEXCEPTION_POINTERS;
typedef EXCEPTION_POINTERS* LPEXCEPTION_POINTERS;

// CryptoAPI and WinTrust (wincrypt.h): the records D2Hell's signature check declares with.
struct _CRYPTOAPI_BLOB {
    DWORD cbData;
    BYTE* pbData;
};
typedef _CRYPTOAPI_BLOB CRYPT_INTEGER_BLOB;
typedef _CRYPTOAPI_BLOB CERT_NAME_BLOB;
typedef _CRYPTOAPI_BLOB CRYPT_OBJID_BLOB;
typedef _CRYPTOAPI_BLOB CRYPT_DATA_BLOB;
WIN32POSIX_SIZE(CRYPT_INTEGER_BLOB, 0x08, 0x10);
struct _CRYPT_ALGORITHM_IDENTIFIER {
    LPSTR pszObjId;
    CRYPT_OBJID_BLOB Parameters;
};
typedef _CRYPT_ALGORITHM_IDENTIFIER CRYPT_ALGORITHM_IDENTIFIER;
struct _CRYPT_BIT_BLOB {
    DWORD cbData;
    BYTE* pbData;
    DWORD cUnusedBits;
};
typedef _CRYPT_BIT_BLOB CRYPT_BIT_BLOB;
struct _CERT_PUBLIC_KEY_INFO {
    CRYPT_ALGORITHM_IDENTIFIER Algorithm;
    CRYPT_BIT_BLOB PublicKey;
};
typedef _CERT_PUBLIC_KEY_INFO CERT_PUBLIC_KEY_INFO;
struct _CERT_EXTENSION {
    LPSTR pszObjId;
    BOOL fCritical;
    CRYPT_OBJID_BLOB Value;
};
typedef _CERT_EXTENSION* PCERT_EXTENSION;
struct _CERT_INFO {
    DWORD dwVersion;
    CRYPT_INTEGER_BLOB SerialNumber;
    CRYPT_ALGORITHM_IDENTIFIER SignatureAlgorithm;
    CERT_NAME_BLOB Issuer;
    FILETIME NotBefore;
    FILETIME NotAfter;
    CERT_NAME_BLOB Subject;
    CERT_PUBLIC_KEY_INFO SubjectPublicKeyInfo;
    CRYPT_BIT_BLOB IssuerUniqueId;
    CRYPT_BIT_BLOB SubjectUniqueId;
    DWORD cExtension;
    PCERT_EXTENSION rgExtension;
};
typedef _CERT_INFO CERT_INFO;
typedef CERT_INFO* PCERT_INFO;
WIN32POSIX_SIZE(CERT_INFO, 0x70, 0xD0);
struct _CERT_CONTEXT {
    DWORD dwCertEncodingType;
    BYTE* pbCertEncoded;
    DWORD cbCertEncoded;
    PCERT_INFO pCertInfo;
    HCERTSTORE hCertStore;
};
typedef _CERT_CONTEXT CERT_CONTEXT;
typedef CERT_CONTEXT* PCERT_CONTEXT;
typedef const CERT_CONTEXT* PCCERT_CONTEXT;

#include "Source/StormMac/Win32Posix/Win32PosixKernel.h"
#include "Source/StormMac/Win32Posix/Win32PosixUser.h"
#include "Source/StormMac/Win32Posix/Win32PosixSockets.h"
#include "Source/StormMac/Win32Posix/Win32PosixDirectSound.h"
#include "Source/StormMac/Win32Posix/Win32PosixShell.h"

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
