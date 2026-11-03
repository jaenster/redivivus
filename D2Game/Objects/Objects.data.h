/**
 * Data defined in D2Game/Objects/Objects.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "D2Client/UNIT/Object.h"

// Forward declarations
struct D2DrlgRoomCoordsStrc;
struct D2ObjectPopulateEntryStrc;
struct D2RoomStrc;
struct SGAMEDATA;
typedef int32_t (__fastcall *D2ObjectPopulateFn)(::SGAMEDATA * pGame, D2RoomStrc * pRoom, D2DrlgRoomCoordsStrc * pRoomCoords);

extern int gaObjectSpawnOffsetGroup0[50];
// Room object-populate dispatch table. 7 records of {int32 nEnabled; D2ObjectPopulateFn *pfnPopulate}. Base and stride proven by OBJECT_RollAndDispatchPopulateFn @0x552400, which reads *(int*)(idx*8 + 0x731e68) and *(code**)(idx*8 + 0x731e6c). Index range 1..6 (the mask loop yields 1..7 and the guard requires idx < count <= 7), so 7 records = 56 bytes end exactly at RackType @0x731ea0.
extern D2ObjectPopulateEntryStrc gaObjectPopulateTable[7];
