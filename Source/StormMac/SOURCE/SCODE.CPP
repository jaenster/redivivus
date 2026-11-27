// StormMac Storm Mac/SOURCE/SCODE.CPP: the code-stream objects (SCodeDelete, SCodeDestroy)
// and the TSList instantiations the file's object holds, 1.14d Intel Mac
// 0000e610..0000eab0 (the static initialiser closes it).
//
// __FILE__ is ".../StormMac/Storm Mac/SOURCE/SCODE.CPP". The list helpers occur twice or
// three times in the binary (one set per list type the file instantiates); each address
// has its own definition here.
#include "../Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <string.h>

void* SMemAlloc(uint32_t nBytes, const char* szFile, int nLine, uint32_t dwFlags);
int SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags);
void SErrSetLastError(DWORD dwErrCode);
void SErrReportResourceLeak(const char* szType);

#define SCODE_FILE "/Users/bclemetson/dev/diablo2/Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/SOURCE/SCODE.CPP"

typedef uintptr_t LINKWORD;

// A list header: the link offset, then the list's own link {last link, first element}.
struct SCODE_LIST {
    int32_t nLinkOffset;  // 0x00
    LINKWORD nLastLink;   // 0x04
    LINKWORD nFirst;      // 0x08: first element; empty: the terminator link | 1
};

// 1.14d mac: 003f0270
static SCODE_LIST s_tBlockList;
// 1.14d mac: 003f027c
// The live code streams.
static SCODE_LIST s_tStreamList;
// 1.14d mac: 003f0288
// A buffer descriptor (pointer, size, capacity), freed at shutdown.
static struct {
    void* pData;
    uint32_t nSize;
    uint32_t nCapacity;
} s_tBuffer1;
// 1.14d mac: 003f0294
static struct {
    void* pData;
    uint32_t nSize;
    uint32_t nCapacity;
} s_tBuffer2;

static inline void* ListElement(LINKWORD nNext) {
    if (nNext == 0 || (nNext & 1) != 0) {
        return NULL;
    }
    return (void*)nNext;
}

static void* TSLIST_DeleteNode(SCODE_LIST* pList, void* pNode);
static void TSLIST_DeleteAll(SCODE_LIST* pList);
static void TSLINK_Unlink(void* pLink);

