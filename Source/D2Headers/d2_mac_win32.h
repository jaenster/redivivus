// The Win32 entry points the shared code calls by their ANSI names, on the Mac.
//
// Storm's Win32-on-Mac layer (Source/StormMac/Win32Mac) implements them under the
// unsuffixed names (DeleteFile, FindFirstFile ...), and the 1.14d Mac build calls them so.
// These are the layer's own functions, declared as it defines them, with the ANSI names
// the shared code spells as thin forwarders. Calls the layer does not implement are not
// declared: the code that makes them is not in the Mac binary (see win32-on-mac.tsv).
#pragma once

#if !defined(_WIN32) && D2_OS_MAC

struct Win32MacFindData;

BOOL DeleteFile(LPCSTR lpFileName);
BOOL CreateDirectory(char* szPathName, void* pSecurityAttributes);
HANDLE FindFirstFile(char* szPattern, Win32MacFindData* pFindData);
BOOL FindNextFile(HANDLE hFind, Win32MacFindData* pFindData);
BOOL FindClose(HANDLE hFind);
BOOL GetComputerName(char* szBuffer, DWORD* pnSize);
BOOL SystemTimeToFileTime(const SYSTEMTIME* lpSystemTime, FILETIME* lpFileTime);
void ExitThread(DWORD dwExitCode);
BOOL SetThreadPriority(HANDLE hThread, int nPriority);

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
static inline BOOL GetComputerNameA(char* szBuffer, DWORD* pnSize) { return GetComputerName(szBuffer, pnSize); }

#endif  // D2_OS_MAC
