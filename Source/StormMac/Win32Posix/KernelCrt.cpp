// Win32Posix: the Microsoft C runtime's path functions (fopen, stat, _access, _mkdir,
// _findfirst ...) over the kernel's path mapping; see Win32PosixCrt.h. Each call maps its path
// as the matching Win32 file call does, then makes the host's call on the POSIX path.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"
#include "Source/StormMac/Win32Posix/Win32PosixCrt.h"

using namespace Win32Posix;

namespace {

// Creates every missing directory above the file's leaf.
void MakeParents(const std::string& sPosixPath) {
    for (size_t i = 1; i < sPosixPath.size(); ++i) {
        if (sPosixPath[i] == '/') {
            mkdir(sPosixPath.substr(0, i).c_str(), 0755);
        }
    }
}

// The mode as the host takes it: the base letter and the plus. The Microsoft modes add
// t, b, c, n, N, S, R, T, D and ccs=..., and the host reads "at+" as append-only.
std::string HostMode(const char* szMode, bool* pbWrites) {
    std::string sOut;
    bool bPlus = false;
    char chBase = 'r';
    for (const char* p = szMode ? szMode : "r"; *p && *p != ','; ++p) {
        if ((*p == 'r' || *p == 'w' || *p == 'a') && sOut.empty()) {
            chBase = *p;
            sOut += *p;
        } else if (*p == '+') {
            bPlus = true;
        }
    }
    if (sOut.empty()) {
        sOut = "r";
    }
    if (bPlus) {
        sOut += '+';
    }
    *pbWrites = chBase != 'r' || bPlus;
    return sOut;
}

// The POSIX path for opening: a write maps as it is spelled, with its directories made; a
// read finds the game's archives in the MPQ directory too.
std::string OpenPath(const char* szPath, bool bWrites) {
    if (!bWrites) {
        return Path_ToPosixForRead(szPath);
    }
    std::string sPath = Path_ToPosix(szPath);
    MakeParents(sPath);
    return sPath;
}

time_t TimeOf(const FILETIME& tFileTime) {
    const uint64_t nTicks = ((uint64_t)tFileTime.dwHighDateTime << 32) | tFileTime.dwLowDateTime;
    if (nTicks < 116444736000000000ull) {
        return 0;
    }
    return (time_t)((nTicks - 116444736000000000ull) / 10000000ull);
}

}  // namespace

FILE* Win32Posix_fopen(const char* szPath, const char* szMode) {
    if (!szPath) {
        errno = EINVAL;
        return nullptr;
    }
    bool bWrites = false;
    std::string sMode = HostMode(szMode, &bWrites);
    return (fopen)(OpenPath(szPath, bWrites).c_str(), sMode.c_str());
}

FILE* Win32Posix_freopen(const char* szPath, const char* szMode, FILE* pStream) {
    if (!szPath) {
        errno = EINVAL;
        return nullptr;
    }
    bool bWrites = false;
    std::string sMode = HostMode(szMode, &bWrites);
    return (freopen)(OpenPath(szPath, bWrites).c_str(), sMode.c_str(), pStream);
}

FILE* _fsopen(const char* szPath, const char* szMode, int nShare) {
    (void)nShare;  // no other process opens the game's files
    return Win32Posix_fopen(szPath, szMode);
}

int _open(const char* szPath, int nFlags, int nMode) {
    int nHost = nFlags & 3;  // _O_RDONLY, _O_WRONLY and _O_RDWR are the host's 0, 1 and 2
    if (nFlags & _O_APPEND) {
        nHost |= O_APPEND;
    }
    if (nFlags & _O_CREAT) {
        nHost |= O_CREAT;
    }
    if (nFlags & _O_TRUNC) {
        nHost |= O_TRUNC;
    }
    if (nFlags & _O_EXCL) {
        nHost |= O_EXCL;
    }
    if (!szPath) {
        errno = EINVAL;
        return -1;
    }
    bool bWrites = (nHost & (3 | O_CREAT | O_TRUNC | O_APPEND)) != 0;
    return open(OpenPath(szPath, bWrites).c_str(), nHost, nMode ? nMode : 0644);
}

int _access(const char* szPath, int nMode) {
    if (!szPath) {
        errno = EINVAL;
        return -1;
    }
    return access(Path_ToPosixForRead(szPath).c_str(), nMode);
}

int Win32Posix_stat(const char* szPath, struct stat* pStat) {
    if (!szPath) {
        errno = EINVAL;
        return -1;
    }
    return (stat)(Path_ToPosixForRead(szPath).c_str(), pStat);
}

int _stat(const char* szPath, struct stat* pStat) {
    return Win32Posix_stat(szPath, pStat);
}

int _stat64(const char* szPath, struct stat* pStat) {
    return Win32Posix_stat(szPath, pStat);
}

int Win32Posix_rename(const char* szOld, const char* szNew) {
    if (!szOld || !szNew) {
        errno = EINVAL;
        return -1;
    }
    std::string sNew = Path_ToPosix(szNew);
    MakeParents(sNew);
    return (rename)(Path_ToPosix(szOld).c_str(), sNew.c_str());
}

int _mkdir(const char* szPath) {
    if (!szPath) {
        errno = EINVAL;
        return -1;
    }
    return mkdir(Path_ToPosix(szPath).c_str(), 0755);
}

int _rmdir(const char* szPath) {
    if (!szPath) {
        errno = EINVAL;
        return -1;
    }
    return rmdir(Path_ToPosix(szPath).c_str());
}

