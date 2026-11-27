// StormMac Storm Mac/SOURCE/SEVT.CPP: Storm's event dispatcher (handler tables per
// event type, push/pop of the handler state, the break-chain request), 1.14d Intel Mac
// 000110bc..00011f70.
//
// The binary's __FILE__ is ".../StormMac/Storm Mac/SOURCE/SEVT.CPP" (lines 0x43..0x18f).
// The file starts at SEvtBreakHandlerChain, after SERR.CPP's static initialiser
// (00011040), and ends with its own at 00011f70; SFILE.CPP follows.
#include "../Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <string.h>

void* SMemAlloc(uint32_t nBytes, const char* szFile, int nLine, uint32_t dwFlags);
int SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags);
void SErrSetLastError(DWORD dwErrCode);
LONG InterlockedIncrement(LPLONG pnValue);
LONG InterlockedDecrement(LPLONG pnValue);
// Out of line in SFILE.CPP's neighbourhood.
void CRITSECT_Init(pthread_mutex_t* pMutex);
void CRITSECT_Destroy(void* pMutex);

extern "C" int __cxa_atexit(void (*pfnDestructor)(void*), void* pObject, void* pDso);
extern "C" void* __dso_handle;

#define SEVT_FILE "/Users/bclemetson/dev/diablo2/Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/SOURCE/SEVT.CPP"

typedef void (*pfnSEvtHandler)(void* pData);

// ---- the intrusive list the break commands are kept on ----

struct TSLink {
    TSLink* pPrev;
    uintptr_t nNext;
};

struct TSList {
    int nLinkOffset;
    TSLink tTerminator;
};

static inline bool IsListEnd(uintptr_t nNext) {
    return nNext == 0 || (nNext & 1) != 0;
}

// 1.14d mac: 00011e76
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

// 1.14d mac: 00011e36
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

// 1.14d mac: 00011eac
// TSList::LinkNode: after (1) or before (2) a node, or the list's end.
static void TSLIST_LinkNode(TSList* pList, TSLink* pNode, int nLinkType, TSLink* pRelativeLink) {
    TSLink* pLink = pNode;
    if (pNode == NULL) {
        pLink = &pList->tTerminator;
    }
    if (pLink->pPrev != NULL) {
        TSLINK_Unlink(pLink);
    }
    TSLink* pRelative = &pList->tTerminator;
    if (pRelativeLink != NULL) {
        pRelative = pRelativeLink;
    }
    if (nLinkType == 2) {
        TSLink* pPrev = pRelative->pPrev;
        pLink->pPrev = pPrev;
        pLink->nNext = pPrev->nNext;
        pPrev->nNext = (uintptr_t)pNode;
        pRelative->pPrev = pLink;
    } else if (nLinkType == 1) {
        pLink->pPrev = pRelative;
        pLink->nNext = pRelative->nNext;
        TSLink* pNextLink = TSLINK_NextLink(pRelative, pList->nLinkOffset);
        pNextLink->pPrev = pLink;
        pRelative->nNext = (uintptr_t)pNode;
    }
}

// 1.14d mac: 00011f20
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

// 1.14d mac: 00011d2e
// TSList::~TSList
static void TSLIST_Destructor(void* pList) {
    TSLIST_UnlinkAll((TSList*)pList);
    TSLINK_Unlink(&((TSList*)pList)->tTerminator);
}

// 1.14d mac: 00011d50
// TSList::NewNode for a node of 0xc bytes plus nExtraBytes.
static TSLink* TSLIST_NewNode(TSList* pList, int nLinkType, int nExtraBytes, uint32_t dwAllocFlags) {
    TSLink* pAlloc = (TSLink*)SMemAlloc(nExtraBytes + 0xc, "", -2, dwAllocFlags | 8);
    TSLink* pNode = NULL;
    if (pAlloc != NULL) {
        pAlloc->pPrev = NULL;
        pAlloc->nNext = 0;
        pNode = pAlloc;
    }
    if (nLinkType != 0) {
        TSLIST_LinkNode(pList, pNode, nLinkType, NULL);
    }
    return pNode;
}

