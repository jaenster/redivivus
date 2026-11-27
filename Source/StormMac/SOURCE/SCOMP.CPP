// StormMac Storm Mac/SOURCE/SCOMP.CPP: Storm's decompression front end and its codecs
// (adaptive Huffman, PKWARE blast, ADPCM, zlib), 1.14d Intel Mac 0000eb42..0000fe80.
//
// The binary's __FILE__ is ".../StormMac/Storm Mac/SOURCE/SCOMP.CPP" (lines 0x69d..0x7b0).
// The file runs from the Huffman tree routines, which follow SCODE's static initialiser
// (0000eab0), to its own static initialiser (0000fe80).
#include "../Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <string.h>

void* SMemAlloc(uint32_t nBytes, const char* szFile, int nLine, uint32_t dwFlags);
int SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags);
void SErrSetLastError(DWORD dwErrCode);
DWORD GetCurrentThreadId(void);
// Out of line in SFILE.CPP's neighbourhood: a recursive pthread mutex.
void CRITSECT_Init(pthread_mutex_t* pMutex);
void CRITSECT_Destroy(void* pMutex);

// PKWARE's blast (3rd party).
typedef unsigned (*blast_in)(void* pHow, unsigned char** ppBuffer);
typedef int (*blast_out)(void* pHow, unsigned char* pBuffer, unsigned nSize);
int blast(blast_in pfnInput, void* pInputHow, blast_out pfnOutput, void* pOutputHow);

// zlib's uncompress.
extern "C" int uncompress(uint8_t* pDest, unsigned long* pnDestLen, const uint8_t* pSource, unsigned long nSourceLen);

extern "C" int __cxa_atexit(void (*pfnDestructor)(void*), void* pObject, void* pDso);
extern "C" void* __dso_handle;

#define SCOMP_FILE "/Users/bclemetson/dev/diablo2/Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/SOURCE/SCOMP.CPP"

// 1.14d mac: 00320f48
// The Huffman weight tables, 0x102 bytes each.
extern const uint8_t gaCompressHuffmanWeightTables[];
// 1.14d mac: 00320d90
// The ADPCM step sizes.
extern const uint32_t gdwADPCMCoefficient[89];
// 1.14d mac: 00320ef4
// The ADPCM step index change per code.
extern const int32_t gnSSCompDeflateFlags[32];

// ---- the intrusive list the Huffman tree is built on ----
//
// The same TSList code appears in every Storm Mac file that has a list; each file carries
// its own copy of the instantiation. A link is {previous link, next element}; the end of a
// list is the terminator link's address | 1.

struct TSLink {
    TSLink* pPrev;
    uintptr_t nNext;  // the next element, or at the end the terminator | 1
};

struct TSList {
    int nLinkOffset;
    TSLink tTerminator;
};

static inline bool IsListEnd(uintptr_t nNext) {
    return nNext == 0 || (nNext & 1) != 0;
}

// 1.14d mac: 0000fdf6
// TSLink::NextLink
static TSLink* TSLINK_NextLink(TSLink* pLink, int nLinkOffset) {
    uintptr_t nNext = pLink->nNext;
    if (IsListEnd(nNext)) {
        return (TSLink*)(nNext & 0xfffffffe);
    }
    if (nLinkOffset < 0) {
        nLinkOffset = (int)((intptr_t)pLink - *(intptr_t*)((char*)pLink->pPrev + 4));
    }
    return (TSLink*)(nNext + nLinkOffset);
}

// 1.14d mac: 0000fdb6
// TSLink::Unlink
static void TSLINK_Unlink(TSLink* pLink) {
    TSLink* pPrev = pLink->pPrev;
    if (pPrev != NULL) {
        TSLink* pNextLink = TSLINK_NextLink(pLink, -1);
        pNextLink->pPrev = pPrev;
        pLink->pPrev->nNext = pLink->nNext;
        pLink->pPrev = NULL;
        pLink->nNext = 0;
    }
}

// 1.14d mac: 0000fd42
// TSList::LinkNode: after (1) or before (2) a node, or the list's end.
static void TSLIST_LinkNode(TSList* pList, TSLink* pNode, int nLinkType, TSLink* pExistingNode) {
    TSLink* pLink = pNode;
    if (pNode == NULL) {
        pLink = &pList->tTerminator;
    }
    if (pLink->pPrev != NULL) {
        TSLINK_Unlink(pLink);
    }
    TSLink* pExistingLink = &pList->tTerminator;
    if (pExistingNode != NULL) {
        pExistingLink = pExistingNode;
    }
    if (nLinkType == 2) {
        TSLink* pPrev = pExistingLink->pPrev;
        pLink->pPrev = pPrev;
        pLink->nNext = pPrev->nNext;
        pPrev->nNext = (uintptr_t)pNode;
        pExistingLink->pPrev = pLink;
    } else if (nLinkType == 1) {
        pLink->pPrev = pExistingLink;
        pLink->nNext = pExistingLink->nNext;
        TSLink* pNextLink = TSLINK_NextLink(pExistingLink, pList->nLinkOffset);
        pNextLink->pPrev = pLink;
        pExistingLink->nNext = (uintptr_t)pNode;
    }
}

