// Win32Posix: file handles, attributes, directories, the find calls and the volume queries
// (Win32Mac's Win32_Files.cpp).
//
// A file handle holds a POSIX descriptor. Win32Mac refuses positions past 32 bits and opens
// fail with 0; this follows the SDK - 64-bit positions through the high word, and
// INVALID_HANDLE_VALUE on failure, since the shared code takes its Windows path. The share
// mode is not enforced (no other process opens the game's files). An overlapped read or write
// runs at the OVERLAPPED's offset and completes before it returns: the record is filled and
// its event set, as for a file opened without FILE_FLAG_OVERLAPPED.
//
// A find walks one directory, its names matched without case against the pattern's * and ?
// as NTFS matches them ("*.*" is every name), in name order. "." and ".." are listed as on
// Windows. There is one drive, C:, the root; it is a fixed disk.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <algorithm>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

namespace {

using namespace Win32Posix;

constexpr DWORD kGenericRead = 0x80000000;
constexpr DWORD kGenericWrite = 0x40000000;
constexpr DWORD kGenericAll = 0x10000000;
constexpr DWORD kFileWriteData = 0x0002;
constexpr DWORD kFileAppendData = 0x0004;
constexpr DWORD kCreateNew = 1;
constexpr DWORD kCreateAlways = 2;
constexpr DWORD kOpenExisting = 3;
constexpr DWORD kOpenAlways = 4;
constexpr DWORD kTruncateExisting = 5;
constexpr DWORD kFlagDeleteOnClose = 0x04000000;
constexpr DWORD kFlagBackupSemantics = 0x02000000;
constexpr DWORD kAttrReadOnly = 0x01;
constexpr DWORD kAttrHidden = 0x02;
constexpr DWORD kAttrDirectory = 0x10;
constexpr DWORD kAttrArchive = 0x20;
constexpr DWORD kInvalidAttributes = 0xFFFFFFFF;
constexpr DWORD kInvalidSetFilePointer = 0xFFFFFFFF;

struct FileObject : Object {
    FileObject() : Object(OBJECT_FILE) {}
    ~FileObject() override {
        if (fd >= 0) {
            close(fd);
        }
        if (bDeleteOnClose) {
            unlink(sPath.c_str());
        }
    }
    int fd = -1;
    std::string sPath;
    bool bDeleteOnClose = false;
};

struct FindObject : Object {
    FindObject() : Object(OBJECT_FIND) {}
    std::vector<WIN32_FIND_DATAA> aEntries;
    size_t nNext = 0;
};

std::shared_ptr<FileObject> GetFile(HANDLE hFile) {
    auto pFile = Handle_GetAs<FileObject>(hFile, OBJECT_FILE);
    if (!pFile) {
        SetError(6);
    }
    return pFile;
}

DWORD AttributesOf(const struct stat& tStat, const char* szName) {
    DWORD dwAttributes = S_ISDIR(tStat.st_mode) ? kAttrDirectory : kAttrArchive;
    if (!(tStat.st_mode & S_IWUSR)) {
        dwAttributes |= kAttrReadOnly;
    }
    if (szName[0] == '.' && strcmp(szName, ".") != 0 && strcmp(szName, "..") != 0) {
        dwAttributes |= kAttrHidden;
    }
    return dwAttributes;
}

const char* LeafOf(const std::string& sPath) {
    size_t nSlash = sPath.rfind('/');
    return sPath.c_str() + (nSlash == std::string::npos ? 0 : nSlash + 1);
}

// The NTFS match of a name against a pattern of * and ?, without case.
bool WildMatch(const char* szPattern, const char* szName) {
    if (strcmp(szPattern, "*.*") == 0 || strcmp(szPattern, "*") == 0) {
        return true;
    }
    const char* pStar = nullptr;
    const char* pResume = nullptr;
    while (*szName) {
        if (*szPattern == '?' || (*szPattern != '*' && tolower((unsigned char)*szPattern) ==
                                                        tolower((unsigned char)*szName))) {
            ++szPattern;
            ++szName;
        } else if (*szPattern == '*') {
            pStar = szPattern++;
            pResume = szName;
        } else if (pStar) {
            szPattern = pStar + 1;
            szName = ++pResume;
        } else {
            return false;
        }
    }
    // "name." matches "name": a trailing dot stands for an empty extension
    while (*szPattern == '*' || (*szPattern == '.' && szPattern[1] == '\0')) {
        ++szPattern;
    }
    return *szPattern == '\0';
}

void FillFindData(WIN32_FIND_DATAA* pData, const struct stat& tStat, const char* szName) {
    memset(pData, 0, sizeof(*pData));
    pData->dwFileAttributes = AttributesOf(tStat, szName);
    pData->ftCreationTime = FileTime_FromTimespec(tStat.st_birthtimespec);
    pData->ftLastAccessTime = FileTime_FromTimespec(tStat.st_atimespec);
    pData->ftLastWriteTime = FileTime_FromTimespec(tStat.st_mtimespec);
    uint64_t nSize = S_ISDIR(tStat.st_mode) ? 0 : (uint64_t)tStat.st_size;
    pData->nFileSizeHigh = (DWORD)(nSize >> 32);
    pData->nFileSizeLow = (DWORD)nSize;
    strncpy(pData->cFileName, szName, sizeof(pData->cFileName) - 1);
}

}  // namespace

