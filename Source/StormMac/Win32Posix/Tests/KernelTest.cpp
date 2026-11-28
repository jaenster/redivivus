// The Win32Posix kernel's unit test: files, the find calls, events and waits, threads and
// critical sections, the clocks, the profile and the registry, virtual memory, file mappings,
// the string calls and the completion port, each against what Windows answers.
//
//   ninja -C <modern build> d2_win32posix_test && <modern build>/d2_win32posix_test
//
// It runs in a fresh temporary install directory (D2_INSTALL_DIR) with its own registry file
// (D2_REGISTRY), and exits nonzero when a check fails.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Source/StormMac/Win32Posix/Win32PosixHost.h"

namespace {

int g_nChecks = 0;
int g_nFailures = 0;

void Check(bool bOk, const char* szWhat, int nLine) {
    ++g_nChecks;
    if (!bOk) {
        ++g_nFailures;
        fprintf(stderr, "FAIL line %d: %s (GetLastError %u)\n", nLine, szWhat, GetLastError());
    }
}
#define CHECK(x) Check((x), #x, __LINE__)

std::string g_sDir;  // the install directory, Windows-spelled, with the separator

std::string WinPath(const char* szLeaf) {
    return g_sDir + szLeaf;
}

void TestFiles() {
    std::string sPath = WinPath("Save\\RoundTrip.d2s");
    CHECK(CreateDirectoryA(WinPath("Save").c_str(), nullptr));
    CHECK(!CreateDirectoryA(WinPath("Save").c_str(), nullptr) && GetLastError() == 183);
    HANDLE h = CreateFileA(sPath.c_str(), 0xC0000000, 0, nullptr, 2 /* CREATE_ALWAYS */, 0x80, nullptr);
    CHECK(h != INVALID_HANDLE_VALUE);
    CHECK(((uintptr_t)h >> 32) == 0 && ((uintptr_t)h & 3) == 0);  // a 32-bit handle value
    static const char kData[] = "Stay a while and listen.";
    DWORD n = 0;
    CHECK(WriteFile(h, kData, sizeof(kData), &n, nullptr) && n == sizeof(kData));
    CHECK(GetFileSize(h, nullptr) == sizeof(kData));
    CHECK(SetFilePointer(h, 5, nullptr, 0) == 5);
    char aBuffer[64] = {};
    CHECK(ReadFile(h, aBuffer, 5, &n, nullptr) && n == 5 && memcmp(aBuffer, "a whi", 5) == 0);
    LONG nHigh = -1;  // with the high word, a negative distance is negative in both halves
    CHECK(SetFilePointer(h, -3, &nHigh, 2 /* FILE_END */) == sizeof(kData) - 3 && nHigh == 0);
    CHECK(ReadFile(h, aBuffer, sizeof(aBuffer), &n, nullptr) && n == 3);
    CHECK(ReadFile(h, aBuffer, sizeof(aBuffer), &n, nullptr) && n == 0);  // end of file is not an error
    FILETIME tWrite;
    CHECK(GetFileTime(h, nullptr, nullptr, &tWrite) && tWrite.dwHighDateTime != 0);
    CHECK(CloseHandle(h));
    CHECK(!CloseHandle(h) && GetLastError() == 6);

    // the case-blind lookup, the forward slash and the drive letter
    std::string sOther = g_sDir;
    for (char& c : sOther) {
        if (c == '\\') {
            c = '/';
        }
    }
    h = CreateFileA((sOther + "SAVE/roundtrip.D2S").c_str(), 0x80000000, 1, nullptr, 3 /* OPEN_EXISTING */, 0, nullptr);
    CHECK(h != INVALID_HANDLE_VALUE);
    CHECK(ReadFile(h, aBuffer, sizeof(aBuffer), &n, nullptr) && n == sizeof(kData) && strcmp(aBuffer, kData) == 0);
    CloseHandle(h);
    // relative to the install directory, as relative to the game's directory on Windows
    h = CreateFileA("save\\RoundTrip.d2s", 0x80000000, 1, nullptr, 3, 0, nullptr);
    CHECK(h != INVALID_HANDLE_VALUE);
    CloseHandle(h);

    CHECK(GetFileAttributesA(WinPath("Save").c_str()) == 0x10);
    CHECK(GetFileAttributesA(sPath.c_str()) == 0x20);
    CHECK(GetFileAttributesA(WinPath("Missing.txt").c_str()) == 0xFFFFFFFF && GetLastError() == 2);
    h = CreateFileA(WinPath("Missing.txt").c_str(), 0x80000000, 1, nullptr, 3, 0, nullptr);
    CHECK(h == INVALID_HANDLE_VALUE && GetLastError() == 2);
    h = CreateFileA(WinPath("NoDir\\x.txt").c_str(), 0x40000000, 0, nullptr, 2, 0, nullptr);
    CHECK(h == INVALID_HANDLE_VALUE && GetLastError() == 3);
    h = CreateFileA(sPath.c_str(), 0x40000000, 0, nullptr, 1 /* CREATE_NEW */, 0, nullptr);
    CHECK(h == INVALID_HANDLE_VALUE && GetLastError() == 80);
    h = CreateFileA(sPath.c_str(), 0x40000000, 0, nullptr, 4 /* OPEN_ALWAYS */, 0, nullptr);
    CHECK(h != INVALID_HANDLE_VALUE && GetLastError() == 183);
    CloseHandle(h);
    CHECK(DeleteFileA(sPath.c_str()));
    CHECK(!DeleteFileA(sPath.c_str()) && GetLastError() == 2);

    char szFull[MAX_PATH];
    CHECK(_fullpath(szFull, "Save\\..\\D2.ini", sizeof(szFull)) && WinPath("D2.ini") == szFull);
    char szModule[MAX_PATH];
    CHECK(GetModuleFileNameA(nullptr, szModule, sizeof(szModule)) == strlen(szModule) && WinPath("Game.exe") == szModule);
    char szTemp[MAX_PATH];
    DWORD nTemp = GetTempPathA(sizeof(szTemp), szTemp);
    CHECK(nTemp > 3 && szTemp[1] == ':' && szTemp[nTemp - 1] == '\\');
    char szTempFile[MAX_PATH];
    UINT uUnique = GetTempFileNameA(g_sDir.c_str(), "BNetAd", 0, szTempFile);
    CHECK(uUnique != 0 && strstr(szTempFile, "BNe") && GetFileAttributesA(szTempFile) == 0x20);
    DeleteFileA(szTempFile);
}

void TestFind() {
    const char* const aNames[] = {"b.mpq", "A.MPQ", "c.txt", "d.mpq.bak"};
    for (const char* szName : aNames) {
        HANDLE h = CreateFileA(WinPath(szName).c_str(), 0x40000000, 0, nullptr, 2, 0, nullptr);
        CHECK(h != INVALID_HANDLE_VALUE);
        CloseHandle(h);
    }
    WIN32_FIND_DATAA tData;
    HANDLE hFind = FindFirstFileA(WinPath("*.mpq").c_str(), &tData);
    CHECK(hFind != INVALID_HANDLE_VALUE);
    std::vector<std::string> aFound;
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            aFound.push_back(tData.cFileName);
        } while (FindNextFileA(hFind, &tData));
        CHECK(GetLastError() == 18);  // ERROR_NO_MORE_FILES
        CHECK(FindClose(hFind));
    }
    CHECK(aFound.size() == 2 && aFound[0] == "A.MPQ" && aFound[1] == "b.mpq");  // name order, case-blind

    hFind = FindFirstFileA(WinPath("*.*").c_str(), &tData);
    int nAll = 0;
    bool bDots = false;
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            ++nAll;
            bDots = bDots || strcmp(tData.cFileName, "..") == 0;
        } while (FindNextFileA(hFind, &tData));
        FindClose(hFind);
    }
    CHECK(bDots && nAll >= 7);  // . .. Save and the four files
    CHECK(FindFirstFileA(WinPath("*.zzz").c_str(), &tData) == INVALID_HANDLE_VALUE && GetLastError() == 2);
    CHECK(FindFirstFileA(WinPath("NoDir\\*.*").c_str(), &tData) == INVALID_HANDLE_VALUE && GetLastError() == 3);
}