// 1.14d mac: 0000fe2c
// TSList::UnlinkAll
static void TSLIST_UnlinkAll(TSList* pList) {
    TSLink* pLink = (TSLink*)pList->tTerminator.nNext;
    while (true) {
        if (IsListEnd((uintptr_t)pLink)) {
            pLink = NULL;
        }
        if (pLink == NULL) {
            break;
        }
        TSLINK_Unlink(pLink);
        pLink = (TSLink*)pList->tTerminator.nNext;
    }
}

// ---- the adaptive Huffman tree ----

struct HuffNode {
    TSLink tLink;         // 0x00  in the list of live nodes, by weight
    int nSymbol;          // 0x08
    uint32_t nWeight;     // 0x0C
    HuffNode* pParent;    // 0x10
    HuffNode* pChild;     // 0x14  the first child; the second is the next in the list
};

#define HUFFMAN_NODES 0x203

struct HuffTree {
    int bFixed;                      // 0x0000  the weights do not adapt (table 0)
    uint32_t nRevision;              // 0x0004  bumped when nodes swap; invalidates the lookup cache
    HuffNode aNodes[HUFFMAN_NODES];  // 0x0008
    TSList tFree;                    // 0x3050
    TSList tActive;                  // 0x305C
    int nUsed;                       // 0x3068
    HuffNode* apBySymbol[0x102];     // 0x306C
};

typedef char HuffNode_size[sizeof(HuffNode) == 0x18 ? 1 : -1];
typedef char HuffTree_size[sizeof(HuffTree) == 0x3474 ? 1 : -1];

static inline HuffNode* HuffNext(HuffNode* pNode) {
    uintptr_t nNext = pNode->tLink.nNext;
    return IsListEnd(nNext) ? NULL : (HuffNode*)nNext;
}

static inline HuffNode* HuffHead(TSList* pList) {
    uintptr_t nNext = pList->tTerminator.nNext;
    return IsListEnd(nNext) ? NULL : (HuffNode*)nNext;
}

// A node from the free list, or the next unused one.
static inline HuffNode* HuffTakeNode(HuffTree* pTree) {
    HuffNode* pNode = HuffHead(&pTree->tFree);
    if (pNode == NULL) {
        pNode = &pTree->aNodes[pTree->nUsed++];
    }
    return pNode;
}

// 1.14d mac: 0000eb42
void HUFFMAN_InitTree(HuffTree* pTree) {
    for (int i = 0; i < HUFFMAN_NODES; i++) {
        pTree->aNodes[i].tLink.pPrev = NULL;
        pTree->aNodes[i].tLink.nNext = 0;
    }
    pTree->tFree.nLinkOffset = 0;
    pTree->tFree.tTerminator.pPrev = &pTree->tFree.tTerminator;
    pTree->tFree.tTerminator.nNext = (uintptr_t)&pTree->tFree.tTerminator | 1;
    pTree->tActive.nLinkOffset = 0;
    pTree->tActive.tTerminator.pPrev = &pTree->tActive.tTerminator;
    pTree->tActive.tTerminator.nNext = (uintptr_t)&pTree->tActive.tTerminator | 1;
    pTree->nRevision = 1;
    pTree->nUsed = 0;
}

