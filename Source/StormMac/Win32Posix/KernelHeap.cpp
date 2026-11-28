// Win32Posix: the process heap and the global memory calls (Win32Mac's Win32_Heap.cpp and the
// Priv_ allocator of Win32_Priv.cpp).
//
// The process heap is the C heap. A global memory block is a C heap block that is its own
// handle, as Win32Mac's are: GlobalLock answers the block, and a GMEM_MOVEABLE block never
// moves. The clipboard (Win32PosixUser) takes such a block from SetClipboardData and owns it
// from then on, as Windows does.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <stdlib.h>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

using namespace Win32Posix;

namespace {

constexpr DWORD kHeapZeroMemory = 0x08;
constexpr UINT kGmemZeroInit = 0x40;

HANDLE ProcessHeap() {
    static HANDLE hHeap = Handle_Insert(std::make_shared<Object>(OBJECT_OTHER));
    return hHeap;
}

}  // namespace

extern "C" HANDLE GetProcessHeap() {
    return ProcessHeap();
}

extern "C" LPVOID HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) {
    (void)hHeap;
    void* p = (dwFlags & kHeapZeroMemory) ? calloc(1, dwBytes ? dwBytes : 1) : malloc(dwBytes ? dwBytes : 1);
    if (!p) {
        SetError(8);
    }
    return p;
}

extern "C" BOOL HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem) {
    (void)hHeap;
    (void)dwFlags;
    free(lpMem);
    return TRUE;
}

HGLOBAL GlobalAlloc(UINT uFlags, SIZE_T dwBytes) {
    void* p = (uFlags & kGmemZeroInit) ? calloc(1, dwBytes ? dwBytes : 1) : malloc(dwBytes ? dwBytes : 1);
    if (!p) {
        SetError(8);
    }
    return (HGLOBAL)p;
}

LPVOID GlobalLock(HGLOBAL hMem) {
    if (!hMem) {
        SetError(6);
    }
    return (LPVOID)hMem;
}

BOOL GlobalUnlock(HGLOBAL hMem) {
    (void)hMem;
    SetError(0);  // NO_ERROR with FALSE: the block is no longer locked
    return FALSE;
}

HGLOBAL GlobalFree(HGLOBAL hMem) {
    free(hMem);
    return nullptr;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
