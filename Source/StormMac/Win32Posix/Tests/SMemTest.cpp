// A stress test of Storm's heap (Source/Storm/Source/SMem.cpp) on a 64-bit target, run on
// the Win32Posix kernel. On a 64-bit target a block header names its heap by a slot in
// SMem's heap table and a free-list link is a block's offset from its heap, so nothing may
// depend on where a heap is mapped: on arm64 macOS every mapping lies above 4 GB.
//
//   ninja -C <modern build> d2_smem_test && <modern build>/d2_smem_test
//
// Every live block is mirrored in a shadow table with a fill byte, and every block's bytes,
// size and heap are checked after each round, so a corrupt free list (a block handed out
// twice, or one overlapping another) shows as a changed byte. Exits nonzero when a check fails.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <stdio.h>
#include <string.h>

#include "Source/Storm/Source/SMem.h"
#include "Source/Storm/Source/SErr.h"
#include "Source/Storm/Source/SReg.h"
#include "Source/Storm/Source/SSTR.h"
#include "Source/D2Client/D2Hell/Src/Archive.h"

// SMem's few Storm neighbours, reduced to what it asks of them.
uint32_t gnSSystemOption3 = 0;

namespace Storm::Source::SErr {
static DWORD g_dwLastError = 0;
void __stdcall SErrSetLastError(DWORD dwErrCode) { g_dwLastError = dwErrCode; }
int __stdcall SErrDisplayError(uint32_t nErrorCode, char * pStackTrace, int32_t nTraceFlags, char * pCustomMsg, int32_t nMessageFlags, UINT nExitCode) {
    fprintf(stderr, "SErrDisplayError 0x%08x %s:%d\n", nErrorCode, pStackTrace ? pStackTrace : "", nTraceFlags);
    if (!nMessageFlags) {
        ExitProcess(2);
    }
    return 0;
}
void __stdcall SErrReportResourceLeak(char * szMessage) { fprintf(stderr, "leak %s\n", szMessage); }
}  // namespace Storm::Source::SErr

namespace Storm::Source::SReg {
BOOL __stdcall SRegLoadValue(char * pTree, char * pKey, uint8_t byFlags, int32_t * pnOut) { return 0; }
}  // namespace Storm::Source::SReg

namespace Storm::Source::SSTR {
uint32_t __stdcall SStrLen(char * szText) { return (uint32_t)strlen(szText); }
uint32_t __stdcall SStrCopy(char * szBufferOutput, char * szBufferInput, int32_t nBufferOutputLength) {
    snprintf(szBufferOutput, (size_t)nBufferOutputLength, "%s", szBufferInput);
    return (uint32_t)strlen(szBufferOutput);
}
uint32_t __stdcall SStrHash(char * pszText, uint8_t bCaseSensitive, uint32_t nSeed) {
    uint32_t nHash = 2166136261u ^ nSeed;
    for (; *pszText; pszText++) {
        nHash = (nHash ^ (uint8_t)*pszText) * 16777619u;
    }
    return nHash;
}
}  // namespace Storm::Source::SSTR

using namespace Storm::Source::SMem;

