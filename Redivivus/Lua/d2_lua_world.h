// The world a server state's mods see: the game's units, rooms and levels, and the monster AI
// hook, as the natives the core builds the classes of redivivus:server on (Game, Level, Room,
// Unit, Monster, Player; core/internal/world.ts, core/internal/ai.ts).
//
// D2LuaWorld is the interface: the game implements it over its SGAMEDATA (engine/
// d2_lua_engine.cpp, the game's build only), the host test over a stand-in world
// (test/d2_lua_test_engine.cpp). This file and d2_lua_world.cpp hold what both share: the natives
// (argument checks, handles, the scoped unit queries) and the bookkeeping of a monster's think.
//
// Engine objects are passed as opaque pointers and never reach Lua: a handle names a unit by
// (type, guid), a room by (level, origin) and a level by its id, and is resolved on every use.
// A unit resolves when it exists, has a room, is not marked deleted and, for a monster, is alive.
#pragma once

#if D2_FEATURE_MODS
#include <stdint.h>

#include <set>
#include <string>
#include <vector>

#include "Redivivus/Lua/d2_lua_api.h"

struct D2UnitStrc;
struct D2RoomStrc;
struct D2LuaRuntime;

enum { D2LUA_UNIT_PLAYER = 0, D2LUA_UNIT_MONSTER = 1, D2LUA_UNIT_TYPES = 6 };
// The stock AI ids (MonStats "AI", rows of monai.txt): the overlay covers these.
enum { D2LUA_AI_STOCK_COUNT = 148 };

struct D2LuaRect {
    int nX, nY, nWidth, nHeight;  // subtiles
};

struct D2LuaLevelInfo {
    std::string sName;  // UTF-8
    int nAct = 0;
    bool bTown = false;
    bool bGenerated = false;
    D2LuaRect tBounds = { 0, 0, 0, 0 };  // valid when generated
};

// A room's stable name: its level and the tile origin of its RoomEx. It survives the room being
// unloaded and loaded again; the room pointer does not.
struct D2LuaRoomKey {
    uint32_t nLevel, nTileX, nTileY;
};

// A MonStats row as an AI reads it, for the game's difficulty.
struct D2LuaMonsterData {
    int nBaseId = 0;
    int nAi = 0;
    int nAiDelay = 0;
    int aAiParams[8] = {};
    int aSkills[8] = {};
    int aSkillModes[8] = {};
};

// The states.txt masks a script can test (redivivus:enums StateMask).
enum { D2LUA_STATEMASK_COUNT = 8 };

// How a script registered for an AI id takes the monster's think (what the overlay returns).
enum D2LuaAiClaim { D2LUA_AI_UNCLAIMED = 0, D2LUA_AI_WRAPPED = 1, D2LUA_AI_REGISTERED = 2 };

enum D2LuaAiMove { D2LUA_MOVE_WALK = 0, D2LUA_MOVE_RUN = 1 };

// The engine's target scan as Mephisto's AI makes it (ForEach with CollectMephistoTargetInfo): the
// units it could attack within a squared distance of 1024. "When player" is filled only when the
// scanning unit is a player (the engine tests the scanner's type, not the target's).
struct D2LuaTargetScan {
    D2UnitStrc* pNearest = nullptr;
    int nNearestDistance = 0x7fffffff;
    D2UnitStrc* pWeakest = nullptr;
    int nWeakestLife = 0x7fffffff;
    D2UnitStrc* pNearestWhenPlayer = nullptr;
    int nNearestWhenPlayerDistance = 0x7fffffff;
    int nCount = 0;
    int nCloseCount = 0;  // within a squared distance of 225
};

class D2LuaWorld {
public:
    virtual ~D2LuaWorld() {}

    // --- the game ---------------------------------------------------------------------------------
    virtual int Difficulty() = 0;
    virtual bool Expansion() = 0;
    virtual std::string Name() = 0;
    virtual uint32_t Seed() = 0;
    virtual uint32_t Frame() = 0;
    // The control units of the clients that are in the game.
    virtual void Players(std::vector<D2UnitStrc*>& aOut) = 0;
    // A system line to every client in the game, text at most 255 bytes, no 0xFF.
    virtual void Message(const char* pszText) = 0;

