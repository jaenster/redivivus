// Win32Posix: the process, modules, the system and user queries, the locale, messages and
// resources (Win32Mac's Win32_Process.cpp, Win32_Resource.cpp and the rest of Win32_utils.cpp).
//
// Modules: the process is Game.exe in the install directory. LoadLibrary refuses the Windows
// system DLLs and the game's own Windows-only helpers (Keyhook.dll, glide3x.dll) - the game asks for them only to reach optional calls (dbghelp, psapi, the
// CryptoAPI in advapi32) and takes its fallback without them, as on Win32Mac, whose advapi32
// answers no procedure either. Any other name is tried with dlopen. GetProcAddress is dlsym.
//
// The locale is the user's (LC_ALL, LC_MESSAGES, LANG), as a Windows LCID; the layer's ANSI code
// page is 1252 in every locale (KernelString.cpp). The executable carries no PE resources, so
// the resource calls find nothing.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <crt_externs.h>
#include <ctype.h>
#include <dlfcn.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#if defined(__APPLE__)
#include <sys/sysctl.h>
#include <mach/mach.h>
#endif

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

using namespace Win32Posix;

namespace {

struct ModuleObject : Object {
    ModuleObject() : Object(OBJECT_MODULE) {}
    void* pLibrary = nullptr;
    std::string sPath;  // POSIX
    int nRefs = 1;
    bool bMain = false;
};

std::mutex& ModuleLock() {
    static std::mutex* pLock = new std::mutex;
    return *pLock;
}

std::vector<HANDLE>& Modules() {
    static auto* pModules = new std::vector<HANDLE>;
    return *pModules;
}

std::string ModuleLeaf(const std::string& s) {
    size_t n = s.find_last_of("\\/");
    return n == std::string::npos ? s : s.substr(n + 1);
}

HANDLE MainModule() {
    static HANDLE hMain = [] {
        auto pModule = std::make_shared<ModuleObject>();
        pModule->pLibrary = dlopen(nullptr, RTLD_NOW);
        pModule->sPath = InstallDir() + "/Game.exe";
        pModule->bMain = true;
        HANDLE h = Handle_Insert(pModule);
        std::lock_guard<std::mutex> tGuard(ModuleLock());
        Modules().push_back(h);
        return h;
    }();
    return hMain;
}

// The loaded module of that file name, the module lock held; 0 when none is.
HANDLE FindModule(const char* szName) {
    std::string sLeaf = ModuleLeaf(szName);
    for (HANDLE h : Modules()) {
        auto pModule = Handle_GetAs<ModuleObject>(h, OBJECT_MODULE);
        if (pModule && strcasecmp(ModuleLeaf(pModule->sPath).c_str(), sLeaf.c_str()) == 0) {
            return h;
        }
    }
    return 0;
}

bool IsWindowsSystemDll(const char* szName) {
    static const char* const kSystem[] = {
        "kernel32", "user32",  "gdi32",   "advapi32", "shell32",  "ole32",   "oleaut32", "ws2_32",  "wsock32",
        "dsound",   "ddraw",   "d3d8",    "d3d9",     "opengl32", "glide3x", "winmm",    "imm32",   "version",
        "dbghelp",  "imagehlp", "psapi",  "ntdll",    "shfolder", "comctl32", "wintrust", "crypt32", "keyhook",
    };
    std::string sLeaf = ModuleLeaf(szName);
    size_t nDot = sLeaf.rfind('.');
    if (nDot != std::string::npos) {
        sLeaf.resize(nDot);
    }
    for (const char* szSystem : kSystem) {
        if (strcasecmp(sLeaf.c_str(), szSystem) == 0) {
            return true;
        }
    }
    return false;
}

}  // namespace

