// Win32Posix: what a POSIX process needs around the Win32 surface - the shell calls (opening a
// URL or a file), the process entry that stands where WinMain does, the crash path that stands
// where SEH and the crash dump do, and the taskbar. Storm's Mac layer has the same pieces in
// Win32MacUser.h (STORM_LaunchURL) and MacShell (main, the Mach exception server). Read through
// Win32Posix.h.
#pragma once

#if !defined(_WIN32) && D2_BACKEND_POSIX

// ---- the shell ----
HINSTANCE ShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory,
                        INT nShowCmd);
// Opens a URL in the user's browser, as Storm's Mac layer does (Win32MacUser.h): nonzero when
// the browser took it.
int STORM_LaunchURL(char* szURL);

// ---- the crash path ----
// The fatal signals (SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT) call pfnCrash once, on an
// alternate stack, with the signal number and the faulting address: where SEH's exception
// filter and the Mac's Mach exception server stand. Fog's error manager installs it
// (ERRORMANAGER_InitErrorManager).
typedef void (*POSIX_CRASHPROC)(int nSignal, void* pFaultAddress);
void POSIX_SetCrashHandler(POSIX_CRASHPROC pfnCrash);

// The process entry is Game/Main.cpp's main(), which hands the command line to
// PreInitApplication as WinMain does. The taskbar has no POSIX counterpart: D2gfx keeps
// empty bodies for it (D2GFX.cpp).

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
