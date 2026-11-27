// Storm's region module for the Mac build (SRGN.CPP; 1.14d Intel Mac band 0001b087..0001c730).
// The file's strings carry no __FILE__ literal; the name is the module's usual one. The band is
// the region handle table: an export table (a hash table keyed by handle with a mutex, TSExportTableSync
// <RGN,HSRGN,HLOCKEDRGN,CCritSect,1>), its pool of 0x60-byte region records (TSHashTableReuse, handing
// out nodes from blocks that double in size) and the fixed-array and list instantiations they use.
// SRgnDelete (0001b087) is the only exported call among them; SRgnDestroy (0001b0e3) tears the table down
// through its Destroy slot, and the static initialiser (0001c6f0) builds the global table.
//
// The instantiations are written as plain functions over the table's layout: each has its own
// address, and the original did not inline them.
#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#include "Source/StormMac/H/stpl.h"

#include <pthread.h>
#include <stdint.h>

#define RGN_NOINLINE __attribute__((noinline))

// 1.14d mac: 00010b98
void SErrSetLastError(uint32_t dwErrCode);
// 1.14d mac: 000176a0
void CRITSECT_Init(pthread_mutex_t* pMutex);

// ---- links and lists ----

struct RgnLink;
void RgnLink_Unlink(RgnLink* pLink);

// A link of the record's two lists; the end of a list is its terminator link's address | 1.
struct RgnLink {
    RgnLink* m_prev;
    uint32_t m_next;

    RgnLink() : m_prev(NULL), m_next(0) {}
    ~RgnLink() { RgnLink_Unlink(this); }
};

struct RgnList;
void RgnList_UnlinkAll(RgnList* pList);

struct RgnList {
    int m_linkoffset;
    RgnLink m_term;

    ~RgnList() { RgnList_UnlinkAll(this); }
};

// The same over the pool's blocks, which link at +0x10.
struct RgnBlockLink;
void RgnBlockLink_Unlink(RgnBlockLink* pLink);

struct RgnBlockLink {
    RgnBlockLink* m_prev;
    uint32_t m_next;

    RgnBlockLink() : m_prev(NULL), m_next(0) {}
    ~RgnBlockLink() { RgnBlockLink_Unlink(this); }
};

struct RgnBlockList;
void RgnBlockList_UnlinkAll(RgnBlockList* pList);

struct RgnBlockList {
    int m_linkoffset;
    RgnBlockLink m_term;

    ~RgnBlockList() { RgnBlockList_UnlinkAll(this); }
};

// ---- the region record ----

// The three arrays a record owns: alloc, count, data, chunk. They free their storage.
template<int N>
struct RgnArr {
    static const uint32_t kElemSize = N;  // N is the element size in bytes
    uint32_t m_alloc;
    uint32_t m_count;
    void* m_data;
    uint32_t m_chunk;

    RgnArr() : m_alloc(0), m_count(0), m_data(NULL), m_chunk(0) {}
    ~RgnArr() {
        if (m_data) {
            SMemFree(m_data, "nothing", -2, 0);
        }
    }
};

struct RGN {
    uint32_t m_handle;     // 0x00: the table's key
    RgnLink m_linktoslot;  // 0x04
    RgnLink m_linktofull;  // 0x0C
    uint32_t m_key;        // 0x14: HASHKEY_NONE
    RgnArr<0x1C> m_a;      // 0x18: elements of 0x1C bytes
    RgnArr<0x10> m_b;      // 0x28: elements of 0x10 bytes
    RgnArr<8> m_c;         // 0x38: elements of 8 bytes
    uint32_t m_f48[2];     // 0x48
    uint32_t m_f50[2];     // 0x50
    uint32_t m_f58;
    uint32_t m_f5C;
};

// The pool's block: a growable array of records on the block list.
struct RgnBlock {
    uint32_t m_alloc;
    uint32_t m_count;
    RGN* m_data;
    uint32_t m_chunk;
    RgnBlockLink m_link;  // 0x10

    RgnBlock() : m_alloc(0), m_count(0), m_data(NULL), m_chunk(0) {}
};