DWORD g_dwSeen = 0;

DWORD WaiterThread(LPVOID pArg) {
    HANDLE hEvent = (HANDLE)pArg;
    if (WaitForSingleObject(hEvent, 5000) != WAIT_OBJECT_0) {
        return 1;
    }
    g_dwSeen = 0x1234;
    return 42;
}

void TestEvents() {
    HANDLE hAuto = CreateEventA(nullptr, FALSE, FALSE, nullptr);
    HANDLE hManual = CreateEventA(nullptr, TRUE, FALSE, nullptr);
    CHECK(hAuto && hManual);
    CHECK(WaitForSingleObject(hAuto, 0) == WAIT_TIMEOUT);
    DWORD dwStart = GetTickCount();
    CHECK(WaitForSingleObject(hAuto, 60) == WAIT_TIMEOUT);
    CHECK(GetTickCount() - dwStart >= 55);
    CHECK(SetEvent(hAuto));
    CHECK(WaitForSingleObject(hAuto, 0) == WAIT_OBJECT_0);
    CHECK(WaitForSingleObject(hAuto, 0) == WAIT_TIMEOUT);  // the wait reset it
    CHECK(SetEvent(hManual));
    CHECK(WaitForSingleObject(hManual, 0) == WAIT_OBJECT_0);
    CHECK(WaitForSingleObject(hManual, 0) == WAIT_OBJECT_0);  // a manual event stays set
    CHECK(ResetEvent(hManual));
    HANDLE aBoth[2] = {hAuto, hManual};
    CHECK(WaitForMultipleObjects(2, aBoth, FALSE, 0) == WAIT_TIMEOUT);
    SetEvent(hManual);
    CHECK(WaitForMultipleObjects(2, aBoth, FALSE, 0) == WAIT_OBJECT_0 + 1);
    CHECK(WaitForMultipleObjects(2, aBoth, TRUE, 0) == WAIT_TIMEOUT);
    SetEvent(hAuto);
    CHECK(WaitForMultipleObjects(2, aBoth, TRUE, 0) == WAIT_OBJECT_0);
    CHECK(WaitForSingleObject(hAuto, 0) == WAIT_TIMEOUT);
    CHECK(WaitForSingleObject((HANDLE)(uintptr_t)0x7777770, 0) == WAIT_FAILED && GetLastError() == 6);

    // a thread wakes on an event set from here, and its handle signals when it ends
    ResetEvent(hManual);
    DWORD dwId = 0;
    HANDLE hThread = CreateThread(nullptr, 0, WaiterThread, hManual, 0, &dwId);
    CHECK(hThread && dwId != 0 && dwId != GetCurrentThreadId());
    CHECK(WaitForSingleObject(hThread, 50) == WAIT_TIMEOUT);
    SetEvent(hManual);
    CHECK(WaitForSingleObject(hThread, 5000) == WAIT_OBJECT_0);
    DWORD dwExit = 0;
    CHECK(GetExitCodeProcess(hThread, &dwExit) && dwExit == 42 && g_dwSeen == 0x1234);
    CloseHandle(hThread);
    CloseHandle(hAuto);
    CloseHandle(hManual);
}

