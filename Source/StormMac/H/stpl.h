// Storm's container templates as the 1.14d Intel Mac build instantiates them: the
// intrusive list link (TSLink), the list (TSExplicitList), the growable array
// (TSGrowableArray) and the string-keyed hash table (TSHashTable). The binary names
// the original header "../../../StormMac/Storm Mac/H/stpl.h" through the __FILE__
// literal of HASHKEY_STRI's frees, so this file sits at the matching place in the Mac
// tree. Hand-written from the instantiations clang-425 emitted into Win32_utils.cpp
// (0002d24c..0002eae0); each address lives on that file's list of the instantiations
// it holds, since one template body here is more than one function there.
//
// The Mac layout of a link is Windows' (Source/Storm/H/stpl.h): the previous link,
// then the next element, but the end of a list is marked with the terminator link's
// address with bit 0 set, where Windows stores its complement.
//
// Mac-only, like the rest of StormMac: see Win32Mac/Source/Win32/Win32Mac.h for why
// the guard is spelled `!defined(_WIN32) && D2_OS_MAC`.
#pragma once

#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <new>
#include <stddef.h>
#include <stdint.h>

// ---- the Storm entry points the templates call ----

// 1.14d mac: 00018b0b
void* SMemAlloc(uint32_t nBytes, const char* szFile, int nLine, uint32_t dwFlags);
// 1.14d mac: 00018c44
int SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags);
// 1.14d mac: 00018c12
void* SMemReAlloc(void* pMem, uint32_t nBytes, const char* szFile, int nLine, uint32_t dwFlags);
// 1.14d mac: 0001cfaf
uint32_t SStrHashHT(const char* szString);
// 1.14d mac: 0001c7d3
int SStrCmpI(const char* szString1, const char* szString2, uint32_t nMaxChars);
// 1.14d mac: 0001c9bd
char* SStrDup(const char* szString);
// 1.14d mac: 00009632
// Storm's global operator delete (an SMemFree).
void operator delete(void* pMem) throw();

#define SMEM_FLAG_ZEROMEMORY 0x08
#define SMEM_FLAG_NOMOVE 0x10

// The value a list's link offset holds until its owner sets it.
#define STORM_LIST_OFFSET_UNSET ((int)0xDDDDDDDD)

// Where LinkNode puts a node relative to the reference node (or the list's end).
#define STORM_LIST_LINK_AFTER 1
#define STORM_LIST_LINK_BEFORE 2

// The original's copies of these templates are hidden: a static initialiser reaches their
// destructors by address, not through a non-lazy pointer.

// ---- TSLink ----

template<class T>
class TSLink {
public:
    TSLink<T>* m_prevlink;  // 0x00: the previous link, or the list's terminator
    T* m_next;              // 0x04: the next element; at the end, the terminator link's address | 1

    TSLink() : m_prevlink(NULL), m_next(NULL) {}
    // A copy is a fresh, unlinked link.
    TSLink(const TSLink<T>&) : m_prevlink(NULL), m_next(NULL) {}
    ~TSLink() { Unlink(); }

    bool IsLinked() { return m_prevlink != NULL; }
    // Both tests are evaluated: the end test is a bitwise or.
    static bool IsTerminator(T* pNext) { return (pNext == NULL) | ((intptr_t)pNext & 1); }
    T* Next() { return IsTerminator(m_next) ? NULL : m_next; }
    TSLink<T>* NextLink(int nLinkOffset);
    void Unlink();
};

template<class T>
TSLink<T>* TSLink<T>::NextLink(int nLinkOffset) {
    T* pNext = m_next;
    if (IsTerminator(pNext)) {
        return (TSLink<T>*)((intptr_t)pNext & ~1);
    }
    if (nLinkOffset < 0) {
        nLinkOffset = (intptr_t)this - (intptr_t)m_prevlink->m_next;
    }
    return (TSLink<T>*)((intptr_t)pNext + nLinkOffset);
}

