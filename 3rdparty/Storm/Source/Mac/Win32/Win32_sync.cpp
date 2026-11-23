// Storm's Win32-on-Mac layer: interlocked operations and critical sections,
// 1.14d Intel Mac band 0002ae74..0002af55. See Win32Mac.h for what is and is not
// known about the original file boundaries.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

// 1.14d mac: 0002ae74
LONG InterlockedDecrement(LPLONG pnValue) {
    return __sync_sub_and_fetch(pnValue, 1);
}

// 1.14d mac: 0002ae86
LONG InterlockedIncrement(LPLONG pnValue) {
    return __sync_add_and_fetch(pnValue, 1);
}

// 1.14d mac: 0002ae98
LONG InterlockedExchange(LPLONG pnTarget, LONG nValue) {
    LONG nOld;
    do {
        nOld = *pnTarget;
    } while (__sync_val_compare_and_swap(pnTarget, nOld, nValue) != nOld);
    return nOld;
}

// 1.14d mac: 0002aeb3
LONG InterlockedCompareExchange(LPLONG pnDest, LONG nExchange, LONG nComparand) {
    return __sync_val_compare_and_swap(pnDest, nComparand, nExchange);
}

// 1.14d mac: 0002aec5
// The same body as InterlockedCompareExchange; the network code calls it on its
// pointer-sized state words. Name not recovered (Ghidra: FUN_0002aec5).
LONG FUN_0002aec5(LPLONG pnDest, LONG nExchange, LONG nComparand) {
    return __sync_val_compare_and_swap(pnDest, nComparand, nExchange);
}

// 1.14d mac: 0002aeda
// Win32 critical sections are re-entrant, so the mutex is a recursive one.
void InitializeCriticalSection(LPCRITICAL_SECTION pCriticalSection) {
    pthread_mutexattr_t tAttr;
    pthread_mutexattr_init(&tAttr);
    pthread_mutexattr_settype(&tAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(pCriticalSection, &tAttr);
    pthread_mutexattr_destroy(&tAttr);
}

// 1.14d mac: 0002af3a
void EnterCriticalSection(LPCRITICAL_SECTION pCriticalSection) {
    pthread_mutex_lock(pCriticalSection);
}

// 1.14d mac: 0002af43
void LeaveCriticalSection(LPCRITICAL_SECTION pCriticalSection) {
    pthread_mutex_unlock(pCriticalSection);
}

// 1.14d mac: 0002af4c
void DeleteCriticalSection(LPCRITICAL_SECTION pCriticalSection) {
    pthread_mutex_destroy(pCriticalSection);
}

#endif  // D2_OS_MAC