namespace Win32Posix {

int File_Descriptor(HANDLE hFile) {
    auto pFile = Handle_GetAs<FileObject>(hFile, OBJECT_FILE);
    return pFile ? pFile->fd : -1;
}

}  // namespace Win32Posix

HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSec,
                   DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplate) {
    (void)dwShareMode;
    (void)lpSec;
    (void)hTemplate;
    if (lpFileName == nullptr || *lpFileName == '\0') {
        SetError(2);
        return INVALID_HANDLE_VALUE;
    }
    bool bWrite = (dwDesiredAccess & (kGenericWrite | kGenericAll | kFileWriteData | kFileAppendData)) != 0;
    bool bRead = (dwDesiredAccess & (kGenericRead | kGenericAll)) != 0 || !bWrite;
    int nFlags = O_CLOEXEC;
    switch (dwCreationDisposition) {
        case kCreateNew: nFlags |= O_CREAT | O_EXCL; break;
        case kCreateAlways: nFlags |= O_CREAT | O_TRUNC; break;
        case kOpenExisting: break;
        case kOpenAlways: nFlags |= O_CREAT; break;
        case kTruncateExisting: nFlags |= O_TRUNC; break;
        default: SetError(87); return INVALID_HANDLE_VALUE;
    }
    if (nFlags & O_TRUNC) {
        bWrite = true;
    }
    nFlags |= bWrite ? (bRead ? O_RDWR : O_WRONLY) : O_RDONLY;
    std::string sPath = (dwCreationDisposition == kOpenExisting && !bWrite) ? Path_ToPosixForRead(lpFileName)
                                                                           : Path_ToPosix(lpFileName);
    struct stat tStat;
    bool bExisted = stat(sPath.c_str(), &tStat) == 0;
    if (bExisted && S_ISDIR(tStat.st_mode) && !(dwFlagsAndAttributes & kFlagBackupSemantics)) {
        SetError(5);
        return INVALID_HANDLE_VALUE;
    }
    if (bExisted && S_ISDIR(tStat.st_mode)) {
        nFlags = (nFlags & ~(O_RDWR | O_WRONLY | O_TRUNC | O_CREAT)) | O_RDONLY;
    }
    int fd = open(sPath.c_str(), nFlags, 0644);
    if (fd < 0) {
        DWORD dwError = ErrorFromErrno(errno);
        if (errno == EEXIST) {
            dwError = 80;  // ERROR_FILE_EXISTS
        } else if (errno == ENOENT) {
            std::string sDir = sPath.substr(0, sPath.rfind('/'));
            struct stat tDir;
            if (!sDir.empty() && stat(sDir.c_str(), &tDir) != 0) {
                dwError = 3;  // ERROR_PATH_NOT_FOUND
            }
        }
        SetError(dwError);
        return INVALID_HANDLE_VALUE;
    }
    auto pFile = std::make_shared<FileObject>();
    pFile->fd = fd;
    pFile->sPath = sPath;
    pFile->bDeleteOnClose = (dwFlagsAndAttributes & kFlagDeleteOnClose) != 0;
    HANDLE hFile = Handle_Insert(pFile);
    if (!hFile) {
        return INVALID_HANDLE_VALUE;
    }
    bool bReportExisted = bExisted && (dwCreationDisposition == kCreateAlways || dwCreationDisposition == kOpenAlways);
    SetError(bReportExisted ? 183 : 0);  // ERROR_ALREADY_EXISTS
    return hFile;
}

BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nBytes, LPDWORD lpBytesRead, LPOVERLAPPED lpOverlapped) {
    if (lpBytesRead) {
        *lpBytesRead = 0;
    }
    auto pFile = GetFile(hFile);
    if (!pFile) {
        return FALSE;
    }
    off_t nOffset = lpOverlapped ? (off_t)(((uint64_t)lpOverlapped->OffsetHigh << 32) | lpOverlapped->Offset) : 0;
    DWORD nDone = 0;
    while (nDone < nBytes) {
        ssize_t n = lpOverlapped ? pread(pFile->fd, (char*)lpBuffer + nDone, nBytes - nDone, nOffset + nDone)
                                 : read(pFile->fd, (char*)lpBuffer + nDone, nBytes - nDone);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            SetError(ErrorFromErrno(errno));
            return FALSE;
        }
        if (n == 0) {
            break;
        }
        nDone += (DWORD)n;
    }
    if (lpBytesRead) {
        *lpBytesRead = nDone;
    }
    if (lpOverlapped) {
        lpOverlapped->Internal = 0;
        lpOverlapped->InternalHigh = nDone;
        if (lpOverlapped->hEvent) {
            SetEvent(lpOverlapped->hEvent);
        }
    }
    return TRUE;
}

BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nBytes, LPDWORD lpBytesWritten, LPOVERLAPPED lpOverlapped) {
    if (lpBytesWritten) {
        *lpBytesWritten = 0;
    }
    auto pFile = GetFile(hFile);
    if (!pFile) {
        return FALSE;
    }
    off_t nOffset = lpOverlapped ? (off_t)(((uint64_t)lpOverlapped->OffsetHigh << 32) | lpOverlapped->Offset) : 0;
    DWORD nDone = 0;
    while (nDone < nBytes) {
        ssize_t n = lpOverlapped ? pwrite(pFile->fd, (const char*)lpBuffer + nDone, nBytes - nDone, nOffset + nDone)
                                 : write(pFile->fd, (const char*)lpBuffer + nDone, nBytes - nDone);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            SetError(ErrorFromErrno(errno));
            if (lpBytesWritten) {
                *lpBytesWritten = nDone;
            }
            return FALSE;
        }
        nDone += (DWORD)n;
    }
    if (lpBytesWritten) {
        *lpBytesWritten = nDone;
    }
    if (lpOverlapped) {
        lpOverlapped->Internal = 0;
        lpOverlapped->InternalHigh = nDone;
        if (lpOverlapped->hEvent) {
            SetEvent(lpOverlapped->hEvent);
        }
    }
    return TRUE;
}

BOOL FlushFileBuffers(HANDLE hFile) {
    auto pFile = GetFile(hFile);
    if (!pFile) {
        return FALSE;
    }
    if (fsync(pFile->fd) != 0) {
        SetError(ErrorFromErrno(errno));
        return FALSE;
    }
    return TRUE;
}

