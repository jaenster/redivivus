// Storm's stpl.h templates as Diablo II instantiates them for SGAMEDATA: the members
// a game table reaches from the moment it is built until it is destroyed - built
// (TSHashTableReuse::Constructor), torn down (Destructor, the vtables' slots) and
// grown (TSHashTable::GrowListArray's SetCount and TSHashObject::Destructor).
//
// In the original every unit that used the table instantiated these members itself and
// the linker kept one copy of each COMDAT: D2Client's Game.cpp, which owns the
// single-player game table, and D2Game's Game.cpp, which runs it, both carried them, and
// Game.exe 1.14d keeps D2Client's (the 1.14d addresses below). Here the members are
// COMDAT the same way and so are the data the compiler emits
// with them (the vtables and the RTTI type names Storm tags the allocations with), so
// each module that reaches them carries its own copy and no module needs another's
// object for them.
//
// The tree spells each instantiation as an explicit specialization (stpl.h declares
// them), and an explicit specialization is an ordinary function: defined out of line
// in a header it is defined once per unit and the link fails. `inline` is what gives
// it the COMDAT linkage an implicit instantiation of the original's out-of-class
// template member had; nothing else about it changes (no member here is inlined by
// VC8 /Ob1 or by the gcc build, which the audit confirms).
//
// Moved verbatim from Source/D2Client/Game/stpl_SGAMEDATA.cpp,
// Source/D2Client/Game/Game/stpl_SGAMEDATA.cpp, Source/D2Client/Game/Game.cpp
// (the EH handler stub of TSHashTable::Destructor) and Source/globals/D2Client.cpp.
#pragma once

#include "Source/D2Headers/d2_platform.h"
#include "Source/Storm/Storm.h"
#include "Source/Storm/H/stpl.h"
#include "Source/D2Client/Game/Game/SGAMEDATA.h"

// Data every unit that names it defines, the linker keeping one copy.
#ifndef D2_SELECTANY
#if defined(_MSC_VER)
#define D2_SELECTANY __declspec(selectany)
#else
#define D2_SELECTANY __attribute__((selectany))
#endif
#endif

struct EHExceptionRecord;
struct EHRegistrationNode;

#ifndef RECON_FPTD_pfnEHHandlerRoutine
#define RECON_FPTD_pfnEHHandlerRoutine
typedef _EXCEPTION_DISPOSITION (*pfnEHHandlerRoutine)(EHExceptionRecord * pExceptionRecord, EHRegistrationNode * pRegistrationNode, _CONTEXT * pContext, void * pDispatcherContext);
#endif

namespace D2Client { namespace GAME { namespace Game {

// compiler-generated C++ EH handler; its frame is never registered with the OS
inline _EXCEPTION_DISPOSITION __GSHandlerCheck_EH_6ca026(EHExceptionRecord * pExceptionRecord, EHRegistrationNode * pRegistrationNode, _CONTEXT * pContext, void * pDispatcherContext) {
    return ExceptionContinueSearch;
}

}}} // namespace D2Client::GAME::Game

namespace D2Client { namespace GAME { namespace Game {

D2_SELECTANY char s___AUSGAMEDATA___0070f56c[16] = {
    '.',
    '?',
    'A',
    'U',
    'S',
    'G',
    'A',
    'M',
    'E',
    'D',
    'A',
    'T',
    'A',
    '@',
    '@',
    '\x00'
};

}}} // namespace D2Client::GAME::Game

namespace D2Client {

D2_SELECTANY char s___AV__TSExplicitList_USGAMEDATA__0070f584[48] = {
    '.',
    '?',
    'A',
    'V',
    '?',
    '$',
    'T',
    'S',
    'E',
    'x',
    'p',
    'l',
    'i',
    'c',
    'i',
    't',
    'L',
    'i',
    's',
    't',
    '@',
    'U',
    'S',
    'G',
    'A',
    'M',
    'E',
    'D',
    'A',
    'T',
    'A',
    '@',
    '@',
    '$',
    '0',
    '?',
    'C',
    'C',
    'C',
    'C',
    'C',
    'C',
    'C',
    'D',
    '@',
    '@',
    '@',
    '\x00'
};

} // namespace D2Client

namespace D2Client { namespace GAME { namespace Record {

D2_SELECTANY char s___AV__TSHashObjectChunk_USGAMEDA_0070f5f4[56] = {
    '.',
    '?',
    'A',
    'V',
    '?',
    '$',
    'T',
    'S',
    'H',
    'a',
    's',
    'h',
    'O',
    'b',
    'j',
    'e',
    'c',
    't',
    'C',
    'h',
    'u',
    'n',
    'k',
    '@',
    'U',
    'S',
    'G',
    'A',
    'M',
    'E',
    'D',
    'A',
    'T',
    'A',
    '@',
    '@',
    'V',
    'H',
    'A',
    'S',
    'H',
    'K',
    'E',
    'Y',
    '_',
    'N',
    'O',
    'N',
    'E',
    '@',
    '@',
    '@',
    '@',
    '\x00',
    '\x00',
    '\x00'
};

}}} // namespace D2Client::GAME::Record

