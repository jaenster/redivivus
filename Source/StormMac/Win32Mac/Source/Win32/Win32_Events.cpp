// StormMac, Win32 Mac layer: events and waits, 1.14d Intel Mac 000216b0..00021a94.
// The original file is unknown: no 1.14d string or initialiser names it, and in the
// named PPC Storm 1.10 Beta (Storm_(Carbon), traceback names) this code is a translation
// unit without a static initialiser, so it carries no file name there either.
// The PPC Storm 1.10 Beta has the same functions in the same order (CreateEvent,
// SignalHandleObject, KillWaitingEvent, SetEvent, ResetEvent, WaitForSingleObject,
// WaitForMultipleObjects, MsgWaitForMultipleObjects) as a unit linked just before
// Win32_Files.cpp, and not part of it: inside one MWCC unit they would follow the file
// functions, not precede them.
//
// Every handle carries a signaled flag under its mutex and a condition broadcast
// when it is set. An event is the bare object; a wait on any handle type (a thread
// signals its own when it ends) goes through the same pair.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <string.h>
#include <time.h>

// 1.14d mac: 000216b0
HANDLE CreateEventA(LPVOID lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName) {
    HANDLE hEvent = 0;
    if (lpEventAttributes == NULL) {
        char* szName = NULL;
        if (lpName != NULL) {
            size_t nSize = strlen(lpName) + 1;
            szName = (char*)MemAlloc(nSize, 0);
            if (szName == NULL) {
                SetLastError(8);
                return 0;
            }
            memcpy(szName, lpName, nSize);
        }
        hEvent = AllocWin32Handle(WIN32MAC_HANDLE_EVENT);
        Win32HandleObject* pEvent = GetWin32HandlePtr(hEvent);
        pEvent->szEventName = szName;
        pEvent->bSignaled = bInitialState != 0;
        pEvent->bManualReset = bManualReset;
        SetLastError(0);
    }
    return hEvent;
}

// 1.14d mac: 00021748
// Sets the signaled state; setting it wakes every waiter.
BOOL Event_SetSignaled(HANDLE hEvent, BOOL bSignaled) {
    Win32HandleObject* pEvent = GetWin32HandlePtr(hEvent);
    pthread_mutex_lock(&pEvent->tMutex);
    if (pEvent->bSignaled == bSignaled) {
        pthread_mutex_unlock(&pEvent->tMutex);
        return 1;
    }
    pEvent->bSignaled = bSignaled != 0;
    BOOL bResult = 1;
    if (bSignaled) {
        bResult = pthread_cond_broadcast(&pEvent->tCond) == 0;
    }
    pthread_mutex_unlock(&pEvent->tMutex);
    return bResult;
}

// 1.14d mac: 000217b5
// Marks the object closed and wakes its waiters, which then fail.
void Event_Close(HANDLE hEvent) {
    Win32HandleObject* pEvent = GetWin32HandlePtr(hEvent);
    if (hEvent != 0) {
        pEvent->bClosed = 1;
        pthread_cond_broadcast(&pEvent->tCond);
    }
}

// 1.14d mac: 000217de
BOOL SetEvent(HANDLE hEvent) {
    return Event_SetSignaled(hEvent, 1);
}

// 1.14d mac: 000217fc
BOOL ResetEvent(HANDLE hEvent) {
    return Event_SetSignaled(hEvent, 0);
}

// 1.14d mac: 0002181a
// The tick count read first is never used; the call is in the asm.
DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) {
    DWORD dwStart = GetTickCount();
    Win32HandleObject* pObject = GetWin32HandlePtr(hHandle);
    DWORD dwResult = WAIT_FAILED;
    if (hHandle != 0 && !pObject->bClosed) {
        pthread_mutex_lock(&pObject->tMutex);
        if (!pObject->bSignaled) {
            if (dwMilliseconds == INFINITE) {
                pthread_cond_wait(&pObject->tCond, &pObject->tMutex);
            } else {
                struct timespec tTimeout = {0, 0};
                tTimeout.tv_sec = dwMilliseconds / 1000;
                tTimeout.tv_nsec = (dwMilliseconds % 1000) * 1000000;
                pthread_cond_timedwait_relative_np(&pObject->tCond, &pObject->tMutex, &tTimeout);
            }
        }
        pObject = GetWin32HandlePtr(hHandle);
        dwResult = WAIT_OBJECT_0;
        if (!pObject->bSignaled) {
            dwResult = pObject->bClosed ? WAIT_FAILED : WAIT_TIMEOUT;
        }
        pObject = GetWin32HandlePtr(hHandle);
        if (pObject->bSignaled && pObject->dwType == WIN32MAC_HANDLE_EVENT && !pObject->bManualReset) {
            pObject->bSignaled = 0;
        }
        pthread_mutex_unlock(&pObject->tMutex);
    }
    return dwResult;
}

// 1.14d mac: 0002190a
// Polls the handles, yielding between rounds, until one (or, waiting for all, all)
// is signaled or the time is up. The wake mask is not read. With INFINITE the end
// time is never set, yet the loop still compares the tick count with it: the asm
// carries the stale register into the comparison, so an infinite wait can end as
// a timeout.
DWORD MsgWaitForMultipleObjects(DWORD nCount, const HANDLE* pHandles, BOOL bWaitAll, DWORD dwMilliseconds,
                                DWORD dwWakeMask) {
    if (nCount == 1) {
        return WaitForSingleObject(pHandles[0], dwMilliseconds);
    }
    for (DWORD i = 0; i < nCount; ++i) {
        Win32HandleObject* pObject = GetWin32HandlePtr(pHandles[i]);
        if (pObject == NULL || pObject->bClosed) {
            return WAIT_FAILED;
        }
    }
    DWORD dwEnd;
    if (dwMilliseconds != INFINITE) {
        dwEnd = GetTickCount() + dwMilliseconds;
    }
    int nNeeded = bWaitAll ? (short)nCount : 1;
    DWORD nFirstSignaled = 0;
    bool bTimedOut;
    do {
        int nSignaled = 0;
        bool bAllValid = true;
        for (DWORD i = 0; i < nCount; ++i) {
            Win32HandleObject* pObject = GetWin32HandlePtr(pHandles[i]);
            if (pObject == NULL || pObject->bClosed) {
                bAllValid = false;
                break;
            }
            pthread_mutex_lock(&pObject->tMutex);
            if (pObject->bSignaled) {
                if (nFirstSignaled == 0) {
                    nFirstSignaled = i + 1;
                }
                ++nSignaled;
            }
            pthread_mutex_unlock(&pObject->tMutex);
        }
        bTimedOut = false;
        if (nSignaled >= nNeeded) {
            break;
        }
        bTimedOut = bAllValid;
        if (dwMilliseconds != INFINITE && (int)(GetTickCount() - dwEnd) >= 0) {
            break;
        }
        pthread_yield_np();
    } while (GetTickCount() < dwEnd);
    for (DWORD i = 0; i < nCount; ++i) {
        Win32HandleObject* pObject = GetWin32HandlePtr(pHandles[i]);
        if (pObject == NULL || pObject->bClosed) {
            return WAIT_FAILED;
        }
        pthread_mutex_lock(&pObject->tMutex);
        if (pObject->bSignaled && pObject->dwType == WIN32MAC_HANDLE_EVENT && !pObject->bManualReset) {
            pObject->bSignaled = 0;
        }
        pthread_mutex_unlock(&pObject->tMutex);
    }
    return bTimedOut ? WAIT_TIMEOUT : nFirstSignaled - 1;
}

#endif  // D2_OS_MAC
