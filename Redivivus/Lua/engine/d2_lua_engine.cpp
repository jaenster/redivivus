// The engine side of the mod runtime: a server state per game, the world its mods see
// (D2LuaWorld over the game's SGAMEDATA) and the hooks Source/ calls (d2_lua_hooks.h). The game's
// build only, with REDIVIVUS_MODS.
//
// A game's state is made on its first tick (on the thread that ticks it, so the state is only
// ever used there) when the boot found mods, and closed at the top of GAME_DestroyGame, before the
// game frees anything. Without a state every hook returns at once and the AI lookup gives the stock
// entry.
//
// The AI overlay: for a monster whose AI id a mod claimed in its game, MONSTERAI_GetAiFunctionFromTable
// returns one of six static entries (one per targeting type) with the host's fpInit and fpMain.
// fpSecondary stays null, so every attach re-inits. fpInit marks the script to start over (it runs
// inside CreateUnit and the other attach sites, so it does nothing else, except the stock init of an
// AI that is only wrapped); fpMain is a think through the core. The stock entry is still reachable:
// the lookup with the overlay switched off.
#if D2_FEATURE_MODS
#include <windows.h>

#include <stdio.h>
#include <string.h>

#include <map>
#include <string>

#include "Source/globals/globals.h"
#include "Source/D2Client/D2ClientStrc.h"
#include "Source/D2Client/Game/Game/SGAMEDATA.h"
#include "Source/D2Common/D2Common.h"
#include "Source/D2Common/D2UnitStrc.h"
#include "Source/D2Common/Unit.h"
#include "Source/D2Common/D2RoomStrc.h"
#include "Source/D2Common/D2MonStatsTxt.h"
#include "Source/D2Common/D2StatListExStrc.h"
#include "Source/D2Common/Drlg/D2DrlgActStrc.h"
#include "Source/D2Common/Drlg/D2DrlgStrc.h"
#include "Source/D2Common/Drlg/D2DrlgLevelStrc.h"
#include "Source/D2Common/Drlg/D2DrlgRoomCoordsStrc.h"
#include "Source/D2Client/Unit/Monster.h"
#include "Source/D2Common/Units/Units.h"
#include "Source/D2Common/Stats/StatsEx.h"
#include "Source/D2Common/DataTbls/ItemStats.h"
#include "Source/D2Common/DataTbls/LvlTbls.h"
#include "Source/D2Common/DataTbls/MonsterTbls.h"
#include "Source/D2Common/Dungeon/Dungeon.h"
#include "Source/D2Common/Collision.h"
#include "Source/D2Common/Drlg/Drlg.h"
#include "Source/D2Common/Monsters/MONSTERS.h"
#include "Source/D2Game/D2Game.h"
#include "Source/D2Game/D2StateExArgStrc.h"
#include "Source/D2Game/Game/Clients.h"
#include "Source/D2Game/Game/SCmd.h"
#include "Source/D2Game/Game/Event.h"
#include "Source/D2Game/Ai/AiGeneral.h"
#include "Source/D2Game/Monster/Ai/AI.h"
#include "Source/D2Game/Monster/Create.h"
#include "Source/D2Game/Monster/Monster.h"
#include "Source/D2Game/Player/PlayerMsg.h"
#include "Source/D2Game/Skills/SkillsServer.h"
#include "Source/D2Game/Unit/SUnit.h"
#include "Source/D2Game/Unit/SUnitDmg.h"
#include "Redivivus/Lua/d2_lua_hooks.h"
#include "Redivivus/Lua/d2_lua_override.h"
#include "Redivivus/Lua/d2_lua_runtime.h"
#include "Redivivus/Lua/d2_lua_world.h"
#include "Redivivus/Lua/engine/d2_lua_engine.h"
#include "Redivivus/Lua/generated/d2_lua_api_main.gen.h"

namespace {

namespace AI = D2Game::Monster::AI;
using D2Common::ITEMS::Items::sgptDataTable;

const uint32_t kGame = 1;  // a state serves one game; its id in the runtime

void Log(const char* pszFormat, const char* pszArg, int nArg) {
    char sz[512];
    snprintf(sz, sizeof(sz), pszFormat, pszArg, nArg);
    D2LUA_LogToGame(sz);
}

// --- the AI entries ----------------------------------------------------------------------------------

thread_local bool t_bStockLookup = false;  // the lookup runs with the overlay switched off

void __fastcall LuaAiInit(::SGAMEDATA* pGame, D2UnitStrc* pUnit, D2AiParamStrc* pAiParam);
void __fastcall LuaAiMain(::SGAMEDATA* pGame, D2UnitStrc* pUnit, D2AiParamStrc* pAiParam);

D2AiListStrc s_aEntries[6] = {
    { (D2AITypes)0, &LuaAiInit, &LuaAiMain, nullptr }, { (D2AITypes)1, &LuaAiInit, &LuaAiMain, nullptr },
    { (D2AITypes)2, &LuaAiInit, &LuaAiMain, nullptr }, { (D2AITypes)3, &LuaAiInit, &LuaAiMain, nullptr },
    { (D2AITypes)4, &LuaAiInit, &LuaAiMain, nullptr }, { (D2AITypes)5, &LuaAiInit, &LuaAiMain, nullptr },
};

D2AiListStrc* StockEntry(D2UnitStrc* pUnit) {
    t_bStockLookup = true;
    D2AiListStrc* p = AI::MONSTERAI_GetAiFunctionFromTable(pUnit, 0);
    t_bStockLookup = false;
    return p;
}

void RunStockMain(::SGAMEDATA* pGame, D2UnitStrc* pUnit, D2AiParamStrc* pAiParam) {
    D2AiListStrc* p = StockEntry(pUnit);
    if (p && p->fpMain) {
        p->fpMain(pGame, pUnit, pAiParam);
    } else {
        AI::AI_Fn1_Idle(pGame, pUnit, pAiParam);
    }
}

D2MonStatsTxt* MonStats(D2UnitStrc* pUnit) {
    int n = pUnit->nClassId;
    return n >= 0 && n < sgptDataTable->nTxtMonStatsSize ? sgptDataTable->pTxtMonStats + n : nullptr;
}

// --- the world -----------------------------------------------------------------------------------------

class EngineWorld : public D2LuaWorld {
public:
    explicit EngineWorld(::SGAMEDATA* p) : pGame(p) {}
    ::SGAMEDATA* pGame;
    D2LuaRuntime* pRt = nullptr;
    D2AiParamStrc* pParam = nullptr;  // the think running now