template<> inline uint32_t TSGrowableArray<TSExplicitList<SGAMEDATA, -572662307>>::CalcChunkSize(uint32_t nCount) {
    if (nCount < 0x15) {
      while (nCount & nCount - 1) {
        nCount &= nCount - 1;
      }

      if (nCount < 1) {
        return 1;
      }
    } else {
      this->m_chunk = 0x15;
      nCount = 0x15;
    }

    return nCount;
}

template<> inline void TSHashObject<SGAMEDATA, HASHKEY_NONE>::Destructor() {
    TSLink<::SGAMEDATA>** pBackLink2;
    TSLink<::SGAMEDATA>* pFullLink = &this->m_linktofull;
    TSLink<::SGAMEDATA>* pPreviousLink = pFullLink->m_prevlink;
    if (pPreviousLink) {
      TSLink<::SGAMEDATA>** pBackLink;
      pBackLink = (TSLink<::SGAMEDATA>**)D2_StormNextLink(pFullLink, -1);
      *pBackLink = pPreviousLink;
      pFullLink->m_prevlink->m_next = pFullLink->m_next;
      pFullLink->m_prevlink = 0;
      pFullLink->m_next = 0;
    }

    TSLink<::SGAMEDATA>* pSlotLink = &this->m_linktoslot;
    pPreviousLink = pSlotLink->m_prevlink;
    if (!pPreviousLink) {
      return;
    }

    pBackLink2 = (TSLink<::SGAMEDATA>**)D2_StormNextLink(pSlotLink, -1);
    *pBackLink2 = pPreviousLink;
    pSlotLink->m_prevlink->m_next = pSlotLink->m_next;
    pSlotLink->m_prevlink = 0;
    pSlotLink->m_next = 0;
}

template<> inline void TSExplicitList<SGAMEDATA, -572662307>::UnlinkAll() {
    while (true) {
      intptr_t nNode = this->m_terminator.m_next > 0 ? this->m_terminator.m_next : 0;
      if (!nNode) {
        break;
      }

      TSLink<::SGAMEDATA>* pLink = (TSLink<::SGAMEDATA>*)(this->m_linkoffset + nNode);
      TSLink<::SGAMEDATA>* pPrevLink = pLink->m_prevlink;
      if (pPrevLink) {
        TSLink<::SGAMEDATA>** ppNextBack = (TSLink<::SGAMEDATA>**)D2_StormNextLink(pLink, -1);
        *ppNextBack = pPrevLink;
        pLink->m_prevlink->m_next = pLink->m_next;
        pLink->m_prevlink = 0;
        pLink->m_next = 0;
      }
    }
}

template<> inline void TSExplicitList<TSHashObjectChunk<SGAMEDATA, HASHKEY_NONE>, 16>::UnlinkAll() {
    while (true) {
      intptr_t nNode = this->m_terminator.m_next > 0 ? this->m_terminator.m_next : 0;
      if (!nNode) {
        break;
      }

      TSLink<TSHashObjectChunk<::SGAMEDATA, HASHKEY_NONE>>* pLink = (TSLink<TSHashObjectChunk<::SGAMEDATA, HASHKEY_NONE>>*)(this->m_linkoffset + nNode);
      TSLink<TSHashObjectChunk<::SGAMEDATA, HASHKEY_NONE>>* pPrevLink = pLink->m_prevlink;
      if (pPrevLink) {
        TSLink<TSHashObjectChunk<::SGAMEDATA, HASHKEY_NONE>>** ppNextBack = (TSLink<TSHashObjectChunk<::SGAMEDATA, HASHKEY_NONE>>**)D2_StormNextLink(pLink, -1);
        *ppNextBack = pPrevLink;
        pLink->m_prevlink->m_next = pLink->m_next;
        pLink->m_prevlink = 0;
        pLink->m_next = 0;
      }
    }
}

template<> inline void TSHashTable<SGAMEDATA, HASHKEY_NONE>::InternalClear(int32_t bUnlinkOnly) {
    this->nFullnessIndicator = 0;
    this->tFullList.UnlinkAll();
    for (uint32_t i = 0; i < this->tSlotListArray.m_count; i++) {
      while (true) {
        TSExplicitList<::SGAMEDATA, -572662307>* pList = &this->tSlotListArray.m_data[i];
        ::SGAMEDATA* pNode = pList->m_terminator.m_next > 0 ? (::SGAMEDATA*)pList->m_terminator.m_next : nullptr;
        if (!pNode) {
          break;
        }
        if (bUnlinkOnly) {
          TSLink<::SGAMEDATA>* pLink = (TSLink<::SGAMEDATA>*)((intptr_t)pNode + pList->m_linkoffset);
          TSLink<::SGAMEDATA>* pPrevLink = pLink->m_prevlink;
          if (pPrevLink) {
            TSLink<::SGAMEDATA>** ppNextBack = (TSLink<::SGAMEDATA>**)D2_StormNextLink(pLink, -1);
            *ppNextBack = pPrevLink;
            pLink->m_prevlink->m_next = pLink->m_next;
            pLink->m_prevlink = 0;
            pLink->m_next = 0;
          }
        } else {
          vc8_thiscall(this->pVtable->InternalDelete)(this, pNode);
        }
      }
    }
}

