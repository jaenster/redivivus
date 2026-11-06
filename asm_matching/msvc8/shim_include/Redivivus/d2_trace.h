// The oracle's stand-in for Redivivus/d2_trace.h, found first on the include path.
//
// The tree's tracing and probe calls are debugging aids added to the
// reconstruction; the original has none, so for the byte comparison they
// compile to nothing. Arguments are not evaluated - every call site passes
// plain reads. The packet counters stay declared: their increments are
// statements, not calls, and remain visible to the audit as non-original code.
#pragma once
#define D2Trace(where) ((void)0)
#define D2TRACE(where) ((void)0)
#define D2TraceFmt(...) ((void)0)
#define D2CRASH_ReportHalt(nLine, nAddress) ((void)0)   /* the crash report a Fog halt writes */
#define D2TRACE_WatchWrite(p) ((void)0)
#define D2ProbeArmClipGuard() ((void)0)
#define D2ProbeArmClipHwbp() ((void)0)
#define D2ProbeArmClipHwbpSelf() ((void)0)
#define D2ProbeCheckBuffer(who, buf, len, ret) ((void)0)
extern "C" int D2ProbeReadTileClipYMin(void);
extern "C" void* D2ProbeWrapThreadStart(void* pfnStart, void* pArg, void** ppArgOut);
extern unsigned g_anClientPktCount[256];
extern unsigned g_anServerPktCount[256];
