// D2Server: the member functions of Storm's TSHashTable<SGAMEDATA,HASHKEY_NONE>
// and the containers inside it, as the 1.00 D2Server.dll instantiates them. No
// __FILE__ string survives; the name is the Ghidra program's.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.
//
// The objects are reached as arrays of dwords, as the template code does. A Storm list
// link is two dwords: the next node, and the previous link encoded either as a
// complemented pointer (negative) or as an offset from the node.

#pragma once

#include "Source/D2Server/D2Server.h"

namespace D2Server::GameTable {

uint32_t * __fastcall GameTable_Construct(uint32_t * pThis);                     // 1.00 D2Server: 1000a240
void __thiscall SGAMEDATA_FreeElement_A(uint32_t * pThis, uint32_t * pElement);                  // 1.00 D2Server: 1000a3c0
uint32_t * __thiscall SGAMEDATA_AllocInChunk_A(uint32_t * pThis, uint32_t * pList, int nExtra, uint32_t dwFlags); // 1.00 D2Server: 1000a3f0
void __fastcall TSHashObject_SGAMEDATA_Destructor(uint32_t * pThis);            // 1.00 D2Server: 1000a420
void __thiscall SGAMEDATA_FreeElement_B(uint32_t * pThis, uint32_t * pElement);                  // 1.00 D2Server: 1000a4e0
uint32_t * __thiscall SGAMEDATA_AllocInChunk_B(uint32_t * pThis, uint32_t * pList, int nExtra, uint32_t dwFlags); // 1.00 D2Server: 1000a510
void __fastcall GameTable_ClearAllChunks(uint32_t * pThis);                     // 1.00 D2Server: 1000a540
void __fastcall TSExplicitList_SGAMEDATA_Destructor(uint32_t * pThis);          // 1.00 D2Server: 1000a720
uint32_t * __thiscall ToMCP_ScalarDelete(uint32_t * pThis, uint8_t nFlags);     // 1.00 D2Server: 1000a830
uint32_t * __thiscall SGAMEDATA_ScalarDelete(uint32_t * pThis, uint8_t nFlags); // 1.00 D2Server: 1000a870
uint32_t * __thiscall GameTable_ScalarDelete(uint32_t * pThis, uint8_t nFlags); // 1.00 D2Server: 1000a8b0
void __fastcall GameTable_Destruct(uint32_t * pThis);                           // 1.00 D2Server: 1000a8f0
void __fastcall TSList_SGAMEDATA_UnlinkNode(uint32_t * pList);                  // 1.00 D2Server: 1000aaf0
void __fastcall TSList_SGAMEDATA_UnlinkNode_B(uint32_t * pList);                // 1.00 D2Server: 1000ab90
uint32_t * __thiscall GameTable_ClearChunks_ScalarDelete(uint32_t * pThis, uint8_t nFlags); // 1.00 D2Server: 1000ac30
void __fastcall TSExplicitList_SGAMEDATA_FreeAllBuckets(uint32_t * pThis);      // 1.00 D2Server: 1000ac70
void __fastcall SGAMEDATA_UnlinkFromBothLists(uint32_t * pElement);             // 1.00 D2Server: 1000ad50
void __fastcall SGAMEDATA_UnlinkFromBothLists_B(uint32_t * pElement);           // 1.00 D2Server: 1000ade0
void __fastcall TSList_UnlinkSingleNode(uint32_t * pLink);                      // 1.00 D2Server: 1000ae70
void __fastcall TSList_SGAMEDATA_UnlinkNode_C(uint32_t * pList);                // 1.00 D2Server: 1000aec0
uint32_t * __thiscall SGAMEDATA_Chunk_AllocElement(uint32_t * pList, int nLinkMode, int nExtra, uint32_t dwFlags); // 1.00 D2Server: 1000af60
void __fastcall TSExplicitList_SGAMEDATA_UnlinkAll(uint32_t * pThis);           // 1.00 D2Server: 1000b070
void __fastcall TSList_SGAMEDATA_UnlinkNode_D(uint32_t * pList);                // 1.00 D2Server: 1000b0e0
uint32_t * __thiscall GameTable_DestructBody_ScalarDelete(uint32_t * pThis, uint8_t nFlags); // 1.00 D2Server: 1000b180
void __fastcall GameTable_DestructBody(uint32_t * pThis);                       // 1.00 D2Server: 1000b1c0
uint32_t * __thiscall TSListNode_GetOffset(uint32_t * pLink, int nOffset);      // 1.00 D2Server: 1000b380
void __fastcall TSList_UnlinkNode(uint32_t * pLink);                            // 1.00 D2Server: 1000b3c0
uint32_t * __thiscall TSListNode_GetNodePtr(uint32_t * pList, int nNode);       // 1.00 D2Server: 1000b410
void __fastcall TSList_UnlinkAll(uint32_t * pList);                             // 1.00 D2Server: 1000b440
void __fastcall TSList_UnlinkNode_B(uint32_t * pLink);                          // 1.00 D2Server: 1000b4b0
uint32_t * __thiscall TSList_SGAMEDATA_ScalarDelete(uint32_t * pList, uint8_t nFlags); // 1.00 D2Server: 1000b500
void __fastcall SGAMEDATA_Chunk_FreeAll(uint32_t * pChunk);                     // 1.00 D2Server: 1000b540
void __fastcall TSListHead_Init_Sentinel(uint32_t * pList);                     // 1.00 D2Server: 1000b620
void __thiscall TSExplicitList_Resize_DrainOld(uint32_t * pList, int nLinkOffset); // 1.00 D2Server: 1000b650
uint32_t * __thiscall TSListNode_GetOffset_B(uint32_t * pLink, int nOffset);    // 1.00 D2Server: 1000b6e0
void __fastcall TSList_UnlinkNode_C(uint32_t * pLink);                          // 1.00 D2Server: 1000b720
int __fastcall TSList_OffsetAdd(int nOffset, int nBase);                        // 1.00 D2Server: 1000b770
void __fastcall TSListHead_Init(uint32_t * pList);                              // 1.00 D2Server: 1000b790
void __fastcall TSListNode_Zero(uint32_t * pArray);                             // 1.00 D2Server: 1000b7c0
void __fastcall TSListHead_Init_B(uint32_t * pList);                            // 1.00 D2Server: 1000b7e0
void __fastcall TSListHead_InitEmpty(uint32_t * pList);                         // 1.00 D2Server: 1000b810
void __thiscall TSExplicitList_Resize(uint32_t * pArray, uint32_t nCount);      // 1.00 D2Server: 1000b830
void __fastcall TSListHead_InitEmpty_B(uint32_t * pList);                       // 1.00 D2Server: 1000ba10
void __fastcall TSListHead_Init_Sentinel_B(uint32_t * pList);                   // 1.00 D2Server: 1000ba30
void __fastcall TSListHead_InitFrom(uint32_t * pList, uint32_t * pSource);      // 1.00 D2Server: 1000ba70

} // namespace D2Server::GameTable