CRITICAL_SECTION g_tSection;
long g_nCounter = 0;

unsigned CounterThread(void*) {
    for (int i = 0; i < 20000; ++i) {
        EnterCriticalSection(&g_tSection);
        EnterCriticalSection(&g_tSection);  // re-entrant
        long n = g_nCounter;
        g_nCounter = n + 1;
        LeaveCriticalSection(&g_tSection);
        LeaveCriticalSection(&g_tSection);
    }
    return 0;
}

void TestThreads() {
    InitializeCriticalSection(&g_tSection);
    HANDLE aThreads[4];
    for (HANDLE& h : aThreads) {
        unsigned nId = 0;
        h = (HANDLE)_beginthreadex(nullptr, 0, CounterThread, nullptr, 0, &nId);
        CHECK(h != nullptr && nId != 0);
    }
    CHECK(WaitForMultipleObjects(4, aThreads, TRUE, 10000) == WAIT_OBJECT_0);
    CHECK(g_nCounter == 4 * 20000);
    CHECK(g_tSection.RecursionCount == 0 && g_tSection.OwningThread == nullptr);
    for (HANDLE h : aThreads) {
        CloseHandle(h);
    }
    DeleteCriticalSection(&g_tSection);

    CRITICAL_SECTION tZero = {};  // entered without InitializeCriticalSection
    EnterCriticalSection(&tZero);
    CHECK(tZero.RecursionCount == 1 && tZero.OwningThread == (HANDLE)(uintptr_t)GetCurrentThreadId());
    LeaveCriticalSection(&tZero);
    DeleteCriticalSection(&tZero);

    // A section at +4 of a packed record, as Fog's pool managers keep theirs: on a 64-bit
    // target its lock slot is not pointer-aligned. Initialised and lazily made both work.
    struct __attribute__((packed)) PackedSection {
        int32_t nId;
        CRITICAL_SECTION tSection;
    };
    static_assert(offsetof(PackedSection, tSection) == 4, "the section sits at +4");
    for (int nLazy = 0; nLazy < 2; ++nLazy) {
        PackedSection tPacked;
        memset(&tPacked, 0, sizeof(tPacked));
        if (!nLazy) {
            InitializeCriticalSection(&tPacked.tSection);
        }
        EnterCriticalSection(&tPacked.tSection);
        EnterCriticalSection(&tPacked.tSection);
        CHECK(tPacked.tSection.RecursionCount == 2);
        LeaveCriticalSection(&tPacked.tSection);
        LeaveCriticalSection(&tPacked.tSection);
        CHECK(tPacked.tSection.RecursionCount == 0);
        DeleteCriticalSection(&tPacked.tSection);
    }

    volatile LONG nValue = 5;
    CHECK(InterlockedIncrement(&nValue) == 6 && InterlockedExchange(&nValue, 9) == 6);
    CHECK(InterlockedCompareExchange(&nValue, 1, 9) == 9 && nValue == 1);
}

