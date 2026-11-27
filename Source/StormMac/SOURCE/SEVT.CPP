// StormMac Storm Mac/SOURCE/SEVT.CPP: Storm's event dispatcher (handler tables per
// event type, push/pop of the handler state, the break-chain request), 1.14d Intel Mac
// 000110bc..00011f70.
//
// The binary's __FILE__ is ".../StormMac/Storm Mac/SOURCE/SEVT.CPP" (lines 0x43..0x18f).
// The file starts at SEvtBreakHandlerChain, after SERR.CPP's static initialiser
// (00011040), and ends with its own at 00011f70; SFILE.CPP follows.
#include "../Win32Mac/Source/Win32/Win32Mac.h"
#pragma GCC visibility push(hidden)
#include "../H/stpl.h"
#pragma GCC visibility pop

#if !defined(_WIN32) && D2_OS_MAC

#include <string.h>

void* SMemAlloc(uint32_t nBytes, const char* szFile, int nLine, uint32_t dwFlags);
int SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags);
void SErrSetLastError(DWORD dwErrCode);
LONG InterlockedIncrement(LPLONG pnValue);
LONG InterlockedDecrement(LPLONG pnValue);
// Out of line in SFILE.CPP's neighbourhood.
void CRITSECT_Init(pthread_mutex_t* pMutex);
void CRITSECT_Destroy(pthread_mutex_t* pMutex) __attribute__((visibility("hidden")));

extern "C" int __cxa_atexit(void (*pfnDestructor)(void*), void* pObject, void* pDso);
extern "C" void* __dso_handle;

#define SEVT_FILE "/Users/bclemetson/dev/diablo2/Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/SOURCE/SEVT.CPP"

typedef void (*pfnSEvtHandler)(void* pData);

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

// The event critical section: made by the static initialiser, which registers its destructor
// straight from Storm's own CRITSECT_Destroy.
class SEvtCritSect : public pthread_mutex_t {
public:
    SEvtCritSect() {
        CRITSECT_Init(this);
        __cxa_atexit((void (*)(void*))CRITSECT_Destroy, this, &__dso_handle);
    }
};

// A request to stop the handler chain being dispatched for pData.
struct SEvtBreakCommand : TSLinkedNode<SEvtBreakCommand> {
    void* pData;  // 0x08
};

// 1.14d mac: 003f04b4
static TSExplicitList<SEvtBreakCommand, 0> s_tBreakCommands;
// 1.14d mac: 003f04c0
// The event critical section (named lpCriticalSection_0077930c in the Mac database).
static SEvtCritSect s_tEventMutex;
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

static inline __attribute__((always_inline)) SEvtType* SEVT_FindType(uint32_t dwEventId, uint32_t dwSubId) {
    if (s_ppTypes == NULL || s_nTypeBuckets == 0) {
        return NULL;
    }
    for (SEvtType* pType = s_ppTypes[(s_nTypeBuckets - 1) & (dwEventId ^ dwSubId)]; pType != NULL;
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
    SEvtBreakCommand* pCommand = s_tBreakCommands.NewNode(STORM_LIST_LINK_BEFORE, 0, 0);
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
                delete pHandler;
                ppBuckets = pTable->ppBuckets;
                pHandler = ppBuckets[nBucket];
            }
            nBuckets = pTable->nBuckets;
        }
    }
    SMemFree(ppBuckets, SEVT_FILE, 0x5d, 0);
    delete pTable;
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
                delete pType;
                ppTypes = s_ppTypes;
                nBucketCount = s_nTypeBuckets;
                pType = s_ppTypes[nBucket];
            }
            nBucket++;
        } while (nBucket < nBucketCount);
    }
    if (ppTypes != NULL) {
        delete ppTypes;
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
        SEvtBreakCommand* pBreak = s_tBreakCommands.Head();
        bool bBreak = false;
        if (pBreak != NULL) {
            do {
                if (pBreak->pData == pData) {
                    bBreak = true;
                    s_tBreakCommands.DeleteNode(pBreak);
                    pBreak = NULL;
                } else {
                    pBreak = s_tBreakCommands.Next(pBreak);
                }
            } while (pBreak != NULL);
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
    if (s_tBreakCommands.Head() != NULL) {
        pthread_mutex_lock(&s_tEventMutex);
        SEvtBreakCommand* pNode = s_tBreakCommands.Head();
        while (pNode != NULL) {
            if (pNode->pData == pData) {
                pNode = s_tBreakCommands.DeleteNode(pNode);
            } else {
                pNode = s_tBreakCommands.Next(pNode);
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
        SEvtType** ppLink = &s_ppTypes[(s_nTypeBuckets - 1) & (nEventId ^ nSubId)];
        SEvtType* pCursor = *ppLink;
        while (pCursor != NULL) {
            SEvtType* pNext = pCursor->pNext;
            if (pCursor == pType) {
                *ppLink = pNext;
                delete pType;
                s_nTypes--;
                pCursor = *ppLink;
            } else {
                ppLink = &pCursor->pNext;
                pCursor = pNext;
            }
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
BOOL SEvtPopState(uint32_t nEventId, uint32_t nSubId) {
    pthread_mutex_lock(&s_tEventMutex);
    BOOL bResult = 0;
    SEvtType* pType = SEVT_FindType(nEventId, nSubId);
    if (pType != NULL) {
        SEvtHandlerTable* pPrevious = pType->pHandlers->pPrev;
        if (pPrevious == NULL) {
            bResult = SEvtUnregisterType(nEventId, nSubId);
        } else {
            SEVT_DeleteIdHashTable(pType->pHandlers);
            pType->pHandlers = pPrevious;
        }
    }
    s_bTablesChanged = 1;
    pthread_mutex_unlock(&s_tEventMutex);
    return bResult;
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
        uint32_t nHome = (s_nTypeBuckets - 1) & (dwEventId ^ dwSubId);
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
                    delete pHandler;
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

template void TSLink<SEvtBreakCommand>::Unlink();
template TSLink<SEvtBreakCommand>* TSLink<SEvtBreakCommand>::NextLink(int);
template void TSExplicitList<SEvtBreakCommand, 0>::UnlinkAll();
template void TSExplicitList<SEvtBreakCommand, 0>::LinkNode(SEvtBreakCommand*, uint32_t, SEvtBreakCommand*);
template SEvtBreakCommand* TSExplicitList<SEvtBreakCommand, 0>::NewNode(uint32_t, uint32_t, uint32_t);
template SEvtBreakCommand* TSExplicitList<SEvtBreakCommand, 0>::DeleteNode(SEvtBreakCommand*);
template TSExplicitList<SEvtBreakCommand, 0>::~TSExplicitList();

// 1.14d mac: 00011d2e TSExplicitList<SEvtBreakCommand,0>::~TSExplicitList[D2]
// 1.14d mac: 00011d50 TSExplicitList<SEvtBreakCommand,0>::NewNode
// 1.14d mac: 00011dc6 TSExplicitList<SEvtBreakCommand,0>::DeleteNode
// 1.14d mac: 00011e36 TSLink<SEvtBreakCommand>::Unlink
// 1.14d mac: 00011e76 TSLink<SEvtBreakCommand>::NextLink
// 1.14d mac: 00011eac TSExplicitList<SEvtBreakCommand,0>::LinkNode
// 1.14d mac: 00011f20 TSExplicitList<SEvtBreakCommand,0>::UnlinkAll
// 1.14d mac: 00011f70 _GLOBAL__I_a

#endif  // D2_OS_MAC
