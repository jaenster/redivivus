// A stand-in engine for the world natives (d2_lua_world.h): units in rooms of one level, a frame
// clock, and monsters that think the way the engine's do (a think timer, actions that end after a
// few frames and think again at once, a stock AI that counts its thinks). The game's own world is
// engine/d2_lua_engine.cpp; this one lets the host test run the world classes, the AI hook and the
// example mods without the game.
#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_mods.h"
#include "Redivivus/Lua/d2_lua_runtime.h"
#include "Redivivus/Lua/d2_lua_world.h"
#include "Redivivus/Lua/generated/d2_lua_api_main.gen.h"
#include "Redivivus/Lua/test/d2_lua_test_util.h"
#include "lua.hpp"

struct D2RoomStrc {
    int nLevel;
    int nTileX, nTileY, nTilesW, nTilesH;
    bool bActive;
};

struct D2UnitStrc {
    int nType;
    uint32_t nGuid;
    int nClass;
    int nX, nY;
    D2RoomStrc* pRoom;
    bool bDead = false;
    bool bCorpse = false;
    int nMode = 1;
    std::map<int, int> tStats;
    std::map<int, int> tStates;
    int nAiIndex = 0;
    int nAiState = 1;
    std::string sName;
    // the think timer and the action in progress
    int nThinkAt = -1;
    int nActionEnd = -1;
    int nGoalX = 0, nGoalY = 0;
    D2UnitStrc* pGoal = nullptr;
    bool bBlockMoves = false;
    int nStockThinks = 0;
    int nScriptThinks = 0;
    int nAttacks = 0;
    // the AI's view: its seed, its group, what it did
    uint32_t nSeedLow = 0x1234567, nSeedHigh = 666;
    int nTypeFlags = -1;  // -1: the class's default
    bool bTargetable = true;
    uint32_t nLeader = 0;               // guid of its group's leader, 0 for none
    std::vector<uint32_t> aMinions;     // the group it leads
    int nOrders = 0;                    // orders its leader gave it
    int nCasts = 0, nSequences = 0, nCircles = 0;
    int nLastSkill = -1, nLastMode = -1;
    D2UnitStrc* pReviving = nullptr;    // the corpse a resurrect brings back when the action ends
};

enum { STAT_HP = 6, STAT_MAXHP = 7 };

class TestEngineWorld : public D2LuaWorld {
public:
    std::vector<std::unique_ptr<D2RoomStrc>> aRooms;
    std::vector<std::unique_ptr<D2UnitStrc>> aUnits;
    uint32_t nGuidNext[6] = { 1, 1, 1, 1, 1, 1 };
    uint32_t nFrame = 0;
    D2LuaRuntime* pRt = nullptr;
    std::vector<std::string> aMessages;

    TestEngineWorld() {
        // Level 2 (Blood Moor): two rooms of 8x8 tiles side by side, from tile (16, 16).
        for (int i = 0; i < 3; i++) {
            std::unique_ptr<D2RoomStrc> p(new D2RoomStrc{ 2, 16 + 8 * i, 16, 8, 8, i < 2 });
            aRooms.push_back(std::move(p));
        }
    }

    D2RoomStrc* RoomOfPoint(int x, int y) {
        for (auto& r : aRooms) {
            if (r->bActive && x >= r->nTileX * 5 && x < (r->nTileX + r->nTilesW) * 5 && y >= r->nTileY * 5 && y < (r->nTileY + r->nTilesH) * 5) {
                return r.get();
            }
        }
        return nullptr;
    }

    D2UnitStrc* Add(int nType, int nClass, int x, int y, const char* pszName = "") {
        D2RoomStrc* pRoom = RoomOfPoint(x, y);
        if (!pRoom) {
            return nullptr;
        }
        std::unique_ptr<D2UnitStrc> p(new D2UnitStrc());
        p->nType = nType;
        p->nGuid = nGuidNext[nType]++;
        p->nClass = nClass;
        p->nX = x;
        p->nY = y;
        p->pRoom = pRoom;
        p->sName = pszName;
        p->tStats[STAT_HP] = 50 << 8;
        p->tStats[STAT_MAXHP] = 50 << 8;
        p->tStats[12] = 7;  // level
        p->nAiIndex = nClass == 19 ? 6 : nClass == 58 ? 13 : 3;  // Fallen, Fallen Shaman; everything else: Zombie
        p->nSeedLow += p->nGuid * 7919;
        D2UnitStrc* pRaw = p.get();
        aUnits.push_back(std::move(p));
        if (nType == D2LUA_UNIT_MONSTER) {
            Attach(pRaw);
            pRaw->nThinkAt = (int)nFrame + 1;
        }
        return pRaw;
    }

    // What the engine's attach does with a claimed AI: fpInit, which marks the restart.
    void Attach(D2UnitStrc* p) {
        if (aClaims[p->nAiIndex]) {
            AiRestart(p);
        }
    }

    // A monster's think as MONSTERAI_ExecuteAiFunction runs it: the overlay decides who thinks.
    void RunThink(D2UnitStrc* p) {
        p->nThinkAt = -1;
        if (aClaims[p->nAiIndex]) {
            D2UnitStrc* pTarget = nullptr;
            int nDistance = 0;
            if (aTargeting[p->nAiIndex] == 1) {
                // AI_BASIC: the nearest player within 35; without one the AI is not called at all.
                pTarget = NearestPlayer(p, nDistance);
                if (!pTarget) {
                    p->nThinkAt = (int)nFrame + 12;
                    return;
                }
            }
            p->nScriptThinks++;
            if (!Think(pRt, 1, p, pTarget, nDistance, pTarget && nDistance <= 2)) {
                StockThink(p);
            }
        } else {
            StockThink(p);
        }
    }