    int Difficulty() override { return pGame->nDifficulty; }
    bool Expansion() override { return pGame->bExpansion != 0; }
    std::string Name() override { return std::string(pGame->szGameName, strnlen(pGame->szGameName, sizeof(pGame->szGameName))); }
    uint32_t Seed() override { return (uint32_t)pGame->pInitSeed.nSeedLow; }
    uint32_t Frame() override { return (uint32_t)pGame->dwGameFrame; }

    void Players(std::vector<D2UnitStrc*>& aOut) override {
        for (D2ClientStrc* p = pGame->pClient; p; p = p->pNext) {
            if (p->ConnectionState != 4) {
                continue;
            }
            D2UnitStrc* pUnit = D2Game::GAME::Clients::CLIENTS_GetControlUnit(p, 0);
            if (pUnit && D2Common::Units::UNITS_GetRoom(pUnit)) {
                aOut.push_back(pUnit);
            }
        }
    }

    void Message(const char* pszText) override {
        char sz[0x100];
        memset(sz, 0, sizeof(sz));
        strncpy(sz, pszText, sizeof(sz) - 1);
        D2Game::PLAYER::PlayerMsg::CHAT_BroadcastMessageToAllClients(pGame, (uint32_t*)sz, 0);
    }

    void SendMessage(D2UnitStrc* pUnit, const char* pszText) override {
        D2ClientStrc* pClient = pUnit->pUnitData.pUnitDataPlayer ? pUnit->pUnitData.pUnitDataPlayer->pClient : nullptr;
        if (!pClient || pClient->ConnectionState != 4) {
            return;
        }
        D2GSPacketSrv0x26 tPacket;
        memset(&tPacket, 0, sizeof(tPacket));
        tPacket.nPacketId = 0x26;
        tPacket.nMessageType = 4;
        strncpy(tPacket.szMessage, pszText, sizeof(tPacket.szMessage) - 1);
        D2Game::GAME::SCmd::NET_D2GS_SERVER_Send_0x26_CHATSTART(pClient, &tPacket);
    }

