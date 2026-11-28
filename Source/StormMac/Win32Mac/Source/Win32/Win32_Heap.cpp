// Storm's Win32-on-Mac layer: the heap the layer allocates from, 1.14d Intel Mac
// 0001e2fd..0001e482. malloc/calloc underneath with a running count of the bytes
// handed out (malloc_size), and Memory Manager handles counted the same way.
// The original file is unknown: no 1.14d string names it (see Source/StormMac/README.md).
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include <Carbon/Carbon.h>
#include <malloc/malloc.h>
#include <stdlib.h>
#include <string.h>

typedef void* (*PFN_ALLOC)(size_t nSize);
typedef void* (*PFN_CALLOC)(size_t nCount, size_t nSize);

// 1.14d mac: 003f0e60
static void* (*s_pfnMalloc)(size_t);
// 1.14d mac: 003f0e64
static void* (*s_pfnCalloc)(size_t, size_t);
// 1.14d mac: 003f0e68
// Bytes handed out and not yet freed.
static int s_nAllocatedBytes;

// 1.14d mac: 0001e2fd
void Mem_SetDefaultAllocators(void) {
    s_pfnMalloc = malloc;
    s_pfnCalloc = calloc;
}

// 1.14d mac: 0001e320
void Mem_Nop(void) {
}

// 1.14d mac: 0001e325
void* MemReAlloc(void* pMem, size_t nSize) {
    int nOldSize = 0;
    if (pMem != NULL) {
        nOldSize = malloc_size(pMem);
    }
    void* pNewMem = realloc(pMem, nSize);
    int nNewSize = malloc_size(pNewMem);
    s_nAllocatedBytes += nNewSize - nOldSize;
    return pNewMem;
}

// 1.14d mac: 0001e374
void MemFree(void* pMem, DWORD dwFlags) {
    s_nAllocatedBytes -= malloc_size(pMem);
    free(pMem);
}

// 1.14d mac: 0001e39a
void MemFreeHandle(Handle hMem) {
    s_nAllocatedBytes -= GetHandleSize(hMem);
    DisposeHandle(hMem);
}

// 1.14d mac: 0001e3c0
// Flag 0x100 zeroes the block, 0x4000 fills it with 0xEE.
void* MemAlloc(size_t nSize, DWORD dwFlags) {
    void* pMem;
    if (s_pfnCalloc == NULL || s_pfnMalloc == NULL) {
        s_pfnMalloc = malloc;
        s_pfnCalloc = calloc;
    }
    if ((dwFlags & 0x100) == 0) {
        pMem = malloc(nSize);
        if ((dwFlags & 0x4000) != 0) {
            memset(pMem, 0xEE, nSize);
        }
    } else {
        pMem = calloc(nSize, 1);
    }
    s_nAllocatedBytes += malloc_size(pMem);
    return pMem;
}

// 1.14d mac: 0001e45c
Handle MemAllocHandle(Size nSize) {
    Handle hMem = NewHandle(nSize);
    s_nAllocatedBytes += nSize;
    return hMem;
}

#endif  // D2_OS_MAC