void TestClocks() {
    DWORD dwStart = GetTickCount();
    LARGE_INTEGER tFreq, tStart, tEnd;
    CHECK(QueryPerformanceFrequency(&tFreq) && tFreq.QuadPart == 10000000);
    QueryPerformanceCounter(&tStart);
    Sleep(100);
    QueryPerformanceCounter(&tEnd);
    DWORD dwElapsed = GetTickCount() - dwStart;
    CHECK(dwElapsed >= 95 && dwElapsed < 1000);
    LONGLONG nTicks = tEnd.QuadPart - tStart.QuadPart;
    CHECK(nTicks >= 950000 && nTicks < 10000000);
    CHECK(timeGetTime() - GetTickCount() < 2);

    SYSTEMTIME tTime = {2001, 6, 0, 29, 12, 34, 56, 789};  // Lord of Destruction's release
    FILETIME tFile;
    CHECK(SystemTimeToFileTime(&tTime, &tFile));
    CHECK(((uint64_t)tFile.dwHighDateTime << 32 | tFile.dwLowDateTime) == 126382916967890000ULL);
    SYSTEMTIME tBack;
    CHECK(FileTimeToSystemTime(&tFile, &tBack));
    CHECK(tBack.wYear == 2001 && tBack.wMonth == 6 && tBack.wDay == 29 && tBack.wDayOfWeek == 5 && tBack.wHour == 12 &&
          tBack.wMinute == 34 && tBack.wSecond == 56 && tBack.wMilliseconds == 789);
    FILETIME tLater = tFile;
    tLater.dwLowDateTime += 1;
    CHECK(CompareFileTime(&tFile, &tLater) == -1 && CompareFileTime(&tLater, &tFile) == 1 &&
          CompareFileTime(&tFile, &tFile) == 0);
    SYSTEMTIME tBad = {2001, 2, 0, 29, 0, 0, 0, 0};
    CHECK(!SystemTimeToFileTime(&tBad, &tFile));
    char szDate[64];
    CHECK(GetDateFormatA(0x409, 0, &tTime, nullptr, szDate, sizeof(szDate)) == 10 && strcmp(szDate, "6/29/2001") == 0);
    CHECK(GetDateFormatA(0x409, 0, &tTime, "ddd',' MMM dd ''yy", szDate, sizeof(szDate)) &&
          strcmp(szDate, "Fri, Jun 29 '01") == 0);
    SYSTEMTIME tNow;
    GetLocalTime(&tNow);
    CHECK(tNow.wYear >= 2024 && tNow.wMonth >= 1 && tNow.wMonth <= 12);
}

