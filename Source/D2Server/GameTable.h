// D2Server: the member functions of Storm's TSHashTable<SGAMEDATA,HASHKEY_NONE>
// and the containers inside it, as the 1.00 D2Server.dll instantiates them. No
// __FILE__ string survives; the name is the Ghidra program's.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.
//
// The types are in D2Server.h. A Storm list link is the next link and the previous
// one, encoded either as a complemented pointer (negative) or as the previous element.

#pragma once

#include "Source/D2Server/D2Server.h"

namespace D2Server::GameTable {

GameDataTable * __fastcall GameTable_Construct(GameDataTable * pThis);                     // 1.00 D2Server: 1000a240
void __thiscall SGAMEDATA_FreeElement_A(GameDataTable * pThis, SGAMEDATA * pElement);                  // 1.00 D2Server: 1000a3c0
SGAMEDATA * __thiscall SGAMEDATA_AllocInChunk_A(GameDataTable * pThis, GameTableList * pList, int nExtra, uint32_t dwFlags); // 1.00 D2Server: 1000a3f0
void __fastcall TSHashObject_SGAMEDATA_Destructor(GameDataTable * pThis);            // 1.00 D2Server: 1000a420
void __thiscall SGAMEDATA_FreeElement_B(GameDataTable * pThis, SGAMEDATA * pElement);                  // 1.00 D2Server: 1000a4e0
SGAMEDATA * __thiscall SGAMEDATA_AllocInChunk_B(GameDataTable * pThis, GameTableList * pList, int nExtra, uint32_t dwFlags); // 1.00 D2Server: 1000a510
void __fastcall GameTable_ClearAllChunks(GameDataTable * pThis);                     // 1.00 D2Server: 1000a540
void __fastcall TSExplicitList_SGAMEDATA_Destructor(GameDataTable * pThis);          // 1.00 D2Server: 1000a720
GameDataTable * __thiscall ToMCP_ScalarDelete(GameDataTable * pThis, uint8_t nFlags);     // 1.00 D2Server: 1000a830
SGAMEDATA * __thiscall SGAMEDATA_ScalarDelete(SGAMEDATA * pThis, uint8_t nFlags); // 1.00 D2Server: 1000a870
GameDataTable * __thiscall GameTable_ScalarDelete(GameDataTable * pThis, uint8_t nFlags); // 1.00 D2Server: 1000a8b0
void __fastcall GameTable_Destruct(GameDataTable * pThis);                           // 1.00 D2Server: 1000a8f0
void __fastcall TSList_SGAMEDATA_UnlinkNode(GameTableList * pList);                  // 1.00 D2Server: 1000aaf0
void __fastcall TSList_SGAMEDATA_UnlinkNode_B(GameTableList * pList);                // 1.00 D2Server: 1000ab90
GameDataTable * __thiscall GameTable_ClearChunks_ScalarDelete(GameDataTable * pThis, uint8_t nFlags); // 1.00 D2Server: 1000ac30
void __fastcall TSExplicitList_SGAMEDATA_FreeAllBuckets(GameTableListArray * pThis);      // 1.00 D2Server: 1000ac70
void __fastcall SGAMEDATA_UnlinkFromBothLists(SGAMEDATA * pElement);             // 1.00 D2Server: 1000ad50
void __fastcall SGAMEDATA_UnlinkFromBothLists_B(SGAMEDATA * pElement);           // 1.00 D2Server: 1000ade0
void __fastcall TSList_UnlinkSingleNode(GameTableLink * pLink);                      // 1.00 D2Server: 1000ae70
void __fastcall TSList_SGAMEDATA_UnlinkNode_C(GameTableList * pList);                // 1.00 D2Server: 1000aec0
SGAMEDATA * __thiscall SGAMEDATA_Chunk_AllocElement(GameTableList * pList, int nLinkMode, int nExtra, uint32_t dwFlags); // 1.00 D2Server: 1000af60
void __fastcall TSExplicitList_SGAMEDATA_UnlinkAll(GameTableListArray * pThis);           // 1.00 D2Server: 1000b070
void __fastcall TSList_SGAMEDATA_UnlinkNode_D(GameTableList * pList);                // 1.00 D2Server: 1000b0e0
GameDataTable * __thiscall GameTable_DestructBody_ScalarDelete(GameDataTable * pThis, uint8_t nFlags); // 1.00 D2Server: 1000b180
void __fastcall GameTable_DestructBody(GameDataTable * pThis);                       // 1.00 D2Server: 1000b1c0
GameTableLink * __thiscall TSListNode_GetOffset(GameTableLink * pLink, intptr_t nOffset);      // 1.00 D2Server: 1000b380
void __fastcall TSList_UnlinkNode(GameTableLink * pLink);                            // 1.00 D2Server: 1000b3c0
GameTableLink * __thiscall TSListNode_GetNodePtr(GameTableList * pList, intptr_t nNode);       // 1.00 D2Server: 1000b410
void __fastcall TSList_UnlinkAll(GameTableList * pList);                             // 1.00 D2Server: 1000b440
void __fastcall TSList_UnlinkNode_B(GameTableLink * pLink);                          // 1.00 D2Server: 1000b4b0
GameTableList * __thiscall TSList_SGAMEDATA_ScalarDelete(GameTableList * pList, uint8_t nFlags); // 1.00 D2Server: 1000b500
void __fastcall SGAMEDATA_Chunk_FreeAll(GameTableChunk * pChunk);                     // 1.00 D2Server: 1000b540
void __fastcall TSListHead_Init_Sentinel(GameTableList * pList);                     // 1.00 D2Server: 1000b620
void __thiscall TSExplicitList_Resize_DrainOld(GameTableList * pList, intptr_t nLinkOffset); // 1.00 D2Server: 1000b650
GameTableLink * __thiscall TSListNode_GetOffset_B(GameTableLink * pLink, intptr_t nOffset);    // 1.00 D2Server: 1000b6e0
void __fastcall TSList_UnlinkNode_C(GameTableLink * pLink);                          // 1.00 D2Server: 1000b720
intptr_t __fastcall TSList_OffsetAdd(intptr_t nOffset, intptr_t nBase);                        // 1.00 D2Server: 1000b770
void __fastcall TSListHead_Init(GameTableList * pList);                              // 1.00 D2Server: 1000b790
void __fastcall TSListNode_Zero(GameTableListArray * pArray);                             // 1.00 D2Server: 1000b7c0
void __fastcall TSListHead_Init_B(GameTableList * pList);                            // 1.00 D2Server: 1000b7e0
void __fastcall TSListHead_InitEmpty(GameTableList * pList);                         // 1.00 D2Server: 1000b810
void __thiscall TSExplicitList_Resize(GameTableListArray * pArray, uint32_t nCount);      // 1.00 D2Server: 1000b830
void __fastcall TSListHead_InitEmpty_B(GameTableList * pList);                       // 1.00 D2Server: 1000ba10
void __fastcall TSListHead_Init_Sentinel_B(GameTableList * pList);                   // 1.00 D2Server: 1000ba30
void __fastcall TSListHead_InitFrom(GameTableList * pList, GameTableList * pSource);      // 1.00 D2Server: 1000ba70

} // namespace D2Server::GameTable
