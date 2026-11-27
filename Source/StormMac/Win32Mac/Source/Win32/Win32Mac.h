// Storm's Win32-on-Mac layer, 1.14d Intel Mac (DiabloII_macho): the Win32 types and
// entry points the layer implements on pthreads and Carbon, and the kernel-object
// table they share.
//
// The original name of this header is unknown; the layout around it is the original
// one (see Source/StormMac/README.md): Win32_utils.cpp's 1.14d __FILE__ is
// ".../StormMac/Storm Mac/Win32 Mac/Source/Win32/Win32_utils.cpp" (0030ece4).
// Win32_Files.cpp, Win32_Process.cpp and Win32_TimeAndDate.cpp carry the names the
// PPC Storm 1.10 Beta's static initialisers give their code; the other files here
// (Win32_Events.cpp, Win32_Memory.cpp, Win32_Rect.cpp ...) hold units whose original
// file name no binary records.
//
// Mac-only: every file here compiles to nothing unless D2_OS_MAC. Each guard is spelled
// `#if !defined(_WIN32) && D2_OS_MAC`: the VC8 oracle's translator reads every source in
// the tree for its cross-file facts and skips only `!defined(_WIN32)` blocks, and this
// layer redeclares Win32 entry points (GetDesktopWindow ...) that would otherwise change
// how Windows TUs are translated.
#pragma once

#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

// Win32 scalar types, fixed-width so the layout holds on a 64-bit Mac too.
typedef int32_t BOOL;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef uint32_t UINT;
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef const char* LPCSTR;
typedef DWORD* LPDWORD;
typedef LONG* LPLONG;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

// A kernel-object handle is a 1-based index into the handle table (GetWin32HandlePtr).
typedef int32_t HANDLE;

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    };
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    int64_t QuadPart;
} LARGE_INTEGER;

// The layer's critical section is a recursive pthread mutex.
typedef pthread_mutex_t CRITICAL_SECTION;
typedef CRITICAL_SECTION* LPCRITICAL_SECTION;

typedef int (*FARPROC)(void);
typedef DWORD (*LPTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);

#define MEM_COMMIT 0x00001000
#define MEM_RESERVE 0x00002000
#define MEM_RELEASE 0x00008000
#define PAGE_NOACCESS 0x01
#define PAGE_READWRITE 0x04
#define PAGE_EXECUTE_READWRITE 0x40

// Kernel-object types (Win32HandleObject::dwType) whose meaning is proven: CreateEventA
// allocates 1, CreateFile 2, AllocModuleHandle 4, THREAD_Create 7. CloseHandle knows 1, 2, 4, 5 and 7 and
// halts on any other. Types 1, 4 and 7 also go on the kernel-object list
// (AddObjectToKernelPool).
enum {
    WIN32MAC_HANDLE_EVENT = 1,
    WIN32MAC_HANDLE_FILE = 2,
    WIN32MAC_HANDLE_MODULE = 4,
    WIN32MAC_HANDLE_THREAD = 7,
};

#define WAIT_OBJECT_0 0x00000000
#define WAIT_TIMEOUT 0x00000102
#define WAIT_FAILED 0xFFFFFFFF
#define INFINITE 0xFFFFFFFF

// The Carbon FSSpec, 0x46 bytes, without pulling Carbon into every file.
#pragma pack(push, 2)
struct Win32MacFSSpec {
    int16_t vRefNum;              // 0x00
    int32_t parID;                // 0x02
    uint8_t name[64];             // 0x06  Str63
};
#pragma pack(pop)

