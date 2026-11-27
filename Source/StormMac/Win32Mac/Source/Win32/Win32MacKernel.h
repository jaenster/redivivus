// Storm's Win32-on-Mac layer: the kernel entry points (files, time, threads, processes,
// locale, strings, interlocked) as the shared game code calls them.
//
// The layer implements them under its own names, without the A suffix and without
// extern "C" (Win32_Files.cpp, Win32_TimeAndDate.cpp, Win32_Process.cpp, Win32_Threads.cpp,
// Win32_String.cpp, Win32_utils.cpp), and the 1.14d Mac build calls them so; the ANSI names
// the shared code spells resolve to them here. A call the layer does not implement is not
// declared: the code that makes it is not in the Mac binary (see win32-on-mac.tsv).
//
// Declared with the shared code's Win32 types (d2_platform.h), so it is read by
// d2_platform.h after them and not by the layer's own units, whose window and GDI headers
// give some of those names other types.
#pragma once

#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <stdio.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#define INVALID_HANDLE_VALUE ((HANDLE)-1)

typedef BOOL* LPBOOL;

// ---- Win32_Files.cpp ----
struct Win32MacFindData;

BOOL DeleteFile(LPCSTR lpFileName);
DWORD GetFileAttributes(LPCSTR lpFileName);
BOOL CreateDirectory(char* szPathName, void* pSecurityAttributes);
HANDLE FindFirstFile(char* szPattern, Win32MacFindData* pFindData);
BOOL FindNextFile(HANDLE hFind, Win32MacFindData* pFindData);
BOOL FindClose(HANDLE hFind);
// CreateFile takes a handle-table index as its template file: the shared code's `nullptr` is 0.
HANDLE CreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPVOID lpSecurityAttributes,
                  DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
BOOL GetFileTime(HANDLE hFile, FILETIME* lpCreationTime, FILETIME* lpLastAccessTime, FILETIME* lpLastWriteTime);
BOOL SetFileTime(HANDLE hFile, const FILETIME* lpCreationTime, const FILETIME* lpLastAccessTime,
                 const FILETIME* lpLastWriteTime);

static inline BOOL DeleteFileA(LPCSTR lpFileName) { return DeleteFile(lpFileName); }
static inline BOOL CreateDirectoryA(LPCSTR lpPathName, void* pSecurityAttributes) {
    return CreateDirectory(const_cast<char*>(lpPathName), pSecurityAttributes);
}
static inline HANDLE FindFirstFileA(LPCSTR lpFileName, _WIN32_FIND_DATAA* pFindData) {
    return FindFirstFile(const_cast<char*>(lpFileName), reinterpret_cast<Win32MacFindData*>(pFindData));
}
static inline BOOL FindNextFileA(HANDLE hFind, _WIN32_FIND_DATAA* pFindData) {
    return FindNextFile(hFind, reinterpret_cast<Win32MacFindData*>(pFindData));
}
static inline HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                                 LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
                                 DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    return CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition,
                      dwFlagsAndAttributes, hTemplateFile);
}
static inline HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                                 LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
                                 DWORD dwFlagsAndAttributes, decltype(nullptr)) {
    return CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition,
                      dwFlagsAndAttributes, 0);
}

// ---- Win32_TimeAndDate.cpp ----
DWORD GetTimeZoneInformation(TIME_ZONE_INFORMATION* lpTimeZoneInformation);
void GetSystemTime(SYSTEMTIME* lpSystemTime);
LONG CompareFileTime(const FILETIME* lpFileTime1, const FILETIME* lpFileTime2);
BOOL SystemTimeToFileTime(const SYSTEMTIME* lpSystemTime, FILETIME* lpFileTime);
BOOL FileTimeToSystemTime(const FILETIME* pFileTime, SYSTEMTIME* pSystemTime);
BOOL FileTimeToLocalFileTime(const FILETIME* pFileTime, FILETIME* pLocalFileTime);
int GetDateFormat(DWORD Locale, DWORD dwFlags, const SYSTEMTIME* lpDate, LPCSTR lpFormat, char* lpDateStr, int cchDate);

// ---- Win32_Process.cpp ----
BOOL GetComputerName(char* szBuffer, DWORD* pnSize);
BOOL GetUserName(char* szBuffer, DWORD* pnSize);
DWORD GetTempPath(DWORD nBufferLength, char* szBuffer);
UINT GetTempFileName(LPCSTR szPathName, LPCSTR szPrefix, UINT uUnique, char* szTempFileName);
BOOL GetDiskFreeSpaceEx(LPCSTR szDirectory, uint64_t* pnFreeBytesAvailable, uint64_t* pnTotalBytes,
                        uint64_t* pnTotalFreeBytes);
int GetLocaleInfo(DWORD dwLocale, DWORD dwLCType, char* szData, size_t nDataSize);
int32_t GetUserDefaultLangID(void);
DWORD LOCALE_GetUserLocaleId(void);
WORD LOCALE_GetDefaultLangId(void);
HANDLE GetModuleHandle(LPCSTR szModuleName);
DWORD GetModuleFileName(HANDLE hModule, char* szFileName, DWORD nSize);

static inline BOOL GetComputerNameA(char* szBuffer, DWORD* pnSize) { return GetComputerName(szBuffer, pnSize); }
#define GetUserNameA GetUserName
#define GetTempPathA GetTempPath
#define GetTempFileNameA GetTempFileName
#define GetLocaleInfoA GetLocaleInfo
#define GetUserDefaultLCID LOCALE_GetUserLocaleId
#define GetSystemDefaultLangID LOCALE_GetDefaultLangId

