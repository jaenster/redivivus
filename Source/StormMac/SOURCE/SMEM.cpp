// Storm's SMem, 1.14d Intel Mac build (DiabloII_macho): the allocation entry points over the
// Win32-on-Mac layer's heap. Band 00018b0b..00018cdb. The Windows twin is
// Source/Storm/Source/SMem.cpp; this one has no heap manager of its own.
#include "Source/StormMac/Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <stdint.h>

#include "Source/StormMac/H/StormApi.h"

// 1.14d mac: 0001e325
void* MemReAlloc(void* pMem, size_t nSize);
// 1.14d mac: 0000ffaa
int SErrDisplayError(uint32_t nErrorCode, const char* pStackTrace, int32_t nTraceFlags, const char* pCustomMsg,
                     int32_t nMessageFlags, UINT nExitCode);
// 1.14d mac: 0002a41f
void ExitProcess(UINT uExitCode);

#define SMEM_FLAG_ZEROMEMORY 0x08
#define SMEM_FLAG_NOMOVE 0x10
#define ERROR_NOT_ENOUGH_MEMORY 8

// 1.14d mac: 003f0944
static bool s_bInitialized;
// 1.14d mac: 003f0948
// "Internal" / "Debug Memory" and "Protect Memory" of the registry.
int g_nDebugMemory;
// 1.14d mac: 003f094c
int g_nProtectMemory;

static void SMEM_DisplayOutOfMemoryError(const char* szFile, int nLine, uint32_t nBytes);

// 1.14d mac: 00018b0b
void* Storm::Source::SMem::SMemAlloc(uint32_t nBytes, const char* szFile, uint32_t nLine, uint32_t dwFlags) {
    uint32_t dwProtect = (dwFlags >> 18) & 0x400;
    uint32_t dwAllocFlags = dwProtect + 0x100;
    if (!(dwFlags & SMEM_FLAG_ZEROMEMORY)) {
        dwAllocFlags = dwProtect;
    }
    void* pMemory = MemAlloc(nBytes, dwAllocFlags);
    if (!pMemory) {
        if (*szFile != '\0') {
            SMEM_DisplayOutOfMemoryError(szFile, nLine, nBytes);
        } else {
            SMEM_DisplayOutOfMemoryError("SMemAlloc()", -1, nBytes);
        }
    }
    return pMemory;
}

// 1.14d mac: 00018b75
static void SMEM_DisplayOutOfMemoryError(const char* szFile, int nLine, uint32_t nBytes) {
    char szBuffer[256];
    SStrPrintf(szBuffer, sizeof(szBuffer), "%d bytes", nBytes);
    SErrDisplayError(ERROR_NOT_ENOUGH_MEMORY, szFile, nLine, szBuffer, 0, 1);
    ExitProcess(1);
}

// 1.14d mac: 00018c12
void* Storm::Source::SMem::SMemReAlloc(void* pMem, uint32_t nBytes, const char* szFile, int nLine, uint32_t dwFlags) {
    if (pMem && (dwFlags & SMEM_FLAG_NOMOVE)) {
        return NULL;
    }
    return MemReAlloc(pMem, nBytes);
}

// 1.14d mac: 00018c2d
int SMemDestroy(void) {
    s_bInitialized = false;
    return 1;
}

// 1.14d mac: 00018c44
int Storm::Source::SMem::SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags) {
    MemFree(pMem, 0);
    return 1;
}

// 1.14d mac: 00018c67
void Storm::Source::SMem::SMemInitialize(void) {
    if (!s_bInitialized) {
        SRegLoadValue("Internal", "Debug Memory", 0, &g_nDebugMemory);
        SRegLoadValue("Internal", "Protect Memory", 0, &g_nProtectMemory);
        s_bInitialized = true;
    }
}

#endif  // D2_OS_MAC