struct RgnElem16 {
    uint64_t a, b;
};
struct RgnElem1C {
    uint64_t a, b, c;
    uint32_t d;
};

// ---- the table ----

void RgnBucketArr_Destroy(void* pArray);

// The array of bucket lists (12 bytes each).
struct RgnBucketArr {
    uint32_t m_alloc;
    uint32_t m_count;
    RgnList* m_data;
    uint32_t m_chunk;

    RgnBucketArr() : m_alloc(0), m_count(0), m_data(NULL), m_chunk(0) {}
    ~RgnBucketArr() { RgnBucketArr_Destroy(this); }
};

// TSHashTable<RGN,HASHKEY_NONE>
class RgnHashTable {
public:
    RgnList m_full;           // 0x04
    uint32_t m_fullness;      // 0x10
    RgnBucketArr m_buckets;   // 0x14
    uint32_t m_slotmask;      // 0x24

    RgnHashTable() : m_fullness(0), m_slotmask((uint32_t)-1) {
        m_full.m_linkoffset = (int)0xDDDDDDDD;
        m_full.m_term.m_prev = &m_full.m_term;
        m_full.m_term.m_next = (uint32_t)&m_full.m_term | 1;
        m_full.m_linkoffset = 0xC;
    }
    virtual void InternalDelete(RGN* pNode);
    virtual RGN* InternalNew(RgnList* pList, uint32_t nExtraBytes, uint32_t dwFlags);
    virtual ~RgnHashTable();
    virtual void Destroy();
};

// TSHashTableReuse<RGN,HASHKEY_NONE,1>
class RgnHashTableReuse : public RgnHashTable {
public:
    RgnList m_free;           // 0x28: nodes handed back
    uint32_t m_blocksize;     // 0x34: records the next block holds
    RgnBlockList m_blocks;    // 0x38

    RgnHashTableReuse();
    virtual void InternalDelete(RGN* pNode);
    virtual RGN* InternalNew(RgnList* pList, uint32_t nExtraBytes, uint32_t dwFlags);
    virtual ~RgnHashTableReuse();
    virtual void Destroy();
};

// TSExportTableSync<RGN,HSRGN,HLOCKEDRGN,CCritSect,1>
class RgnExportTable : public RgnHashTableReuse {
public:
    uint32_t m_f44;
    uint32_t m_nextHandle;    // 0x48
    uint32_t m_f4C;
    pthread_mutex_t m_mutex;  // 0x50

    RgnExportTable();
    virtual ~RgnExportTable();
};

// ---- link functions ----

// 1.14d mac: 0001b238
RGN_NOINLINE RgnLink* RgnLink_NextLink(RgnLink* pLink, int nLinkOffset) {
    uint32_t pNext = pLink->m_next;
    if (pNext == 0 || (pNext & 1)) {
        return (RgnLink*)(pNext & 0xFFFFFFFE);
    }
    if (nLinkOffset < 0) {
        nLinkOffset = (int)pLink - (int)pLink->m_prev->m_next;
    }
    return (RgnLink*)(pNext + nLinkOffset);
}

// 1.14d mac: 0001b1f8
RGN_NOINLINE void RgnLink_Unlink(RgnLink* pLink) {
    RgnLink* pPrev = pLink->m_prev;
    if (pPrev != NULL) {
        RgnLink_NextLink(pLink, -1)->m_prev = pPrev;
        pLink->m_prev->m_next = pLink->m_next;
        pLink->m_prev = NULL;
        pLink->m_next = 0;
    }
}

// 1.14d mac: 0001b88e
RGN_NOINLINE RgnBlockLink* RgnBlockLink_NextLink(RgnBlockLink* pLink, int nLinkOffset) {
    uint32_t pNext = pLink->m_next;
    if (pNext == 0 || (pNext & 1)) {
        return (RgnBlockLink*)(pNext & 0xFFFFFFFE);
    }
    if (nLinkOffset < 0) {
        nLinkOffset = (int)pLink - (int)pLink->m_prev->m_next;
    }
    return (RgnBlockLink*)(pNext + nLinkOffset);
}