// 1.14d mac: 00011dc6
// Unlinks and frees a node, returning the node after it (or null).
static TSLink* LINKEDLIST_UnlinkFreeAndGetCount(TSList* pList, TSLink* pNode) {
    TSLink* pLinkSlot = &pList->tTerminator;
    if (pNode != NULL) {
        pLinkSlot = pNode;
    }
    uintptr_t nNext = pLinkSlot->nNext;
    TSLINK_Unlink(pNode);
    TSLINK_Unlink(pNode);
    TSLink* pResult = NULL;
    if (nNext != 0 && (nNext & 1) == 0) {
        pResult = (TSLink*)nNext;
    }
    SMemFree(pNode, "", -2, 0);
    return pResult;
}

// ---- the event tables ----

// One handler: the key it answers, its registration order, the callback.
struct SEvtHandler {
    uint32_t dwKey;
    uint32_t nPriority;
    pfnSEvtHandler pfnHandler;
    SEvtHandler* pNext;
};

// The handlers of one event type, by key; a pushed state links the previous one.
struct SEvtHandlerTable {
    SEvtHandler** ppBuckets;  // 0x00
    uint32_t nBuckets;        // 0x04
    uint32_t nHandlers;       // 0x08
    SEvtHandlerTable* pPrev;  // 0x0C
};

// One event type, (id, subId), in the table of types.
struct SEvtType {
    uint32_t dwEventId;           // 0x00
    uint32_t dwSubId;             // 0x04
    uint32_t nLastPriority;       // 0x08
    SEvtHandlerTable* pHandlers;  // 0x0C
    SEvtType* pNext;              // 0x10
};

// A request to stop the handler chain being dispatched for pData.
struct SEvtBreakCommand {
    TSLink tLink;
    void* pData;  // 0x08
};

// 1.14d mac: 003f04b4
static TSList s_tBreakCommands;
// The event critical section (named lpCriticalSection_0077930c in the Mac database).
static pthread_mutex_t s_tEventMutex;
// 1.14d mac: 003f04ec
static uint32_t s_nTypeBuckets;
// 1.14d mac: 003f04f0
static SEvtType** s_ppTypes;
// 1.14d mac: 003f04f4
static uint32_t s_nTypes;
// 1.14d mac: 003f04f8
// Set whenever a table changed: a dispatch in progress looks its handler up again.
static uint32_t s_bTablesChanged;
// 1.14d mac: 003f04fc
static LONG s_nDispatchDepth;

static SEvtType* SEVT_FindType(uint32_t dwEventId, uint32_t dwSubId) {
    if (s_ppTypes == NULL || s_nTypeBuckets == 0) {
        return NULL;
    }
    for (SEvtType* pType = s_ppTypes[(s_nTypeBuckets - 1) & (dwSubId ^ dwEventId)]; pType != NULL;
         pType = pType->pNext) {
        if (pType->dwEventId == dwEventId && pType->dwSubId == dwSubId) {
            return pType;
        }
    }
    return NULL;
}

// 1.14d mac: 000110bc
BOOL SEvtBreakHandlerChain(void* pData) {
    pthread_mutex_lock(&s_tEventMutex);
    SEvtBreakCommand* pCommand = (SEvtBreakCommand*)TSLIST_NewNode(&s_tBreakCommands, 2, 0, 0);
    pCommand->pData = pData;
    pthread_mutex_unlock(&s_tEventMutex);
    return 1;
}

// 1.14d mac: 000111df
static void SEVT_DeleteIdHashTable(SEvtHandlerTable* pTable) {
    SEvtHandler** ppBuckets = pTable->ppBuckets;
    uint32_t nBuckets = pTable->nBuckets;
    if (nBuckets != 0) {
        for (uint32_t nBucket = 0; nBucket < nBuckets; nBucket++) {
            SEvtHandler* pHandler = ppBuckets[nBucket];
            while (pHandler != NULL) {
                ppBuckets[nBucket] = pHandler->pNext;
                operator delete(pHandler);
                ppBuckets = pTable->ppBuckets;
                pHandler = ppBuckets[nBucket];
            }
            nBuckets = pTable->nBuckets;
        }
    }
    SMemFree(ppBuckets, SEVT_FILE, 0x5d, 0);
    operator delete(pTable);
}