DWORD SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod) {
    auto pFile = GetFile(hFile);
    if (!pFile) {
        return kInvalidSetFilePointer;
    }
    int64_t nDistance = lpDistanceToMoveHigh
                            ? (int64_t)(((uint64_t)(uint32_t)*lpDistanceToMoveHigh << 32) | (uint32_t)lDistanceToMove)
                            : (int64_t)lDistanceToMove;
    int nWhence = dwMoveMethod == 1 ? SEEK_CUR : dwMoveMethod == 2 ? SEEK_END : SEEK_SET;
    if (dwMoveMethod > 2) {
        SetError(87);
        return kInvalidSetFilePointer;
    }
    off_t nBase = nWhence == SEEK_SET ? 0 : lseek(pFile->fd, 0, nWhence);
    if (nBase + nDistance < 0) {
        SetError(131);  // ERROR_NEGATIVE_SEEK
        return kInvalidSetFilePointer;
    }
    off_t nPos = lseek(pFile->fd, nDistance, nWhence);
    if (nPos < 0) {
        SetError(ErrorFromErrno(errno));
        return kInvalidSetFilePointer;
    }
    if (lpDistanceToMoveHigh) {
        *lpDistanceToMoveHigh = (LONG)((uint64_t)nPos >> 32);
    } else if ((uint64_t)nPos > 0xFFFFFFFFu) {
        SetError(87);  // the position does not fit the low word alone
        return kInvalidSetFilePointer;
    }
    SetError(0);
    return (DWORD)nPos;
}

DWORD GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh) {
    auto pFile = GetFile(hFile);
    struct stat tStat;
    if (!pFile || fstat(pFile->fd, &tStat) != 0) {
        if (pFile) {
            SetError(ErrorFromErrno(errno));
        }
        return 0xFFFFFFFF;  // INVALID_FILE_SIZE
    }
    if (lpFileSizeHigh) {
        *lpFileSizeHigh = (DWORD)((uint64_t)tStat.st_size >> 32);
    }
    SetError(0);
    return (DWORD)tStat.st_size;
}

BOOL GetFileTime(HANDLE hFile, LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime) {
    auto pFile = GetFile(hFile);
    struct stat tStat;
    if (!pFile || fstat(pFile->fd, &tStat) != 0) {
        return FALSE;
    }
    if (lpCreationTime) {
        *lpCreationTime = FileTime_FromTimespec(tStat.st_birthtimespec);
    }
    if (lpLastAccessTime) {
        *lpLastAccessTime = FileTime_FromTimespec(tStat.st_atimespec);
    }
    if (lpLastWriteTime) {
        *lpLastWriteTime = FileTime_FromTimespec(tStat.st_mtimespec);
    }
    return TRUE;
}

// The creation time is the file system's to keep: only the access and write times are set.
BOOL SetFileTime(HANDLE hFile, const FILETIME* lpCreationTime, const FILETIME* lpLastAccessTime,
                 const FILETIME* lpLastWriteTime) {
    (void)lpCreationTime;
    auto pFile = GetFile(hFile);
    if (!pFile) {
        return FALSE;
    }
    struct timespec aTimes[2];
    aTimes[0].tv_nsec = UTIME_OMIT;
    aTimes[1].tv_nsec = UTIME_OMIT;
    if (lpLastAccessTime) {
        aTimes[0] = FileTime_ToTimespec(*lpLastAccessTime);
    }
    if (lpLastWriteTime) {
        aTimes[1] = FileTime_ToTimespec(*lpLastWriteTime);
    }
    if (futimens(pFile->fd, aTimes) != 0) {
        SetError(ErrorFromErrno(errno));
        return FALSE;
    }
    return TRUE;
}

BOOL DeleteFileA(LPCSTR lpFileName) {
    std::string sPath = Path_ToPosix(lpFileName);
    struct stat tStat;
    if (stat(sPath.c_str(), &tStat) == 0 && S_ISDIR(tStat.st_mode)) {
        SetError(5);
        return FALSE;
    }
    if (unlink(sPath.c_str()) != 0) {
        SetError(ErrorFromErrno(errno));
        return FALSE;
    }
    return TRUE;
}

DWORD GetFileAttributesA(LPCSTR lpFileName) {
    std::string sPath = Path_ToPosixForRead(lpFileName);
    struct stat tStat;
    if (stat(sPath.c_str(), &tStat) != 0) {
        SetError(ErrorFromErrno(errno));
        return kInvalidAttributes;
    }
    return AttributesOf(tStat, LeafOf(sPath));
}

