// Win32Posix: threads, Sleep and the critical sections (Win32Mac's Win32_Threads.cpp and the
// critical sections of Win32_Process.cpp).
//
// A thread is a pthread behind a waitable object that is signaled when the thread ends; the
// thread holds its own reference, so its handle may be closed while it runs. The thread id is
// the pthread's Mach port name: 32 bits, nonzero and unique while the thread lives, as a
// Windows thread id is. A critical section is re-entrant, as on Windows and in Win32Mac: a
// recursive pthread mutex, which the SDK record holds in LockSemaphore. A section the game
// zero-initialises and enters without InitializeCriticalSection gets its mutex on first use.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

using namespace Win32Posix;

namespace {

struct ThreadObject : Object {
    ThreadObject() : Object(OBJECT_THREAD) {
        bWaitable = true;
        bManualReset = true;
    }
    DWORD (*pfnStart)(LPVOID) = nullptr;
    unsigned (*pfnStartEx)(void*) = nullptr;
    LPVOID pParameter = nullptr;
    DWORD dwExitCode = 0x103;  // STILL_ACTIVE
    DWORD dwThreadId = 0;
    int nPriority = 0;
};

thread_local ThreadObject* tl_pThread = nullptr;
thread_local int tl_nPriority = 0;  // a thread the layer did not start (the main one)

void FinishThread(ThreadObject* pThread, DWORD dwExitCode) {
    {
        std::lock_guard<std::mutex> tGuard(DispatcherLock());
        pThread->dwExitCode = dwExitCode;
    }
    Dispatcher_Signal(pThread, true);
}

struct StartBlock {
    std::shared_ptr<ThreadObject> pThread;
};

void* ThreadEntry(void* pArg) {
    StartBlock* pStart = (StartBlock*)pArg;
    std::shared_ptr<ThreadObject> pThread = std::move(pStart->pThread);
    delete pStart;
    tl_pThread = pThread.get();
    DWORD dwExitCode = pThread->pfnStart ? pThread->pfnStart(pThread->pParameter)
                                         : (DWORD)pThread->pfnStartEx(pThread->pParameter);
    FinishThread(pThread.get(), dwExitCode);
    tl_pThread = nullptr;
    return nullptr;
}

HANDLE StartThread(std::shared_ptr<ThreadObject> pThread, size_t nStackSize, LPDWORD pdwThreadId) {
    HANDLE hThread = Handle_Insert(pThread);
    if (!hThread) {
        return 0;
    }
    pthread_attr_t tAttr;
    pthread_attr_init(&tAttr);
    pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED);
    // Windows rounds the stack up and never goes below its 1 MB default for the game's
    // threads; a pthread's 512 KB default is too small for the game's deep frames.
    size_t nStack = nStackSize < 0x100000 ? 0x100000 : (nStackSize + 0xFFFF) & ~(size_t)0xFFFF;
    pthread_attr_setstacksize(&tAttr, nStack);
    StartBlock* pStart = new StartBlock{pThread};
    pthread_t tThread;
    int nError = pthread_create(&tThread, &tAttr, ThreadEntry, pStart);
    pthread_attr_destroy(&tAttr);
    if (nError != 0) {
        delete pStart;
        Handle_Remove(hThread);
        SetError(ErrorFromErrno(nError));
        return 0;
    }
    pThread->dwThreadId = pthread_mach_thread_np(tThread);
    if (pdwThreadId) {
        *pdwThreadId = pThread->dwThreadId;
    }
    return hThread;
}

}  // namespace

extern "C" HANDLE CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize,
                               LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags,
                               LPDWORD lpThreadId) {
    (void)lpThreadAttributes;
    (void)dwCreationFlags;  // CREATE_SUSPENDED: the layer has no ResumeThread, so it starts now
    auto pThread = std::make_shared<ThreadObject>();
    pThread->pfnStart = lpStartAddress;
    pThread->pParameter = lpParameter;
    return StartThread(pThread, dwStackSize, lpThreadId);
}

uintptr_t _beginthreadex(void* lpSecurity, unsigned nStackSize, unsigned (*pfnStart)(void*), void* pArgList,
                         unsigned nInitFlag, unsigned* pThreadId) {
    (void)lpSecurity;
    (void)nInitFlag;
    auto pThread = std::make_shared<ThreadObject>();
    pThread->pfnStartEx = pfnStart;
    pThread->pParameter = pArgList;
    DWORD dwThreadId = 0;
    HANDLE hThread = StartThread(pThread, nStackSize, &dwThreadId);
    if (pThreadId) {
        *pThreadId = dwThreadId;
    }
    return (uintptr_t)hThread;
}