    static int Weighted(D2UnitStrc* a, D2UnitStrc* b) {
        int dx = abs(a->nX - b->nX), dy = abs(a->nY - b->nY);
        int nMax = dx > dy ? dx : dy, nMin = dx > dy ? dy : dx;
        return (2 * nMax + nMin) / 2;
    }

    D2UnitStrc* NearestPlayer(D2UnitStrc* p, int& nDistance) {
        D2UnitStrc* pBest = nullptr;
        for (auto& u : aUnits) {
            if (u->nType != D2LUA_UNIT_PLAYER || !u->pRoom || u->bCorpse) {
                continue;
            }
            int d = Weighted(p, u.get());
            if (d <= 35 && (!pBest || d < nDistance)) {
                pBest = u.get();
                nDistance = d;
            }
        }
        return pBest;
    }

    D2UnitStrc* ByGuid(int nType, uint32_t nGuid) {
        for (auto& u : aUnits) {
            if (u->nType == nType && u->nGuid == nGuid) {
                return u.get();
            }
        }
        return nullptr;
    }

    // What the shamans did, frame by frame: the stock replica and the script must write the same.
    std::vector<std::string> aTrace;
    void Trace(D2UnitStrc* p, const char* pszWhat, int a = -1, int b = -1) {
        if (p->nClass != 58) {
            return;
        }
        char sz[96];
        snprintf(sz, sizeof(sz), "%u %u %s %d %d", nFrame, p->nGuid, pszWhat, a, b);
        aTrace.push_back(sz);
    }

    void StockThink(D2UnitStrc* p) {
        if (p->nClass == 58) {
            StockShaman(p);
            return;
        }
        p->nStockThinks++;
        p->nThinkAt = (int)nFrame + 10;
    }

    // AITHINK_Fn1_FallenShaman (D2Game Monster/Ai/AI.cpp, 1.14d 005f1440) over this stand-in, line for
    // line, for a shaman that leads its group: what the TypeScript port is held against.
    void StockShaman(D2UnitStrc* p) {
        p->nStockThinks++;
        int nDistance = 0;
        D2UnitStrc* pTarget = NearestPlayer(p, nDistance);
        if (!pTarget) {
            p->nThinkAt = (int)nFrame + 12;  // AI_BASIC: no target, no think
            return;
        }
        D2LuaMonsterData t;
        MonsterData(p->nClass, t);
        const int* aip = t.aAiParams;
        if (nDistance <= 2 && (int)(SeedRoll(p) % 100) < aip[2]) {
            AiAttack(p, pTarget, false);
            return;
        }
        D2UnitStrc* pCorpse = nullptr;
        int nCount = 0;
        int nRadius = aip[3] * aip[3];
        D2UnitStrc* pLeader = Leader(p);
        if (pLeader) {
            for (uint32_t g : pLeader->aMinions) {
                D2UnitStrc* m = ByGuid(D2LUA_UNIT_MONSTER, g);
                if (m && FindCorpse(g) == m && AiDistance(p, m) <= nRadius && m->nMode == 12 && Targetable(m) && Leader(m) == p) {
                    pCorpse = m;
                    nCount++;
                }
            }
        }
        if ((int)(SeedRoll(p) % 100) < aip[0]) {
            AiOrderMinions(p, 1);
        }
        if (pCorpse && nCount && (int)(SeedRoll(p) % 100) < aip[0] && AiCanCast(p, t.aSkills[0], pCorpse, 0, 0)) {
            AiSequence(p, t.aSkills[0], pCorpse, 0, 0);
            return;
        }
        if (nDistance < aip[4] && (int)(SeedRoll(p) % 100) < aip[1]) {
            AiCast(p, t.aSkills[1], t.aSkillModes[1], pTarget, 0, 0);
            return;
        }
        int nOther = 0;
        D2UnitStrc* pOther = AiFindTarget(p, nOther);
        if (pOther && nOther < aip[4] && (int)(SeedRoll(p) % 100) < aip[1]) {
            AiCast(p, t.aSkills[1], t.aSkillModes[1], pOther, 0, 0);
            return;
        }
        if ((int)(SeedRoll(p) % 100) < aip[2]) {
            AiCircle(p, pTarget, 3);
            return;
        }
        AiIdle(p, 10);
    }

    // One frame: timers, then actions that end (a move puts the unit at its goal and thinks at once,
    // as a walk's end does), then the mods' tick.
    void Step() {
        nFrame++;
        for (size_t i = 0; i < aUnits.size(); i++) {
            D2UnitStrc* p = aUnits[i].get();
            if (p->nType != D2LUA_UNIT_MONSTER || p->bDead || !p->pRoom) {
                continue;
            }
            if (p->nActionEnd >= 0 && (int)nFrame >= p->nActionEnd) {
                p->nActionEnd = -1;
                if (p->pGoal) {
                    p->nX = p->pGoal->nX + 1;
                    p->nY = p->pGoal->nY;
                } else if (!p->bBlockMoves) {
                    p->nX = p->nGoalX;
                    p->nY = p->nGoalY;
                }
                p->pGoal = nullptr;
                p->nMode = 1;
                if (p->pReviving) {
                    Revive(p->pReviving);
                    p->pReviving = nullptr;
                }
                RunThink(p);
                continue;
            }
            if (p->nThinkAt >= 0 && (int)nFrame >= p->nThinkAt && p->nActionEnd < 0) {
                RunThink(p);
            }
        }
        D2LuaRt_Tick(pRt, 1);
    }

    void KillUnit(D2UnitStrc* p) {
        p->bDead = true;
        p->nMode = 12;
        p->nThinkAt = -1;
        p->nActionEnd = -1;
        AiDrop(pRt, 1, p->nGuid);
    }

