// Win32Posix: the shell calls (ShellExecuteA, SHGetFolderPathA, STORM_LaunchURL) and the crash
// path (the fatal-signal handler, the backtrace writer, the halt report) - Win32PosixShell.h.
// Storm's Mac layer has the same pieces in Win32MacUser.h (STORM_LaunchURL) and MacShell (the
// Mach exception server).
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <SDL3/SDL_misc.h>
#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <execinfo.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/ucontext.h>
#include <sys/wait.h>
#include <unistd.h>
#include <thread>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

extern char** environ;

using namespace Win32Posix;

namespace {

constexpr intptr_t SE_ERR_FNF_VALUE = 2;
constexpr intptr_t SE_ERR_NOASSOC_VALUE = 31;
constexpr intptr_t SHELL_SUCCESS_VALUE = 33;  // ShellExecute's success is any value above 32

bool IsUrl(const char* sz) {
    const char* szColon = strchr(sz, ':');
    if (!szColon || szColon == sz) {
        return false;
    }
    for (const char* p = sz; p < szColon; ++p) {
        if (!(isalnum((unsigned char)*p) || *p == '+' || *p == '-' || *p == '.')) {
            return false;
        }
    }
    // "C:\..." is a drive, not a scheme.
    return szColon - sz > 1;
}

// Starts argv[0] in sDir (the current directory when empty) and reaps it on a thread of its
// own, so it never lingers as a zombie: ShellExecute does not wait.
bool Spawn(const std::vector<std::string>& aArgs, const std::string& sDir) {
    std::vector<char*> aArgv;
    for (const std::string& s : aArgs) {
        aArgv.push_back(const_cast<char*>(s.c_str()));
    }
    aArgv.push_back(nullptr);
    posix_spawn_file_actions_t tActions;
    posix_spawn_file_actions_init(&tActions);
    if (!sDir.empty()) {
        posix_spawn_file_actions_addchdir_np(&tActions, sDir.c_str());
    }
    pid_t nPid = 0;
    int nError = posix_spawnp(&nPid, aArgv[0], &tActions, nullptr, aArgv.data(), environ);
    posix_spawn_file_actions_destroy(&tActions);
    if (nError != 0) {
        return false;
    }
    std::thread([nPid] {
        int nStatus = 0;
        while (waitpid(nPid, &nStatus, 0) < 0 && errno == EINTR) {
        }
    }).detach();
    return true;
}

}  // namespace

// ---- the shell ----

HINSTANCE ShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory,
                        INT nShowCmd) {
    (void)hwnd;
    (void)nShowCmd;
    if (lpOperation && *lpOperation && strcasecmp(lpOperation, "open") != 0) {
        return (HINSTANCE)SE_ERR_NOASSOC_VALUE;
    }
    if (!lpFile || !*lpFile) {
        return (HINSTANCE)SE_ERR_FNF_VALUE;
    }
    if (IsUrl(lpFile)) {
        return (HINSTANCE)(SDL_OpenURL(lpFile) ? SHELL_SUCCESS_VALUE : SE_ERR_NOASSOC_VALUE);
    }
    std::string sDir = lpDirectory && *lpDirectory ? Path_ToPosix(lpDirectory) : std::string();
    std::string sPath = Path_ToPosix(lpFile);
    struct stat tStat;
    if (stat(sPath.c_str(), &tStat) != 0) {
        return (HINSTANCE)SE_ERR_FNF_VALUE;
    }
    std::vector<std::string> aArgs;
    if (S_ISREG(tStat.st_mode) && access(sPath.c_str(), X_OK) == 0) {
        aArgs.push_back(sPath);
        for (const char* p = lpParameters ? lpParameters : ""; *p;) {
            while (*p == ' ') {
                ++p;
            }
            const char* pEnd = p;
            while (*pEnd && *pEnd != ' ') {
                ++pEnd;
            }
            if (pEnd != p) {
                aArgs.emplace_back(p, pEnd);
            }
            p = pEnd;
        }
    } else {
#if defined(__APPLE__)
        aArgs = {"/usr/bin/open", sPath};
#else
        aArgs = {"xdg-open", sPath};
#endif
    }
    return (HINSTANCE)(Spawn(aArgs, sDir) ? SHELL_SUCCESS_VALUE : SE_ERR_NOASSOC_VALUE);
}