template<> inline void TSExplicitList<SGAMEDATA, -572662307>::Clear() {
    while (true) {
      ::SGAMEDATA* pObject = (intptr_t)this->m_terminator.m_next > 0 ? (::SGAMEDATA*)this->m_terminator.m_next : nullptr;
      if (!pObject) {
        break;
      }

      TSLink<::SGAMEDATA>* pLink = &pObject->m_linktofull;
      TSLink<::SGAMEDATA>* pPrev = pLink->m_prevlink;
      if (pPrev) {
        *(TSLink<::SGAMEDATA>**)D2_StormNextLink(pLink, -1) = pPrev;
        pLink->m_prevlink->m_next = pLink->m_next;
        pLink->m_prevlink = 0;
        pLink->m_next = 0;
      }

      pLink = &pObject->m_linktoslot;
      pPrev = pLink->m_prevlink;
      if (pPrev) {
        *(TSLink<::SGAMEDATA>**)D2_StormNextLink(pLink, -1) = pPrev;
        pLink->m_prevlink->m_next = pLink->m_next;
        pLink->m_prevlink = 0;
        pLink->m_next = 0;
      }

      Storm::Source::SMem::SMemFree(pObject, D2Client::GAME::Game::s___AUSGAMEDATA___0070f56c, -2, 0);
    }
}

template<> inline void TSGrowableArray<TSExplicitList<SGAMEDATA, -572662307>>::ReallocData(uint32_t nAlloc) {
    TSExplicitList<::SGAMEDATA, -572662307>* pNewBuf;
    TSExplicitList<::SGAMEDATA, -572662307>* puVar1; // a slot list sits one pointer-sized word (its int link offset, padded) before its terminator
    TSLink<::SGAMEDATA>** pPrevLink2;
    TSLink<::SGAMEDATA>* pTVar2;
    intptr_t encPrev; // steer-ok: generated form, kept as emitted
    int nNewCapSaved;
    TSLink<::SGAMEDATA>* pNewTerminator;
    TSLink<::SGAMEDATA>* pNext;
    uint32_t nAllocCapacity = nAlloc;
    TSExplicitList<::SGAMEDATA, -572662307>* pBlock = this->m_data;
    if (nAlloc < this->m_count) {
      TSLink<::SGAMEDATA>** pPrevLink;
      pNewBuf = pBlock + nAlloc;
      do {
        pTVar2 = &pNewBuf->m_terminator;
        ((TSExplicitList<::SGAMEDATA, -572662307>*)((intptr_t*)pTVar2 - 1))->UnlinkAll();
        pNext = pTVar2->m_prevlink;
        if (pNext) {
          encPrev = pTVar2->m_next;
          pPrevLink = (TSLink<::SGAMEDATA>**)D2_StormNextLink(pTVar2, -1);
          *pPrevLink = pNext;
          pTVar2->m_prevlink->m_next = pTVar2->m_next;
          pTVar2->m_prevlink = 0;
          pTVar2->m_next = 0;
        }

        nAlloc++;
        pNewBuf = (TSExplicitList<::SGAMEDATA, -572662307>*)(pTVar2 + 1);
      } while (nAlloc < this->m_count);
    }

    this->m_alloc = nAllocCapacity;
    pNewBuf = (TSExplicitList<::SGAMEDATA, -572662307>*)Storm::Source::SMem::SMemReAlloc((uint32_t*)pBlock, nAllocCapacity * sizeof(TSExplicitList<::SGAMEDATA, -572662307>), D2Client::s___AV__TSExplicitList_USGAMEDATA__0070f584, -2, 0x10);
    this->m_data = pNewBuf;
    if (pNewBuf) {
      return;
    }

    pNewBuf = (TSExplicitList<::SGAMEDATA, -572662307>*)Storm::Source::SMem::SMemAlloc(nAllocCapacity * sizeof(TSExplicitList<::SGAMEDATA, -572662307>), D2Client::s___AV__TSExplicitList_USGAMEDATA__0070f584, -2, 0);
    this->m_data = pNewBuf;
    if (!pBlock) {
      return;
    }

    nAlloc = this->m_count;
    if (nAllocCapacity < this->m_count) {
      nAlloc = nAllocCapacity;
    }
    if (nAlloc > 0) {
      pNewBuf = pBlock;
      do {
        pNext = &pNewBuf->m_terminator;
        puVar1 = (TSExplicitList<::SGAMEDATA, -572662307>*)(-(intptr_t)sizeof(intptr_t) - (intptr_t)pBlock + (intptr_t)pNext + (intptr_t)this->m_data);
        if (puVar1) {
          pNewTerminator = &puVar1->m_terminator;
          pNewTerminator->m_prevlink = 0;
          puVar1->m_terminator.m_next = 0;
          nNewCapSaved = ((TSExplicitList<::SGAMEDATA, -572662307>*)((intptr_t*)pNext - 1))->m_linkoffset;
          pNewTerminator->m_prevlink = pNewTerminator;
          puVar1->m_linkoffset = nNewCapSaved;
          puVar1->m_terminator.m_next = ~(intptr_t)pNewTerminator;
        }

        ((TSExplicitList<::SGAMEDATA, -572662307>*)((intptr_t*)pNext - 1))->UnlinkAll();
        pTVar2 = pNext->m_prevlink;
        if (pTVar2) {
          encPrev = pNext->m_next;
          pPrevLink2 = (TSLink<::SGAMEDATA>**)D2_StormNextLink(pNext, -1);
          *pPrevLink2 = pTVar2;
          pNext->m_prevlink->m_next = pNext->m_next;
          pNext->m_prevlink = 0;
          pNext->m_next = 0;
        }

        pNewBuf = (TSExplicitList<::SGAMEDATA, -572662307>*)(pNext + 1);
        nAlloc--;
      } while (nAlloc);
    }

    Storm::Source::SMem::SMemFree(pBlock, D2Client::s___AV__TSExplicitList_USGAMEDATA__0070f584, -2, 0);
}

