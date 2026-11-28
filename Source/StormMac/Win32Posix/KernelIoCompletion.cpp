// Win32Posix: the I/O completion port QServerNT's workers pump. Win32Mac has none (the Mac
// game has no QServer).
//
// A port is a queue of completion packets under its own lock. PostQueuedCompletionStatus and
// the Sockets units' finished overlapped operations (CompletionPort_Post) enqueue; each
// GetQueuedCompletionStatus takes one, first in first out. CreateIoCompletionPort on a handle
// (a socket, by its value) records the port and key the Sockets units post its completions
// with.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <deque>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

namespace {

using namespace Win32Posix;

struct Packet {
    DWORD dwBytes;
    ULONG_PTR nKey;
    LPOVERLAPPED pOverlapped;
};

struct PortObject : Object {
    PortObject() : Object(OBJECT_COMPLETION_PORT) {}
    std::mutex tLock;
    std::condition_variable tReady;
    std::deque<Packet> aQueue;
};

struct Association {
    HANDLE hPort;
    ULONG_PTR nKey;
};

std::mutex& AssociationLock() {
    static std::mutex* pLock = new std::mutex;
    return *pLock;
}

std::map<uintptr_t, Association>& Associations() {
    static auto* pMap = new std::map<uintptr_t, Association>;
    return *pMap;
}

}  // namespace

namespace Win32Posix {

bool CompletionPort_Post(HANDLE hPort, DWORD dwBytes, ULONG_PTR nKey, LPOVERLAPPED pOverlapped) {
    auto pPort = Handle_GetAs<PortObject>(hPort, OBJECT_COMPLETION_PORT);
    if (!pPort) {
        return false;
    }
    {
        std::lock_guard<std::mutex> tGuard(pPort->tLock);
        pPort->aQueue.push_back(Packet{dwBytes, nKey, pOverlapped});
    }
    pPort->tReady.notify_one();
    return true;
}

bool CompletionPort_Lookup(HANDLE hFile, HANDLE* phPort, ULONG_PTR* pnKey) {
    std::lock_guard<std::mutex> tGuard(AssociationLock());
    auto it = Associations().find((uintptr_t)hFile);
    if (it == Associations().end()) {
        return false;
    }
    *phPort = it->second.hPort;
    *pnKey = it->second.nKey;
    return true;
}

}  // namespace Win32Posix

HANDLE CreateIoCompletionPort(HANDLE FileHandle, HANDLE ExistingCompletionPort, ULONG_PTR CompletionKey,
                              DWORD NumberOfConcurrentThreads) {
    (void)NumberOfConcurrentThreads;
    HANDLE hPort = ExistingCompletionPort;
    if (hPort) {
        if (!Handle_GetAs<PortObject>(hPort, OBJECT_COMPLETION_PORT)) {
            SetError(6);
            return 0;
        }
    } else {
        hPort = Handle_Insert(std::make_shared<PortObject>());
        if (!hPort) {
            return 0;
        }
    }
    if (FileHandle != INVALID_HANDLE_VALUE) {
        std::lock_guard<std::mutex> tGuard(AssociationLock());
        Associations()[(uintptr_t)FileHandle] = Association{hPort, CompletionKey};
    }
    return hPort;
}

BOOL GetQueuedCompletionStatus(HANDLE CompletionPort, LPDWORD lpNumberOfBytesTransferred, ULONG_PTR* lpCompletionKey,
                               LPOVERLAPPED* lpOverlapped, DWORD dwMilliseconds) {
    auto pPort = Handle_GetAs<PortObject>(CompletionPort, OBJECT_COMPLETION_PORT);
    *lpOverlapped = nullptr;
    if (!pPort) {
        SetError(6);
        return FALSE;
    }
    std::unique_lock<std::mutex> tLock(pPort->tLock);
    auto bReady = [&] { return !pPort->aQueue.empty(); };
    if (dwMilliseconds == INFINITE) {
        pPort->tReady.wait(tLock, bReady);
    } else if (!pPort->tReady.wait_for(tLock, std::chrono::milliseconds(dwMilliseconds), bReady)) {
        SetError(WAIT_TIMEOUT);
        return FALSE;
    }
    Packet tPacket = pPort->aQueue.front();
    pPort->aQueue.pop_front();
    *lpNumberOfBytesTransferred = tPacket.dwBytes;
    *lpCompletionKey = tPacket.nKey;
    *lpOverlapped = tPacket.pOverlapped;
    return TRUE;
}

BOOL PostQueuedCompletionStatus(HANDLE CompletionPort, DWORD dwNumberOfBytesTransferred, ULONG_PTR dwCompletionKey,
                                LPOVERLAPPED lpOverlapped) {
    if (!CompletionPort_Post(CompletionPort, dwNumberOfBytesTransferred, dwCompletionKey, lpOverlapped)) {
        SetError(6);
        return FALSE;
    }
    return TRUE;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
