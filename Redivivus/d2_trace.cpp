#include <windows.h>
#include <stdio.h>

#define D2_TRACE_DEFINITIONS
#include "Redivivus/d2_trace.h"
#include "Source/globals/globals.h"
#include "Source/D2gfx/D2gfx.globals.h"

static FILE* g_pLog = 0;

#ifndef REDIVIVUS_DEV
// Per-opcode packet counters. The control channel defines and reports them in a
// development build; without it they are counted and never read.
unsigned g_anClientPktCount[256];
unsigned g_anServerPktCount[256];
#endif

// Under the Visual C++ build Redivivus/d2_trace.h is the msvc8 oracle's stand-in,
// in which every tracing call compiles to nothing; only the counters above stay.
#if !(defined(_MSC_VER) && defined(D2Trace))

#ifdef REDIVIVUS_DEV
// Prefer the host share. Reading the log back out of the VM's %TEMP% costs a PowerShell
// round trip that times out entirely once the file passes a few hundred thousand lines,
// which is the normal size after a few minutes of per-frame tracing. Written to the share
// it lands on the host filesystem, where tailing and grepping it are ordinary local file
// operations. %TEMP% stays as the fallback for a run with no share mounted.
static const char* const TRACE_SHARE_DIR = "\\\\Mac\\code\\re\\diablo2\\d2-clean-114d\\test\\";
#endif

static FILE* TraceLog() {
    if (!g_pLog) {
        char szPath[MAX_PATH];
#ifndef REDIVIVUS_DEV
        // A release build logs here only for an armed write watchpoint; crash
        // reports are Redivivus/d2_crash.cpp.
        DWORD nDir = GetModuleFileNameA(0, szPath, MAX_PATH);
        while (nDir > 0 && szPath[nDir - 1] != '\\') nDir--;
        lstrcpyA(szPath + nDir, "d2trace.log");
        g_pLog = fopen(szPath, "a");
#else
        lstrcpyA(szPath, TRACE_SHARE_DIR);
        lstrcatA(szPath, "d2trace.log");
        g_pLog = fopen(szPath, "a");
        if (!g_pLog) {
            // C:\ is not writable by a normal user, so the fallback is %TEMP%.
            DWORD n = GetTempPathA(MAX_PATH, szPath);
            if (n == 0 || n >= MAX_PATH) lstrcpyA(szPath, ".\\");
            lstrcatA(szPath, "d2trace.log");
            g_pLog = fopen(szPath, "a");
        }
#endif
        if (g_pLog) setvbuf(g_pLog, 0, _IONBF, 0);
    }
    return g_pLog;
}

extern "C" void D2Trace(const char* pszWhere) {
#ifndef REDIVIVUS_DEV
    // Per-call tracing is a development aid; a release build logs crashes only.
    (void)pszWhere;
    return;
#endif
    char szLine[512];
    wsprintfA(szLine, "D2TRACE %lu %s\n", (unsigned long)GetCurrentThreadId(), pszWhere);
    OutputDebugStringA(szLine);
    FILE* p = TraceLog();
    if (p) { fputs(szLine, p); fflush(p); }
}


extern "C" void D2TraceFmt(const char* pszFormat, ...) {
    char szBody[768];
    va_list args;
    va_start(args, pszFormat);
    wvsprintfA(szBody, pszFormat, args);
    va_end(args);
    D2Trace(szBody);
}

// The interesting failure is an access violation, and the address it faults at
// says more than any amount of stepping. This records it before Windows tears
// the process down, along with the module-relative offset, which is what maps
// back to a function in the map file.

// ---------------------------------------------------------------------------
// Hardware write-watchpoint.
//
// A wild store into a global is the defect class this reconstruction produces
// most, and the one a backtrace cannot find: by the time the value is READ the
// writer is long gone. x86 debug registers catch the store itself. DR0 holds the
// address, DR7 arms it as a 4-byte write watch, and the trap arrives as
// EXCEPTION_SINGLE_STEP at the instruction AFTER the store - enough to name the
// function, since the handler already walks EBP.
//
// Debug registers are per-thread, so this is armed on the game thread.
// ---------------------------------------------------------------------------
static void* g_watchAddr = 0;