// 1.14d mac: 0001b84e
RGN_NOINLINE void RgnBlockLink_Unlink(RgnBlockLink* pLink) {
    RgnBlockLink* pPrev = pLink->m_prev;
    if (pPrev != NULL) {
        RgnBlockLink_NextLink(pLink, -1)->m_prev = pPrev;
        pLink->m_prev->m_next = pLink->m_next;
        pLink->m_prev = NULL;
        pLink->m_next = 0;
    }
}

// 1.14d mac: 0001b26e
RGN_NOINLINE void RgnList_LinkNode(RgnList* pList, RGN* pNode, int nLinkType, RGN* pExisting) {
    RgnLink* pLink = pNode ? (RgnLink*)((char*)pNode + pList->m_linkoffset) : &pList->m_term;
    if (pLink->m_prev != NULL) {
        RgnLink_Unlink(pLink);
    }
    RgnLink* pExistingLink = pExisting ? (RgnLink*)((char*)pExisting + pList->m_linkoffset) : &pList->m_term;
    if (nLinkType == 2) {
        RgnLink* pPrev = pExistingLink->m_prev;
        pLink->m_prev = pPrev;
        pLink->m_next = pPrev->m_next;
        pPrev->m_next = (uint32_t)pNode;
        pExistingLink->m_prev = pLink;
    } else if (nLinkType == 1) {
        pLink->m_prev = pExistingLink;
        pLink->m_next = pExistingLink->m_next;
        RgnLink_NextLink(pExistingLink, pList->m_linkoffset)->m_prev = pLink;
        pExistingLink->m_next = (uint32_t)pNode;
    }
}

// 1.14d mac: 0001c066
RGN_NOINLINE void RgnBlockList_LinkNode(RgnBlockList* pList, RgnBlock* pNode, int nLinkType, RgnBlock* pExisting) {
    RgnBlockLink* pLink = pNode ? (RgnBlockLink*)((char*)pNode + pList->m_linkoffset) : &pList->m_term;
    if (pLink->m_prev != NULL) {
        RgnBlockLink_Unlink(pLink);
    }
    RgnBlockLink* pExistingLink = pExisting ? (RgnBlockLink*)((char*)pExisting + pList->m_linkoffset) : &pList->m_term;
    if (nLinkType == 2) {
        RgnBlockLink* pPrev = pExistingLink->m_prev;
        pLink->m_prev = pPrev;
        pLink->m_next = pPrev->m_next;
        pPrev->m_next = (uint32_t)pNode;
        pExistingLink->m_prev = pLink;
    } else if (nLinkType == 1) {
        pLink->m_prev = pExistingLink;
        pLink->m_next = pExistingLink->m_next;
        RgnBlockLink_NextLink(pExistingLink, pList->m_linkoffset)->m_prev = pLink;
        pExistingLink->m_next = (uint32_t)pNode;
    }
}

// 1.14d mac: 0001b2f0
RGN_NOINLINE void RgnList_UnlinkAll(RgnList* pList) {
    uint32_t pNode;
    for (;;) {
        pNode = pList->m_term.m_next;
        if (pNode == 0 || (pNode & 1)) {
            pNode = 0;
        }
        if (pNode == 0) {
            break;
        }
        RgnLink_Unlink((RgnLink*)(pList->m_linkoffset + pNode));
    }
}

// 1.14d mac: 0001c5c6
RGN_NOINLINE void RgnBlockList_UnlinkAll(RgnBlockList* pList) {
    uint32_t pNode;
    for (;;) {
        pNode = pList->m_term.m_next;
        if (pNode == 0 || (pNode & 1)) {
            pNode = 0;
        }
        if (pNode == 0) {
            break;
        }
        RgnBlockLink_Unlink((RgnBlockLink*)(pList->m_linkoffset + pNode));
    }
}

// ---- fixed arrays of plain elements ----

