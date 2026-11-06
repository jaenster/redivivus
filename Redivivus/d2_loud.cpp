// The loud build's runtime handlers (REDIVIVUS_LOUD). Compiled into that build only;
// the normal build never sees this file.
//
// The loud build compiles every game TU with
//   -ftrivial-auto-var-init=pattern   uninitialised locals hold 0xFE bytes, not stale stack
//   -fstack-protector-strong          a write past a local array smashes a canary
//   -fsanitize=bounds-strict          an index past a declared array calls a handler
// and this file supplies the handlers those flags call. mingw has no ubsan runtime, so
// the two out-of-bounds entry points are defined here against the ABI GCC emits.
//
// Out of bounds: logged once per check site, then the game continues. Smashed canary:
// logged, the crash report is written, and the process ends; a frame whose canary is
// gone has a return address that cannot be trusted.
//
// Guard zones: the generator follows every global and static datum with a 32-byte
// pattern (D2_GLOBAL_GUARD and friends in d2_platform.h, REDIVIVUS_LOUD_GUARDS) and an
// entry in the .d2grd section. D2LOUD_GuardPump walks them from the message loops, at
// most once per 40 ms, and once more at exit; a damaged guard is logged once as
//   D2GUARD <owner> +<offset into the owner> bytes=<the 32 guard bytes> frame=<walk> next=<next global>
// to this report and to the trace (d2trace.log). D2_GUARD_HALT=1 makes the first hit
// fatal (crash report, process ends); D2_GUARD_WATCH=<owner name> arms a hardware write
// watch on that owner's guard on the game thread, so the trace names the writing EIP.
//
// Everything goes to d2loud-YYYYMMDD-HHMMSS.txt next to the exe (%TEMP% when that
// folder is not writable). The game thread only records the site and walks the EBP
// chain; a worker thread formats and symbolises it with libbacktrace, so the first
// report does not stall the game while the DWARF is parsed.
//
// This file must be compiled WITHOUT the loud flags: a bounds check in the handler
// would recurse, and the canary check in __stack_chk_fail's own frame is meaningless.

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "thirdparty/libbacktrace/backtrace.h"
#include "support/d2_trace.h"

#ifndef REDIVIVUS_BUILD_ID
#define REDIVIVUS_BUILD_ID "unknown"
#endif

extern "C" void D2CRASH_ReportHalt(int nLine, unsigned nAddress);

// ---------------------------------------------------------------------------
// The ubsan data GCC emits beside each check (gcc/ubsan.cc; the same layout as
// compiler-rt's ubsan_value.h and ubsan_handlers.h).
// ---------------------------------------------------------------------------
struct UbsanSourceLocation {
    const char* pszFile;
    unsigned int nLine;
    unsigned int nColumn;
};

// nKind 0 is an integer: bit 0 of nInfo is signedness, nInfo >> 1 is log2(bit width).
// szName is the quoted type name, e.g. 'int [10]'.
struct UbsanTypeDescriptor {
    unsigned short nKind;
    unsigned short nInfo;
    char szName[1];
};

struct UbsanOutOfBoundsData {
    UbsanSourceLocation loc;
    const UbsanTypeDescriptor* pArrayType;
    const UbsanTypeDescriptor* pIndexType;
};

// An integer no wider than a pointer is passed by value, a wider one by address.
typedef UINT_PTR UbsanValueHandle;