int _unlink(const char* szPath) {
    if (!szPath) {
        errno = EINVAL;
        return -1;
    }
    return unlink(Path_ToPosix(szPath).c_str());
}

int _chmod(const char* szPath, int nMode) {
    if (!szPath) {
        errno = EINVAL;
        return -1;
    }
    // _S_IWRITE (0x80) is the Microsoft CRT's write bit; the host's owner bits follow it.
    mode_t nHost = (nMode & 0x80) ? 0644 : 0444;
    return chmod(Path_ToPosix(szPath).c_str(), nHost);
}

// The game's current directory is the install directory, which a relative path is taken
// against: _chdir checks that its argument is a directory and leaves the process where it is.
int _chdir(const char* szPath) {
    if (!szPath) {
        errno = EINVAL;
        return -1;
    }
    struct stat tStat;
    if ((stat)(Path_ToPosix(szPath).c_str(), &tStat) != 0) {
        return -1;
    }
    if (!S_ISDIR(tStat.st_mode)) {
        errno = ENOTDIR;
        return -1;
    }
    return 0;
}

char* _getcwd(char* szBuffer, int nMaxLength) {
    std::string sWin = Path_ToWindows(InstallDir());
    if (szBuffer == nullptr) {
        return strdup(sWin.c_str());
    }
    if (nMaxLength <= 0 || sWin.size() >= (size_t)nMaxLength) {
        errno = ERANGE;
        return nullptr;
    }
    memcpy(szBuffer, sWin.c_str(), sWin.size() + 1);
    return szBuffer;
}

namespace {

void FillFindData(struct _finddata_t* pOut, const WIN32_FIND_DATAA& tIn) {
    pOut->attrib = 0;
    if (tIn.dwFileAttributes & 0x01) {
        pOut->attrib |= _A_RDONLY;
    }
    if (tIn.dwFileAttributes & 0x02) {
        pOut->attrib |= _A_HIDDEN;
    }
    if (tIn.dwFileAttributes & 0x10) {
        pOut->attrib |= _A_SUBDIR;
    }
    pOut->time_create = (int)TimeOf(tIn.ftCreationTime);
    pOut->time_access = (int)TimeOf(tIn.ftLastAccessTime);
    pOut->time_write = (int)TimeOf(tIn.ftLastWriteTime);
    pOut->size = tIn.nFileSizeLow;
    strncpy(pOut->name, tIn.cFileName, sizeof(pOut->name) - 1);
    pOut->name[sizeof(pOut->name) - 1] = '\0';
}

}  // namespace

intptr_t _findfirst(const char* szFileSpec, struct _finddata_t* pFindData) {
    if (!szFileSpec || !pFindData) {
        errno = EINVAL;
        return -1;
    }
    WIN32_FIND_DATAA tData;
    HANDLE hFind = FindFirstFileA(szFileSpec, &tData);
    if (hFind == INVALID_HANDLE_VALUE) {
        errno = ENOENT;
        return -1;
    }
    FillFindData(pFindData, tData);
    return (intptr_t)(uintptr_t)hFind;
}

int _findnext(intptr_t hFind, struct _finddata_t* pFindData) {
    WIN32_FIND_DATAA tData;
    if (!FindNextFileA((HANDLE)(uintptr_t)hFind, &tData)) {
        errno = ENOENT;
        return -1;
    }
    FillFindData(pFindData, tData);
    return 0;
}

int _findclose(intptr_t hFind) {
    return FindClose((HANDLE)(uintptr_t)hFind) ? 0 : -1;
}

void _splitpath(const char* szPath, char* szDrive, char* szDir, char* szFname, char* szExt) {
    std::string sPath = szPath ? szPath : "";
    std::string sDrive, sDir, sFname, sExt;
    size_t nStart = 0;
    if (sPath.size() >= 2 && sPath[1] == ':') {
        sDrive = sPath.substr(0, 2);
        nStart = 2;
    }
    size_t nSlash = sPath.find_last_of("\\/");
    size_t nLeaf = nSlash == std::string::npos || nSlash < nStart ? nStart : nSlash + 1;
    sDir = sPath.substr(nStart, nLeaf - nStart);
    std::string sLeaf = sPath.substr(nLeaf);
    size_t nDot = sLeaf.rfind('.');
    if (nDot == std::string::npos) {
        sFname = sLeaf;
    } else {
        sFname = sLeaf.substr(0, nDot);
        sExt = sLeaf.substr(nDot);
    }
    if (szDrive) {
        strlcpy(szDrive, sDrive.c_str(), _MAX_DRIVE);
    }
    if (szDir) {
        strlcpy(szDir, sDir.c_str(), _MAX_DIR);
    }
    if (szFname) {
        strlcpy(szFname, sFname.c_str(), _MAX_FNAME);
    }
    if (szExt) {
        strlcpy(szExt, sExt.c_str(), _MAX_EXT);
    }
}

void _makepath(char* szPath, const char* szDrive, const char* szDir, const char* szFname, const char* szExt) {
    std::string s;
    if (szDrive && *szDrive) {
        s += szDrive[0];
        s += ':';
    }
    if (szDir && *szDir) {
        s += szDir;
        if (s.back() != '\\' && s.back() != '/') {
            s += '\\';
        }
    }
    if (szFname) {
        s += szFname;
    }
    if (szExt && *szExt) {
        if (*szExt != '.') {
            s += '.';
        }
        s += szExt;
    }
    memcpy(szPath, s.c_str(), s.size() + 1);
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