// 1.14d mac: 00011118
BOOL SEvtDestroy(void) {
    pthread_mutex_lock(&s_tEventMutex);
    SEvtType** ppTypes = s_ppTypes;
    if (s_nTypeBuckets != 0) {
        uint32_t nBucketCount = s_nTypeBuckets;
        uint32_t nBucket = 0;
        do {
            SEvtType* pType = ppTypes[nBucket];
            while (pType != NULL) {
                SEvtHandlerTable* pTable = pType->pHandlers;
                while (pTable != NULL) {
                    pType->pHandlers = pTable->pPrev;
                    SEVT_DeleteIdHashTable(pTable);
                    ppTypes = s_ppTypes;
                    pTable = pType->pHandlers;
                }
                ppTypes[nBucket] = pType->pNext;
                operator delete(pType);
                ppTypes = s_ppTypes;
                nBucketCount = s_nTypeBuckets;
                pType = s_ppTypes[nBucket];
            }
            nBucket++;
        } while (nBucket < nBucketCount);
    }
    if (ppTypes != NULL) {
        operator delete(ppTypes);
    }
    s_ppTypes = NULL;
    s_nTypeBuckets = 0;
    s_nTypes = 0;
    s_bTablesChanged = 1;
    pthread_mutex_unlock(&s_tEventMutex);
    return 1;
}

// 1.14d mac: 00011280
// Calls the handlers registered for the key, highest registration order first. Each is
// looked up again under the lock, so a handler may change the tables; a break command for
// pData ends the chain.
BOOL SEvtDispatch(uint32_t dwEventId, uint32_t dwSubId, uint32_t dwKey, void* pData) {
    InterlockedIncrement(&s_nDispatchDepth);
    uint32_t nLastPriority = 0xffffffff;
    BOOL bResult = 0;
    SEvtHandler* pHandler = NULL;
    SEvtHandler* pNextHandler;
    do {
        pthread_mutex_lock(&s_tEventMutex);
        SEvtBreakCommand* pBreak = (SEvtBreakCommand*)s_tBreakCommands.tTerminator.nNext;
        if (IsListEnd((uintptr_t)pBreak)) {
            pBreak = NULL;
        }
        bool bBreak = false;
        if (pBreak != NULL) {
            do {
                if (pBreak->pData == pData) {
                    bBreak = true;
                    LINKEDLIST_UnlinkFreeAndGetCount(&s_tBreakCommands, &pBreak->tLink);
                    pBreak = NULL;
                } else {
                    pBreak = (SEvtBreakCommand*)pBreak->tLink.nNext;
                }
            } while (pBreak != NULL && ((uintptr_t)pBreak & 1) == 0);
            if (bBreak) {
                pthread_mutex_unlock(&s_tEventMutex);
                break;
            }
        }
        if (pHandler == NULL || (s_bTablesChanged & 1) != 0) {
            SEvtType* pType = SEVT_FindType(dwEventId, dwSubId);
            if (pType == NULL) {
                pthread_mutex_unlock(&s_tEventMutex);
                break;
            }
            SEvtHandlerTable* pTable = pType->pHandlers;
            if (pTable->ppBuckets == NULL || pTable->nBuckets == 0) {
                pthread_mutex_unlock(&s_tEventMutex);
                break;
            }
            // the first handler for the key below the last priority called
            SEvtHandler* pCursor = pTable->ppBuckets[(pTable->nBuckets - 1) & dwKey];
            pHandler = NULL;
            while (pCursor != NULL) {
                if (pCursor->dwKey == dwKey && pCursor->nPriority < nLastPriority) {
                    pHandler = pCursor;
                    break;
                }
                pCursor = pCursor->pNext;
            }
            if (s_nDispatchDepth == 1) {
                s_bTablesChanged = 0;
            }
        }
        if (pHandler == NULL) {
            pthread_mutex_unlock(&s_tEventMutex);
            break;
        }
        nLastPriority = pHandler->nPriority;
        pfnSEvtHandler pfnHandler = pHandler->pfnHandler;
        do {
            pHandler = pHandler->pNext;
            pNextHandler = NULL;
            if (pHandler == NULL) {
                break;
            }
            pNextHandler = pHandler;
        } while (pHandler->dwKey != dwKey);
        pthread_mutex_unlock(&s_tEventMutex);
        if (pfnHandler != NULL) {
            pfnHandler(pData);
            bResult = 1;
        }
        pHandler = pNextHandler;
    } while (pNextHandler != NULL);
    InterlockedDecrement(&s_nDispatchDepth);
    SEvtBreakCommand* pQueued = (SEvtBreakCommand*)s_tBreakCommands.tTerminator.nNext;
    if (pQueued != NULL && ((uintptr_t)pQueued & 1) == 0) {
        pthread_mutex_lock(&s_tEventMutex);
        SEvtBreakCommand* pNode = NULL;
        if (s_tBreakCommands.tTerminator.nNext != 0 && (s_tBreakCommands.tTerminator.nNext & 1) == 0) {
            pNode = (SEvtBreakCommand*)s_tBreakCommands.tTerminator.nNext;
        }
        while (pNode != NULL && ((uintptr_t)pNode & 1) == 0) {
            if (pNode->pData == pData) {
                pNode = (SEvtBreakCommand*)LINKEDLIST_UnlinkFreeAndGetCount(&s_tBreakCommands, &pNode->tLink);
            } else {
                pNode = (SEvtBreakCommand*)pNode->tLink.nNext;
            }
        }
        pthread_mutex_unlock(&s_tEventMutex);
    }
    return bResult;
}

