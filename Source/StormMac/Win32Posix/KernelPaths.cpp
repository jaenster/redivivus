// Win32Posix: the path mapping, the install and MPQ directories, the module file name, the
// temporary files and _fullpath (Win32Mac's Win32_Paths.cpp, the temp-path functions of
// Win32_Process.cpp and the path cache of Win32_utils.cpp).
//
// Win32Mac hands the game Mac path tokens ("\{MacPathNN}\") and resolves them through the File
// Manager. Here the game sees the POSIX path itself, Windows-spelled: /Users/x/f is
// "C:\Users\x\f" (see Win32PosixKernelPriv.h). The process's module is Game.exe in the
// install directory, so the game finds its data beside "itself" as on Windows.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

namespace Win32Posix {

std::string HomeDir() {
    const char* szHome = getenv("HOME");
    if (szHome && *szHome) {
        return szHome;
    }
    struct passwd* pEntry = getpwuid(getuid());
    return pEntry && pEntry->pw_dir ? pEntry->pw_dir : "/tmp";
}

std::string AppSupportDir() {
#if defined(__APPLE__)
    return HomeDir() + "/Library/Application Support";
#else
    const char* szData = getenv("XDG_DATA_HOME");
    return szData && *szData ? std::string(szData) : HomeDir() + "/.local/share";
#endif
}

namespace {

void MakeDirs(const std::string& sPath) {
    for (size_t i = 1; i <= sPath.size(); ++i) {
        if (i == sPath.size() || sPath[i] == '/') {
            mkdir(sPath.substr(0, i).c_str(), 0755);
        }
    }
}

std::string StripTrailingSlash(std::string s) {
    while (s.size() > 1 && s.back() == '/') {
        s.pop_back();
    }
    return s;
}

bool Exists(const std::string& sPath) {
    struct stat tStat;
    return lstat(sPath.c_str(), &tStat) == 0;
}

// The entry of the directory whose name matches without case; empty when none does.
std::string FindNoCase(const std::string& sDir, const std::string& sName) {
    DIR* pDir = opendir(sDir.empty() ? "/" : sDir.c_str());
    if (!pDir) {
        return std::string();
    }
    std::string sFound;
    while (struct dirent* pEntry = readdir(pDir)) {
        if (strcasecmp(pEntry->d_name, sName.c_str()) == 0) {
            sFound = pEntry->d_name;
            break;
        }
    }
    closedir(pDir);
    return sFound;
}

// The components of an absolute POSIX path, with "." and ".." applied.
std::vector<std::string> Components(const std::string& sPath) {
    std::vector<std::string> aParts;
    size_t nStart = 0;
    while (nStart <= sPath.size()) {
        size_t nEnd = sPath.find('/', nStart);
        if (nEnd == std::string::npos) {
            nEnd = sPath.size();
        }
        std::string sPart = sPath.substr(nStart, nEnd - nStart);
        if (sPart == "..") {
            if (!aParts.empty()) {
                aParts.pop_back();
            }
        } else if (!sPart.empty() && sPart != ".") {
            aParts.push_back(sPart);
        }
        nStart = nEnd + 1;
    }
    return aParts;
}

}  // namespace

std::string Path_ToPosix(const char* szWinPath) {
    std::string s = szWinPath ? szWinPath : "";
    for (char& c : s) {
        if (c == '\\') {
            c = '/';
        }
    }
    if (s.size() >= 2 && s[1] == ':' && ((s[0] | 0x20) >= 'a' && (s[0] | 0x20) <= 'z')) {
        s = s.substr(2);
        if (s.empty() || s[0] != '/') {
            s = "/" + s;
        }
    } else if (s.empty() || s[0] != '/') {
        s = InstallDir() + "/" + s;
    }
    bool bTrailingSlash = !s.empty() && s.back() == '/';
    std::vector<std::string> aParts = Components(s);
    std::string sOut;
    bool bResolving = true;
    for (size_t i = 0; i < aParts.size(); ++i) {
        std::string sNext = sOut + "/" + aParts[i];
        if (bResolving && !Exists(sNext)) {
            std::string sMatch = FindNoCase(sOut, aParts[i]);
            if (!sMatch.empty()) {
                sNext = sOut + "/" + sMatch;
            } else {
                bResolving = false;  // the rest does not exist under any spelling
            }
        }
        sOut = sNext;
    }
    if (sOut.empty()) {
        sOut = "/";
    } else if (bTrailingSlash) {
        sOut += "/";
    }
    return sOut;
}

std::string Path_ToPosixForRead(const char* szWinPath) {
    std::string sPath = Path_ToPosix(szWinPath);
    if (Exists(sPath) || MpqDir() == InstallDir()) {
        return sPath;
    }
    size_t nSlash = sPath.rfind('/');
    std::string sDir = sPath.substr(0, nSlash);
    std::string sLeaf = sPath.substr(nSlash + 1);
    if (sDir != InstallDir() || sLeaf.size() < 4 || strcasecmp(sLeaf.c_str() + sLeaf.size() - 4, ".mpq") != 0) {
        return sPath;
    }
    std::string sMatch = FindNoCase(MpqDir(), sLeaf);
    return MpqDir() + "/" + (sMatch.empty() ? sLeaf : sMatch);
}

std::string Path_ToWindows(const std::string& sPosixPath) {
    std::string s = "C:" + sPosixPath;
    for (char& c : s) {
        if (c == '/') {
            c = '\\';
        }
    }
    return s;
}

const std::string& InstallDir() {
    static const std::string sDir = [] {
        const char* szEnv = getenv("D2_INSTALL_DIR");
        std::string s = szEnv && *szEnv ? StripTrailingSlash(szEnv)
                                        : AppSupportDir() + "/Diablo II";
        MakeDirs(s);
        char szReal[PATH_MAX];
        return std::string(realpath(s.c_str(), szReal) ? szReal : s.c_str());
    }();
    return sDir;
}

const std::string& MpqDir() {
    static const std::string sDir = [] {
        const char* szEnv = getenv("D2_MPQ_DIR");
        std::string s;
        if (szEnv && *szEnv) {
            s = szEnv;
        } else {
            char szValue[MAX_PATH];
            std::string sIni = Path_ToWindows(InstallDir() + "/Win32Posix.ini");
            if (GetPrivateProfileStringA("Paths", "MpqDir", "", szValue, sizeof(szValue), sIni.c_str()) && *szValue) {
                s = szValue[0] == '/' ? std::string(szValue) : Path_ToPosix(szValue);
            }
        }
        if (s.empty()) {
            return InstallDir();
        }
        s = StripTrailingSlash(s);
        char szReal[PATH_MAX];
        return std::string(realpath(s.c_str(), szReal) ? szReal : s.c_str());
    }();
    return sDir;
}

}  // namespace Win32Posix

