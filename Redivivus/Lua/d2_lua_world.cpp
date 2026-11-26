// See d2_lua_world.h. The natives the core builds the world classes on, and a think's
// bookkeeping. Functions Lua calls hold no object with a destructor in their frames (results are
// collected into per-thread scratch vectors, then pushed).
#if D2_FEATURE_MODS
#include "Redivivus/Lua/d2_lua_world.h"

#include <math.h>
#include <string.h>

#include "Redivivus/Lua/d2_lua_core.h"
#include "Redivivus/Lua/d2_lua_rt.h"
#include "lua.hpp"

// --- the stock AI ids ----------------------------------------------------------------------------
// The rows of monai.txt in the order the engine's general AI table has them (AI.cpp,
// arrGeneralAiList), named after the row; the MonsterAiId enum of redivivus:enums has the same.
static const char* const s_apszAiNames[D2LUA_AI_STOCK_COUNT] = {
    "None", "Idle", "Skeleton", "Zombie", "Bighead", "BloodHawk", "Fallen", "Brute", "SandRaider", "Wraith",
    "CorruptRogue", "Baboon", "Goatman", "FallenShaman", "QuillRat", "SandMaggot", "ClawViper", "SandLeaper",
    "PantherWoman", "Swarm", "Scarab", "Mummy", "GreaterMummy", "Vulture", "Mosquito", "WillOWisp", "Arach",
    "ThornHulk", "Vampire", "BatDemon", "Fetish", "NpcOutOfTown", "Npc", "HellMeteor", "Andariel", "CorruptArcher",
    "CorruptLancer", "SkeletonBow", "MaggotLarva", "PinHead", "MaggotEgg", "Towner", "Vendor", "FoulCrowNest",
    "Duriel", "Sarcophagus", "ElementalBeast", "FlyingScimitar", "ZakarumZealot", "ZakarumPriest", "Mephisto",
    "Diablo", "FrogDemon", "Summoner", "NpcStationary", "Izual", "Tentacle", "TentacleHead", "Navi", "BloodRaven",
    "GoodNpcRanged", "Hireable", "TownRogue", "GargoyleTrap", "SkeletonMage", "FetishShaman", "SandMaggotQueen",
    "NecroPet", "VileMother", "VileDog", "FingerMage", "Regurgitator", "DoomKnight", "AbyssKnight", "OblivionKnight",
    "QuillMother", "EvilHole", "TrapMissile", "TrapRightArrow", "TrapLeftArrow", "TrapPoison", "JarJar",
    "InvisoSpawner", "MosquitoNest", "BoneWall", "HighPriest", "Hydra", "TrapMelee", "SevenTombsIllusion",
    "Megademon", "Griswold", "DarkWanderer", "TrapNova", "ArcaneTower", "DesertTurret", "PantherJavelin",
    "FetishBlowgun", "Spirit", "Smith", "TrappedSoul", "Buffy", "AssassinSentry", "BladeCreeper", "InvisoPet",
    "DeathSentry", "ShadowWarrior", "ShadowMaster", "Raven", "DruidWolf", "Totem", "Vines", "CycleOfLife",
    "DruidBear", "SiegeTower", "ReanimatedHorde", "SiegeBeast", "Minion", "SuicideMinion", "Succubus",
    "SuccubusWitch", "Overseer", "MinionSpawner", "Imp", "Catapult", "FrozenHorror", "BloodLord", "CatapultSpotter",
    "NpcBarb", "Nihlathak", "GenericSpawner", "DeathMauler", "Wussie", "AncientStatue", "Ancient", "BaalThrone",
    "BaalCrab", "BaalTaunt", "PutridDefiler", "BaalToStairs", "BaalTentacle", "BaalCrabClone", "BaalMinion",
    "ClawViperEx", "ShadowMasterEx", "UberIzual", "UberBaal", "UberMephisto", "UberDiablo",
};

const char* D2LuaWorld_AiName(int nAiIndex) {
    return nAiIndex >= 0 && nAiIndex < D2LUA_AI_STOCK_COUNT ? s_apszAiNames[nAiIndex] : nullptr;
}

int D2LuaWorld_AiIndex(const char* pszName) {
    for (int i = 0; i < D2LUA_AI_STOCK_COUNT; i++) {
        if (strcmp(s_apszAiNames[i], pszName) == 0) {
            return i;
        }
    }
    return -1;
}

D2LuaWorld* D2LuaWorld_Of(lua_State* L) {
    D2LuaRuntime* pRt = D2LuaRt_FromState(L);
    return pRt ? pRt->tDesc.pWorld : nullptr;
}

static D2LuaWorld* World(lua_State* L) {
    D2LuaWorld* w = D2LuaWorld_Of(L);
    if (!w) {
        luaL_error(L, "this state has no game world (only a server state running a game has one)");
    }
    return w;
}

// --- handle classes ------------------------------------------------------------------------------

static uint64_t UnitId(int nType, uint32_t nGuid) {
    return ((uint64_t)(uint32_t)nType << 32) | nGuid;
}

static void* ResolveUnit(lua_State* L, uint64_t nId) {
    D2LuaWorld* w = D2LuaWorld_Of(L);
    int nType = (int)(nId >> 32);
    if (!w || nType < 0 || nType >= D2LUA_UNIT_TYPES) {
        return nullptr;
    }
    return w->FindUnit(nType, (uint32_t)nId);
}

static uint64_t IdOfUnitNever(void* pObject) {
    (void)pObject;
    return 0;  // units are pushed by id (PushUnit), never through IdOf
}

static void* ResolveLevel(lua_State* L, uint64_t nId) {
    D2LuaWorld* w = D2LuaWorld_Of(L);
    return w && w->LevelExists((int)nId) ? (void*)(uintptr_t)(nId + 1) : nullptr;
}

static void* ResolveRoom(lua_State* L, uint64_t nId) {
    D2LuaWorld* w = D2LuaWorld_Of(L);
    if (!w) {
        return nullptr;
    }
    D2LuaRoomKey tKey = { (uint32_t)(nId >> 40), (uint32_t)((nId >> 20) & 0xFFFFF), (uint32_t)(nId & 0xFFFFF) };
    return w->RoomFind(tKey);
}

static void* ResolveCorpse(lua_State* L, uint64_t nId) {
    D2LuaWorld* w = D2LuaWorld_Of(L);
    return w ? w->FindCorpse((uint32_t)nId) : nullptr;
}

