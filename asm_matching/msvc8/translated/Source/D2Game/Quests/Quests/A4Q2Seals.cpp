#include "Source/D2Game/Quests/QUESTS.h"
#include "Source/D2Client/D2GSPacketSrv0x27.h"
#include "Source/D2Client/D2GSPacketSrv0x91.h"
#include "Source/D2Client/Game/SCmd.h"
#include "Source/D2Game/Ai/AiGeneral.h"
#include "Source/D2Game/Game/Clients.h"
#include "Source/D2Game/Game/SCmd.h"
#include "Source/D2Game/Game/Server.h"
#include "Source/D2Game/Items/ItemMode.h"
#include "Source/D2Game/Items/Items/Drop.h"
#include "Source/D2Game/Monster/Ai/AI.h"
#include "Source/D2Game/Monster/Create.h"
#include "Source/D2Game/Monster/MonsterAI.h"
#include "Source/D2Game/Monsters/Spawn.h"
#include "Source/D2Game/Player/PlayerIntro.h"
#include "Source/D2Game/Player/PlayerMsg.h"
#include "Source/D2Game/Quests/A1Q1.h"
#include "Source/D2Game/Quests/A1Q2.h"
#include "Source/D2Game/Quests/A1Q4.h"
#include "Source/D2Game/Quests/A1Q5.h"
#include "Source/D2Game/Quests/A1Q6.h"
#include "Source/D2Game/Quests/A2Q1.h"
#include "Source/D2Game/Quests/A2Q3.h"
#include "Source/D2Game/Quests/A2Q4.h"
#include "Source/D2Game/Quests/A2Q6.h"
#include "Source/D2Game/Quests/A2Q7.h"
#include "Source/D2Game/Quests/A3Q1.h"
#include "Source/D2Game/Quests/A3Q2.h"
#include "Source/D2Game/Quests/A3Q5.h"
#include "Source/D2Game/Quests/A3Q6.h"
#include "Source/D2Game/Quests/A3Q7.h"
#include "Source/D2Game/Quests/A4Q2.h"
#include "Source/D2Game/Quests/A4Q3.h"
#include "Source/D2Game/Quests/A5Q1.h"
#include "Source/D2Game/Quests/A5Q2.h"
#include "Source/D2Game/Quests/A5Q3.h"
#include "Source/D2Game/Quests/A5Q4.h"
#include "Source/D2Game/Quests/A5Q5.h"
#include "Source/D2Game/Quests/A5Q6.h"
#include "Source/D2Game/Unit/SUnit.h"
#include "Source/globals/globals.h"
#include "Source/D2Game/D2Game.globals.h"
#include "Source/D2Headers/d2_platform.h"
#include "Source/D2Game/Quests/QUESTS.inline.h"
#include "Source/D2Game/Quests/Quests/A1Q0.h"
#include "Source/D2Game/Quests/A1Q3.h"
#include "Source/D2Game/Quests/Quests/A1Q7.h"
#include "Source/D2Game/Quests/A2Q0.h"
#include "Source/D2Game/Quests/A2Q2.h"
#include "Source/D2Game/Quests/A2Q5.h"
#include "Source/D2Game/Quests/A2Q8.h"
#include "Source/D2Game/Quests/A3Q0.h"
#include "Source/D2Game/Quests/A3Q3.h"
#include "Source/D2Game/Quests/A3Q4.h"
#include "Source/D2Game/Quests/Quests/A4Q0.h"
#include "Source/D2Game/Quests/A4Q1.h"
#include "Source/D2Game/Quests/Quests/A4Q4.h"
#include "Source/D2Game/Quests/Quests/Quest37.h"
#include "Source/D2Game/Quests/Quests/Quest38.h"
#include "Source/D2Game/Quests/Quests/Quest39.h"
#include "Source/D2Game/Quests/Quests/Quest40.h"
#define DEFAULT_SECURITY_COOKIE 0xBB40E64E