// 1.14d mac: 0001bea8
RGN_NOINLINE void RgnArr8_ReallocData(RgnArr<8>* pArray, int nNewSize) {
    pArray->m_alloc = nNewSize;
    if (pArray->m_data != NULL || nNewSize != 0) {
        pArray->m_data = SMemReAlloc(pArray->m_data, nNewSize * pArray->kElemSize, "nothing", -2, 0);
    }
}

// 1.14d mac: 0001be5a
RGN_NOINLINE void RgnArr8_Set(RgnArr<8>* pArray, int nCount, void* pSource) {
    RgnArr8_ReallocData(pArray, nCount);
    uint64_t* pDest = (uint64_t*)pArray->m_data;
    uint64_t* pSrc = (uint64_t*)pSource;
    for (int i = 0; i != nCount; i++) {
        if (pDest + i) {
            pDest[i] = pSrc[i];
        }
    }
    pArray->m_count = nCount;
}

// 1.14d mac: 0001bf54
RGN_NOINLINE void RgnArr16_ReallocData(RgnArr<0x10>* pArray, int nNewSize) {
    pArray->m_alloc = nNewSize;
    if (pArray->m_data != NULL || nNewSize != 0) {
        pArray->m_data = SMemReAlloc(pArray->m_data, nNewSize * pArray->kElemSize, "nothing", -2, 0);
    }
}

// 1.14d mac: 0001befa
RGN_NOINLINE void RgnArr16_Set(RgnArr<0x10>* pArray, int nCount, void* pSource) {
    RgnArr16_ReallocData(pArray, nCount);
    RgnElem16* pDest = (RgnElem16*)pArray->m_data;
    RgnElem16* pSrc = (RgnElem16*)pSource;
    for (int i = 0; i != nCount; i++) {
        if (pDest + i) {
            pDest[i] = pSrc[i];
        }
    }
    pArray->m_count = nCount;
}

// 1.14d mac: 0001c014
RGN_NOINLINE void RgnArr1C_ReallocData(RgnArr<0x1C>* pArray, int nNewSize) {
    pArray->m_alloc = nNewSize;
    if (pArray->m_data != NULL || nNewSize != 0) {
        pArray->m_data = SMemReAlloc(pArray->m_data, nNewSize * pArray->kElemSize, "nothing", -2, 0);
    }
}

// 1.14d mac: 0001bfa6
RGN_NOINLINE void RgnArr1C_Set(RgnArr<0x1C>* pArray, int nCount, void* pSource) {
    RgnArr1C_ReallocData(pArray, nCount);
    RgnElem1C* pDest = (RgnElem1C*)pArray->m_data;
    RgnElem1C* pSrc = (RgnElem1C*)pSource;
    for (int i = 0; i != nCount; i++) {
        if (pDest + i) {
            pDest[i] = pSrc[i];
        }
    }
    pArray->m_count = nCount;
}

// ---- the region record's copy and the array of records ----

// 1.14d mac: 0001bce0
// Copy-constructs a record: links cleared, the three arrays copied, the rest taken as is.
RGN_NOINLINE void SRGN_CopyRegionRecord(RGN* pDest, RGN* pSource) {
    pDest->m_linktoslot.m_next = 0;
    pDest->m_linktoslot.m_prev = NULL;
    pDest->m_linktofull.m_next = 0;
    pDest->m_linktofull.m_prev = NULL;
    pDest->m_a.m_alloc = 0;
    pDest->m_a.m_count = 0;
    pDest->m_a.m_data = NULL;
    RgnArr1C_Set(&pDest->m_a, pSource->m_a.m_count, pSource->m_a.m_data);
    pDest->m_a.m_chunk = pSource->m_a.m_chunk;
    pDest->m_b.m_alloc = 0;
    pDest->m_b.m_count = 0;
    pDest->m_b.m_data = NULL;
    RgnArr16_Set(&pDest->m_b, pSource->m_b.m_count, pSource->m_b.m_data);
    pDest->m_b.m_chunk = pSource->m_b.m_chunk;
    pDest->m_c.m_alloc = 0;
    pDest->m_c.m_count = 0;
    pDest->m_c.m_data = NULL;
    RgnArr8_Set(&pDest->m_c, pSource->m_c.m_count, pSource->m_c.m_data);
    pDest->m_c.m_chunk = pSource->m_c.m_chunk;
    uint64_t qwFirst = *(uint64_t*)pSource->m_f48;
    *(uint64_t*)pDest->m_f50 = *(uint64_t*)pSource->m_f50;
    *(uint64_t*)pDest->m_f48 = qwFirst;
    pDest->m_f58 = pSource->m_f58;
    pDest->m_f5C = pSource->m_f5C;
}