template<> inline void TSGrowableArray<TSExplicitList<SGAMEDATA, -572662307>>::Destructor() {
    TSExplicitList<::SGAMEDATA, -572662307>* pSlot;
    TSLink<::SGAMEDATA>* pNext;
    uint32_t nIndex = 0;
    if (this->m_count > 0) {
      TSLink<::SGAMEDATA>* pTVar1;
      do {
        pSlot = &this->m_data[nIndex];
        pSlot->UnlinkAll();
        pTVar1 = &pSlot->m_terminator;
        pNext = pTVar1->m_prevlink;
        if (pNext) {
          *(TSLink<::SGAMEDATA>**)D2_StormNextLink(pTVar1, -1) = pNext;
          pTVar1->m_prevlink->m_next = pTVar1->m_next;
          pTVar1->m_prevlink = 0;
          pTVar1->m_next = 0;
        }

        nIndex++;
      } while (nIndex < this->m_count);
    }
    if (!this->m_data) {
      return;
    }

    Storm::Source::SMem::SMemFree(this->m_data, D2Client::s___AV__TSExplicitList_USGAMEDATA__0070f584, -2, 0);
}

template<> inline void TSGrowableArray<TSExplicitList<SGAMEDATA, -572662307>>::SetCount(uint32_t nCount) {
    if (nCount > this->m_count) {
      uint32_t nGranularity;
      if (nCount > this->m_alloc) {
        nGranularity = this->m_chunk;
        if (!nGranularity) {
          nGranularity = this->CalcChunkSize(nCount);
        }

        uint32_t nAligned = nCount;
        if (nCount % nGranularity) {
          nAligned = nGranularity - nCount % nGranularity + nCount;
        }

        this->ReallocData(nAligned);
      }

      const uint32_t nOldCount = this->m_count;
      if (nOldCount < nCount) {
        int nByteOffset = nOldCount * sizeof(*this->m_data);
        int nRemaining = nCount - nOldCount;
        do {
          TSExplicitList<::SGAMEDATA, -572662307>* pInitSlot = reinterpret_cast<TSExplicitList<::SGAMEDATA, -572662307>*>(reinterpret_cast<char*>(this->m_data) + nByteOffset);
          if (pInitSlot) {
            TSLink<::SGAMEDATA>* pTerminator = &pInitSlot->m_terminator;
            pTerminator->m_prevlink = 0;
            pTerminator->m_next = 0;
            pInitSlot->m_terminator.m_prevlink = pTerminator;
            pInitSlot->m_linkoffset = -0x22222223;
            pInitSlot->m_terminator.m_next = ~reinterpret_cast<intptr_t>(pTerminator);
          }

          nByteOffset += sizeof(*this->m_data);
          nRemaining--;
        } while (nRemaining);
      }
    } else if (nCount < this->m_count) {
      uint32_t nIndex = nCount;
      int nByteOffset = nCount * sizeof(*this->m_data);
      do {
        TSExplicitList<::SGAMEDATA, -572662307>* pSlotI = reinterpret_cast<TSExplicitList<::SGAMEDATA, -572662307>*>(reinterpret_cast<char*>(this->m_data) + nByteOffset);
        pSlotI->UnlinkAll();
        TSLink<::SGAMEDATA>* pTerminator = &pSlotI->m_terminator;
        TSLink<::SGAMEDATA>* pPrevious = pTerminator->m_prevlink;
        if (pPrevious) {
          TSLink<::SGAMEDATA>** pBackLink = reinterpret_cast<TSLink<::SGAMEDATA>**>(D2_StormNextLink(pTerminator, -1));
          *pBackLink = pPrevious;
          pTerminator->m_prevlink->m_next = pTerminator->m_next;
          pTerminator->m_prevlink = 0;
          pTerminator->m_next = 0;
        }

        nIndex++;
        nByteOffset += sizeof(*this->m_data);
      } while (nIndex < this->m_count);

      this->m_count = nCount;
      return;
    }

    this->m_count = nCount;
}