namespace D2Game { namespace Quests { namespace QUESTS {

D2QuestDataStrc * __fastcall GetQuestDataByQuestNoInternal(::SGAMEDATA * pGame, eD2QuestsInternal eQuestNoInternal);
void __fastcall FindSpawnableLocation(struct D2RoomStrc * pRoom, POINT * pPoint, uint32_t nScanRadius, int32_t neCollisionFlags, struct D2RoomStrc * * pRoomsNear, int32_t nMaxTries, int32_t nSearchIterations);

static void CreateSealGlow(D2ObjectOperateFnArg * pOperateArg, POINT * pPoint, int32_t nSealIndex) {
    D2RoomStrc* pVictimRoom = D2Common::Collision::DRLGROOM_FindBetterNearbyRoom(D2Common::Units::UNITS_GetRoom(pOperateArg->sArg.pVictim), pPoint->x, pPoint->y);
    if (!pVictimRoom) {
      return;
    }

    D2RoomStrc* pSpawnRoom = 0;
    FindSpawnableLocation(pVictimRoom, pPoint, 3, 0x3f11, &pSpawnRoom, nSealIndex == 1 ? 0xd : (nSealIndex != 3 ? 0xf : 0xe), 100);
    if (!pSpawnRoom) {
      return;
    }

    if (!UNIT::SUnit::CreateUnit(UNIT_OBJECT, 0x83, pPoint->x, pPoint->y, pOperateArg->sArg.pGame, pSpawnRoom, 1, 0, 0)) {
      return;
    }

    D2Common::DUNGEON::Dungeon::ToggleHasPortalFlagRoom(pSpawnRoom, 0);
    A4Q2::OBJOP_A4Q2_DiabloSealActivateKillChaos(pOperateArg);
}

BOOL __fastcall OBJOP_A4Q2_DiabloSealCreateGlow1(D2ObjectOperateFnArg * pOperation) {
    D2QuestDataA4Q2Strc* pA4Q2Data;
    POINT* pGlowCoord;
    D2UnitStrc* pVictim = pOperation->sArg.pVictim;
    if (!pVictim || pVictim->eAnimMode.ePlayerMode == Death) {
      D2QuestDataStrc* pQuestData;
      pQuestData = GetQuestDataByQuestNoInternal(pOperation->sArg.pGame, QUESTINT_TerrorsEnd);
      if (pQuestData) {
        pA4Q2Data = pQuestData->pQuestSpecificData.pA4Q2;
        pGlowCoord = &pA4Q2Data->sInfectorCoords;
        D2Common::Units::GetCoords(pOperation->sArg.pVictim, pGlowCoord);
        pA4Q2Data->sInfectorCoords.y += -0x34;
        pGlowCoord->x += -0xc;
        CreateSealGlow(pOperation, pGlowCoord, 1);
      }
    }

    return 0;
}

BOOL __fastcall OBJOP_A4Q2_DiabloSealCreateGlow2(D2ObjectOperateFnArg * pOperation) {
    D2QuestDataA4Q2Strc* pA4Q2Data;
    POINT* pCoord;
    D2UnitStrc* pVictim = pOperation->sArg.pVictim;
    if (!pVictim || pVictim->eAnimMode.ePlayerMode == Death) {
      D2QuestDataStrc* pQuestData;
      pQuestData = GetQuestDataByQuestNoInternal(pOperation->sArg.pGame, QUESTINT_TerrorsEnd);
      if (pQuestData) {
        pA4Q2Data = pQuestData->pQuestSpecificData.pA4Q2;
        pCoord = &pA4Q2Data->sSeisCoords;
        D2Common::Units::GetCoords(pOperation->sArg.pVictim, pCoord);
        pA4Q2Data->sSeisCoords.y += 0x21;
        pCoord->x += -0x27;
        CreateSealGlow(pOperation, pCoord, 3);
      }
    }

    return 0;
}

BOOL __fastcall OBJOP_A4Q2_DiabloSealCreateGlow3(D2ObjectOperateFnArg * pOperation) {
    D2QuestDataA4Q2Strc* pA4Q2Data;
    POINT* pCoord;
    D2UnitStrc* pVictim = pOperation->sArg.pVictim;
    if (!pVictim || pVictim->eAnimMode.ePlayerMode == Death) {
      D2QuestDataStrc* pQuestData;
      pQuestData = GetQuestDataByQuestNoInternal(pOperation->sArg.pGame, QUESTINT_TerrorsEnd);
      if (pQuestData) {
        pA4Q2Data = pQuestData->pQuestSpecificData.pA4Q2;
        pCoord = &pA4Q2Data->sVisierCoords;
        D2Common::Units::GetCoords(pOperation->sArg.pVictim, pCoord);
        pA4Q2Data->sVisierCoords.y += 0x10;
        pCoord->x += 0x20;
        CreateSealGlow(pOperation, pCoord, 5);
      }
    }

    return 0;
}

}}} // namespace D2Game::Quests::QUESTS