extern "C" HMODULE LoadLibraryA(LPCSTR lpLibFileName) {
    if (!lpLibFileName || !*lpLibFileName) {
        SetError(87);
        return nullptr;
    }
    MainModule();
    {
        std::lock_guard<std::mutex> tGuard(ModuleLock());
        if (HANDLE h = FindModule(lpLibFileName)) {
            Handle_GetAs<ModuleObject>(h, OBJECT_MODULE)->nRefs++;
            return (HMODULE)h;
        }
    }
    if (IsWindowsSystemDll(lpLibFileName)) {
        SetError(126);  // ERROR_MOD_NOT_FOUND
        return nullptr;
    }
    std::string sPath = Path_ToPosixForRead(lpLibFileName);
    void* pLibrary = dlopen(sPath.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!pLibrary) {
        SetError(126);
        return nullptr;
    }
    auto pModule = std::make_shared<ModuleObject>();
    pModule->pLibrary = pLibrary;
    pModule->sPath = sPath;
    HANDLE h = Handle_Insert(pModule);
    if (!h) {
        dlclose(pLibrary);
        return nullptr;
    }
    std::lock_guard<std::mutex> tGuard(ModuleLock());
    Modules().push_back(h);
    return (HMODULE)h;
}

extern "C" BOOL FreeLibrary(HMODULE hLibModule) {
    auto pModule = Handle_GetAs<ModuleObject>((HANDLE)hLibModule, OBJECT_MODULE);
    if (!pModule) {
        SetError(6);
        return FALSE;
    }
    if (pModule->bMain) {
        return TRUE;
    }
    std::lock_guard<std::mutex> tGuard(ModuleLock());
    if (--pModule->nRefs == 0) {
        auto& aModules = Modules();
        for (size_t i = 0; i < aModules.size(); ++i) {
            if (aModules[i] == (HANDLE)hLibModule) {
                aModules.erase(aModules.begin() + (long)i);
                break;
            }
        }
        dlclose(pModule->pLibrary);
        Handle_Remove((HANDLE)hLibModule);
    }
    return TRUE;
}

extern "C" HMODULE GetModuleHandleA(LPCSTR lpModuleName) {
    HANDLE hMain = MainModule();
    if (!lpModuleName) {
        return (HMODULE)hMain;
    }
    std::lock_guard<std::mutex> tGuard(ModuleLock());
    if (HANDLE h = FindModule(lpModuleName)) {
        return (HMODULE)h;
    }
    SetError(126);
    return nullptr;
}

extern "C" FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
    auto pModule = Handle_GetAs<ModuleObject>((HANDLE)hModule, OBJECT_MODULE);
    if (!pModule) {
        SetError(6);
        return nullptr;
    }
    if ((uintptr_t)lpProcName < 0x10000) {
        SetError(127);  // an ordinal: no export table to look it up in
        return nullptr;
    }
    FARPROC pfn = (FARPROC)dlsym(pModule->pLibrary, lpProcName);
    if (!pfn) {
        SetError(127);  // ERROR_PROC_NOT_FOUND
    }
    return pfn;
}

DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize) {
    auto pModule = Handle_GetAs<ModuleObject>(hModule ? (HANDLE)hModule : MainModule(), OBJECT_MODULE);
    if (!pModule) {
        SetError(6);
        return 0;
    }
    std::string sPath = Path_ToWindows(pModule->sPath);
    if (nSize == 0) {
        SetError(122);
        return 0;
    }
    if (sPath.size() >= nSize) {
        memcpy(lpFilename, sPath.c_str(), nSize - 1);
        lpFilename[nSize - 1] = '\0';
        SetError(122);  // ERROR_INSUFFICIENT_BUFFER
        return nSize;
    }
    memcpy(lpFilename, sPath.c_str(), sPath.size() + 1);
    SetError(0);
    return (DWORD)sPath.size();
}

// ---- the process ----

HANDLE GetCurrentProcess() {
    return (HANDLE)(intptr_t)-1;
}

extern "C" DWORD GetCurrentProcessId() {
    return (DWORD)getpid();
}

extern "C" void ExitProcess(UINT uExitCode) {
    exit((int)uExitCode);
}

BOOL TerminateProcess(HANDLE hProcess, UINT uExitCode) {
    if (hProcess != GetCurrentProcess()) {
        SetError(5);  // no other process is the layer's to end
        return FALSE;
    }
    _exit((int)uExitCode);
}

