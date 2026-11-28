// Win32Posix: the handle table, CloseHandle, the waits and the last error (Win32Mac's
// Win32_Handles.cpp, Win32_CloseHandle.cpp and the waits of Win32_Events.cpp).
//
// The table is a vector of slots under one mutex; a handle is 0x100 + 4 * slot, so it fits in
// 32 bits and is never 0 or INVALID_HANDLE_VALUE. A freed slot is reused last-in first-out, as
// Windows reuses handle values. The waits run on one condition variable: every state change
// of every waitable object wakes all waiters, which recheck their own objects. Win32Mac polls
// a multiple-object wait with yields; this blocks.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <errno.h>
#include <chrono>
#include <vector>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

namespace Win32Posix {

namespace {

constexpr uintptr_t kHandleBase = 0x100;
constexpr size_t kMaxHandles = 0x100000;

struct HandleTable {
    std::mutex tLock;
    std::vector<std::shared_ptr<Object>> aSlots;
    std::vector<size_t> aFree;
};

HandleTable& Table() {
    static HandleTable* pTable = new HandleTable;  // never destroyed: threads may outlive exit
    return *pTable;
}

bool SlotOf(HANDLE hHandle, size_t* pnSlot) {
    uintptr_t nValue = (uintptr_t)hHandle;
    if (nValue < kHandleBase || (nValue & 3) != 0) {
        return false;
    }
    *pnSlot = (nValue - kHandleBase) / 4;
    return *pnSlot < kMaxHandles;
}

std::condition_variable& DispatcherWake() {
    static std::condition_variable* pWake = new std::condition_variable;
    return *pWake;
}

thread_local DWORD tl_dwLastError = 0;

}  // namespace

HANDLE Handle_Insert(std::shared_ptr<Object> pObject) {
    HandleTable& t = Table();
    std::lock_guard<std::mutex> tGuard(t.tLock);
    size_t nSlot;
    if (!t.aFree.empty()) {
        nSlot = t.aFree.back();
        t.aFree.pop_back();
    } else {
        if (t.aSlots.size() >= kMaxHandles) {
            SetError(4);  // ERROR_TOO_MANY_OPEN_FILES
            return 0;
        }
        nSlot = t.aSlots.size();
        t.aSlots.emplace_back();
    }
    t.aSlots[nSlot] = std::move(pObject);
    return (HANDLE)(kHandleBase + nSlot * 4);
}

std::shared_ptr<Object> Handle_Get(HANDLE hHandle) {
    size_t nSlot;
    if (!SlotOf(hHandle, &nSlot)) {
        return nullptr;
    }
    HandleTable& t = Table();
    std::lock_guard<std::mutex> tGuard(t.tLock);
    return nSlot < t.aSlots.size() ? t.aSlots[nSlot] : nullptr;
}

bool Handle_Remove(HANDLE hHandle) {
    size_t nSlot;
    if (!SlotOf(hHandle, &nSlot)) {
        return false;
    }
    std::shared_ptr<Object> pObject;  // released outside the lock: a destructor may close more
    HandleTable& t = Table();
    {
        std::lock_guard<std::mutex> tGuard(t.tLock);
        if (nSlot >= t.aSlots.size() || !t.aSlots[nSlot]) {
            return false;
        }
        pObject = std::move(t.aSlots[nSlot]);
        t.aSlots[nSlot] = nullptr;
        t.aFree.push_back(nSlot);
    }
    return true;
}

DWORD Handle_Value(HANDLE hHandle) {
    return (DWORD)(uintptr_t)hHandle;
}

std::mutex& DispatcherLock() {
    static std::mutex* pLock = new std::mutex;
    return *pLock;
}

void Dispatcher_Signal(Object* pObject, bool bSignaled) {
    {
        std::lock_guard<std::mutex> tGuard(DispatcherLock());
        pObject->bSignaled = bSignaled;
    }
    if (bSignaled) {
        DispatcherWake().notify_all();
    }
}

void SetError(DWORD dwError) {
    tl_dwLastError = dwError;
}

DWORD ErrorFromErrno(int nErrno) {
    switch (nErrno) {
        case 0: return 0;
        case ENOENT: return 2;          // ERROR_FILE_NOT_FOUND
        case ENOTDIR: return 3;         // ERROR_PATH_NOT_FOUND
        case EMFILE:
        case ENFILE: return 4;          // ERROR_TOO_MANY_OPEN_FILES
        case EACCES:
        case EPERM:
        case EROFS:
        case EISDIR: return 5;          // ERROR_ACCESS_DENIED
        case EBADF: return 6;           // ERROR_INVALID_HANDLE
        case ENOMEM: return 8;          // ERROR_NOT_ENOUGH_MEMORY
        case EEXIST: return 183;        // ERROR_ALREADY_EXISTS
        case ENOSPC: return 112;        // ERROR_DISK_FULL
        case EINVAL: return 87;         // ERROR_INVALID_PARAMETER
        case ENAMETOOLONG: return 206;  // ERROR_FILENAME_EXCED_RANGE
        case ENOTEMPTY: return 145;     // ERROR_DIR_NOT_EMPTY
        case EBUSY: return 32;          // ERROR_SHARING_VIOLATION
        case EIO: return 1117;          // ERROR_IO_DEVICE
        default: return 31;             // ERROR_GEN_FAILURE
    }
}

}  // namespace Win32Posix

