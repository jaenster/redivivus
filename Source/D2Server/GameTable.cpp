// D2Server: the member functions of Storm's TSHashTable<SGAMEDATA,HASHKEY_NONE> and
// of the lists inside it, as the 1.00 D2Server.dll instantiates them for the
// game-data table (ToMCP.cpp's g_gameDataParam1). No __FILE__ string survives; the
// name is the Ghidra program's. Several functions are the same template member
// emitted more than once; each copy is kept, as the binary has it.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.
//
// The layouts are in D2Server.h: GameTableLink, GameTableList, GameTableListArray,
// SGAMEDATA, GameTableChunk and GameDataTable. A link's previous field is either a
// complemented pointer to the previous link (negative) or the previous element, to
// which the list's link offset is added.

#include "Source/D2Server/GameTable.h"
#include "Source/D2Server/Core/ToMCP.h"
#include "Source/D2Server/D2ServerImports.h"

namespace D2Server::GameTable {

using namespace D2Server::Imports;

static const char s_szSGameData[] = ".?AUSGAMEDATA@@";
static const char s_szHashObjectChunk[] = ".?AV?$TSHashObjectChunk@USGAMEDATA@@VHASHKEY_NONE@@@@";
static const char s_szExplicitList[] = ".?AV?$TSExplicitList@USGAMEDATA@@$0?CCCCCCCD@@@";

// The vtables of the table's class and its bases: delete an element, allocate one,
// the scalar deleting destructor, the base destructor.
static void * const s_vtblExplicitList[4] = {                         // 1.00 D2Server: 1001f004
    (void *)SGAMEDATA_FreeElement_B, (void *)SGAMEDATA_AllocInChunk_B,
    (void *)GameTable_ClearChunks_ScalarDelete, (void *)TSExplicitList_SGAMEDATA_Destructor,
};
static void * const s_vtblHashObject[4] = {                           // 1.00 D2Server: 1001f018
    (void *)SGAMEDATA_FreeElement_A, (void *)SGAMEDATA_AllocInChunk_A,
    (void *)GameTable_ScalarDelete, (void *)TSHashObject_SGAMEDATA_Destructor,
};
static void * const s_vtblGameTable[4] = {                            // 1.00 D2Server: 1001f02c
    (void *)SGAMEDATA_FreeElement_A, (void *)SGAMEDATA_AllocInChunk_A,
    (void *)ToMCP_ScalarDelete, (void *)TSHashObject_SGAMEDATA_Destructor,
};
static void * const s_vtblHashTable[4] = {                            // 1.00 D2Server: 1001f040
    (void *)SGAMEDATA_FreeElement_A, (void *)SGAMEDATA_AllocInChunk_A,
    (void *)GameTable_DestructBody_ScalarDelete, (void *)TSHashObject_SGAMEDATA_Destructor,
};

typedef void (__thiscall * ElementDeleteFn)(GameDataTable * pThis, SGAMEDATA * pElement);
typedef GameTableLink * (__thiscall * PrevLinkFn)(GameTableLink * pLink, intptr_t nOffset);

static inline GameTableLink * GtLink(uintptr_t uAddress) {
    return (GameTableLink *)uAddress;
}

// The link of the element uElement in pList.
static inline GameTableLink * GtElementLink(GameTableList * pList, uintptr_t uElement) {
    return GtLink((uintptr_t)pList->nLinkOffset + uElement);
}

// The link unlink every member inlines: point the previous link past this one.
static inline void GtUnlink(GameTableLink * pLink) {
    uintptr_t uNext = pLink->uNext;
    if (uNext == 0) {
        return;
    }
    uintptr_t uPrev = pLink->uPrev;
    GameTableLink * pPrevLink;
    if ((intptr_t)uPrev < 0) {
        pPrevLink = GtLink(~uPrev);
    } else {
        pPrevLink = GtLink((uintptr_t)pLink + (uPrev - GtLink(uNext)->uPrev));
    }
    pPrevLink->uNext = uNext;
    GtLink(pLink->uNext)->uPrev = pLink->uPrev;
    pLink->uNext = 0;
    pLink->uPrev = 0;
}

// Unlinks every element of a list, last first.
static inline void GtUnlinkAllElements(GameTableList * pList) {
    while ((intptr_t)pList->head.uPrev > 0) {
        GtUnlink(GtElementLink(pList, pList->head.uPrev));
    }
}

// The out-of-line form of the unlink, with the previous link found by
// TSListNode_GetOffset.
static inline void GtUnlinkVia(GameTableLink * pLink, PrevLinkFn pfnGetOffset) {
    if (pLink->uNext != 0) {
        GameTableLink * pPrevLink = pfnGetOffset(pLink, -1);
        pPrevLink->uNext = pLink->uNext;
        GtLink(pLink->uNext)->uPrev = pLink->uPrev;
        pLink->uNext = 0;
        pLink->uPrev = 0;
    }
}

// 1.00 D2Server: 1000a240
GameDataTable * __fastcall GameTable_Construct(GameDataTable * pThis) {
    TSListHead_Init_Sentinel(&pThis->elements);
    TSListNode_Zero(&pThis->buckets);
    pThis->pVtable = s_vtblExplicitList;
    pThis->dw10 = 0;
    TSExplicitList_Resize_DrainOld(&pThis->elements, offsetof(SGAMEDATA, listLink));
    pThis->nBucketMask = 3;
    TSExplicitList_Resize(&pThis->buckets, 4);
    for (uint32_t i = 0; i <= pThis->nBucketMask; ++i) {
        TSExplicitList_Resize_DrainOld(&pThis->buckets.pLists[i], offsetof(SGAMEDATA, bucketLink));
    }
    TSListHead_Init(&pThis->reuse);
    pThis->reuse.nLinkOffset = (intptr_t)(int32_t)0xDDDDDDDD;
    TSListHead_InitEmpty(&pThis->reuse);
    TSListHead_Init_B(&pThis->chunks);
    pThis->chunks.nLinkOffset = offsetof(GameTableChunk, link);
    TSListHead_InitEmpty_B(&pThis->chunks);
    pThis->pVtable = s_vtblHashObject;
    pThis->dw34 = 0x10;
    TSExplicitList_Resize_DrainOld(&pThis->reuse, offsetof(SGAMEDATA, bucketLink));
    pThis->pVtable = s_vtblHashTable;
    pThis->dwHashSeed = ~(uint32_t)(uintptr_t)pThis & 0x0FFFFFFF;
    pThis->dw4C = 0;
    InitializeCriticalSection(&pThis->cs);
    pThis->pVtable = s_vtblGameTable;
    return pThis;
}

// 1.00 D2Server: 1000a3c0
void __thiscall SGAMEDATA_FreeElement_A(GameDataTable * pThis, SGAMEDATA * pElement) {
    SGAMEDATA_UnlinkFromBothLists(pElement);
    SMemFree(pElement, s_szSGameData, -2, 0);
}

// 1.00 D2Server: 1000a3f0
SGAMEDATA * __thiscall SGAMEDATA_AllocInChunk_A(GameDataTable * pThis, GameTableList * pList, int nExtra, uint32_t dwFlags) {
    return SGAMEDATA_Chunk_AllocElement(pList, 1, nExtra, dwFlags);
}

// 1.00 D2Server: 1000a420
void __fastcall TSHashObject_SGAMEDATA_Destructor(GameDataTable * pThis) {
    pThis->dw10 = 0;
    while ((intptr_t)pThis->elements.head.uPrev > 0) {
        TSList_UnlinkNode(GtElementLink(&pThis->elements, pThis->elements.head.uPrev));
    }
    for (uint32_t i = 0; i < pThis->buckets.nCount; ++i) {
        intptr_t nElement;
        while ((nElement = (intptr_t)pThis->buckets.pLists[i].head.uPrev) > 0) {
            ((ElementDeleteFn)pThis->pVtable[0])(pThis, (SGAMEDATA *)nElement);
        }
    }
    intptr_t nPtr;
    while ((nPtr = (intptr_t)pThis->chunks.head.uPrev) > 0) {
        SGAMEDATA_Chunk_FreeAll((GameTableChunk *)nPtr);
        SMemFree((void *)nPtr, s_szHashObjectChunk, -2, 0);
    }
    while ((nPtr = (intptr_t)pThis->reuse.head.uPrev) > 0) {
        SGAMEDATA_UnlinkFromBothLists((SGAMEDATA *)nPtr);
        SMemFree((void *)nPtr, s_szSGameData, -2, 0);
    }
}

// 1.00 D2Server: 1000a4e0
void __thiscall SGAMEDATA_FreeElement_B(GameDataTable * pThis, SGAMEDATA * pElement) {
    SGAMEDATA_UnlinkFromBothLists(pElement);
    SMemFree(pElement, s_szSGameData, -2, 0);
}

// 1.00 D2Server: 1000a510
SGAMEDATA * __thiscall SGAMEDATA_AllocInChunk_B(GameDataTable * pThis, GameTableList * pList, int nExtra, uint32_t dwFlags) {
    return SGAMEDATA_Chunk_AllocElement(pList, 1, nExtra, dwFlags);
}

// 1.00 D2Server: 1000a540
void __fastcall GameTable_ClearAllChunks(GameDataTable * pThis) {
    pThis->pVtable = s_vtblExplicitList;
    pThis->dw10 = 0;
    while ((intptr_t)pThis->elements.head.uPrev >= 1) {
        TSList_UnlinkNode(TSListNode_GetNodePtr(&pThis->elements, (intptr_t)pThis->elements.head.uPrev));
    }
    for (uint32_t i = 0; i < pThis->buckets.nCount; ++i) {
        GameTableList * pBucket = &pThis->buckets.pLists[i];
        while ((intptr_t)pBucket->head.uPrev >= 1) {
            GtUnlink(GtElementLink(pBucket, pBucket->head.uPrev));
        }
    }
    for (uint32_t i = 0; i < pThis->buckets.nCount; ++i) {
        TSList_SGAMEDATA_UnlinkNode_C(&pThis->buckets.pLists[i]);
    }
    if (pThis->buckets.pLists != nullptr) {
        SMemFree(pThis->buckets.pLists, s_szExplicitList, -2, 0);
    }
    while ((intptr_t)pThis->elements.head.uPrev > 0) {
        GtUnlinkVia(GtElementLink(&pThis->elements, pThis->elements.head.uPrev), TSListNode_GetOffset);
    }
    GtUnlink(&pThis->elements.head);
}

// 1.00 D2Server: 1000a720
void __fastcall TSExplicitList_SGAMEDATA_Destructor(GameDataTable * pThis) {
    pThis->dw10 = 0;
    while ((intptr_t)pThis->elements.head.uPrev > 0) {
        TSList_UnlinkNode(GtElementLink(&pThis->elements, pThis->elements.head.uPrev));
    }
    for (uint32_t i = 0; i < pThis->buckets.nCount; ++i) {
        GameTableList * pBucket = &pThis->buckets.pLists[i];
        while ((intptr_t)pBucket->head.uPrev >= 1) {
            GtUnlink(GtElementLink(pBucket, pBucket->head.uPrev));
        }
    }
    pThis->dw10 = 0;
    pThis->nBucketMask = 0;
    for (uint32_t i = 0; i < pThis->buckets.nCount; ++i) {
        TSList_SGAMEDATA_ScalarDelete(&pThis->buckets.pLists[i], 0);
    }
    if (pThis->buckets.pLists != nullptr) {
        SMemFree(pThis->buckets.pLists, s_szExplicitList, -2, 0);
    }
    pThis->buckets.nCapacity = 0;
    pThis->buckets.nCount = 0;
    pThis->buckets.pLists = nullptr;
}

// 1.00 D2Server: 1000a830
GameDataTable * __thiscall ToMCP_ScalarDelete(GameDataTable * pThis, uint8_t nFlags) {
    Core::ToMCP::ToMCP_Destructor(pThis);
    if ((nFlags & 1) && pThis) {
        SMemFree(pThis, "delete", -1, 0);
    }
    return pThis;
}

// 1.00 D2Server: 1000a870
SGAMEDATA * __thiscall SGAMEDATA_ScalarDelete(SGAMEDATA * pThis, uint8_t nFlags) {
    SGAMEDATA_UnlinkFromBothLists(pThis);
    if ((nFlags & 1) && pThis) {
        SMemFree(pThis, "delete", -1, 0);
    }
    return pThis;
}

// 1.00 D2Server: 1000a8b0
GameDataTable * __thiscall GameTable_ScalarDelete(GameDataTable * pThis, uint8_t nFlags) {
    GameTable_Destruct(pThis);
    if ((nFlags & 1) && pThis) {
        SMemFree(pThis, "delete", -1, 0);
    }
    return pThis;
}

// The table's destructor below its own class: the chunks and elements, then the
// base lists.
// 1.00 D2Server: 1000a8f0
void __fastcall GameTable_Destruct(GameDataTable * pThis) {
    pThis->pVtable = s_vtblHashObject;
    intptr_t nPtr;
    while ((nPtr = (intptr_t)pThis->chunks.head.uPrev) > 0) {
        SGAMEDATA_Chunk_FreeAll((GameTableChunk *)nPtr);
        SMemFree((void *)nPtr, s_szHashObjectChunk, -2, 0);
    }
    while ((nPtr = (intptr_t)pThis->reuse.head.uPrev) > 0) {
        SGAMEDATA_UnlinkFromBothLists_B((SGAMEDATA *)nPtr);
        SMemFree((void *)nPtr, s_szSGameData, -2, 0);
    }
    while ((intptr_t)pThis->chunks.head.uPrev > 0) {
        TSList_UnlinkNode_C(GtElementLink(&pThis->chunks, pThis->chunks.head.uPrev));
    }
    GtUnlinkVia(&pThis->chunks.head, TSListNode_GetOffset_B);
    while ((intptr_t)pThis->reuse.head.uPrev > 0) {
        TSList_UnlinkNode(GtElementLink(&pThis->reuse, pThis->reuse.head.uPrev));
    }
    GtUnlinkVia(&pThis->reuse.head, TSListNode_GetOffset);
    pThis->pVtable = s_vtblExplicitList;
    pThis->dw10 = 0;
    TSList_UnlinkAll(&pThis->elements);
    for (uint32_t i = 0; i < pThis->buckets.nCount; ++i) {
        intptr_t nSlot;
        while ((nSlot = (intptr_t)pThis->buckets.pLists[i].head.uPrev) >= 1) {
            TSList_UnlinkNode(TSListNode_GetNodePtr(&pThis->buckets.pLists[i], nSlot));
        }
    }
    TSExplicitList_SGAMEDATA_UnlinkAll(&pThis->buckets);
    TSList_UnlinkAll(&pThis->elements);
    TSList_UnlinkNode(&pThis->elements.head);
}

// 1.00 D2Server: 1000aaf0
void __fastcall TSList_SGAMEDATA_UnlinkNode(GameTableList * pList) {
    GtUnlinkAllElements(pList);
    GtUnlink(&pList->head);
}

// 1.00 D2Server: 1000ab90
void __fastcall TSList_SGAMEDATA_UnlinkNode_B(GameTableList * pList) {
    GtUnlinkAllElements(pList);
    GtUnlink(&pList->head);
}

// 1.00 D2Server: 1000ac30
GameDataTable * __thiscall GameTable_ClearChunks_ScalarDelete(GameDataTable * pThis, uint8_t nFlags) {
    GameTable_ClearAllChunks(pThis);
    if ((nFlags & 1) && pThis) {
        SMemFree(pThis, "delete", -1, 0);
    }
    return pThis;
}

// 1.00 D2Server: 1000ac70
void __fastcall TSExplicitList_SGAMEDATA_FreeAllBuckets(GameTableListArray * pThis) {
    for (uint32_t i = 0; i < pThis->nCount; ++i) {
        GameTableList * pBucket = &pThis->pLists[i];
        while ((intptr_t)pBucket->head.uPrev > 0) {
            TSList_UnlinkNode(GtLink(TSList_OffsetAdd((intptr_t)pBucket->head.uPrev, pBucket->nLinkOffset)));
        }
        TSList_UnlinkNode(&pBucket->head);
    }
    if (pThis->pLists != nullptr) {
        SMemFree(pThis->pLists, s_szExplicitList, -2, 0);
    }
}

// 1.00 D2Server: 1000ad50
void __fastcall SGAMEDATA_UnlinkFromBothLists(SGAMEDATA * pElement) {
    GtUnlink(&pElement->listLink);
    GtUnlink(&pElement->bucketLink);
}

// 1.00 D2Server: 1000ade0
void __fastcall SGAMEDATA_UnlinkFromBothLists_B(SGAMEDATA * pElement) {
    GtUnlink(&pElement->listLink);
    GtUnlink(&pElement->bucketLink);
}

// 1.00 D2Server: 1000ae70
void __fastcall TSList_UnlinkSingleNode(GameTableLink * pLink) {
    GtUnlink(pLink);
}

// 1.00 D2Server: 1000aec0
void __fastcall TSList_SGAMEDATA_UnlinkNode_C(GameTableList * pList) {
    GtUnlinkAllElements(pList);
    GtUnlink(&pList->head);
}

// A new element, its extra bytes after SGAMEDATA, linked at the tail of the list
// (nLinkMode 1) or at its head (any other nonzero mode).
// 1.00 D2Server: 1000af60
SGAMEDATA * __thiscall SGAMEDATA_Chunk_AllocElement(GameTableList * pList, int nLinkMode, int nExtra, uint32_t dwFlags) {
    SGAMEDATA * pBlock = (SGAMEDATA *)SMemAlloc(nExtra + sizeof(SGAMEDATA), s_szSGameData, -2, dwFlags | 8);
    if (pBlock) {
        pBlock->bucketLink.uNext = 0;
        pBlock->bucketLink.uPrev = 0;
        pBlock->listLink.uNext = 0;
        pBlock->listLink.uPrev = 0;
    }
    if (nLinkMode == 0) {
        return pBlock;
    }
    GameTableLink * pLink = pBlock ? GtElementLink(pList, (uintptr_t)pBlock) : &pList->head;
    GtUnlink(pLink);
    GameTableLink * pHead = &pList->head;
    if (nLinkMode != 1) {
        uintptr_t uFirst = pHead->uNext;
        pLink->uNext = uFirst;
        pLink->uPrev = GtLink(uFirst)->uPrev;
        GtLink(uFirst)->uPrev = (uintptr_t)pBlock;
        pHead->uNext = (uintptr_t)pLink;
        return pBlock;
    }
    pLink->uNext = (uintptr_t)pHead;
    pLink->uPrev = pList->head.uPrev;
    uintptr_t uPrev = pList->head.uPrev;
    intptr_t nBase = pList->nLinkOffset;
    if ((intptr_t)uPrev < 0) {
        GtLink(~uPrev)->uNext = (uintptr_t)pLink;
        pList->head.uPrev = (uintptr_t)pBlock;
        return pBlock;
    }
    if (nBase < 0) {
        nBase = (intptr_t)((uintptr_t)pHead - GtLink(pHead->uNext)->uPrev);
    }
    GtLink(uPrev + nBase)->uNext = (uintptr_t)pLink;
    pList->head.uPrev = (uintptr_t)pBlock;
    return pBlock;
}

// 1.00 D2Server: 1000b070
void __fastcall TSExplicitList_SGAMEDATA_UnlinkAll(GameTableListArray * pThis) {
    for (uint32_t i = 0; i < pThis->nCount; ++i) {
        TSList_SGAMEDATA_UnlinkNode(&pThis->pLists[i]);
    }
    if (pThis->pLists != nullptr) {
        SMemFree(pThis->pLists, s_szExplicitList, -2, 0);
    }
}

// 1.00 D2Server: 1000b0e0
void __fastcall TSList_SGAMEDATA_UnlinkNode_D(GameTableList * pList) {
    GtUnlinkAllElements(pList);
    GtUnlink(&pList->head);
}

// 1.00 D2Server: 1000b180
GameDataTable * __thiscall GameTable_DestructBody_ScalarDelete(GameDataTable * pThis, uint8_t nFlags) {
    GameTable_DestructBody(pThis);
    if ((nFlags & 1) && pThis) {
        SMemFree(pThis, "delete", -1, 0);
    }
    return pThis;
}

// As GameTable_Destruct, with each element deleted through SGAMEDATA's scalar
// deleting destructor.
// 1.00 D2Server: 1000b1c0
void __fastcall GameTable_DestructBody(GameDataTable * pThis) {
    pThis->pVtable = s_vtblHashObject;
    intptr_t nPtr;
    while ((nPtr = (intptr_t)pThis->chunks.head.uPrev) >= 1) {
        SGAMEDATA_Chunk_FreeAll((GameTableChunk *)nPtr);
        SMemFree((void *)nPtr, s_szHashObjectChunk, -2, 0);
    }
    while ((nPtr = (intptr_t)pThis->reuse.head.uPrev) > 0) {
        SGAMEDATA_ScalarDelete((SGAMEDATA *)nPtr, 0);
        SMemFree((void *)nPtr, s_szSGameData, -2, 0);
    }
    while ((intptr_t)pThis->chunks.head.uPrev > 0) {
        TSList_UnlinkNode_C(GtElementLink(&pThis->chunks, pThis->chunks.head.uPrev));
    }
    GtUnlinkVia(&pThis->chunks.head, TSListNode_GetOffset_B);
    while ((intptr_t)pThis->reuse.head.uPrev > 0) {
        TSList_UnlinkNode(GtElementLink(&pThis->reuse, pThis->reuse.head.uPrev));
    }
    GtUnlinkVia(&pThis->reuse.head, TSListNode_GetOffset);
    pThis->pVtable = s_vtblExplicitList;
    pThis->dw10 = 0;
    TSList_UnlinkAll(&pThis->elements);
    for (uint32_t i = 0; i < pThis->buckets.nCount; ++i) {
        intptr_t nSlot;
        while ((nSlot = (intptr_t)pThis->buckets.pLists[i].head.uPrev) >= 1) {
            TSList_UnlinkNode(TSListNode_GetNodePtr(&pThis->buckets.pLists[i], nSlot));
        }
    }
    TSExplicitList_SGAMEDATA_UnlinkAll(&pThis->buckets);
    TSList_UnlinkAll(&pThis->elements);
    TSList_UnlinkNode(&pThis->elements.head);
}

// The link whose next field points at this link.
// 1.00 D2Server: 1000b380
GameTableLink * __thiscall TSListNode_GetOffset(GameTableLink * pLink, intptr_t nOffset) {
    uintptr_t uPrev = pLink->uPrev;
    if ((intptr_t)uPrev < 0) {
        return GtLink(~uPrev);
    }
    if (nOffset < 0) {
        nOffset = (intptr_t)((uintptr_t)pLink - GtLink(pLink->uNext)->uPrev);
    }
    return GtLink(uPrev + nOffset);
}

// 1.00 D2Server: 1000b3c0
void __fastcall TSList_UnlinkNode(GameTableLink * pLink) {
    GtUnlink(pLink);
}

// 1.00 D2Server: 1000b410
GameTableLink * __thiscall TSListNode_GetNodePtr(GameTableList * pList, intptr_t nNode) {
    if (nNode != 0) {
        return GtElementLink(pList, (uintptr_t)nNode);
    }
    return &pList->head;
}

// 1.00 D2Server: 1000b440
void __fastcall TSList_UnlinkAll(GameTableList * pList) {
    GtUnlinkAllElements(pList);
}

// 1.00 D2Server: 1000b4b0
void __fastcall TSList_UnlinkNode_B(GameTableLink * pLink) {
    GtUnlink(pLink);
}

// 1.00 D2Server: 1000b500
GameTableList * __thiscall TSList_SGAMEDATA_ScalarDelete(GameTableList * pList, uint8_t nFlags) {
    TSList_SGAMEDATA_UnlinkNode(pList);
    if ((nFlags & 1) && pList) {
        SMemFree(pList, "delete", -1, 0);
    }
    return pList;
}

// 1.00 D2Server: 1000b540
void __fastcall SGAMEDATA_Chunk_FreeAll(GameTableChunk * pChunk) {
    GtUnlink(&pChunk->link);
    for (uint32_t i = 0; i < pChunk->nCount; ++i) {
        SGAMEDATA * pSlot = &pChunk->pElements[i];
        TSList_UnlinkNode(&pSlot->listLink);
        GtUnlinkVia(&pSlot->bucketLink, TSListNode_GetOffset);
    }
    if (pChunk->pElements != nullptr) {
        SMemFree(pChunk->pElements, s_szSGameData, -2, 0);
    }
}

// 1.00 D2Server: 1000b620
void __fastcall TSListHead_Init_Sentinel(GameTableList * pList) {
    pList->head.uNext = 0;
    pList->head.uPrev = 0;
    pList->head.uNext = (uintptr_t)&pList->head;
    pList->nLinkOffset = (intptr_t)(int32_t)0xDDDDDDDD;
    pList->head.uPrev = ~(uintptr_t)&pList->head;
}

// Empties the list and changes its link offset.
// 1.00 D2Server: 1000b650
void __thiscall TSExplicitList_Resize_DrainOld(GameTableList * pList, intptr_t nLinkOffset) {
    if (nLinkOffset == pList->nLinkOffset) {
        return;
    }
    GtUnlinkAllElements(pList);
    pList->head.uNext = (uintptr_t)&pList->head;
    pList->head.uPrev = ~(uintptr_t)&pList->head;
    pList->nLinkOffset = nLinkOffset;
}

// 1.00 D2Server: 1000b6e0
GameTableLink * __thiscall TSListNode_GetOffset_B(GameTableLink * pLink, intptr_t nOffset) {
    uintptr_t uPrev = pLink->uPrev;
    if ((intptr_t)uPrev < 0) {
        return GtLink(~uPrev);
    }
    if (nOffset < 0) {
        nOffset = (intptr_t)((uintptr_t)pLink - GtLink(pLink->uNext)->uPrev);
    }
    return GtLink(uPrev + nOffset);
}

// 1.00 D2Server: 1000b720
void __fastcall TSList_UnlinkNode_C(GameTableLink * pLink) {
    GtUnlink(pLink);
}

// 1.00 D2Server: 1000b770
intptr_t __fastcall TSList_OffsetAdd(intptr_t nOffset, intptr_t nBase) {
    return nOffset + nBase;
}

// 1.00 D2Server: 1000b790
void __fastcall TSListHead_Init(GameTableList * pList) {
    pList->head.uNext = 0;
    pList->head.uPrev = 0;
    pList->head.uNext = (uintptr_t)&pList->head;
    pList->nLinkOffset = 0;
    pList->head.uPrev = ~(uintptr_t)&pList->head;
}

// 1.00 D2Server: 1000b7c0
void __fastcall TSListNode_Zero(GameTableListArray * pArray) {
    pArray->nCapacity = 0;
    pArray->nCount = 0;
    pArray->pLists = nullptr;
    pArray->nGrowth = 0;
}

// 1.00 D2Server: 1000b7e0
void __fastcall TSListHead_Init_B(GameTableList * pList) {
    pList->head.uNext = 0;
    pList->head.uPrev = 0;
    pList->head.uNext = (uintptr_t)&pList->head;
    pList->nLinkOffset = 0;
    pList->head.uPrev = ~(uintptr_t)&pList->head;
}

// 1.00 D2Server: 1000b810
void __fastcall TSListHead_InitEmpty(GameTableList * pList) {
    pList->head.uNext = (uintptr_t)&pList->head;
    pList->head.uPrev = ~(uintptr_t)&pList->head;
}

// Grows or shrinks an array of lists to nCount, the capacity in steps of the growth
// value (the largest power of two not above the count, until it is set; 21 past 20).
// 1.00 D2Server: 1000b830
void __thiscall TSExplicitList_Resize(GameTableListArray * pArray, uint32_t nCount) {
    if (pArray->nCount < nCount) {
        if (pArray->nCapacity < nCount) {
            uint32_t nStep = pArray->nGrowth;
            if (nStep == 0) {
                if (nCount < 0x15) {
                    nStep = nCount;
                    for (uint32_t nBits = (nCount - 1) & nCount; nBits != 0; nBits = (nBits - 1) & nBits) {
                        nStep = nBits;
                    }
                    if (nStep == 0) {
                        nStep = 1;
                    }
                } else {
                    pArray->nGrowth = 0x15;
                    nStep = 0x15;
                }
            }
            uint32_t nCapacity = nCount;
            if (nCount % nStep != 0) {
                nCapacity = (nStep - nCount % nStep) + nCount;
            }
            GameTableList * pOld = pArray->pLists;
            if (nCapacity < pArray->nCount) {
                GameTableList * pList = pOld + nCapacity;
                for (uint32_t i = nCapacity; i < pArray->nCount; ++i, ++pList) {
                    TSList_SGAMEDATA_ScalarDelete(pList, 0);
                }
            }
            pArray->nCapacity = nCapacity;
            pArray->pLists = (GameTableList *)SMemReAlloc(pOld, nCapacity * sizeof(GameTableList), s_szExplicitList, -2, 0x10);
            if (pArray->pLists == nullptr) {
                pArray->pLists = (GameTableList *)SMemAlloc(nCapacity * sizeof(GameTableList), s_szExplicitList, -2, 0);
                if (pOld) {
                    uint32_t nMove = pArray->nCount > nCapacity ? nCapacity : pArray->nCount;
                    GameTableList * pList = pOld;
                    for (uint32_t i = 0; nMove != 0; --nMove, ++i, ++pList) {
                        TSListHead_InitFrom(&pArray->pLists[i], pList);
                        TSList_SGAMEDATA_ScalarDelete(pList, 0);
                    }
                    SMemFree(pOld, s_szExplicitList, -2, 0);
                }
            }
        }
        if (pArray->nCount < nCount) {
            for (uint32_t i = pArray->nCount; i < nCount; ++i) {
                TSListHead_Init_Sentinel_B(&pArray->pLists[i]);
            }
            pArray->nCount = nCount;
            return;
        }
    } else if (nCount < pArray->nCount) {
        for (uint32_t i = nCount; i < pArray->nCount; ++i) {
            TSList_SGAMEDATA_UnlinkNode(&pArray->pLists[i]);
        }
    }
    pArray->nCount = nCount;
}

// 1.00 D2Server: 1000ba10
void __fastcall TSListHead_InitEmpty_B(GameTableList * pList) {
    pList->head.uNext = (uintptr_t)&pList->head;
    pList->head.uPrev = ~(uintptr_t)&pList->head;
}

// 1.00 D2Server: 1000ba30
void __fastcall TSListHead_Init_Sentinel_B(GameTableList * pList) {
    if (pList) {
        pList->head.uNext = 0;
        pList->head.uPrev = 0;
        pList->head.uNext = (uintptr_t)&pList->head;
        pList->nLinkOffset = (intptr_t)(int32_t)0xDDDDDDDD;
        pList->head.uPrev = ~(uintptr_t)&pList->head;
    }
}

// A head with the source list's link offset.
// 1.00 D2Server: 1000ba70
void __fastcall TSListHead_InitFrom(GameTableList * pList, GameTableList * pSource) {
    if (pList) {
        pList->head.uNext = 0;
        pList->head.uPrev = 0;
        intptr_t nOffset = pSource->nLinkOffset;
        pList->head.uNext = (uintptr_t)&pList->head;
        pList->nLinkOffset = nOffset;
        pList->head.uPrev = ~(uintptr_t)&pList->head;
    }
}

} // namespace D2Server::GameTable