// One slot of the handle table, 0xB8 bytes. Every object can be waited on: the
// mutex/condition pair carries the signaled state. The fields from 0x54 on depend
// on the type.
struct Win32HandleObject {
    DWORD dwType;                 // 0x00  0 = free slot
    BYTE bClosed;                 // 0x04  set by Event_Close; a wait on it fails
    BYTE bSignaled;               // 0x05
    BYTE pad06[2];                // 0x06
    pthread_mutex_t tMutex;       // 0x08
    pthread_cond_t tCond;         // 0x34
    LONG nRefCount;               // 0x50
    union {
        BYTE aTypeData[0xB8 - 0x54];
        struct {                  // thread (7); a free slot keeps the next free index in dwThread
            DWORD dwThread;       // 0x54  its pthread_t
            BOOL bCloseOnExit;    // 0x58  close the handle when the thread exits
            DWORD dwExitCode;     // 0x5C
            BOOL bExited;         // 0x60
        };
        struct {                  // event (1)
            char* szEventName;    // 0x54  freed by CloseHandle
            BYTE bManualReset;    // 0x58
        };
        struct {                  // file (2)
            int16_t nRefNum;      // 0x54  File Manager refnum of the open fork
            Win32MacFSSpec tSpec; // 0x56
            BYTE bWritable;       // 0x9C  opened with GENERIC_WRITE: the volume is flushed on close
            BYTE bResourceFork;   // 0x9D
            BYTE pad9E[2];        // 0x9E
            FILETIME tCreationTime;   // 0xA0  SetFileTime keeps them; closing writes creation and write back
            FILETIME tLastAccessTime; // 0xA8
            FILETIME tLastWriteTime;  // 0xB0
        };
        struct {                  // type 5
            void* pType5Data;     // 0x54  freed by CloseHandle
        };
        struct {                  // module (4)
            void* pLibrary;       // 0x54  dlopen handle; NULL for a data-file module
            DWORD dwPSNHigh;      // 0x58  the process a module was launched as (ProcessSerialNumber)
            DWORD dwPSNLow;       // 0x5C
            char* szModulePath;   // 0x60  freed by CloseHandle
            void* pAccelList;     // 0x64  accelerator tables loaded from the module
            void* hResource;      // 0x68  a data-file module's 'WRes' resource (a Handle)
        };
    };
};
#define WIN32MAC_LAYOUT(name, cond) typedef char name[(cond) ? 1 : -1]
WIN32MAC_LAYOUT(Win32HandleObject_size, sizeof(Win32HandleObject) == 0xB8);
WIN32MAC_LAYOUT(Win32HandleObject_spec, offsetof(Win32HandleObject, tSpec) == 0x56);
WIN32MAC_LAYOUT(Win32HandleObject_fork, offsetof(Win32HandleObject, bResourceFork) == 0x9D);
WIN32MAC_LAYOUT(Win32HandleObject_times, offsetof(Win32HandleObject, tLastWriteTime) == 0xB0);
WIN32MAC_LAYOUT(Win32HandleObject_module, offsetof(Win32HandleObject, szModulePath) == 0x60);

// What CreateThread hands the new thread: THREAD_StartRoutine's argument, 0x88 bytes.
struct Win32ThreadStartBlock {
    LPTHREAD_START_ROUTINE pfnStart;  // 0x00
    LPVOID pParameter;                // 0x04
    char szName[0x80];                // 0x08
};

// The layer's own heap (0001e374, 0001e3c0): malloc/calloc underneath, flag 0x100 zeroes.
void* MemAlloc(size_t nSize, DWORD dwFlags);
void MemFree(void* pMem, DWORD dwFlags);
void* Priv_Alloc(size_t nSize);
void Priv_Free(void* pMem);

// Microseconds since start-up as two dwords, low first (0001eabd).
DWORD* MAC_GetMicroseconds64(DWORD* pCounterOut);
DWORD FPMilliseconds(void);


void SetLastError(DWORD dwErrCode);
// 1.14d mac: 00034519
void fp_fatal(const char* szFormat, ...);
// 1.14d mac: 000225d2
// Writes the file's times back when its handle closes.
void SetFileTimeAtClose(HANDLE hFile);
DWORD GetTickCount(void);
struct FSSpec;
// 1.14d mac: 0002c935
// Resolves a Win32-style path to a File Manager FSSpec (path cache, Win32_utils.cpp).
int16_t FilepathToFSSpec(FSSpec* pSpec, LPCSTR szPath, int hBase);
// 1.14d mac: 00554e40
// Creator and file type for new files; zero asks the application (creator) or the
// path's extension (type).
extern DWORD g_dwNewFileCreator;
// 1.14d mac: 00554e44
extern DWORD g_dwNewFileType;
// 1.14d mac: 00033f9a
// The running application's FSSpec and creator; every out pointer may be NULL.
int GetApplicationInformation(void* pName, FSSpec* pFileSpec, DWORD* pdwSignature, uint64_t* pPSN);
// 1.14d mac: 00033cef
DWORD GetFileTypeAndCreatorForPath(LPCSTR szPath, DWORD* pdwCreator);