void ExitThread(DWORD dwExitCode) {
    if (tl_pThread) {
        FinishThread(tl_pThread, dwExitCode);
        tl_pThread = nullptr;
    }
    pthread_exit(nullptr);
}

extern "C" DWORD GetCurrentThreadId() {
    return pthread_mach_thread_np(pthread_self());
}

HANDLE GetCurrentThread(void) {
    return (HANDLE)(intptr_t)-2;
}

BOOL SetThreadPriority(HANDLE hThread, int nPriority) {
    if (hThread == GetCurrentThread()) {
        if (tl_pThread) {
            tl_pThread->nPriority = nPriority;
        } else {
            tl_nPriority = nPriority;
        }
        return TRUE;
    }
    auto pThread = Handle_GetAs<ThreadObject>(hThread, OBJECT_THREAD);
    if (!pThread) {
        SetError(6);
        return FALSE;
    }
    pThread->nPriority = nPriority;
    return TRUE;
}

int GetThreadPriority(HANDLE hThread) {
    if (hThread == GetCurrentThread()) {
        return tl_pThread ? tl_pThread->nPriority : tl_nPriority;
    }
    auto pThread = Handle_GetAs<ThreadObject>(hThread, OBJECT_THREAD);
    if (!pThread) {
        SetError(6);
        return 0x7FFFFFFF;  // THREAD_PRIORITY_ERROR_RETURN
    }
    return pThread->nPriority;
}

namespace Win32Posix {
bool Thread_ExitCode(HANDLE hThread, LPDWORD pdwExitCode) {
    auto pThread = Handle_GetAs<ThreadObject>(hThread, OBJECT_THREAD);
    if (!pThread) {
        return false;
    }
    std::lock_guard<std::mutex> tGuard(DispatcherLock());
    *pdwExitCode = pThread->dwExitCode;
    return true;
}
}  // namespace Win32Posix

extern "C" void Sleep(DWORD dwMilliseconds) {
    if (dwMilliseconds == 0) {
        sched_yield();
        return;
    }
    if (dwMilliseconds == INFINITE) {
        for (;;) {
            pause();
        }
    }
    struct timespec tLeft = {(time_t)(dwMilliseconds / 1000), (long)(dwMilliseconds % 1000) * 1000000L};
    while (nanosleep(&tLeft, &tLeft) != 0 && errno == EINTR) {
    }
}

// ---- critical sections ----

namespace {

pthread_mutex_t* NewSectionMutex() {
    pthread_mutex_t* pMutex = new pthread_mutex_t;
    pthread_mutexattr_t tAttr;
    pthread_mutexattr_init(&tAttr);
    pthread_mutexattr_settype(&tAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(pMutex, &tAttr);
    pthread_mutexattr_destroy(&tAttr);
    return pMutex;
}

pthread_mutex_t* SectionMutex(LPCRITICAL_SECTION pSection) {
    HANDLE* ppSlot = &pSection->LockSemaphore;
    pthread_mutex_t* pMutex = (pthread_mutex_t*)__atomic_load_n(ppSlot, __ATOMIC_ACQUIRE);
    if (pMutex) {
        return pMutex;
    }
    pthread_mutex_t* pNew = NewSectionMutex();
    HANDLE pExpected = nullptr;
    if (__atomic_compare_exchange_n(ppSlot, &pExpected, (HANDLE)pNew, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
        return pNew;
    }
    pthread_mutex_destroy(pNew);
    delete pNew;
    return (pthread_mutex_t*)pExpected;
}

}  // namespace

extern "C" void InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection) {
    lpCriticalSection->DebugInfo = nullptr;
    lpCriticalSection->LockCount = -1;
    lpCriticalSection->RecursionCount = 0;
    lpCriticalSection->OwningThread = nullptr;
    lpCriticalSection->SpinCount = 0;
    lpCriticalSection->LockSemaphore = (HANDLE)NewSectionMutex();
}

extern "C" void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection) {
    pthread_mutex_lock(SectionMutex(lpCriticalSection));
    lpCriticalSection->OwningThread = (HANDLE)(uintptr_t)GetCurrentThreadId();
    lpCriticalSection->RecursionCount++;
    lpCriticalSection->LockCount++;
}

extern "C" void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection) {
    lpCriticalSection->LockCount--;
    if (--lpCriticalSection->RecursionCount == 0) {
        lpCriticalSection->OwningThread = nullptr;
    }
    pthread_mutex_unlock(SectionMutex(lpCriticalSection));
}

extern "C" void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection) {
    pthread_mutex_t* pMutex = (pthread_mutex_t*)lpCriticalSection->LockSemaphore;
    lpCriticalSection->LockSemaphore = nullptr;
    if (pMutex) {
        pthread_mutex_destroy(pMutex);
        delete pMutex;
    }
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