void TestProfile() {
    std::string sIni = WinPath("D2.ini");
    FILE* pFile = fopen((getenv("D2_INSTALL_DIR") + std::string("/D2.ini")).c_str(), "wb");
    fputs("; the game's settings\r\n[Diablo II]\r\nWindowed = 1\r\nTitle=\"Diablo II\"\r\n\r\n[Video]\r\nGamma=-3\r\n",
          pFile);
    fclose(pFile);
    char szValue[64];
    CHECK(GetPrivateProfileStringA("diablo ii", "TITLE", "x", szValue, sizeof(szValue), sIni.c_str()) == 9 &&
          strcmp(szValue, "Diablo II") == 0);
    CHECK(GetPrivateProfileIntA("Diablo II", "Windowed", 0, sIni.c_str()) == 1);
    CHECK((int)GetPrivateProfileIntA("Video", "Gamma", 0, sIni.c_str()) == -3);
    CHECK(GetPrivateProfileIntA("Video", "Missing", 77, sIni.c_str()) == 77);
    CHECK(GetPrivateProfileStringA("Video", "Missing", "dflt  ", szValue, sizeof(szValue), sIni.c_str()) == 4 &&
          strcmp(szValue, "dflt") == 0);
    CHECK(GetPrivateProfileStringA("Diablo II", "Title", "", szValue, 6, sIni.c_str()) == 5 &&
          strcmp(szValue, "Diabl") == 0);
    char szList[64];
    DWORD n = GetPrivateProfileStringA(nullptr, nullptr, "", szList, sizeof(szList), sIni.c_str());
    CHECK(n == 16 && memcmp(szList, "Diablo II\0Video\0\0", 17) == 0);
    CHECK(WritePrivateProfileStringA("Video", "Gamma", "4", sIni.c_str()));
    CHECK(WritePrivateProfileStringA("Diablo II", "NoSound", "1", sIni.c_str()));
    CHECK(GetPrivateProfileIntA("Video", "Gamma", 0, sIni.c_str()) == 4);
    CHECK(GetPrivateProfileIntA("Diablo II", "NoSound", 0, sIni.c_str()) == 1);
    CHECK(GetPrivateProfileIntA("Diablo II", "Windowed", 0, sIni.c_str()) == 1);
    CHECK(WritePrivateProfileStringA("Diablo II", "NoSound", nullptr, sIni.c_str()));
    CHECK(GetPrivateProfileIntA("Diablo II", "NoSound", 9, sIni.c_str()) == 9);
    // a bare name is in the install directory
    CHECK(GetPrivateProfileIntA("Diablo II", "Windowed", 0, "D2.ini") == 1);
}