namespace {

// ---------------------------------------------------------------------------
// The exe's own extent and the report file.
// ---------------------------------------------------------------------------
DWORD g_dwExeLo = 0, g_dwExeHi = 0;
char g_szExePath[MAX_PATH];

void InitExe() {
    if (g_dwExeLo) return;
    DWORD n = GetModuleFileNameA(0, g_szExePath, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) g_szExePath[0] = 0;
    HMODULE hExe = GetModuleHandleA(0);
    const IMAGE_NT_HEADERS* pNt =
        (const IMAGE_NT_HEADERS*)((const char*)hExe + ((const IMAGE_DOS_HEADER*)hExe)->e_lfanew);
    g_dwExeHi = (DWORD)(UINT_PTR)hExe + pNt->OptionalHeader.SizeOfImage;
    g_dwExeLo = (DWORD)(UINT_PTR)hExe;
}

HANDLE g_hFile = INVALID_HANDLE_VALUE;
bool g_bOpenTried = false;

// Same folder rule and naming as the crash report (support/d2_crash.cpp).
bool OpenLog() {
    if (g_bOpenTried) return g_hFile != INVALID_HANDLE_VALUE;
    g_bOpenTried = true;
    SYSTEMTIME st;
    GetLocalTime(&st);
    char szDir[MAX_PATH];
    lstrcpyA(szDir, g_szExePath);
    DWORD n = lstrlenA(szDir);
    while (n > 0 && szDir[n - 1] != '\\' && szDir[n - 1] != '/') --n;
    szDir[n] = 0;
    for (int nTry = 0; nTry < 2; ++nTry) {
        if (nTry == 1) {
            DWORD m = GetTempPathA(MAX_PATH, szDir);
            if (m == 0 || m >= MAX_PATH - 40) return false;
        }
        for (int nSuffix = 1; nSuffix < 10; ++nSuffix) {
            char szPath[MAX_PATH + 40];
            if (nSuffix == 1)
                wsprintfA(szPath, "%sd2loud-%04u%02u%02u-%02u%02u%02u.txt", szDir, st.wYear,
                          st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
            else
                wsprintfA(szPath, "%sd2loud-%04u%02u%02u-%02u%02u%02u-%d.txt", szDir, st.wYear,
                          st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, nSuffix);
            g_hFile = CreateFileA(szPath, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_NEW,
                                  FILE_ATTRIBUTE_NORMAL, 0);
            if (g_hFile != INVALID_HANDLE_VALUE) {
                OutputDebugStringA("Redivivus loud report: ");
                OutputDebugStringA(szPath);
                OutputDebugStringA("\n");
                return true;
            }
            if (GetLastError() != ERROR_FILE_EXISTS) break;
        }
    }
    return false;
}

void Out(const char* pszFormat, ...) {
    char szLine[2048];
    va_list args;
    va_start(args, pszFormat);
    int n = vsnprintf(szLine, sizeof(szLine), pszFormat, args);
    va_end(args);
    if (n <= 0 || g_hFile == INVALID_HANDLE_VALUE) return;
    if (n >= (int)sizeof(szLine)) n = sizeof(szLine) - 1;
    DWORD dwWritten;
    WriteFile(g_hFile, szLine, (DWORD)n, &dwWritten, 0);
}

const char* BaseName(const char* pszPath) {
    const char* p = pszPath;
    for (const char* q = pszPath; *q; ++q)
        if (*q == '\\' || *q == '/') p = q + 1;
    return p;
}

void ModuleOffset(DWORD dwAddr, char* pszOut) {
    HMODULE hMod = 0;
    char szPath[MAX_PATH];
    if (dwAddr && GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                     GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                     (LPCSTR)(UINT_PTR)dwAddr, &hMod) &&
        GetModuleFileNameA(hMod, szPath, MAX_PATH)) {
        wsprintfA(pszOut, "%s+0x%06lX", BaseName(szPath),
                  (unsigned long)(dwAddr - (DWORD)(UINT_PTR)hMod));
    } else {
        wsprintfA(pszOut, "0x%08lX", (unsigned long)dwAddr);
    }
}

// ---------------------------------------------------------------------------
// Symbolisation (worker thread only; libbacktrace state is single-threaded).
// The demangling is the same as the crash report's: strip the i386 call
// decoration, then libstdc++'s callback demangler, which does not allocate.
// ---------------------------------------------------------------------------
backtrace_state* g_pBtState = 0;
bool g_bBtTried = false;

void OnBtError(void*, const char* pszMsg, int nErr) {
    if (nErr == -1) Out("  (no debug info: %s)\r\n", pszMsg);
}

extern "C" int __gcclibcxx_demangle_callback(const char* pszMangled,
                                             void (*pfnSink)(const char*, size_t, void*),
                                             void* pOpaque);

struct DemangleSink {
    char* pszOut;
    size_t nCap;
    size_t nLen;
};

void DemangleAppend(const char* p, size_t n, void* pOpaque) {
    DemangleSink* pSink = (DemangleSink*)pOpaque;
    if (pSink->nLen + n >= pSink->nCap) n = pSink->nCap - 1 - pSink->nLen;
    memcpy(pSink->pszOut + pSink->nLen, p, n);
    pSink->nLen += n;
    pSink->pszOut[pSink->nLen] = 0;
}

const char* Demangle(const char* pszName) {
    static char s_szMangled[2048];
    static char s_szDemangled[4096];
    if (!pszName) return "?";
    const char* p = pszName;
    if (*p == '@') ++p;
    if (p[0] == '_' && p[1] == '_' && p[2] == 'Z') ++p;
    if (p[0] != '_' || p[1] != 'Z') return pszName;
    size_t n = strlen(p);
    if (n >= sizeof(s_szMangled)) return pszName;
    size_t nEnd = n;
    while (nEnd > 0 && p[nEnd - 1] >= '0' && p[nEnd - 1] <= '9') --nEnd;
    if (nEnd < n && nEnd > 0 && p[nEnd - 1] == '@') n = nEnd - 1;
    memcpy(s_szMangled, p, n);
    s_szMangled[n] = 0;
    DemangleSink sink = { s_szDemangled, sizeof(s_szDemangled), 0 };
    s_szDemangled[0] = 0;
    if (__gcclibcxx_demangle_callback(s_szMangled, DemangleAppend, &sink) != 0 || !sink.nLen)
        return pszName;
    return s_szDemangled;
}

struct PcInfo {
    int nHits;
};

int OnPcInfo(void* pData, uintptr_t, const char* pszFile, int nLine, const char* pszFunc) {
    PcInfo* pInfo = (PcInfo*)pData;
    if (!pszFunc && !pszFile) return 0;
    Out(pInfo->nHits ? "\r\n          inlined in %s" : "  %s", Demangle(pszFunc));
    if (pszFile) Out("  %s:%d", pszFile, nLine);
    ++pInfo->nHits;
    return pInfo->nHits >= 3;
}

void OnSymInfo(void* pData, uintptr_t uPc, const char* pszName, uintptr_t uVal, uintptr_t) {
    if (!pszName) return;
    Out("  %s+0x%lX", Demangle(pszName), (unsigned long)(uPc - uVal));
    ++((PcInfo*)pData)->nHits;
}

void OnBtErrorQuiet(void*, const char*, int) {}

// One frame: module+offset, then function and file:line when this exe holds it.
// Frame 0 is the check site itself (the instruction after the handler call);
// the rest are return addresses, stepped back one byte into their CALL.
void WriteFrame(int i, DWORD dwAddr) {
    char szMod[MAX_PATH + 40];
    ModuleOffset(dwAddr, szMod);
    Out("    #%-2d %s", i, szMod);
    if (!g_bBtTried) {
        g_bBtTried = true;
        if (g_szExePath[0]) g_pBtState = backtrace_create_state(g_szExePath, 0, OnBtError, 0);
    }
    if (g_pBtState && dwAddr >= g_dwExeLo && dwAddr < g_dwExeHi) {
        PcInfo info = { 0 };
        backtrace_pcinfo(g_pBtState, dwAddr - 1, OnPcInfo, OnBtErrorQuiet, &info);
        if (!info.nHits) backtrace_syminfo(g_pBtState, dwAddr - 1, OnSymInfo, OnBtErrorQuiet, &info);
    }
    Out("\r\n");
}

// ---------------------------------------------------------------------------
// The stack, walked on the reporting thread along the EBP chain. The game is
// built without optimisation, so every game frame has one. Each link is checked
// against the thread's stack bounds, and a frame is kept only when its return
// address lies inside this exe, so a smashed frame ends the walk rather than
// faulting it.
// ---------------------------------------------------------------------------
const int MAX_FRAMES = 12;

int WalkStack(DWORD dwFirst, DWORD dwEbp, DWORD* pFrames) {
    int n = 0;
    pFrames[n++] = dwFirst;
    NT_TIB* pTib = (NT_TIB*)NtCurrentTeb();
    DWORD dwBase = (DWORD)(UINT_PTR)pTib->StackBase;
    DWORD dwLimit = (DWORD)(UINT_PTR)pTib->StackLimit;
    DWORD dwFrame = dwEbp;
    while (n < MAX_FRAMES) {
        if (dwFrame < dwLimit || dwFrame + 8 > dwBase || (dwFrame & 3)) break;
        const DWORD* p = (const DWORD*)(UINT_PTR)dwFrame;
        DWORD dwRet = p[1];
        if (dwRet < g_dwExeLo || dwRet >= g_dwExeHi) break;
        pFrames[n++] = dwRet;
        if (p[0] <= dwFrame) break;
        dwFrame = p[0];
    }
    return n;
}

// ---------------------------------------------------------------------------
// The queue between the reporting threads and the worker. A spin lock, because
// it needs no initialisation: a check can fire from any constructor.
// ---------------------------------------------------------------------------
enum ReportKind { REPORT_BOUNDS, REPORT_BOUNDS_ABORT, REPORT_STACK_SMASH, REPORT_GUARD, REPORT_GUARD_LAYOUT };

struct Report {
    ReportKind eKind;
    const UbsanOutOfBoundsData* pData;
    long long nIndex;
    bool bIndexSigned;
    bool bIndexKnown;
    unsigned nOrdinal;
    DWORD dwThread;
    SYSTEMTIME st;
    int nFrames;
    DWORD aFrames[MAX_FRAMES];
    // REPORT_GUARD: the entry, the walk that saw it, and the guard bytes as found.
    // REPORT_GUARD_LAYOUT: nGuard misplaced guards, the first at nIndex.
    int nGuard;
    unsigned nWalk;
    unsigned char abGuard[32];
};

// ---------------------------------------------------------------------------
// Guard zones between globals. The generator emits, after every data definition,
// a 32-byte guard and a {owner, size, guard, name} entry in .d2grd$m (the macros
// are in d2_platform.h). The linker sorts .d2grd$a < $m < $z, so the two markers
// below bracket every entry in the exe.
// ---------------------------------------------------------------------------
struct GuardEntry {
    const void* pOwner;
    unsigned nOwnerSize;
    const unsigned char* pGuard;
    const char* pszName;
};
const int GUARD_BYTES = 32;
const GuardEntry g_guardFirst __attribute__((used, section(".d2grd$a"))) = { 0, 0, 0, 0 };
const GuardEntry g_guardLast __attribute__((used, section(".d2grd$z"))) = { 0, 0, 0, 0 };

// The fill the macros use: 0xA5 0xD2 repeated, 0xD2A5D2A5 as a dword.
inline unsigned char GuardFill(int i) { return (i & 1) ? 0xD2 : 0xA5; }

const GuardEntry* g_pGuards = 0;      // the entries, in link order
int g_nGuards = 0;
int* g_pnByOwner = 0;                 // entry indexes sorted by owner address
int* g_pnRankOf = 0;                  // entry index -> position in g_pnByOwner
volatile LONG* g_pnGuardHit = 0;      // per entry: 1 once reported
volatile LONG g_nGuardsDamaged = 0;
int g_nGuardsMisplaced = 0;
volatile LONG g_nGuardInit = 0;       // 0 none, 1 running, 2 done

int GuardCount() { return g_nGuards; }

const GuardEntry& Guard(int i) { return g_pGuards[i]; }

// The datum that follows guard i in memory: the next owner by address.
const GuardEntry* NextOwner(int i) {
    int nRank = g_pnRankOf ? g_pnRankOf[i] : -1;
    if (nRank < 0 || nRank + 1 >= g_nGuards) return 0;
    return &g_pGuards[g_pnByOwner[nRank + 1]];
}

int FirstDamaged(const unsigned char* pBytes) {
    for (int i = 0; i < GUARD_BYTES; ++i)
        if (pBytes[i] != GuardFill(i)) return i;
    return -1;
}

// The one-line form, shared by the report and the trace:
//   D2GUARD <owner> +<offset into owner> bytes=<32 bytes> frame=<walk> next=<next global>
void FormatGuardHit(const Report& r, char* pszOut, int nCap) {
    const GuardEntry& e = Guard(r.nGuard);
    int nFirst = FirstDamaged(r.abGuard);
    char szBytes[GUARD_BYTES * 3 + 1];
    for (int i = 0; i < GUARD_BYTES; ++i) wsprintfA(szBytes + i * 3, "%02X ", r.abGuard[i]);
    szBytes[GUARD_BYTES * 3 - 1] = 0;
    const GuardEntry* pNext = NextOwner(r.nGuard);
    _snprintf(pszOut, nCap, "D2GUARD %s +%u bytes=%s frame=%u next=%s", e.pszName,
              (unsigned)((e.pGuard - (const unsigned char*)e.pOwner) + (nFirst < 0 ? 0 : nFirst)), szBytes, r.nWalk,
              pNext ? pNext->pszName : "(none)");
    pszOut[nCap - 1] = 0;
}

void WriteGuardDetail(const Report& r) {
    const GuardEntry& e = Guard(r.nGuard);
    int nDamaged = 0;
    for (int i = 0; i < GUARD_BYTES; ++i) nDamaged += r.abGuard[i] != GuardFill(i);
    int nFirst = FirstDamaged(r.abGuard);
    char szOwner[MAX_PATH + 40];
    ModuleOffset((DWORD)(UINT_PTR)e.pOwner, szOwner);
    Out("  GLOBAL OVERRUN: %d of the %d guard bytes after %s were changed, the first at "
        "+%u from the owner's start.\r\n", nDamaged, GUARD_BYTES, e.pszName,
        (unsigned)((e.pGuard - (const unsigned char*)e.pOwner) + (nFirst < 0 ? 0 : nFirst)));
    Out("  owner %s (%u bytes, declared size), guard at owner+%ld\r\n", szOwner, e.nOwnerSize,
        (long)(e.pGuard - (const unsigned char*)e.pOwner));
    const GuardEntry* pNext = NextOwner(r.nGuard);
    if (pNext)
        Out("  next global %s at owner+%ld (%u bytes): it holds whatever ran past the guard\r\n",
            pNext->pszName, (long)((const unsigned char*)pNext->pOwner - (const unsigned char*)e.pOwner),
            pNext->nOwnerSize);
    Out("  seen by guard walk %u; the walk before it found the guard intact. The owner is most\r\n"
        "  likely typed too small in Ghidra, or its writer runs past it.\r\n", r.nWalk);
}

void WriteGuardLayout(const Report& r) {
    const GuardEntry& e = Guard((int)r.nIndex);
    Out("  GUARD LAYOUT: %d of %d guarded globals have no guard within 16 bytes after them,\r\n"
        "  so an overrun of those is not seen. First: %s (owner %p, %u bytes, "
        "guard at owner%+ld)\r\n", r.nGuard, g_nGuards, e.pszName, e.pOwner, e.nOwnerSize,
        (long)(e.pGuard - (const unsigned char*)e.pOwner));
}

const int QUEUE_SIZE = 256;
Report g_aQueue[QUEUE_SIZE];
volatile LONG g_nQueueHead = 0;   // next to write; only under the lock
volatile LONG g_nQueueTail = 0;   // next to read; only under the lock
volatile LONG g_nQueueLock = 0;
volatile LONG g_nDropped = 0;
volatile LONG g_nWritten = 0;     // reports the worker has finished writing
volatile LONG g_nQueued = 0;      // reports accepted into the queue
HANDLE g_hWake = 0;
HANDLE g_hWorker = 0;
volatile LONG g_nWorkerState = 0; // 0 none, 1 starting, 2 running

void Lock() {
    while (InterlockedCompareExchange(&g_nQueueLock, 1, 0) != 0) Sleep(0);
}
void Unlock() { InterlockedExchange(&g_nQueueLock, 0); }

// ---------------------------------------------------------------------------
// The once-per-site table: open addressing on the check's own data pointer,
// which GCC emits once per check site. Insert is one compare-exchange, so two
// threads hitting a new site at once still report it once.
// ---------------------------------------------------------------------------
const int SITE_SLOTS = 1 << 15;
const void* volatile g_apSite[SITE_SLOTS];
volatile LONG g_anSiteHits[SITE_SLOTS];
volatile LONG g_nSites = 0;
volatile LONG g_nSitesLost = 0;

// True the first time this site is seen.
bool FirstHit(const void* pKey) {
    UINT_PTR h = (UINT_PTR)pKey;
    h ^= h >> 15;
    h *= 0x2C1B3C6DU;
    h ^= h >> 12;
    for (int nProbe = 0; nProbe < SITE_SLOTS; ++nProbe) {
        int i = (int)((h + nProbe) & (SITE_SLOTS - 1));
        const void* pSeen = g_apSite[i];
        if (pSeen == pKey) {
            InterlockedIncrement(&g_anSiteHits[i]);
            return false;
        }
        if (!pSeen) {
            PVOID pOld = InterlockedCompareExchangePointer((PVOID volatile*)&g_apSite[i],
                                                           (PVOID)pKey, 0);
            if (!pOld || pOld == pKey) {
                InterlockedIncrement(&g_anSiteHits[i]);
                if (!pOld) {
                    InterlockedIncrement(&g_nSites);
                    return true;
                }
                return false;
            }
        }
    }
    // Table full: report nothing more rather than everything again.
    InterlockedIncrement(&g_nSitesLost);
    return false;
}

void WriteHeader() {
    Out("Redivivus loud build report\r\n");
    Out("Every entry below is a defect at the location it names. Send this whole file.\r\n\r\n");
    Out("Build:     %s (loud), gcc %s\r\n", REDIVIVUS_BUILD_ID, __VERSION__);
    Out("Exe:       %s\r\n", g_szExePath);
#ifdef REDIVIVUS_LOUD_NOSP
    Out("Checks:    -fsanitize=bounds-strict (logged once per site, game continues), "
        "-ftrivial-auto-var-init=pattern (0xFE); no stack protector\r\n\r\n");
#else
    Out("Checks:    -fsanitize=bounds-strict (logged once per site, game continues), "
        "-fstack-protector-strong (fatal), -ftrivial-auto-var-init=pattern (0xFE)\r\n\r\n");
#endif
    Out("Guards:    %d guard zones of 32 bytes after the globals, walked every 40 ms of the "
        "message loop (frame = walk number) and at exit\r\n\r\n", GuardCount());
    FlushFileBuffers(g_hFile);
}

void WriteIndex(const Report& r) {
    if (!r.bIndexKnown)
        Out("?");
    else if (r.bIndexSigned)
        Out("%I64d", r.nIndex);
    else
        Out("%I64u", (unsigned long long)r.nIndex);
}

void WriteReport(const Report& r) {
    Out("[%u] %04u-%02u-%02u %02u:%02u:%02u  thread %lu\r\n", r.nOrdinal, r.st.wYear,
        r.st.wMonth, r.st.wDay, r.st.wHour, r.st.wMinute, r.st.wSecond,
        (unsigned long)r.dwThread);
    if (r.eKind == REPORT_GUARD) {
        char szLine[512];
        FormatGuardHit(r, szLine, sizeof(szLine));
        Out("  %s\r\n", szLine);
        WriteGuardDetail(r);
        Out("\r\n");
        return;
    }
    if (r.eKind == REPORT_GUARD_LAYOUT) {
        WriteGuardLayout(r);
        Out("\r\n");
        return;
    }
    if (r.eKind == REPORT_STACK_SMASH) {
        Out("  STACK SMASHED: a write ran past a local array and over the frame's canary.\r\n");
        Out("  The function that returns through frame #0 below owns the smashed frame.\r\n");
        Out("  The crash report (d2crash-*.txt, HALT line=0) follows; the game ends here.\r\n");
    } else {
        const UbsanOutOfBoundsData* d = r.pData;
        Out("  OUT OF BOUNDS%s: index ", r.eKind == REPORT_BOUNDS_ABORT ? " (abort)" : "");
        WriteIndex(r);
        Out(" into %s (index type %s)\r\n", d->pArrayType->szName, d->pIndexType->szName);
        Out("  at %s:%u:%u\r\n", d->loc.pszFile ? d->loc.pszFile : "?", d->loc.nLine,
            d->loc.nColumn);
    }
    for (int i = 0; i < r.nFrames; ++i) WriteFrame(i, r.aFrames[i]);
    Out("\r\n");
}

DWORD WINAPI WorkerThread(LPVOID) {
    for (;;) {
        WaitForSingleObject(g_hWake, 1000);
        for (;;) {
            Report r;
            Lock();
            bool bAny = g_nQueueTail != g_nQueueHead;
            if (bAny) {
                r = g_aQueue[g_nQueueTail % QUEUE_SIZE];
                g_nQueueTail = g_nQueueTail + 1;
            }
            Unlock();
            if (!bAny) break;
            if (OpenLog()) {
                if (r.nOrdinal == 1) WriteHeader();
                WriteReport(r);
                FlushFileBuffers(g_hFile);
            }
            InterlockedIncrement(&g_nWritten);
        }
    }
    return 0;
}

void EnsureWorker() {
    if (g_nWorkerState == 2) return;
    if (InterlockedCompareExchange(&g_nWorkerState, 1, 0) == 0) {
        g_hWake = CreateEventA(0, FALSE, FALSE, 0);
        g_hWorker = CreateThread(0, 4 << 20, WorkerThread, 0, 0, 0);
        InterlockedExchange(&g_nWorkerState, 2);
    }
    while (g_nWorkerState != 2) Sleep(0);
}

volatile LONG g_nOrdinal = 0;

unsigned Push(Report& r);

// Queue one report. Returns its ordinal, or 0 when the queue was full.
unsigned Enqueue(ReportKind eKind, const UbsanOutOfBoundsData* pData, UbsanValueHandle hIndex,
                 DWORD dwSite, DWORD dwEbp) {
    InitExe();
    Report r;
    memset(&r, 0, sizeof(r));
    r.eKind = eKind;
    r.pData = pData;
    if (pData && pData->pIndexType && pData->pIndexType->nKind == 0) {
        unsigned nBits = 1u << (pData->pIndexType->nInfo >> 1);
        r.bIndexSigned = pData->pIndexType->nInfo & 1;
        r.bIndexKnown = true;
        if (nBits > sizeof(UbsanValueHandle) * 8) {
            r.nIndex = *(const long long*)(UINT_PTR)hIndex;
        } else if (r.bIndexSigned) {
            int nShift = 64 - (int)nBits;
            r.nIndex = ((long long)(unsigned long long)hIndex << nShift) >> nShift;
        } else {
            r.nIndex = (long long)(unsigned long long)hIndex;
        }
    }
    r.dwThread = GetCurrentThreadId();
    GetLocalTime(&r.st);
    r.nFrames = WalkStack(dwSite, dwEbp, r.aFrames);
    return Push(r);
}

// Queue a filled-in report. Returns its ordinal, or 0 when the queue was full.
unsigned Push(Report& r) {
    EnsureWorker();
    unsigned nOrdinal = 0;
    Lock();
    if (g_nQueueHead - g_nQueueTail < QUEUE_SIZE) {
        nOrdinal = (unsigned)InterlockedIncrement(&g_nOrdinal);
        r.nOrdinal = nOrdinal;
        g_aQueue[g_nQueueHead % QUEUE_SIZE] = r;
        g_nQueueHead = g_nQueueHead + 1;
        InterlockedIncrement(&g_nQueued);
    } else {
        InterlockedIncrement(&g_nDropped);
    }
    Unlock();
    if (g_hWake) SetEvent(g_hWake);
    return nOrdinal;
}

// Wait, bounded, until the worker has written everything queued so far.
void Drain(DWORD dwTimeoutMs) {
    DWORD dwStart = GetTickCount();
    while (g_nWritten < g_nQueued && GetTickCount() - dwStart < dwTimeoutMs) {
        if (g_hWake) SetEvent(g_hWake);
        Sleep(10);
    }
}

__attribute__((noreturn)) void FatalHandOff(DWORD dwSite) {
    Drain(60000);
    if (g_hFile != INVALID_HANDLE_VALUE) FlushFileBuffers(g_hFile);
    D2CRASH_ReportHalt(0, (unsigned)dwSite);
    TerminateProcess(GetCurrentProcess(), 0xC0000409);   // STATUS_STACK_BUFFER_OVERRUN
    for (;;) Sleep(INFINITE);
}

// ---------------------------------------------------------------------------
// The guard walk.
// ---------------------------------------------------------------------------
bool EnvSet(const char* pszName, char* pszValue, DWORD nCap) {
    DWORD n = GetEnvironmentVariableA(pszName, pszValue, nCap);
    return n > 0 && n < nCap;
}

bool g_bGuardHalt = false;
char g_szGuardWatch[256];

// Collect the entries, fill the zero-initialised guards (the .bss ones), order the
// owners by address for the "next global" column, and check that every guard sits
// directly after its owner. Runs once, on the first walk.
void GuardInit() {
    if (g_nGuardInit == 2) return;
    if (InterlockedCompareExchange(&g_nGuardInit, 1, 0) != 0) {
        while (g_nGuardInit != 2) Sleep(0);
        return;
    }
    InitExe();
    char szValue[16];
    g_bGuardHalt = EnvSet("D2_GUARD_HALT", szValue, sizeof(szValue)) && szValue[0] == '1';
    if (!EnvSet("D2_GUARD_WATCH", g_szGuardWatch, sizeof(g_szGuardWatch))) g_szGuardWatch[0] = 0;

    // Pointer arithmetic between two distinct objects, done on integers on purpose.
    UINT_PTR uFirst = (UINT_PTR)&g_guardFirst + sizeof(GuardEntry);
    UINT_PTR uLast = (UINT_PTR)&g_guardLast;
    int nAll = uLast > uFirst ? (int)((uLast - uFirst) / sizeof(GuardEntry)) : 0;
    const GuardEntry* pAll = (const GuardEntry*)uFirst;
    HANDLE hHeap = GetProcessHeap();
    GuardEntry* pCopy = nAll ? (GuardEntry*)HeapAlloc(hHeap, 0, nAll * sizeof(GuardEntry)) : 0;
    int n = 0;
    for (int i = 0; pCopy && i < nAll; ++i)
        if (pAll[i].pGuard && pAll[i].pOwner) pCopy[n++] = pAll[i];
    g_pGuards = pCopy;
    g_nGuards = n;
    if (n) {
        g_pnByOwner = (int*)HeapAlloc(hHeap, 0, n * sizeof(int));
        g_pnRankOf = (int*)HeapAlloc(hHeap, 0, n * sizeof(int));
        g_pnGuardHit = (volatile LONG*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, n * sizeof(LONG));
    }
    if (!g_pnByOwner || !g_pnRankOf || !g_pnGuardHit) {
        g_nGuards = 0;
        InterlockedExchange(&g_nGuardInit, 2);
        return;
    }

    // A .bss guard starts as zeroes; give it the fill. The others already hold it.
    for (int i = 0; i < n; ++i) {
        unsigned char* p = (unsigned char*)pCopy[i].pGuard;
        bool bZero = true;
        for (int b = 0; b < GUARD_BYTES && bZero; ++b) bZero = p[b] == 0;
        if (bZero)
            for (int b = 0; b < GUARD_BYTES; ++b) p[b] = GuardFill(b);
        g_pnByOwner[i] = i;
    }
    // Order by owner, and for one owner by how far its guard sits past the owner's end
    // (a guard before the end counts as infinitely far). An initialised owner carries
    // two guards, one in .data and one zero-filled, because a non-constant initializer
    // puts the owner in .bss; the nearer one is the one that guards it.
    struct Key {
        static UINT_PTR Gap(const GuardEntry& e) {
            UINT_PTR uEnd = (UINT_PTR)e.pOwner + e.nOwnerSize;
            return (UINT_PTR)e.pGuard >= uEnd ? (UINT_PTR)e.pGuard - uEnd : ~(UINT_PTR)0;
        }
        static bool Less(const GuardEntry& a, const GuardEntry& b) {
            if (a.pOwner != b.pOwner) return (UINT_PTR)a.pOwner < (UINT_PTR)b.pOwner;
            return Gap(a) < Gap(b);
        }
    };
    for (int nGap = n / 2; nGap > 0; nGap /= 2)    // shell sort: a few thousand, once
        for (int i = nGap; i < n; ++i) {
            int v = g_pnByOwner[i];
            int j = i;
            for (; j >= nGap && Key::Less(pCopy[v], pCopy[g_pnByOwner[j - nGap]]); j -= nGap)
                g_pnByOwner[j] = g_pnByOwner[j - nGap];
            g_pnByOwner[j] = v;
        }
    GuardEntry* pKept = (GuardEntry*)HeapAlloc(hHeap, 0, n * sizeof(GuardEntry));
    int nKept = 0;
    for (int i = 0; pKept && i < n; ++i) {
        const GuardEntry& e = pCopy[g_pnByOwner[i]];
        if (nKept && pKept[nKept - 1].pOwner == e.pOwner) continue;
        pKept[nKept++] = e;
    }
    if (pKept) {
        g_pGuards = pKept;
        g_nGuards = n = nKept;
    }
    int nMisplaced = 0, nFirstMisplaced = -1, nNear = 0;
    for (int i = 0; i < n; ++i) {
        g_pnByOwner[i] = i;
        g_pnRankOf[i] = i;
        UINT_PTR uGap = Key::Gap(g_pGuards[i]);
        if (uGap == 0) continue;
        // A function-local static with a first-call initializer has its 8-byte init
        // flag between it and the guard: covered, with a gap.
        if (uGap <= 16) { ++nNear; continue; }
        if (nFirstMisplaced < 0) nFirstMisplaced = i;
        ++nMisplaced;
    }
    g_nGuardsMisplaced = nMisplaced;

    D2TraceFmt("D2GUARD armed: %d guarded globals, %d guard(s) up to 16 bytes past the owner, "
               "%d not after their owner%s%s", n, nNear, nMisplaced,
               g_bGuardHalt ? ", halt on first hit" : "",
               g_szGuardWatch[0] ? ", write watch requested" : "");
    if (nMisplaced) {
        Report r;
        memset(&r, 0, sizeof(r));
        r.eKind = REPORT_GUARD_LAYOUT;
        r.nGuard = nMisplaced;
        r.nIndex = nFirstMisplaced;
        r.dwThread = GetCurrentThreadId();
        GetLocalTime(&r.st);
        Push(r);
    }
    InterlockedExchange(&g_nGuardInit, 2);
}

volatile LONG g_nGuardWalks = 0;
volatile LONG g_nGuardWalkLock = 0;

// One walk over every guard. Each damaged guard is reported once.
void GuardWalk() {
    GuardInit();
    if (!g_nGuards) return;
    if (InterlockedCompareExchange(&g_nGuardWalkLock, 1, 0) != 0) return;
    unsigned nWalk = (unsigned)InterlockedIncrement(&g_nGuardWalks);
    bool bHalt = false;
    for (int i = 0; i < g_nGuards; ++i) {
        const unsigned char* p = g_pGuards[i].pGuard;
        if (FirstDamaged(p) < 0 || g_pnGuardHit[i]) continue;
        if (InterlockedExchange(&g_pnGuardHit[i], 1)) continue;
        InterlockedIncrement(&g_nGuardsDamaged);
        Report r;
        memset(&r, 0, sizeof(r));
        r.eKind = REPORT_GUARD;
        r.nGuard = i;
        r.nWalk = nWalk;
        memcpy(r.abGuard, p, GUARD_BYTES);
        r.dwThread = GetCurrentThreadId();
        GetLocalTime(&r.st);
        char szLine[512];
        FormatGuardHit(r, szLine, sizeof(szLine));
        D2Trace(szLine);
        Push(r);
        bHalt = bHalt || g_bGuardHalt;
    }
    InterlockedExchange(&g_nGuardWalkLock, 0);
    if (bHalt) FatalHandOff((DWORD)(UINT_PTR)__builtin_return_address(0));
}

// D2_GUARD_WATCH: a hardware write watch on one owner's guard. Debug registers are
// per thread, so it is armed from the first pump, on the game thread.
void GuardArmWatch() {
    if (!g_szGuardWatch[0]) return;
    for (int i = 0; i < g_nGuards; ++i) {
        const char* pszName = g_pGuards[i].pszName;
        const char* pszTail = pszName;
        for (const char* q = pszName; *q; ++q)
            if (q[0] == ':' && q[1] == ':') pszTail = q + 2;
        if (lstrcmpA(pszName, g_szGuardWatch) && lstrcmpA(pszTail, g_szGuardWatch)) continue;
        UINT_PTR u = ((UINT_PTR)g_pGuards[i].pGuard + 3) & ~(UINT_PTR)3;
        D2TraceFmt("D2GUARD watch on the guard of %s at %p", pszName, (void*)u);
        D2TRACE_WatchWrite((void*)u);
        return;
    }
    D2TraceFmt("D2GUARD watch: no guarded global named %s", g_szGuardWatch);
}

// At a normal exit: how often each reported site fired in all.
// Best effort - a process ended by ExitProcess never gets here.
struct LoudSummary {
    ~LoudSummary() {
        // A last walk: an overrun after the final frame is still reported.
        if (g_nGuardInit == 2) GuardWalk();
        if (!g_nQueued) return;
        Drain(5000);
        if (g_nQueueLock || g_hFile == INVALID_HANDLE_VALUE) return;
        Out("Summary: %ld site(s) reported, %ld report(s) dropped (queue full), "
            "%ld site(s) not tracked (table full)\r\n",
            (long)g_nSites, (long)g_nDropped, (long)g_nSitesLost);
        Out("Guards: %ld of %d damaged over %ld walk(s), %d misplaced\r\n",
            (long)g_nGuardsDamaged, g_nGuards, (long)g_nGuardWalks, g_nGuardsMisplaced);
        for (int i = 0; i < SITE_SLOTS; ++i) {
            const UbsanOutOfBoundsData* d = (const UbsanOutOfBoundsData*)g_apSite[i];
            if (!d) continue;
            Out("  %8ld hits  %s:%u:%u\r\n", (long)g_anSiteHits[i],
                d->loc.pszFile ? d->loc.pszFile : "?", d->loc.nLine, d->loc.nColumn);
        }
        FlushFileBuffers(g_hFile);
    }
};
LoudSummary g_loudSummary __attribute__((init_priority(101)));

}  // namespace