template<> inline void TSHashTable<SGAMEDATA, HASHKEY_NONE>::Destructor() {
    pfnEHHandlerRoutine pSehHandler = D2Client::GAME::Game::__GSHandlerCheck_EH_6ca026;
    void* pSehFrame = ExceptionList;
    ExceptionList = &pSehFrame;
    this->pVtable = (TSHashTable<::SGAMEDATA, HASHKEY_NONE>::Vtable*)(void*)TSHashTable<::SGAMEDATA, HASHKEY_NONE>::vftable;
    uint32_t nEHState = 1;
    this->InternalClear(1);
    this->tSlotListArray.Destructor();
    this->tFullList.UnlinkAll();
    TSLink<::SGAMEDATA>* pNext = this->tFullList.m_terminator.m_prevlink;
    TSLink<::SGAMEDATA>* pTVar1 = &this->tFullList.m_terminator;
    if (pNext) {
      TSLink<::SGAMEDATA>** pPrevLink;
      pPrevLink = (TSLink<::SGAMEDATA>**)D2_StormNextLink(pTVar1, -1);
      *pPrevLink = pNext;
      pTVar1->m_prevlink->m_next = pTVar1->m_next;
      pTVar1->m_prevlink = 0;
      pTVar1->m_next = 0;
    }

    ExceptionList = pSehFrame;
}

// @blizzard TSHashTable<SGAMEDATA,HASHKEY_NONE>::InternalDelete (STORM.H)
template<> inline void __fastcall TSHashTable<SGAMEDATA, HASHKEY_NONE>::InternalDelete(TSHashTable<::SGAMEDATA, HASHKEY_NONE>* pThis, int, ::SGAMEDATA * pNode) {
    pNode->Destructor();
    Storm::Source::SMem::SMemFree(pNode, D2Client::GAME::Game::s___AUSGAMEDATA___0070f56c, -2, 0);
}

// @blizzard TSHashTable<SGAMEDATA,HASHKEY_NONE>::InternalNew (STORM.H)
template<> inline ::SGAMEDATA * __fastcall TSHashTable<SGAMEDATA, HASHKEY_NONE>::InternalNew(TSHashTable<::SGAMEDATA, HASHKEY_NONE>* pThis, int, TSExplicitList<::SGAMEDATA, -572662307>* pList, uint32_t nExtraBytes, uint32_t dwFlags) {
    ::SGAMEDATA* pAlloc = (::SGAMEDATA*)Storm::Source::SMem::SMemAlloc(nExtraBytes + sizeof(::SGAMEDATA), D2Client::GAME::Game::s___AUSGAMEDATA___0070f56c, -2, dwFlags | 8);
    ::SGAMEDATA* pNewEntry;
    if (pAlloc) {
      pAlloc->m_linktoslot.m_prevlink = 0;
      pAlloc->m_linktoslot.m_next = 0;
      pAlloc->m_linktofull.m_prevlink = 0;
      pAlloc->m_linktofull.m_next = 0;
      pNewEntry = pAlloc;
    } else {
      pNewEntry = 0;
    }

    TSLink<::SGAMEDATA>* pLink = pNewEntry ? (TSLink<::SGAMEDATA>*)((intptr_t)pNewEntry + pList->m_linkoffset) : &pList->m_terminator;

    if (pLink->m_prevlink) {
      *(TSLink<::SGAMEDATA>**)D2_StormNextLink(pLink, -1) = pLink->m_prevlink;
      pLink->m_prevlink->m_next = pLink->m_next;
      pLink->m_prevlink = 0;
      pLink->m_next = 0;
    }

    TSLink<::SGAMEDATA>* pTerminator = &pList->m_terminator;
    pLink->m_prevlink = pTerminator;
    pLink->m_next = pTerminator->m_next;
    *(TSLink<::SGAMEDATA>**)D2_StormNextLink(pTerminator, pList->m_linkoffset) = pLink;
    pTerminator->m_next = (intptr_t)pNewEntry;
    return pNewEntry;
}

// @blizzard TSHashTable<SGAMEDATA,HASHKEY_NONE>::Destroy (STORM.H)
template<> inline void __fastcall TSHashTable<SGAMEDATA, HASHKEY_NONE>::Destroy(TSHashTable<::SGAMEDATA, HASHKEY_NONE>* pThis, int) {
    pThis->InternalClear(1);
    pThis->nFullnessIndicator = 0;
    pThis->nSlotMask = 0;
    TSGrowableArray<TSExplicitList<SGAMEDATA, -572662307>>* pArray = &pThis->tSlotListArray;
    pArray->Destructor();
    pArray->m_alloc = 0;
    pArray->m_count = 0;
    pArray->m_data = 0;
}

// @blizzard TSHashTable<SGAMEDATA,HASHKEY_NONE> scalar deleting destructor (STORM.H)
template<> inline TSHashTable<::SGAMEDATA, HASHKEY_NONE>* __fastcall TSHashTable<SGAMEDATA, HASHKEY_NONE>::ScalarDeletingDestructor(TSHashTable<::SGAMEDATA, HASHKEY_NONE>* pThis, int, uint32_t dwFlags) {
    pThis->Destructor();
    if (dwFlags & 1 && pThis) {
      Storm::Source::SMem::SMemFree(pThis, "delete", -1, 0);
    }

    return pThis;
}