void TestRegistry() {
    HKEY hKey;
    const char* szKey = "Software\\Blizzard Entertainment\\Diablo II";
    CHECK(RegOpenKeyExA(HKEY_CURRENT_USER, szKey, 0, 0x20019, &hKey) == 2);
    DWORD dwDisposition = 0;
    CHECK(RegCreateKeyExA(HKEY_CURRENT_USER, szKey, 0, nullptr, 0, 0xF003F, nullptr, &hKey, &dwDisposition) == 0 &&
          dwDisposition == 1);
    CHECK(RegSetValueExA(hKey, "Save Path", 0, 1, (const BYTE*)"C:\\Saves\\", 10) == 0);
    DWORD dwResolution = 1;
    CHECK(RegSetValueExA(hKey, "Resolution", 0, 4, (const BYTE*)&dwResolution, 4) == 0);
    CHECK(RegCloseKey(hKey) == 0);
    HKEY hParent;
    CHECK(RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Blizzard Entertainment", 0, 0x20019, &hParent) == 0);
    RegCloseKey(hParent);
    CHECK(RegOpenKeyExA(HKEY_CURRENT_USER, "software\\BLIZZARD ENTERTAINMENT\\diablo ii", 0, 0x20019, &hKey) == 0);
    char szValue[64];
    DWORD dwType = 0, cb = 4;
    CHECK(RegQueryValueExA(hKey, "Save Path", nullptr, &dwType, (LPBYTE)szValue, &cb) == 234 && cb == 10);
    cb = sizeof(szValue);
    CHECK(RegQueryValueExA(hKey, "save path", nullptr, &dwType, (LPBYTE)szValue, &cb) == 0 && dwType == 1 && cb == 10 &&
          strcmp(szValue, "C:\\Saves\\") == 0);
    DWORD dwRead = 0;
    cb = 4;
    CHECK(RegQueryValueExA(hKey, "Resolution", nullptr, &dwType, (LPBYTE)&dwRead, &cb) == 0 && dwType == 4 && dwRead == 1);
    CHECK(RegDeleteValueA(hKey, "Resolution") == 0);
    CHECK(RegQueryValueExA(hKey, "Resolution", nullptr, &dwType, (LPBYTE)&dwRead, &cb) == 2);
    CHECK(RegFlushKey(hKey) == 0);
    RegCloseKey(hKey);
    FILE* pFile = fopen(getenv("D2_REGISTRY"), "rb");
    char aText[512] = {};
    CHECK(pFile && fread(aText, 1, sizeof(aText) - 1, pFile) > 0);
    if (pFile) {
        fclose(pFile);
    }
    CHECK(strstr(aText, "[HKEY_CURRENT_USER\\Software\\Blizzard Entertainment\\Diablo II]") &&
          strstr(aText, "Save Path=sz:C:\\Saves\\"));
}

void TestMemory() {
    SYSTEM_INFO tInfo;
    GetSystemInfo(&tInfo);
    CHECK(tInfo.dwPageSize == 0x1000 && tInfo.dwAllocationGranularity == 0x10000 && tInfo.dwNumberOfProcessors >= 1);
    uint8_t* pBase = (uint8_t*)VirtualAlloc(nullptr, 0x100000, MEM_RESERVE, PAGE_NOACCESS);
    CHECK(pBase && ((uintptr_t)pBase & 0xFFFF) == 0);
    MEMORY_BASIC_INFORMATION mbi;
    CHECK(VirtualQuery(pBase, &mbi, sizeof(mbi)) == sizeof(mbi) && mbi.State == MEM_RESERVE &&
          mbi.AllocationBase == pBase && mbi.RegionSize == 0x100000);
    CHECK(IsBadReadPtr(pBase, 1));
    uint8_t* pCommit = (uint8_t*)VirtualAlloc(pBase + 0x3000, 0x2000, MEM_COMMIT, PAGE_READWRITE);
    CHECK(pCommit == pBase + 0x3000);
    pCommit[0] = 1;
    pCommit[0x1FFF] = 2;
    CHECK(!IsBadWritePtr(pCommit, 0x2000));
    CHECK(VirtualQuery(pBase + 0x3800, &mbi, sizeof(mbi)) && mbi.State == MEM_COMMIT && mbi.BaseAddress == pBase + 0x3000 &&
          mbi.RegionSize == 0x2000 && mbi.Protect == PAGE_READWRITE);
    CHECK(VirtualFree(pCommit, 0x2000, 0x4000 /* MEM_DECOMMIT */));
    CHECK(VirtualQuery(pCommit, &mbi, sizeof(mbi)) && mbi.State == MEM_RESERVE);
    pCommit = (uint8_t*)VirtualAlloc(pBase + 0x3000, 0x1000, MEM_COMMIT, PAGE_READWRITE);
    CHECK(pCommit && pCommit[0] == 0);  // recommitted memory reads zero
    CHECK(!VirtualFree(pBase + 0x1000, 0, MEM_RELEASE));  // only the base releases
    CHECK(VirtualFree(pBase, 0, MEM_RELEASE));
    CHECK(VirtualQuery(pBase, &mbi, sizeof(mbi)) && mbi.State != MEM_COMMIT);
    uint8_t* pBoth = (uint8_t*)VirtualAlloc(nullptr, 5000, MEM_COMMIT, PAGE_READWRITE);  // commit alone reserves too
    CHECK(pBoth && pBoth[4999] == 0);
    VirtualFree(pBoth, 0, MEM_RELEASE);

    int nLocal = 0;
    CHECK(!IsBadReadPtr(&nLocal, sizeof(nLocal)) && IsBadReadPtr(nullptr, 4) && !IsBadReadPtr(nullptr, 0));
    CHECK(!IsBadCodePtr((FARPROC)&GetTickCount) && IsBadCodePtr(nullptr));
    CHECK(VirtualQuery((LPCVOID)0x10000, &mbi, sizeof(mbi)) && mbi.State != MEM_COMMIT);

    uint8_t* p = (uint8_t*)HeapAlloc(GetProcessHeap(), 0x08, 64);
    CHECK(p && p[63] == 0 && HeapFree(GetProcessHeap(), 0, p));
    HGLOBAL hMem = GlobalAlloc(0x2002, 16);
    CHECK(hMem && GlobalLock(hMem) == hMem && GlobalFree(hMem) == nullptr);
}

void TestMapping() {
    std::string sPath = WinPath("Mapped.bin");
    HANDLE hFile = CreateFileA(sPath.c_str(), 0xC0000000, 0, nullptr, 2, 0, nullptr);
    std::vector<uint8_t> aData(70000);
    for (size_t i = 0; i < aData.size(); ++i) {
        aData[i] = (uint8_t)(i * 7);
    }
    DWORD n;
    WriteFile(hFile, aData.data(), (DWORD)aData.size(), &n, nullptr);
    HANDLE hMapping = CreateFileMappingA(hFile, nullptr, 0x8000002 /* SEC_COMMIT | PAGE_READONLY */, 0, 0, nullptr);
    CHECK(hMapping != nullptr);
    const uint8_t* pView = (const uint8_t*)MapViewOfFile(hMapping, 4 /* FILE_MAP_READ */, 0, 0, 0);
    CHECK(pView && pView[69999] == (uint8_t)(69999 * 7) && memcmp(pView, aData.data(), aData.size()) == 0);
    CHECK(UnmapViewOfFile(pView));
    CHECK(!UnmapViewOfFile(pView));
    CloseHandle(hMapping);
    CloseHandle(hFile);
}

void TestStrings() {
    char sz[256];
    CHECK(wsprintfA(sz, "%s %d %u %5d|%-4s|%x %X %02x %c %ld", "abc", -5, 4000000000u, 42, "ab", 255, 255, 7, 'Z', 123L) > 0 &&
          strcmp(sz, "abc -5 4000000000    42|ab  |ff FF 07 Z 123") == 0);
    uint16_t aWide[] = {'w', 'i', 'd', 'e', 0};
    CHECK(wsprintfA(sz, "%S|%ls|%hs|%I64d|%%|%.2s", aWide, aWide, "nar", -1234567890123LL, "xyz") > 0 &&
          strcmp(sz, "wide|wide|nar|-1234567890123|%|xy") == 0);
    WCHAR aOut[64];
    uint16_t aFormat[] = {'%', 's', '-', '%', 'S', '-', '%', 'd', 0};
    CHECK(wsprintfW(aOut, aFormat, aWide, "n", 7) == 8 && aOut[0] == 'w' && aOut[5] == 'n' && aOut[7] == '7' &&
          aOut[8] == 0);

    WCHAR aUtf16[16];
    CHECK(MultiByteToWideChar(CP_UTF8, 0, "\xE2\x82\xAC" "a\xF0\x9F\x98\x80", -1, nullptr, 0) == 5);
    CHECK(MultiByteToWideChar(CP_UTF8, 0, "\xE2\x82\xAC" "a\xF0\x9F\x98\x80", -1, aUtf16, 16) == 5 && aUtf16[0] == 0x20AC &&
          aUtf16[1] == 'a' && aUtf16[2] == 0xD83D && aUtf16[3] == 0xDE00 && aUtf16[4] == 0);
    char aBack[16];
    CHECK(WideCharToMultiByte(CP_UTF8, 0, aUtf16, -1, aBack, sizeof(aBack), nullptr, nullptr) == 9 &&
          strcmp(aBack, "\xE2\x82\xAC" "a\xF0\x9F\x98\x80") == 0);
    CHECK(MultiByteToWideChar(CP_ACP, 0, "\x80\xE9", 2, aUtf16, 16) == 2 && aUtf16[0] == 0x20AC && aUtf16[1] == 0xE9);
    BOOL bDefault = FALSE;
    aUtf16[2] = 0x4E2D;  // not in 1252
    CHECK(WideCharToMultiByte(CP_ACP, 0, aUtf16, 3, aBack, sizeof(aBack), nullptr, &bDefault) == 3 && aBack[0] == '\x80' &&
          aBack[1] == '\xE9' && aBack[2] == '?' && bDefault);
    CHECK(MultiByteToWideChar(CP_UTF8, 8 /* MB_ERR_INVALID_CHARS */, "\xC0\x80", 2, aUtf16, 16) == 0 &&
          GetLastError() == 1113);
    CHECK(MultiByteToWideChar(CP_ACP, 0, "abc", -1, aUtf16, 2) == 0 && GetLastError() == 122);

    char szCase[] = "MiXeD \xC9";
    CHECK(CharLowerBuffA(szCase, 7) == 7 && strcmp(szCase, "mixed \xE9") == 0);
    CHECK(strcmp(_itoa(-42, sz, 10), "-42") == 0 && strcmp(_itoa(-1, sz, 16), "ffffffff") == 0);
    CHECK(strcmp(_ultoa(0xFFFFFFFFul, sz, 10), "4294967295") == 0 && strcmp(_i64toa(-9000000000LL, sz, 10), "-9000000000") == 0);
    CHECK(strcmp(_ui64toa(255, sz, 2), "11111111") == 0);
    char szWord[] = "Hello";
    CHECK(strcmp(_strupr(szWord), "HELLO") == 0 && strcmp(_strlwr(szWord), "hello") == 0 &&
          strcmp(_strrev(szWord), "olleh") == 0);
    char szLang[16];
    CHECK(GetLocaleInfoA(0x409, 3, szLang, sizeof(szLang)) == 4 && strcmp(szLang, "ENU") == 0);
    CHECK(GetLocaleInfoA(0x409, 0x1002, szLang, 0) == 14);
    CHECK(GetACP() == 1252);
}

void TestProcess() {
    HMODULE hMain = GetModuleHandleA(nullptr);
    CHECK(hMain && GetModuleHandleA("game.exe") == hMain);
    CHECK(GetProcAddress(hMain, "GetTickCount") == (FARPROC)&GetTickCount);
    CHECK(LoadLibraryA("advapi32.dll") == nullptr && GetLastError() == 126);
    CHECK(LoadLibraryA("NoSuch.dll") == nullptr);
    HMODULE hSystem = LoadLibraryA("/usr/lib/libSystem.B.dylib");
    CHECK(hSystem && GetProcAddress(hSystem, "getpid") != nullptr && GetProcAddress(hSystem, "NoSuchCall") == nullptr);
    CHECK(FreeLibrary(hSystem));
    char szName[64];
    DWORD n = sizeof(szName);
    CHECK(GetComputerNameA(szName, &n) && n == strlen(szName) && n <= 15);
    n = sizeof(szName);
    CHECK(GetUserNameA(szName, &n) && n == strlen(szName) + 1);
    CHECK(GetCurrentProcessId() == (DWORD)getpid());
    DWORD dwCode;
    CHECK(GetExitCodeProcess(GetCurrentProcess(), &dwCode) && dwCode == 0x103);
    CHECK((GetVersion() & 0xFF) == 6 && !(GetVersion() & 0x80000000));
    CHECK(strstr(GetCommandLineA(), "Game.exe") != nullptr);
    CHECK(FindResourceA(hMain, "X", "Y") == nullptr);
    char szText[128];
    CHECK(FormatMessageA(0x1000, nullptr, 2, 0, szText, sizeof(szText), nullptr) > 0 &&
          strstr(szText, "cannot find the file"));
    CHECK(SetThreadPriority(GetCurrentThread(), 1) && GetThreadPriority(GetCurrentThread()) == 1);
}

DWORD PortWorker(LPVOID pArg) {
    DWORD dwBytes = 0;
    ULONG_PTR nKey = 0;
    LPOVERLAPPED pOverlapped = nullptr;
    if (!GetQueuedCompletionStatus((HANDLE)pArg, &dwBytes, &nKey, &pOverlapped, 5000)) {
        return 0;
    }
    return dwBytes == 512 && nKey == 7 ? 1 : 2;
}

void TestCompletionPort() {
    HANDLE hPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    CHECK(hPort != nullptr);
    DWORD dwBytes;
    ULONG_PTR nKey;
    LPOVERLAPPED pOverlapped = (LPOVERLAPPED)1;
    CHECK(!GetQueuedCompletionStatus(hPort, &dwBytes, &nKey, &pOverlapped, 10) && pOverlapped == nullptr &&
          GetLastError() == WAIT_TIMEOUT);
    HANDLE hThread = CreateThread(nullptr, 0, PortWorker, hPort, 0, nullptr);
    Sleep(20);
    CHECK(PostQueuedCompletionStatus(hPort, 512, 7, nullptr));
    CHECK(WaitForSingleObject(hThread, 5000) == WAIT_OBJECT_0);
    DWORD dwExit = 0;
    CHECK(GetExitCodeProcess(hThread, &dwExit) && dwExit == 1);
    CloseHandle(hThread);
    CloseHandle(hPort);
}

}  // namespace