// 1.14d mac: 000115a5
BOOL SEvtUnregisterType(uint32_t nEventId, uint32_t nSubId) {
    pthread_mutex_lock(&s_tEventMutex);
    BOOL bRemoved = 0;
    SEvtType* pType = SEVT_FindType(nEventId, nSubId);
    if (pType != NULL) {
        SEvtHandlerTable* pTable = pType->pHandlers;
        while (pTable != NULL) {
            pType->pHandlers = pTable->pPrev;
            SEVT_DeleteIdHashTable(pTable);
            pTable = pType->pHandlers;
        }
        SEvtType** ppLink = &s_ppTypes[(s_nTypeBuckets - 1) & (nSubId ^ nEventId)];
        while (*ppLink != NULL) {
            if (*ppLink == pType) {
                *ppLink = pType->pNext;
                operator delete(pType);
                s_nTypes--;
                break;
            }
            ppLink = &(*ppLink)->pNext;
        }
        s_bTablesChanged = 1;
        bRemoved = 1;
    }
    pthread_mutex_unlock(&s_tEventMutex);
    return bRemoved;
}

// 1.14d mac: 00011504
// Drops the current handler table and goes back to the one pushed before it; without one
// the event type goes.
void SEvtPopState(uint32_t nEventId, uint32_t nSubId) {
    pthread_mutex_lock(&s_tEventMutex);
    SEvtType* pType = SEVT_FindType(nEventId, nSubId);
    if (pType != NULL) {
        SEvtHandlerTable* pPrevious = pType->pHandlers->pPrev;
        if (pPrevious == NULL) {
            SEvtUnregisterType(nEventId, nSubId);
        } else {
            SEVT_DeleteIdHashTable(pType->pHandlers);
            pType->pHandlers = pPrevious;
        }
    }
    s_bTablesChanged = 1;
    pthread_mutex_unlock(&s_tEventMutex);
}