// Raises a node's weight by one and every ancestor's, keeping the list sorted by swapping
// a node with the last one that weighs less.
static void HuffRebalance(HuffTree* pTree, HuffNode* pStart, bool bExplicitFirstWeight, uint32_t nFirstWeight) {
    TSList* pActive = &pTree->tActive;
    uint32_t nWeight = nFirstWeight;
    for (HuffNode* pNode = pStart; pNode != NULL; pNode = pNode->pParent) {
        if (!bExplicitFirstWeight || pNode != pStart) {
            nWeight = pNode->nWeight + 1;
        }
        pNode->nWeight = nWeight;
        HuffNode* pSwap = pNode;
        HuffNode* pExisting;
        HuffNode* pCursor;
        do {
            pCursor = pSwap;
            pSwap = HuffNext(pCursor);
            pExisting = NULL;
            if (pSwap == NULL) {
                break;
            }
            pExisting = pSwap;
        } while (pSwap->nWeight < nWeight);
        // pCursor is the last node that weighs less than the new weight
        if (pCursor != pNode) {
            TSLIST_LinkNode(pActive, &pCursor->tLink, 2, &pNode->tLink);
            TSLIST_LinkNode(pActive, &pNode->tLink, 2, pExisting ? &pExisting->tLink : NULL);
            HuffNode* pCursorParentChild = pCursor->pParent->pChild;
            if (pNode->pParent->pChild == pNode) {
                pNode->pParent->pChild = pCursor;
            }
            if (pCursorParentChild == pCursor) {
                pCursor->pParent->pChild = pNode;
            }
            HuffNode* pTemp = pNode->pParent;
            pNode->pParent = pCursor->pParent;
            pCursor->pParent = pTemp;
            pTree->nRevision++;
        }
    }
}

// 1.14d mac: 0000ebb6
// Splits the lightest leaf in two: its old symbol and the new one, then rebalances.
void HUFFMAN_InsertNewSymbol(HuffTree* pTree, int nSymbol) {
    TSList* pActive = &pTree->tActive;
    HuffNode* pOld = HuffHead(&pTree->tActive);
    HuffNode* pNode = HuffHead(&pTree->tFree);
    if (pNode == NULL) {
        pNode = &pTree->aNodes[pTree->nUsed++];
    }
    TSLIST_LinkNode(pActive, &pNode->tLink, 1, NULL);
    pNode->pChild = NULL;
    pNode->nSymbol = pOld->nSymbol;
    pNode->nWeight = pOld->nWeight;
    pNode->pParent = pOld;
    pTree->apBySymbol[pNode->nSymbol] = pNode;
    HuffNode* pNew = HuffTakeNode(pTree);
    TSLIST_LinkNode(pActive, &pNew->tLink, 1, NULL);
    pNew->pChild = NULL;
    pNew->nSymbol = nSymbol;
    pNew->nWeight = 0;
    pNew->pParent = pOld;
    pTree->apBySymbol[nSymbol] = pNew;
    pOld->pChild = pNew;
    if (pNew != NULL) {
        uint32_t nWeight = 1;
        HuffNode* pCursorNode = pNew;
        while (true) {
            pCursorNode->nWeight = nWeight;
            HuffNode* pSwap = pCursorNode;
            HuffNode* pExisting;
            HuffNode* pCursor;
            do {
                pCursor = pSwap;
                pSwap = HuffNext(pCursor);
                pExisting = NULL;
                if (pSwap == NULL) {
                    break;
                }
                pExisting = pSwap;
            } while (pSwap->nWeight < nWeight);
            if (pCursor != pCursorNode) {
                TSLIST_LinkNode(pActive, &pCursor->tLink, 2, &pCursorNode->tLink);
                TSLIST_LinkNode(pActive, &pCursorNode->tLink, 2, pExisting ? &pExisting->tLink : NULL);
                HuffNode* pCursorParentChild = pCursor->pParent->pChild;
                if (pCursorNode->pParent->pChild == pCursorNode) {
                    pCursorNode->pParent->pChild = pCursor;
                }
                if (pCursorParentChild == pCursor) {
                    pCursor->pParent->pChild = pCursorNode;
                }
                HuffNode* pTemp = pCursorNode->pParent;
                pCursorNode->pParent = pCursor->pParent;
                pCursor->pParent = pTemp;
                pTree->nRevision++;
            }
            pCursorNode = pCursorNode->pParent;
            if (pCursorNode == NULL) {
                break;
            }
            nWeight = pCursorNode->nWeight + 1;
        }
    }
}