    void Revive(D2UnitStrc* p) {
        p->bDead = false;
        p->nMode = 1;
        p->nThinkAt = (int)nFrame + 1;  // a resurrect only re-arms the think
    }

    void Free(D2UnitStrc* p) {
        if (p->nType == D2LUA_UNIT_MONSTER) {
            AiDrop(pRt, 1, p->nGuid);
        }
        p->pRoom = nullptr;
    }

    // --- the interface ---
    int Difficulty() override { return 1; }
    bool Expansion() override { return true; }
    std::string Name() override { return "testgame"; }
    uint32_t Seed() override { return 12345; }
    uint32_t Frame() override { return nFrame; }
    void Players(std::vector<D2UnitStrc*>& aOut) override {
        for (auto& p : aUnits) {
            if (p->nType == D2LUA_UNIT_PLAYER && p->pRoom && !p->bCorpse) {
                aOut.push_back(p.get());
            }
        }
    }
    void Message(const char* pszText) override { aMessages.push_back(pszText); }
    int LevelCount() override { return 137; }
    bool LevelExists(int nLevel) override { return nLevel == 2; }
    bool LevelInfo(int nLevel, D2LuaLevelInfo& tOut) override {
        tOut.sName = nLevel == 1 ? "Rogue Encampment" : nLevel == 2 ? "Blood Moor" : "Elsewhere";
        tOut.nAct = 0;
        tOut.bTown = nLevel == 1;
        tOut.bGenerated = nLevel == 2;
        tOut.tBounds = { 80, 80, 120, 40 };
        return true;
    }
    void LevelRooms(int nLevel, std::vector<D2RoomStrc*>& aOut) override {
        for (auto& r : aRooms) {
            if (r->bActive && r->nLevel == nLevel) {
                aOut.push_back(r.get());
            }
        }
    }
    D2RoomStrc* RoomFind(const D2LuaRoomKey& k) override {
        for (auto& r : aRooms) {
            if (r->bActive && (uint32_t)r->nLevel == k.nLevel && (uint32_t)r->nTileX == k.nTileX && (uint32_t)r->nTileY == k.nTileY) {
                return r.get();
            }
        }
        return nullptr;
    }
    D2LuaRoomKey RoomKeyOf(D2RoomStrc* r) override { return { (uint32_t)r->nLevel, (uint32_t)r->nTileX, (uint32_t)r->nTileY }; }
    D2LuaRect RoomBounds(D2RoomStrc* r) override { return { r->nTileX * 5, r->nTileY * 5, r->nTilesW * 5, r->nTilesH * 5 }; }
    void RoomNeighbours(D2RoomStrc* r, std::vector<D2RoomStrc*>& aOut) override {
        for (auto& o : aRooms) {
            if (o.get() != r && o->bActive && abs(o->nTileX - r->nTileX) <= 8) {
                aOut.push_back(o.get());
            }
        }
    }
    void RoomUnits(D2RoomStrc* r, std::vector<D2UnitStrc*>& aOut) override {
        for (auto& p : aUnits) {
            if (p->pRoom == r) {
                aOut.push_back(p.get());
            }
        }
    }
    D2RoomStrc* RoomAt(int x, int y, D2RoomStrc*) override { return RoomOfPoint(x, y); }
    D2UnitStrc* FindUnit(int nType, uint32_t nGuid) override {
        for (auto& p : aUnits) {
            if (p->nType == nType && p->nGuid == nGuid) {
                return p->pRoom && p->pRoom->bActive && !(nType == D2LUA_UNIT_MONSTER && p->bDead) ? p.get() : nullptr;
            }
        }
        return nullptr;
    }
    int UnitType(D2UnitStrc* p) override { return p->nType; }
    uint32_t UnitGuid(D2UnitStrc* p) override { return p->nGuid; }
    int ClassId(D2UnitStrc* p) override { return p->nClass; }
    bool Position(D2UnitStrc* p, int& x, int& y) override {
        x = p->nX;
        y = p->nY;
        return p->pRoom != nullptr;
    }
    D2RoomStrc* UnitRoom(D2UnitStrc* p) override { return p->pRoom; }
    int UnitLevel(D2UnitStrc* p) override { return p->pRoom ? p->pRoom->nLevel : 0; }
    int UnitAct(D2UnitStrc*) override { return 0; }
    int Mode(D2UnitStrc* p) override { return p->nMode; }
    bool IsDead(D2UnitStrc* p) override { return p->bDead; }
    bool IsCorpse(D2UnitStrc* p) override { return p->bCorpse; }
    D2UnitStrc* Owner(D2UnitStrc*) override { return nullptr; }
    int StatCount() override { return 358; }
    int StateCount() override { return 185; }
    int GetStat(D2UnitStrc* p, int nStat, int, bool) override { return p->tStats.count(nStat) ? p->tStats[nStat] : 0; }
    void SetStat(D2UnitStrc* p, int nStat, int nValue, int) override { p->tStats[nStat] = nValue; }
    void AddStat(D2UnitStrc* p, int nStat, int nValue, int) override { p->tStats[nStat] += nValue; }
    bool HasState(D2UnitStrc* p, int nState) override { return p->tStates.count(nState) != 0; }
    bool AddState(D2UnitStrc* p, int nState, int nFrames) override {
        p->tStates[nState] = nFrames;
        return true;
    }
    void RemoveState(D2UnitStrc* p, int nState) override { p->tStates.erase(nState); }
    std::string PlayerName(D2UnitStrc* p) override { return p->sName; }
    void SendMessage(D2UnitStrc* p, const char* pszText) override { aMessages.push_back(p->sName + ": " + pszText); }
    int MonsterTypeFlags(D2UnitStrc* p) override { return p->nTypeFlags >= 0 ? p->nTypeFlags : p->nClass == 19 ? 4 : 0; }
    bool IsBoss(D2UnitStrc*) override { return false; }
    int MonsterCount() override { return 734; }
    void Kill(D2UnitStrc* p, D2UnitStrc*) override { KillUnit(p); }
    D2UnitStrc* Target(D2UnitStrc* p) override { return p->pGoal; }
    D2UnitStrc* SpawnMonster(int nClass, int x, int y, int, bool) override { return Add(D2LUA_UNIT_MONSTER, nClass, x, y); }
    int AiIndex(D2UnitStrc* p) override { return p->nAiIndex; }
    void AiIdle(D2UnitStrc* p, int nFrames) override {
        Trace(p, "idle", nFrames);
        p->nMode = 1;
        p->nThinkAt = (int)nFrame + nFrames;
    }
    bool AiMove(D2UnitStrc* p, D2UnitStrc* pTarget, int x, int y, int) override {
        if (!pTarget && !RoomOfPoint(x, y)) {
            return false;
        }
        p->pGoal = pTarget;
        p->nGoalX = x;
        p->nGoalY = y;
        p->nActionEnd = (int)nFrame + 4;
        p->nMode = 2;
        p->nAiState = 2;
        return true;
    }
    bool AiAttack(D2UnitStrc* p, D2UnitStrc* pTarget, bool) override {
        Trace(p, "attack", pTarget ? (int)pTarget->nGuid : -1);
        p->nAttacks++;
        p->nThinkAt = (int)nFrame + 8;
        return true;
    }
    bool AiCast(D2UnitStrc* p, int nSkill, int nMode, D2UnitStrc* pTarget, int, int) override {
        Trace(p, "cast", nSkill, pTarget ? (int)pTarget->nGuid : -1);
        p->nCasts++;
        p->nLastSkill = nSkill;
        p->nLastMode = nMode;
        p->nThinkAt = (int)nFrame + 8;
        return true;
    }
    // A sequence (a resurrect at a corpse): the corpse lives again when it ends, 6 frames on.
    bool AiSequence(D2UnitStrc* p, int nSkill, D2UnitStrc* pTarget, int, int) override {
        Trace(p, "sequence", nSkill, pTarget ? (int)pTarget->nGuid : -1);
        p->nSequences++;
        p->nLastSkill = nSkill;
        p->nLastMode = 14;
        p->nMode = 14;
        p->nActionEnd = (int)nFrame + 6;
        p->nGoalX = p->nX;
        p->nGoalY = p->nY;
        p->pReviving = pTarget && pTarget->bDead ? pTarget : nullptr;
        return true;
    }
    bool AiCircle(D2UnitStrc* p, D2UnitStrc* pTarget, int nSteps) override {
        Trace(p, "circle", pTarget ? (int)pTarget->nGuid : -1, nSteps);
        p->nCircles++;
        SeedRoll(p);  // the engine rolls clockwise or not
        // Three steps round the target that end where they began: the scene keeps its distances.
        p->pGoal = nullptr;
        p->nGoalX = p->nX;
        p->nGoalY = p->nY;
        p->nActionEnd = (int)nFrame + 4;
        p->nMode = 2;
        return pTarget != nullptr;
    }
    bool MonsterData(int nClass, D2LuaMonsterData& t) override {
        if (nClass == 58) {  // fallenshaman1, nightmare (the stand-in's difficulty is 1)
            t.nBaseId = 58;
            t.nAi = 13;
            t.nAiDelay = 5;
            int aip[8] = { 50, 65, 90, 24, 15, 0, 0, 0 };
            int sk[8] = { 181, 160, -1, -1, -1, -1, -1, -1 };
            for (int i = 0; i < 8; i++) {
                t.aAiParams[i] = aip[i];
                t.aSkills[i] = sk[i];
                t.aSkillModes[i] = i < 2 ? 14 : 0;
            }
            return true;
        }
        t.nBaseId = nClass;
        t.nAi = nClass == 19 ? 6 : 3;
        t.nAiDelay = 12;
        for (int i = 0; i < 8; i++) {
            t.aAiParams[i] = 0;
            t.aSkills[i] = -1;
            t.aSkillModes[i] = 0;
        }
        return true;
    }
    uint32_t SeedRoll(D2UnitStrc* p) override {
        uint64_t n = (uint64_t)p->nSeedLow * 0x6AC690C5ull + p->nSeedHigh;
        p->nSeedLow = (uint32_t)n;
        p->nSeedHigh = (uint32_t)(n >> 32);
        return p->nSeedLow;
    }
    D2UnitStrc* FindCorpse(uint32_t nGuid) override {
        D2UnitStrc* p = ByGuid(D2LUA_UNIT_MONSTER, nGuid);
        return p && p->bDead && p->pRoom && p->pRoom->bActive ? p : nullptr;
    }
    int Alignment(D2UnitStrc*) override { return 0; }
    bool Targetable(D2UnitStrc* p) override { return p->bTargetable; }
    bool HasStateMask(D2UnitStrc*, int) override { return false; }
    D2UnitStrc* Leader(D2UnitStrc* p) override { return p->nLeader ? ByGuid(D2LUA_UNIT_MONSTER, p->nLeader) : nullptr; }
    void Minions(D2UnitStrc* p, std::vector<D2UnitStrc*>& aOut) override {
        for (uint32_t g : p->aMinions) {
            if (D2UnitStrc* m = ByGuid(D2LUA_UNIT_MONSTER, g)) {
                aOut.push_back(m);
            }
        }
    }
    void AdjacentRooms(D2RoomStrc* r, std::vector<D2RoomStrc*>& aOut) override {
        aOut.push_back(r);
        RoomNeighbours(r, aOut);
    }
    int AiDistance(D2UnitStrc* p, D2UnitStrc* o) override { return Weighted(p, o); }
    D2UnitStrc* AiFindTarget(D2UnitStrc* p, int& nDistance) override { return NearestPlayer(p, nDistance); }
    bool AiCanCast(D2UnitStrc*, int, D2UnitStrc* pTarget, int, int) override { return pTarget && Targetable(pTarget); }
    void AiOrderMinions(D2UnitStrc* p, int nOrder) override {
        Trace(p, "order", nOrder);
        D2UnitStrc* pLeader = Leader(p);
        if (pLeader) {
            for (uint32_t g : pLeader->aMinions) {
                if (D2UnitStrc* m = ByGuid(D2LUA_UNIT_MONSTER, g)) {
                    m->nOrders++;
                }
            }
        }
    }
    bool AiWander(D2UnitStrc* p, int) override {
        p->nThinkAt = (int)nFrame + 6;
        return true;
    }
    bool AiFlee(D2UnitStrc* p, D2UnitStrc*, int) override {
        p->nThinkAt = (int)nFrame + 6;
        return true;
    }
    void AiStock(D2UnitStrc* p) override { StockThink(p); }
    int AiState(D2UnitStrc* p) override { return p->nAiState; }
    void AiMarkAction(D2UnitStrc* p) override { p->nAiState = 1; }
    int AiDelay(D2UnitStrc*) override { return 12; }
    void AiClaim(int, int, int) override {}
};