// 1.14d mac: 0001ba84
RGN_NOINLINE void RgnArrRGN_ReallocData(RgnBlock* pArray, uint32_t nNewSize) {
    RGN* pOld = pArray->m_data;
    if (nNewSize < pArray->m_count) {
        for (uint32_t i = nNewSize; i < pArray->m_count; i++) {
            pOld[i].~RGN();
        }
    }
    pArray->m_alloc = nNewSize;
    pArray->m_data = (RGN*)SMemReAlloc(pOld, nNewSize * sizeof(RGN), "nothing", -2, SMEM_FLAG_NOMOVE);
    if (pArray->m_data == NULL) {
        pArray->m_data = (RGN*)SMemAlloc(nNewSize * sizeof(RGN), "nothing", -2, 0);
        if (pOld != NULL) {
            uint32_t nCopy = pArray->m_count < nNewSize ? pArray->m_count : nNewSize;
            for (uint32_t i = 0; i < nCopy; i++) {
                if (&pArray->m_data[i] != NULL) {
                    SRGN_CopyRegionRecord(&pArray->m_data[i], &pOld[i]);
                }
                pOld[i].~RGN();
            }
            SMemFree(pOld, "nothing", -2, 0);
        }
    }
}

// 1.14d mac: 0001ba12
RGN_NOINLINE void RgnArrRGN_Reserve(RgnBlock* pArray, int nCount, int bRound) {
    uint32_t nNewSize = pArray->m_count + nCount;
    if (pArray->m_alloc < nNewSize) {
        if (bRound) {
            uint32_t nChunk = pArray->m_chunk;
            if (nChunk == 0) {
                nChunk = nNewSize;
                if (nNewSize < 8) {
                    uint32_t nTop;
                    do {
                        nTop = nChunk;
                        nChunk = nTop & (nTop - 1);
                    } while (nChunk);
                    nChunk = nTop > 1 ? nTop : 1;
                } else {
                    pArray->m_chunk = 8;
                    nChunk = 8;
                }
            }
            if (nNewSize % nChunk) {
                nNewSize = nChunk + nNewSize - nNewSize % nChunk;
            }
        }
        RgnArrRGN_ReallocData(pArray, nNewSize);
    }
}

// 1.14d mac: 0001c0e8
RGN_NOINLINE RGN* RgnArrRGN_New(RgnBlock* pArray) {
    RgnArrRGN_Reserve(pArray, 1, 1);
    RGN* pNode = &pArray->m_data[pArray->m_count];
    if (pNode != NULL) {
        new (pNode) RGN;
    }
    pArray->m_count++;
    return pNode;
}

// 1.14d mac: 0001b732
// Destroys an array of records and frees its storage.
RGN_NOINLINE void RgnArrRGN_Destroy(RgnBlock* pArray) {
    RGN* pData = pArray->m_data;
    for (uint32_t i = 0; i < pArray->m_count; i++) {
        pData[i].~RGN();
        pData = pArray->m_data;
    }
    if (pData != NULL) {
        SMemFree(pData, "nothing", -2, 0);
    }
}

// 1.14d mac: 0001c46c
RGN_NOINLINE void RgnBucketArr_Destroy(void* pArrayRaw) {
    RgnBucketArr* pArray = (RgnBucketArr*)pArrayRaw;
    RgnList* pData = pArray->m_data;
    for (uint32_t i = 0; i < pArray->m_count; i++) {
        pData[i].~RgnList();
        pData = pArray->m_data;
    }
    if (pData != NULL) {
        SMemFree(pData, "nothing", -2, 0);
    }
}

