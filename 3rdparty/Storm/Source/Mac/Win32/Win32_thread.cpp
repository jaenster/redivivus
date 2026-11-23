// Storm's Win32-on-Mac layer: threads and the clock, 1.14d Intel Mac band
// 0002bdea..0002c143. A thread is a kernel object (WIN32MAC_HANDLE_THREAD) that is
// signaled when the thread ends. See Win32Mac.h for what is and is not known about
// the original file boundaries.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <string.h>
#include <unistd.h>

static void THREAD_SignalExit(pthread_t pThread, DWORD dwExitCode);
static void* THREAD_StartRoutine(void* pArg);
static void THREAD_CleanupHandler(void* pArg);

// 1.14d mac: 0002bdea
// Only the plain form is supported: no security attributes and no creation flags
// (CREATE_SUSPENDED). The thread handle stays valid until it is closed, and a
// thread created with bCloseOnExit closes it itself when it ends.
static HANDLE THREAD_Create(LPVOID lpThreadAttributes, size_t dwStackSize, LPTHREAD_START_ROUTINE pfnStart,
                            LPVOID pParameter, DWORD dwCreationFlags, LPDWORD pThreadId, LPCSTR szName,
                            BOOL bCloseOnExit) {
    HANDLE hResult = 0;
    if (lpThreadAttributes == NULL && dwCreationFlags == 0) {
        HANDLE hThread = AllocWin32Handle(WIN32MAC_HANDLE_THREAD);
        GetWin32HandlePtr(hThread)->bCloseOnExit = bCloseOnExit;

        Win32ThreadStartBlock* pStart = (Win32ThreadStartBlock*)Priv_Alloc(sizeof(Win32ThreadStartBlock));
        pStart->pfnStart = pfnStart;
        pStart->pParameter = pParameter;
        strlcpy(pStart->szName, szName, sizeof(pStart->szName));

        size_t nStackSize = 0x10000;
        if (dwStackSize != 0) {
            nStackSize = dwStackSize < 0x4000 ? 0x4000 : dwStackSize;
        }

        pthread_attr_t tAttr;
        if (pthread_attr_init(&tAttr) == 0) {
            pthread_attr_setstacksize(&tAttr, nStackSize);
            pthread_t pThread;
            int nError = pthread_create(&pThread, &tAttr, THREAD_StartRoutine, pStart);
            pthread_attr_destroy(&tAttr);
            if (nError == 0) {
                GetWin32HandlePtr(hThread)->dwThread = (DWORD)pThread;
                *pThreadId = (DWORD)pThread;
                hResult = hThread;
            }
        }
    }
    return hResult;
}

// 1.14d mac: 0002bf09
// The Mac signature carries the thread's name, for pthread_setname_np.
HANDLE CreateThread(LPVOID lpThreadAttributes, size_t dwStackSize, LPTHREAD_START_ROUTINE pfnStart,
                    LPVOID pParameter, DWORD dwCreationFlags, LPDWORD pThreadId, LPCSTR szName) {
    return THREAD_Create(lpThreadAttributes, dwStackSize, pfnStart, pParameter, dwCreationFlags, pThreadId,
                         szName, 0);
}

// 1.14d mac: 0002bf49
// The CRT spelling of CreateThread, with the same extra name argument.
HANDLE _beginthreadex(LPVOID lpSecurity, size_t nStackSize, LPTHREAD_START_ROUTINE pfnStart, LPVOID pArgList,
                      DWORD dwInitFlag, LPDWORD pThreadId, LPCSTR szName) {
    return THREAD_Create(lpSecurity, nStackSize, pfnStart, pArgList, dwInitFlag, pThreadId, szName, 0);
}

// 1.14d mac: 0002bf89
void ExitThread(DWORD dwExitCode) {
    THREAD_SignalExit(pthread_self(), dwExitCode);
    pthread_exit((void*)dwExitCode);
}

// 1.14d mac: 0002bfa7
// Marks every handle on this pthread as ended with the exit code and signals it.
// HANDLE_FindThreadByPthread hands out a reference, released by the CloseHandle at
// the end of each pass; a handle created to close itself on exit is closed too.
static void THREAD_SignalExit(pthread_t pThread, DWORD dwExitCode) {
    HANDLE hThread;
    while ((hThread = HANDLE_FindThreadByPthread(pThread)) != 0) {
        Win32HandleObject* pObject = GetWin32HandlePtr(hThread);
        Event_SetSignaled(hThread, 1);
        if (!pObject->bExited) {
            pObject->dwThread = 0;
            pObject->dwExitCode = dwExitCode;
            pObject->bExited = 1;
            if (pObject->bCloseOnExit) {
                CloseHandle(hThread);
            }
        }
        CloseHandle(hThread);
    }
}

// 1.14d mac: 0002c019
BOOL SetThreadPriority(HANDLE hThread, int nPriority) {
    return 1;
}

// 1.14d mac: 0002c023
void THREAD_Yield(void) {
    pthread_yield_np();
}

// 1.14d mac: 0002c02c
DWORD GetCurrentThreadId(void) {
    return (DWORD)pthread_self();
}

// 1.14d mac: 0002c035
void Sleep(DWORD dwMilliseconds) {
    if (dwMilliseconds != 0) {
        usleep(dwMilliseconds * 1000);
    } else {
        pthread_yield_np();
    }
}

// 1.14d mac: 0002c05e
// The pthread entry: names the thread, runs the Win32 start routine with a
// cleanup handler pushed (so a pthread_cancel or pthread_exit still signals the
// handle), then signals the handle with the routine's result. It ends without a
// return statement: the asm sets no eax after the last call, and clang-425 does
// not treat falling off a non-void function as unreachable.
static void* THREAD_StartRoutine(void* pArg) {
    Win32ThreadStartBlock* pStart = (Win32ThreadStartBlock*)pArg;
    pthread_setname_np(pStart->szName);
    DWORD dwExitCode;
    pthread_cleanup_push(THREAD_CleanupHandler, pStart);
    dwExitCode = pStart->pfnStart(pStart->pParameter);
    pthread_cleanup_pop(0);
    Priv_Free(pStart);
    THREAD_SignalExit(pthread_self(), dwExitCode);
}

// 1.14d mac: 0002c0c8
static void THREAD_CleanupHandler(void* pArg) {
    Priv_Free(pArg);
    THREAD_SignalExit(pthread_self(), (DWORD)-1);
}

// 1.14d mac: 0002c0ee
DWORD GetTickCount(void) {
    return FPMilliseconds();
}

// 1.14d mac: 0002c0f7
DWORD TIME_GetMilliseconds(void) {
    return FPMilliseconds();
}

// 1.14d mac: 0002c100
BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount) {
    LARGE_INTEGER tCounter;
    MAC_GetMicroseconds64(&tCounter.LowPart);
    lpPerformanceCount->LowPart = tCounter.LowPart;
    lpPerformanceCount->HighPart = tCounter.HighPart;
    return 1;
}

// 1.14d mac: 0002c129
// The counter is in microseconds.
BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency) {
    lpFrequency->LowPart = 1000000;
    lpFrequency->HighPart = 0;
    return 1;
}

#endif  // D2_OS_MAC