    // --- levels and rooms ---
    D2DrlgLevelStrc* LevelOf(int nLevel) {
        int nAct = D2Common::Drlg::GetActNoFromLevelNumber((eD2LevelId)nLevel);
        if (nAct < 0 || nAct > 4 || !pGame->pAct[nAct] || !pGame->pAct[nAct]->pDrlg) {
            return nullptr;
        }
        for (D2DrlgLevelStrc* p = pGame->pAct[nAct]->pDrlg->pLevel; p; p = p->pLevelNext) {
            if (p->eD2LevelId == nLevel) {
                return p;
            }
        }
        return nullptr;
    }
    int LevelCount() override { return sgptDataTable->nTxtLevelsSize; }
    bool LevelExists(int nLevel) override { return LevelOf(nLevel) != nullptr; }
    bool LevelInfo(int nLevel, D2LuaLevelInfo& tOut) override {
        D2LevelsTxt* pTxt = D2Common::DATATBLS::LvlTbls::TXT_Levels_GetLine((eD2LevelId)nLevel);
        if (!pTxt) {
            return false;
        }
        tOut.sName.clear();
        for (int i = 0; i < 40 && pTxt->wszLevelName[i]; i++) {
            uint16_t c = pTxt->wszLevelName[i];
            if (c < 0x80) {
                tOut.sName += (char)c;
            } else if (c < 0x800) {
                tOut.sName += (char)(0xC0 | (c >> 6));
                tOut.sName += (char)(0x80 | (c & 0x3F));
            } else {
                tOut.sName += (char)(0xE0 | (c >> 12));
                tOut.sName += (char)(0x80 | ((c >> 6) & 0x3F));
                tOut.sName += (char)(0x80 | (c & 0x3F));
            }
        }
        if (tOut.sName.empty()) {
            tOut.sName.assign(pTxt->LevelName, strnlen(pTxt->LevelName, sizeof(pTxt->LevelName)));
        }
        tOut.nAct = D2Common::Drlg::GetActNoFromLevelNumber((eD2LevelId)nLevel);
        tOut.bTown = D2Common::DUNGEON::Dungeon::IsTownLevelId(nLevel) != 0;
        D2DrlgLevelStrc* pLevel = LevelOf(nLevel);
        tOut.bGenerated = pLevel && pLevel->pRoomExFirst;
        if (tOut.bGenerated) {
            tOut.tBounds = { (int)pLevel->sCoordinatesAndSize.WorldPosition.x * 5, (int)pLevel->sCoordinatesAndSize.WorldPosition.y * 5,
                             (int)pLevel->sCoordinatesAndSize.WorldSize.x * 5, (int)pLevel->sCoordinatesAndSize.WorldSize.y * 5 };
        }
        return true;
    }
    template <typename F>
    void ForEachActiveRoom(F f) {
        for (int nAct = 0; nAct < 5; nAct++) {
            if (!pGame->pAct[nAct]) {
                continue;
            }
            for (D2RoomStrc* p = pGame->pAct[nAct]->pRoom; p; p = p->pDrlgRoomNext) {
                if (!f(p)) {
                    return;
                }
            }
        }
    }
    void LevelRooms(int nLevel, std::vector<D2RoomStrc*>& aOut) override {
        ForEachActiveRoom([&](D2RoomStrc* p) {
            if ((int)D2Common::DUNGEON::Dungeon::DRLGROOM_GetLevelIdFromRoom(p) == nLevel) {
                aOut.push_back(p);
            }
            return true;
        });
    }
    D2LuaRoomKey RoomKeyOf(D2RoomStrc* p) override {
        return { (uint32_t)D2Common::DUNGEON::Dungeon::DRLGROOM_GetLevelIdFromRoom(p), (uint32_t)p->sCoords.sTileRect.left,
                 (uint32_t)p->sCoords.sTileRect.top };
    }
    D2RoomStrc* RoomFind(const D2LuaRoomKey& k) override {
        D2RoomStrc* pFound = nullptr;
        ForEachActiveRoom([&](D2RoomStrc* p) {
            D2LuaRoomKey o = RoomKeyOf(p);
            if (o.nLevel == k.nLevel && o.nTileX == k.nTileX && o.nTileY == k.nTileY) {
                pFound = p;
                return false;
            }
            return true;
        });
        return pFound;
    }
    D2LuaRect RoomBounds(D2RoomStrc* p) override {
        D2DrlgRoomCoordsStrc t;
        memset(&t, 0, sizeof(t));
        D2Common::DUNGEON::Dungeon::DRLGROOM_GetRoomCoordinates(p, &t);
        return { t.dwXStart, t.dwYStart, t.dwXSize, t.dwYSize };
    }
    void RoomNeighbours(D2RoomStrc* p, std::vector<D2RoomStrc*>& aOut) override {
        for (uint32_t i = 0; p->ppRoomList && i < p->nRoomListCount; i++) {
            if (p->ppRoomList[i] && p->ppRoomList[i] != p) {
                aOut.push_back(p->ppRoomList[i]);
            }
        }
    }
    void RoomUnits(D2RoomStrc* p, std::vector<D2UnitStrc*>& aOut) override {
        for (D2UnitStrc* u = p->pUnitList; u; u = u->pRoomNext) {
            aOut.push_back(u);
        }
    }
    D2RoomStrc* RoomAt(int nX, int nY, D2RoomStrc* pNear) override {
        if (pNear) {
            D2RoomStrc* p = D2Common::Collision::DRLGROOM_FindBetterNearbyRoom(pNear, nX, nY);
            if (p) {
                return p;
            }
        }
        for (int nAct = 0; nAct < 5; nAct++) {
            if (pGame->pAct[nAct]) {
                D2RoomStrc* p = D2Common::DUNGEON::Dungeon::FindRoomBySubtileCoordinates(pGame->pAct[nAct], nX, nY);
                if (p) {
                    return p;
                }
            }
        }
        return nullptr;
    }