BOOL IsDebuggerPresent(void) {
#if defined(__APPLE__)
    // The kernel's process record says whether the process is traced (P_TRACED).
    int aMib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()};
    struct kinfo_proc tInfo;
    size_t nSize = sizeof(tInfo);
    memset(&tInfo, 0, sizeof(tInfo));
    if (sysctl(aMib, sizeof(aMib) / sizeof(aMib[0]), &tInfo, &nSize, nullptr, 0) != 0) {
        return FALSE;
    }
    return (tInfo.kp_proc.p_flag & P_TRACED) != 0;
#else
    // Linux: the tracer's pid in /proc/self/status, 0 when none.
    static const char szTracerKey[] = "TracerPid:";
    FILE* pStatus = fopen("/proc/self/status", "r");
    char szLine[256];
    BOOL bTraced = FALSE;
    while (pStatus && fgets(szLine, sizeof(szLine), pStatus)) {
        if (strncmp(szLine, szTracerKey, sizeof(szTracerKey) - 1) == 0) {
            bTraced = atoi(szLine + sizeof(szTracerKey) - 1) != 0;
            break;
        }
    }
    if (pStatus) {
        fclose(pStatus);
    }
    return bTraced;
#endif
}

BOOL GetExitCodeProcess(HANDLE hProcess, LPDWORD lpExitCode) {
    if (hProcess == GetCurrentProcess()) {
        *lpExitCode = 0x103;  // STILL_ACTIVE
        return TRUE;
    }
    if (Thread_ExitCode(hProcess, lpExitCode)) {
        return TRUE;
    }
    SetError(6);
    return FALSE;
}

LPSTR GetCommandLineA(void) {
    static std::string sLine = [] {
        std::string s = "\"" + Path_ToWindows(InstallDir() + "/Game.exe") + "\"";
        int nArgs = *_NSGetArgc();
        char** aszArgs = *_NSGetArgv();
        for (int i = 1; i < nArgs; ++i) {
            bool bQuote = strchr(aszArgs[i], ' ') != nullptr || strchr(aszArgs[i], '\t') != nullptr;
            s += bQuote ? std::string(" \"") + aszArgs[i] + "\"" : std::string(" ") + aszArgs[i];
        }
        return s;
    }();
    return &sLine[0];
}

extern "C" void OutputDebugStringA(LPCSTR lpOutputString) {
    if (lpOutputString) {
        fputs(lpOutputString, stderr);
    }
}

// ---- the system and the user ----

BOOL GetComputerNameA(LPSTR lpBuffer, LPDWORD nSize) {
    char szHost[256] = {};
    gethostname(szHost, sizeof(szHost) - 1);
    std::string sName(szHost, strcspn(szHost, "."));
    if (sName.size() > 15) {
        sName.resize(15);  // MAX_COMPUTERNAME_LENGTH
    }
    for (char& c : sName) {
        c = (char)toupper((unsigned char)c);
    }
    if (*nSize <= sName.size()) {
        *nSize = (DWORD)sName.size() + 1;
        SetError(111);  // ERROR_BUFFER_OVERFLOW
        return FALSE;
    }
    memcpy(lpBuffer, sName.c_str(), sName.size() + 1);
    *nSize = (DWORD)sName.size();
    return TRUE;
}

BOOL GetUserNameA(LPSTR lpBuffer, LPDWORD pcbBuffer) {
    struct passwd* pEntry = getpwuid(getuid());
    const char* szUser = pEntry && pEntry->pw_name ? pEntry->pw_name : getenv("USER");
    std::string sUser = szUser ? szUser : "Player";
    if (*pcbBuffer <= sUser.size()) {
        *pcbBuffer = (DWORD)sUser.size() + 1;
        SetError(122);
        return FALSE;
    }
    memcpy(lpBuffer, sUser.c_str(), sUser.size() + 1);
    *pcbBuffer = (DWORD)sUser.size() + 1;
    return TRUE;
}

// Windows 7 SP1 (6.1.7601), an NT platform: the version the game's version checks were last
// tuned to, and new enough for every path they take.
DWORD GetVersion(void) {
    return (7601u << 16) | (1u << 8) | 6u;
}