// 1.14d mac: 0000ed92
// A leaf per symbol with a weight in the table (sorted by weight), the end and escape
// symbols with weight 1, then the pairing of the lightest two until one node is left.
void HUFFMAN_BuildTree(HuffTree* pTree, uint8_t nWeightTable) {
    TSList* pActive = &pTree->tActive;
    HuffNode* pNode = HuffHead(&pTree->tActive);
    while (pNode != NULL) {
        TSLIST_LinkNode(&pTree->tFree, &pNode->tLink, 1, NULL);
        pNode = HuffHead(&pTree->tActive);
    }
    memset(pTree->apBySymbol, 0, sizeof(pTree->apBySymbol[0]) * 0x102);
    uint32_t nMaxWeight = 0;
    for (int nSymbol = 0; nSymbol != 0x100; nSymbol++) {
        uint32_t nWeight = gaCompressHuffmanWeightTables[nSymbol + nWeightTable * 0x102];
        if (nWeight != 0) {
            HuffNode* pLeaf = HuffTakeNode(pTree);
            TSLIST_LinkNode(pActive, &pLeaf->tLink, 2, NULL);
            pLeaf->pParent = NULL;
            pLeaf->pChild = NULL;
            pTree->apBySymbol[nSymbol] = pLeaf;
            pLeaf->nSymbol = nSymbol;
            pLeaf->nWeight = nWeight;
            if (nWeight < nMaxWeight) {
                HuffNode* pCursor = HuffHead(&pTree->tActive);
                HuffNode* pBefore;
                while (true) {
                    pBefore = NULL;
                    if (pCursor == NULL) {
                        break;
                    }
                    pBefore = pCursor;
                    if (nWeight <= pCursor->nWeight) {
                        break;
                    }
                    pCursor = (HuffNode*)pCursor->tLink.nNext;
                    if (IsListEnd((uintptr_t)pCursor)) {
                        pCursor = NULL;
                    }
                }
                TSLIST_LinkNode(pActive, &pLeaf->tLink, 2, pBefore ? &pBefore->tLink : NULL);
            } else {
                nMaxWeight = nWeight;
            }
        }
    }
    for (int i = 0; i != 2; i++) {
        HuffNode* pSpecial = HuffTakeNode(pTree);
        TSLIST_LinkNode(pActive, &pSpecial->tLink, 1, NULL);
        pSpecial->pParent = NULL;
        pSpecial->pChild = NULL;
        pTree->apBySymbol[0x100 + i] = pSpecial;
        pSpecial->nSymbol = 0x100 + i;
        pSpecial->nWeight = 1;
    }
    HuffNode* pLight = HuffHead(&pTree->tActive);
    while (pLight != NULL) {
        HuffNode* pSecond = HuffNext(pLight);
        if (pSecond == NULL) {
            break;
        }
        HuffNode* pParent = HuffTakeNode(pTree);
        TSLIST_LinkNode(pActive, &pParent->tLink, 2, NULL);
        pParent->pParent = NULL;
        uint32_t nSum = pSecond->nWeight + pLight->nWeight;
        pParent->nWeight = nSum;
        pParent->pChild = pLight;
        pLight->pParent = pParent;
        pSecond->pParent = pParent;
        if (nSum < nMaxWeight) {
            HuffNode* pCursor = pSecond;
            HuffNode* pBefore;
            do {
                pCursor = HuffNext(pCursor);
                pBefore = NULL;
            } while (pCursor != NULL && (pBefore = pCursor, pCursor->nWeight < nSum));
            TSLIST_LinkNode(pActive, &pParent->tLink, 2, pBefore ? &pBefore->tLink : NULL);
        } else {
            nMaxWeight = nSum;
        }
        pLight = HuffNext(pSecond);
    }
    pTree->nRevision = 1;
}

// A 7-bit lookup of already decoded code prefixes, valid while its revision is the tree's.
struct HuffCacheEntry {
    uint32_t nRevision;
    uint32_t nBits;
    uint32_t nValue;  // the symbol (up to 7 bits) or the node reached after 7 bits
};