// 1.14d mac: 0000e610
// Frees the three buffers of a code stream, then the stream.
BOOL SCodeDelete(uint8_t* pCodeStream) {
    if (pCodeStream == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    if (*(void**)(pCodeStream + 0x1c) != NULL) {
        SMemFree(*(void**)(pCodeStream + 0x1c), SCODE_FILE, 0x454, 0);
        *(void**)(pCodeStream + 0x1c) = NULL;
    }
    if (*(void**)(pCodeStream + 0x34) != NULL) {
        SMemFree(*(void**)(pCodeStream + 0x34), SCODE_FILE, 0x458, 0);
        *(void**)(pCodeStream + 0x34) = NULL;
    }
    if (*(void**)(pCodeStream + 0x4c) != NULL) {
        SMemFree(*(void**)(pCodeStream + 0x4c), SCODE_FILE, 0x45c, 0);
        *(void**)(pCodeStream + 0x4c) = NULL;
    }
    TSLIST_DeleteNode(&s_tStreamList, pCodeStream);
    return 1;
}

// 1.14d mac: 0000e6e7
// Every stream still alive is a leak, then deleted.
BOOL SCodeDestroy(void) {
    while (ListElement(s_tStreamList.nFirst) != NULL) {
        SErrReportResourceLeak("HSCODESTREAM");
        SCodeDelete((uint8_t*)ListElement(s_tStreamList.nFirst));
    }
    if (s_tBuffer1.pData != NULL) {
        SMemFree(s_tBuffer1.pData, SCODE_FILE, 0x473, 0);
    }
    memset(&s_tBuffer1, 0, 0xc);
    if (s_tBuffer2.pData != NULL) {
        SMemFree(s_tBuffer2.pData, SCODE_FILE, 0x476, 0);
    }
    memset(&s_tBuffer2, 0, 0xc);
    TSLIST_DeleteAll(&s_tBlockList);
    return 1;
}

static void TSLIST_UnlinkAll(SCODE_LIST* pList);

// The list instantiations, by the order the binary holds them.

// 1.14d mac: 0000e7f4
static void TSLIST_Destructor_1(void* pList) {
    TSLIST_UnlinkAll((SCODE_LIST*)pList);
    TSLINK_Unlink((LINKWORD*)pList + 1);
}

// 1.14d mac: 0000e816
static void TSLIST_Destructor_2(void* pList) {
    TSLIST_UnlinkAll((SCODE_LIST*)pList);
    TSLINK_Unlink((LINKWORD*)pList + 1);
}

// 1.14d mac: 0000e838
// TSList::DeleteNode: unlinks and frees the node, returns the element after it.
static void* TSLIST_DeleteNode(SCODE_LIST* pList, void* pNode) {
    LINKWORD* pLink = (LINKWORD*)pList + 1;
    if (pNode != NULL) {
        pLink = (LINKWORD*)pNode;
    }
    LINKWORD nNext = pLink[1];
    TSLINK_Unlink(pNode);
    TSLINK_Unlink(pNode);
    void* pResult = NULL;
    if (nNext != 0 && (nNext & 1) == 0) {
        pResult = (void*)nNext;
    }
    SMemFree(pNode, "", -2, 0);
    return pResult;
}

static void* TSLINK_NextLink(void* pLink, int nLinkOffset);

// 1.14d mac: 0000e8a8
static void TSLINK_Unlink(void* pLinkArg) {
    LINKWORD* pLink = (LINKWORD*)pLinkArg;
    LINKWORD nPrevious = *pLink;
    if (nPrevious != 0) {
        LINKWORD* pNextLink = (LINKWORD*)TSLINK_NextLink(pLink, -1);
        *pNextLink = nPrevious;
        ((LINKWORD*)*pLink)[1] = pLink[1];
        *pLink = 0;
        pLink[1] = 0;
    }
}

// 1.14d mac: 0000e8e8
static void* TSLINK_NextLink(void* pLinkArg, int nLinkOffset) {
    LINKWORD* pLink = (LINKWORD*)pLinkArg;
    LINKWORD nNext = pLink[1];
    if (nNext == 0 || (nNext & 1) != 0) {
        return (void*)(nNext & 0xfffffffe);
    }
    if (nLinkOffset < 0) {
        nLinkOffset = (int)((LINKWORD)pLink - ((LINKWORD*)*pLink)[1]);
    }
    return (void*)(nNext + nLinkOffset);
}

// 1.14d mac: 0000e91e
static void TSLIST_UnlinkAll(SCODE_LIST* pList) {
    void* pNode;
    while ((pNode = ListElement(pList->nFirst)) != NULL) {
        TSLINK_Unlink(pNode);
    }
}

// 1.14d mac: 0000e966
static void TSLIST_UnlinkAll_2(SCODE_LIST* pList) {
    void* pNode;
    while ((pNode = ListElement(pList->nFirst)) != NULL) {
        TSLINK_Unlink(pNode);
    }
}

// 1.14d mac: 0000e9ae
static void TSLINK_Unlink_2(void* pLinkArg) {
    LINKWORD* pLink = (LINKWORD*)pLinkArg;
    LINKWORD nPrevious = *pLink;
    if (nPrevious != 0) {
        LINKWORD* pNextLink = (LINKWORD*)TSLINK_NextLink(pLink, -1);
        *pNextLink = nPrevious;
        ((LINKWORD*)*pLink)[1] = pLink[1];
        *pLink = 0;
        pLink[1] = 0;
    }
}

// 1.14d mac: 0000e9ee
static void* TSLINK_NextLink_2(void* pLinkArg, int nLinkOffset) {
    LINKWORD* pLink = (LINKWORD*)pLinkArg;
    LINKWORD nNext = pLink[1];
    if (nNext == 0 || (nNext & 1) != 0) {
        return (void*)(nNext & 0xfffffffe);
    }
    if (nLinkOffset < 0) {
        nLinkOffset = (int)((LINKWORD)pLink - ((LINKWORD*)*pLink)[1]);
    }
    return (void*)(nNext + nLinkOffset);
}

// 1.14d mac: 0000ea24
// TSList::DeleteAll
static void TSLIST_DeleteAll(SCODE_LIST* pList) {
    void* pLink = ListElement(pList->nFirst);
    while (pLink != NULL) {
        TSLINK_Unlink(pLink);
        TSLINK_Unlink(pLink);
        SMemFree(pLink, "", -2, 0);
        pLink = ListElement(pList->nFirst);
    }
}

extern "C" int __cxa_atexit(void (*pfnDestructor)(void*), void* pObject, void* pDso);
extern "C" void* __dso_handle;

// 1.14d mac: 0000eab0
// The file's static initialiser: both lists empty, a destructor for each.
void SCODE_InitStaticLists(void) {
    s_tBlockList.nLinkOffset = 0;
    s_tBlockList.nLastLink = (LINKWORD)&s_tBlockList.nLastLink;
    s_tBlockList.nFirst = (LINKWORD)&s_tBlockList.nLastLink + 1;
    __cxa_atexit(TSLIST_Destructor_1, &s_tBlockList, &__dso_handle);
    s_tStreamList.nLinkOffset = 0;
    s_tStreamList.nLastLink = (LINKWORD)&s_tStreamList.nLastLink;
    s_tStreamList.nFirst = (LINKWORD)&s_tStreamList.nLastLink + 1;
    __cxa_atexit(TSLIST_Destructor_2, &s_tStreamList, &__dso_handle);
}

#endif  // D2_OS_MAC