template<class T>
void TSLink<T>::Unlink() {
    TSLink<T>* pPrev = m_prevlink;
    if (pPrev != NULL) {
        NextLink(-1)->m_prevlink = pPrev;
        m_prevlink->m_next = m_next;
        m_prevlink = NULL;
        m_next = NULL;
    }
}

// An element that carries its own link as its first member.
template<class T>
class TSLinkedNode {
public:
    TSLink<T> m_link;

    ~TSLinkedNode() { m_link.Unlink(); }
};

// ---- TSExplicitList ----

template<class T, int OFFSET>
class TSExplicitList {
public:
    int m_linkoffset;        // 0x00: where an element's link sits in it
    TSLink<T> m_terminator;  // 0x04: the list's own link: last link, first element

    TSExplicitList() {
        m_linkoffset = OFFSET;
        InitializeTerminator();
    }
    // A copy takes the link offset, not the elements.
    TSExplicitList(const TSExplicitList<T, OFFSET>& tSource) {
        m_linkoffset = tSource.m_linkoffset;
        InitializeTerminator();
    }
    ~TSExplicitList() { UnlinkAll(); }

    void InitializeTerminator() {
        m_terminator.m_prevlink = &m_terminator;
        m_terminator.m_next = (T*)((intptr_t)&m_terminator | 1);
    }
    void SetLinkOffset(int nLinkOffset) {
        UnlinkAll();
        m_linkoffset = nLinkOffset;
        InitializeTerminator();
    }
    void ChangeLinkOffset(int nLinkOffset) {
        if (nLinkOffset != m_linkoffset) {
            SetLinkOffset(nLinkOffset);
        }
    }
    TSLink<T>* Link(T* pNode) { return pNode ? (TSLink<T>*)((intptr_t)pNode + LinkOffset()) : &m_terminator; }
    // A list of a fixed offset ignores the field; the unset sentinel defers to it.
    int LinkOffset() { return OFFSET == STORM_LIST_OFFSET_UNSET ? m_linkoffset : OFFSET; }
    T* Head() { return m_terminator.Next(); }
    bool IsEmpty() { return TSLink<T>::IsTerminator(m_terminator.m_next); }
    T* Next(T* pNode) { return Link(pNode)->Next(); }
    void UnlinkNode(T* pNode) { Link(pNode)->Unlink(); }
    void UnlinkAll();
    void LinkNode(T* pNode, uint32_t nLinkType, T* pExisting);
    T* NewNode(uint32_t nLinkType, uint32_t nExtraBytes, uint32_t dwFlags);
    // Destroys the node (its destructor unlinks it), frees it, and returns the node after it.
    T* DeleteNode(T* pNode) {
        T* pNext = Next(pNode);
        pNode->~T();
        SMemFree(pNode, "", -2, 0);
        return pNext;
    }
    void DeleteAll();
};

template<class T, int OFFSET>
void TSExplicitList<T, OFFSET>::DeleteAll() {
    T* pNode;
    while ((pNode = Head()) != NULL) {
        pNode->~T();
        SMemFree(pNode, "", -2, 0);
    }
}

template<class T, int OFFSET>
void TSExplicitList<T, OFFSET>::UnlinkAll() {
    T* pNode;
    while ((pNode = Head()) != NULL) {
        UnlinkNode(pNode);
    }
}

template<class T, int OFFSET>
void TSExplicitList<T, OFFSET>::LinkNode(T* pNode, uint32_t nLinkType, T* pExisting) {
    TSLink<T>* pLink = Link(pNode);
    if (pLink->IsLinked()) {
        pLink->Unlink();
    }
    TSLink<T>* pExistingLink = Link(pExisting);
    switch (nLinkType) {
    case STORM_LIST_LINK_AFTER:
        pLink->m_prevlink = pExistingLink;
        pLink->m_next = pExistingLink->m_next;
        pExistingLink->NextLink(m_linkoffset)->m_prevlink = pLink;
        pExistingLink->m_next = pNode;
        break;
    case STORM_LIST_LINK_BEFORE: {
        TSLink<T>* pPrev = pExistingLink->m_prevlink;
        pLink->m_prevlink = pPrev;
        pLink->m_next = pPrev->m_next;
        pPrev->m_next = pNode;
        pExistingLink->m_prevlink = pLink;
        break;
    }
    }
}