// 1.14d mac: 0000f5b0
// The first input dword selects the weight table (0: fixed weights). Codes are read a bit
// at a time from a 32-bit buffer refilled 16 bits at a time; 0x100 ends the data and
// 0x101 is followed by a literal byte that joins the tree.
void HUFFMAN_Decompress(uint8_t* pOutBuffer, int* pnOutSize, const uint32_t* pInBuffer) {
    HuffTree tTree;
    HuffCacheEntry aLookupCache[0x80];
    const uint8_t* pInput = (const uint8_t*)(pInBuffer + 1);
    uint32_t dwBitBuffer = *pInBuffer;
    HUFFMAN_InitTree(&tTree);
    for (int i = 0; i < 0x80; i++) {
        aLookupCache[i].nRevision = 0;
    }
    int nRemaining = *pnOutSize;
    uint8_t* pOut = pOutBuffer;
    int nWritten;
    if (nRemaining == 0) {
        nWritten = 0;
    } else {
        uint8_t nWeightTable = (uint8_t)dwBitBuffer;
        HUFFMAN_BuildTree(&tTree, nWeightTable);
        tTree.bFixed = nWeightTable == 0;
        dwBitBuffer >>= 8;
        uint32_t nBitCount = 0x18;
        while (true) {
            if (nBitCount < 8) {
                dwBitBuffer = (uint32_t)*(const uint16_t*)pInput << (nBitCount & 0x1f) | dwBitBuffer;
                pInput += 2;
                nBitCount += 0x10;
            }
            uint32_t nIndex = dwBitBuffer & 0x7f;
            HuffCacheEntry* pEntry = &aLookupCache[nIndex];
            uint32_t nSymbol;
            HuffNode* pStart = NULL;
            bool bWalk = true;
            if (pEntry->nRevision == tTree.nRevision) {
                uint32_t nBits = pEntry->nBits;
                if (nBits > 7) {
                    nBitCount -= 7;
                    dwBitBuffer >>= 7;
                    pStart = (HuffNode*)pEntry->nValue;
                } else {
                    nBitCount -= nBits;
                    dwBitBuffer >>= (nBits & 0x1f);
                    nSymbol = pEntry->nValue;
                    bWalk = false;
                }
            } else {
                // from the root, the last node of the live list
                TSLink* pLast = tTree.tActive.tTerminator.pPrev;
                uintptr_t nRoot = *(uintptr_t*)((char*)pLast->pPrev + 4);
                pStart = IsListEnd(nRoot) ? NULL : (HuffNode*)nRoot;
            }
            if (bWalk) {
                uint32_t nDepth = 0;
                HuffNode* pAtSeven = NULL;
                HuffNode* pLeaf = pStart->pChild;
                HuffNode* pStep;
                do {
                    pStep = pLeaf;
                    uint32_t nBit = dwBitBuffer & 1;
                    nBitCount--;
                    if (nBitCount == 0) {
                        dwBitBuffer = *(const uint32_t*)pInput;
                        pInput += 4;
                        nBitCount = 0x20;
                    } else {
                        dwBitBuffer >>= 1;
                    }
                    if (nBit != 0) {
                        pStep = (HuffNode*)pStep->tLink.nNext;
                    }
                    if (nDepth == 6) {
                        pAtSeven = pStep;
                    }
                    nDepth++;
                    pLeaf = pStep->pChild;
                } while (pStep->pChild != NULL);
                if (pEntry->nRevision != tTree.nRevision) {
                    if (nDepth < 8) {
                        uint32_t nFill = 0xffffffffU >> ((0x20 - nDepth) & 0x1f) & nIndex;
                        uint32_t nStride = 1u << (nDepth & 0x1f);
                        while (true) {
                            aLookupCache[nFill].nRevision = tTree.nRevision;
                            aLookupCache[nFill].nBits = nDepth;
                            aLookupCache[nFill].nValue = pStep->nSymbol;
                            nFill += nStride;
                            if (nFill > 0x7f) {
                                break;
                            }
                        }
                    } else {
                        pEntry->nRevision = tTree.nRevision;
                        pEntry->nBits = nDepth;
                        pEntry->nValue = (uint32_t)pAtSeven;
                    }
                }
                nSymbol = pStep->nSymbol;
            }
            if (nSymbol == 0x100) {
                break;
            }
            if (nSymbol == 0x101) {
                if (nBitCount < 9) {
                    dwBitBuffer = (uint32_t)*(const uint16_t*)pInput << (nBitCount & 0x1f) | dwBitBuffer;
                    pInput += 2;
                    nBitCount += 0x10;
                }
                nSymbol = dwBitBuffer & 0xff;
                nBitCount -= 8;
                dwBitBuffer >>= 8;
                HUFFMAN_InsertNewSymbol(&tTree, nSymbol);
                if (tTree.bFixed == 0) {
                    HuffRebalance(&tTree, tTree.apBySymbol[nSymbol], false, 0);
                }
            }
            *pOut++ = (uint8_t)nSymbol;
            nRemaining--;
            if (nRemaining == 0) {
                break;
            }
            if (tTree.bFixed != 0) {
                HuffRebalance(&tTree, tTree.apBySymbol[nSymbol], false, 0);
            }
        }
        nWritten = (int)(pOut - pOutBuffer);
    }
    *pnOutSize = nWritten;
    TSLIST_UnlinkAll(&tTree.tActive);
    TSLINK_Unlink(&tTree.tActive.tTerminator);
    TSLIST_UnlinkAll(&tTree.tFree);
    TSLINK_Unlink(&tTree.tFree.tTerminator);
    for (int i = HUFFMAN_NODES - 1; i >= 1; i--) {
        TSLINK_Unlink(&tTree.aNodes[i].tLink);
        TSLINK_Unlink(&tTree.aNodes[i].tLink);
    }
}

// ---- blast, zlib ----

