// Win32Posix: virtual memory and the pointer probes (Win32Mac's Win32_Memory.cpp).
//
// Win32Mac's VirtualAlloc is a plain allocation. Here a reservation is an mmap the layer keeps
// a record of, so commit, decommit and VirtualQuery mean what they do on Windows: a reservation
// starts on the 64 KB allocation granularity, and its pages - the 4 KB pages the shared code is
// written for - are committed and decommitted one by one. The host page may be larger (16 KB
// on arm64): a commit opens every host page it touches, a decommit returns only the host
// pages it covers whole, so a committed 4 KB page never loses its memory.
//
// Nothing is mapped executable: arm64 macOS gives no writable executable memory without
// MAP_JIT, and the code the game generates or loads at run time (D2Hell, the Warden loader) is
// x86 code either way. The EXECUTE protections map to their data equivalents.
//
// VirtualQuery and the IsBad*Ptr probes read the task's own map (mach_vm_region) for memory the
// layer did not reserve; __PAGEZERO is reported reserved, as it is.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <errno.h>
#include <mach-o/getsect.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <sys/mman.h>
#include <unistd.h>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

namespace {

using namespace Win32Posix;

constexpr uintptr_t kPage = 0x1000;
constexpr uintptr_t kGranularity = 0x10000;
constexpr DWORD kMemDecommit = 0x4000;
constexpr DWORD kMemPrivate = 0x20000;

struct Region {
    size_t nSize;
    DWORD dwAllocationProtect;
    std::vector<DWORD> aProtect;  // per 4 KB page; 0 while it is not committed
};

std::mutex& RegionLock() {
    static std::mutex* pLock = new std::mutex;
    return *pLock;
}

std::map<uintptr_t, Region>& Regions() {
    static auto* pRegions = new std::map<uintptr_t, Region>;
    return *pRegions;
}

uintptr_t HostPage() {
    static const uintptr_t nPage = (uintptr_t)getpagesize();
    return nPage;
}

uintptr_t RoundUp(uintptr_t n, uintptr_t nTo) {
    return (n + nTo - 1) & ~(nTo - 1);
}

int HostProtection(DWORD flProtect) {
    switch (flProtect & 0xFF) {
        case PAGE_NOACCESS: return PROT_NONE;
        case 0x02:  // PAGE_READONLY
        case 0x10:  // PAGE_EXECUTE
        case 0x20:  // PAGE_EXECUTE_READ
            return PROT_READ;
        default: return PROT_READ | PROT_WRITE;  // READWRITE, WRITECOPY and their EXECUTE forms
    }
}

// The region holding the address, the lock held; Regions().end() when none does.
std::map<uintptr_t, Region>::iterator FindRegion(uintptr_t nAddress) {
    auto& tRegions = Regions();
    auto it = tRegions.upper_bound(nAddress);
    if (it == tRegions.begin()) {
        return tRegions.end();
    }
    --it;
    return nAddress < it->first + it->second.nSize ? it : tRegions.end();
}

bool Overlaps(uintptr_t nBase, size_t nSize) {
    auto& tRegions = Regions();
    auto it = tRegions.lower_bound(nBase);
    if (it != tRegions.end() && it->first < nBase + nSize) {
        return true;
    }
    return FindRegion(nBase) != tRegions.end();
}

LPVOID Reserve(uintptr_t nAddress, size_t nSize, DWORD flAllocationType, DWORD flProtect) {
    bool bCommit = (flAllocationType & MEM_COMMIT) != 0;
    int nProt = bCommit ? HostProtection(flProtect) : PROT_NONE;
    uintptr_t nBase;
    size_t nLength;
    if (nAddress) {
        nBase = nAddress & ~(kGranularity - 1);
        nLength = RoundUp(nAddress + nSize - nBase, HostPage());
        if (Overlaps(nBase, nLength)) {
            SetError(487);  // ERROR_INVALID_ADDRESS
            return nullptr;
        }
        void* p = mmap((void*)nBase, nLength, nProt, MAP_PRIVATE | MAP_ANON, -1, 0);
        if (p == MAP_FAILED) {
            SetError(8);
            return nullptr;
        }
        if ((uintptr_t)p != nBase) {
            munmap(p, nLength);
            SetError(487);
            return nullptr;
        }
    } else {
        nLength = RoundUp(nSize, HostPage());
        size_t nOver = nLength + kGranularity;
        void* p = mmap(nullptr, nOver, nProt, MAP_PRIVATE | MAP_ANON, -1, 0);
        if (p == MAP_FAILED) {
            SetError(8);
            return nullptr;
        }
        nBase = RoundUp((uintptr_t)p, kGranularity);
        if (nBase > (uintptr_t)p) {
            munmap(p, nBase - (uintptr_t)p);
        }
        uintptr_t nEnd = (uintptr_t)p + nOver;
        if (nEnd > nBase + nLength) {
            munmap((void*)(nBase + nLength), nEnd - (nBase + nLength));
        }
    }
    Region tRegion;
    tRegion.nSize = nLength;
    tRegion.dwAllocationProtect = flProtect;
    tRegion.aProtect.assign(nLength / kPage, bCommit ? flProtect : 0);
    Regions()[nBase] = std::move(tRegion);
    return (LPVOID)nBase;
}

// The task's map at an address: the region at or after it. False past the last region.
bool HostRegion(uintptr_t nAddress, uintptr_t* pnStart, uintptr_t* pnEnd, int* pnProt) {
    mach_vm_address_t nStart = nAddress;
    mach_vm_size_t nSize = 0;
    vm_region_basic_info_data_64_t tInfo;
    mach_msg_type_number_t nCount = VM_REGION_BASIC_INFO_COUNT_64;
    mach_port_t tObject;
    if (mach_vm_region(mach_task_self(), &nStart, &nSize, VM_REGION_BASIC_INFO_64, (vm_region_info_t)&tInfo, &nCount,
                       &tObject) != KERN_SUCCESS) {
        return false;
    }
    *pnStart = (uintptr_t)nStart;
    *pnEnd = (uintptr_t)(nStart + nSize);
    *pnProt = tInfo.protection;
    return true;
}

uintptr_t PageZeroEnd() {
    static const uintptr_t nEnd = [] {
        const struct segment_command_64* pSeg = getsegbyname("__PAGEZERO");
        return pSeg ? (uintptr_t)pSeg->vmsize : (uintptr_t)0;
    }();
    return nEnd;
}

DWORD Win32Protection(int nProt) {
    if (nProt & VM_PROT_WRITE) {
        return (nProt & VM_PROT_EXECUTE) ? 0x40 : PAGE_READWRITE;
    }
    if (nProt & VM_PROT_READ) {
        return (nProt & VM_PROT_EXECUTE) ? 0x20 : 0x02;
    }
    return PAGE_NOACCESS;
}

bool RangeAccessible(const void* p, uintptr_t nBytes, int nWant) {
    if (nBytes == 0) {
        return true;
    }
    uintptr_t nAddress = (uintptr_t)p;
    if (nAddress == 0 || nAddress + nBytes < nAddress) {
        return false;
    }
    uintptr_t nEnd = nAddress + nBytes;
    while (nAddress < nEnd) {
        uintptr_t nStart, nRegionEnd;
        int nProt;
        if (!HostRegion(nAddress, &nStart, &nRegionEnd, &nProt) || nStart > nAddress || (nProt & nWant) != nWant) {
            return false;
        }
        nAddress = nRegionEnd;
    }
    return true;
}

}  // namespace

