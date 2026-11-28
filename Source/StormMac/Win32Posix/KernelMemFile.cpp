// Win32Posix: file mappings (Win32Mac's Win32_MemFile.cpp, which has no file mapping of its
// own: Storm and D2Sound read their files instead).
//
// A mapping holds a duplicate of its file's descriptor and the size; a view is an mmap of it,
// shared when the mapping is writable and private otherwise. A mapping of INVALID_HANDLE_VALUE
// is anonymous memory of the size given, not shared between views.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

namespace {

using namespace Win32Posix;

constexpr DWORD kPageReadWrite = 0x04;
constexpr DWORD kPageExecuteReadWrite = 0x40;
constexpr DWORD kFileMapCopy = 0x0001;
constexpr DWORD kFileMapWrite = 0x0002;

struct MappingObject : Object {
    MappingObject() : Object(OBJECT_MAPPING) {}
    ~MappingObject() override {
        if (fd >= 0) {
            close(fd);
        }
    }
    int fd = -1;
    uint64_t nSize = 0;
    bool bWritable = false;
};

std::mutex& ViewLock() {
    static std::mutex* pLock = new std::mutex;
    return *pLock;
}

std::map<uintptr_t, size_t>& Views() {
    static auto* pViews = new std::map<uintptr_t, size_t>;
    return *pViews;
}

}  // namespace

HANDLE CreateFileMappingA(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect,
                          DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName) {
    (void)lpFileMappingAttributes;
    (void)lpName;
    DWORD dwPage = flProtect & 0xFF;
    auto pMapping = std::make_shared<MappingObject>();
    pMapping->bWritable = dwPage == kPageReadWrite || dwPage == kPageExecuteReadWrite;
    uint64_t nSize = ((uint64_t)dwMaximumSizeHigh << 32) | dwMaximumSizeLow;
    if (hFile != INVALID_HANDLE_VALUE) {
        int fd = File_Descriptor(hFile);
        if (fd < 0) {
            SetError(6);
            return 0;
        }
        struct stat tStat;
        if (fstat(fd, &tStat) != 0) {
            SetError(ErrorFromErrno(errno));
            return 0;
        }
        if (nSize == 0) {
            nSize = (uint64_t)tStat.st_size;
        }
        if (nSize == 0) {
            SetError(1006);  // ERROR_FILE_INVALID: an empty file cannot be mapped
            return 0;
        }
        if (nSize > (uint64_t)tStat.st_size) {
            if (!pMapping->bWritable || ftruncate(fd, (off_t)nSize) != 0) {
                SetError(8);
                return 0;
            }
        }
        pMapping->fd = dup(fd);
    } else if (nSize == 0) {
        SetError(87);
        return 0;
    }
    pMapping->nSize = nSize;
    HANDLE hMapping = Handle_Insert(pMapping);
    if (hMapping) {
        SetError(0);
    }
    return hMapping;
}

LPVOID MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow,
                     SIZE_T dwNumberOfBytesToMap) {
    auto pMapping = Handle_GetAs<MappingObject>(hFileMappingObject, OBJECT_MAPPING);
    if (!pMapping) {
        SetError(6);
        return nullptr;
    }
    uint64_t nOffset = ((uint64_t)dwFileOffsetHigh << 32) | dwFileOffsetLow;
    if (nOffset >= pMapping->nSize || (nOffset & 0xFFFF) != 0) {
        SetError(87);  // past the end, or not on the 64 KB allocation granularity
        return nullptr;
    }
    size_t nLength = dwNumberOfBytesToMap ? dwNumberOfBytesToMap : (size_t)(pMapping->nSize - nOffset);
    bool bWrite = (dwDesiredAccess & kFileMapWrite) != 0;
    bool bCopy = (dwDesiredAccess & kFileMapCopy) != 0 && !bWrite;
    if (bWrite && !pMapping->bWritable) {
        SetError(5);
        return nullptr;
    }
    int nProt = PROT_READ | (bWrite || bCopy ? PROT_WRITE : 0);
    void* pView;
    if (pMapping->fd >= 0) {
        int nFlags = bWrite ? MAP_SHARED : MAP_PRIVATE;
        pView = mmap(nullptr, nLength, nProt, nFlags, pMapping->fd, (off_t)nOffset);
    } else {
        // each view of an anonymous mapping is its own memory: the game maps one once
        pView = mmap(nullptr, nLength, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    }
    if (pView == MAP_FAILED) {
        SetError(ErrorFromErrno(errno));
        return nullptr;
    }
    std::lock_guard<std::mutex> tGuard(ViewLock());
    Views()[(uintptr_t)pView] = nLength;
    return pView;
}

BOOL UnmapViewOfFile(LPCVOID lpBaseAddress) {
    size_t nLength;
    {
        std::lock_guard<std::mutex> tGuard(ViewLock());
        auto it = Views().find((uintptr_t)lpBaseAddress);
        if (it == Views().end()) {
            SetError(487);  // ERROR_INVALID_ADDRESS
            return FALSE;
        }
        nLength = it->second;
        Views().erase(it);
    }
    munmap((void*)lpBaseAddress, nLength);
    return TRUE;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
