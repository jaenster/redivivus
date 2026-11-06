// Startup tracing. The binary is -mwindows, so there is no console and printf
// goes nowhere. This writes to both OutputDebugStringA (visible under
// WINEDEBUG=+debugstr) and an unbuffered file, so a hang still leaves a trail.
#pragma once

extern "C" void D2Trace(const char* pszWhere);

// Same, with arguments. Deliberately wsprintfA rather than snprintf: it is in
// user32, needs no CRT state, and so still works while the error handler is
// unwinding. Only the %-specifiers wsprintfA supports are safe here.
extern "C" void D2TraceFmt(const char* pszFormat, ...);

// A release build traces nothing (D2Trace returns at once), and it does not evaluate
// the arguments either: a trace line is free to read state that is only safe to read
// in a development build. DRLGACTMISC_AllocDrlgLevel's LVL-ALLOC line takes
// __builtin_return_address(1) and (2), which walk the EBP chain; off a frame without one
// that faults, and as a call the release build evaluated it on every level allocation.
// The definitions in d2_trace.cpp spell the names in parentheses, which no
// function-like macro expands.
#if !defined(REDIVIVUS_DEV) && !defined(D2_TRACE_DEFINITIONS)
#define D2Trace(...) ((void)0)
#define D2TraceFmt(...) ((void)0)
#endif

// Write a crash report for a Fog halt (support/d2_crash.cpp): the line, the
// call-site address and the halting thread's stack. Declared here because the
// generated ErrorManager.cpp already includes this header.
extern "C" void D2CRASH_ReportHalt(int nLine, unsigned nAddress);
extern "C" int D2ProbeReadTileClipYMin(void);
extern "C" void D2ProbeArmClipGuard(void);
extern "C" void D2ProbeArmClipHwbpSelf(void);
extern "C" void* D2ProbeWrapThreadStart(void* pfnStart, void* pArg, void** ppArgOut);
extern "C" void D2ProbeCheckBuffer(const char* pszWho, const void* pBuf, unsigned nLen, void* pRet);
extern "C" void D2ProbeArmClipHwbp(void);

#define D2TRACE(where) D2Trace(where)

extern "C" void D2TRACE_WatchWrite(void* pAddress);

extern unsigned g_anClientPktCount[256];
extern unsigned g_anServerPktCount[256];