// The page is the 4 KB one the shared code is written for (KernelMemory.cpp commits by it);
// the allocation granularity is Windows' 64 KB.
void GetSystemInfo(struct _SYSTEM_INFO* lpSystemInfo) {
    memset(lpSystemInfo, 0, sizeof(*lpSystemInfo));
#if D2_ARCH_ARM64
    lpSystemInfo->wProcessorArchitecture = 12;  // PROCESSOR_ARCHITECTURE_ARM64
#else
    lpSystemInfo->wProcessorArchitecture = 9;  // PROCESSOR_ARCHITECTURE_AMD64
    lpSystemInfo->dwProcessorType = 8664;      // PROCESSOR_AMD_X8664
#endif
    lpSystemInfo->dwPageSize = 0x1000;
    lpSystemInfo->lpMinimumApplicationAddress = (LPVOID)0x10000;
    lpSystemInfo->lpMaximumApplicationAddress = (LPVOID)0x7FFFFFFEFFFFULL;
    long nCpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (nCpus < 1) {
        nCpus = 1;
    }
    if (nCpus > 64) {
        nCpus = 64;
    }
    lpSystemInfo->dwNumberOfProcessors = (DWORD)nCpus;
    lpSystemInfo->dwActiveProcessorMask = nCpus >= 64 ? ~(DWORD_PTR)0 : (((DWORD_PTR)1 << nCpus) - 1);
    lpSystemInfo->dwAllocationGranularity = 0x10000;
    lpSystemInfo->wProcessorLevel = 6;
}

// Physical memory from the kernel; the page file is the swap usage sysctl reports, the virtual
// range the one GetSystemInfo hands out. Fields are SIZE_T, so nothing saturates on 64-bit.
void GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer) {
    memset(lpBuffer, 0, sizeof(*lpBuffer));
    lpBuffer->dwLength = sizeof(*lpBuffer);
    uint64_t nTotal = 0;
    size_t nSize = sizeof(nTotal);
    if (sysctlbyname("hw.memsize", &nTotal, &nSize, nullptr, 0) != 0) {
        nTotal = 0;
    }
    vm_statistics64_data_t tStats;
    mach_msg_type_number_t nCount = HOST_VM_INFO64_COUNT;
    uint64_t nAvail = 0;
    if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&tStats, &nCount) == KERN_SUCCESS) {
        nAvail = ((uint64_t)tStats.free_count + tStats.inactive_count + tStats.purgeable_count) * (uint64_t)vm_page_size;
    }
    if (nAvail > nTotal) {
        nAvail = nTotal;
    }
    struct xsw_usage tSwap;
    nSize = sizeof(tSwap);
    if (sysctlbyname("vm.swapusage", &tSwap, &nSize, nullptr, 0) != 0) {
        memset(&tSwap, 0, sizeof(tSwap));
    }
    lpBuffer->dwMemoryLoad = nTotal ? (DWORD)((nTotal - nAvail) * 100 / nTotal) : 0;
    lpBuffer->dwTotalPhys = (SIZE_T)nTotal;
    lpBuffer->dwAvailPhys = (SIZE_T)nAvail;
    lpBuffer->dwTotalPageFile = (SIZE_T)(nTotal + tSwap.xsu_total);
    lpBuffer->dwAvailPageFile = (SIZE_T)(nAvail + tSwap.xsu_avail);
    lpBuffer->dwTotalVirtual = (SIZE_T)(0x7FFFFFFEFFFFULL - 0x10000);
    lpBuffer->dwAvailVirtual = lpBuffer->dwTotalVirtual;
}

// ---- messages and resources ----