// The layer's interlocked functions take LONG*; the shared code passes volatile pointers,
// and `long`, a type distinct from LONG (int32_t) on the Mac.
static inline LONG InterlockedIncrement(volatile LONG* p) { return InterlockedIncrement((LPLONG)p); }
static inline LONG InterlockedDecrement(volatile LONG* p) { return InterlockedDecrement((LPLONG)p); }
static inline long InterlockedIncrement(volatile long* p) { return InterlockedIncrement((LPLONG)p); }
static inline long InterlockedDecrement(volatile long* p) { return InterlockedDecrement((LPLONG)p); }
static inline LONG InterlockedExchange(volatile LONG* p, LONG v) { return InterlockedExchange((LPLONG)p, v); }
static inline LONG InterlockedCompareExchange(volatile LONG* p, LONG exchange, LONG comparand) {
    return InterlockedCompareExchange((LPLONG)p, exchange, comparand);
}
static inline PVOID InterlockedCompareExchangePointer(PVOID volatile* ppDestination, PVOID pExchange, PVOID pComparand) {
    return (PVOID)InterlockedCompareExchange((LPLONG)ppDestination, (LONG)(intptr_t)pExchange, (LONG)(intptr_t)pComparand);
}
static inline LONG d2_InterlockedCompareExchange(LONG* pDestination, LONG nExchange, LONG nComperand) {
    return InterlockedCompareExchange((LPLONG)pDestination, nExchange, nComperand);
}

// ---- Win32_Process.cpp, Win32_Memory.cpp, Win32_Threads.cpp, Win32_TimeAndDate.cpp, Win32_utils.cpp ----
BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);
BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency);
DWORD GetLastError(void);
DWORD GetCurrentThreadId(void);
void OutputDebugString(LPCSTR szOutputString);
FARPROC GetProcAddress(HANDLE hModule, LPCSTR szProcName);
HANDLE LoadLibrary(LPCSTR szFileName);
BOOL FreeLibrary(HANDLE hModule);
void InitializeCriticalSection(LPCRITICAL_SECTION pCriticalSection);
void EnterCriticalSection(LPCRITICAL_SECTION pCriticalSection);
void LeaveCriticalSection(LPCRITICAL_SECTION pCriticalSection);
void DeleteCriticalSection(LPCRITICAL_SECTION pCriticalSection);
int MessageBox(void* hWnd, LPCSTR szText, LPCSTR szCaption, UINT uType);
LPVOID VirtualAlloc(LPVOID lpAddress, size_t dwSize, DWORD flAllocationType, DWORD flProtect);
BOOL VirtualFree(LPVOID lpAddress, size_t dwSize, DWORD dwFreeType);

DWORD TIME_GetMilliseconds(void);
DWORD MsgWaitForMultipleObjects(DWORD nCount, const HANDLE* pHandles, BOOL bWaitAll, DWORD dwMilliseconds,
                                DWORD dwWakeMask);
// Win32Mac has no timeGetTime of its own: the 1.14d Mac build reads the millisecond timer (0002c0f7).
#define timeGetTime TIME_GetMilliseconds
static inline DWORD WaitForMultipleObjects(DWORD nCount, const HANDLE* pHandles, BOOL bWaitAll,
                                           DWORD dwMilliseconds) {
    return MsgWaitForMultipleObjects(nCount, pHandles, bWaitAll, dwMilliseconds, 0);
}
#define GetFileAttributesA GetFileAttributes
// The layer's module handles are handle-table indices (HANDLE, an int); the shared code holds
// them as HMODULE pointers, the same value.
static inline HMODULE GetModuleHandleA(LPCSTR szModuleName) { return (HMODULE)(intptr_t)GetModuleHandle(szModuleName); }
static inline DWORD GetModuleFileNameA(HMODULE hModule, LPSTR szFileName, DWORD nSize) {
    return GetModuleFileName((HANDLE)(intptr_t)hModule, szFileName, nSize);
}
static inline HMODULE LoadLibraryA(LPCSTR szFileName) { return (HMODULE)(intptr_t)LoadLibrary(szFileName); }
static inline FARPROC GetProcAddress(HMODULE hModule, LPCSTR szProcName) {
    return GetProcAddress((HANDLE)(intptr_t)hModule, szProcName);
}
static inline BOOL FreeLibrary(HMODULE hModule) { return FreeLibrary((HANDLE)(intptr_t)hModule); }
#define OutputDebugStringA OutputDebugString
// The Mac binary formats with libsystem's sprintf (SPRITECACHE_LoadDC6 0013333b), never wsprintf.
#define wsprintfA sprintf
static inline int MessageBoxA(HWND hWnd, LPCSTR szText, LPCSTR szCaption, UINT uType) {
    return MessageBox((void*)hWnd, szText, szCaption, uType);
}

// ---- Win32_Threads.cpp ----
// The Mac signatures carry the thread's name, for pthread_setname_np.
HANDLE CreateThread(LPVOID lpThreadAttributes, size_t dwStackSize, LPTHREAD_START_ROUTINE pfnStart,
                    LPVOID pParameter, DWORD dwCreationFlags, LPDWORD pThreadId, LPCSTR szName);
HANDLE _beginthreadex(LPVOID lpSecurity, size_t nStackSize, LPTHREAD_START_ROUTINE pfnStart, LPVOID pArgList,
                      DWORD dwInitFlag, LPDWORD pThreadId, LPCSTR szName);
void ExitThread(DWORD dwExitCode);
BOOL SetThreadPriority(HANDLE hThread, int nPriority);

// ---- Win32_String.cpp, Win32_utils.cpp ----
char* _strrev(char* szString);
char* _strlwr(char* szString);
DWORD CharLowerBuff(char* lpsz, DWORD cchLength);
#define CharLowerBuffA CharLowerBuff
#define wvsprintfA vsprintf

#endif  // D2_OS_MAC