// ---- list nodes ----

// 1.14d mac: 0001c4ec
RGN_NOINLINE RGN* RgnList_NewNode(RgnList* pList, int nLinkType, int nExtraBytes, uint32_t dwFlags) {
    void* pMem = SMemAlloc(nExtraBytes + sizeof(RGN), "", -2, dwFlags | SMEM_FLAG_ZEROMEMORY);
    RGN* pNode = NULL;
    if (pMem != NULL) {
        pNode = new (pMem) RGN;
    }
    if (nLinkType != 0) {
        RgnList_LinkNode(pList, pNode, nLinkType, NULL);
    }
    return pNode;
}

// 1.14d mac: 0001b980
RGN_NOINLINE RgnBlock* RgnBlockList_NewNode(RgnBlockList* pList, int nLinkType, int nExtraBytes, uint32_t dwFlags) {
    void* pMem = SMemAlloc(nExtraBytes + sizeof(RgnBlock), "", -2, dwFlags | SMEM_FLAG_ZEROMEMORY);
    RgnBlock* pNode = NULL;
    if (pMem != NULL) {
        pNode = new (pMem) RgnBlock;
    }
    if (nLinkType != 0) {
        RgnBlockList_LinkNode(pList, pNode, nLinkType, NULL);
    }
    return pNode;
}

// 1.14d mac: 0001b61a
// Deletes every record on the list.
RGN_NOINLINE void RgnFreeList_Clear(RgnList* pList) {
    RGN* pObject;
    for (;;) {
        pObject = (RGN*)pList->m_term.m_next;
        if (pObject == NULL || ((uint32_t)pObject & 1)) {
            pObject = NULL;
        }
        if (pObject == NULL) {
            break;
        }
        pObject->~RGN();
        SMemFree(pObject, "", -2, 0);
    }
}

// 1.14d mac: 0001b58e
// Deletes every block on the list, with the records it holds.
RGN_NOINLINE void RgnBlockList_Clear(RgnBlockList* pList) {
    RgnBlock* pObject;
    for (;;) {
        pObject = (RgnBlock*)pList->m_term.m_next;
        if (pObject == NULL || ((uint32_t)pObject & 1)) {
            pObject = NULL;
        }
        if (pObject == NULL) {
            break;
        }
        pObject->m_link.~RgnBlockLink();
        RgnArrRGN_Destroy(pObject);
        SMemFree(pObject, "", -2, 0);
    }
}

// ---- the hash table ----

// 1.14d mac: 0001b8c4
RGN_NOINLINE void RgnHashTable_Clear(RgnHashTable* pTable, int bUnlinkOnly) {
    pTable->m_fullness = 0;
    RgnList_UnlinkAll(&pTable->m_full);
    uint32_t nBuckets = pTable->m_buckets.m_count;
    if (nBuckets != 0) {
        RgnList* pBuckets = pTable->m_buckets.m_data;
        for (uint32_t nBucket = 0; nBucket < nBuckets; nBucket++) {
            RgnList* pBucket = &pBuckets[nBucket];
            uint32_t pNode = pBucket->m_term.m_next;
            if (pNode == 0 || (pNode & 1)) {
                pNode = 0;
            }
            if (pNode != 0) {
                do {
                    if (bUnlinkOnly == 0) {
                        pTable->InternalDelete((RGN*)pNode);
                    } else {
                        RgnLink_Unlink((RgnLink*)(pNode + pBucket->m_linkoffset));
                    }
                    pBuckets = pTable->m_buckets.m_data;
                    pBucket = &pBuckets[nBucket];
                    pNode = pBucket->m_term.m_next;
                    if (pNode == 0 || (pNode & 1)) {
                        pNode = 0;
                    }
                } while (pNode != 0);
                nBuckets = pTable->m_buckets.m_count;
            }
        }
    }
}