// One entry of the Mac path token table: "\\{MacPathNN}\\" names entry NN (hex, 1-based),
// a volume and directory. Entries are only ever added (PATH_MakeMacPathToken).
struct Win32MacPathToken {
    int16_t nToken;               // 0x00  NN
    int16_t nVRefNum;             // 0x02
    int32_t nDirID;               // 0x04
    int32_t n08;                  // 0x08  0 from every caller seen
    int32_t n0C;                  // 0x0C  0 from every caller seen
};
// 1.14d mac: 0002c71b
BOOL PATH_IsMacPathToken(LPCSTR szPath);
// 1.14d mac: 0002c785
// The token table entry a token path names.
Win32MacPathToken* FilepathWorkingDirectory(LPCSTR szPath);
// 1.14d mac: 0002c7c3
// "\\{MacPathNN}\\name" for a directory, registering it in the token table.
void PATH_MakeMacPathToken(char* szOut, const unsigned char* pName, int16_t nVRefNum, int32_t nDirID, int32_t n08,
                           int32_t n0C);
// 1.14d mac: 0002c8f7
void FSSpecToFilepath(char* szOut, const FSSpec* pSpec);
// 1.14d mac: 0002c6e3
// 8 uppercase hex digits, not terminated.
void HEX_FormatDword(DWORD dwValue, char* pOut);

void Event_Close(HANDLE hEvent);
DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);

LONG InterlockedDecrement(LPLONG pnValue);
HANDLE AllocWin32Handle(DWORD dwType);
void FreeWin32Handle(HANDLE hHandle);
BOOL AddObjectToKernelPool(HANDLE hObject);
BOOL HANDLE_RemoveFromList(HANDLE hObject);
void SMemZero(void* pDest, size_t nSize);
Win32HandleObject* GetWin32HandlePtr(HANDLE hHandle);
HANDLE HANDLE_FindThreadByPthread(pthread_t pThread);
BOOL Event_SetSignaled(HANDLE hEvent, BOOL bSignaled);
BOOL CloseHandle(HANDLE hHandle);
// The File Manager refnum behind a file handle (Win32_Files.cpp).
BOOL HANDLE_GetFileRefNum(HANDLE hFile, int16_t* pnRefNum);


// ---- Win32_Region.cpp: strip regions (handles are 1-based indexes, 0 is none) ----
// A rect of the layer's own, the same four dwords as the Win32 RECT.
struct Win32MacRect {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};
// 1.14d mac: 00031b0d
int NewEmptyStripRgn(void);
// 1.14d mac: 00031b18
// 1.14d mac: 00031ce9
void Rgn_SetStripRgnRect(int hRgn, const Win32MacRect* pRect);
// 1.14d mac: 00031dca
void DisposeStripRgn(int hRgn);
// 1.14d mac: 00031e16
int CountStripRgnRects(int hRgn);
// 1.14d mac: 00031e3b
int GetStripRgnRects(int hRgn, Win32MacRect* pRects, int nMaxRects);
// 1.14d mac: 00031ed2
void GetStripRgnBoundingRect(int hRgn, Win32MacRect* pRect);
// 1.14d mac: 00031f17
bool IsStripRgnEmpty(int hRgn);
// 1.14d mac: 000320a6
void CopyStripRgn(int hDest, int hSrc);
// 1.14d mac: 0003215e
void Rgn_UnionStripRgn(int hDest, int hSrc1, int hSrc2);
// 1.14d mac: 00032754
void Rgn_IntersectStripRgn(int hDest, int hSrc1, int hSrc2);
// 1.14d mac: 00032887
void Rgn_SubtractStripRgn(int hDest, int hSrc1, int hSrc2);
// 1.14d mac: 000329b1
void Rgn_DestroyAllRegions(void);
// ---- end Win32_Region.cpp ----

// ---- Win32 data types the shared game code declares with ----
// The shared code (Fog, Storm, D2Net, D2Sound, D2Client ...) compiled against this layer
// on the Mac. These are the Win32 names it declares with that the layer does not define
// above: where the shared code holds one by value, the Win32 SDK's i386 layout; where it
// only passes a pointer, an incomplete type.
typedef CRITICAL_SECTION _RTL_CRITICAL_SECTION;
typedef struct HACCEL__* HACCEL;