    // --- levels and rooms -------------------------------------------------------------------------
    virtual int LevelCount() = 0;
    // Whether the level exists in the game now (allocated); never allocates or generates one.
    virtual bool LevelExists(int nLevel) = 0;
    // What the tables say about any level id, and the bounds when it exists and is generated.
    virtual bool LevelInfo(int nLevel, D2LuaLevelInfo& tOut) = 0;
    // The level's active rooms.
    virtual void LevelRooms(int nLevel, std::vector<D2RoomStrc*>& aOut) = 0;
    virtual D2RoomStrc* RoomFind(const D2LuaRoomKey& tKey) = 0;
    virtual D2LuaRoomKey RoomKeyOf(D2RoomStrc* pRoom) = 0;
    virtual D2LuaRect RoomBounds(D2RoomStrc* pRoom) = 0;
    // The active rooms next to it, itself excluded.
    virtual void RoomNeighbours(D2RoomStrc* pRoom, std::vector<D2RoomStrc*>& aOut) = 0;
    // Every unit in the room's list (players' corpses included; the query filters).
    virtual void RoomUnits(D2RoomStrc* pRoom, std::vector<D2UnitStrc*>& aOut) = 0;
    // The active room holding a subtile, near a room the caller has (or any), or null.
    virtual D2RoomStrc* RoomAt(int nX, int nY, D2RoomStrc* pNear) = 0;

    // --- units ------------------------------------------------------------------------------------
    // The usable unit (see the top of the file), or null. Type is checked by the caller (0..5).
    virtual D2UnitStrc* FindUnit(int nType, uint32_t nGuid) = 0;
    virtual int UnitType(D2UnitStrc* pUnit) = 0;
    virtual uint32_t UnitGuid(D2UnitStrc* pUnit) = 0;
    virtual int ClassId(D2UnitStrc* pUnit) = 0;
    virtual bool Position(D2UnitStrc* pUnit, int& nX, int& nY) = 0;
    virtual D2RoomStrc* UnitRoom(D2UnitStrc* pUnit) = 0;
    virtual int UnitLevel(D2UnitStrc* pUnit) = 0;
    virtual int UnitAct(D2UnitStrc* pUnit) = 0;
    virtual int Mode(D2UnitStrc* pUnit) = 0;
    virtual bool IsDead(D2UnitStrc* pUnit) = 0;
    // A player's corpse: in a room's list, never a player a mod gets.
    virtual bool IsCorpse(D2UnitStrc* pUnit) = 0;
    virtual D2UnitStrc* Owner(D2UnitStrc* pUnit) = 0;
    virtual int StatCount() = 0;
    virtual int StateCount() = 0;
    // Stat values as the engine keeps them; the six fixed-point stats are shifted by the core.
    virtual int GetStat(D2UnitStrc* pUnit, int nStat, int nLayer, bool bBase) = 0;
    virtual void SetStat(D2UnitStrc* pUnit, int nStat, int nValue, int nLayer) = 0;
    virtual void AddStat(D2UnitStrc* pUnit, int nStat, int nValue, int nLayer) = 0;
    virtual bool HasState(D2UnitStrc* pUnit, int nState) = 0;
    virtual bool AddState(D2UnitStrc* pUnit, int nState, int nFrames) = 0;
    virtual void RemoveState(D2UnitStrc* pUnit, int nState) = 0;

    // --- players ----------------------------------------------------------------------------------
    virtual std::string PlayerName(D2UnitStrc* pUnit) = 0;
    // A line to the player's client only (at most 255 bytes, no 0xFF).
    virtual void SendMessage(D2UnitStrc* pUnit, const char* pszText) = 0;

