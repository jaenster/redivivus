// Storm's Win32-on-Mac layer: the allocator hooks of the layer's own heap. 1.14d Intel Mac
// 0001e2fd, in the band after the SVID.CPP code and before MemAlloc (0001e374); no
// __FILE__ literal names its file.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <stdlib.h>

// The allocator pair the layer's heap calls underneath (MemAlloc, MemFree).
// 1.14d mac: 003f0e60
void* (*g_pfnMalloc)(size_t);
// 1.14d mac: 003f0e64
void* (*g_pfnCalloc)(size_t, size_t);

// 1.14d mac: 0001e2fd
void Mem_SetDefaultAllocators(void) {
    g_pfnMalloc = malloc;
    g_pfnCalloc = calloc;
}

#endif  // D2_OS_MAC