using namespace Win32Posix;

namespace {

// Copies a string out the way the Win32 path calls do: the length without the terminator on
// success, the size the buffer needs (with it) when it is too small.
DWORD CopyPathOut(const std::string& s, LPSTR lpBuffer, DWORD nBufferLength) {
    if (lpBuffer == nullptr || nBufferLength <= s.size()) {
        return (DWORD)s.size() + 1;
    }
    memcpy(lpBuffer, s.c_str(), s.size() + 1);
    return (DWORD)s.size();
}

}  // namespace

DWORD GetTempPathA(DWORD nBufferLength, LPSTR lpBuffer) {
    const char* szTmp = getenv("TMPDIR");
    std::string sDir = StripTrailingSlash(szTmp && *szTmp ? szTmp : "/tmp");
    char szReal[PATH_MAX];
    if (realpath(sDir.c_str(), szReal)) {
        sDir = szReal;
    }
    return CopyPathOut(Path_ToWindows(sDir) + "\\", lpBuffer, nBufferLength);
}

UINT GetTempFileNameA(LPCSTR lpPathName, LPCSTR lpPrefixString, UINT uUnique, LPSTR lpTempFileName) {
    std::string sDir = lpPathName ? lpPathName : "";
    if (!sDir.empty() && sDir.back() != '\\' && sDir.back() != '/') {
        sDir += "\\";
    }
    std::string sPrefix = lpPrefixString ? std::string(lpPrefixString).substr(0, 3) : std::string();
    UINT uValue = uUnique & 0xFFFF;
    bool bCreate = uValue == 0;
    if (bCreate) {
        uValue = (UINT)(GetTickCount() & 0xFFFF);
    }
    for (int nTry = 0; nTry < 0x10000; ++nTry) {
        if (uValue == 0) {
            uValue = 1;
        }
        char szName[16];
        snprintf(szName, sizeof(szName), "%X.TMP", uValue);
        std::string sWin = sDir + sPrefix + szName;
        if (sWin.size() >= MAX_PATH) {
            SetError(206);
            return 0;
        }
        if (!bCreate) {
            memcpy(lpTempFileName, sWin.c_str(), sWin.size() + 1);
            return uValue;
        }
        std::string sPosix = Path_ToPosix(sWin.c_str());
        int fd = open(sPosix.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0644);
        if (fd >= 0) {
            close(fd);
            memcpy(lpTempFileName, sWin.c_str(), sWin.size() + 1);
            return uValue;
        }
        if (errno != EEXIST) {
            SetError(ErrorFromErrno(errno));
            return 0;
        }
        uValue = (uValue + 1) & 0xFFFF;
    }
    SetError(80);  // ERROR_FILE_EXISTS
    return 0;
}

char* _fullpath(char* szAbsPath, const char* szRelPath, size_t nMaxLength) {
    std::string sWin = Path_ToWindows(Path_ToPosix(szRelPath && *szRelPath ? szRelPath : "."));
    if (szAbsPath == nullptr) {
        return strdup(sWin.c_str());
    }
    if (sWin.size() >= nMaxLength) {
        errno = ERANGE;
        return nullptr;
    }
    memcpy(szAbsPath, sWin.c_str(), sWin.size() + 1);
    return szAbsPath;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