template<class T, int OFFSET>
T* TSExplicitList<T, OFFSET>::NewNode(uint32_t nLinkType, uint32_t nExtraBytes, uint32_t dwFlags) {
    void* pMem = SMemAlloc(sizeof(T) + nExtraBytes, "", -2, dwFlags | SMEM_FLAG_ZEROMEMORY);
    T* pNode = pMem ? new (pMem) T : NULL;
    if (nLinkType) {
        LinkNode(pNode, nLinkType, NULL);
    }
    return pNode;
}

// ---- TSList ----

// A list whose elements start with their link: the node is its own link, so no link offset
// is applied to a node (the list still carries one, which LinkNode hands to NextLink).
template<class T>
class TSLinkList {
public:
    int m_linkoffset;        // 0x00
    TSLink<T> m_terminator;  // 0x04: the list's own link: last link, first element

    TSLinkList() : m_linkoffset(0) { InitializeTerminator(); }
    ~TSLinkList() { UnlinkAll(); }

    void InitializeTerminator() {
        m_terminator.m_prevlink = &m_terminator;
        m_terminator.m_next = (T*)((intptr_t)&m_terminator | 1);
    }
    TSLink<T>* Link(T* pNode) { return pNode ? (TSLink<T>*)pNode : &m_terminator; }
    T* Head() { return m_terminator.Next(); }
    T* Next(T* pNode) { return Link(pNode)->Next(); }
    void UnlinkNode(T* pNode) { Link(pNode)->Unlink(); }
    void UnlinkAll();
    void LinkNode(T* pNode, uint32_t nLinkType, T* pExisting);
    T* NewNode(uint32_t nLinkType, uint32_t nExtraBytes, uint32_t dwFlags);
    // Unlinks the node, frees it, and returns the node after it.
    T* DeleteNode(T* pNode) {
        T* pNext = Next(pNode);
        UnlinkNode(pNode);
        pNode->~T();
        SMemFree(pNode, "", -2, 0);
        return pNext;
    }
};

template<class T>
void TSLinkList<T>::UnlinkAll() {
    T* pNode;
    while ((pNode = Head()) != NULL) {
        UnlinkNode(pNode);
    }
}

template<class T>
void TSLinkList<T>::LinkNode(T* pNode, uint32_t nLinkType, T* pExisting) {
    TSLink<T>* pLink = Link(pNode);
    if (pLink->IsLinked()) {
        pLink->Unlink();
    }
    TSLink<T>* pExistingLink = pExisting ? (TSLink<T>*)pExisting : &m_terminator;
    switch (nLinkType) {
    case STORM_LIST_LINK_AFTER:
        pLink->m_prevlink = pExistingLink;
        pLink->m_next = pExistingLink->m_next;
        pExistingLink->NextLink(m_linkoffset)->m_prevlink = pLink;
        pExistingLink->m_next = pNode;
        break;
    case STORM_LIST_LINK_BEFORE: {
        TSLink<T>* pPrev = pExistingLink->m_prevlink;
        pLink->m_prevlink = pPrev;
        pLink->m_next = pPrev->m_next;
        pPrev->m_next = pNode;
        pExistingLink->m_prevlink = pLink;
        break;
    }
    }
}

template<class T>
T* TSLinkList<T>::NewNode(uint32_t nLinkType, uint32_t nExtraBytes, uint32_t dwFlags) {
    void* pMem = SMemAlloc(sizeof(T) + nExtraBytes, "", -2, dwFlags | SMEM_FLAG_ZEROMEMORY);
    T* pNode = pMem ? new (pMem) T : NULL;
    if (nLinkType) {
        LinkNode(pNode, nLinkType, NULL);
    }
    return pNode;
}

// ---- TSGrowableArray ----