int STORM_LaunchURL(char* szURL) {
    return szURL && *szURL && SDL_OpenURL(szURL);
}

HRESULT SHGetFolderPathA(HWND hwnd, int nFolder, HANDLE hToken, DWORD dwFlags, LPSTR pszPath) {
    (void)hwnd;
    (void)hToken;
    (void)dwFlags;
    constexpr HRESULT E_FAIL_VALUE = (HRESULT)0x80004005u;
    std::string sDir;
    switch (nFolder & 0xff) {  // the high bits are CSIDL_FLAG_CREATE and its kin
        case 0x1a:             // CSIDL_APPDATA
        case 0x1c:             // CSIDL_LOCAL_APPDATA
        case 0x23:             // CSIDL_COMMON_APPDATA
            sDir = AppSupportDir();
            break;
        case 0x05:  // CSIDL_PERSONAL
            sDir = HomeDir() + "/Documents";
            break;
        default:
            if (pszPath) {
                *pszPath = '\0';
            }
            return E_FAIL_VALUE;
    }
    mkdir(sDir.c_str(), 0755);
    std::string sWin = Path_ToWindows(sDir);
    if (!pszPath || sWin.size() >= MAX_PATH) {
        return E_FAIL_VALUE;
    }
    memcpy(pszPath, sWin.c_str(), sWin.size() + 1);
    return 0;
}

// ---- the crash path ----

namespace {

POSIX_CRASHPROC g_pfnCrash = nullptr;
std::atomic<int> g_nCrashing{0};
alignas(16) char g_aAltStack[256 * 1024];

void WriteToStderr(void* pContext, const char* szLine) {
    (void)pContext;
    fprintf(stderr, "  %s\n", szLine);
}

// The faulting instruction's address, from the signal's machine context.
void* FaultPc(void* pContext) {
#if defined(__APPLE__) && defined(__aarch64__)
    return pContext ? (void*)((ucontext_t*)pContext)->uc_mcontext->__ss.__pc : nullptr;
#elif defined(__APPLE__) && defined(__x86_64__)
    return pContext ? (void*)((ucontext_t*)pContext)->uc_mcontext->__ss.__rip : nullptr;
#else
    (void)pContext;
    return nullptr;
#endif
}

// The interrupted thread's registers and its frame-pointer chain, walked from the machine
// context: backtrace() on the alternate stack can stop at the signal frame, and a leaf that
// keeps no frame record is only visible through pc and lr. Each frame is printed as the module
// offset atos takes (`atos -o "Diablo II" -l <base>` with the base printed here).
void WriteFaultContext(void* pContext) {
#if defined(__APPLE__) && defined(__aarch64__)
    if (!pContext) {
        return;
    }
    const auto& tState = ((ucontext_t*)pContext)->uc_mcontext->__ss;
    for (int i = 0; i < 29; i += 4) {
        fprintf(stderr, "  x%-2d %016llx  x%-2d %016llx  x%-2d %016llx  x%-2d %016llx\n", i,
                (unsigned long long)tState.__x[i], i + 1, (unsigned long long)tState.__x[i + 1], i + 2,
                (unsigned long long)tState.__x[i + 2], i + 3, (unsigned long long)(i + 3 < 29 ? tState.__x[i + 3] : 0));
    }
    fprintf(stderr, "  fp %016llx  lr %016llx  sp %016llx\n", (unsigned long long)tState.__fp,
            (unsigned long long)tState.__lr, (unsigned long long)tState.__sp);
    Dl_info tInfo;
    if (dladdr((void*)tState.__pc, &tInfo) && tInfo.dli_fbase) {
        fprintf(stderr, "  module base %p\n", tInfo.dli_fbase);
    }
    auto WriteFrame = [](const char* szWhat, uintptr_t nAddress) {
        Dl_info tFrame;
        if (dladdr((void*)nAddress, &tFrame) && tFrame.dli_sname) {
            fprintf(stderr, "  %s %p %s + %ld\n", szWhat, (void*)nAddress, tFrame.dli_sname,
                    (long)(nAddress - (uintptr_t)tFrame.dli_saddr));
        } else {
            fprintf(stderr, "  %s %p\n", szWhat, (void*)nAddress);
        }
    };
    WriteFrame("lr", (uintptr_t)tState.__lr);
    uintptr_t* pFrame = (uintptr_t*)tState.__fp;
    for (int nDepth = 0; nDepth < 48 && pFrame && ((uintptr_t)pFrame & 7) == 0; ++nDepth) {
        uintptr_t nReturn = pFrame[1];
        if (!nReturn) {
            break;
        }
        WriteFrame("fp", nReturn);
        uintptr_t* pNext = (uintptr_t*)pFrame[0];
        if (pNext <= pFrame) {
            break;
        }
        pFrame = pNext;
    }
#else
    (void)pContext;
#endif
}

void OnFatalSignal(int nSignal, siginfo_t* pInfo, void* pContext) {
    // One report: a second fault (in the handler, or on another thread) takes the default
    // action, which SA_RESETHAND has put back.
    if (g_nCrashing.exchange(1) == 0) {
        // The signal, the address and the faulting instruction go to stderr first, with the
        // stack, so a report survives a game handler that faults or blocks in turn.
        fprintf(stderr, "[CRASH] signal %d, address %p, pc %p\n", nSignal, pInfo ? pInfo->si_addr : nullptr,
                FaultPc(pContext));
        POSIX_WriteBacktrace(WriteToStderr, nullptr, 0);
        WriteFaultContext(pContext);
        fflush(stderr);
        if (g_pfnCrash) {
            g_pfnCrash(nSignal, pInfo ? pInfo->si_addr : nullptr);
        }
    }
    signal(nSignal, SIG_DFL);
    raise(nSignal);
}

}  // namespace

