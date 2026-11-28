// LINK STUB, not Blizzard's crashy.
//
// The 1.14d Mac binary carries crashy, Blizzard's crash reporter (3rdParty/crashy in Blizzard's
// tree, with contrib/mach_star/mach_override inside; no public copy exists). Reconstructing it
// is a separate task. Fog's ErrorManager references the symbols below; this file defines them
// with the signatures the references mangle to, so the Mac objects link. None of them reports
// anything: no report is opened, no backtrace is written, no crash callback is installed, and
// a debugger is never detected.
//
// Mac-only: compiled by cmake/mac.cmake (d2_mac_link_deps), never by the Windows build.
// The guard keeps these plain-call declarations out of the VC8 translation, which would
// otherwise read Report.cpp's __thiscall Open, Close, IsOpen and AddAttachment as __cdecl.
#if !defined(_WIN32)

struct Report;
struct D2BlizzardErrorArgument;

typedef void (*CrashyWriteLine)(Report*, char*);

extern "C" void D2CRASH_ReportHalt(int nLine, unsigned nAddress) {
    (void)nLine; (void)nAddress;
}

namespace Fog { namespace Src { namespace ErrorManager {
void CRASH_InitCrashReporter() {}
void CRASHY_Exit(int nExitCode) { (void)nExitCode; }
void CRASHY_WriteModules(CrashyWriteLine pfnWrite, Report* pReport) { (void)pfnWrite; (void)pReport; }
unsigned CRASHY_GetCurrentThread() { return 0; }
void CRASHY_SetCrashCallback(void* pCallback, void* pUserData) { (void)pCallback; (void)pUserData; }
void CRASHY_WriteFrameBacktrace(unsigned nFrames, void** ppFrames, unsigned nSkip, CrashyWriteLine pfnWrite,
                                Report* pReport) {
    (void)nFrames; (void)ppFrames; (void)nSkip; (void)pfnWrite; (void)pReport;
}
void CRASHY_WriteThreadBacktrace(unsigned nThreadId, void* pContext, CrashyWriteLine pfnWrite, Report* pReport,
                                 char bFlagA, char bFlagB, int nSkip) {
    (void)nThreadId; (void)pContext; (void)pfnWrite; (void)pReport; (void)bFlagA; (void)bFlagB; (void)nSkip;
}
void CRASHY_WriteThreadRegisters(unsigned nThreadId, CrashyWriteLine pfnWrite, Report* pReport) {
    (void)nThreadId; (void)pfnWrite; (void)pReport;
}
void CRASHY_WriteOtherThreadBacktraces(CrashyWriteLine pfnWrite, Report* pReport, unsigned nSkipThreadId) {
    (void)pfnWrite; (void)pReport; (void)nSkipThreadId;
}
void ErrorReportingSetSupportPath(const char* szPath) { (void)szPath; }
} } } // namespace Fog::Src::ErrorManager

namespace Fog { namespace _3rdparty { namespace crashy {
void AddAttachment(Report* pReport, char* szPath, signed char bFlag) { (void)pReport; (void)szPath; (void)bFlag; }
unsigned isDebuggerPresent() { return 0; }
bool ErrorReportingLaunch(char* szText, D2BlizzardErrorArgument* pArg) { (void)szText; (void)pArg; return false; }
void SYSTEM_SetGlobalString(char* szText) { (void)szText; }
void thunk_InitializeDebugSubsystem() {}
bool Open(Report* pReport, char* szPath) { (void)pReport; (void)szPath; return false; }
void Close(Report* pReport) { (void)pReport; }
bool IsOpen(Report* pReport) { (void)pReport; return false; }
void Printf(Report* pReport, char* szFormat, ...) { (void)pReport; (void)szFormat; }
} } } // namespace Fog::_3rdparty::crashy

#endif  // !defined(_WIN32)