    // --- units ---
    D2UnitStrc* FindUnit(int nType, uint32_t nGuid) override {
        if (nType < 0 || nType > 5) {
            return nullptr;
        }
        D2UnitStrc* p = D2Game::UNIT::SUnit::FindUnitFromTable(pGame, (eD2UnitType)nType, nGuid);
        if (!p || !D2Common::Units::UNITS_GetRoom(p) || (p->eUnitFlagsEx & UNITFLAGEX_HASBEENDELETED)) {
            return nullptr;
        }
        if (nType == D2LUA_UNIT_MONSTER && D2Common::Monsters::MONSTERS::CheckIfMonsterIsDead(p)) {
            return nullptr;
        }
        return p;
    }
    int UnitType(D2UnitStrc* p) override { return (int)p->eUnitType; }
    uint32_t UnitGuid(D2UnitStrc* p) override { return (uint32_t)p->nUnitGUID; }
    int ClassId(D2UnitStrc* p) override { return p->nClassId; }
    bool Position(D2UnitStrc* p, int& nX, int& nY) override {
        if (!D2Common::Units::UNITS_GetRoom(p) || !p->pPath.pDynamicPath) {
            return false;
        }
        POINT pt = { 0, 0 };
        D2Common::Units::GetCoords(p, &pt);
        nX = pt.x;
        nY = pt.y;
        return true;
    }
    D2RoomStrc* UnitRoom(D2UnitStrc* p) override { return D2Common::Units::UNITS_GetRoom(p); }
    int UnitLevel(D2UnitStrc* p) override {
        D2RoomStrc* pRoom = D2Common::Units::UNITS_GetRoom(p);
        return pRoom ? (int)D2Common::DUNGEON::Dungeon::DRLGROOM_GetLevelIdFromRoom(pRoom) : 0;
    }
    int UnitAct(D2UnitStrc* p) override { return (int)p->nAct; }
    int Mode(D2UnitStrc* p) override { return (int)p->eAnimMode.ePlayerMode; }
    bool IsDead(D2UnitStrc* p) override {
        if (p->eUnitType == UNIT_MONSTER) {
            return D2Common::Monsters::MONSTERS::CheckIfMonsterIsDead(p) != 0;
        }
        if (p->eUnitType == UNIT_PLAYER) {
            return D2Game::UNIT::SUnit::CheckForModeZeroDeath(p) != 0;
        }
        return false;
    }
    bool IsCorpse(D2UnitStrc* p) override {
        return p->eUnitType == UNIT_PLAYER && D2Common::DATATBLS::ItemStats::UNIT_GetUnitState(p, STATE_playerbody);
    }
    D2UnitStrc* Owner(D2UnitStrc* p) override { return D2Game::UNIT::SUnit::GetOwnerUnit(pGame, p); }
    int StatCount() override { return sgptDataTable->nTxtItemStatCostSize; }
    int StateCount() override { return sgptDataTable->nTxtStatesSize; }
    int GetStat(D2UnitStrc* p, int nStat, int nLayer, bool bBase) override {
        return bBase ? D2Common::Stats::StatsEx::SERVER_GetUnitStat(p, (eD2UnitStat)nStat, (uint16_t)nLayer)
                     : D2Common::Stats::StatsEx::STATSEX_GetCurrentStat(p, (eD2UnitStat)nStat, (uint16_t)nLayer);
    }
    void SetStat(D2UnitStrc* p, int nStat, int nValue, int nLayer) override {
        D2Common::Stats::StatsEx::STATLIST_SetUnitStat(p, (eD2UnitStat)nStat, nValue, (uint16_t)nLayer);
    }
    void AddStat(D2UnitStrc* p, int nStat, int nValue, int nLayer) override {
        if (nValue) {
            D2Common::Stats::StatsEx::STATLIST_AddUnitStat(p, (eD2UnitStat)nStat, nValue, (int16_t)nLayer);
        }
    }
    bool HasState(D2UnitStrc* p, int nState) override {
        return D2Common::DATATBLS::ItemStats::UNIT_GetUnitState(p, (eD2States)nState) != 0;
    }
    bool AddState(D2UnitStrc* p, int nState, int nFrames) override {
        D2StateExArgStrc tArg;
        memset(&tArg, 0, sizeof(tArg));
        tArg.pUnit = nullptr;
        tArg.pTarget = p;
        tArg.eSkill = (eD2Skills)0;
        tArg.nSkillLvl = 1;
        tArg.nDurationInFrames = nFrames;
        tArg.eStat = (eD2UnitStat)-1;
        tArg.nSkillCalc = 0;
        tArg.eAuraState = (eD2States)0;
        tArg.pFunction = nullptr;
        return D2Game::SKILLS::SkillsServer::SetStateWithExpiryTime(&tArg) != nullptr;
    }
    void RemoveState(D2UnitStrc* p, int nState) override {
        D2StatListExStrc* pList = D2Common::Stats::StatsEx::GetStatsByState(p, (eD2States)nState);
        if (pList) {
            D2Common::Stats::StatsEx::STATLIST_RemoveAndRecalc(p, pList);
            D2Common::Stats::StatsEx::FreeStatList(&pList->sStatList);
        } else {
            D2Common::DATATBLS::ItemStats::SetState(p, (eD2States)nState, 0);
        }
    }
    std::string PlayerName(D2UnitStrc* p) override {
        D2UnitDataPlayerStrc* pData = p->pUnitData.pUnitDataPlayer;
        return pData ? std::string(pData->szCharName, strnlen(pData->szCharName, sizeof(pData->szCharName))) : std::string();
    }