struct SCompBlastState {
    uint8_t* pOut;     // 0x00
    uint32_t nOutPos;  // 0x04
    uint32_t nOutSize; // 0x08
    uint8_t* pIn;      // 0x0C
    uint32_t nInPos;   // 0x10
    uint32_t nInSize;  // 0x14
};

// 1.14d mac: 0000f14c
// All the input at once.
static unsigned SCOMP_BlastReadCallback(void* pState, unsigned char** ppBuffer) {
    *ppBuffer = ((SCompBlastState*)pState)->pIn;
    return ((SCompBlastState*)pState)->nInSize;
}

// 1.14d mac: 0000f15f
static int SCOMP_BlastWriteCallback(void* pState, unsigned char* pData, unsigned nSize) {
    SCompBlastState* pBlast = (SCompBlastState*)pState;
    memcpy(pBlast->pOut + pBlast->nOutPos, pData, nSize);
    pBlast->nOutPos += nSize;
    return 0;
}

// 1.14d mac: 0000f0e9
void SCOMP_DecompressImplode(uint8_t* pOutBuffer, uint32_t* pnOutSize, uint8_t* pInBuffer, uint32_t nInSize) {
    SCompBlastState tState;
    tState.pOut = pOutBuffer;
    tState.nOutPos = 0;
    tState.nOutSize = *pnOutSize;
    tState.pIn = pInBuffer;
    tState.nInPos = 0;
    tState.nInSize = nInSize;
    blast(SCOMP_BlastReadCallback, &tState, SCOMP_BlastWriteCallback, &tState);
    *pnOutSize = tState.nOutPos;
}

// ---- the shared temporary buffer ----

// 1.14d mac: 00554e14
static pthread_mutex_t s_tTempMutex;
// 1.14d mac: 003f02a0
static void* s_pTempBuffer;
// 1.14d mac: 003f02a4
// The thread using the shared buffer, 0 while it is free.
static DWORD s_dwTempOwner;
// 1.14d mac: 003f02a8
static uint32_t s_nTempSize;

// 1.14d mac: 0000f191
// The shared buffer when free (*pShared = 1), else a block of its own (*pShared = 0).
static void* SCOMP_AcquireTempBuffer(uint32_t nSize, int32_t* pShared) {
    pthread_mutex_lock(&s_tTempMutex);
    DWORD dwThread = GetCurrentThreadId();
    void* pBuffer;
    if (s_pTempBuffer == NULL) {
        s_nTempSize = nSize;
        s_pTempBuffer = SMemAlloc(nSize, SCOMP_FILE, 0x69d, 0);
        s_dwTempOwner = 0;
    } else if (s_dwTempOwner != 0) {
        pBuffer = SMemAlloc(nSize, SCOMP_FILE, 0x6a3, 0);
        *pShared = 0;
        pthread_mutex_unlock(&s_tTempMutex);
        return pBuffer;
    }
    if (s_nTempSize < nSize) {
        SMemFree(s_pTempBuffer, SCOMP_FILE, 0x6ac, 0);
        s_nTempSize = nSize;
        s_pTempBuffer = SMemAlloc(nSize, SCOMP_FILE, 0x6ae, 0);
    }
    pBuffer = s_pTempBuffer;
    *pShared = 1;
    s_dwTempOwner = dwThread;
    pthread_mutex_unlock(&s_tTempMutex);
    return pBuffer;
}

// 1.14d mac: 0000f2c1
static void SCOMP_ReleaseTempBuffer(void* pBuf, int32_t bShared) {
    if (pBuf != NULL) {
        pthread_mutex_lock(&s_tTempMutex);
        if (bShared == 0) {
            SMemFree(pBuf, SCOMP_FILE, 0x6c4, 0);
        } else {
            s_dwTempOwner = 0;
        }
        pthread_mutex_unlock(&s_tTempMutex);
    }
}

// ---- SCompDecompress ----

typedef void (*SCompCodec)(void* pDest, uint32_t* pnDestSize, void* pSource, uint32_t nSourceSize, uint32_t dwFlags);

struct SCompMethod {
    uint32_t dwFlag;
    SCompCodec pfnDecompress;
};

// 1.14d mac: 003cf6f4
// Imploding, zlib, ADPCM stereo, ADPCM mono, Huffman, in the order the data was
// compressed in; the last flag set is undone first.
extern const SCompMethod g_atSCompMethods[5];