static const luaL_Reg s_aNone[] = { { nullptr, nullptr } };
static const D2LuaHandleClass kClass_Player = { "Player", ResolveUnit, IdOfUnitNever, s_aNone, s_aNone, s_aNone };
static const D2LuaHandleClass kClass_Monster = { "Monster", ResolveUnit, IdOfUnitNever, s_aNone, s_aNone, s_aNone };
static const D2LuaHandleClass kClass_Unit = { "Unit", ResolveUnit, IdOfUnitNever, s_aNone, s_aNone, s_aNone };
static const D2LuaHandleClass kClass_Level = { "Level", ResolveLevel, IdOfUnitNever, s_aNone, s_aNone, s_aNone };
static const D2LuaHandleClass kClass_Room = { "Room", ResolveRoom, IdOfUnitNever, s_aNone, s_aNone, s_aNone };
static const D2LuaHandleClass kClass_Corpse = { "Corpse", ResolveCorpse, IdOfUnitNever, s_aNone, s_aNone, s_aNone };

static const D2LuaHandleClass* ClassOfType(int nType) {
    return nType == D2LUA_UNIT_PLAYER ? &kClass_Player : nType == D2LUA_UNIT_MONSTER ? &kClass_Monster : &kClass_Unit;
}

static void PushUnit(lua_State* L, D2LuaWorld* w, D2UnitStrc* pUnit) {
    if (!pUnit) {
        lua_pushnil(L);
        return;
    }
    int nType = w->UnitType(pUnit);
    D2LuaHandle_PushId(L, ClassOfType(nType), UnitId(nType, w->UnitGuid(pUnit)));
}

static void PushRoom(lua_State* L, D2LuaWorld* w, D2RoomStrc* pRoom) {
    if (!pRoom) {
        lua_pushnil(L);
        return;
    }
    D2LuaRoomKey k = w->RoomKeyOf(pRoom);
    D2LuaHandle_PushId(L, &kClass_Room, ((uint64_t)k.nLevel << 40) | ((uint64_t)(k.nTileX & 0xFFFFF) << 20) | (k.nTileY & 0xFFFFF));
}

// The unit behind the handle at i, any unit class; raises when it is not one or is not valid.
static D2UnitStrc* CheckUnit(lua_State* L, int i, const char* pszWhere, bool bOptional = false) {
    if (bOptional && lua_isnoneornil(L, i)) {
        return nullptr;
    }
    const char* pszClass = nullptr;
    uint64_t nId = 0;
    if (!D2LuaHandle_Inspect(L, i, &pszClass, &nId)
        || (strcmp(pszClass, "Player") != 0 && strcmp(pszClass, "Monster") != 0 && strcmp(pszClass, "Unit") != 0)) {
        luaL_error(L, "bad argument #%d to '%s' (a unit expected, got %s)", i, pszWhere, pszClass ? pszClass : luaL_typename(L, i));
    }
    D2UnitStrc* pUnit = (D2UnitStrc*)ResolveUnit(L, nId);
    if (!pUnit) {
        luaL_error(L, "%s: this %s is not valid (it died, its room was unloaded, or it is gone)", pszWhere, pszClass);
    }
    return pUnit;
}

static D2UnitStrc* CheckMonster(lua_State* L, int i, const char* pszWhere) {
    D2UnitStrc* pUnit = CheckUnit(L, i, pszWhere);
    if (World(L)->UnitType(pUnit) != D2LUA_UNIT_MONSTER) {
        luaL_error(L, "bad argument #%d to '%s' (a monster expected)", i, pszWhere);
    }
    return pUnit;
}

static void PushCorpse(lua_State* L, D2LuaWorld* w, D2UnitStrc* pUnit) {
    D2LuaHandle_PushId(L, &kClass_Corpse, w->UnitGuid(pUnit));
}

// A unit or a corpse at i (a skill's target); raises when it is neither or is not valid.
static D2UnitStrc* CheckUnitOrCorpse(lua_State* L, int i, const char* pszWhere, bool bOptional = false) {
    if (bOptional && lua_isnoneornil(L, i)) {
        return nullptr;
    }
    const char* pszClass = nullptr;
    uint64_t nId = 0;
    if (D2LuaHandle_Inspect(L, i, &pszClass, &nId) && strcmp(pszClass, "Corpse") == 0) {
        D2UnitStrc* p = (D2UnitStrc*)ResolveCorpse(L, nId);
        if (!p) {
            luaL_error(L, "%s: this Corpse is not valid (the monster is alive again, or gone)", pszWhere);
        }
        return p;
    }
    return CheckUnit(L, i, pszWhere);
}

static D2RoomStrc* CheckRoom(lua_State* L, int i, const char* pszWhere) {
    return (D2RoomStrc*)D2LuaHandle_Check(L, i, &kClass_Room, pszWhere, false);
}

static int CheckLevelHandle(lua_State* L, int i, const char* pszWhere) {
    return (int)((uintptr_t)D2LuaHandle_Check(L, i, &kClass_Level, pszWhere, false) - 1);
}

static int ArgInt(lua_State* L, int i, const char* pszWhere) {
    if (lua_type(L, i) != LUA_TNUMBER) {
        luaL_error(L, "bad argument #%d to '%s' (integer expected, got %s)", i, pszWhere, luaL_typename(L, i));
    }
    lua_Number n = lua_tonumber(L, i);
    if (n != floor(n) || n < -2147483648.0 || n > 2147483647.0) {
        luaL_error(L, "bad argument #%d to '%s' (a 32-bit integer expected)", i, pszWhere);
    }
    return (int)n;
}

static int ArgType(lua_State* L, int i, const char* pszWhere) {
    if (lua_isnoneornil(L, i)) {
        return -1;
    }
    int n = ArgInt(L, i, pszWhere);
    if (n < 0 || n >= D2LUA_UNIT_TYPES) {
        luaL_error(L, "bad argument #%d to '%s' (no unit type %d)", i, pszWhere, n);
    }
    return n;
}

static void PushRect(lua_State* L, const D2LuaRect& r) {
    lua_pushinteger(L, r.nX);
    lua_pushinteger(L, r.nY);
    lua_pushinteger(L, r.nWidth);
    lua_pushinteger(L, r.nHeight);
}