    // --- monsters ---
    int MonsterTypeFlags(D2UnitStrc* p) override {
        return p->pUnitData.pUnitDataMonster ? p->pUnitData.pUnitDataMonster->eMonsterType : 0;
    }
    bool IsBoss(D2UnitStrc* p) override {
        D2MonStatsTxt* pTxt = MonStats(p);
        return pTxt && pTxt->boss;
    }
    int MonsterCount() override { return sgptDataTable->nTxtMonStatsSize; }
    void Kill(D2UnitStrc* p, D2UnitStrc* pKiller) override {
        if (IsDead(p)) {
            return;
        }
        D2Common::Stats::StatsEx::STATLIST_SetUnitStat(p, UNITSTAT_hitpoints, 0, 0);
        D2Game::UNIT::SUnitDmg::SERVER_KillMonster(pGame, p, pKiller, 1);
    }
    D2UnitStrc* Target(D2UnitStrc* p) override { return D2Game::UNIT::SUnit::GetTargetEx(pGame, p); }
    D2UnitStrc* SpawnMonster(int nClass, int nX, int nY, int nRadius, bool bMinions) override {
        D2RoomStrc* pRoom = RoomAt(nX, nY, nullptr);
        if (!pRoom) {
            return nullptr;
        }
        POINT pt = { nX, nY };
        pRoom = D2Common::Collision::GetFreeCoordinates_WithMaxDistance(pRoom, &pt, 2, 0x3C01, TRUE, nRadius);
        if (!pRoom) {
            return nullptr;
        }
        return D2Game::Monster::Create::SpawnMonsterAtRoomPos(pGame, pRoom, pt.x, pt.y, (uint32_t)nClass, eD2MonsterAnimMode_ns::Neutral, -1,
                                                              bMinions ? 0 : 0x40);
    }
    int AiIndex(D2UnitStrc* p) override {
        D2MonStatsTxt* pTxt = MonStats(p);
        return pTxt ? pTxt->AI : -1;
    }
    bool MonsterData(int nClass, D2LuaMonsterData& t) override {
        if (nClass < 0 || nClass >= sgptDataTable->nTxtMonStatsSize) {
            return false;
        }
        D2MonStatsTxt* pTxt = sgptDataTable->pTxtMonStats + nClass;
        int nDiff = pGame->nDifficulty < 3 ? pGame->nDifficulty : 0;
        t.nBaseId = pTxt->BaseId;
        t.nAi = pTxt->AI;
        t.nAiDelay = pTxt->aidel[nDiff];
        for (int i = 0; i < 8; i++) {
            t.aAiParams[i] = pTxt->aip[i][nDiff];
            t.aSkills[i] = pTxt->aSkill[i];
            t.aSkillModes[i] = (uint8_t)pTxt->aSkMode[i];
        }
        return true;
    }
    uint32_t SeedRoll(D2UnitStrc* p) override { return D2_SeedRoll(&p->sSeed); }
    D2UnitStrc* FindCorpse(uint32_t nGuid) override {
        D2UnitStrc* p = D2Game::UNIT::SUnit::FindUnitFromTable(pGame, UNIT_MONSTER, nGuid);
        if (!p || !D2Common::Units::UNITS_GetRoom(p) || (p->eUnitFlagsEx & UNITFLAGEX_HASBEENDELETED)) {
            return nullptr;
        }
        return D2Common::Monsters::MONSTERS::CheckIfMonsterIsDead(p) ? p : nullptr;
    }
    int Alignment(D2UnitStrc* p) override { return D2Common::Stats::StatsEx::STATSEX_GetAlignment(p); }
    bool Targetable(D2UnitStrc* p) override { return ((p->eUnitFlags >> 1) & 1) != 0; }
    bool HasStateMask(D2UnitStrc* p, int nMask) override {
        namespace S = D2Common::DATATBLS::ItemStats;
        switch (nMask) {
        case 0: return S::STATES_HasUdeadFlagState(p) != 0;
        case 1: return S::STATES_HasHideFlagState(p) != 0;
        case 2: return S::STATES_HasActiveFlagState(p) != 0;
        case 3: return S::STATES_HasAuraFlagState(p) != 0;
        case 4: return S::STATES_HasRemHitFlagState(p) != 0;
        case 5: return S::STATES_HasRestrictFlagState(p) != 0;
        case 6: return S::STATES_HasLifeFlagState(p) != 0;
        case 7: return S::STATES_HasShatterFlagState(p) != 0;
        default: return false;
        }
    }
    D2UnitStrc* Leader(D2UnitStrc* p) override { return D2Game::Ai::AiGeneral::GetOwnerUnit(p); }
    void Minions(D2UnitStrc* p, std::vector<D2UnitStrc*>& aOut) override {
        D2AiGeneralStrc* pAi = p->eUnitType == UNIT_MONSTER && p->pUnitData.pUnitDataMonster ? p->pUnitData.pUnitDataMonster->pAiGeneral : nullptr;
        for (D2MonsterDataMinionList* n = pAi ? pAi->pMinionList : nullptr; n; n = n->pNext) {
            D2UnitStrc* m = D2Game::UNIT::SUnit::FindUnitFromTable(pGame, UNIT_MONSTER, n->nMinionGUID);
            if (m) {
                aOut.push_back(m);
            }
        }
    }
    void AdjacentRooms(D2RoomStrc* pRoom, std::vector<D2RoomStrc*>& aOut) override {
        D2RoomStrc** ppList = nullptr;
        int nCount = 0;
        D2Common::DUNGEON::Dungeon::DRLGROOM_GetAdjacentRoomsList(pRoom, &ppList, &nCount);
        for (int i = 0; i < nCount; i++) {
            if (ppList[i]) {
                aOut.push_back(ppList[i]);
            }
        }
    }
    int AiDistance(D2UnitStrc* p, D2UnitStrc* pOther) override { return AI::GetDistance(p, pOther); }
    D2UnitStrc* AiFindTarget(D2UnitStrc* p, int& nDistance) override {
        uint32_t n = 0;
        D2UnitStrc* t = AI::FindTargetAndDistance(pGame, p, &n, nullptr);
        nDistance = (int)n;
        return t;
    }
    bool AiCanCast(D2UnitStrc* p, int nSkill, D2UnitStrc* pTarget, int nX, int nY) override {
        return AI::AI_Monster_ValidateSkillAtPosition(pGame, p, nSkill, pTarget, nX, nY) != 0;
    }
    void AiOrderMinions(D2UnitStrc* p, int nOrder) override {
        D2AiCmdArgumentsStrc tArgs;
        memset(&tArgs, 0, sizeof(tArgs));
        tArgs.nCmdId = nOrder;
        D2Game::Ai::AiGeneral::CreateMinion(pGame, p, &tArgs);
    }