DWORD FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer,
                     DWORD nSize, va_list* Arguments) {
    (void)dwLanguageId;
    (void)Arguments;  // inserts are left as written: the game passes FORMAT_MESSAGE_IGNORE_INSERTS
    std::string sText;
    if (dwFlags & 0x400) {  // FORMAT_MESSAGE_FROM_STRING
        sText = lpSource ? (const char*)lpSource : "";
    } else if (dwFlags & 0x1000) {  // FORMAT_MESSAGE_FROM_SYSTEM
        static const struct {
            DWORD dwId;
            const char* szText;
        } kMessages[] = {
            {0, "The operation completed successfully."},
            {2, "The system cannot find the file specified."},
            {3, "The system cannot find the path specified."},
            {4, "The system cannot open the file."},
            {5, "Access is denied."},
            {6, "The handle is invalid."},
            {8, "Not enough memory resources are available to process this command."},
            {18, "There are no more files."},
            {80, "The file exists."},
            {87, "The parameter is incorrect."},
            {112, "There is not enough space on the disk."},
            {122, "The data area passed to a system call is too small."},
            {126, "The specified module could not be found."},
            {127, "The specified procedure could not be found."},
            {183, "Cannot create a file when that file already exists."},
            {487, "Attempt to access invalid address."},
        };
        for (const auto& m : kMessages) {
            if (m.dwId == dwMessageId) {
                sText = std::string(m.szText) + "\r\n";
            }
        }
        if (sText.empty()) {
            SetError(317);  // ERROR_MR_MID_NOT_FOUND
            return 0;
        }
    } else {
        SetError(1813);  // FORMAT_MESSAGE_FROM_HMODULE: no module has a message table here
        return 0;
    }
    if (dwFlags & 0x100) {  // FORMAT_MESSAGE_ALLOCATE_BUFFER: the caller frees it with LocalFree
        char* pOut = strdup(sText.c_str());
        *(char**)lpBuffer = pOut;
        return (DWORD)sText.size();
    }
    if (nSize <= sText.size()) {
        SetError(122);
        return 0;
    }
    memcpy(lpBuffer, sText.c_str(), sText.size() + 1);
    return (DWORD)sText.size();
}

HRSRC FindResourceA(HMODULE hModule, LPCSTR lpName, LPCSTR lpType) {
    (void)hModule;
    (void)lpName;
    (void)lpType;
    SetError(1813);  // ERROR_RESOURCE_TYPE_NOT_FOUND
    return nullptr;
}

HGLOBAL LoadResource(HMODULE hModule, HRSRC hResInfo) {
    (void)hModule;
    (void)hResInfo;
    SetError(1812);  // ERROR_RESOURCE_DATA_NOT_FOUND
    return nullptr;
}

LPVOID LockResource(HGLOBAL hResData) {
    return hResData;
}

DWORD SizeofResource(HMODULE hModule, HRSRC hResInfo) {
    (void)hModule;
    (void)hResInfo;
    SetError(1812);
    return 0;
}

BOOL FreeResource(HGLOBAL hResData) {
    (void)hResData;
    return FALSE;
}

int LoadStringA(HINSTANCE hInstance, UINT uID, LPSTR lpBuffer, int cchBufferMax) {
    (void)hInstance;
    (void)uID;
    if (lpBuffer && cchBufferMax > 0) {
        lpBuffer[0] = '\0';
    }
    SetError(1814);  // ERROR_RESOURCE_NAME_NOT_FOUND
    return 0;
}

// ---- the locale ----