extern "C" LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) {
    if (dwSize == 0 || !(flAllocationType & (MEM_COMMIT | MEM_RESERVE))) {
        SetError(87);
        return nullptr;
    }
    std::lock_guard<std::mutex> tGuard(RegionLock());
    uintptr_t nAddress = (uintptr_t)lpAddress;
    if (!(flAllocationType & MEM_RESERVE) && nAddress) {
        auto it = FindRegion(nAddress);
        if (it == Regions().end() || nAddress + dwSize > it->first + it->second.nSize) {
            SetError(487);
            return nullptr;
        }
        uintptr_t nStart = nAddress & ~(kPage - 1);
        uintptr_t nEnd = RoundUp(nAddress + dwSize, kPage);
        uintptr_t nHostStart = nStart & ~(HostPage() - 1);
        uintptr_t nHostEnd = RoundUp(nEnd, HostPage());
        if (mprotect((void*)nHostStart, nHostEnd - nHostStart, HostProtection(flProtect)) != 0) {
            SetError(ErrorFromErrno(errno));
            return nullptr;
        }
        for (uintptr_t n = nStart; n < nEnd; n += kPage) {
            it->second.aProtect[(n - it->first) / kPage] = flProtect;
        }
        return (LPVOID)nStart;
    }
    return Reserve(nAddress, dwSize, flAllocationType, flProtect);
}