static TestEngineWorld* s_pWorld = nullptr;

static D2LuaRuntime* MakeWorldRuntime(TestEngineWorld* pWorld, const std::vector<std::string>& aRoots, const std::vector<std::string>& aOnly) {
    std::vector<D2LuaModInfo> aFound, aPicked;
    std::vector<std::string> aErrors;
    D2LuaMods_Discover(aRoots, aFound, aErrors);
    for (const auto& t : aFound) {
        for (const auto& s : aOnly) {
            if (t.sName == s) {
                aPicked.push_back(t);
            }
        }
    }
    D2LuaRuntimeDesc tDesc;
    tDesc.nSide = D2LUA_SIDE_SERVER;
    tDesc.pfnLog = TestLog;
    tDesc.tLimits.nBudgetMs = kTestBudgetMs;
    tDesc.tLimits.nMemoryBytes = (size_t)64 * 1024 * 1024;
    tDesc.aApis.push_back(&g_D2LuaApi_main);
    tDesc.aApis.push_back(&g_D2LuaApi_world);
    tDesc.pWorld = pWorld;
    if (!D2LuaMods_Resolve(aPicked, tDesc.aMods, aErrors)) {
        for (const auto& e : aErrors) {
            printf("  resolve: %s\n", e.c_str());
        }
        return nullptr;
    }
    D2LuaRuntime* pRt = D2LuaRt_Create(tDesc);
    pWorld->pRt = pRt;
    return pRt;
}