namespace {

struct LocaleEntry {
    const char* szPosix;  // language_TERRITORY
    LCID lcid;
    const char* szAbbrevLang;
    const char* szEnglishLang;
    const char* szEnglishCountry;
    const char* szAbbrevCountry;
    const char* szCountryCode;
};

const LocaleEntry kLocales[] = {
    {"en_US", 0x0409, "ENU", "English", "United States", "USA", "1"},
    {"en_GB", 0x0809, "ENG", "English", "United Kingdom", "GBR", "44"},
    {"de_DE", 0x0407, "DEU", "German", "Germany", "DEU", "49"},
    {"fr_FR", 0x040C, "FRA", "French", "France", "FRA", "33"},
    {"es_ES", 0x0C0A, "ESN", "Spanish", "Spain", "ESP", "34"},
    {"it_IT", 0x0410, "ITA", "Italian", "Italy", "ITA", "39"},
    {"nl_NL", 0x0413, "NLD", "Dutch", "Netherlands", "NLD", "31"},
    {"pt_BR", 0x0416, "PTB", "Portuguese", "Brazil", "BRA", "55"},
    {"pl_PL", 0x0415, "PLK", "Polish", "Poland", "POL", "48"},
    {"ru_RU", 0x0419, "RUS", "Russian", "Russia", "RUS", "7"},
    {"ja_JP", 0x0411, "JPN", "Japanese", "Japan", "JPN", "81"},
    {"ko_KR", 0x0412, "KOR", "Korean", "Korea", "KOR", "82"},
    {"zh_TW", 0x0404, "CHT", "Chinese", "Taiwan", "TWN", "886"},
    {"zh_CN", 0x0804, "CHS", "Chinese", "China", "CHN", "86"},
};

const LocaleEntry& UserLocale() {
    static const LocaleEntry* pEntry = [] {
        const char* szLocale = nullptr;
        for (const char* szVar : {"LC_ALL", "LC_MESSAGES", "LANG"}) {
            const char* sz = getenv(szVar);
            if (sz && *sz && strcmp(sz, "C") != 0 && strcmp(sz, "POSIX") != 0) {
                szLocale = sz;
                break;
            }
        }
        if (szLocale) {
            for (const LocaleEntry& e : kLocales) {
                if (strncasecmp(szLocale, e.szPosix, 5) == 0) {
                    return &e;
                }
            }
            for (const LocaleEntry& e : kLocales) {
                if (strncasecmp(szLocale, e.szPosix, 2) == 0) {
                    return &e;
                }
            }
        }
        return &kLocales[0];
    }();
    return *pEntry;
}

const LocaleEntry& LocaleOf(LCID Locale) {
    for (const LocaleEntry& e : kLocales) {
        if (e.lcid == (Locale & 0xFFFF)) {
            return e;
        }
    }
    return UserLocale();  // LOCALE_USER_DEFAULT, LOCALE_SYSTEM_DEFAULT and the rest
}

}  // namespace

int GetLocaleInfoA(LCID Locale, LCTYPE LCType, LPSTR lpLCData, int cchData) {
    const LocaleEntry& e = LocaleOf(Locale);
    char szNumber[16];
    std::string sValue;
    switch (LCType & 0xFFFF) {
        case 0x0001: snprintf(szNumber, sizeof(szNumber), "%04x", e.lcid); sValue = szNumber; break;  // ILANGUAGE
        case 0x0002:                                                                              // SLANGUAGE
            sValue = std::string(e.szEnglishLang) + " (" + e.szEnglishCountry + ")";
            break;
        case 0x0003: sValue = e.szAbbrevLang; break;                       // SABBREVLANGNAME
        case 0x0005: sValue = e.szCountryCode; break;                      // ICOUNTRY
        case 0x0006:                                                       // SCOUNTRY
        case 0x1002: sValue = e.szEnglishCountry; break;                   // SENGCOUNTRY
        case 0x0007: sValue = e.szAbbrevCountry; break;                    // SABBREVCTRYNAME
        case 0x1001: sValue = e.szEnglishLang; break;                      // SENGLANGUAGE
        case 0x0059: sValue = std::string(e.szPosix, 2); break;            // SISO639LANGNAME
        case 0x005A: sValue = std::string(e.szPosix + 3, 2); break;        // SISO3166CTRYNAME
        case 0x000B: sValue = "437"; break;                                // IDEFAULTCODEPAGE
        case 0x1004: sValue = "1252"; break;                               // IDEFAULTANSICODEPAGE
        default: SetError(1004); return 0;                                 // ERROR_INVALID_FLAGS
    }
    int nNeeded = (int)sValue.size() + 1;
    if (cchData == 0) {
        return nNeeded;
    }
    if (cchData < nNeeded) {
        SetError(122);
        return 0;
    }
    memcpy(lpLCData, sValue.c_str(), (size_t)nNeeded);
    return nNeeded;
}

LCID GetUserDefaultLCID(void) {
    return UserLocale().lcid;
}

LANGID GetUserDefaultLangID(void) {
    return (LANGID)UserLocale().lcid;
}

LANGID GetSystemDefaultLangID(void) {
    return (LANGID)UserLocale().lcid;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