// 1.14d mac: 0001b1ce
RGN_NOINLINE void RgnHashTable_Unlink(RgnHashTable* pTable, RGN* pEntry) {
    if (pEntry->m_linktoslot.m_next != 0) {
        RgnLink_Unlink(&pEntry->m_linktoslot);
        RgnLink_Unlink(&pEntry->m_linktofull);
    }
}

// 1.14d mac: 0001b33c
RGN_NOINLINE RGN* RgnHashTable_Ptr(RgnHashTable* pTable, uint32_t dwKey) {
    if (pTable->m_slotmask != 0xFFFFFFFF) {
        RgnList* pBuckets = pTable->m_buckets.m_data;
        RgnList* pBucket = &pBuckets[pTable->m_slotmask & dwKey];
        RGN* pEntry = (RGN*)pBucket->m_term.m_next;
        if (pEntry == NULL || ((uint32_t)pEntry & 1)) {
            pEntry = NULL;
        }
        while (pEntry != NULL && !((uint32_t)pEntry & 1)) {
            if (pEntry->m_handle == dwKey) {
                return pEntry;
            }
            RgnLink* pLink;
            if (pEntry == NULL) {
                pLink = &pBucket->m_term;
            } else {
                pLink = (RgnLink*)((char*)pEntry + pBucket->m_linkoffset);
            }
            pEntry = (RGN*)pLink->m_next;
        }
    }
    return NULL;
}

// 1.14d mac: 0001c2f4
void RgnHashTable::InternalDelete(RGN* pNode) {
    pNode->~RGN();
    SMemFree(pNode, "", -2, 0);
}

// 1.14d mac: 0001c3ce
RGN* RgnHashTable::InternalNew(RgnList* pList, uint32_t nExtraBytes, uint32_t dwFlags) {
    return RgnList_NewNode(pList, 1, nExtraBytes, dwFlags);
}

// 1.14d mac: 0001c258 RgnHashTable::~RgnHashTable[D2]
RgnHashTable::~RgnHashTable() {
    RgnHashTable_Clear(this, 1);
}

// 1.14d mac: 0001c3fa RgnHashTable::~RgnHashTable[D1]
// 1.14d mac: 0001c404 RgnHashTable::~RgnHashTable[D0]

// 1.14d mac: 0001c41e
void RgnHashTable::Destroy() {
    RgnHashTable_Clear(this, 1);
    m_fullness = 0;
    m_slotmask = 0xFFFFFFFF;
    RgnBucketArr_Destroy(&m_buckets);
    m_buckets.m_alloc = 0;
    m_buckets.m_count = 0;
    m_buckets.m_data = NULL;
}

// ---- the table with a pool of records ----

// 1.14d mac: 0001c612 RgnHashTableReuse::RgnHashTableReuse[C1]
RgnHashTableReuse::RgnHashTableReuse() {
    m_free.m_linkoffset = (int)0xDDDDDDDD;
    m_free.m_term.m_prev = &m_free.m_term;
    m_free.m_term.m_next = (uint32_t)&m_free.m_term | 1;
    m_blocksize = 16;
    m_blocks.m_linkoffset = 16;
    m_blocks.m_term.m_prev = &m_blocks.m_term;
    m_blocks.m_term.m_next = (uint32_t)&m_blocks.m_term | 1;
    m_free.m_linkoffset = 4;
}

// 1.14d mac: 0001b408 RgnHashTableReuse::InternalDelete
void RgnHashTableReuse::InternalDelete(RGN* pNode) {
    RgnLink* pLink = pNode ? (RgnLink*)((char*)pNode + m_full.m_linkoffset) : &m_full.m_term;
    RgnLink_Unlink(pLink);
    RgnList_LinkNode(&m_free, pNode, 1, NULL);
}