int main() {
    char szTemplate[] = "/tmp/win32posix-test-XXXXXX";
    char* szDir = mkdtemp(szTemplate);
    if (!szDir) {
        perror("mkdtemp");
        return 2;
    }
    char szReal[PATH_MAX];
    realpath(szDir, szReal);
    setenv("D2_INSTALL_DIR", szReal, 1);
    setenv("D2_REGISTRY", (std::string(szReal) + "/Registry.ini").c_str(), 1);
    g_sDir = "C:" + std::string(szReal) + "/";
    for (char& c : g_sDir) {
        if (c == '/') {
            c = '\\';
        }
    }
    TestFiles();
    TestFind();
    TestEvents();
    TestThreads();
    TestClocks();
    TestProfile();
    TestRegistry();
    TestMemory();
    TestMapping();
    TestStrings();
    TestProcess();
    TestCompletionPort();
    printf("win32posix kernel: %d checks, %d failed (%s)\n", g_nChecks, g_nFailures, szReal);
    if (g_nFailures == 0) {
        std::string sCommand = "rm -rf '" + std::string(szReal) + "'";
        if (system(sCommand.c_str()) != 0) {
            fprintf(stderr, "could not remove %s\n", szReal);
        }
    }
    return g_nFailures ? 1 : 0;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
