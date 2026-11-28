// Storm's Win32-on-Mac layer: movable-memory handles, the heap and virtual-memory
// calls, and the memory primitives, 1.14d Intel Mac band 000289b8..00028e09. See
// Win32Mac.h for what is and is not known about the original file boundaries.
//
// A handle (the GDI objects use them) is a pointer to a master-pointer slot; the
// slot points at a block that starts with a 16-byte header. The slots live in a
// chain of 15-slot blocks.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include <string.h>

#define HANDLE_SIGNATURE 0x484E444C  // 'HNDL'
#define HANDLE_TYPE_ANY 0x2A2A2A2A   // '****'

struct HandleHeader {
    DWORD dwSignature;  // 0x00  HANDLE_SIGNATURE
    DWORD dwType;       // 0x04
    LONG nLockCount;    // 0x08
    DWORD dwSize;       // 0x0C  bytes of data after the header
};

struct MasterPointerBlock {
    HandleHeader* apSlot[15];   // 0x00
    MasterPointerBlock* pNext;  // 0x3C
};


// 1.14d mac: 003f1130
static MasterPointerBlock* s_pMasterPointers;

// 1.14d mac: 000289b8
void Mem_DestroyMasterPointer(void) {
    MasterPointerBlock* pBlock = s_pMasterPointers;
    while (pBlock != NULL) {
        MasterPointerBlock* pNext = pBlock->pNext;
        MemFree(pBlock, 0);
        pBlock = pNext;
    }
}

static void Mem_Priv_FreeMasterPointer(HandleHeader** pMasterPtr);

// 1.14d mac: 000289ee
// Takes a free slot (adding a block when every slot is in use), allocates the data
// with its header, and copies pInitData in when given; without it the data is zeroed.
HandleHeader** Mem_HandleAlloc(DWORD dwType, size_t nSize, const void* pInitData) {
    size_t nAllocSize = nSize + sizeof(HandleHeader);
    if (s_pMasterPointers == NULL) {
        s_pMasterPointers = (MasterPointerBlock*)MemAlloc(sizeof(MasterPointerBlock), 0x100);
        if (s_pMasterPointers == NULL) {
            return NULL;
        }
    }

    HandleHeader** pMasterPtr = NULL;
    MasterPointerBlock* pBlock = s_pMasterPointers;
    for (;;) {
        for (int i = 0; i < 15; ++i) {
            if (pBlock->apSlot[i] == NULL) {
                pMasterPtr = &pBlock->apSlot[i];
                break;
            }
        }
        if (pMasterPtr != NULL) {
            break;
        }
        if (pBlock->pNext == NULL) {
            break;
        }
        pBlock = pBlock->pNext;
    }

    if (pMasterPtr == NULL) {
        if (pBlock == NULL) {
            return NULL;
        }
        pBlock->pNext = (MasterPointerBlock*)MemAlloc(sizeof(MasterPointerBlock), 0x100);
        if (pBlock->pNext == NULL) {
            return NULL;
        }
        pMasterPtr = &pBlock->pNext->apSlot[0];
    }

    *pMasterPtr = (HandleHeader*)MemAlloc(nAllocSize, pInitData != NULL ? 0 : 0x100);
    if (*pMasterPtr == NULL) {
        Mem_Priv_FreeMasterPointer(pMasterPtr);
        return NULL;
    }
    if (pMasterPtr != NULL) {
        (*pMasterPtr)->dwSignature = HANDLE_SIGNATURE;
        (*pMasterPtr)->dwType = dwType;
        (*pMasterPtr)->nLockCount = 0;
        (*pMasterPtr)->dwSize = nSize;
        if (pInitData != NULL) {
            SMemCopy(*pMasterPtr + 1, pInitData, nSize);
        }
    }
    return pMasterPtr;
}

// 1.14d mac: 00028b01
// Clears the slot; a block left with no slot in use is unlinked and freed. The
// emptiness test compares the slots' addresses, not their contents, so it never
// finds a block empty and blocks are never freed here.
static void Mem_Priv_FreeMasterPointer(HandleHeader** pMasterPtr) {
    MasterPointerBlock** ppBlock = &s_pMasterPointers;
    MasterPointerBlock* pBlock = NULL;
    MasterPointerBlock* pPrev;
    BOOL bEmpty;
    BOOL bFound;
    do {
        pPrev = pBlock;
        pBlock = *ppBlock;
        if (pBlock == NULL) {
            return;
        }
        bEmpty = 1;
        bFound = 0;
        for (int i = 0; i < 15; ++i) {
            if (&pBlock->apSlot[i] != NULL) {
                bEmpty = 0;
            }
            if (&pBlock->apSlot[i] == pMasterPtr) {
                *pMasterPtr = NULL;
                bFound = 1;
            }
        }
        ppBlock = &pBlock->pNext;
    } while (!bFound);
    if (bEmpty) {
        if (pPrev != NULL) {
            pPrev->pNext = pBlock->pNext;
        } else {
            s_pMasterPointers = pBlock->pNext;
        }
        MemFree(pBlock, 0);
    }
}