template<> inline void TSExplicitList<TSHashObjectChunk<SGAMEDATA, HASHKEY_NONE>, 16>::Clear() {
    TSHashObjectChunk<::SGAMEDATA, HASHKEY_NONE>* pChunk;
    while ((pChunk = (intptr_t)this->m_terminator.m_next > 0 ? (TSHashObjectChunk<::SGAMEDATA, HASHKEY_NONE>*)this->m_terminator.m_next : 0) != 0) {

      TSLink<TSHashObjectChunk<::SGAMEDATA, HASHKEY_NONE>>* pLink = &pChunk->m_link;
      if (pLink->m_prevlink) {
        *(TSLink<TSHashObjectChunk<::SGAMEDATA, HASHKEY_NONE>>**)D2_StormNextLink(pLink, -1) = pLink->m_prevlink;
        pLink->m_prevlink->m_next = pLink->m_next;
        pLink->m_prevlink = 0;
        pLink->m_next = 0;
      }

      for (uint32_t nIndex = 0; nIndex < pChunk->nArrayCount; nIndex++) {
        ::SGAMEDATA* pEntry = &pChunk->pArray[nIndex];
        TSLink<::SGAMEDATA>* pEntryLink = &pEntry->m_linktofull;
        if (pEntryLink->m_prevlink) {
          *(TSLink<::SGAMEDATA>**)D2_StormNextLink(pEntryLink, -1) = pEntryLink->m_prevlink;
          pEntryLink->m_prevlink->m_next = pEntryLink->m_next;
          pEntryLink->m_prevlink = 0;
          pEntryLink->m_next = 0;
        }

        pEntryLink = &pEntry->m_linktoslot;
        if (pEntryLink->m_prevlink) {
          *(TSLink<::SGAMEDATA>**)D2_StormNextLink(pEntryLink, -1) = pEntryLink->m_prevlink;
          pEntryLink->m_prevlink->m_next = pEntryLink->m_next;
          pEntryLink->m_prevlink = 0;
          pEntryLink->m_next = 0;
        }
      }

      if (pChunk->pArray) {
        Storm::Source::SMem::SMemFree(pChunk->pArray, D2Client::GAME::Game::s___AUSGAMEDATA___0070f56c, -2, 0);
      }

      Storm::Source::SMem::SMemFree(pChunk, D2Client::GAME::Record::s___AV__TSHashObjectChunk_USGAMEDA_0070f5f4, -2, 0);
    }
}

template<> inline TSHashTable<::SGAMEDATA, HASHKEY_NONE>* TSHashTable<SGAMEDATA, HASHKEY_NONE>::Constructor() {
    this->pVtable = (TSHashTable<::SGAMEDATA, HASHKEY_NONE>::Vtable*)(void*)TSHashTable<::SGAMEDATA, HASHKEY_NONE>::vftable;
    TSExplicitList<::SGAMEDATA, -572662307>* pBucket = &this->tFullList;
    this->tFullList.m_terminator.m_next = 0;
    TSLink<::SGAMEDATA>* pListHead = &this->tFullList.m_terminator;
    pListHead->m_prevlink = pListHead;
    pBucket->m_linkoffset = -0x22222223;
    this->tFullList.m_terminator.m_next = ~(uintptr_t)pListHead;
    this->tSlotListArray.m_alloc = 0;
    this->tSlotListArray.m_count = 0;
    this->tSlotListArray.m_data = 0;
    this->tSlotListArray.m_chunk = 0;
    this->nFullnessIndicator = 0;
    if (pBucket->m_linkoffset != offsetof(::SGAMEDATA, m_linktofull)) {
      pBucket->UnlinkAll();
      pListHead = &this->tFullList.m_terminator;
      pListHead->m_prevlink = pListHead;
      pBucket->m_linkoffset = offsetof(::SGAMEDATA, m_linktofull);
      this->tFullList.m_terminator.m_next = ~(uintptr_t)pListHead;
    }

    this->nSlotMask = 3;
    this->tSlotListArray.SetCount(4);
    uint32_t nBucketIdx = 0;
    do {
      pBucket = &this->tSlotListArray.m_data[nBucketIdx];
      if (pBucket->m_linkoffset != offsetof(::SGAMEDATA, m_linktoslot)) {
        pBucket->UnlinkAll();
        pListHead = &pBucket->m_terminator;
        pListHead->m_prevlink = pListHead;
        pBucket->m_linkoffset = offsetof(::SGAMEDATA, m_linktoslot);
        pBucket->m_terminator.m_next = ~(uintptr_t)pListHead;
      }

      nBucketIdx++;
    } while (nBucketIdx <= this->nSlotMask);

    return this;
}

template<> inline void TSHashTableReuse<SGAMEDATA, HASHKEY_NONE, 0>::Destructor() {
    TSLink<TSHashObjectChunk<::SGAMEDATA, HASHKEY_NONE>>** pPrevLink;
    this->pVtable = (TSHashTable<::SGAMEDATA, HASHKEY_NONE>::Vtable*)(void*)TSHashTableReuse<::SGAMEDATA, HASHKEY_NONE, 0>::vftable;
    this->tChunkList.Clear();
    this->tReuseList.Clear();
    this->tChunkList.UnlinkAll();
    TSLink<TSHashObjectChunk<::SGAMEDATA, HASHKEY_NONE>>* pNode = this->tChunkList.m_terminator.m_prevlink;
    TSLink<TSHashObjectChunk<::SGAMEDATA, HASHKEY_NONE>>* pTVar1 = &this->tChunkList.m_terminator;
    if (pNode) {
      pPrevLink = (TSLink<TSHashObjectChunk<::SGAMEDATA, HASHKEY_NONE>>**)D2_StormNextLink(pTVar1, -1);
      *pPrevLink = pNode;
      pTVar1->m_prevlink->m_next = pTVar1->m_next;
      pTVar1->m_prevlink = 0;
      pTVar1->m_next = 0;
    }

    this->tReuseList.UnlinkAll();
    TSLink<::SGAMEDATA>* pTVar3 = this->tReuseList.m_terminator.m_prevlink;
    TSLink<::SGAMEDATA>* pTVar2 = &this->tReuseList.m_terminator;
    if (pTVar3) {
      TSLink<::SGAMEDATA>** pPrevLink2;
      pPrevLink2 = (TSLink<::SGAMEDATA>**)D2_StormNextLink(pTVar2, -1);
      *pPrevLink2 = pTVar3;
      pTVar2->m_prevlink->m_next = pTVar2->m_next;
      pTVar2->m_prevlink = 0;
      pTVar2->m_next = 0;
    }

    this->TSHashTable<::SGAMEDATA, HASHKEY_NONE>::Destructor();
}