template<class T>
class TSGrowableArray {
public:
    uint32_t m_alloc;  // 0x00: elements allocated
    uint32_t m_count;  // 0x04: elements in use
    T* m_data;         // 0x08
    uint32_t m_chunk;  // 0x0C: growth granularity; 0 until CalcChunkSize picks one

    TSGrowableArray() : m_alloc(0), m_count(0), m_data(NULL), m_chunk(0) {}
    ~TSGrowableArray();

    uint32_t Count() { return m_count; }
    T& operator[](uint32_t nIndex) { return m_data[nIndex]; }
    uint32_t CalcChunkSize(uint32_t nCount);
    static uint32_t RoundToMultiple(uint32_t nValue, uint32_t nMultiple) {
        if (nValue % nMultiple) {
            nValue = nValue + nMultiple - nValue % nMultiple;
        }
        return nValue;
    }
    void ReallocData(uint32_t nAlloc);
    void Reserve(uint32_t nCount, int bRound);
    void SetCount(uint32_t nCount);
    T* New();
    void Clear() {
        this->~TSGrowableArray();
        m_alloc = 0;
        m_count = 0;
        m_data = NULL;
    }
};

template<class T>
TSGrowableArray<T>::~TSGrowableArray() {
    for (uint32_t i = 0; i < m_count; i++) {
        m_data[i].~T();
    }
    if (m_data) {
        SMemFree(m_data, "nothing", -2, 0);
    }
}

template<class T>
uint32_t TSGrowableArray<T>::CalcChunkSize(uint32_t nCount) {
    if (m_chunk) {
        return m_chunk;
    }
    if (nCount < 256 / sizeof(T)) {
        uint32_t nTop;
        do {
            nTop = nCount;
            nCount &= nCount - 1;
        } while (nCount);
        return nTop > 1 ? nTop : 1;
    }
    m_chunk = 256 / sizeof(T);
    return m_chunk;
}

template<class T>
void TSGrowableArray<T>::ReallocData(uint32_t nAlloc) {
    T* pOldData = m_data;
    if (nAlloc < m_count) {
        for (uint32_t i = nAlloc; i < m_count; i++) {
            pOldData[i].~T();
        }
    }
    m_alloc = nAlloc;
    m_data = (T*)SMemReAlloc(pOldData, nAlloc * sizeof(T), "nothing", -2, SMEM_FLAG_NOMOVE);
    if (m_data == NULL) {
        m_data = (T*)SMemAlloc(nAlloc * sizeof(T), "nothing", -2, 0);
        if (pOldData) {
            uint32_t nCopy = m_count < nAlloc ? m_count : nAlloc;
            for (uint32_t i = 0; i < nCopy; i++) {
                new (&m_data[i]) T(pOldData[i]);
                pOldData[i].~T();
            }
            SMemFree(pOldData, "nothing", -2, 0);
        }
    }
}

template<class T>
void TSGrowableArray<T>::Reserve(uint32_t nCount, int bRound) {
    uint32_t nAlloc = m_count + nCount;
    if (nAlloc > m_alloc) {
        if (bRound) {
            nAlloc = RoundToMultiple(nAlloc, CalcChunkSize(nAlloc));
        }
        ReallocData(nAlloc);
    }
}

template<class T>
void TSGrowableArray<T>::SetCount(uint32_t nCount) {
    if (nCount > m_count) {
        Reserve(nCount - m_count, 1);
        for (uint32_t i = m_count; i < nCount; i++) {
            new (&m_data[i]) T;
        }
    } else if (nCount < m_count) {
        for (uint32_t i = nCount; i < m_count; i++) {
            m_data[i].~T();
        }
    }
    m_count = nCount;
}

template<class T>
T* TSGrowableArray<T>::New() {
    Reserve(1, 1);
    new (&m_data[m_count]) T;
    return &m_data[m_count++];
}

// ---- the hash table ----

// A case-insensitive string key that owns a copy of its string.
class HASHKEY_STRI {
public:
    char* m_str;