BOOL CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
    (void)lpSecurityAttributes;
    if (mkdir(Path_ToPosix(lpPathName).c_str(), 0755) != 0) {
        SetError(ErrorFromErrno(errno));
        return FALSE;
    }
    return TRUE;
}

HANDLE FindFirstFileA(LPCSTR lpFileName, struct _WIN32_FIND_DATAA* lpFindFileData) {
    std::string sPattern = lpFileName ? lpFileName : "";
    size_t nSep = sPattern.find_last_of("\\/");
    std::string sDirWin = nSep == std::string::npos ? std::string(".") : sPattern.substr(0, nSep + 1);
    std::string sMask = nSep == std::string::npos ? sPattern : sPattern.substr(nSep + 1);
    if (sMask.empty()) {
        SetError(2);
        return INVALID_HANDLE_VALUE;
    }
    std::string sDir = Path_ToPosix(sDirWin.c_str());
    DIR* pDir = opendir(sDir.c_str());
    if (!pDir) {
        SetError(errno == ENOENT ? 3 : ErrorFromErrno(errno));
        return INVALID_HANDLE_VALUE;
    }
    auto pFind = std::make_shared<FindObject>();
    while (struct dirent* pEntry = readdir(pDir)) {
        if (!WildMatch(sMask.c_str(), pEntry->d_name)) {
            continue;
        }
        if (sDir == "/" && (strcmp(pEntry->d_name, ".") == 0 || strcmp(pEntry->d_name, "..") == 0)) {
            continue;  // a drive's root lists neither
        }
        struct stat tStat;
        std::string sEntry = (sDir.back() == '/' ? sDir : sDir + "/") + pEntry->d_name;
        if (stat(sEntry.c_str(), &tStat) != 0) {
            continue;  // a dangling link
        }
        WIN32_FIND_DATAA tData;
        FillFindData(&tData, tStat, pEntry->d_name);
        pFind->aEntries.push_back(tData);
    }
    closedir(pDir);
    std::sort(pFind->aEntries.begin(), pFind->aEntries.end(), [](const WIN32_FIND_DATAA& a, const WIN32_FIND_DATAA& b) {
        return strcasecmp(a.cFileName, b.cFileName) < 0;
    });
    if (pFind->aEntries.empty()) {
        SetError(2);
        return INVALID_HANDLE_VALUE;
    }
    *lpFindFileData = pFind->aEntries[0];
    pFind->nNext = 1;
    HANDLE hFind = Handle_Insert(pFind);
    return hFind ? hFind : INVALID_HANDLE_VALUE;
}

BOOL FindNextFileA(HANDLE hFindFile, struct _WIN32_FIND_DATAA* lpFindFileData) {
    auto pFind = Handle_GetAs<FindObject>(hFindFile, OBJECT_FIND);
    if (!pFind) {
        SetError(6);
        return FALSE;
    }
    if (pFind->nNext >= pFind->aEntries.size()) {
        SetError(18);  // ERROR_NO_MORE_FILES
        return FALSE;
    }
    *lpFindFileData = pFind->aEntries[pFind->nNext++];
    return TRUE;
}

BOOL FindClose(HANDLE hFindFile) {
    if (!Handle_GetAs<FindObject>(hFindFile, OBJECT_FIND)) {
        SetError(6);
        return FALSE;
    }
    return CloseHandle(hFindFile);
}

// ---- volumes ----

namespace {

bool StatVolume(LPCSTR lpRootPathName, struct statfs* pStat) {
    std::string sPath = lpRootPathName ? Path_ToPosix(lpRootPathName) : InstallDir();
    if (statfs(sPath.c_str(), pStat) != 0) {
        SetError(ErrorFromErrno(errno));
        return false;
    }
    return true;
}

}  // namespace

