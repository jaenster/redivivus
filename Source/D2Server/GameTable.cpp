// D2Server: the member functions of Storm's TSHashTable<SGAMEDATA,HASHKEY_NONE> and
// of the lists inside it, as the 1.00 D2Server.dll instantiates them for the
// game-data table (ToMCP.cpp's g_gameDataParam1). No __FILE__ string survives; the
// name is the Ghidra program's. Several functions are the same template member
// emitted more than once; each copy is kept, as the binary has it.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.
//
// Layouts, in dwords:
//   list    [0] offset of the link inside an element, [1] head next, [2] head prev
//   link    [0] next, [1] prev: negative = complemented pointer to the previous
//           link, otherwise an element pointer the list's offset is added to
//   array   [0] capacity, [1] count, [2] the lists (12 bytes each), [3] growth step
//   table   [0] vtable, [1..3] element list, [4] 0, [5..9] bucket array (+[9] mask),
//           [0xa..0xd] element list, [0xe..0x11] chunk list, [0x12] hash seed,
//           [0x13] 0, [0x14..0x19] CRITICAL_SECTION
//   element SGAMEDATA: [1..2] one link, [3..4] the other
//   chunk   [1] count, [2] elements (0x3060 bytes each), [4..5] link

#include "Source/D2Server/GameTable.h"
#include "Source/D2Server/Core/ToMCP.h"
#include "Source/D2Server/D2ServerImports.h"