using namespace Win32Posix;

extern "C" DWORD GetLastError() {
    return tl_dwLastError;
}

extern "C" void SetLastError(DWORD dwErrCode) {
    tl_dwLastError = dwErrCode;
}

extern "C" BOOL CloseHandle(HANDLE hObject) {
    if (!Handle_Remove(hObject)) {
        SetError(6);  // ERROR_INVALID_HANDLE
        return FALSE;
    }
    return TRUE;
}

namespace {

// Whether the object satisfies a wait now; the dispatcher lock is held. Objects that are not
// waitable (files, mappings) are signaled, as a file handle with no pending I/O is.
bool IsSignaled(const Object* pObject) {
    return !pObject->bWaitable || pObject->bSignaled;
}

// The wait itself consumes an auto-reset event.
void Consume(Object* pObject) {
    if (pObject->bWaitable && !pObject->bManualReset) {
        pObject->bSignaled = false;
    }
}

DWORD WaitOn(DWORD nCount, const HANDLE* pHandles, BOOL bWaitAll, DWORD dwMilliseconds) {
    if (nCount == 0 || nCount > 64 || pHandles == nullptr) {
        SetError(87);
        return WAIT_FAILED;
    }
    std::vector<std::shared_ptr<Object>> aObjects(nCount);
    for (DWORD i = 0; i < nCount; ++i) {
        // GetCurrentProcess and GetCurrentThread: never signaled while the caller runs.
        if (pHandles[i] == (HANDLE)(intptr_t)-1 || pHandles[i] == (HANDLE)(intptr_t)-2) {
            aObjects[i] = std::make_shared<Object>(OBJECT_OTHER);
            aObjects[i]->bWaitable = true;
            continue;
        }
        aObjects[i] = Handle_Get(pHandles[i]);
        if (!aObjects[i]) {
            SetError(6);
            return WAIT_FAILED;
        }
    }
    auto tDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(dwMilliseconds);
    std::unique_lock<std::mutex> tLock(DispatcherLock());
    for (;;) {
        if (bWaitAll) {
            bool bAll = true;
            for (auto& p : aObjects) {
                bAll = bAll && IsSignaled(p.get());
            }
            if (bAll) {
                for (auto& p : aObjects) {
                    Consume(p.get());
                }
                return WAIT_OBJECT_0;
            }
        } else {
            for (DWORD i = 0; i < nCount; ++i) {
                if (IsSignaled(aObjects[i].get())) {
                    Consume(aObjects[i].get());
                    return WAIT_OBJECT_0 + i;
                }
            }
        }
        if (dwMilliseconds == 0) {
            return WAIT_TIMEOUT;
        }
        if (dwMilliseconds == INFINITE) {
            DispatcherWake().wait(tLock);
        } else if (DispatcherWake().wait_until(tLock, tDeadline) == std::cv_status::timeout) {
            // one last look: a signal may have raced the timeout
            dwMilliseconds = 0;
        }
    }
}

}  // namespace

extern "C" DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) {
    return WaitOn(1, &hHandle, FALSE, dwMilliseconds);
}

extern "C" DWORD WaitForMultipleObjects(DWORD nCount, const HANDLE* lpHandles, BOOL bWaitAll, DWORD dwMilliseconds) {
    return WaitOn(nCount, lpHandles, bWaitAll, dwMilliseconds);
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
