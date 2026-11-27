// The Win32 names the shared game code (D2Client, D2Game ...) calls, on the Mac: each is
// an entry point Storm's Win32-on-Mac layer implements under its own name (no A suffix),
// declared here with the shared code's types. Included by d2_platform.h; Mac-only, spelled
// `!defined(_WIN32) && D2_OS_MAC` as in Win32Mac.h.
#pragma once

#if !defined(_WIN32) && D2_OS_MAC

#include <stdio.h>

#ifndef LOWORD
#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#endif
#ifndef HIWORD
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#endif
#ifndef MAKELONG
#define MAKELONG(a, b) ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#endif
#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wParam) ((short)HIWORD(wParam))
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef WM_CLOSE
#define WM_CLOSE 0x0010
#endif
#ifndef WM_MOUSEMOVE
#define WM_MOUSEMOVE 0x0200
#endif
#ifndef MAKELPARAM
#define MAKELPARAM(l, h) ((LPARAM)MAKELONG(l, h))
#endif
#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) ((type*)((char*)(address) - (size_t)(&((type*)0)->field)))
#endif

typedef BOOL* LPBOOL;

// Rectangles and the keyboard (Win32_Rect.cpp, Win32_Keys.cpp); the message pump, PtInRect, GetWindowRect and
// SetCursorPos are declared in d2_platform.h.
BOOL CopyRect(RECT* pDest, RECT* pSrc);
BOOL IntersectRect(RECT* pDest, RECT* pRect1, RECT* pRect2);
short GetAsyncKeyState(BYTE byVirtKey);

// The user name, the string and time helpers, and the interlocked pair the layer implements
// on LONG; the shared code also calls them with `long`, a distinct type on the Mac.
BOOL GetUserName(char* szBuffer, DWORD* pnSize);
#define GetUserNameA GetUserName
char* _strlwr(char* szString);
LONG CompareFileTime(const FILETIME* lpFileTime1, const FILETIME* lpFileTime2);
LONG InterlockedIncrement(LPLONG pnValue);
LONG InterlockedDecrement(LPLONG pnValue);
static inline long InterlockedIncrement(volatile long* p) { return InterlockedIncrement((LPLONG)p); }
static inline long InterlockedDecrement(volatile long* p) { return InterlockedDecrement((LPLONG)p); }
#define wvsprintfA vsprintf

// CreateFile takes a handle-table index as its template file: the shared code's `nullptr` is 0.
HANDLE CreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPVOID lpSecurityAttributes,
                  DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
static inline HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                                 LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
                                 DWORD dwFlagsAndAttributes, decltype(nullptr)) {
    return CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition,
                      dwFlagsAndAttributes, 0);
}

#endif  // D2_OS_MAC