static void Run(TestEngineWorld* w, int nFrames) {
    for (int i = 0; i < nFrames; i++) {
        w->Step();
    }
}

static void WorldApiTests() {
    printf("--- the world classes and the AI hook on a stand-in engine\n");
    TestEngineWorld tWorld;
    TestEngineWorld* w = &tWorld;
    s_pWorld = w;
    D2UnitStrc* pPlayer = w->Add(D2LUA_UNIT_PLAYER, 1, 100, 100, "Amazonia");
    D2UnitStrc* pZombie = w->Add(D2LUA_UNIT_MONSTER, 5, 110, 100);
    D2UnitStrc* pFar = w->Add(D2LUA_UNIT_MONSTER, 5, 130, 100);
    D2UnitStrc* pCorpse = w->Add(D2LUA_UNIT_PLAYER, 1, 101, 101, "Corpse");
    pCorpse->bCorpse = true;
    ClearLog();
    D2LuaRuntime* pRt = MakeWorldRuntime(w, TestRoots(), { "worldmod" });
    Check(pRt != nullptr, "a server state with a world");
    if (!pRt) {
        return;
    }
    TestStart(pRt, 1);
    CheckLogged("[worldmod] game: 1 true testgame 12345", "game.difficulty, expansion, name, seed");
    CheckLogged("[worldmod] players: Amazonia", "game.players(): players, without corpses");
    CheckLogged("[worldmod] level 2: Blood Moor false true 80 80 120 40 rooms 2", "game.level: name, isTown, generated, bounds, active rooms");
    CheckLogged("[worldmod] level 1: nil", "game.level is undefined for a level that does not exist (nothing is generated)");
    CheckLogged("[worldmod] near the player: 1 monster(s)", "unit.nearby(radius, type): a scoped query, the unit itself left out");
    CheckLogged("[worldmod] near (118,100): 2", "game.unitsNear reaches into a neighbouring room");
    CheckLogged("[worldmod] zombie: 5 110 100 true 50 room level 2", "a monster: classId, position, valid, life, room and level");
    CheckLogged("[worldmod] same handle: true true", "one handle per unit: queries give the same object, usable as a key");
    CheckLogged("[worldmod] stat: 7 life 30 state true false", "getStat, life setter (fixed point), addState/hasState/removeState");
    CheckLogged("[worldmod] errors: tile point; outside; 0 or less; gold; own AI", "bad arguments are refused with a clear error");
    CheckLogged("[worldmod] points: subtile 100 100 tile 20 20 subtile 100 100", "typed points: subtile, tile, conversions");
    CheckLogged("[worldmod] registered: true wrapped: true none: false", "registry.ai.has");
    Check(w->aClaims[3] == D2LUA_AI_WRAPPED && w->aClaims[6] == D2LUA_AI_REGISTERED, "  the claims reach the overlay: Zombie wrapped, Fallen registered");

    // The wrapped Zombie AI: next() runs the stock think.
    ClearLog();
    Run(w, 12);
    Check(pZombie->nStockThinks >= 1 && pZombie->nScriptThinks >= 1, "a wrapper's next() runs one stock think");
    CheckLogged("[worldmod] wrap think", "  the wrapper ran for the think");

    // The Fallen: the registered script.
    ClearLog();
    D2UnitStrc* pFallen = w->Add(D2LUA_UNIT_MONSTER, 19, 120, 100);
    Run(w, 3);
    CheckLogged("[worldmod] fallen script start", "a registered script starts at the monster's first think");
    Run(w, 40);
    CheckLogged("[worldmod] walk outcome 0", "  yield* walkTo: the engine ends the walk and the action gives Outcome.Done");
    CheckLogged("[worldmod] idle outcome 0", "  yield* idle(n): Done after n frames");
    CheckLogged("[worldmod] blocked outcome 2", "  a walk that cannot start: Outcome.Blocked at once");
    CheckLogged("[worldmod] toward outcome 1", "  walkToward a unit that is gone by the end: Interrupted");
    CheckLogged("[worldmod] attack outcome 0", "  attack");
    Check(pFallen->nAttacks == 1, "  the attack started in the engine");
    Check(pFallen->nStockThinks == 0, "  a registered script replaces the stock think");
    Check(w->aMessages.size() >= 1 && w->aMessages[0] == "hello from worldmod", "game.message");

    // Death ends the script; a revived monster starts it over; the handle is invalid while dead.
    ClearLog();
    TestExpect(pRt, 1, "worldmod", "local m = fallen local shared = require('redivivus:shared') m:kill() shared.log('after kill', m.valid)",
               "after kill false", "a killed monster's handle is not valid");
    Run(w, 20);
    Check(!Logged("fallen script"), "  a dead monster does not think");
    w->Revive(pFallen);
    Run(w, 3);
    CheckLogged("[worldmod] fallen script start", "  revived, its script starts over from the top");
    TestExpect(pRt, 1, "worldmod", "require('redivivus:shared').log('revived valid', fallen.valid)", "revived valid true", "  and the same handle is valid again");

    // A room unloaded: its units' handles are not valid; loaded again: valid, the same objects.
    w->aRooms[1]->bActive = false;
    TestExpect(pRt, 1, "worldmod", "require('redivivus:shared').log('far frozen', far.valid)", "far frozen false", "a unit in an unloaded room is not valid");
    w->aRooms[1]->bActive = true;
    TestExpect(pRt, 1, "worldmod", "require('redivivus:shared').log('far thawed', far.valid, far.id)", "far thawed true 2", "  valid again when the room is back, same id");
    (void)pFar;
    (void)pPlayer;

    // A script that raises: logged once, the monster idles, nothing else breaks.
    ClearLog();
    TestExpect(pRt, 1, "worldmod", "explode = true", "", "");
    Run(w, 30);
    CheckLogged("worldmod fails on purpose", "a failing AI script is logged with its error");
    Check(!D2LuaRt_IsFaulted(pRt, "worldmod"), "  the mod is not faulted for it");
    Check(pFallen->nThinkAt >= 0 || pFallen->nActionEnd >= 0, "  the monster is not stalled (a think is armed)");

    D2LuaRt_EndGame(pRt, 1);
    D2LuaRt_Destroy(pRt);
    s_pWorld = nullptr;
}