// @blizzard TSHashTableReuse<SGAMEDATA,HASHKEY_NONE,0>::InternalDelete (STORM.H)
template<> inline void __fastcall TSHashTableReuse<SGAMEDATA, HASHKEY_NONE, 0>::InternalDelete(TSHashTableReuse<::SGAMEDATA, HASHKEY_NONE, 0>* pThis, int, ::SGAMEDATA * pNode) {
    pNode->Destructor();
    Storm::Source::SMem::SMemFree(pNode, D2Client::GAME::Game::s___AUSGAMEDATA___0070f56c, -2, 0);
}

// @blizzard TSHashTableReuse<SGAMEDATA,HASHKEY_NONE,0>::InternalNew (STORM.H)
template<> inline ::SGAMEDATA * __fastcall TSHashTableReuse<SGAMEDATA, HASHKEY_NONE, 0>::InternalNew(TSHashTableReuse<::SGAMEDATA, HASHKEY_NONE, 0>* pThis, int, TSExplicitList<::SGAMEDATA, -572662307>* pList, uint32_t nExtraBytes, uint32_t dwFlags) {
    TSLink<::SGAMEDATA>* ppSlot;
    ::SGAMEDATA* pGame = (::SGAMEDATA*)Storm::Source::SMem::SMemAlloc(nExtraBytes + sizeof(::SGAMEDATA), D2Client::GAME::Game::s___AUSGAMEDATA___0070f56c, -2, dwFlags | 8);
    if (pGame) {
      pGame->m_linktoslot.m_prevlink = 0;
      pGame->m_linktoslot.m_next = 0;
      pGame->m_linktofull.m_prevlink = 0;
      pGame->m_linktofull.m_next = 0;
#if defined(_WIN64)
      ppSlot = (TSLink<::SGAMEDATA>*)((char*)pGame + pList->m_linkoffset);
#else
      ppSlot = (TSLink<::SGAMEDATA>*)(pGame->szGameName + (pList->m_linkoffset - 0x2a));
#endif
    } else {
      pGame = 0;
      ppSlot = &pList->m_terminator;
    }

    TSLink<::SGAMEDATA>* pExisting = ppSlot->m_prevlink;
    if (pExisting) {
      char* pPrevLink;
      pPrevLink = (char*)D2_StormNextLink(ppSlot, -1);
      *(TSLink<::SGAMEDATA>**)pPrevLink = (TSLink<::SGAMEDATA>*)pExisting;
      ppSlot->m_prevlink->m_next = ppSlot->m_next;
      ppSlot->m_prevlink = 0;
      ppSlot->m_next = 0;
    }

    pExisting = &pList->m_terminator;
    ppSlot->m_prevlink = pExisting;
    ppSlot->m_next = pList->m_terminator.m_next;
    intptr_t pTailLink2 = pList->m_terminator.m_next;
    intptr_t pHeadOffset = pList->m_linkoffset;
    if (pTailLink2 < 0) {
      *(TSLink<::SGAMEDATA>**)~pTailLink2 = (TSLink<::SGAMEDATA>*)ppSlot;
    } else {
      if (pHeadOffset < 0) {
        pHeadOffset = (intptr_t)pExisting - pExisting->m_prevlink->m_next;
      }

      *(TSLink<::SGAMEDATA>**)(pTailLink2 + pHeadOffset) = (TSLink<::SGAMEDATA>*)ppSlot;
    }

    pExisting->m_next = (intptr_t)pGame;
    return pGame;
}

// @blizzard TSHashTableReuse<SGAMEDATA,HASHKEY_NONE,0>::Destroy (STORM.H)
template<> inline void __fastcall TSHashTableReuse<SGAMEDATA, HASHKEY_NONE, 0>::Destroy(TSHashTableReuse<::SGAMEDATA, HASHKEY_NONE, 0>* pThis, int) {
    pThis->InternalClear(0);
    pThis->tChunkList.Clear();
    pThis->tReuseList.Clear();
}

// @blizzard TSHashTableReuse<SGAMEDATA,HASHKEY_NONE,0> scalar deleting destructor (STORM.H)
template<> inline TSHashTableReuse<::SGAMEDATA, HASHKEY_NONE, 0>* __fastcall TSHashTableReuse<SGAMEDATA, HASHKEY_NONE, 0>::ScalarDeletingDestructor(TSHashTableReuse<::SGAMEDATA, HASHKEY_NONE, 0>* pThis, int, uint32_t dwFlags) {
    pThis->Destructor();
    if (dwFlags & 1 && pThis) {
      Storm::Source::SMem::SMemFree(pThis, "delete", -1, 0);
    }

    return pThis;
}