// --- scratch -------------------------------------------------------------------------------------
static thread_local std::vector<D2UnitStrc*> s_aUnits;
static thread_local std::vector<D2RoomStrc*> s_aRooms;
static thread_local std::vector<D2RoomStrc*> s_aFrontier;
static thread_local std::vector<D2RoomStrc*> s_aNext;
static thread_local std::vector<D2RoomStrc*> s_aNb;
static thread_local std::vector<D2UnitStrc*> s_aIn;
static thread_local std::string s_sText;
static thread_local D2LuaLevelInfo s_tLevel;

// Push the units in s_aUnits as an array of handles, those of type nType (any when -1), players
// without their corpses.
static void PushUnitList(lua_State* L, D2LuaWorld* w, int nType) {
    lua_createtable(L, (int)s_aUnits.size(), 0);
    int n = 0;
    for (size_t i = 0; i < s_aUnits.size(); i++) {
        D2UnitStrc* p = s_aUnits[i];
        int t = w->UnitType(p);
        if ((nType >= 0 && t != nType) || (t == D2LUA_UNIT_PLAYER && w->IsCorpse(p)) || !w->FindUnit(t, w->UnitGuid(p))) {
            continue;
        }
        PushUnit(L, w, p);
        lua_rawseti(L, -2, ++n);
    }
}

// --- the game ------------------------------------------------------------------------------------

static int N_gameInfo(lua_State* L) {
    D2LuaWorld* w = World(L);
    s_sText = w->Name();
    lua_pushinteger(L, w->Difficulty());
    lua_pushboolean(L, w->Expansion());
    lua_pushstring(L, s_sText.c_str());
    lua_pushnumber(L, (lua_Number)w->Seed());
    lua_pushnumber(L, (lua_Number)w->Frame());
    return 5;
}

static int N_players(lua_State* L) {
    D2LuaWorld* w = World(L);
    s_aUnits.clear();
    w->Players(s_aUnits);
    PushUnitList(L, w, D2LUA_UNIT_PLAYER);
    return 1;
}

static int N_message(lua_State* L) {
    D2LuaWorld* w = World(L);
    size_t n = 0;
    const char* s = luaL_checklstring(L, 1, &n);
    char sz[256];
    size_t j = 0;
    for (size_t i = 0; i < n && j < 255; i++) {
        if ((unsigned char)s[i] != 0xFF && s[i] != 0) {
            sz[j++] = s[i];
        }
    }
    sz[j] = 0;
    w->Message(sz);
    return 0;
}

static int N_findUnit(lua_State* L) {
    D2LuaWorld* w = World(L);
    int nType = ArgType(L, 1, "game.findUnit");
    if (nType < 0) {
        return luaL_error(L, "bad argument #1 to 'game.findUnit' (a unit type expected)");
    }
    lua_Number nGuid = luaL_checknumber(L, 2);
    if (nGuid < 0 || nGuid > 4294967295.0 || nGuid != floor(nGuid)) {
        lua_pushnil(L);
        return 1;
    }
    D2UnitStrc* p = w->FindUnit(nType, (uint32_t)nGuid);
    if (p && nType == D2LUA_UNIT_PLAYER && w->IsCorpse(p)) {
        p = nullptr;
    }
    PushUnit(L, w, p);
    return 1;
}

static int N_spawnMonster(lua_State* L) {
    D2LuaWorld* w = World(L);
    int nClass = ArgInt(L, 1, "game.spawnMonster");
    if (nClass < 0 || nClass >= w->MonsterCount()) {
        return luaL_error(L, "game.spawnMonster: no monster %d", nClass);
    }
    int nX = ArgInt(L, 2, "game.spawnMonster");
    int nY = ArgInt(L, 3, "game.spawnMonster");
    int nRadius = ArgInt(L, 4, "game.spawnMonster");
    if (nRadius < 0 || nRadius > 30) {
        return luaL_error(L, "game.spawnMonster: radius %d is outside 0..30", nRadius);
    }
    PushUnit(L, w, w->SpawnMonster(nClass, nX, nY, nRadius, lua_toboolean(L, 5) != 0));
    return 1;
}

// Units within radius of (x, y), squared Euclidean in subtiles: the room there and outward through
// neighbours while a room's rectangle comes within the radius. A snapshot of membership.
static int N_unitsNear(lua_State* L) {
    D2LuaWorld* w = World(L);
    int nX = ArgInt(L, 1, "unitsNear");
    int nY = ArgInt(L, 2, "unitsNear");
    lua_Number nRadius = luaL_checknumber(L, 3);
    int nType = ArgType(L, 4, "unitsNear");
    D2UnitStrc* pExclude = lua_isnoneornil(L, 5) ? nullptr : CheckUnit(L, 5, "unitsNear");
    if (nRadius < 0 || nRadius > 200) {
        return luaL_error(L, "unitsNear: radius %f is outside 0..200", nRadius);
    }
    s_aUnits.clear();
    s_aRooms.clear();
    s_aFrontier.clear();
    D2RoomStrc* pStart = w->RoomAt(nX, nY, nullptr);
    if (pStart) {
        s_aFrontier.push_back(pStart);
        s_aRooms.push_back(pStart);
    }
    while (!s_aFrontier.empty()) {
        s_aNext.clear();
        for (D2RoomStrc* pRoom : s_aFrontier) {
            s_aIn.clear();
            w->RoomUnits(pRoom, s_aIn);
            for (D2UnitStrc* p : s_aIn) {
                int ux, uy;
                if (p == pExclude || !w->Position(p, ux, uy)) {
                    continue;
                }
                double dx = ux - nX, dy = uy - nY;
                if (dx * dx + dy * dy <= nRadius * nRadius) {
                    s_aUnits.push_back(p);
                }
            }
            s_aNb.clear();
            w->RoomNeighbours(pRoom, s_aNb);
            for (D2RoomStrc* pNb : s_aNb) {
                bool bSeen = false;
                for (D2RoomStrc* pDone : s_aRooms) {
                    bSeen = bSeen || pDone == pNb;
                }
                if (bSeen) {
                    continue;
                }
                D2LuaRect r = w->RoomBounds(pNb);
                double cx = nX < r.nX ? r.nX : nX > r.nX + r.nWidth ? r.nX + r.nWidth : nX;
                double cy = nY < r.nY ? r.nY : nY > r.nY + r.nHeight ? r.nY + r.nHeight : nY;
                if ((cx - nX) * (cx - nX) + (cy - nY) * (cy - nY) <= nRadius * nRadius) {
                    s_aRooms.push_back(pNb);
                    s_aNext.push_back(pNb);
                }
            }
        }
        s_aFrontier.swap(s_aNext);
    }
    PushUnitList(L, w, nType);
    return 1;
}