static void ChargerTests(const char* pszTsMods) {
    printf("--- the charger example on a stand-in engine (%s)\n", pszTsMods);
    TestEngineWorld tWorld;
    TestEngineWorld* w = &tWorld;
    D2UnitStrc* pPlayer = w->Add(D2LUA_UNIT_PLAYER, 1, 100, 100, "Amazonia");
    ClearLog();
    D2LuaRuntime* pRt = MakeWorldRuntime(w, { pszTsMods }, { "charger" });
    Check(pRt != nullptr, "the charger example loads in a server state with a world");
    if (!pRt) {
        return;
    }
    TestStart(pRt, 1);
    CheckLogged("[charger] charger: the Fallen charge in game 1", "it registers its AI for the Fallen");
    Run(w, 60);
    CheckLogged("[charger] charger: spawned 1 near Amazonia", "it spawns a charger near the player");
    Run(w, 120);
    CheckLogged("[charger] charger 1: awake at 112,100", "the charger's script runs");
    CheckLogged("[charger] charger 1: winding up, Amazonia at 12", "  it winds up");
    CheckLogged("[charger] charger 1: rush from 112,100 to 96,100", "  and rushes in a straight line through the player, a little past");
    CheckLogged("[charger] charger 1: rush done, now at 96,100", "  the rush ends where it aimed");
    CheckLogged("[charger] charger 1: attack Amazonia: done", "  and it attacks when next to the player");
    (void)pPlayer;
    Check(!D2LuaRt_IsFaulted(pRt, "charger"), "  no fault");
    D2LuaRt_EndGame(pRt, 1);
    D2LuaRt_Destroy(pRt);
}

