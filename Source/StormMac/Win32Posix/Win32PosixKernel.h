// Win32Posix: the kernel entry points (files, time, threads, modules, locale, the registry,
// strings and the CRT's Microsoft extras) the shared game code calls, as the SDK declares them.
// Read through Win32Posix.h. d2_platform.h still declares the file calls it gives every
// non-Windows build (ReadFile, CreateFileA, SetFilePointer ...) over these types.
#pragma once

#if !defined(_WIN32) && D2_BACKEND_POSIX

#define HKEY_CURRENT_USER ((HKEY)(uintptr_t)0x80000001)
#define HKEY_LOCAL_MACHINE ((HKEY)(uintptr_t)0x80000002)
#define ERROR_SUCCESS 0L

// ---- the core the stand-ins used to give (time, errors, modules, heap, threads, memory) ----
extern "C" {
DWORD GetTickCount();
void Sleep(DWORD dwMilliseconds);
BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);
BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency);
int wsprintfA(LPSTR lpOut, LPCSTR lpFmt, ...);
int wsprintfW(LPWSTR lpOut, LPCWSTR lpFmt, ...);
DWORD GetLastError();
void SetLastError(DWORD dwErrCode);
DWORD GetCurrentThreadId();
DWORD GetCurrentProcessId();
void OutputDebugStringA(LPCSTR lpOutputString);
HMODULE GetModuleHandleA(LPCSTR lpModuleName);
FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
HMODULE LoadLibraryA(LPCSTR lpLibFileName);
BOOL FreeLibrary(HMODULE hLibModule);
HANDLE GetProcessHeap();
LPVOID HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
BOOL HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
void InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
HANDLE CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress,
                    LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
DWORD WaitForMultipleObjects(DWORD nCount, const HANDLE* lpHandles, BOOL bWaitAll, DWORD dwMilliseconds);
BOOL CloseHandle(HANDLE hObject);
void ExitProcess(UINT uExitCode);
LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
}
BOOL IsBadCodePtr(FARPROC lpfn);
BOOL IsBadReadPtr(const void* lp, UINT_PTR ucb);
BOOL IsBadWritePtr(LPVOID lp, UINT_PTR ucb);
HANDLE CreateEventA(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName);
HANDLE CreateEventW(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCWSTR lpName);
BOOL SetEvent(HANDLE hEvent);
BOOL ResetEvent(HANDLE hEvent);

// The interlocked operations, as the compiler's atomics (full barriers, as on Windows).
static inline LONG InterlockedIncrement(volatile LONG* p) { return __atomic_add_fetch(p, 1, __ATOMIC_SEQ_CST); }
static inline LONG InterlockedDecrement(volatile LONG* p) { return __atomic_sub_fetch(p, 1, __ATOMIC_SEQ_CST); }
static inline LONG InterlockedExchange(volatile LONG* p, LONG v) { return __atomic_exchange_n(p, v, __ATOMIC_SEQ_CST); }
static inline LONG InterlockedCompareExchange(volatile LONG* p, LONG exchange, LONG comparand) {
    __atomic_compare_exchange_n(p, &comparand, exchange, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return comparand;
}
// The generated code keeps some counters as `long`, which is LONG on Windows and 64 bits here.
static inline long InterlockedIncrement(volatile long* p) { return __atomic_add_fetch(p, 1, __ATOMIC_SEQ_CST); }
static inline long InterlockedDecrement(volatile long* p) { return __atomic_sub_fetch(p, 1, __ATOMIC_SEQ_CST); }

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
DWORD GetLogicalDriveStringsA(DWORD nBufferLength, LPSTR lpBuffer);
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

// The I/O completion port QServerNT's workers pump: emulated over the BSD sockets' readiness.
HANDLE CreateIoCompletionPort(HANDLE FileHandle, HANDLE ExistingCompletionPort, ULONG_PTR CompletionKey,
                              DWORD NumberOfConcurrentThreads);
BOOL GetQueuedCompletionStatus(HANDLE CompletionPort, LPDWORD lpNumberOfBytesTransferred, ULONG_PTR* lpCompletionKey,
                               LPOVERLAPPED* lpOverlapped, DWORD dwMilliseconds);
BOOL PostQueuedCompletionStatus(HANDLE CompletionPort, DWORD dwNumberOfBytesTransferred, ULONG_PTR dwCompletionKey,
                                LPOVERLAPPED lpOverlapped);
HANDLE GetCurrentThread(void);
int GetThreadPriority(HANDLE hThread);

// ---- memory ----
#define MEM_FREE 0x00010000
#define GMEM_MOVEABLE 0x0002
#define GMEM_DDESHARE 0x2000
SIZE_T VirtualQuery(LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength);
HGLOBAL GlobalAlloc(UINT uFlags, SIZE_T dwBytes);
LPVOID GlobalLock(HGLOBAL hMem);
BOOL GlobalUnlock(HGLOBAL hMem);
HGLOBAL GlobalFree(HGLOBAL hMem);

// ---- the private profile (D2.ini), read the way Windows reads an .ini file ----
UINT GetPrivateProfileIntA(LPCSTR lpAppName, LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName);
DWORD GetPrivateProfileStringA(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize,
                               LPCSTR lpFileName);
BOOL WritePrivateProfileStringA(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString, LPCSTR lpFileName);

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
#define CP_ACP 0
#define CP_UTF8 65001
UINT GetACP(void);
int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr,
                        int cchWideChar);
int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr,
                        int cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar);
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