// ---------------------------------------------------------------------------
// The entry points the instrumented code calls.
// ---------------------------------------------------------------------------
extern "C" __attribute__((noinline)) void __ubsan_handle_out_of_bounds(UbsanOutOfBoundsData* pData,
                                                                       UbsanValueHandle hIndex) {
    if (!FirstHit(pData)) return;
    Enqueue(REPORT_BOUNDS, pData, hIndex, (DWORD)(UINT_PTR)__builtin_return_address(0),
            (DWORD)(UINT_PTR)__builtin_frame_address(1));
}

// -fno-sanitize-recover: the same report, then the crash report and the end.
extern "C" __attribute__((noinline, noreturn)) void __ubsan_handle_out_of_bounds_abort(
    UbsanOutOfBoundsData* pData, UbsanValueHandle hIndex) {
    DWORD dwSite = (DWORD)(UINT_PTR)__builtin_return_address(0);
    FirstHit(pData);
    Enqueue(REPORT_BOUNDS_ABORT, pData, hIndex, dwSite,
            (DWORD)(UINT_PTR)__builtin_frame_address(1));
    FatalHandOff(dwSite);
}

// Called from the epilogue of a function whose canary no longer matches. The
// return address is inside that function; its EBP is still the live one, but the
// saved EBP and return address above it may be the overwritten bytes, which the
// walk's bounds checks stop on.
extern "C" __attribute__((noinline, noreturn)) void __stack_chk_fail(void) {
    DWORD dwSite = (DWORD)(UINT_PTR)__builtin_return_address(0);
    Enqueue(REPORT_STACK_SMASH, 0, 0, dwSite, (DWORD)(UINT_PTR)__builtin_frame_address(1));
    FatalHandOff(dwSite);
}

// Called from the game's two message loops (D2Win MessagePump, D2Client
// MessageGameLoop) on the game thread. Walks the guards at most once per 40 ms.
extern "C" void D2LOUD_GuardPump(void) {
    static DWORD s_dwLast = 0;
    static bool s_bWatchArmed = false;
    DWORD dwNow = GetTickCount();
    if (s_dwLast && dwNow - s_dwLast < 40) return;
    s_dwLast = dwNow ? dwNow : 1;
    GuardWalk();
    if (!s_bWatchArmed) {
        s_bWatchArmed = true;
        GuardArmWatch();
    }
}