// 1.14d mac: 0001169c
// Pushes a copy of the event's handler table; the old one is kept below it.
BOOL SEvtPushState(uint32_t nEventId, uint32_t nSubId) {
    pthread_mutex_lock(&s_tEventMutex);
    BOOL bResult = 0;
    SEvtType* pType = SEVT_FindType(nEventId, nSubId);
    if (pType != NULL) {
        SEvtHandlerTable* pSnapshot = (SEvtHandlerTable*)SMemAlloc(0x10, SEVT_FILE, 0x11f, 0);
        SEvtHandlerTable* pCurrent = pType->pHandlers;
        pSnapshot->nBuckets = pCurrent->nBuckets;
        pSnapshot->nHandlers = pCurrent->nHandlers;
        pSnapshot->ppBuckets = (SEvtHandler**)SMemAlloc(pCurrent->nBuckets << 2, SEVT_FILE, 0x43, 0);
        for (uint32_t nBucket = 0; nBucket < pCurrent->nBuckets; nBucket++) {
            SEvtHandler** ppTail = &pSnapshot->ppBuckets[nBucket];
            for (SEvtHandler* pOld = pCurrent->ppBuckets[nBucket]; pOld != NULL; pOld = pOld->pNext) {
                SEvtHandler* pCopy = (SEvtHandler*)SMemAlloc(0x10, SEVT_FILE, 0x4a, 0);
                *ppTail = pCopy;
                *pCopy = *pOld;
                ppTail = &pCopy->pNext;
            }
            *ppTail = NULL;
        }
        pSnapshot->pPrev = pType->pHandlers;
        pType->pHandlers = pSnapshot;
        s_bTablesChanged = 1;
        bResult = 1;
    }
    pthread_mutex_unlock(&s_tEventMutex);
    return bResult;
}