    HASHKEY_STRI() : m_str(NULL) {}
    ~HASHKEY_STRI() {
        if (m_str) {
            // the original header's line
            SMemFree(m_str, __FILE__, 2691, 0);
        }
    }
    HASHKEY_STRI& operator=(const char* szString) {
        if (m_str != szString) {
            if (m_str) {
                SMemFree(m_str, __FILE__, 2706, 0);
            }
            m_str = SStrDup(szString);
        }
        return *this;
    }
    bool operator==(const char* szString) { return SStrCmpI(m_str, szString, 0x7FFFFFFF) == 0; }
};

template<class T, class TKey>
class TSHashObject {
public:
    uint32_t m_hashval;      // 0x00
    TSLink<T> m_linktoslot;  // 0x04: in its slot's list
    TSLink<T> m_linktofull;  // 0x0C: in the table's list of every node
    TKey m_key;              // 0x14
};

template<class T, class TKey>
class TSHashTable {
public:
    typedef TSExplicitList<T, STORM_LIST_OFFSET_UNSET> List;

    // 0x00: vtable
    List m_fulllist;                         // 0x04
    uint32_t m_fullnessIndicator;            // 0x10
    TSGrowableArray<List> m_slotlistarray;  // 0x14
    uint32_t m_slotmask;                     // 0x24: slot count - 1; -1 until the first node

    TSHashTable() : m_fullnessIndicator(0) {
        m_fulllist.SetLinkOffset(offsetof(T, m_linktofull));
        m_slotmask = (uint32_t)-1;
    }
    // Never inlined: the original keeps it a function of its own even where the table's type is known.
    __attribute__((noinline)) virtual void InternalDelete(T* pNode);
    virtual T* InternalNew(List* pList, uint32_t nExtraBytes, uint32_t dwFlags);
    virtual ~TSHashTable();
    virtual void Destroy();

    void InternalClear(int bUnlinkOnly);
    // Takes a node out of both lists; the node's own storage stays.
    void Unlink(T* pNode);
    // Unlinks and deletes a node and returns the one after it in the full list.
    T* DeleteNode(T* pNode);
    void InitializeTableSlots();
    int MonitorFullness(uint32_t nSlot);
    void GrowListArray(uint32_t nNewSize);
    T* InternalNewNode(uint32_t nHashVal, uint32_t nExtraBytes, uint32_t dwFlags);
    T* Ptr(const char* szKey);
    T* New(const char* szKey, uint32_t nExtraBytes, uint32_t dwFlags) {
        uint32_t nHashVal = SStrHashHT(szKey);
        T* pNode = InternalNewNode(nHashVal, nExtraBytes, dwFlags);
        pNode->m_hashval = nHashVal;
        pNode->m_key = szKey;
        return pNode;
    }
};

template<class T, class TKey>
void TSHashTable<T, TKey>::InternalDelete(T* pNode) {
    pNode->~T();
    SMemFree(pNode, "", -2, 0);
}

template<class T, class TKey>
T* TSHashTable<T, TKey>::InternalNew(List* pList, uint32_t nExtraBytes, uint32_t dwFlags) {
    return pList->NewNode(STORM_LIST_LINK_AFTER, nExtraBytes, dwFlags);
}

template<class T, class TKey>
TSHashTable<T, TKey>::~TSHashTable() {
    InternalClear(1);
}

template<class T, class TKey>
void TSHashTable<T, TKey>::Destroy() {
    InternalClear(1);
    m_fullnessIndicator = 0;
    m_slotmask = (uint32_t)-1;
    m_slotlistarray.Clear();
}

template<class T, class TKey>
void TSHashTable<T, TKey>::InternalClear(int bUnlinkOnly) {
    m_fullnessIndicator = 0;
    m_fulllist.UnlinkAll();
    for (uint32_t i = 0; i < m_slotlistarray.Count(); i++) {
        List* pList = &m_slotlistarray[i];
        T* pNode;
        while ((pNode = pList->Head()) != NULL) {
            if (bUnlinkOnly) {
                pList->UnlinkNode(pNode);
            } else {
                InternalDelete(pNode);
            }
        }
    }
}