// The sizes in 4 KB clusters of 512-byte sectors, the cluster counts capped at 32 bits.
BOOL GetDiskFreeSpaceA(LPCSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector,
                       LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters) {
    struct statfs tStat;
    if (!StatVolume(lpRootPathName, &tStat)) {
        return FALSE;
    }
    uint64_t nFree = (uint64_t)tStat.f_bavail * tStat.f_bsize / 4096;
    uint64_t nTotal = (uint64_t)tStat.f_blocks * tStat.f_bsize / 4096;
    if (lpSectorsPerCluster) {
        *lpSectorsPerCluster = 8;
    }
    if (lpBytesPerSector) {
        *lpBytesPerSector = 512;
    }
    if (lpNumberOfFreeClusters) {
        *lpNumberOfFreeClusters = nFree > 0xFFFFFFFFu ? 0xFFFFFFFFu : (DWORD)nFree;
    }
    if (lpTotalNumberOfClusters) {
        *lpTotalNumberOfClusters = nTotal > 0xFFFFFFFFu ? 0xFFFFFFFFu : (DWORD)nTotal;
    }
    return TRUE;
}

BOOL GetDiskFreeSpaceExA(LPCSTR lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailableToCaller,
                         PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes) {
    struct statfs tStat;
    if (!StatVolume(lpDirectoryName, &tStat)) {
        return FALSE;
    }
    if (lpFreeBytesAvailableToCaller) {
        lpFreeBytesAvailableToCaller->QuadPart = (ULONGLONG)tStat.f_bavail * tStat.f_bsize;
    }
    if (lpTotalNumberOfBytes) {
        lpTotalNumberOfBytes->QuadPart = (ULONGLONG)tStat.f_blocks * tStat.f_bsize;
    }
    if (lpTotalNumberOfFreeBytes) {
        lpTotalNumberOfFreeBytes->QuadPart = (ULONGLONG)tStat.f_bfree * tStat.f_bsize;
    }
    return TRUE;
}

DWORD GetLogicalDriveStringsA(DWORD nBufferLength, LPSTR lpBuffer) {
    static const char kDrives[] = "C:\\\0";  // one drive, and the list's closing terminator
    if (lpBuffer == nullptr || nBufferLength < sizeof(kDrives)) {
        return sizeof(kDrives);
    }
    memcpy(lpBuffer, kDrives, sizeof(kDrives));
    return sizeof(kDrives) - 1;
}

UINT GetDriveTypeA(LPCSTR lpRootPathName) {
    if (lpRootPathName && lpRootPathName[0] && lpRootPathName[1] == ':' && (lpRootPathName[0] | 0x20) != 'c') {
        return 1;  // DRIVE_NO_ROOT_DIR
    }
    return 3;  // DRIVE_FIXED
}

BOOL GetVolumeInformationA(LPCSTR lpRootPathName, LPSTR lpVolumeNameBuffer, DWORD nVolumeNameSize,
                           LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags,
                           LPSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize) {
    struct statfs tStat;
    if (!StatVolume(lpRootPathName, &tStat)) {
        return FALSE;
    }
    if (lpVolumeNameBuffer && nVolumeNameSize) {
        const char* szName = strrchr(tStat.f_mntonname, '/');
        szName = szName && szName[1] ? szName + 1 : "Macintosh HD";
        strlcpy(lpVolumeNameBuffer, szName, nVolumeNameSize);
    }
    if (lpVolumeSerialNumber) {
        *lpVolumeSerialNumber = (DWORD)tStat.f_fsid.val[0] ^ (DWORD)tStat.f_fsid.val[1];
    }
    if (lpMaximumComponentLength) {
        *lpMaximumComponentLength = 255;
    }
    if (lpFileSystemFlags) {
        *lpFileSystemFlags = 0x2 | 0x4;  // FILE_CASE_PRESERVED_NAMES | FILE_UNICODE_ON_DISK
    }
    if (lpFileSystemNameBuffer && nFileSystemNameSize) {
        std::string sName = tStat.f_fstypename;
        for (char& c : sName) {
            c = (char)toupper((unsigned char)c);
        }
        strlcpy(lpFileSystemNameBuffer, sName.c_str(), nFileSystemNameSize);
    }
    return TRUE;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