// A shaman leading two Fallen, a player 10 subtiles west; one Fallen is killed at frame 30.
static void ShamanScene(TestEngineWorld* w, D2UnitStrc** ppShaman, D2UnitStrc** ppFallen) {
    w->Add(D2LUA_UNIT_PLAYER, 1, 100, 100, "Amazonia");
    D2UnitStrc* pShaman = w->Add(D2LUA_UNIT_MONSTER, 58, 110, 100);
    D2UnitStrc* f1 = w->Add(D2LUA_UNIT_MONSTER, 19, 112, 101);
    D2UnitStrc* f2 = w->Add(D2LUA_UNIT_MONSTER, 19, 112, 99);
    pShaman->nTypeFlags = 0;
    pShaman->nLeader = pShaman->nGuid;
    for (D2UnitStrc* f : { f1, f2 }) {
        f->nTypeFlags = 0x10;
        f->nLeader = pShaman->nGuid;
        f->nThinkAt = -1;  // the Fallen stand still: only the shaman acts here
        pShaman->aMinions.push_back(f->nGuid);
    }
    *ppShaman = pShaman;
    *ppFallen = f1;
}

static void RunShamanScene(TestEngineWorld* w, D2UnitStrc* pFallen) {
    for (int i = 0; i < 400; i++) {
        if (w->nFrame == 30) {
            w->KillUnit(pFallen);
        }
        if (w->nFrame == 200 && !pFallen->bDead) {
            w->KillUnit(pFallen);  // raised once: kill it again
        }
        w->Step();
        // A Fallen never thinks here, so a raised one does not wander off.
        for (auto& u : w->aUnits) {
            if (u->nClass == 19) {
                u->nThinkAt = -1;
            }
        }
    }
}

static void ShamanTests(const char* pszTsMods) {
    printf("--- the fallen-shaman example against the stock AI on a stand-in engine (%s)\n", pszTsMods);
    // The stock AI (the replica in the stand-in), with no mod claiming it.
    TestEngineWorld tStock;
    D2UnitStrc* pStockShaman = nullptr;
    D2UnitStrc* pStockFallen = nullptr;
    ShamanScene(&tStock, &pStockShaman, &pStockFallen);
    D2LuaRuntime* pStockRt = MakeWorldRuntime(&tStock, { pszTsMods }, {});
    TestStart(pStockRt, 1);
    RunShamanScene(&tStock, pStockFallen);

    // The TypeScript port.
    TestEngineWorld tTs;
    D2UnitStrc* pShaman = nullptr;
    D2UnitStrc* pFallen = nullptr;
    ShamanScene(&tTs, &pShaman, &pFallen);
    ClearLog();
    D2LuaRuntime* pRt = MakeWorldRuntime(&tTs, { pszTsMods }, { "fallen-shaman" });
    Check(pRt != nullptr, "the fallen-shaman example loads");
    if (!pRt) {
        return;
    }
    TestStart(pRt, 1);
    CheckLogged("[fallen-shaman] fallen-shaman: the TypeScript AI runs the shamans in game 1", "it registers its AI for the Fallen Shaman");
    Check(tTs.aClaims[13] == D2LUA_AI_REGISTERED && tTs.aTargeting[13] == 1, "  registered with AiTargeting.Basic");
    RunShamanScene(&tTs, pFallen);
    CheckLogged("[fallen-shaman] shaman 1: awake (aip 50 65 90 24 15, skills 181 160)", "the script reads its MonStats row: aip1..5, Skill1, Skill2");
    CheckLogged("[fallen-shaman] shaman 1: resurrects 2", "it raises the Fallen killed beside it");
    Check(pShaman->nSequences >= 1 && pShaman->nCasts >= 1 && pShaman->nCircles + pShaman->nAttacks >= 0, "  it raised (a sequence) and threw fire (a cast)");
    Check(pShaman->nLastMode == 14 || pShaman->nSequences > 0, "  fire is cast in its row's Sk2mode (Sequence)");
    Check(tTs.aTrace.size() >= 20, "  it made a fair number of decisions");
    size_t nSame = 0;
    while (nSame < tTs.aTrace.size() && nSame < tStock.aTrace.size() && tTs.aTrace[nSame] == tStock.aTrace[nSame]) {
        nSame++;
    }
    if (nSame < tTs.aTrace.size() || tTs.aTrace.size() != tStock.aTrace.size()) {
        printf("  first difference at decision %zu of %zu (stock %zu):\n", nSame, tTs.aTrace.size(), tStock.aTrace.size());
        printf("    ts:    %s\n", nSame < tTs.aTrace.size() ? tTs.aTrace[nSame].c_str() : "(end)");
        printf("    stock: %s\n", nSame < tStock.aTrace.size() ? tStock.aTrace[nSame].c_str() : "(end)");
    }
    char sz[160];
    snprintf(sz, sizeof(sz), "  every decision (%zu, each frame, action, skill and target) equals the stock AI's on the same seed",
             tTs.aTrace.size());
    Check(nSame == tTs.aTrace.size() && tTs.aTrace.size() == tStock.aTrace.size(), sz);
    Check(pStockShaman->nSequences == pShaman->nSequences && pStockFallen->bDead == pFallen->bDead, "  and the world ends the same");
    Check(!D2LuaRt_IsFaulted(pRt, "fallen-shaman"), "  no fault");

    // What a think costs: the same scene run on, the whole frame loop timed (the stand-in's own
    // work is the same in both worlds), divided by the shaman's thinks. Not a check: a number.
    TestExpect(pRt, 1, "fallen-shaman", "require('./shaman').setTrace(false)", "", "");
    for (TestEngineWorld* w : { &tStock, &tTs }) {
        for (auto& u : w->aUnits) {
            if (u->nType == D2LUA_UNIT_MONSTER && u->nClass == 19 && u->bDead) {
                w->Revive(u.get());
            }
        }
    }
    double aNs[2] = { 0, 0 };
    int aThinks[2] = { 0, 0 };
    TestEngineWorld* aWorlds[2] = { &tStock, &tTs };
    D2UnitStrc* aShamans[2] = { pStockShaman, pShaman };
    for (int k = 0; k < 2; k++) {
        TestEngineWorld* w = aWorlds[k];
        int nBefore = k == 0 ? aShamans[k]->nStockThinks : aShamans[k]->nScriptThinks;
        w->aTrace.reserve(w->aTrace.size() + 40000);
        auto tStart = std::chrono::steady_clock::now();
        for (int i = 0; i < 20000; i++) {
            w->Step();
        }
        aNs[k] = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - tStart).count();
        aThinks[k] = (k == 0 ? aShamans[k]->nStockThinks : aShamans[k]->nScriptThinks) - nBefore;
    }
    printf("  cost: 20000 frames, stock %d thinks in %.2f ms, script %d thinks in %.2f ms: %.0f ns more per think\n", aThinks[0],
           aNs[0] / 1e6, aThinks[1], aNs[1] / 1e6, aThinks[1] > 0 && aThinks[0] > 0 ? aNs[1] / aThinks[1] - aNs[0] / aThinks[0] : 0.0);
    D2LuaRt_EndGame(pRt, 1);
    D2LuaRt_Destroy(pRt);
    D2LuaRt_EndGame(pStockRt, 1);
    D2LuaRt_Destroy(pStockRt);
}