float POSIX_GetSystemVersion(void) {
    char szVersion[32] = "";
    size_t nSize = sizeof(szVersion);
    if (sysctlbyname("kern.osproductversion", szVersion, &nSize, nullptr, 0) != 0) {
        return 0.0f;
    }
    int nMajor = 0, nMinor = 0;
    sscanf(szVersion, "%d.%d", &nMajor, &nMinor);
    return (float)nMajor + (float)nMinor / (nMinor >= 10 ? 100.0f : 10.0f);
}

void POSIX_SetCrashHandler(POSIX_CRASHPROC pfnCrash) {
    g_pfnCrash = pfnCrash;
    // The handler runs on an alternate stack, so a stack overflow on this thread is reported
    // too. Other threads fault on their own stacks.
    stack_t tStack;
    memset(&tStack, 0, sizeof(tStack));
    tStack.ss_sp = g_aAltStack;
    tStack.ss_size = sizeof(g_aAltStack);
    sigaltstack(&tStack, nullptr);
    struct sigaction tAction;
    memset(&tAction, 0, sizeof(tAction));
    tAction.sa_sigaction = OnFatalSignal;
    tAction.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_RESETHAND;
    sigemptyset(&tAction.sa_mask);
    for (int nSignal : {SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT}) {
        sigaction(nSignal, &tAction, nullptr);
    }
}

void POSIX_WriteBacktrace(POSIX_LINEPROC pfnLine, void* pContext, int nSkipFrames) {
    void* aFrames[64];
    int nFrames = backtrace(aFrames, (int)(sizeof(aFrames) / sizeof(aFrames[0])));
    char** pszSymbols = backtrace_symbols(aFrames, nFrames);
    // Frame 0 is this function.
    for (int i = 1 + (nSkipFrames > 0 ? nSkipFrames : 0); i < nFrames; ++i) {
        char szFallback[32];
        if (!pszSymbols) {
            snprintf(szFallback, sizeof(szFallback), "%p", aFrames[i]);
        }
        pfnLine(pContext, pszSymbols ? pszSymbols[i] : szFallback);
    }
    free(pszSymbols);
}

// The halt report Fog's ERROR_UnrecoverableInternalError_Halt asks for before it hands over to
// FOG_ErrorHandler (Redivivus/d2_trace.h; Redivivus/d2_crash.cpp on Windows): the line, the
// call site and the halting thread's stack, on stderr. Crash.txt follows from the error handler.
extern "C" void D2CRASH_ReportHalt(int nLine, unsigned nAddress) {
    fprintf(stderr, "[HALT] line %d, call site %08x\n", nLine, nAddress);
    POSIX_WriteBacktrace(WriteToStderr, nullptr, 1);
    fflush(stderr);
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