namespace D2Server::GameTable {

using namespace D2Server::Imports;

#define GT_PTR(x) ((uint32_t *)(uintptr_t)(x))
#define GT_U32(p) ((uint32_t)(uintptr_t)(p))

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

typedef void (__thiscall * ElementDeleteFn)(uint32_t * pThis, uint32_t * pElement);

// The link unlink every member inlines: point the previous link past this one.
static inline void GtUnlink(uint32_t * pLink) {
    uint32_t dwNext = pLink[0];
    if (dwNext == 0) {
        return;
    }
    uint32_t dwPrev = pLink[1];
    uint32_t * pPrevNext;
    if ((int32_t)dwPrev < 0) {
        pPrevNext = GT_PTR(~dwPrev);
    } else {
        pPrevNext = GT_PTR(GT_U32(pLink) + (dwPrev - *GT_PTR(dwNext + 4)));
    }
    *pPrevNext = dwNext;
    *GT_PTR(pLink[0] + 4) = pLink[1];
    pLink[0] = 0;
    pLink[1] = 0;
}

// Unlinks every element of a list, last first.
static inline void GtUnlinkAllElements(uint32_t * pList) {
    while ((int32_t)pList[2] > 0) {
        GtUnlink(GT_PTR(pList[0] + pList[2]));
    }
}

// The out-of-line form of the unlink, with the previous link found by
// TSListNode_GetOffset.
static inline void GtUnlinkVia(uint32_t * pLink, uint32_t * (__thiscall * pfnGetOffset)(uint32_t *, int)) {
    if (pLink[0] != 0) {
        uint32_t * pPrevNext = pfnGetOffset(pLink, -1);
        *pPrevNext = pLink[0];
        *GT_PTR(pLink[0] + 4) = pLink[1];
        pLink[0] = 0;
        pLink[1] = 0;
    }
}

// 1.00 D2Server: 1000a240
uint32_t * __fastcall GameTable_Construct(uint32_t * pThis) {
    TSListHead_Init_Sentinel(pThis + 1);
    TSListNode_Zero(pThis + 5);
    pThis[0] = GT_U32(s_vtblExplicitList);
    pThis[4] = 0;
    TSExplicitList_Resize_DrainOld(pThis + 1, 0x0C);
    pThis[9] = 3;
    TSExplicitList_Resize(pThis + 5, 4);
    for (uint32_t i = 0; i <= pThis[9]; ++i) {
        TSExplicitList_Resize_DrainOld(GT_PTR(pThis[7] + i * 0x0C), 4);
    }
    TSListHead_Init(pThis + 0x0A);
    pThis[0x0A] = 0xDDDDDDDD;
    TSListHead_InitEmpty(pThis + 0x0A);
    TSListHead_Init_B(pThis + 0x0E);
    pThis[0x0E] = 0x10;
    TSListHead_InitEmpty_B(pThis + 0x0E);
    pThis[0] = GT_U32(s_vtblHashObject);
    pThis[0x0D] = 0x10;
    TSExplicitList_Resize_DrainOld(pThis + 0x0A, 4);
    pThis[0] = GT_U32(s_vtblHashTable);
    pThis[0x12] = ~GT_U32(pThis) & 0x0FFFFFFF;
    pThis[0x13] = 0;
    InitializeCriticalSection((LPCRITICAL_SECTION)(pThis + 0x14));
    pThis[0] = GT_U32(s_vtblGameTable);
    return pThis;
}

// 1.00 D2Server: 1000a3c0
void __thiscall SGAMEDATA_FreeElement_A(uint32_t * pThis, uint32_t * pElement) {
    SGAMEDATA_UnlinkFromBothLists(pElement);
    SMemFree(pElement, s_szSGameData, -2, 0);
}

// 1.00 D2Server: 1000a3f0
uint32_t * __thiscall SGAMEDATA_AllocInChunk_A(uint32_t * pThis, uint32_t * pList, int nExtra, uint32_t dwFlags) {
    return SGAMEDATA_Chunk_AllocElement(pList, 1, nExtra, dwFlags);
}

// 1.00 D2Server: 1000a420
void __fastcall TSHashObject_SGAMEDATA_Destructor(uint32_t * pThis) {
    pThis[4] = 0;
    while ((int32_t)pThis[3] > 0) {
        TSList_UnlinkNode(GT_PTR(pThis[1] + pThis[3]));
    }
    for (uint32_t i = 0; i < pThis[6]; ++i) {
        int32_t nElement;
        while ((nElement = *(int32_t *)GT_PTR(pThis[7] + i * 0x0C + 8)) > 0) {
            ((ElementDeleteFn)((void **)(uintptr_t)pThis[0])[0])(pThis, GT_PTR(nElement));
        }
    }
    int32_t nPtr;
    while ((nPtr = (int32_t)pThis[0x10]) > 0) {
        SGAMEDATA_Chunk_FreeAll(GT_PTR(nPtr));
        SMemFree(GT_PTR(nPtr), s_szHashObjectChunk, -2, 0);
    }
    while ((nPtr = (int32_t)pThis[0x0C]) > 0) {
        SGAMEDATA_UnlinkFromBothLists(GT_PTR(nPtr));
        SMemFree(GT_PTR(nPtr), s_szSGameData, -2, 0);
    }
}

// 1.00 D2Server: 1000a4e0
void __thiscall SGAMEDATA_FreeElement_B(uint32_t * pThis, uint32_t * pElement) {
    SGAMEDATA_UnlinkFromBothLists(pElement);
    SMemFree(pElement, s_szSGameData, -2, 0);
}

// 1.00 D2Server: 1000a510
uint32_t * __thiscall SGAMEDATA_AllocInChunk_B(uint32_t * pThis, uint32_t * pList, int nExtra, uint32_t dwFlags) {
    return SGAMEDATA_Chunk_AllocElement(pList, 1, nExtra, dwFlags);
}

// 1.00 D2Server: 1000a540
void __fastcall GameTable_ClearAllChunks(uint32_t * pThis) {
    pThis[0] = GT_U32(s_vtblExplicitList);
    pThis[4] = 0;
    while ((int32_t)pThis[3] >= 1) {
        TSList_UnlinkNode(TSListNode_GetNodePtr(pThis + 1, (int)pThis[3]));
    }
    for (uint32_t i = 0; i < pThis[6]; ++i) {
        uint32_t * pBucket = GT_PTR(pThis[7] + i * 0x0C);
        while ((int32_t)pBucket[2] >= 1) {
            GtUnlink(GT_PTR(pBucket[0] + pBucket[2]));
        }
    }
    for (uint32_t i = 0; i < pThis[6]; ++i) {
        TSList_SGAMEDATA_UnlinkNode_C(GT_PTR(pThis[7] + i * 0x0C));
    }
    if (pThis[7] != 0) {
        SMemFree(GT_PTR(pThis[7]), s_szExplicitList, -2, 0);
    }
    while ((int32_t)pThis[3] > 0) {
        GtUnlinkVia(GT_PTR(pThis[1] + pThis[3]), TSListNode_GetOffset);
    }
    GtUnlink(pThis + 2);
}

// 1.00 D2Server: 1000a720
void __fastcall TSExplicitList_SGAMEDATA_Destructor(uint32_t * pThis) {
    pThis[4] = 0;
    while ((int32_t)pThis[3] > 0) {
        TSList_UnlinkNode(GT_PTR(pThis[1] + pThis[3]));
    }
    for (uint32_t i = 0; i < pThis[6]; ++i) {
        uint32_t * pBucket = GT_PTR(pThis[7] + i * 0x0C);
        while ((int32_t)pBucket[2] >= 1) {
            GtUnlink(GT_PTR(pBucket[0] + pBucket[2]));
        }
    }
    pThis[4] = 0;
    pThis[9] = 0;
    for (uint32_t i = 0; i < pThis[6]; ++i) {
        TSList_SGAMEDATA_ScalarDelete(GT_PTR(pThis[7] + i * 0x0C), 0);
    }
    if (pThis[7] != 0) {
        SMemFree(GT_PTR(pThis[7]), s_szExplicitList, -2, 0);
    }
    pThis[5] = 0;
    pThis[6] = 0;
    pThis[7] = 0;
}

// 1.00 D2Server: 1000a830
uint32_t * __thiscall ToMCP_ScalarDelete(uint32_t * pThis, uint8_t nFlags) {
    Core::ToMCP::ToMCP_Destructor(pThis);
    if ((nFlags & 1) && pThis) {
        SMemFree(pThis, "delete", -1, 0);
    }
    return pThis;
}

// 1.00 D2Server: 1000a870
uint32_t * __thiscall SGAMEDATA_ScalarDelete(uint32_t * pThis, uint8_t nFlags) {
    SGAMEDATA_UnlinkFromBothLists(pThis);
    if ((nFlags & 1) && pThis) {
        SMemFree(pThis, "delete", -1, 0);
    }
    return pThis;
}

// 1.00 D2Server: 1000a8b0
uint32_t * __thiscall GameTable_ScalarDelete(uint32_t * pThis, uint8_t nFlags) {
    GameTable_Destruct(pThis);
    if ((nFlags & 1) && pThis) {
        SMemFree(pThis, "delete", -1, 0);
    }
    return pThis;
}

// The table's destructor below its own class: the chunks and elements, then the
// base lists.
// 1.00 D2Server: 1000a8f0
void __fastcall GameTable_Destruct(uint32_t * pThis) {
    pThis[0] = GT_U32(s_vtblHashObject);
    int32_t nPtr;
    while ((nPtr = (int32_t)pThis[0x10]) > 0) {
        SGAMEDATA_Chunk_FreeAll(GT_PTR(nPtr));
        SMemFree(GT_PTR(nPtr), s_szHashObjectChunk, -2, 0);
    }
    while ((nPtr = (int32_t)pThis[0x0C]) > 0) {
        SGAMEDATA_UnlinkFromBothLists_B(GT_PTR(nPtr));
        SMemFree(GT_PTR(nPtr), s_szSGameData, -2, 0);
    }
    while ((int32_t)pThis[0x10] > 0) {
        TSList_UnlinkNode_C(GT_PTR(pThis[0x0E] + pThis[0x10]));
    }
    GtUnlinkVia(pThis + 0x0F, TSListNode_GetOffset_B);
    while ((int32_t)pThis[0x0C] > 0) {
        TSList_UnlinkNode(GT_PTR(pThis[0x0A] + pThis[0x0C]));
    }
    GtUnlinkVia(pThis + 0x0B, TSListNode_GetOffset);
    pThis[0] = GT_U32(s_vtblExplicitList);
    pThis[4] = 0;
    TSList_UnlinkAll(pThis + 1);
    for (uint32_t i = 0; i < pThis[6]; ++i) {
        int32_t nSlot;
        while ((nSlot = *(int32_t *)GT_PTR(pThis[7] + i * 0x0C + 8)) >= 1) {
            TSList_UnlinkNode(TSListNode_GetNodePtr(GT_PTR(pThis[7] + i * 0x0C), nSlot));
        }
    }
    TSExplicitList_SGAMEDATA_UnlinkAll(pThis + 5);
    TSList_UnlinkAll(pThis + 1);
    TSList_UnlinkNode(pThis + 2);
}

// 1.00 D2Server: 1000aaf0
void __fastcall TSList_SGAMEDATA_UnlinkNode(uint32_t * pList) {
    GtUnlinkAllElements(pList);
    GtUnlink(pList + 1);
}

// 1.00 D2Server: 1000ab90
void __fastcall TSList_SGAMEDATA_UnlinkNode_B(uint32_t * pList) {
    GtUnlinkAllElements(pList);
    GtUnlink(pList + 1);
}

// 1.00 D2Server: 1000ac30
uint32_t * __thiscall GameTable_ClearChunks_ScalarDelete(uint32_t * pThis, uint8_t nFlags) {
    GameTable_ClearAllChunks(pThis);
    if ((nFlags & 1) && pThis) {
        SMemFree(pThis, "delete", -1, 0);
    }
    return pThis;
}

// 1.00 D2Server: 1000ac70
void __fastcall TSExplicitList_SGAMEDATA_FreeAllBuckets(uint32_t * pThis) {
    for (uint32_t i = 0; i < pThis[1]; ++i) {
        uint32_t * pBucket = GT_PTR(pThis[2] + i * 0x0C);
        while ((int32_t)pBucket[2] > 0) {
            TSList_UnlinkNode(GT_PTR(TSList_OffsetAdd((int)pBucket[2], (int)pBucket[0])));
        }
        TSList_UnlinkNode(pBucket + 1);
    }
    if (pThis[2] != 0) {
        SMemFree(GT_PTR(pThis[2]), s_szExplicitList, -2, 0);
    }
}

// 1.00 D2Server: 1000ad50
void __fastcall SGAMEDATA_UnlinkFromBothLists(uint32_t * pElement) {
    GtUnlink(pElement + 3);
    GtUnlink(pElement + 1);
}

// 1.00 D2Server: 1000ade0
void __fastcall SGAMEDATA_UnlinkFromBothLists_B(uint32_t * pElement) {
    GtUnlink(pElement + 3);
    GtUnlink(pElement + 1);
}

// 1.00 D2Server: 1000ae70
void __fastcall TSList_UnlinkSingleNode(uint32_t * pLink) {
    GtUnlink(pLink);
}

// 1.00 D2Server: 1000aec0
void __fastcall TSList_SGAMEDATA_UnlinkNode_C(uint32_t * pList) {
    GtUnlinkAllElements(pList);
    GtUnlink(pList + 1);
}

// A new element, its extra bytes after the 0x3060 of SGAMEDATA, linked at the tail
// of the list (nLinkMode 1) or at its head (any other nonzero mode).
// 1.00 D2Server: 1000af60
uint32_t * __thiscall SGAMEDATA_Chunk_AllocElement(uint32_t * pList, int nLinkMode, int nExtra, uint32_t dwFlags) {
    uint32_t * pBlock = (uint32_t *)SMemAlloc(nExtra + 0x3060, s_szSGameData, -2, dwFlags | 8);
    if (pBlock) {
        pBlock[1] = 0;
        pBlock[2] = 0;
        pBlock[3] = 0;
        pBlock[4] = 0;
    }
    if (nLinkMode == 0) {
        return pBlock;
    }
    uint32_t * pLink = pBlock ? GT_PTR(pList[0] + GT_U32(pBlock)) : pList + 1;
    GtUnlink(pLink);
    uint32_t * pHead = pList + 1;
    if (nLinkMode != 1) {
        uint32_t dwFirst = *pHead;
        pLink[0] = dwFirst;
        pLink[1] = *GT_PTR(dwFirst + 4);
        *GT_PTR(dwFirst + 4) = GT_U32(pBlock);
        *pHead = GT_U32(pLink);
        return pBlock;
    }
    pLink[0] = GT_U32(pHead);
    pLink[1] = pList[2];
    uint32_t dwPrev = pList[2];
    int32_t nBase = (int32_t)pList[0];
    if ((int32_t)dwPrev < 0) {
        *GT_PTR(~dwPrev) = GT_U32(pLink);
        pList[2] = GT_U32(pBlock);
        return pBlock;
    }
    if (nBase < 0) {
        nBase = (int32_t)(GT_U32(pHead) - *GT_PTR(*pHead + 4));
    }
    *GT_PTR(dwPrev + nBase) = GT_U32(pLink);
    pList[2] = GT_U32(pBlock);
    return pBlock;
}

// 1.00 D2Server: 1000b070
void __fastcall TSExplicitList_SGAMEDATA_UnlinkAll(uint32_t * pThis) {
    for (uint32_t i = 0; i < pThis[1]; ++i) {
        TSList_SGAMEDATA_UnlinkNode(GT_PTR(pThis[2] + i * 0x0C));
    }
    if (pThis[2] != 0) {
        SMemFree(GT_PTR(pThis[2]), s_szExplicitList, -2, 0);
    }
}

// 1.00 D2Server: 1000b0e0
void __fastcall TSList_SGAMEDATA_UnlinkNode_D(uint32_t * pList) {
    GtUnlinkAllElements(pList);
    GtUnlink(pList + 1);
}

// 1.00 D2Server: 1000b180
uint32_t * __thiscall GameTable_DestructBody_ScalarDelete(uint32_t * pThis, uint8_t nFlags) {
    GameTable_DestructBody(pThis);
    if ((nFlags & 1) && pThis) {
        SMemFree(pThis, "delete", -1, 0);
    }
    return pThis;
}

// As GameTable_Destruct, with each element deleted through SGAMEDATA's scalar
// deleting destructor.
// 1.00 D2Server: 1000b1c0
void __fastcall GameTable_DestructBody(uint32_t * pThis) {
    pThis[0] = GT_U32(s_vtblHashObject);
    int32_t nPtr;
    while ((nPtr = (int32_t)pThis[0x10]) >= 1) {
        SGAMEDATA_Chunk_FreeAll(GT_PTR(nPtr));
        SMemFree(GT_PTR(nPtr), s_szHashObjectChunk, -2, 0);
    }
    while ((nPtr = (int32_t)pThis[0x0C]) > 0) {
        SGAMEDATA_ScalarDelete(GT_PTR(nPtr), 0);
        SMemFree(GT_PTR(nPtr), s_szSGameData, -2, 0);
    }
    while ((int32_t)pThis[0x10] > 0) {
        TSList_UnlinkNode_C(GT_PTR(pThis[0x0E] + pThis[0x10]));
    }
    GtUnlinkVia(pThis + 0x0F, TSListNode_GetOffset_B);
    while ((int32_t)pThis[0x0C] > 0) {
        TSList_UnlinkNode(GT_PTR(pThis[0x0A] + pThis[0x0C]));
    }
    GtUnlinkVia(pThis + 0x0B, TSListNode_GetOffset);
    pThis[0] = GT_U32(s_vtblExplicitList);
    pThis[4] = 0;
    TSList_UnlinkAll(pThis + 1);
    for (uint32_t i = 0; i < pThis[6]; ++i) {
        int32_t nSlot;
        while ((nSlot = *(int32_t *)GT_PTR(pThis[7] + i * 0x0C + 8)) >= 1) {
            TSList_UnlinkNode(TSListNode_GetNodePtr(GT_PTR(pThis[7] + i * 0x0C), nSlot));
        }
    }
    TSExplicitList_SGAMEDATA_UnlinkAll(pThis + 5);
    TSList_UnlinkAll(pThis + 1);
    TSList_UnlinkNode(pThis + 2);
}

// The link whose next field points at this link.
// 1.00 D2Server: 1000b380
uint32_t * __thiscall TSListNode_GetOffset(uint32_t * pLink, int nOffset) {
    uint32_t dwPrev = pLink[1];
    if ((int32_t)dwPrev < 0) {
        return GT_PTR(~dwPrev);
    }
    if (nOffset < 0) {
        nOffset = (int)(GT_U32(pLink) - *GT_PTR(pLink[0] + 4));
    }
    return GT_PTR(dwPrev + nOffset);
}

// 1.00 D2Server: 1000b3c0
void __fastcall TSList_UnlinkNode(uint32_t * pLink) {
    GtUnlink(pLink);
}

// 1.00 D2Server: 1000b410
uint32_t * __thiscall TSListNode_GetNodePtr(uint32_t * pList, int nNode) {
    if (nNode != 0) {
        return GT_PTR(pList[0] + nNode);
    }
    return pList + 1;
}

// 1.00 D2Server: 1000b440
void __fastcall TSList_UnlinkAll(uint32_t * pList) {
    GtUnlinkAllElements(pList);
}

// 1.00 D2Server: 1000b4b0
void __fastcall TSList_UnlinkNode_B(uint32_t * pLink) {
    GtUnlink(pLink);
}

// 1.00 D2Server: 1000b500
uint32_t * __thiscall TSList_SGAMEDATA_ScalarDelete(uint32_t * pList, uint8_t nFlags) {
    TSList_SGAMEDATA_UnlinkNode(pList);
    if ((nFlags & 1) && pList) {
        SMemFree(pList, "delete", -1, 0);
    }
    return pList;
}

// 1.00 D2Server: 1000b540
void __fastcall SGAMEDATA_Chunk_FreeAll(uint32_t * pChunk) {
    GtUnlink(pChunk + 4);
    for (uint32_t i = 0; i < pChunk[1]; ++i) {
        uint32_t * pSlot = GT_PTR(pChunk[2] + i * 0x3060);
        TSList_UnlinkNode(pSlot + 3);
        GtUnlinkVia(pSlot + 1, TSListNode_GetOffset);
    }
    if (pChunk[2] != 0) {
        SMemFree(GT_PTR(pChunk[2]), s_szSGameData, -2, 0);
    }
}

// 1.00 D2Server: 1000b620
void __fastcall TSListHead_Init_Sentinel(uint32_t * pList) {
    pList[1] = 0;
    pList[2] = 0;
    pList[1] = GT_U32(pList + 1);
    pList[0] = 0xDDDDDDDD;
    pList[2] = ~GT_U32(pList + 1);
}

// Empties the list and changes its link offset.
// 1.00 D2Server: 1000b650
void __thiscall TSExplicitList_Resize_DrainOld(uint32_t * pList, int nLinkOffset) {
    if (nLinkOffset == (int)pList[0]) {
        return;
    }
    GtUnlinkAllElements(pList);
    pList[1] = GT_U32(pList + 1);
    pList[2] = ~GT_U32(pList + 1);
    pList[0] = (uint32_t)nLinkOffset;
}

// 1.00 D2Server: 1000b6e0
uint32_t * __thiscall TSListNode_GetOffset_B(uint32_t * pLink, int nOffset) {
    uint32_t dwPrev = pLink[1];
    if ((int32_t)dwPrev < 0) {
        return GT_PTR(~dwPrev);
    }
    if (nOffset < 0) {
        nOffset = (int)(GT_U32(pLink) - *GT_PTR(pLink[0] + 4));
    }
    return GT_PTR(dwPrev + nOffset);
}

// 1.00 D2Server: 1000b720
void __fastcall TSList_UnlinkNode_C(uint32_t * pLink) {
    GtUnlink(pLink);
}

// 1.00 D2Server: 1000b770
int __fastcall TSList_OffsetAdd(int nOffset, int nBase) {
    return nOffset + nBase;
}

// 1.00 D2Server: 1000b790
void __fastcall TSListHead_Init(uint32_t * pList) {
    pList[1] = 0;
    pList[2] = 0;
    pList[1] = GT_U32(pList + 1);
    pList[0] = 0;
    pList[2] = ~GT_U32(pList + 1);
}

// 1.00 D2Server: 1000b7c0
void __fastcall TSListNode_Zero(uint32_t * pArray) {
    pArray[0] = 0;
    pArray[1] = 0;
    pArray[2] = 0;
    pArray[3] = 0;
}

// 1.00 D2Server: 1000b7e0
void __fastcall TSListHead_Init_B(uint32_t * pList) {
    pList[1] = 0;
    pList[2] = 0;
    pList[1] = GT_U32(pList + 1);
    pList[0] = 0;
    pList[2] = ~GT_U32(pList + 1);
}

// 1.00 D2Server: 1000b810
void __fastcall TSListHead_InitEmpty(uint32_t * pList) {
    pList[1] = GT_U32(pList + 1);
    pList[2] = ~GT_U32(pList + 1);
}

// Grows or shrinks an array of lists to nCount, the capacity in steps of the growth
// value (the largest power of two not above the count, until it is set; 21 past 20).
// 1.00 D2Server: 1000b830
void __thiscall TSExplicitList_Resize(uint32_t * pArray, uint32_t nCount) {
    if (pArray[1] < nCount) {
        if (pArray[0] < nCount) {
            uint32_t nStep = pArray[3];
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
                    pArray[3] = 0x15;
                    nStep = 0x15;
                }
            }
            uint32_t nCapacity = nCount;
            if (nCount % nStep != 0) {
                nCapacity = (nStep - nCount % nStep) + nCount;
            }
            uint32_t * pOld = GT_PTR(pArray[2]);
            if (nCapacity < pArray[1]) {
                uint32_t * pList = pOld + nCapacity * 3;
                for (uint32_t i = nCapacity; i < pArray[1]; ++i, pList += 3) {
                    TSList_SGAMEDATA_ScalarDelete(pList, 0);
                }
            }
            pArray[0] = nCapacity;
            pArray[2] = GT_U32(SMemReAlloc(pOld, nCapacity * 0x0C, s_szExplicitList, -2, 0x10));
            if (pArray[2] == 0) {
                pArray[2] = GT_U32(SMemAlloc(nCapacity * 0x0C, s_szExplicitList, -2, 0));
                if (pOld) {
                    uint32_t nMove = pArray[1] > nCapacity ? nCapacity : pArray[1];
                    uint32_t * pList = pOld;
                    for (uint32_t nOffset = 0; nMove != 0; --nMove, nOffset += 0x0C, pList += 3) {
                        TSListHead_InitFrom(GT_PTR(pArray[2] + nOffset), pList);
                        TSList_SGAMEDATA_ScalarDelete(pList, 0);
                    }
                    SMemFree(pOld, s_szExplicitList, -2, 0);
                }
            }
        }
        if (pArray[1] < nCount) {
            for (uint32_t i = pArray[1]; i < nCount; ++i) {
                TSListHead_Init_Sentinel_B(GT_PTR(pArray[2] + i * 0x0C));
            }
            pArray[1] = nCount;
            return;
        }
    } else if (nCount < pArray[1]) {
        for (uint32_t i = nCount; i < pArray[1]; ++i) {
            TSList_SGAMEDATA_UnlinkNode(GT_PTR(pArray[2] + i * 0x0C));
        }
    }
    pArray[1] = nCount;
}

// 1.00 D2Server: 1000ba10
void __fastcall TSListHead_InitEmpty_B(uint32_t * pList) {
    pList[1] = GT_U32(pList + 1);
    pList[2] = ~GT_U32(pList + 1);
}

// 1.00 D2Server: 1000ba30
void __fastcall TSListHead_Init_Sentinel_B(uint32_t * pList) {
    if (pList) {
        pList[1] = 0;
        pList[2] = 0;
        pList[1] = GT_U32(pList + 1);
        pList[0] = 0xDDDDDDDD;
        pList[2] = ~GT_U32(pList + 1);
    }
}

// A head with the source list's link offset.
// 1.00 D2Server: 1000ba70
void __fastcall TSListHead_InitFrom(uint32_t * pList, uint32_t * pSource) {
    if (pList) {
        pList[1] = 0;
        pList[2] = 0;
        uint32_t dwOffset = pSource[0];
        pList[1] = GT_U32(pList + 1);
        pList[0] = dwOffset;
        pList[2] = ~GT_U32(pList + 1);
    }
}

} // namespace D2Server::GameTable