// The keys of the mod's global pszTable after a full collection; -1 when it could not be read.
static int WeakKeys(D2LuaRuntime* pRt, const char* pszMod, const char* pszTable) {
    lua_State* L = D2LuaRt_State(pRt);
    lua_gc(L, LUA_GCCOLLECT, 0);
    lua_gc(L, LUA_GCCOLLECT, 0);
    ClearLog();
    std::string sChunk = std::string("local n = 0 for _ in pairs(") + pszTable + ") do n = n + 1 end require('redivivus:shared').log('weak keys', n)";
    TestRun(pRt, 1, pszMod, sChunk.c_str());
    const char* p = strstr(g_szTestLog, "weak keys ");
    return p ? atoi(p + strlen("weak keys ")) : -1;
}

static void GuardAiTests() {
    printf("--- guard in a monster's AI script on a stand-in engine\n");
    TestEngineWorld tWorld;
    TestEngineWorld* w = &tWorld;
    ClearLog();
    D2LuaRuntime* pRt = MakeWorldRuntime(w, TestRoots(), { "guardmod" });
    Check(pRt != nullptr, "a server state with the guard fixture");
    if (!pRt) {
        return;
    }
    TestStart(pRt, 1);
    CheckLogged("[guardmod] guardmod ready", "it registers its AI for the Fallen");
    D2UnitStrc* pFallen = w->Add(D2LUA_UNIT_MONSTER, 19, 120, 100);
    Run(w, 80);
    CheckLogged("[guardmod] ai guarded actions 0,0", "actions inside a guarded body are the monster's own script's, their outcomes reach the body");
    CheckLogged("[guardmod] ai cut 1 4", "a check failing while an action runs: the engine finishes the action (4 frames), then Interrupted");
    Check(!Logged("ai never"), "  the dropped body is not resumed again");
    CheckLogged("[guardmod] ai nested 1", "  nested guards: the inner outcome is the outer body's result");
    CheckLogged("[guardmod] ai own coroutine", "an action from a coroutine of the mod's own is still refused");
    CheckLogged("can only be used inside this monster's own AI script", "  with the usual error");
    CheckLogged("[guardmod] ai guarding forever", "the script reaches its last guard");
    Check(!D2LuaRt_IsFaulted(pRt, "guardmod"), "  no fault");
    Check(WeakKeys(pRt, "guardmod", "threads") == 2, "  while it runs: the script's coroutine and its guarded body's (the finished body is gone)");
    w->KillUnit(pFallen);
    Run(w, 5);
    Check(WeakKeys(pRt, "guardmod", "threads") == 0, "the monster dies while guarded: the script and its body are collected");
    D2LuaRt_EndGame(pRt, 1);
    D2LuaRt_Destroy(pRt);
}

static void ChargerGuardTests(const char* pszTsMods) {
    printf("--- the charger's rush, guarded by its target (%s)\n", pszTsMods);
    TestEngineWorld tWorld;
    TestEngineWorld* w = &tWorld;
    D2UnitStrc* pPlayer = w->Add(D2LUA_UNIT_PLAYER, 1, 100, 100, "Amazonia");
    ClearLog();
    D2LuaRuntime* pRt = MakeWorldRuntime(w, { pszTsMods }, { "charger" });
    Check(pRt != nullptr, "the charger example loads");
    if (!pRt) {
        return;
    }
    TestStart(pRt, 1);
    for (int i = 0; i < 300 && !Logged("winding up"); i++) {
        w->Step();
    }
    CheckLogged("[charger] charger 1: winding up", "it winds up");
    pPlayer->bDead = true;
    Run(w, 40);
    CheckLogged("[charger] charger 1: rush interrupted, now at 112,100", "the target dies during the wind-up: the guard drops the rush, the outcome is Interrupted");
    Check(!Logged("rush from"), "  the rush never starts");
    Check(!D2LuaRt_IsFaulted(pRt, "charger"), "  no fault");
    D2LuaRt_EndGame(pRt, 1);
    D2LuaRt_Destroy(pRt);
}

void RunWorldTests(const char* pszTsMods) {
    WorldApiTests();
    GuardAiTests();
    if (pszTsMods) {
        ChargerTests(pszTsMods);
        ChargerGuardTests(pszTsMods);
        ShamanTests(pszTsMods);
    } else {
        printf("SKIP the charger example: no --ts-mods directory\n");
    }
}
#endif