// 1.14d mac: 0001183c
// The first handler registered for a type makes the type; the tables double when half full.
// A handler goes at the head of its key's chain with the next registration order.
BOOL SEvtRegisterHandler(uint32_t dwEventId, uint32_t dwSubId, uint32_t dwKey, int dwReserved,
                         pfnSEvtHandler pfnHandler) {
    if (dwReserved != 0 || pfnHandler == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    pthread_mutex_lock(&s_tEventMutex);
    SEvtType* pType = SEVT_FindType(dwEventId, dwSubId);
    if (pType == NULL) {
        uint32_t nBuckets = s_nTypeBuckets;
        SEvtType** ppBuckets = s_ppTypes;
        if (s_nTypeBuckets >> 1 <= s_nTypes) {
            uint32_t nNext = 1;
            do {
                nBuckets = nNext;
                nNext = nBuckets * 2;
            } while (nBuckets <= s_nTypes * 2 + 2);
            ppBuckets = (SEvtType**)SMemAlloc(nBuckets * 4, SEVT_FILE, 0x145, 8);
            SEvtType** ppOld = s_ppTypes;
            uint32_t nOldBuckets = s_nTypeBuckets;
            if (ppOld != NULL && nOldBuckets != 0) {
                for (uint32_t nBucket = 0; nBucket < nOldBuckets; nBucket++) {
                    SEvtType* pMove = ppOld[nBucket];
                    while (pMove != NULL) {
                        SEvtType* pFollowing = pMove->pNext;
                        uint32_t nIndex = (pMove->dwSubId ^ pMove->dwEventId) & (nBuckets - 1);
                        pMove->pNext = ppBuckets[nIndex];
                        ppBuckets[nIndex] = pMove;
                        pMove = pFollowing;
                    }
                }
            }
            if (ppOld != NULL) {
                SMemFree(ppOld, SEVT_FILE, 0x156, 0);
            }
        }
        s_ppTypes = ppBuckets;
        s_nTypeBuckets = nBuckets;
        SEvtType* pNew = (SEvtType*)SMemAlloc(0x14, SEVT_FILE, 0x15f, 8);
        uint32_t nHome = (s_nTypeBuckets - 1) & (dwSubId ^ dwEventId);
        pNew->dwEventId = dwEventId;
        pNew->dwSubId = dwSubId;
        pNew->pHandlers = (SEvtHandlerTable*)SMemAlloc(0x10, SEVT_FILE, 0x162, 8);
        pNew->pNext = s_ppTypes[nHome];
        s_ppTypes[nHome] = pNew;
        s_nTypes++;
        pType = s_ppTypes[nHome];
    }
    SEvtHandlerTable* pTable = pType->pHandlers;
    uint32_t nTableBuckets = pTable->nBuckets;
    if (nTableBuckets >> 1 <= pTable->nHandlers) {
        uint32_t nNext = 1;
        do {
            nTableBuckets = nNext;
            nNext = nTableBuckets * 2;
        } while (nTableBuckets <= pTable->nBuckets * 2 + 2);
        SEvtHandler** ppNewBuckets = (SEvtHandler**)SMemAlloc(nTableBuckets * 4, SEVT_FILE, 0x170, 8);
        SEvtHandler*** pppTails = (SEvtHandler***)SMemAlloc(nTableBuckets * 4, SEVT_FILE, 0x171, 8);
        for (uint32_t i = 0; i < nTableBuckets; i++) {
            pppTails[i] = &ppNewBuckets[i];
        }
        SEvtHandler** ppOldBuckets = pTable->ppBuckets;
        if (ppOldBuckets != NULL && pTable->nBuckets != 0) {
            for (uint32_t nBucket = 0; nBucket < pTable->nBuckets; nBucket++) {
                SEvtHandler* pMove = ppOldBuckets[nBucket];
                while (pMove != NULL) {
                    uint32_t nIndex = pMove->dwKey & (nTableBuckets - 1);
                    SEvtHandler* pFollowing = pMove->pNext;
                    pMove->pNext = NULL;
                    *pppTails[nIndex] = pMove;
                    pppTails[nIndex] = &pMove->pNext;
                    pMove = pFollowing;
                }
            }
        }
        SMemFree(pppTails, SEVT_FILE, 0x184, 0);
        if (pType->pHandlers->ppBuckets != NULL) {
            SMemFree(pType->pHandlers->ppBuckets, SEVT_FILE, 0x186, 0);
        }
        pType->pHandlers->ppBuckets = ppNewBuckets;
        pType->pHandlers->nBuckets = nTableBuckets;
    }
    uint32_t nHome = (nTableBuckets - 1) & dwKey;
    SEvtHandler* pNewHandler = (SEvtHandler*)SMemAlloc(0x10, SEVT_FILE, 399, 8);
    pNewHandler->dwKey = dwKey;
    pNewHandler->nPriority = ++pType->nLastPriority;
    pNewHandler->pfnHandler = pfnHandler;
    pNewHandler->pNext = pType->pHandlers->ppBuckets[nHome];
    pType->pHandlers->ppBuckets[nHome] = pNewHandler;
    pType->pHandlers->nHandlers++;
    s_bTablesChanged = 1;
    pthread_mutex_unlock(&s_tEventMutex);
    return 1;
}

// 1.14d mac: 00011c21
// Removes the handlers of a key: all of them, or only the one with the given callback.
BOOL SEvtUnregisterHandler(uint32_t dwType, uint32_t dwSubType, uint32_t dwId, pfnSEvtHandler pfnHandler) {
    pthread_mutex_lock(&s_tEventMutex);
    BOOL bResult = 0;
    SEvtType* pType = SEVT_FindType(dwType, dwSubType);
    if (pType != NULL) {
        SEvtHandlerTable* pTable = pType->pHandlers;
        if (pTable->ppBuckets != NULL && pTable->nBuckets != 0) {
            SEvtHandler** ppLink = &pTable->ppBuckets[(pTable->nBuckets - 1) & dwId];
            SEvtHandler* pHandler = *ppLink;
            while (pHandler != NULL) {
                if (pHandler->dwKey == dwId && (pfnHandler == NULL || pHandler->pfnHandler == pfnHandler)) {
                    *ppLink = pHandler->pNext;
                    operator delete(pHandler);
                    pHandler = *ppLink;
                    s_bTablesChanged = 1;
                    pType->pHandlers->nHandlers--;
                    bResult = 1;
                } else {
                    ppLink = &pHandler->pNext;
                    pHandler = pHandler->pNext;
                }
            }
        }
    }
    pthread_mutex_unlock(&s_tEventMutex);
    return bResult;
}

// 1.14d mac: 00011f70
// The static initialiser: the break-command list and the event mutex, destroyed at exit.
void SEVT_InitStatics(void) {
    s_tBreakCommands.nLinkOffset = 0;
    s_tBreakCommands.tTerminator.pPrev = &s_tBreakCommands.tTerminator;
    s_tBreakCommands.tTerminator.nNext = (uintptr_t)&s_tBreakCommands.tTerminator | 1;
    __cxa_atexit(TSLIST_Destructor, &s_tBreakCommands, &__dso_handle);
    CRITSECT_Init(&s_tEventMutex);
    __cxa_atexit(CRITSECT_Destroy, &s_tEventMutex, &__dso_handle);
}

#endif  // D2_OS_MAC
