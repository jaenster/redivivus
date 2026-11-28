// Win32Posix: the kernel entry points (files, time, threads, modules, locale, the registry,
// strings and the CRT's Microsoft extras) the shared game code calls, as the SDK declares them.
// Read through Win32Posix.h, after d2_platform.h's stand-ins (which declare the few it already
// has: GetTickCount, Sleep, the critical sections, CreateThread, ReadFile, CreateFileA ...).
#pragma once

#if !defined(_WIN32) && D2_BACKEND_POSIX

typedef BOOL* LPBOOL;
typedef long LSTATUS;
typedef DWORD LCID;
typedef DWORD LCTYPE;

#define HKEY_CURRENT_USER ((HKEY)(uintptr_t)0x80000001)
#define HKEY_LOCAL_MACHINE ((HKEY)(uintptr_t)0x80000002)
#define ERROR_SUCCESS 0L

union _ULARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    unsigned long long QuadPart;
};
typedef _ULARGE_INTEGER ULARGE_INTEGER;
typedef ULARGE_INTEGER* PULARGE_INTEGER;

// ---- files and directories ----
BOOL DeleteFileA(LPCSTR lpFileName);
BOOL CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
HANDLE FindFirstFileA(LPCSTR lpFileName, struct _WIN32_FIND_DATAA* lpFindFileData);
BOOL FindNextFileA(HANDLE hFindFile, struct _WIN32_FIND_DATAA* lpFindFileData);
BOOL FindClose(HANDLE hFindFile);
BOOL FlushFileBuffers(HANDLE hFile);
BOOL GetFileTime(HANDLE hFile, LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime);
BOOL SetFileTime(HANDLE hFile, const FILETIME* lpCreationTime, const FILETIME* lpLastAccessTime,
                 const FILETIME* lpLastWriteTime);
DWORD GetTempPathA(DWORD nBufferLength, LPSTR lpBuffer);
UINT GetTempFileNameA(LPCSTR lpPathName, LPCSTR lpPrefixString, UINT uUnique, LPSTR lpTempFileName);
BOOL GetDiskFreeSpaceA(LPCSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector,
                       LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters);
BOOL GetDiskFreeSpaceExA(LPCSTR lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailableToCaller,
                         PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes);
#define GetDiskFreeSpaceEx GetDiskFreeSpaceExA
UINT GetDriveTypeA(LPCSTR lpRootPathName);
BOOL GetVolumeInformationA(LPCSTR lpRootPathName, LPSTR lpVolumeNameBuffer, DWORD nVolumeNameSize,
                           LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentLength,
                           LPDWORD lpFileSystemFlags, LPSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize);
HANDLE CreateFileMappingA(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect,
                          DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName);
LPVOID MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh,
                     DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap);
BOOL UnmapViewOfFile(LPCVOID lpBaseAddress);

// ---- time ----
void GetSystemTime(LPSYSTEMTIME lpSystemTime);
DWORD GetTimeZoneInformation(struct _TIME_ZONE_INFORMATION* lpTimeZoneInformation);
LONG CompareFileTime(const FILETIME* lpFileTime1, const FILETIME* lpFileTime2);
BOOL SystemTimeToFileTime(const SYSTEMTIME* lpSystemTime, LPFILETIME lpFileTime);
BOOL FileTimeToSystemTime(const FILETIME* lpFileTime, LPSYSTEMTIME lpSystemTime);
BOOL FileTimeToLocalFileTime(const FILETIME* lpFileTime, LPFILETIME lpLocalFileTime);
int GetDateFormatA(LCID Locale, DWORD dwFlags, const SYSTEMTIME* lpDate, LPCSTR lpFormat, LPSTR lpDateStr,
                   int cchDate);
#define GetDateFormat GetDateFormatA

// ---- process, system, modules, resources ----
BOOL GetComputerNameA(LPSTR lpBuffer, LPDWORD nSize);
BOOL GetUserNameA(LPSTR lpBuffer, LPDWORD pcbBuffer);
LPSTR GetCommandLineA(void);
DWORD GetVersion(void);
void GetSystemInfo(struct _SYSTEM_INFO* lpSystemInfo);
BOOL TerminateProcess(HANDLE hProcess, UINT uExitCode);
BOOL GetExitCodeProcess(HANDLE hProcess, LPDWORD lpExitCode);
#define GetModuleHandle GetModuleHandleA
#define GetModuleFileName GetModuleFileNameA
HRSRC FindResourceA(HMODULE hModule, LPCSTR lpName, LPCSTR lpType);
HGLOBAL LoadResource(HMODULE hModule, HRSRC hResInfo);
LPVOID LockResource(HGLOBAL hResData);
DWORD SizeofResource(HMODULE hModule, HRSRC hResInfo);
BOOL FreeResource(HGLOBAL hResData);
int LoadStringA(HINSTANCE hInstance, UINT uID, LPSTR lpBuffer, int cchBufferMax);
DWORD FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer,
                     DWORD nSize, va_list* Arguments);

// ---- locale ----
int GetLocaleInfoA(LCID Locale, LCTYPE LCType, LPSTR lpLCData, int cchData);
LCID GetUserDefaultLCID(void);
LANGID GetUserDefaultLangID(void);
LANGID GetSystemDefaultLangID(void);

// ---- threads and synchronisation ----
void ExitThread(DWORD dwExitCode);
BOOL SetThreadPriority(HANDLE hThread, int nPriority);
uintptr_t _beginthreadex(void* lpSecurity, unsigned nStackSize, unsigned (*pfnStart)(void*), void* pArgList,
                         unsigned nInitFlag, unsigned* pThreadId);
static inline PVOID InterlockedCompareExchangePointer(PVOID volatile* ppDestination, PVOID pExchange,
                                                      PVOID pComparand) {
    return __sync_val_compare_and_swap(ppDestination, pComparand, pExchange);
}
// The tree's InterlockedCompareExchange by a plain pointer, which Bnclient stores in a function
// pointer (d2_platform.h defines it so on Windows).
static inline LONG d2_InterlockedCompareExchange(LONG* pDestination, LONG nExchange, LONG nComperand) {
    return InterlockedCompareExchange((volatile LONG*)pDestination, nExchange, nComperand);
}

// ---- the registry ----
LSTATUS RegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, DWORD samDesired, HKEY* phkResult);
LSTATUS RegCreateKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions,
                        DWORD samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, HKEY* phkResult,
                        LPDWORD lpdwDisposition);
LSTATUS RegCloseKey(HKEY hKey);
LSTATUS RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData,
                         LPDWORD lpcbData);
LSTATUS RegSetValueExA(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE* lpData,
                       DWORD cbData);
LSTATUS RegDeleteValueA(HKEY hKey, LPCSTR lpValueName);
LSTATUS RegFlushKey(HKEY hKey);

// ---- strings ----
DWORD CharLowerBuffA(LPSTR lpsz, DWORD cchLength);
int wvsprintfA(LPSTR lpOutput, LPCSTR lpFmt, va_list arglist);

// ---- the Microsoft CRT's extras ----
char* _itoa(int nValue, char* szBuffer, int nRadix);
char* _ultoa(unsigned long nValue, char* szBuffer, int nRadix);
char* _i64toa(long long nValue, char* szBuffer, int nRadix);
char* _ui64toa(unsigned long long nValue, char* szBuffer, int nRadix);
char* _strlwr(char* szString);
char* _strupr(char* szString);
char* _fullpath(char* szAbsPath, const char* szRelPath, size_t nMaxLength);

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
