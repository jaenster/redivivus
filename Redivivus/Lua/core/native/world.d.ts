/**
 * The game world's natives as the core sees them (Redivivus/Lua/d2_lua_world.cpp, table "world"
 * of the natives): present in a server state that runs a game. Mods never see this table. Unit,
 * room and level arguments are handles of the native classes Player, Monster, Unit, Room and Level;
 * every native resolves them and raises a clean error for one that is not valid.
 * Hand-written: these natives are registered by hand, not by the generator.
 */

/** A handle of one of the native classes. */
export type NativeHandle = object;

/** @noSelf */
export interface NativeWorld {
    /** The members of a handle class: methods, getters, setters (the core fills them). */
    members(className: string): LuaMultiReturn<[LuaTable<string, unknown>, LuaTable<string, unknown>, LuaTable<string, unknown>]>;
    gameInfo(): LuaMultiReturn<[number, boolean, string, number, number]>;
    players(): NativeHandle[];
    message(text: string): void;
    findUnit(type: number, guid: number): NativeHandle | undefined;
    spawnMonster(classId: number, x: number, y: number, radius: number, minions: boolean): NativeHandle | undefined;
    unitsNear(x: number, y: number, radius: number, type: number | undefined, exclude: NativeHandle | undefined): NativeHandle[];
    level(id: number): NativeHandle | undefined;
    levelHandle(id: number): NativeHandle | undefined;
    levelId(level: NativeHandle): number;
    /** name, act, town, generated, then x, y, width, height when generated; undefined for no such level. */
    levelInfo(id: number): LuaMultiReturn<[string | undefined, number, boolean, boolean, number?, number?, number?, number?]>;
    levelRooms(level: NativeHandle): NativeHandle[];
    levelUnits(level: NativeHandle, type: number | undefined): NativeHandle[];
    /** x, y, width, height (subtiles), level id. */
    roomInfo(room: NativeHandle): LuaMultiReturn<[number, number, number, number, number]>;
    roomNeighbours(room: NativeHandle): NativeHandle[];
    roomUnits(room: NativeHandle, type: number | undefined): NativeHandle[];
    /** type, guid, class id. */
    unitInfo(unit: NativeHandle): LuaMultiReturn<[number, number, number]>;
    /** The id and class name of any handle, valid or not. */
    handleId(v: unknown): LuaMultiReturn<[number | undefined, string]>;
    unitPosition(unit: NativeHandle): LuaMultiReturn<[number | undefined, number]>;
    unitRoom(unit: NativeHandle): NativeHandle | undefined;
    /** level id, act, mode, dead. */
    unitPlace(unit: NativeHandle): LuaMultiReturn<[number, number, number, boolean]>;
    unitOwner(unit: NativeHandle): NativeHandle | undefined;
    getStat(unit: NativeHandle, stat: number, layer: number | undefined, base: boolean): number;
    setStat(unit: NativeHandle, stat: number, value: number, layer: number | undefined, add: boolean): void;
    hasState(unit: NativeHandle, state: number): boolean;
    addState(unit: NativeHandle, state: number, frames: number | undefined): boolean;
    removeState(unit: NativeHandle, state: number): void;
    /** type flags, boss. */
    monsterInfo(monster: NativeHandle): LuaMultiReturn<[number, boolean]>;
    kill(monster: NativeHandle, killer: NativeHandle | undefined): void;
    target(unit: NativeHandle): NativeHandle | undefined;
    playerName(player: NativeHandle): string;
    sendMessage(player: NativeHandle, text: string): void;
    aiName(index: number): string | undefined;
    aiIndex(name: string): number;
    /** Claim an AI id for this game: 0 none, 1 wrapped, 2 registered; targeting 0..2. */
    aiClaim(index: number, claim: number, targeting: number): void;
    /** AI id, whether its think runs now, AI state, AI delay. */
    aiUnit(monster: NativeHandle): LuaMultiReturn<[number, boolean, number, number]>;
    aiIdle(monster: NativeHandle, frames: number): void;
    aiMove(monster: NativeHandle, target: NativeHandle | undefined, x: number, y: number, run: boolean): boolean;
    aiAttack(monster: NativeHandle, target: NativeHandle, secondary: boolean): boolean;
    /** SERVER_UNIT_CastSkill in mode, or with sequence MONSTERAI_CommandSkillSequence. Target: a unit or corpse. */
    aiCast(monster: NativeHandle, skill: number, target: NativeHandle | undefined, x: number, y: number, mode: number, sequence: boolean): boolean;
    aiCircle(monster: NativeHandle, target: NativeHandle, steps: number): boolean;
    aiDistance(monster: NativeHandle, other: NativeHandle): number;
    aiFindTarget(monster: NativeHandle): LuaMultiReturn<[NativeHandle | undefined, number]>;
    aiCanCast(monster: NativeHandle, skill: number, target: NativeHandle | undefined, x: number, y: number): boolean;
    aiOrderMinions(monster: NativeHandle, order: number): void;
    /** The MonStats row (arrays 1-based), or undefined. */
    monsterData(classId: number): { baseId: number; ai: number; aiDelay: number; aiParams: number[]; skills: number[]; skillModes: number[] } | undefined;
    /** The next value of the monster's seed. */
    unitRoll(monster: NativeHandle): number;
    /** class, x, y, mode, alignment, targetable, type flags. */
    corpseInfo(corpse: NativeHandle): LuaMultiReturn<[number, number, number, number, number, boolean, number]>;
    alignment(unitOrCorpse: NativeHandle): number;
    hasStateMask(unitOrCorpse: NativeHandle, mask: number): boolean;
    leader(unitOrCorpse: NativeHandle): NativeHandle | undefined;
    minions(monster: NativeHandle, dead: boolean): NativeHandle[];
    corpsesNear(monster: NativeHandle, radius: number): NativeHandle[];
    aiWander(monster: NativeHandle, radius: number): boolean;
    aiFlee(monster: NativeHandle, from: NativeHandle, distance: number): boolean;
    aiStock(monster: NativeHandle): void;
}