    // --- monsters ---------------------------------------------------------------------------------
    virtual int MonsterTypeFlags(D2UnitStrc* pUnit) = 0;
    virtual bool IsBoss(D2UnitStrc* pUnit) = 0;
    virtual int MonsterCount() = 0;
    // Kill as a script does: death mode, drops, quest hooks. The killer gets the credit.
    virtual void Kill(D2UnitStrc* pUnit, D2UnitStrc* pKiller) = 0;
    // The target of the unit's current mode.
    virtual D2UnitStrc* Target(D2UnitStrc* pUnit) = 0;
    // Spawn a monster of the class at the free spot nearest (x, y) within nRadius subtiles in the
    // active room there. Null when there is no room or no spot.
    virtual D2UnitStrc* SpawnMonster(int nClass, int nX, int nY, int nRadius, bool bMinions) = 0;
    // The monster's AI id (its MonStats "AI").
    virtual int AiIndex(D2UnitStrc* pUnit) = 0;
    // The MonStats row of a class. False for no such row.
    virtual bool MonsterData(int nClass, D2LuaMonsterData& tOut) = 0;
    // Roll the unit's own seed (the stream the stock AIs roll) and return the new value.
    virtual uint32_t SeedRoll(D2UnitStrc* pUnit) = 0;
    // A dead monster that still lies in an active room (a corpse), or null.
    virtual D2UnitStrc* FindCorpse(uint32_t nGuid) = 0;
    virtual int Alignment(D2UnitStrc* pUnit) = 0;
    virtual bool Targetable(D2UnitStrc* pUnit) = 0;
    virtual bool HasStateMask(D2UnitStrc* pUnit, int nMask) = 0;
    // The leader of the monster's group as the AI keeps it (itself for a leader), or null.
    virtual D2UnitStrc* Leader(D2UnitStrc* pUnit) = 0;
    // The units in the minion list of the group the monster leads (alive or dead), in its order.
    virtual void Minions(D2UnitStrc* pUnit, std::vector<D2UnitStrc*>& aOut) = 0;
    // The rooms the engine's AI scans around a room (its adjacent list, the room itself among them).
    virtual void AdjacentRooms(D2RoomStrc* pRoom, std::vector<D2RoomStrc*>& aOut) = 0;
    // The AI's distance (AI::GetDistance): weighted, less the unit's size.
    virtual int AiDistance(D2UnitStrc* pUnit, D2UnitStrc* pOther) = 0;
    // The AI's target search (FindTargetAndDistance), with its distance.
    virtual D2UnitStrc* AiFindTarget(D2UnitStrc* pUnit, int& nDistance) = 0;
    // Whether the skill could be cast at the unit or the point now (AI_Monster_ValidateSkillAtPosition).
    virtual bool AiCanCast(D2UnitStrc* pUnit, int nSkill, D2UnitStrc* pTarget, int nX, int nY) = 0;
    // An order to every monster in the group (CreateMinion: AllocCommandsForMinions).
    virtual void AiOrderMinions(D2UnitStrc* pUnit, int nOrder) = 0;
    // Life in percent of its maximum, as the engine rounds it (GetCurrentLifePercentage).
    virtual int LifePercent(D2UnitStrc* pUnit) = 0;
    // Whether a collision of the mask lies between the two units (TestCollisionWithUnit).
    virtual bool CollidesWith(D2UnitStrc* pUnit, D2UnitStrc* pOther, int nMask) = 0;
    virtual void AiTargetScan(D2UnitStrc* pUnit, D2LuaTargetScan& tOut) = 0;
    // How its next moves path (AITACTICS_SetPathOverrides).
    virtual void AiPathOverrides(D2UnitStrc* pUnit, int nPathType, int nVelocity, int nSteps) = 0;