// 1.14d mac: 00028b90
BOOL Mem_HandleFree(DWORD dwType, HandleHeader** hHandle) {
    if (dwType != HANDLE_TYPE_ANY && (*hHandle)->dwType != dwType) {
        return 0;
    }
    if (hHandle == NULL) {
        return 0;
    }
    HandleHeader* pHeader = *hHandle;
    if (pHeader->nLockCount != 0) {
        return 0;
    }
    if (pHeader != NULL) {
        MemFree(pHeader, 0);
    }
    Mem_Priv_FreeMasterPointer(hHandle);
    return 1;
}

// 1.14d mac: 00028bea
void* Mem_HandleLock(DWORD dwType, HandleHeader** hHandle) {
    if (dwType != HANDLE_TYPE_ANY && (*hHandle)->dwType != dwType) {
        return NULL;
    }
    if (hHandle == NULL) {
        return NULL;
    }
    HandleHeader* pHeader = *hHandle;
    if (pHeader->dwSize == 0) {
        return NULL;
    }
    ++pHeader->nLockCount;
    return pHeader + 1;
}

// 1.14d mac: 00028c26
// Returns whether the handle is still locked.
BOOL Mem_HandleUnLock(DWORD dwType, HandleHeader** hHandle) {
    if (dwType != HANDLE_TYPE_ANY && (*hHandle)->dwType != dwType) {
        return 0;
    }
    if (hHandle == NULL) {
        return 1;
    }
    if ((*hHandle)->dwSize == 0) {
        return 1;
    }
    return --(*hHandle)->nLockCount != 0;
}

// 1.14d mac: 00028c5e
DWORD Mem_HandleType(HandleHeader** hHandle) {
    if (hHandle == NULL) {
        return 0;
    }
    return (*hHandle)->dwType;
}

// 1.14d mac: 00028c73
LPVOID HeapAlloc(HANDLE hHeap, DWORD dwFlags, size_t dwBytes) {
    return Priv_Alloc(dwBytes);
}

// 1.14d mac: 00028c89
BOOL HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem) {
    Priv_Free(lpMem);
    return 1;
}

// 1.14d mac: 00028ca4
HANDLE GetProcessHeap(void) {
    return 0;
}

// 1.14d mac: 00028cab
// Only the three forms the game uses: reserve-no-access and commit-read-write of
// fresh memory (one allocation), and committing memory already reserved (a no-op).
LPVOID VirtualAlloc(LPVOID lpAddress, size_t dwSize, DWORD flAllocationType, DWORD flProtect) {
    if (lpAddress == NULL && flAllocationType == MEM_RESERVE && flProtect == PAGE_NOACCESS) {
        return MemAlloc(dwSize, 0);
    }
    if (lpAddress == NULL && flAllocationType == MEM_COMMIT &&
        (flProtect == PAGE_EXECUTE_READWRITE || flProtect == PAGE_READWRITE)) {
        return MemAlloc(dwSize, 0);
    }
    if (lpAddress != NULL && flAllocationType == MEM_COMMIT && flProtect == PAGE_READWRITE) {
        return lpAddress;
    }
    fp_fatal("bad parms to VirtualAlloc");
    return lpAddress;
}

// 1.14d mac: 00028d2e
BOOL VirtualFree(LPVOID lpAddress, size_t dwSize, DWORD dwFreeType) {
    if (lpAddress != NULL && dwSize == 0 && dwFreeType == MEM_RELEASE) {
        MemFree(lpAddress, 0);
        return 1;
    }
    return 0;
}

// 1.14d mac: 00028d67
BOOL VirtualUnlock(LPVOID lpAddress, size_t dwSize) {
    return 1;
}

// 1.14d mac: 00028d71
// Returns 0. 112 call sites across the game: IsBadCodePtr, whose argument the Mac layer never reads
// (the shared code calls it as IsBadCodePtr(FARPROC)).
BOOL IsBadCodePtr(FARPROC lpfn) {
    return 0;
}

// 1.14d mac: 00028d78
BOOL IsBadReadPtr(const void* lp, size_t ucb) {
    return 0;
}

namespace Storm { namespace Source { namespace SMem {
// 1.14d mac: 00028d7f
void SMemZero(void* pDest, size_t nSize) {
    bzero(pDest, nSize);
}

// 1.14d mac: 00028d9c
void SMemFill(void* pDest, size_t nSize, BYTE nValue) {
    memset(pDest, nValue, nSize);
}

// 1.14d mac: 00028dc1
void SMemCopy(void* pDest, void* pSource, size_t nSize) {
    memcpy(pDest, pSource, nSize);
}
} } }

// 1.14d mac: 00028de5
void MoveMemory(void* pDest, const void* pSource, size_t nSize) {
    memmove(pDest, pSource, nSize);
}

#endif  // D2_OS_MAC