extern "C" void D2TRACE_WatchWrite(void* pAddress) {
    g_watchAddr = pAddress;
    CONTEXT ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    HANDLE hThread = GetCurrentThread();
    if (!GetThreadContext(hThread, &ctx)) return;
    ctx.Dr0 = (DWORD)(UINT_PTR)pAddress;
    // L0 arms DR0; RW0=01 (write) and LEN0=11 (four bytes) live at bits 16-19.
    ctx.Dr7 = (ctx.Dr7 & ~0xF0000UL) | 0xD0000UL | 1UL;
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    if (SetThreadContext(hThread, &ctx)) D2TraceFmt("WATCH armed on %p", pAddress);
    else D2TraceFmt("WATCH failed on %p err=%lu", pAddress, GetLastError());
}

static LONG CALLBACK TraceVectoredHandler(EXCEPTION_POINTERS* pExc) {
    DWORD dwCode = pExc->ExceptionRecord->ExceptionCode;
    // Ignore the non-fatal ones the loader and debugger raise routinely.
    if (dwCode == 0x40010006 /* DBG_PRINTEXCEPTION_C */ ||
        dwCode == 0x406D1388 /* thread-name */) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // The watchpoint trap: name the store and carry on. EIP is the instruction
    // after the write, so the value written is already in place to print.
    if (dwCode == EXCEPTION_SINGLE_STEP && g_watchAddr) {
        char szLine[256];
        wsprintfA(szLine, "D2WATCH %p -> %08lX from eip=%08lX rva=%08lX\n",
                  g_watchAddr, (unsigned long)*(DWORD*)g_watchAddr,
                  (unsigned long)pExc->ContextRecord->Eip,
                  (unsigned long)(pExc->ContextRecord->Eip
                                  - (DWORD)(UINT_PTR)GetModuleHandleA(0)));
        OutputDebugStringA(szLine);
        FILE* pLogW = TraceLog();
        if (pLogW) { fputs(szLine, pLogW); fflush(pLogW); }
        return EXCEPTION_CONTINUE_EXECUTION;
    }
#ifndef REDIVIVUS_DEV
    // A release build reports crashes through Redivivus/d2_crash.cpp only.
    return EXCEPTION_CONTINUE_SEARCH;
#endif
    void* pAddr = pExc->ExceptionRecord->ExceptionAddress;
    HMODULE hBase = GetModuleHandleA(0);
    char szLine[512];
    FILE* pLog = TraceLog();

    wsprintfA(szLine, "D2FAULT code=%08lX addr=%p base=%p rva=%08lX\n",
              (unsigned long)dwCode, pAddr, (void*)hBase,
              (unsigned long)((char*)pAddr - (char*)hBase));
    OutputDebugStringA(szLine);
    if (pLog) { fputs(szLine, pLog); fflush(pLog); }

    if (dwCode == EXCEPTION_ACCESS_VIOLATION) {
        wsprintfA(szLine, "D2FAULT access %s at %p\n",
                  pExc->ExceptionRecord->ExceptionInformation[0] ? "WRITE" : "READ",
                  (void*)pExc->ExceptionRecord->ExceptionInformation[1]);
        OutputDebugStringA(szLine);
        if (pLog) { fputs(szLine, pLog); fflush(pLog); }
    }

    // Nothing here is optimised, so EBP chains are intact and walking them by
    // hand gives a real stack without a debugger present. Each frame is logged
    // as a module-relative RVA, which addr2line resolves against the unstripped
    // build back on the host.
    CONTEXT* pCtx = pExc->ContextRecord;
    wsprintfA(szLine, "D2FAULT eip=%08lX ebp=%08lX esp=%08lX\n",
              (unsigned long)pCtx->Eip, (unsigned long)pCtx->Ebp,
              (unsigned long)pCtx->Esp);
    OutputDebugStringA(szLine);
    if (pLog) { fputs(szLine, pLog); fflush(pLog); }

    // A call through a null pointer leaves EIP and EBP at zero, so the chain below
    // has nothing to walk. The return address the CALL pushed is still at [ESP], and
    // the frames above it are still on the stack - scan upward and report every slot
    // that lands inside our image. That recovers the caller when nothing else can.
    // A jump to a wild address (a call through a bad pointer, or a RET into garbage)
    // leaves EBP pointing at a frame the chain cannot trust either, so scan then too.
    DWORD dwLo = (DWORD)(UINT_PTR)hBase;
    IMAGE_NT_HEADERS* pNt = (IMAGE_NT_HEADERS*)((char*)hBase + ((IMAGE_DOS_HEADER*)hBase)->e_lfanew);
    DWORD dwHi = dwLo + pNt->OptionalHeader.SizeOfImage;
    if (!pCtx->Eip || !pCtx->Ebp || pCtx->Eip < dwLo || pCtx->Eip >= dwHi) {
        DWORD* pStack = (DWORD*)pCtx->Esp;
        for (int i = 0, nFound = 0; i < 1024 && nFound < 32; ++i) {
            if (IsBadReadPtr(pStack + i, sizeof(DWORD))) break;
            DWORD dwVal = pStack[i];
            if (dwVal <= dwLo || dwVal >= dwHi) continue;
            wsprintfA(szLine, "D2SCAN +%03X ret=%08lX rva=%08lX\n", i * 4,
                      (unsigned long)dwVal, (unsigned long)(dwVal - dwLo));
            OutputDebugStringA(szLine);
            if (pLog) { fputs(szLine, pLog); fflush(pLog); }
            ++nFound;
        }
    }

    DWORD* pFrame = (DWORD*)pCtx->Ebp;
    for (int i = 0; i < 24 && pFrame; ++i) {
        if (IsBadReadPtr(pFrame, sizeof(DWORD) * 2)) break;
        DWORD dwRet = pFrame[1];
        if (!dwRet) break;
        wsprintfA(szLine, "D2FRAME %2d ret=%08lX rva=%08lX\n", i,
                  (unsigned long)dwRet, (unsigned long)(dwRet - (DWORD)(UINT_PTR)hBase));
        OutputDebugStringA(szLine);
        if (pLog) { fputs(szLine, pLog); fflush(pLog); }
        DWORD* pNext = (DWORD*)pFrame[0];
        if (pNext <= pFrame) break;   // chain must ascend, or it is garbage
        pFrame = pNext;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

// Installed by a static initializer so it is live before WinMain runs.
// Priority 150: ahead of the static-init runner at 200, so a fault inside one of
// the game's own constructors still lands in the log.
__attribute__((init_priority(150))) static struct TraceInit {
    TraceInit() {
        AddVectoredExceptionHandler(1, TraceVectoredHandler);
        // The stack guarantee that keeps this handler runnable on a stack overflow
        // is reserved by Redivivus/d2_crash.cpp.
        D2Trace("vectored handler installed");
    }
} g_traceInit;

// Reads the wall-tile clip global through its real linkage so the probe cannot be
// folded to a constant, letting any trace point report the live value.
extern "C" int D2ProbeReadTileClipYMin(void) {
    return *(volatile int*)&D2gfx::gnTileClipYMin;
}

// Reports any buffer handed to a file read whose range covers the wall-clip global. The read
// is filled by the kernel, so the store never trips a CPU watchpoint and only the destination
// range can reveal it.
extern "C" void D2ProbeCheckBuffer(const char* pszWho, const void* pBuf, unsigned nLen, void* pRet) {
    uintptr_t nTgt = (uintptr_t)&D2gfx::gnTileClipYMin;
    uintptr_t nLo = (uintptr_t)pBuf;
    if (!nLo) return;
    if (nTgt >= nLo && nTgt < nLo + (uintptr_t)nLen)
        D2TraceFmt("BUFHIT *** %s covers gnTileClipYMin *** buf=%08X len=%u ret=%08X",
                   pszWho, (unsigned)nLo, nLen, (unsigned)(uintptr_t)pRet);
}
#endif