    // --- the actions a script AI starts (each counts as scheduling the next think) ----------------
    // Think again after nFrames (the engine's PlanNextAiMove: back to neutral first).
    virtual void AiIdle(D2UnitStrc* pUnit, int nFrames) = 0;
    // Walk or run to a unit or a subtile, with StartMonsterMove's flags. False when no path could be started.
    virtual bool AiMove(D2UnitStrc* pUnit, D2UnitStrc* pTarget, int nX, int nY, int nMove, int nFlags) = 0;
    virtual bool AiAttack(D2UnitStrc* pUnit, D2UnitStrc* pTarget, bool bSecondary) = 0;
    virtual bool AiCast(D2UnitStrc* pUnit, int nSkill, int nMode, D2UnitStrc* pTarget, int nX, int nY) = 0;
    // A sequence skill (MONSTERAI_CommandSkillSequence). False when the mode could not be set.
    virtual bool AiSequence(D2UnitStrc* pUnit, int nSkill, D2UnitStrc* pTarget, int nX, int nY) = 0;
    // Close in along a curve (SetCombatStateAndApproachTarget; nFlags non-zero: its direction flags).
    virtual bool AiCircle(D2UnitStrc* pUnit, D2UnitStrc* pTarget, int nSteps, int nFlags) = 0;
    // Walk toward a unit to nOffset subtiles past it along the line (WalkTowardsUnitOffset).
    virtual bool AiWalkPast(D2UnitStrc* pUnit, D2UnitStrc* pTarget, int nOffset, bool bForce) = 0;
    // Walk to a random spot around a unit (WalkRandomOffsetAroundUnit).
    virtual bool AiWanderAround(D2UnitStrc* pUnit, D2UnitStrc* pTarget, int nRadius) = 0;
    // Drop its pending think, then walk to a distance from a unit (MONSTERAI_CommandWalkToDistance).
    virtual bool AiWalkToDistance(D2UnitStrc* pUnit, D2UnitStrc* pTarget, int nMaxStep, int nDistance) = 0;
    virtual bool AiWander(D2UnitStrc* pUnit, int nRadius) = 0;
    virtual bool AiFlee(D2UnitStrc* pUnit, D2UnitStrc* pFrom, int nDistance) = 0;
    // One think of the stock AI, with the engine's parameters of the think now running.
    virtual void AiStock(D2UnitStrc* pUnit) = 0;
    // The monster's AI state (the engine's: the mode it last entered through the lifecycle).
    virtual int AiState(D2UnitStrc* pUnit) = 0;
    // Mark the start of an action (AI state 1), so a hit during it shows on the next think.
    virtual void AiMarkAction(D2UnitStrc* pUnit) = 0;
    // The monster's AI delay for the game's difficulty, in frames.
    virtual int AiDelay(D2UnitStrc* pUnit) = 0;
    // A mod's claim on an AI id changed in this game (the overlay consults it).
    virtual void AiClaim(int nAiIndex, int nClaim, int nTargeting) = 0;

    // --- the think bookkeeping, shared by every world --------------------------------------------
    std::vector<uint8_t> aClaims = std::vector<uint8_t>(D2LUA_AI_STOCK_COUNT, 0);
    std::vector<uint8_t> aTargeting = std::vector<uint8_t>(D2LUA_AI_STOCK_COUNT, 0);
    std::set<uint32_t> tRestart;   // monsters whose script restarts at their next think
    std::set<uint32_t> tScripted;  // monsters the core keeps a script state for
    std::set<uint32_t> tThinking;  // monsters whose think is running (no nested think)
    bool bScheduled = false;       // the running think armed the next one (an action, idle, stock)
    D2UnitStrc* pThinking = nullptr;  // the monster of the innermost running think
    // One think of a claimed monster, through the core. False when the state could not run it
    // (no game, the core failed): the caller then runs the stock think.
    bool Think(D2LuaRuntime* pRt, uint32_t nGame, D2UnitStrc* pUnit, D2UnitStrc* pTarget, int nDistance, bool bEngaged);
    // The AI restarts (an attach ran fpInit): the script starts over at the next think.
    void AiRestart(D2UnitStrc* pUnit);
    // The monster died or is gone (freed, frozen with its room): its script ends.
    void AiDrop(D2LuaRuntime* pRt, uint32_t nGame, uint32_t nGuid);
};

// The world of the state L belongs to, or null (the data stage, a client, a state without one).
D2LuaWorld* D2LuaWorld_Of(lua_State* L);

// Units as the overrides hand them to mods (d2_lua_override.cpp): push the handle of a unit (false
// when the state has no world, so the caller passes it on opaque); the (type, guid) a unit handle at
// index i names (false when it is no unit handle); whether a unit is the one a key names; the usable
// unit of a key, or null.
bool D2LuaWorld_PushUnit(lua_State* L, D2UnitStrc* pUnit);
bool D2LuaWorld_HandleKey(lua_State* L, int i, int* pnType, uint32_t* pnGuid);
bool D2LuaWorld_UnitIs(lua_State* L, D2UnitStrc* pUnit, int nType, uint32_t nGuid);
D2UnitStrc* D2LuaWorld_FindUnit(lua_State* L, int nType, uint32_t nGuid);

// The natives (handle classes Player, Monster, Unit, Room, Level; the "world" natives table of
// the core). Bound into a server state with a world.
extern const D2LuaApiDesc g_D2LuaApi_world;

// The stock AI ids' names (MonsterAiId values), and back; -1 when unknown.
const char* D2LuaWorld_AiName(int nAiIndex);
int D2LuaWorld_AiIndex(const char* pszName);
#endif