// The x86 thread context and exception record: the Mac build has neither (its crash
// handler is a Mach exception server), the shared code names them in prototypes only.
struct _CONTEXT;
typedef _CONTEXT CONTEXT;
typedef CONTEXT* PCONTEXT;
struct _EXCEPTION_RECORD;
typedef _EXCEPTION_RECORD EXCEPTION_RECORD;
typedef EXCEPTION_RECORD* PEXCEPTION_RECORD;
struct _EXCEPTION_POINTERS;

struct tagRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
};
typedef tagRGBQUAD RGBQUAD;

struct tagPALETTEENTRY {
    BYTE peRed;
    BYTE peGreen;
    BYTE peBlue;
    BYTE peFlags;
};
typedef tagPALETTEENTRY PALETTEENTRY;
typedef PALETTEENTRY* LPPALETTEENTRY;

// The format Win32_DirectSound.cpp's buffers keep, wFormatTag through cbSize.
#pragma pack(push, 1)
struct WAVEFORMATEX {
    WORD wFormatTag;              // 0x00
    WORD nChannels;               // 0x02
    DWORD nSamplesPerSec;         // 0x04
    DWORD nAvgBytesPerSec;        // 0x08
    WORD nBlockAlign;             // 0x0C
    WORD wBitsPerSample;          // 0x0E
    WORD cbSize;                  // 0x10
};
#pragma pack(pop)
WIN32MAC_LAYOUT(WAVEFORMATEX_size, sizeof(WAVEFORMATEX) == 0x12);

struct _SYSTEM_INFO {
    DWORD dwOemId;                // 0x00
    DWORD dwPageSize;             // 0x04
    LPVOID lpMinimumApplicationAddress; // 0x08
    LPVOID lpMaximumApplicationAddress; // 0x0C
    DWORD dwActiveProcessorMask;  // 0x10
    DWORD dwNumberOfProcessors;   // 0x14
    DWORD dwProcessorType;        // 0x18
    DWORD dwAllocationGranularity; // 0x1C
    WORD wProcessorLevel;         // 0x20
    WORD wProcessorRevision;      // 0x22
};
typedef _SYSTEM_INFO SYSTEM_INFO;
WIN32MAC_LAYOUT(SYSTEM_INFO_size, sizeof(SYSTEM_INFO) == 0x24);

// What FindFirstFile / FindNextFile fill, 0x140 bytes (Win32MacFindData in Win32_Files.cpp).
struct _WIN32_FIND_DATAA {
    DWORD dwFileAttributes;       // 0x000
    FILETIME ftCreationTime;      // 0x004
    FILETIME ftLastAccessTime;    // 0x00C
    FILETIME ftLastWriteTime;     // 0x014
    DWORD nFileSizeHigh;          // 0x01C
    DWORD nFileSizeLow;           // 0x020
    DWORD dwReserved0;            // 0x024
    DWORD dwReserved1;            // 0x028
    char cFileName[260];          // 0x02C
    char cAlternateFileName[14];  // 0x130
};
WIN32MAC_LAYOUT(WIN32_FIND_DATAA_size, sizeof(_WIN32_FIND_DATAA) == 0x140);

// What WSOCK_WSAStartup fills (Windows Sockets 1.1).
struct WSAData {
    WORD wVersion;                // 0x000
    WORD wHighVersion;            // 0x002
    char szDescription[257];      // 0x004
    char szSystemStatus[129];     // 0x105
    uint16_t iMaxSockets;         // 0x186
    uint16_t iMaxUdpDg;           // 0x188
    char* lpVendorInfo;           // 0x18C
};
typedef WSAData WSADATA;
WIN32MAC_LAYOUT(WSADATA_size, sizeof(WSADATA) == 0x190);
// ---- end of the shared code's Win32 data types ----

// ---- Win32_String.cpp: the Microsoft CRT number formatting the layer supplies ----
char* _itoa(int nValue, char* szBuffer, int nRadix);
char* _ui64toa(uint64_t nValue, char* szBuffer, int nRadix);
// No body in 1.14d: its one caller, STRTABLE_FormatInt64, is not in the Mac binary either.
char* _i64toa(int64_t nValue, char* szBuffer, int nRadix);
char* _ultoa(DWORD dwValue, char* szBuffer, DWORD dwRadix);

#endif  // D2_OS_MAC