extern "C" BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType) {
    std::lock_guard<std::mutex> tGuard(RegionLock());
    uintptr_t nAddress = (uintptr_t)lpAddress;
    auto it = FindRegion(nAddress);
    if (it == Regions().end()) {
        SetError(487);
        return FALSE;
    }
    if (dwFreeType & MEM_RELEASE) {
        if (dwSize != 0 || nAddress != it->first) {
            SetError(87);
            return FALSE;
        }
        munmap((void*)it->first, it->second.nSize);
        Regions().erase(it);
        return TRUE;
    }
    if (!(dwFreeType & kMemDecommit)) {
        SetError(87);
        return FALSE;
    }
    uintptr_t nStart = nAddress & ~(kPage - 1);
    uintptr_t nEnd = dwSize ? RoundUp(nAddress + dwSize, kPage) : it->first + it->second.nSize;
    if (nEnd > it->first + it->second.nSize) {
        SetError(87);
        return FALSE;
    }
    for (uintptr_t n = nStart; n < nEnd; n += kPage) {
        it->second.aProtect[(n - it->first) / kPage] = 0;
    }
    // return the host pages no committed page is left in: a fresh mapping reads zero again
    for (uintptr_t nHost = nStart & ~(HostPage() - 1); nHost < nEnd; nHost += HostPage()) {
        bool bInUse = false;
        for (uintptr_t n = nHost; n < nHost + HostPage(); n += kPage) {
            if (n >= it->first && n < it->first + it->second.nSize && it->second.aProtect[(n - it->first) / kPage]) {
                bInUse = true;
            }
        }
        if (!bInUse) {
            mmap((void*)nHost, HostPage(), PROT_NONE, MAP_PRIVATE | MAP_ANON | MAP_FIXED, -1, 0);
        }
    }
    return TRUE;
}

SIZE_T VirtualQuery(LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength) {
    if (dwLength < sizeof(MEMORY_BASIC_INFORMATION)) {
        SetError(24);  // ERROR_BAD_LENGTH
        return 0;
    }
    uintptr_t nAddress = (uintptr_t)lpAddress;
    MEMORY_BASIC_INFORMATION tInfo = {};
    {
        std::lock_guard<std::mutex> tGuard(RegionLock());
        auto it = FindRegion(nAddress);
        if (it != Regions().end()) {
            const Region& r = it->second;
            size_t nPage = (nAddress - it->first) / kPage;
            DWORD dwProtect = r.aProtect[nPage];
            size_t nLast = nPage;
            while (nLast + 1 < r.aProtect.size() && r.aProtect[nLast + 1] == dwProtect) {
                ++nLast;
            }
            tInfo.BaseAddress = (PVOID)(it->first + nPage * kPage);
            tInfo.AllocationBase = (PVOID)it->first;
            tInfo.AllocationProtect = r.dwAllocationProtect;
            tInfo.RegionSize = (nLast + 1 - nPage) * kPage;
            tInfo.State = dwProtect ? MEM_COMMIT : MEM_RESERVE;
            tInfo.Protect = dwProtect;
            tInfo.Type = kMemPrivate;
            *lpBuffer = tInfo;
            return sizeof(tInfo);
        }
    }
    uintptr_t nPageBase = nAddress & ~(HostPage() - 1);
    tInfo.BaseAddress = (PVOID)nPageBase;
    if (nAddress < PageZeroEnd()) {
        tInfo.AllocationBase = nullptr;
        tInfo.AllocationProtect = PAGE_NOACCESS;
        tInfo.RegionSize = PageZeroEnd() - nPageBase;
        tInfo.State = MEM_RESERVE;
        tInfo.Type = kMemPrivate;
        *lpBuffer = tInfo;
        return sizeof(tInfo);
    }
    uintptr_t nStart, nEnd;
    int nProt;
    if (!HostRegion(nAddress, &nStart, &nEnd, &nProt)) {
        if (nAddress >= MACH_VM_MAX_ADDRESS) {
            SetError(87);
            return 0;
        }
        nStart = MACH_VM_MAX_ADDRESS;
        nProt = 0;
    }
    if (nStart > nAddress) {
        tInfo.RegionSize = nStart - nPageBase;
        tInfo.State = MEM_FREE;
        tInfo.Protect = PAGE_NOACCESS;
    } else {
        tInfo.AllocationBase = (PVOID)nStart;
        tInfo.AllocationProtect = Win32Protection(nProt);
        tInfo.RegionSize = nEnd - nPageBase;
        tInfo.State = MEM_COMMIT;
        tInfo.Protect = Win32Protection(nProt);
        tInfo.Type = kMemPrivate;
    }
    *lpBuffer = tInfo;
    return sizeof(tInfo);
}

BOOL IsBadReadPtr(const void* lp, UINT_PTR ucb) {
    return !RangeAccessible(lp, ucb, VM_PROT_READ);
}

BOOL IsBadWritePtr(LPVOID lp, UINT_PTR ucb) {
    return !RangeAccessible(lp, ucb, VM_PROT_READ | VM_PROT_WRITE);
}

BOOL IsBadCodePtr(FARPROC lpfn) {
    return !RangeAccessible((const void*)lpfn, 1, VM_PROT_READ);
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