// --- levels and rooms ----------------------------------------------------------------------------

static int N_level(lua_State* L) {
    D2LuaWorld* w = World(L);
    int nLevel = ArgInt(L, 1, "game.level");
    if (nLevel <= 0 || nLevel >= w->LevelCount() || !w->LevelExists(nLevel)) {
        lua_pushnil(L);
        return 1;
    }
    D2LuaHandle_PushId(L, &kClass_Level, (uint64_t)nLevel);
    return 1;
}

static int N_levelId(lua_State* L) {
    lua_pushinteger(L, CheckLevelHandle(L, 1, "level.id"));
    return 1;
}

// levelInfo(id): name, act, town, generated, x, y, width, height (bounds only when generated).
static int N_levelInfo(lua_State* L) {
    D2LuaWorld* w = World(L);
    int nLevel = ArgInt(L, 1, "Level.info");
    s_tLevel = D2LuaLevelInfo();
    if (nLevel <= 0 || nLevel >= w->LevelCount() || !w->LevelInfo(nLevel, s_tLevel)) {
        lua_pushnil(L);
        return 1;
    }
    lua_pushstring(L, s_tLevel.sName.c_str());
    lua_pushinteger(L, s_tLevel.nAct);
    lua_pushboolean(L, s_tLevel.bTown);
    lua_pushboolean(L, s_tLevel.bGenerated);
    if (!s_tLevel.bGenerated) {
        return 4;
    }
    PushRect(L, s_tLevel.tBounds);
    return 8;
}

static int N_levelRooms(lua_State* L) {
    D2LuaWorld* w = World(L);
    int nLevel = CheckLevelHandle(L, 1, "level.rooms");
    s_aRooms.clear();
    w->LevelRooms(nLevel, s_aRooms);
    lua_createtable(L, (int)s_aRooms.size(), 0);
    for (size_t i = 0; i < s_aRooms.size(); i++) {
        PushRoom(L, w, s_aRooms[i]);
        lua_rawseti(L, -2, (int)i + 1);
    }
    return 1;
}

// The units of a level: its active rooms' lists.
static int N_levelUnits(lua_State* L) {
    D2LuaWorld* w = World(L);
    int nLevel = CheckLevelHandle(L, 1, "level.units");
    int nType = ArgType(L, 2, "level.units");
    s_aRooms.clear();
    w->LevelRooms(nLevel, s_aRooms);
    s_aUnits.clear();
    for (D2RoomStrc* pRoom : s_aRooms) {
        w->RoomUnits(pRoom, s_aUnits);
    }
    PushUnitList(L, w, nType);
    return 1;
}

// roomInfo(room): x, y, width, height (subtiles), level id.
static int N_roomInfo(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2RoomStrc* pRoom = CheckRoom(L, 1, "room");
    PushRect(L, w->RoomBounds(pRoom));
    lua_pushinteger(L, (int)w->RoomKeyOf(pRoom).nLevel);
    return 5;
}

static int N_roomNeighbours(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2RoomStrc* pRoom = CheckRoom(L, 1, "room.neighbours");
    s_aRooms.clear();
    w->RoomNeighbours(pRoom, s_aRooms);
    lua_createtable(L, (int)s_aRooms.size(), 0);
    for (size_t i = 0; i < s_aRooms.size(); i++) {
        PushRoom(L, w, s_aRooms[i]);
        lua_rawseti(L, -2, (int)i + 1);
    }
    return 1;
}

static int N_roomUnits(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2RoomStrc* pRoom = CheckRoom(L, 1, "room.units");
    int nType = ArgType(L, 2, "room.units");
    s_aUnits.clear();
    w->RoomUnits(pRoom, s_aUnits);
    PushUnitList(L, w, nType);
    return 1;
}