namespace {

int g_nChecks = 0;
int g_nFailures = 0;

void Check(bool bOk, const char* szWhat, int nLine) {
    ++g_nChecks;
    if (!bOk && ++g_nFailures <= 20) {
        fprintf(stderr, "FAIL line %d: %s\n", nLine, szWhat);
    }
}
#define CHECK(x) Check((x), #x, __LINE__)

uint32_t g_nRandom = 0x2545f491;
uint32_t Random(uint32_t nLimit) {
    g_nRandom ^= g_nRandom << 13;
    g_nRandom ^= g_nRandom >> 17;
    g_nRandom ^= g_nRandom << 5;
    return g_nRandom % nLimit;
}

// A request size: mostly small, some across the bins, a few large (VirtualAlloc'd) blocks.
uint32_t RandomSize() {
    uint32_t nPick = Random(100);
    if (nPick < 60) return Random(64);
    if (nPick < 90) return 64 + Random(1024);
    if (nPick < 98) return 1024 + Random(0xf000);
    return 0xfe88 + Random(0x40000);
}

struct Live {
    uint8_t* pData;
    uint32_t nSize;
    uint8_t byFill;
    HSHEAP hHeap;  // 0 for a block from SMemAlloc
};

// A fixed list in place of std::vector, whose headers clash with the Win32 surface.
template <typename T, uint32_t kCapacity>
struct List {
    T aItems[kCapacity];
    uint32_t nCount = 0;
    bool empty() const { return !nCount; }
    uint32_t size() const { return nCount; }
    T& operator[](uint32_t n) { return aItems[n]; }
    T& back() { return aItems[nCount - 1]; }
    void pop_back() { nCount--; }
    void push_back(const T& t) {
        if (nCount < kCapacity) aItems[nCount++] = t;
    }
    T* begin() { return aItems; }
    T* end() { return aItems + nCount; }
};
typedef List<Live, 0x20000> LiveList;
LiveList g_aLive;

bool Intact(const Live& t) {
    for (uint32_t i = 0; i < t.nSize; i++) {
        if (t.pData[i] != t.byFill) {
            return false;
        }
    }
    return true;
}

uintptr_t g_nHighestHeap = 0;
uint32_t g_nMisaligned = 0;

void Fill(Live& t) {
    t.byFill = (uint8_t)(1 + Random(255));
    memset(t.pData, t.byFill, t.nSize);
    if ((uintptr_t)t.pData & 7) {
        g_nMisaligned++;
    }
}

void VerifyAll(LiveList& aLive) {
    for (const Live& t : aLive) {
        CHECK(Intact(t));
        CHECK(SMemGetSize(t.pData, nullptr, 0) == (int32_t)t.nSize);
        if (t.hHeap) {
            CHECK(SMemGetHeapByPtr(t.pData) == t.hHeap);
            CHECK(SMemHeapSize(t.hHeap, 0, t.pData) == (int32_t)t.nSize);
        }
    }
}

// Many created heaps, allocated, freed and reallocated at random with a shadow of every block.
void TestHeaps() {
    const int kHeaps = 600;  // more than 256: every bucket holds several
    static List<HSHEAP, 1024> aHeaps;
    aHeaps.nCount = 0;
    for (int i = 0; i < kHeaps; i++) {
        HSHEAP h = SMemHeapCreate(0, 0x1000, 0);
        CHECK(h >= 0x80000000);
        aHeaps.push_back(h);
    }

    LiveList& aLive = g_aLive;
    aLive.nCount = 0;
    for (int nRound = 0; nRound < 40; nRound++) {
        for (int nOp = 0; nOp < 4000; nOp++) {
            uint32_t nKind = Random(10);
            if (nKind < 5 || aLive.empty()) {
                Live t;
                t.hHeap = aHeaps[Random(kHeaps)];
                t.nSize = RandomSize();
                t.pData = (uint8_t*)SMemHeapAlloc(t.hHeap, 0, t.nSize);
                CHECK(t.pData != nullptr);
                if (!t.pData) continue;
                Fill(t);
                aLive.push_back(t);
            } else if (nKind < 8) {
                uint32_t n = Random((uint32_t)aLive.size());
                Live t = aLive[n];
                CHECK(Intact(t));
                CHECK(SMemHeapFree(t.hHeap, 0, t.pData));
                aLive[n] = aLive.back();
                aLive.pop_back();
            } else {
                uint32_t n = Random((uint32_t)aLive.size());
                Live& t = aLive[n];
                uint32_t nNew = Random(4) ? t.nSize + Random(256) : RandomSize();  // grow in place, or anything
                uint8_t* pNew = (uint8_t*)SMemHeapReAlloc(t.hHeap, 0, (uint32_t*)t.pData, nNew);
                CHECK(pNew != nullptr);
                if (!pNew) continue;
                uint32_t nKept = nNew < t.nSize ? nNew : t.nSize;
                for (uint32_t i = 0; i < nKept; i++) {
                    if (pNew[i] != t.byFill) {
                        CHECK(!"realloc kept the old bytes");
                        break;
                    }
                }
                t.pData = pNew;
                t.nSize = nNew;
                Fill(t);
            }
        }
        VerifyAll(aLive);
    }

    // The walk finds every live block of a heap.
    SMemBlockDetailsStrc tInfo;
    for (int i = 0; i < 8; i++) {
        HSHEAP h = aHeaps[i];
        size_t nExpected = 0;
        for (const Live& t : aLive) nExpected += t.hHeap == h;
        size_t nFound = 0;
        uint8_t* pBlock = nullptr;
        uint8_t* pPrev = nullptr;
        for (;;) {
            memset(&tInfo, 0, sizeof(tInfo));
            tInfo.dwSize = sizeof(tInfo);
            if (!SMemFindNextBlock(h, pPrev, &pBlock, (int*)&tInfo)) break;
            if (tInfo.bAllocated) {
                nFound++;
                CHECK(tInfo.bValid);
            }
            pPrev = pBlock;
        }
        CHECK(nFound == nExpected);
    }

    for (const Live& t : aLive) {
        CHECK(SMemHeapFree(t.hHeap, 0, t.pData));
    }
    for (HSHEAP h : aHeaps) {
        CHECK(SMemHeapDestroy(h));
    }
}

// The default heaps (one per caller), grown past their first reservation so a heap chains a
// second, larger region, then emptied so SMem releases them and their table slots.
void TestCallerHeaps() {
    static char szFile[] = "SMemTest.cpp";
    LiveList& aLive = g_aLive;
    aLive.nCount = 0;
    for (int nRound = 0; nRound < 6; nRound++) {
        for (int i = 0; i < 20000; i++) {
            Live t;
            t.hHeap = 0;
            t.nSize = Random(3) ? Random(200) : RandomSize();
            t.pData = (uint8_t*)SMemAlloc(t.nSize, szFile, (uint32_t)(i % 97), 0);
            CHECK(t.pData != nullptr);
            if (!t.pData) continue;
            Fill(t);
            aLive.push_back(t);
            uintptr_t nAddr = (uintptr_t)t.pData;
            if (nAddr > g_nHighestHeap) g_nHighestHeap = nAddr;
        }
        for (int i = 0; i < 4000; i++) {
            Live& t = aLive[Random((uint32_t)aLive.size())];
            uint32_t nNew = RandomSize();
            uint8_t* pNew = (uint8_t*)SMemReAlloc(t.pData, nNew, szFile, 1, 0);
            CHECK(pNew != nullptr);
            if (!pNew) continue;
            t.pData = pNew;
            t.nSize = nNew;
            Fill(t);
        }
        VerifyAll(aLive);
        // free half, at random
        for (size_t n = aLive.size() / 2; n; n--) {
            uint32_t k = Random((uint32_t)aLive.size());
            CHECK(Intact(aLive[k]));
            CHECK(SMemFree(aLive[k].pData, szFile, 2, 0));
            aLive[k] = aLive.back();
            aLive.pop_back();
        }
    }
    for (const Live& t : aLive) {
        CHECK(SMemFree(t.pData, szFile, 3, 0));
    }
    SMEM_FreeEmptyHeaps();
}

// More heaps over time than the table has slots: a destroyed heap's slot is reused.
void TestSlotReuse() {
    for (int i = 0; i < 70000; i++) {
        HSHEAP h = SMemHeapCreate(0, 0x1000, 0);
        uint32_t* p = SMemHeapAlloc(h, 0, 24);
        CHECK(p && SMemGetHeapByPtr((uint8_t*)p) == h);
        CHECK(SMemHeapFree(h, 0, p));
        CHECK(SMemHeapDestroy(h));
    }
}

// Heaps created, used and destroyed from several threads at once, so the slot claims and
// releases of different buckets race. SMemHeapCreate itself is serialised: it returns its
// static id counter after leaving the bucket lock, so a concurrent create can hand both
// callers the same id (as in 1.14d).
CRITICAL_SECTION g_tCreateLock;

uint32_t ThreadBlockSize(int k) {
    return (uint32_t)(16 + k * 40);
}

DWORD __stdcall HeapThread(LPVOID pParam) {
    int* pnFailures = (int*)pParam;
    for (int i = 0; i < 3000; i++) {
        EnterCriticalSection(&g_tCreateLock);
        HSHEAP h = SMemHeapCreate(0, 0x1000, 0);
        LeaveCriticalSection(&g_tCreateLock);
        uint8_t* aBlocks[16];
        for (int k = 0; k < 16; k++) {
            uint32_t nSize = ThreadBlockSize(k);
            aBlocks[k] = (uint8_t*)SMemHeapAlloc(h, 0, nSize);
            memset(aBlocks[k], k + 1, nSize);
        }
        for (int k = 0; k < 16; k++) {
            if (aBlocks[k][ThreadBlockSize(k) - 1] != k + 1 || SMemGetHeapByPtr(aBlocks[k]) != h) (*pnFailures)++;
            SMemHeapFree(h, 0, aBlocks[k]);
        }
        SMemHeapDestroy(h);
    }
    return 0;
}

void TestThreads() {
    HANDLE aThreads[8];
    InitializeCriticalSection(&g_tCreateLock);
    int aFailures[8] = {};
    for (int n = 0; n < 8; n++) {
        aThreads[n] = CreateThread(nullptr, 0, HeapThread, &aFailures[n], 0, nullptr);
        CHECK(aThreads[n] != nullptr);
    }
    for (int n = 0; n < 8; n++) {
        CHECK(WaitForSingleObject(aThreads[n], 0xffffffff) == 0);
        CloseHandle(aThreads[n]);
        CHECK(aFailures[n] == 0);
    }
}

}  // namespace

int main() {
    SMemInitialize();
    TestHeaps();
    TestCallerHeaps();
    TestSlotReuse();
    TestThreads();
    // The point of the test on arm64: the heaps live above 4 GB.
    if (sizeof(void*) == 8) {
        CHECK(g_nHighestHeap > 0xffffffffu);
    }
    printf("highest block %p, %u of the blocks not 8-byte aligned\n", (void*)g_nHighestHeap, g_nMisaligned);
    CHECK(g_nMisaligned == 0);
    SMemDestroy();

    // Again with the debug layouts: a guard word after every block, and fill on alloc and free.
    SMemInitialize();
    SMemSetDebugFlags(1 | 8, 1 | 8);
    TestHeaps();
    TestCallerHeaps();
    SMemDestroy();
    printf("%d checks, %d failures\n", g_nChecks, g_nFailures);
    return g_nFailures ? 1 : 0;
}

#endif