// 1.14d mac: 0001b452 RgnHashTableReuse::InternalNew
RGN* RgnHashTableReuse::InternalNew(RgnList* pList, uint32_t nExtraBytes, uint32_t dwFlags) {
    RGN* pNode = (RGN*)m_free.m_term.m_next;
    if (pNode == NULL || ((uint32_t)pNode & 1)) {
        pNode = NULL;
    }
    if (pNode == NULL) {
        RgnBlock* pHead;
        for (;;) {
            pHead = (RgnBlock*)m_blocks.m_term.m_next;
            if (pHead == NULL || ((uint32_t)pHead & 1)) {
                pHead = NULL;
            }
            if (pHead != NULL && pHead->m_alloc != pHead->m_count) {
                break;
            }
            RgnBlock* pBlock = RgnBlockList_NewNode(&m_blocks, 1, 0, 0);
            uint32_t nNewSize = m_blocksize + pBlock->m_count;
            if (pBlock->m_alloc < nNewSize) {
                RgnArrRGN_ReallocData(pBlock, nNewSize);
            }
            m_blocksize *= 2;
        }
        pNode = RgnArrRGN_New(pHead);
    }
    RgnList_LinkNode(pList, pNode, 1, NULL);
    return pNode;
}

// 1.14d mac: 0001c1b6 RgnHashTableReuse::~RgnHashTableReuse[D2]
RgnHashTableReuse::~RgnHashTableReuse() {
    RgnBlockList_Clear(&m_blocks);
    RgnFreeList_Clear(&m_free);
    m_blocksize = 16;
}

// 1.14d mac: 0001c2d0 RgnHashTableReuse::~RgnHashTableReuse[D1]
// 1.14d mac: 0001c2da RgnHashTableReuse::~RgnHashTableReuse[D0]

// 1.14d mac: 0001b550
void RgnHashTableReuse::Destroy() {
    RgnHashTable_Clear(this, 0);
    RgnBlockList_Clear(&m_blocks);
    RgnFreeList_Clear(&m_free);
    m_blocksize = 16;
}

// ---- the export table ----

// 1.14d mac: 0001b3aa RgnExportTable::RgnExportTable[C1]
RgnExportTable::RgnExportTable() {
    m_nextHandle = ((uint32_t)this & 0xFFFFFFF) ^ 0xFFFFFFF;
    m_f4C = 0;
    CRITSECT_Init(&m_mutex);
}

// 1.14d mac: 0001b106 RgnExportTable::~RgnExportTable[D1]
RgnExportTable::~RgnExportTable() {
    pthread_mutex_destroy(&m_mutex);
}

// 1.14d mac: 0001b514 RgnExportTable::~RgnExportTable[D0]

// 1.14d mac: 0001b13a
RGN_NOINLINE RGN* RgnExportTable_Lock(RgnExportTable* pTable, uint32_t hHandle, uint32_t* pdwLockState, int bForWrite) {
    pthread_mutex_lock(&pTable->m_mutex);
    *pdwLockState = -(uint32_t)(bForWrite == 0) | 1;
    RGN* pEntry = RgnHashTable_Ptr(pTable, hHandle);
    if (pEntry == NULL) {
        pthread_mutex_unlock(&pTable->m_mutex);
        *pdwLockState = 0;
    }
    return pEntry;
}

// 1.14d mac: 0001b194
RGN_NOINLINE void RgnExportTable_DeleteUnlock(RgnExportTable* pTable, RGN* pEntry, int dwLockState) {
    RgnHashTable_Unlink(pTable, pEntry);
    pTable->InternalDelete(pEntry);
    if (dwLockState != 0) {
        pthread_mutex_unlock(&pTable->m_mutex);
    }
}

// 1.14d mac: 003f0db4
// The region handle table.
RgnExportTable g_tRgnTable;

// 1.14d mac: 0001b087
void SRgnDelete(int hRgn) {
    if (hRgn == 0) {
        SErrSetLastError(0x57);
    } else {
        uint32_t dwLockState;
        RGN* pRgn = RgnExportTable_Lock(&g_tRgnTable, hRgn, &dwLockState, 1);
        RgnExportTable_DeleteUnlock(&g_tRgnTable, pRgn, dwLockState);
    }
}

// 1.14d mac: 0001b0e3
void SRgnDestroy() {
    g_tRgnTable.Destroy();
}

// 1.14d mac: 0001c6f0 _GLOBAL__I_a

#endif  // D2_OS_MAC
