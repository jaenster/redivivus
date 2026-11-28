// Win32Posix: what a POSIX process needs around the Win32 surface - the shell calls (opening a
// URL or a file), the process entry that stands where WinMain does, the crash path that stands
// where SEH and the crash dump do, and the taskbar. Storm's Mac layer has the same pieces in
// Win32MacUser.h (STORM_LaunchURL) and MacShell (main, the Mach exception server). Read through
// Win32Posix.h.
#pragma once

#if !defined(_WIN32) && D2_BACKEND_POSIX

// ---- the shell ----
// Opens lpFile: a URL in the user's browser, an executable started with lpParameters (split at
// spaces) in lpDirectory, any other file with the desktop's handler for it (open on macOS,
// xdg-open elsewhere). As on Windows, a value above 32 is success; SE_ERR_FNF (2) when there
// is no such file, SE_ERR_NOASSOC (31) when it could not be started. The process is not
// waited for.
HINSTANCE ShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory,
                        INT nShowCmd);
// Opens a URL in the user's browser, as Storm's Mac layer does (Win32MacUser.h): nonzero when
// the browser took it.
int STORM_LaunchURL(char* szURL);
// The special folders, Windows-spelled without the trailing backslash: CSIDL_APPDATA (0x1a),
// CSIDL_LOCAL_APPDATA (0x1c) and CSIDL_COMMON_APPDATA (0x23) are the user's application data
// directory the kernel keeps the install directory in (~/Library/Application Support on
// macOS), CSIDL_PERSONAL (5) is ~/Documents. S_OK, or E_FAIL for any other folder; the
// directory is created when missing.
HRESULT SHGetFolderPathA(HWND hwnd, int nFolder, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);

// The system version as the Mac's Gestalt 'sysv' decimal reads (macOS 15.1 is 15.1f), or 0
// when the kernel does not say.
float POSIX_GetSystemVersion(void);

// ---- the crash path ----
// The fatal signals (SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT) call pfnCrash once, on an
// alternate stack, with the signal number and the faulting address: where SEH's exception
// filter and the Mac's Mach exception server stand. Fog's error manager installs it
// (ERRORMANAGER_InitErrorManager).
typedef void (*POSIX_CRASHPROC)(int nSignal, void* pFaultAddress);
void POSIX_SetCrashHandler(POSIX_CRASHPROC pfnCrash);
// The calling thread's stack, one line per frame (module, address, symbol + offset, without a
// line break), innermost first after nSkipFrames of the writer's callers.
typedef void (*POSIX_LINEPROC)(void* pContext, const char* szLine);
void POSIX_WriteBacktrace(POSIX_LINEPROC pfnLine, void* pContext, int nSkipFrames);

// The process entry is Game/Main.cpp's main(), which hands the command line to
// PreInitApplication as WinMain does. The taskbar has no POSIX counterpart: D2gfx keeps
// empty bodies for it (D2GFX.cpp).

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