template<class T, class TKey>
void TSHashTable<T, TKey>::Unlink(T* pNode) {
    if (pNode->m_linktoslot.m_next != NULL) {
        pNode->m_linktoslot.Unlink();
        pNode->m_linktofull.Unlink();
    }
}

template<class T, class TKey>
T* TSHashTable<T, TKey>::DeleteNode(T* pNode) {
    T* pNext = m_fulllist.Next(pNode);
    Unlink(pNode);
    InternalDelete(pNode);
    return pNext;
}

template<class T, class TKey>
void TSHashTable<T, TKey>::InitializeTableSlots() {
    m_slotmask = 3;
    m_slotlistarray.SetCount(4);
    for (uint32_t i = 0; i <= m_slotmask; i++) {
        m_slotlistarray[i].ChangeLinkOffset(offsetof(T, m_linktoslot));
    }
}

template<class T, class TKey>
int TSHashTable<T, TKey>::MonitorFullness(uint32_t nSlot) {
    if (m_slotmask >= 0x1FFF) {
        return 0;
    }
    m_fullnessIndicator = m_fullnessIndicator > 3 ? m_fullnessIndicator - 3 : 0;
    List* pList = &m_slotlistarray[nSlot];
    for (T* pNode = pList->Head(); pNode; pNode = pList->Next(pNode)) {
        if (++m_fullnessIndicator > 13) {
            m_fullnessIndicator = 0;
            GrowListArray((m_slotmask + 1) * 2);
            return 1;
        }
    }
    return 0;
}

template<class T, class TKey>
void TSHashTable<T, TKey>::GrowListArray(uint32_t nNewSize) {
    uint32_t nOldMask = m_slotmask;
    List tTempList;
    tTempList.SetLinkOffset(offsetof(T, m_linktoslot));
    if (nOldMask != (uint32_t)-1) {
        for (uint32_t i = 0; i != nOldMask + 1; i++) {
            T* pNode;
            while ((pNode = m_slotlistarray[i].Head()) != NULL) {
                tTempList.LinkNode(pNode, STORM_LIST_LINK_BEFORE, NULL);
            }
        }
    }
    m_slotlistarray.SetCount(nNewSize);
    for (uint32_t i = 0; i < nNewSize; i++) {
        m_slotlistarray[i].ChangeLinkOffset(offsetof(T, m_linktoslot));
    }
    m_slotmask = nNewSize - 1;
    T* pNode;
    while ((pNode = tTempList.Head()) != NULL) {
        m_slotlistarray[pNode->m_hashval & m_slotmask].LinkNode(pNode, STORM_LIST_LINK_BEFORE, NULL);
    }
}

template<class T, class TKey>
T* TSHashTable<T, TKey>::InternalNewNode(uint32_t nHashVal, uint32_t nExtraBytes, uint32_t dwFlags) {
    if (m_slotmask == (uint32_t)-1) {
        InitializeTableSlots();
    }
    uint32_t nSlot = nHashVal & m_slotmask;
    if (MonitorFullness(nSlot)) {
        nSlot = nHashVal & m_slotmask;
    }
    T* pNode = InternalNew(&m_slotlistarray[nSlot], nExtraBytes, dwFlags);
    m_fulllist.LinkNode(pNode, STORM_LIST_LINK_BEFORE, NULL);
    return pNode;
}

template<class T, class TKey>
T* TSHashTable<T, TKey>::Ptr(const char* szKey) {
    if (m_slotmask == (uint32_t)-1) {
        return NULL;
    }
    uint32_t nHashVal = SStrHashHT(szKey);
    List* pList = &m_slotlistarray[nHashVal & m_slotmask];
    for (T* pNode = pList->Head(); pNode; pNode = m_slotlistarray[nHashVal & m_slotmask].Next(pNode)) {
        if (pNode->m_hashval == nHashVal && pNode->m_key == szKey) {
            return pNode;
        }
    }
    return NULL;
}


#endif  // D2_OS_MAC