    int LifePercent(D2UnitStrc* p) override { return D2Common::Units::GetCurrentLifePercentage(p); }
    bool CollidesWith(D2UnitStrc* p, D2UnitStrc* pOther, int nMask) override {
        return D2Common::Units::TestCollisionWithUnit(p, pOther, nMask) != 0;
    }
    void AiTargetScan(D2UnitStrc* p, D2LuaTargetScan& tOut) override {
        D2AiMephistoScanStrc tScan;
        memset(&tScan, 0, sizeof(tScan));
        tScan.nNearestDistance = 0x7fffffff;
        tScan.nWeakestLife = 0x7fffffff;
        tScan.nNearestWhenPlayerDistance = 0x7fffffff;
        AI::ForEach(pGame, p, (D2MonsterAiParameterStruct*)&tScan, (fpMonsterAiForEach)AI::CollectMephistoTargetInfo, 1);
        tOut.pNearest = tScan.pNearestUnit;
        tOut.nNearestDistance = tScan.nNearestDistance;
        tOut.pWeakest = tScan.pWeakestUnit;
        tOut.nWeakestLife = tScan.nWeakestLife;
        tOut.pNearestWhenPlayer = tScan.pNearestWhenPlayer;
        tOut.nNearestWhenPlayerDistance = tScan.nNearestWhenPlayerDistance;
        tOut.nCount = tScan.nTotalCount;
        tOut.nCloseCount = tScan.nCloseCount;
    }
    void AiPathOverrides(D2UnitStrc* p, int nPathType, int nVelocity, int nSteps) override {
        AI::AITACTICS_SetPathOverrides(p, nPathType, nVelocity, (uint8_t)nSteps);
    }