static int N_levelHandle(lua_State* L) {
    D2LuaWorld* w = World(L);
    int nLevel = ArgInt(L, 1, "level");
    if (nLevel > 0 && nLevel < w->LevelCount() && w->LevelExists(nLevel)) {
        D2LuaHandle_PushId(L, &kClass_Level, (uint64_t)nLevel);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// --- units ---------------------------------------------------------------------------------------

// unitInfo(u): type, guid, class id.
static int N_unitInfo(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckUnit(L, 1, "unit");
    lua_pushinteger(L, w->UnitType(p));
    lua_pushnumber(L, (lua_Number)w->UnitGuid(p));
    lua_pushinteger(L, w->ClassId(p));
    return 3;
}

// The id of any handle's object, even an invalid one: what the core keys its tables by.
static int N_handleId(lua_State* L) {
    const char* pszClass = nullptr;
    uint64_t nId = 0;
    if (!D2LuaHandle_Inspect(L, 1, &pszClass, &nId)) {
        lua_pushnil(L);
        return 1;
    }
    lua_pushnumber(L, (lua_Number)nId);
    lua_pushstring(L, pszClass);
    return 2;
}

static int N_unitPosition(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckUnit(L, 1, "unit.position");
    int x, y;
    if (!w->Position(p, x, y)) {
        lua_pushnil(L);
        return 1;
    }
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

static int N_unitRoom(lua_State* L) {
    D2LuaWorld* w = World(L);
    PushRoom(L, w, w->UnitRoom(CheckUnit(L, 1, "unit.room")));
    return 1;
}

// unitPlace(u): level id, act, mode, dead.
static int N_unitPlace(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckUnit(L, 1, "unit");
    lua_pushinteger(L, w->UnitLevel(p));
    lua_pushinteger(L, w->UnitAct(p));
    lua_pushinteger(L, w->Mode(p));
    lua_pushboolean(L, w->IsDead(p));
    return 4;
}

static int N_unitOwner(lua_State* L) {
    D2LuaWorld* w = World(L);
    PushUnit(L, w, w->Owner(CheckUnit(L, 1, "unit.owner")));
    return 1;
}

static int StatArg(lua_State* L, D2LuaWorld* w, int i, const char* pszWhere) {
    int n = ArgInt(L, i, pszWhere);
    if (n < 0 || n >= w->StatCount()) {
        luaL_error(L, "%s: no stat %d", pszWhere, n);
    }
    return n;
}

static int LayerArg(lua_State* L, int i, const char* pszWhere) {
    if (lua_isnoneornil(L, i)) {
        return 0;
    }
    int n = ArgInt(L, i, pszWhere);
    if (n < 0 || n > 0xFFFF) {
        luaL_error(L, "%s: layer %d is outside 0..65535", pszWhere, n);
    }
    return n;
}

static int StateArg(lua_State* L, D2LuaWorld* w, int i, const char* pszWhere) {
    int n = ArgInt(L, i, pszWhere);
    if (n <= 0 || n >= w->StateCount()) {
        luaL_error(L, "%s: no state %d", pszWhere, n);
    }
    return n;
}

// getStat(u, stat, layer, base)
static int N_getStat(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckUnit(L, 1, "unit.getStat");
    int nStat = StatArg(L, w, 2, "unit.getStat");
    lua_pushinteger(L, w->GetStat(p, nStat, LayerArg(L, 3, "unit.getStat"), lua_toboolean(L, 4) != 0));
    return 1;
}

// setStat(u, stat, value, layer, add)
static int N_setStat(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckUnit(L, 1, "unit.setStat");
    int nStat = StatArg(L, w, 2, "unit.setStat");
    int nValue = ArgInt(L, 3, "unit.setStat");
    int nLayer = LayerArg(L, 4, "unit.setStat");
    if (lua_toboolean(L, 5)) {
        w->AddStat(p, nStat, nValue, nLayer);
    } else {
        w->SetStat(p, nStat, nValue, nLayer);
    }
    return 0;
}

static int N_hasState(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckUnit(L, 1, "unit.hasState");
    lua_pushboolean(L, w->HasState(p, StateArg(L, w, 2, "unit.hasState")));
    return 1;
}

static int N_addState(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckUnit(L, 1, "unit.addState");
    int nState = StateArg(L, w, 2, "unit.addState");
    int nFrames = lua_isnoneornil(L, 3) ? 0 : ArgInt(L, 3, "unit.addState");
    if (nFrames < 0) {
        return luaL_error(L, "unit.addState: frames %d is negative", nFrames);
    }
    lua_pushboolean(L, w->AddState(p, nState, nFrames));
    return 1;
}

static int N_removeState(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckUnit(L, 1, "unit.removeState");
    w->RemoveState(p, StateArg(L, w, 2, "unit.removeState"));
    return 0;
}

// monsterInfo(m): type flags, boss.
static int N_monsterInfo(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckMonster(L, 1, "monster");
    lua_pushinteger(L, w->MonsterTypeFlags(p));
    lua_pushboolean(L, w->IsBoss(p));
    return 2;
}

// monsterData(class): a table of the MonStats row as the AI reads it, or nil.
static int N_monsterData(lua_State* L) {
    D2LuaWorld* w = World(L);
    int nClass = ArgInt(L, 1, "monsterData");
    D2LuaMonsterData t;
    if (nClass < 0 || nClass >= w->MonsterCount() || !w->MonsterData(nClass, t)) {
        lua_pushnil(L);
        return 1;
    }
    lua_createtable(L, 0, 6);
    lua_pushinteger(L, t.nBaseId);
    lua_setfield(L, -2, "baseId");
    lua_pushinteger(L, t.nAi);
    lua_setfield(L, -2, "ai");
    lua_pushinteger(L, t.nAiDelay);
    lua_setfield(L, -2, "aiDelay");
    const int* aArrays[3] = { t.aAiParams, t.aSkills, t.aSkillModes };
    const char* aNames[3] = { "aiParams", "skills", "skillModes" };
    for (int a = 0; a < 3; a++) {
        lua_createtable(L, 8, 0);
        for (int i = 0; i < 8; i++) {
            lua_pushinteger(L, aArrays[a][i]);
            lua_rawseti(L, -2, i + 1);
        }
        lua_setfield(L, -2, aNames[a]);
    }
    return 1;
}

// unitRoll(monster): the next value of its seed, 0..2^32-1.
static int N_unitRoll(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckMonster(L, 1, "monster.random");
    lua_pushnumber(L, (lua_Number)w->SeedRoll(p));
    return 1;
}

// corpseInfo(corpse): class, x, y, mode, alignment, targetable, type flags.
static int N_corpseInfo(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = (D2UnitStrc*)D2LuaHandle_Check(L, 1, &kClass_Corpse, "corpse", false);
    int x = 0, y = 0;
    w->Position(p, x, y);
    lua_pushinteger(L, w->ClassId(p));
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    lua_pushinteger(L, w->Mode(p));
    lua_pushinteger(L, w->Alignment(p));
    lua_pushboolean(L, w->Targetable(p));
    lua_pushinteger(L, w->MonsterTypeFlags(p));
    return 7;
}

static int N_alignment(lua_State* L) {
    D2LuaWorld* w = World(L);
    lua_pushinteger(L, w->Alignment(CheckUnitOrCorpse(L, 1, "alignment")));
    return 1;
}

static int N_hasStateMask(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckUnitOrCorpse(L, 1, "hasStateMask");
    int nMask = ArgInt(L, 2, "hasStateMask");
    if (nMask < 0 || nMask >= D2LUA_STATEMASK_COUNT) {
        return luaL_error(L, "hasStateMask: no state mask %d (a StateMask)", nMask);
    }
    lua_pushboolean(L, w->HasStateMask(p, nMask));
    return 1;
}

// leader(monster|corpse): the leader of its group as a Monster handle, or nil.
static int N_leader(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = w->Leader(CheckUnitOrCorpse(L, 1, "leader"));
    if (!p) {
        lua_pushnil(L);
    } else {
        D2LuaHandle_PushId(L, &kClass_Monster, UnitId(D2LUA_UNIT_MONSTER, w->UnitGuid(p)));
    }
    return 1;
}

// minions(monster, dead): the living minions as Monster handles, or the dead ones as Corpse handles.
static int N_minions(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckMonster(L, 1, "monster.minions");
    bool bDead = lua_toboolean(L, 2) != 0;
    s_aIn.clear();
    w->Minions(p, s_aIn);
    lua_createtable(L, (int)s_aIn.size(), 0);
    int n = 0;
    for (D2UnitStrc* m : s_aIn) {
        uint32_t nGuid = w->UnitGuid(m);
        if (bDead ? w->FindCorpse(nGuid) == m : w->FindUnit(D2LUA_UNIT_MONSTER, nGuid) == m) {
            if (bDead) {
                PushCorpse(L, w, m);
            } else {
                PushUnit(L, w, m);
            }
            lua_rawseti(L, -2, ++n);
        }
    }
    return 1;
}

// corpsesNear(monster, radius): the corpses in the rooms the AI scans, within radius, in scan order.
static int N_corpsesNear(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckMonster(L, 1, "monster.corpsesNear");
    lua_Number nRadius = luaL_checknumber(L, 2);
    if (nRadius < 0 || nRadius > 200) {
        return luaL_error(L, "monster.corpsesNear: radius %f is outside 0..200", nRadius);
    }
    int nX = 0, nY = 0;
    w->Position(p, nX, nY);
    s_aRooms.clear();
    D2RoomStrc* pRoom = w->UnitRoom(p);
    if (pRoom) {
        w->AdjacentRooms(pRoom, s_aRooms);
    }
    lua_newtable(L);
    int n = 0;
    for (D2RoomStrc* r : s_aRooms) {
        s_aIn.clear();
        w->RoomUnits(r, s_aIn);
        for (D2UnitStrc* u : s_aIn) {
            int ux, uy;
            if (u == p || w->UnitType(u) != D2LUA_UNIT_MONSTER || w->FindCorpse(w->UnitGuid(u)) != u || !w->Position(u, ux, uy)) {
                continue;
            }
            double dx = ux - nX, dy = uy - nY;
            if (dx * dx + dy * dy <= nRadius * nRadius) {
                PushCorpse(L, w, u);
                lua_rawseti(L, -2, ++n);
            }
        }
    }
    return 1;
}

static int N_aiDistance(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckMonster(L, 1, "monster.aiDistanceTo");
    lua_pushinteger(L, w->AiDistance(p, CheckUnitOrCorpse(L, 2, "monster.aiDistanceTo")));
    return 1;
}

static int N_aiFindTarget(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckMonster(L, 1, "monster.findTarget");
    int nDistance = 0;
    D2UnitStrc* t = w->AiFindTarget(p, nDistance);
    if (!t || !w->FindUnit(w->UnitType(t), w->UnitGuid(t))) {
        lua_pushnil(L);
        return 1;
    }
    PushUnit(L, w, t);
    lua_pushinteger(L, nDistance);
    return 2;
}

// aiCanCast(monster, skill, target|nil, x, y)
static int N_aiCanCast(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckMonster(L, 1, "monster.canCast");
    int nSkill = ArgInt(L, 2, "monster.canCast");
    if (nSkill < 0 || nSkill > 0xFFFF) {
        return luaL_error(L, "monster.canCast: no skill %d", nSkill);
    }
    D2UnitStrc* pTarget = CheckUnitOrCorpse(L, 3, "monster.canCast", true);
    int nX = pTarget ? 0 : ArgInt(L, 4, "monster.canCast");
    int nY = pTarget ? 0 : ArgInt(L, 5, "monster.canCast");
    lua_pushboolean(L, w->AiCanCast(p, nSkill, pTarget, nX, nY));
    return 1;
}

static int N_aiOrderMinions(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckMonster(L, 1, "monster.orderMinions");
    int nOrder = ArgInt(L, 2, "monster.orderMinions");
    if (nOrder != 1) {
        return luaL_error(L, "monster.orderMinions: no order %d (a MinionOrder)", nOrder);
    }
    w->AiOrderMinions(p, nOrder);
    return 0;
}

static int N_kill(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckMonster(L, 1, "monster.kill");
    D2UnitStrc* pKiller = CheckUnit(L, 2, "monster.kill", true);
    w->Kill(p, pKiller);
    return 0;
}

// target(unit): what its current mode aims at; a dead monster (a resurrect's aim) as its Corpse.
static int N_target(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* t = w->Target(CheckUnit(L, 1, "unit.target"));
    if (t && w->UnitType(t) == D2LUA_UNIT_MONSTER && w->FindCorpse(w->UnitGuid(t)) == t) {
        PushCorpse(L, w, t);
    } else {
        PushUnit(L, w, t);
    }
    return 1;
}

static int N_playerName(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckUnit(L, 1, "player.name");
    s_sText = w->UnitType(p) == D2LUA_UNIT_PLAYER ? w->PlayerName(p) : std::string();
    lua_pushstring(L, s_sText.c_str());
    return 1;
}

// sendMessage(player, text): a line to that player only, at most 255 bytes, no 0xFF.
static int N_sendMessage(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckUnit(L, 1, "player.sendMessage");
    if (w->UnitType(p) != D2LUA_UNIT_PLAYER) {
        return luaL_error(L, "player.sendMessage: not a player");
    }
    size_t n = 0;
    const char* s = luaL_checklstring(L, 2, &n);
    char sz[256];
    size_t j = 0;
    for (size_t i = 0; i < n && j < 255; i++) {
        if ((unsigned char)s[i] != 0xFF && s[i] != 0) {
            sz[j++] = s[i];
        }
    }
    sz[j] = 0;
    w->SendMessage(p, sz);
    return 0;
}

// --- the monster AI ------------------------------------------------------------------------------

static int N_aiName(lua_State* L) {
    const char* psz = D2LuaWorld_AiName(ArgInt(L, 1, "aiName"));
    if (psz) {
        lua_pushstring(L, psz);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int N_aiIndex(lua_State* L) {
    lua_pushinteger(L, D2LuaWorld_AiIndex(luaL_checkstring(L, 1)));
    return 1;
}

static int N_aiClaim(lua_State* L) {
    D2LuaWorld* w = World(L);
    int nIndex = ArgInt(L, 1, "aiClaim");
    int nClaim = ArgInt(L, 2, "aiClaim");
    int nTargeting = ArgInt(L, 3, "aiClaim");
    if (nIndex < 0 || nIndex >= D2LUA_AI_STOCK_COUNT || nClaim < 0 || nClaim > 2 || nTargeting < 0 || nTargeting > 2) {
        return luaL_error(L, "aiClaim: bad claim");
    }
    w->aClaims[nIndex] = (uint8_t)nClaim;
    w->aTargeting[nIndex] = (uint8_t)nTargeting;
    w->AiClaim(nIndex, nClaim, nTargeting);
    return 0;
}

// aiUnit(m): the monster's AI id, whether its think is the one running now, AI state, AI delay.
static int N_aiUnit(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckMonster(L, 1, "monster");
    lua_pushinteger(L, w->AiIndex(p));
    lua_pushboolean(L, w->pThinking == p);
    lua_pushinteger(L, w->AiState(p));
    lua_pushinteger(L, w->AiDelay(p));
    return 4;
}

// The monster whose think runs now, checked against the handle at 1: actions start only there.
static D2UnitStrc* CheckThinking(lua_State* L, const char* pszWhere) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckMonster(L, 1, pszWhere);
    if (w->pThinking != p) {
        luaL_error(L, "%s: a monster acts only inside its own AI's think", pszWhere);
    }
    return p;
}

static int N_aiIdle(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckThinking(L, "monster.idle");
    int nFrames = ArgInt(L, 2, "monster.idle");
    if (nFrames < 1 || nFrames > 25 * 60) {
        return luaL_error(L, "monster.idle: frames %d is outside 1..1500", nFrames);
    }
    w->AiIdle(p, nFrames);
    w->bScheduled = true;
    return 0;
}

// aiMove(m, target|nil, x, y, run)
static int N_aiMove(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckThinking(L, "monster.move");
    D2UnitStrc* pTarget = CheckUnit(L, 2, "monster.move", true);
    int nX = pTarget ? 0 : ArgInt(L, 3, "monster.move");
    int nY = pTarget ? 0 : ArgInt(L, 4, "monster.move");
    if (pTarget == p) {
        return luaL_error(L, "monster.move: a monster cannot move to itself");
    }
    w->AiMarkAction(p);
    bool bOk = w->AiMove(p, pTarget, nX, nY, lua_toboolean(L, 5) ? D2LUA_MOVE_RUN : D2LUA_MOVE_WALK);
    w->bScheduled = w->bScheduled || bOk;
    lua_pushboolean(L, bOk);
    return 1;
}

static int N_aiAttack(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckThinking(L, "monster.attack");
    D2UnitStrc* pTarget = CheckUnit(L, 2, "monster.attack");
    if (pTarget == p) {
        return luaL_error(L, "monster.attack: a monster cannot attack itself");
    }
    w->AiMarkAction(p);
    bool bOk = w->AiAttack(p, pTarget, lua_toboolean(L, 3) != 0);
    w->bScheduled = true;  // a failed start goes back to neutral, which arms the AI delay
    lua_pushboolean(L, bOk);
    return 1;
}

// aiCast(m, skill, target|nil, x, y, mode, sequence): SERVER_UNIT_CastSkill in the mode, or with
// sequence MONSTERAI_CommandSkillSequence.
static int N_aiCast(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckThinking(L, "monster.castSkill");
    int nSkill = ArgInt(L, 2, "monster.castSkill");
    if (nSkill < 0 || nSkill > 0xFFFF) {
        return luaL_error(L, "monster.castSkill: no skill %d", nSkill);
    }
    D2UnitStrc* pTarget = CheckUnitOrCorpse(L, 3, "monster.castSkill", true);
    int nX = pTarget ? 0 : ArgInt(L, 4, "monster.castSkill");
    int nY = pTarget ? 0 : ArgInt(L, 5, "monster.castSkill");
    int nMode = ArgInt(L, 6, "monster.castSkill");
    if (nMode < 0 || nMode >= 16) {
        return luaL_error(L, "monster.castSkill: mode %d is not a monster mode", nMode);
    }
    bool bSequence = lua_toboolean(L, 7) != 0;
    w->AiMarkAction(p);
    bool bOk = bSequence ? w->AiSequence(p, nSkill, pTarget, nX, nY) : w->AiCast(p, nSkill, nMode, pTarget, nX, nY);
    // A failed cast arms a think itself; a sequence that could not start arms none (the host then
    // idles the monster for its AI delay).
    w->bScheduled = w->bScheduled || !bSequence || bOk;
    lua_pushboolean(L, bOk);
    return 1;
}

// aiCircle(m, target, steps)
static int N_aiCircle(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckThinking(L, "monster.circleToward");
    D2UnitStrc* pTarget = CheckUnit(L, 2, "monster.circleToward");
    int nSteps = ArgInt(L, 3, "monster.circleToward");
    if (pTarget == p) {
        return luaL_error(L, "monster.circleToward: a monster cannot close in on itself");
    }
    if (nSteps < 1 || nSteps > 255) {
        return luaL_error(L, "monster.circleToward: steps %d is outside 1..255", nSteps);
    }
    w->AiMarkAction(p);
    bool bOk = w->AiCircle(p, pTarget, nSteps);
    w->bScheduled = w->bScheduled || bOk;
    lua_pushboolean(L, bOk);
    return 1;
}

static int N_aiWander(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckThinking(L, "monster.wander");
    int nRadius = ArgInt(L, 2, "monster.wander");
    if (nRadius < 1 || nRadius > 30) {
        return luaL_error(L, "monster.wander: radius %d is outside 1..30", nRadius);
    }
    w->AiMarkAction(p);
    bool bOk = w->AiWander(p, nRadius);
    w->bScheduled = w->bScheduled || bOk;
    lua_pushboolean(L, bOk);
    return 1;
}

static int N_aiFlee(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckThinking(L, "monster.flee");
    D2UnitStrc* pFrom = CheckUnit(L, 2, "monster.flee");
    int nDistance = ArgInt(L, 3, "monster.flee");
    if (nDistance < 1 || nDistance > 30) {
        return luaL_error(L, "monster.flee: distance %d is outside 1..30", nDistance);
    }
    w->AiMarkAction(p);
    bool bOk = w->AiFlee(p, pFrom, nDistance);
    w->bScheduled = w->bScheduled || bOk;
    lua_pushboolean(L, bOk);
    return 1;
}

static int N_aiStock(lua_State* L) {
    D2LuaWorld* w = World(L);
    D2UnitStrc* p = CheckThinking(L, "next");
    w->bScheduled = true;
    w->AiStock(p);
    return 0;
}

static const luaL_Reg s_aWorldNatives[] = {
    { "gameInfo", N_gameInfo },
    { "players", N_players },
    { "message", N_message },
    { "findUnit", N_findUnit },
    { "spawnMonster", N_spawnMonster },
    { "unitsNear", N_unitsNear },
    { "level", N_level },
    { "levelHandle", N_levelHandle },
    { "levelId", N_levelId },
    { "levelInfo", N_levelInfo },
    { "levelRooms", N_levelRooms },
    { "levelUnits", N_levelUnits },
    { "roomInfo", N_roomInfo },
    { "roomNeighbours", N_roomNeighbours },
    { "roomUnits", N_roomUnits },
    { "unitInfo", N_unitInfo },
    { "handleId", N_handleId },
    { "unitPosition", N_unitPosition },
    { "unitRoom", N_unitRoom },
    { "unitPlace", N_unitPlace },
    { "unitOwner", N_unitOwner },
    { "getStat", N_getStat },
    { "setStat", N_setStat },
    { "hasState", N_hasState },
    { "addState", N_addState },
    { "removeState", N_removeState },
    { "monsterInfo", N_monsterInfo },
    { "kill", N_kill },
    { "target", N_target },
    { "playerName", N_playerName },
    { "sendMessage", N_sendMessage },
    { "aiName", N_aiName },
    { "aiIndex", N_aiIndex },
    { "aiClaim", N_aiClaim },
    { "aiUnit", N_aiUnit },
    { "aiIdle", N_aiIdle },
    { "aiMove", N_aiMove },
    { "aiAttack", N_aiAttack },
    { "aiCast", N_aiCast },
    { "aiWander", N_aiWander },
    { "aiFlee", N_aiFlee },
    { "aiStock", N_aiStock },
    { "aiCircle", N_aiCircle },
    { "aiDistance", N_aiDistance },
    { "aiFindTarget", N_aiFindTarget },
    { "aiCanCast", N_aiCanCast },
    { "aiOrderMinions", N_aiOrderMinions },
    { "monsterData", N_monsterData },
    { "unitRoll", N_unitRoll },
    { "corpseInfo", N_corpseInfo },
    { "alignment", N_alignment },
    { "hasStateMask", N_hasStateMask },
    { "leader", N_leader },
    { "minions", N_minions },
    { "corpsesNear", N_corpsesNear },
    { nullptr, nullptr },
};

// members(class): the class's method, getter and setter tables (the core fills them).
static int N_members(lua_State* L) {
    const char* pszClass = luaL_checkstring(L, 1);
    if (!D2LuaHandle_PushMembers(L, pszClass)) {
        return luaL_error(L, "no handle class %s", pszClass);
    }
    return 3;
}

static void Register_world(lua_State* L, int nSide) {
    if (!(nSide & D2LUA_SIDE_SERVER)) {
        return;
    }
    D2LuaHandle_Register(L, &kClass_Player);
    D2LuaHandle_Register(L, &kClass_Monster);
    D2LuaHandle_Register(L, &kClass_Unit);
    D2LuaHandle_Register(L, &kClass_Level);
    D2LuaHandle_Register(L, &kClass_Room);
    D2LuaHandle_Register(L, &kClass_Corpse);
}

static void Natives_world(lua_State* L, int nTable, int nSide) {
    if (!(nSide & D2LUA_SIDE_SERVER)) {
        return;
    }
    D2LuaApi_PushNatives(L, s_aWorldNatives);
    lua_pushcfunction(L, N_members);
    lua_setfield(L, -2, "members");
    lua_setfield(L, nTable, "world");
}

const D2LuaApiDesc g_D2LuaApi_world = { "world", Register_world, Natives_world };

// --- the think bookkeeping -----------------------------------------------------------------------

void D2LuaWorld::AiRestart(D2UnitStrc* pUnit) {
    tRestart.insert(UnitGuid(pUnit));
}

bool D2LuaWorld::Think(D2LuaRuntime* pRt, uint32_t nGame, D2UnitStrc* pUnit, D2UnitStrc* pTarget, int nDistance, bool bEngaged) {
    uint32_t nGuid = UnitGuid(pUnit);
    if (tThinking.count(nGuid)) {
        return true;  // a think inside its own think: the outer one schedules the next
    }
    int nAi = AiIndex(pUnit);
    const char* pszAi = D2LuaWorld_AiName(nAi);
    if (!pszAi || !pRt) {
        return false;
    }
    bool bRestart = tRestart.erase(nGuid) != 0;
    tThinking.insert(nGuid);
    tScripted.insert(nGuid);
    bool bWasScheduled = bScheduled;
    D2UnitStrc* pWasThinking = pThinking;
    bScheduled = false;
    pThinking = pUnit;
    lua_State* L = D2LuaRt_State(pRt);
    int nTop = lua_gettop(L);
    lua_pushnumber(L, (lua_Number)nGame);
    PushUnit(L, this, pUnit);
    lua_pushstring(L, pszAi);
    lua_pushboolean(L, bRestart);
    if (pTarget && FindUnit(UnitType(pTarget), UnitGuid(pTarget))) {
        PushUnit(L, this, pTarget);
    } else {
        lua_pushnil(L);
    }
    lua_pushinteger(L, nDistance);
    lua_pushboolean(L, bEngaged);
    bool bRan = D2LuaRt_CallHost(pRt, nGame, "aiThink", 7, 1) && lua_toboolean(L, -1);
    lua_settop(L, nTop);
    bool bThought = bScheduled;
    bScheduled = bWasScheduled;
    pThinking = pWasThinking;
    tThinking.erase(nGuid);
    if (!bRan) {
        return bThought;  // the core did not run it; the caller runs the stock think unless it did
    }
    if (!bThought && FindUnit(D2LUA_UNIT_MONSTER, nGuid) == pUnit && !IsDead(pUnit)) {
        AiIdle(pUnit, AiDelay(pUnit));  // nothing armed the next think: the monster must not stall
    }
    return true;
}

void D2LuaWorld::AiDrop(D2LuaRuntime* pRt, uint32_t nGame, uint32_t nGuid) {
    tRestart.erase(nGuid);
    if (!tScripted.erase(nGuid) || !pRt) {
        return;
    }
    lua_State* L = D2LuaRt_State(pRt);
    int nTop = lua_gettop(L);
    lua_pushnumber(L, (lua_Number)nGame);
    lua_pushnumber(L, (lua_Number)UnitId(D2LUA_UNIT_MONSTER, nGuid));
    D2LuaRt_CallHost(pRt, nGame, "aiDrop", 2, 0);
    lua_settop(L, nTop);
}
#endif