template<> inline TSHashTableReuse<::SGAMEDATA, HASHKEY_NONE, 0>* TSHashTableReuse<SGAMEDATA, HASHKEY_NONE, 0>::Constructor() {
    this->TSHashTable<::SGAMEDATA, HASHKEY_NONE>::Constructor();
    this->pVtable = (TSHashTable<::SGAMEDATA, HASHKEY_NONE>::Vtable*)(void*)TSHashTableReuse<::SGAMEDATA, HASHKEY_NONE, 0>::vftable;
    TSExplicitList<::SGAMEDATA, -572662307>* pReuseList = &this->tReuseList;
    this->tReuseList.m_terminator.m_next = 0;
    TSLink<::SGAMEDATA>* pListHead = &this->tReuseList.m_terminator;
    pListHead->m_prevlink = pListHead;
    this->tReuseList.m_terminator.m_next = ~(uintptr_t)pListHead;
    pReuseList->m_linkoffset = -0x22222223;
    this->tChunkList.m_terminator.m_next = 0;
    TSLink<TSHashObjectChunk<::SGAMEDATA, HASHKEY_NONE>>* pTVar1 = &this->tChunkList.m_terminator;
    pTVar1->m_prevlink = pTVar1;
    this->tChunkList.m_linkoffset = offsetof(SGAMEDATAChunk, m_link);
    this->tChunkList.m_terminator.m_next = ~(uintptr_t)pTVar1;
    this->nChunkSize = 0x10;
    if (pReuseList->m_linkoffset != offsetof(::SGAMEDATA, m_linktoslot)) {
      pReuseList->UnlinkAll();
      pListHead = &this->tReuseList.m_terminator;
      pListHead->m_prevlink = pListHead;
      pReuseList->m_linkoffset = offsetof(::SGAMEDATA, m_linktoslot);
      this->tReuseList.m_terminator.m_next = ~(uintptr_t)pListHead;
    }

    return this;
}

// @blizzard TSExportTableSync<SGAMEDATA,HSGAMEDATA__*,HSGAMEDATALOCK__*,CCritSect,0> scalar deleting destructor (STORM.H)
template<> inline TSExportTableSync<::SGAMEDATA, HSGAMEDATA__*, HSGAMEDATALOCK__*, CCritSect, 0>* __fastcall TSExportTableSync<SGAMEDATA, HSGAMEDATA__*, HSGAMEDATALOCK__*, CCritSect, 0>::ScalarDeletingDestructor(TSExportTableSync<::SGAMEDATA, HSGAMEDATA__*, HSGAMEDATALOCK__*, CCritSect, 0>* pThis, int, uint32_t dwFlags) {
    /* LPCRITICAL_SECTION lpCriticalSection for DeleteCriticalSection */ DeleteCriticalSection(&pThis->tSync.m_critsect);
    pThis->TSHashTableReuse<::SGAMEDATA, HASHKEY_NONE, 0>::Destructor();
    if (dwFlags & 1 && pThis) {
      Storm::Source::SMem::SMemFree(pThis, "delete", -1, 0);
    }

    return pThis;
}

// Static data members

template<> D2_SELECTANY void * TSHashTable<SGAMEDATA, HASHKEY_NONE>::vftable[4] = { (void*)&TSHashTable<SGAMEDATA, HASHKEY_NONE>::InternalDelete, (void*)&TSHashTable<SGAMEDATA, HASHKEY_NONE>::InternalNew, (void*)&TSHashTable<SGAMEDATA, HASHKEY_NONE>::ScalarDeletingDestructor, (void*)&TSHashTable<SGAMEDATA, HASHKEY_NONE>::Destroy };

template<> D2_SELECTANY void * TSExportTableSync<SGAMEDATA, HSGAMEDATA__*, HSGAMEDATALOCK__*, CCritSect, 0>::vftable[4] = { (void*)&TSHashTableReuse<SGAMEDATA, HASHKEY_NONE, 0>::InternalDelete, (void*)&TSHashTableReuse<SGAMEDATA, HASHKEY_NONE, 0>::InternalNew, (void*)&TSExportTableSync<SGAMEDATA, HSGAMEDATA__*, HSGAMEDATALOCK__*, CCritSect, 0>::ScalarDeletingDestructor, (void*)&TSHashTableReuse<SGAMEDATA, HASHKEY_NONE, 0>::Destroy };

template<> D2_SELECTANY void * TSHashTableReuse<SGAMEDATA, HASHKEY_NONE, 0>::vftable[4] = { (void*)&TSHashTableReuse<SGAMEDATA, HASHKEY_NONE, 0>::InternalDelete, (void*)&TSHashTableReuse<SGAMEDATA, HASHKEY_NONE, 0>::InternalNew, (void*)&TSHashTableReuse<SGAMEDATA, HASHKEY_NONE, 0>::ScalarDeletingDestructor, (void*)&TSHashTableReuse<SGAMEDATA, HASHKEY_NONE, 0>::Destroy };