    // --- actions ---
    void AiIdle(D2UnitStrc* p, int nFrames) override { AI::PlanNextAiMove(pGame, p, nFrames); }
    bool AiMove(D2UnitStrc* p, D2UnitStrc* pTarget, int nX, int nY, int nMove, int nFlags) override {
        eD2MonsterAnimMode eMode = nMove == D2LUA_MOVE_RUN ? eD2MonsterAnimMode_ns::Run : eD2MonsterAnimMode_ns::Walk;
        return AI::StartMonsterMove(pGame, p, pTarget, eMode, nX, nY, 1, nFlags) != 0;
    }
    bool AiAttack(D2UnitStrc* p, D2UnitStrc* pTarget, bool bSecondary) override {
        return AI::AI_ChangeModeAndTargetUnit(pGame, p, bSecondary ? eD2MonsterAnimMode_ns::Attack2 : eD2MonsterAnimMode_ns::Attack1, pTarget) != 0;
    }
    bool AiCast(D2UnitStrc* p, int nSkill, int nMode, D2UnitStrc* pTarget, int nX, int nY) override {
        return AI::SERVER_UNIT_CastSkill(pGame, p, (uint8_t)nMode, (eD2Skills)nSkill, pTarget, nX, nY) != 0;
    }
    bool AiSequence(D2UnitStrc* p, int nSkill, D2UnitStrc* pTarget, int nX, int nY) override {
        return AI::MONSTERAI_CommandSkillSequence(pGame, p, (eD2Skills)nSkill, pTarget, nX, nY) != 0;
    }
    bool AiCircle(D2UnitStrc* p, D2UnitStrc* pTarget, int nSteps, int nFlags) override {
        return AI::SetCombatStateAndApproachTarget(pGame, p, pTarget, (uint8_t)nSteps, nFlags) != 0;
    }
    bool AiWalkPast(D2UnitStrc* p, D2UnitStrc* pTarget, int nOffset, bool bForce) override {
        return AI::WalkTowardsUnitOffset(pGame, p, pTarget, (uint8_t)nOffset, bForce ? 1 : 0) != 0;
    }
    bool AiWanderAround(D2UnitStrc* p, D2UnitStrc* pTarget, int nRadius) override {
        return AI::WalkRandomOffsetAroundUnit(pGame, p, pTarget, (uint32_t)nRadius) != 0;
    }
    bool AiWalkToDistance(D2UnitStrc* p, D2UnitStrc* pTarget, int nMaxStep, int nDistance) override {
        D2Game::GAME::Event::EVENT_RemoveUnitTimersByType(pGame, p, UNITEVENTCALLBACK_AITHINK, 0);
        return AI::MONSTERAI_CommandWalkToDistance(pGame, p, pTarget, nMaxStep, nDistance) != 0;
    }
    bool AiWander(D2UnitStrc* p, int nRadius) override { return AI::WalkRandomOffset(pGame, p, (uint32_t)nRadius) != 0; }
    bool AiFlee(D2UnitStrc* p, D2UnitStrc* pFrom, int nDistance) override {
        return AI::MONSTERAI_CommandRunAwayFromUnit(pGame, p, pFrom, (uint8_t)nDistance, 0) != 0;
    }
    void AiStock(D2UnitStrc* p) override {
        if (pParam) {
            RunStockMain(pGame, p, pParam);
        }
    }
    int AiState(D2UnitStrc* p) override { return D2Game::Monster::GetAiState(p); }
    void AiMarkAction(D2UnitStrc* p) override {
        if (p->pUnitData.pUnitDataMonster) {
            D2Game::Monster::SetAiState(p, 1);
        }
    }
    int AiDelay(D2UnitStrc* p) override {
        D2MonStatsTxt* pTxt = MonStats(p);
        int n = pTxt && pGame->nDifficulty < 3 ? pTxt->aidel[pGame->nDifficulty] : 0;
        return n > 0 ? n : 1;
    }
    void AiClaim(int nAiIndex, int nClaim, int nTargeting) override {
        char sz[160];
        snprintf(sz, sizeof(sz), "[mods] the monster AI %s is %s by a mod (targeting %d)", D2LuaWorld_AiName(nAiIndex),
                 nClaim == D2LUA_AI_REGISTERED ? "replaced" : "wrapped", nTargeting);
        D2LUA_LogToGame(sz);
    }
};

// --- the games -----------------------------------------------------------------------------------------

struct GameMods {
    D2LuaRuntime* pRt = nullptr;
    EngineWorld* pWorld = nullptr;
};

CRITICAL_SECTION& Lock() {
    static CRITICAL_SECTION s_cs;
    static bool s_bInit = [] {
        InitializeCriticalSection(&s_cs);
        return true;
    }();
    (void)s_bInit;
    return s_cs;
}

std::map<::SGAMEDATA*, GameMods>& Games() {
    static std::map<::SGAMEDATA*, GameMods> s_tGames;
    return s_tGames;
}

volatile LONG s_nGames = 0;  // games with a state: the hooks skip the lock when 0

GameMods* Find(::SGAMEDATA* pGame) {
    if (!s_nGames || !pGame) {
        return nullptr;
    }
    EnterCriticalSection(&Lock());
    auto it = Games().find(pGame);
    GameMods* p = it == Games().end() ? nullptr : &it->second;
    LeaveCriticalSection(&Lock());
    return p;
}

EngineWorld* WorldOf(::SGAMEDATA* pGame) {
    GameMods* g = Find(pGame);
    return g && g->pRt ? g->pWorld : nullptr;
}

::SGAMEDATA* GameOfUnit(D2UnitStrc* pUnit) {
    return pUnit->uSideData.pServerSide.pGame;
}

GameMods* Start(::SGAMEDATA* pGame, const D2LuaBootResult* pBoot) {
    GameMods tMods;
    tMods.pWorld = new EngineWorld(pGame);
    D2LuaRuntimeDesc tDesc;
    tDesc.nSide = D2LUA_SIDE_SERVER;
    tDesc.aMods = pBoot->aOrder;
    tDesc.aApis.push_back(&g_D2LuaApi_main);
    tDesc.aApis.push_back(&g_D2LuaApi_world);
    tDesc.pfnLog = D2LUA_LogToGame;
    tDesc.sSettingsFile = D2LUA_SettingsFile();
    tDesc.pWorld = tMods.pWorld;
    tDesc.aRawGrant = pBoot->aRawGrant;
    tMods.pRt = D2LuaRt_Create(tDesc);
    tMods.pWorld->pRt = tMods.pRt;
    EnterCriticalSection(&Lock());
    GameMods* p = &(Games()[pGame] = tMods);
    InterlockedIncrement(&s_nGames);
    LeaveCriticalSection(&Lock());
    if (!tMods.pRt) {
        Log("[mods] game '%s': the server state could not start; no mods in this game%.0d", pGame->szGameName, 0);
        return p;
    }
    std::string sError;
    if (!D2LuaRt_StartGame(tMods.pRt, kGame, nullptr, sError)) {
        Log("[mods] game '%s': %s", pGame->szGameName, 0);
    }
    Log("[mods] game '%s' started with %d mod(s)", pGame->szGameName, (int)pBoot->aOrder.size());
    return p;
}

}  // namespace

// --- the hooks -------------------------------------------------------------------------------------------

D2AiListStrc* D2LuaAi_Overlay(D2UnitStrc* pUnit, int nAiIndex) {
    if (t_bStockLookup || !s_nGames || !pUnit || pUnit->eUnitType != UNIT_MONSTER || nAiIndex < 0 || nAiIndex >= D2LUA_AI_STOCK_COUNT) {
        return nullptr;
    }
    EngineWorld* w = WorldOf(GameOfUnit(pUnit));
    if (!w || !w->aClaims[nAiIndex]) {
        return nullptr;
    }
    int nTargeting = w->aTargeting[nAiIndex];
    if (w->aClaims[nAiIndex] != D2LUA_AI_REGISTERED) {
        D2AiListStrc* pStock = StockEntry(pUnit);
        nTargeting = pStock ? (int)pStock->eAIType : 0;
    }
    return &s_aEntries[nTargeting >= 0 && nTargeting < 6 ? nTargeting : 0];
}

