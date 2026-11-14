/**
 * Data defined in D2Game/Objects/Objects.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Game)
#error "D2Game header included by a module that may not use D2Game (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2Game) && !defined(D2_LOADS_D2Game)
#error "internal D2Game header included from another module; include D2Game's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2DrlgRoomCoordsStrc;
struct D2ObjectPopulateEntryStrc;
struct D2RoomStrc;
struct SGAMEDATA;
typedef int32_t (__fastcall *D2ObjectPopulateFn)(::SGAMEDATA * pGame, D2RoomStrc * pRoom, D2DrlgRoomCoordsStrc * pRoomCoords);

extern uint8_t gbObjectInitTableSetup;
extern int gaObjectSpawnOffsetGroup0[50];
// Room object-populate dispatch table. 7 records of {int32 nEnabled; D2ObjectPopulateFn *pfnPopulate}. Base and stride proven by OBJECT_RollAndDispatchPopulateFn @0x552400, which reads *(int*)(idx*8 + 0x731e68) and *(code**)(idx*8 + 0x731e6c). Index range 1..6 (the mask loop yields 1..7 and the guard requires idx < count <= 7), so 7 records = 56 bytes end exactly at RackType @0x731ea0.
extern D2ObjectPopulateEntryStrc gaObjectPopulateTable[7];
