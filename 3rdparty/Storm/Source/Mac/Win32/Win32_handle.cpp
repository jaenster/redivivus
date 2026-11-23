// Storm's Win32-on-Mac layer: the kernel-object handle table, 1.14d Intel Mac band
// 000257bc..00025a77. See Win32Mac.h for what is and is not known about the
// original file boundaries.
//
// The table is a libc++ std::deque of 0xB8-byte slots (22 to a 4 KB block, which is
// the divide by 0x16 in every lookup). A handle is its slot index plus one. Free
// slots form a list through dwThread, headed by s_nFreeHandle; the table grows by
// 0x40 slots when the list is empty.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <deque>

// 1.14d mac: 003f0edc
static std::deque<Win32HandleObject> s_aHandles;
// 1.14d mac: 003f0ef4
// Zero in the image; initialised at run time, outside this band.
pthread_mutex_t g_tHandleTableMutex;
// 1.14d mac: 00397298
static DWORD s_nFreeHandle = (DWORD)-1;

// 1.14d mac: 000257bc
Win32HandleObject* GetWin32HandlePtr(HANDLE hHandle) {
    if (hHandle == 0) {
        return NULL;
    }
    Win32HandleObject* pObject = &s_aHandles[hHandle - 1];
    if (pObject->dwType == 0) {
        return NULL;
    }
    return pObject;
}

// 1.14d mac: 00025807
void FreeWin32Handle(HANDLE hHandle) {
    pthread_mutex_lock(&g_tHandleTableMutex);
    DWORD nIndex = hHandle - 1;
    Win32HandleObject& tObject = s_aHandles[nIndex];
    if (tObject.dwType == 1 || tObject.dwType == 4 || tObject.dwType == 7) {
        HANDLE_RemoveFromList(hHandle);
    }
    pthread_cond_destroy(&tObject.tCond);
    pthread_mutex_destroy(&tObject.tMutex);
    SMemZero(&tObject, sizeof(tObject));
    tObject.dwThread = s_nFreeHandle;
    s_nFreeHandle = nIndex;
    pthread_mutex_unlock(&g_tHandleTableMutex);
}

// 1.14d mac: 000258d5
HANDLE AllocWin32Handle(DWORD dwType) {
    if (dwType == 0) {
        return 0;
    }
    pthread_mutex_lock(&g_tHandleTableMutex);
    if (s_nFreeHandle == (DWORD)-1) {
        s_nFreeHandle = s_aHandles.size();
        s_aHandles.resize(s_aHandles.size() + 0x40);
        for (DWORD i = s_nFreeHandle; i < s_aHandles.size() - 1; ++i) {
            s_aHandles[i].dwThread = i + 1;
        }
        s_aHandles[s_aHandles.size() - 1].dwThread = (DWORD)-1;
    }
    DWORD nIndex = s_nFreeHandle;
    s_nFreeHandle = s_aHandles[nIndex].dwThread;
    pthread_mutex_unlock(&g_tHandleTableMutex);

    Win32HandleObject& tObject = s_aHandles[nIndex];
    pthread_mutex_init(&tObject.tMutex, NULL);
    pthread_cond_init(&tObject.tCond, NULL);
    tObject.nRefCount = 1;
    tObject.dwType = dwType;
    HANDLE hHandle = nIndex + 1;
    if (dwType == 1 || dwType == 4 || dwType == 7) {
        AddObjectToKernelPool(hHandle);
    }
    return hHandle;
}

#endif  // D2_OS_MAC