namespace {

void __fastcall LuaAiInit(::SGAMEDATA* pGame, D2UnitStrc* pUnit, D2AiParamStrc* pAiParam) {
    EngineWorld* w = WorldOf(pGame);
    int nAi = 0;
    D2MonStatsTxt* pTxt = MonStats(pUnit);
    nAi = pTxt ? pTxt->AI : -1;
    bool bRegistered = w && nAi >= 0 && nAi < D2LUA_AI_STOCK_COUNT && w->aClaims[nAi] == D2LUA_AI_REGISTERED;
    if (!bRegistered) {
        D2AiListStrc* pStock = StockEntry(pUnit);
        if (pStock && pStock->fpInit) {
            pStock->fpInit(pGame, pUnit, pAiParam);
        }
    }
    if (w) {
        w->AiRestart(pUnit);
    }
}

void __fastcall LuaAiMain(::SGAMEDATA* pGame, D2UnitStrc* pUnit, D2AiParamStrc* pAiParam) {
    EngineWorld* w = WorldOf(pGame);
    if (!w) {
        RunStockMain(pGame, pUnit, pAiParam);
        return;
    }
    D2AiParamStrc* pWas = w->pParam;
    w->pParam = pAiParam;
    bool bRan = w->Think(w->pRt, kGame, pUnit, pAiParam ? pAiParam->pTarget : nullptr, pAiParam ? pAiParam->nDistanceToTarget : 0,
                         pAiParam && pAiParam->bEngagedInCombat);
    w->pParam = pWas;
    if (!bRan) {
        RunStockMain(pGame, pUnit, pAiParam);
    }
}

}  // namespace

void D2LuaEngine_Tick(::SGAMEDATA* pGame) {
    const D2LuaBootResult* pBoot = D2LUA_BootResult();
    if (!pBoot || pBoot->aOrder.empty()) {
        return;
    }
    GameMods* g = Find(pGame);
    if (!g) {
        g = Start(pGame, pBoot);
        // made inside the game's scope (ServerGameLoop): the rest of this frame is the state's
        if (g->pRt) {
            D2LuaOv_BindThread(D2LUA_SIDE_SERVER, g->pRt);
        }
    }
    if (g->pRt) {
        D2LuaRt_Tick(g->pRt, kGame);
    }
}

// The thread works for the game until the scope ends: function overrides on it run in the game's
// server state. Returns the state bound before (scopes nest: a game's frame inside another's).
// The scopes are also timed (outermost only): every 1000 of them the log gets their average and
// longest, the server's time per frame and command with whatever the mods override.
namespace {
thread_local int t_nScopeDepth = 0;
thread_local LARGE_INTEGER t_tScopeStart;
thread_local double t_nScopeSum = 0, t_nScopeMax = 0;
thread_local int t_nScopes = 0;
}  // namespace

void* D2LuaEngine_ScopeBegin(::SGAMEDATA* pGame) {
    if (t_nScopeDepth++ == 0) {
        QueryPerformanceCounter(&t_tScopeStart);
    }
    if (!s_nGames) {
        return D2LuaOv_BindThread(D2LUA_SIDE_SERVER, nullptr);
    }
    GameMods* g = Find(pGame);
    return D2LuaOv_BindThread(D2LUA_SIDE_SERVER, g ? g->pRt : nullptr);
}

void D2LuaEngine_ScopeEnd(void* pWas) {
    D2LuaOv_BindThread(D2LUA_SIDE_SERVER, (D2LuaRuntime*)pWas);
    if (--t_nScopeDepth == 0) {
        LARGE_INTEGER tEnd, tFreq;
        QueryPerformanceCounter(&tEnd);
        QueryPerformanceFrequency(&tFreq);
        double nUs = (double)(tEnd.QuadPart - t_tScopeStart.QuadPart) * 1e6 / (double)tFreq.QuadPart;
        t_nScopeSum += nUs;
        t_nScopeMax = nUs > t_nScopeMax ? nUs : t_nScopeMax;
        if (++t_nScopes == 1000) {
            char sz[160];
            snprintf(sz, sizeof(sz), "[mods] server scopes: %d, %.1f us on average, %.1f us the longest", t_nScopes, t_nScopeSum / t_nScopes,
                     t_nScopeMax);
            D2LUA_LogToGame(sz);
            t_nScopes = 0;
            t_nScopeSum = t_nScopeMax = 0;
        }
    }
}

void D2LuaEngine_GameEnd(::SGAMEDATA* pGame) {
    GameMods* g = Find(pGame);
    if (!g) {
        return;
    }
    GameMods tMods = *g;
    EnterCriticalSection(&Lock());
    Games().erase(pGame);
    InterlockedDecrement(&s_nGames);
    LeaveCriticalSection(&Lock());
    if (tMods.pRt) {
        D2LuaRt_EndGame(tMods.pRt, kGame);
        D2LuaRt_Destroy(tMods.pRt);
    }
    delete tMods.pWorld;
    Log("[mods] game '%s' ended%.0d", pGame->szGameName, 0);
}

void D2LuaEngine_UnitFreed(::SGAMEDATA* pGame, D2UnitStrc* pUnit) {
    if (!s_nGames || !pUnit || pUnit->eUnitType != UNIT_MONSTER) {
        return;
    }
    EngineWorld* w = WorldOf(pGame);
    if (w) {
        w->AiDrop(w->pRt, kGame, (uint32_t)pUnit->nUnitGUID);
    }
}

void D2LuaEngine_MonsterDied(::SGAMEDATA* pGame, D2UnitStrc* pUnit) {
    D2LuaEngine_UnitFreed(pGame, pUnit);
}
#endif