// 1.14d mac: 0000f32f
// The first byte holds the methods that were applied; they are undone last to first, the
// data ping-ponging between the destination and a temporary buffer.
BOOL SCompDecompress(uint8_t* pDst, uint32_t* pnDstSize, uint8_t* pSrc, uint32_t nSrcSizeIn, uint32_t dwFlags) {
    uint32_t nSize = *pnDstSize;
    if (nSrcSizeIn == 0 || pSrc == NULL || nSize < nSrcSizeIn || pDst == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    if (nSize == nSrcSizeIn) {
        if (pDst != pSrc && 0 < (int)nSrcSizeIn) {
            memmove(pDst, pSrc, nSrcSizeIn);
        }
        return 1;
    }
    uint8_t* pSrcCur = pSrc + 1;
    uint32_t dwMethodFlags = *pSrc;
    uint32_t nMethods = 0;
    uint32_t dwUnknown = dwMethodFlags;
    for (int i = 4; i >= 0; i--) {
        dwUnknown &= ~g_atSCompMethods[i].dwFlag;
        nMethods += (g_atSCompMethods[i].dwFlag & dwMethodFlags) != 0;
    }
    if (dwUnknown != 0) {
        return 0;
    }
    uint32_t nSrcSize = nSrcSizeIn - 1;
    int32_t bShared = 0;
    uint8_t* pTemp = NULL;
    if (nMethods > 1 ||
        (pSrcCur < pDst + nSrcSize && nMethods != 0 && pDst < pSrc + nSrcSizeIn)) {
        pTemp = (uint8_t*)SCOMP_AcquireTempBuffer(nSize, &bShared);
    }
    for (int i = 4; i >= 0; i--) {
        if ((g_atSCompMethods[i].dwFlag & dwMethodFlags) != 0) {
            nMethods--;
            uint8_t* pTarget = pDst;
            if ((nMethods & 1) != 0) {
                pTarget = pTemp;
            }
            uint8_t* pStage = pTarget;
            // never decode into a buffer the source overlaps
            if (pSrcCur < pTarget + nSrcSize && pTarget < pSrcCur + nSrcSize) {
                pStage = pTemp;
                if (pTemp < pTarget + nSrcSize && pTarget < pTemp + nSrcSize) {
                    pStage = pDst;
                }
            }
            uint32_t nSavedSize = nSize;
            g_atSCompMethods[i].pfnDecompress(pStage, &nSavedSize, pSrcCur, nSrcSize, dwFlags);
            pSrcCur = pStage;
            nSrcSize = nSavedSize;
        }
    }
    if (pSrcCur != pDst && 0 < (int)nSrcSize) {
        memmove(pDst, pSrcCur, nSrcSize);
    }
    *pnDstSize = nSrcSize;
    SCOMP_ReleaseTempBuffer(pTemp, bShared);
    return 1;
}

// 1.14d mac: 0000f500
BOOL SCOMP_Destroy(void) {
    if (s_pTempBuffer != NULL) {
        SMemFree(s_pTempBuffer, SCOMP_FILE, 0x7b0, 0);
        s_pTempBuffer = NULL;
    }
    return 1;
}

// ---- ADPCM ----

// 1.14d mac: 0000fae1
// Two channels at most; each starts from its first sample, the step index at 0x2c. Codes
// with bit 7 set change the step index (or repeat the last sample); the others add a
// delta built from the step size and move the index.
static uint32_t ADPCM_Decompress(int16_t* pOutBuffer, uint32_t nOutSize, const uint8_t* pInBuffer, int32_t nInSize,
                                 uint32_t nChannels) {
    int aiPredicted[2];
    int aiStepIndex[2];
    aiStepIndex[0] = 0x2c;
    aiStepIndex[1] = 0x2c;
    int16_t* pOut = pOutBuffer;
    const uint8_t* pIn = pInBuffer;
    uint32_t nOutBytesLeft = nOutSize;
    if (nChannels != 0) {
        int nUsed = 0;
        for (uint32_t nChannel = 0; nChannel < nChannels; nChannel++) {
            int16_t nSample = *(const int16_t*)(pInBuffer + nChannel * 2 + 2);
            aiPredicted[nChannel] = nSample;
            if (nOutSize + nUsed < 2) {
                return -nUsed;
            }
            pOutBuffer[nChannel] = nSample;
            nUsed -= 2;
        }
        nOutBytesLeft = nOutSize + nUsed;
        pOut = (int16_t*)((char*)pOutBuffer - nUsed);
        pIn = pInBuffer - nUsed;
    }
    const uint8_t* pInEnd = pInBuffer + nInSize;
    pIn += 2;
    if (pIn < pInEnd) {
        uint32_t nChannel = nChannels - 1;
        do {
            while (true) {
                if (nChannels == 2) {
                    nChannel = nChannel == 0;
                }
                uint8_t byCode = *pIn;
                if ((byCode & 0x80) == 0) {
                    break;
                }
                const uint8_t* pNext = pIn + 1;
                int nNewIndex;
                bool bAdjust = true;
                if ((byCode & 0x7f) == 1) {
                    nNewIndex = aiStepIndex[nChannel] + 8;
                    if (0x58 < nNewIndex) {
                        nNewIndex = 0x58;
                    }
                } else if ((byCode & 0x7f) == 2) {
                    nNewIndex = aiStepIndex[nChannel] - 8;
                    if (nNewIndex < 0) {
                        nNewIndex = 0;
                    }
                } else {
                    bAdjust = false;
                    nNewIndex = 0;
                    if ((byCode & 0x7f) == 0) {
                        if (aiStepIndex[nChannel] != 0) {
                            aiStepIndex[nChannel]--;
                        }
                        if (1 < nOutBytesLeft) {
                            *pOut = (int16_t)aiPredicted[nChannel];
                            nOutBytesLeft -= 2;
                            pOut++;
                            pIn++;
                            goto next;
                        }
                        goto done;
                    }
                }
                if (bAdjust) {
                    aiStepIndex[nChannel] = nNewIndex;
                    if (nChannels == 2) {
                        nChannel = nChannel == 0;
                    }
                }
                pIn = pNext;
                if (pInEnd <= pNext) {
                    goto done;
                }
            }
            {
                uint8_t byCode = *pIn;
                int nIndex = aiStepIndex[nChannel];
                uint32_t nStep = gdwADPCMCoefficient[nIndex];
                int nDelta = (nStep >> (pInBuffer[1] & 0x1f)) + ((int)((uint32_t)byCode << 31) >> 31 & nStep);
                if ((byCode & 2) != 0) {
                    nDelta += nStep >> 1;
                }
                if ((byCode & 4) != 0) {
                    nDelta += nStep >> 2;
                }
                if ((byCode & 8) != 0) {
                    nDelta += nStep >> 3;
                }
                if ((byCode & 0x10) != 0) {
                    nDelta += nStep >> 4;
                }
                if ((byCode & 0x20) != 0) {
                    nDelta += nStep >> 5;
                }
                int nPredicted;
                if ((byCode & 0x40) == 0) {
                    nPredicted = aiPredicted[nChannel] + nDelta;
                    if (0x7fff < nPredicted) {
                        nPredicted = 0x7fff;
                    }
                } else {
                    nPredicted = aiPredicted[nChannel] - nDelta;
                    if (nPredicted < -0x8000) {
                        nPredicted = -0x8000;
                    }
                }
                aiPredicted[nChannel] = nPredicted;
                if (nOutBytesLeft < 2) {
                    break;
                }
                *pOut = (int16_t)nPredicted;
                nIndex += gnSSCompDeflateFlags[byCode & 0x1f];
                aiStepIndex[nChannel] = nIndex;
                if (nIndex < 0) {
                    aiStepIndex[nChannel] = 0;
                } else if (0x58 < nIndex) {
                    aiStepIndex[nChannel] = 0x58;
                }
                nOutBytesLeft -= 2;
                pOut++;
                pIn++;
            }
        next:;
        } while (pIn < pInEnd);
    }
done:
    return (uint32_t)((char*)pOut - (char*)pOutBuffer);
}

// 1.14d mac: 0000f54e
void SCOMP_DecompressADPCMMono(int16_t* pOutBuffer, uint32_t* pnOutSize, const uint8_t* pInBuffer, int32_t nInSize) {
    *pnOutSize = ADPCM_Decompress(pOutBuffer, *pnOutSize, pInBuffer, nInSize, 1);
}

// 1.14d mac: 0000f57f
void SCOMP_DecompressADPCMStereo(int16_t* pOutBuffer, uint32_t* pnOutSize, const uint8_t* pInBuffer,
                                 int32_t nInSize) {
    *pnOutSize = ADPCM_Decompress(pOutBuffer, *pnOutSize, pInBuffer, nInSize, 2);
}

// 1.14d mac: 0000fad8
// A tail call to zlib.
void thunk_uncompress(uint8_t* pDest, unsigned long* pnDestLen, const uint8_t* pSource, unsigned long nSourceLen) {
    uncompress(pDest, pnDestLen, pSource, nSourceLen);
}

// 1.14d mac: 0000fe80
// The static initialiser: the temp buffer's mutex, destroyed at exit.
void SCOMP_InitStatics(void) {
    CRITSECT_Init(&s_tTempMutex);
    __cxa_atexit(CRITSECT_Destroy, &s_tTempMutex, &__dso_handle);
}

#endif  // D2_OS_MAC
