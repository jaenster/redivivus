// The world classes of redivivus:server (Unit, Monster, Player, Room, Level) over the native
// handle classes of d2_lua_world.cpp. A handle is a native userdata naming an engine object; its
// members are the functions installed here into the class's member tables, so they are core code
// and every one of them reaches the engine through a native that resolves the handle again. The
// game members (difficulty, players(), spawnMonster(), ...) are added to each mod's Game object.
//
// A state with a world serves exactly one game (the engine makes a state per game).
import type { AnyUnit, Level, Monster, MonsterData, Player, Room, SpawnOptions, Subtile, Unit } from "../redivivus/server";
import { MonsterTypeFlag, StatId, UnitType } from "../redivivus/enums";
import type { NativeHandle, NativeWorld } from "../native/world";
import * as ai from "./ai";
import { argBoolean, argInteger, argNumber, argString, argTable } from "./check";
import * as games from "./games";
import { native, world } from "./native";
import * as points from "./points";

type Fn = (this: void, ...args: any[]) => unknown;

// The six stats the engine keeps in 1/256ths.
const FIXED = new LuaTable<number, boolean>();
for (const s of [StatId.Hitpoints, StatId.Maxhp, StatId.Mana, StatId.Maxmana, StatId.Stamina, StatId.Maxstamina]) FIXED.set(s, true);

let installed = false;

function W(): NativeWorld {
    if (!world) return error("this state has no game world", 3);
    return world;
}

/** The handles of a capture as a Lua iterator: each step skips a unit that is no longer valid. */
export function iterate<T>(list: unknown[]): LuaIterable<T> {
    let i = 0;
    return (() => {
        while (i < list.length) {
            const v = list[i];
            i++;
            if (native.handleIsLive(v)) return v;
        }
        return undefined;
    }) as unknown as LuaIterable<T>;
}

function typeArg(v: unknown, i: number, where: string): number | undefined {
    if (v === undefined) return undefined;
    return argInteger(v, i, where);
}

function position(u: NativeHandle): Subtile {
    const [x, y] = W().unitPosition(u);
    if (x === undefined) return error("this unit has no position in the world", 3);
    return points.subtile(x, y);
}

/** A unit or corpse handle or a subtile point, as a point. */
function placeOf(v: unknown, i: number, where: string): Subtile {
    if (native.isHandle(v)) {
        const [, cls] = W().handleId(v);
        if (cls === "Player" || cls === "Monster" || cls === "Unit") return position(v as NativeHandle);
        if (cls === "Corpse") {
            const [, x, y] = W().corpseInfo(v as NativeHandle);
            return points.subtile(x, y);
        }
    }
    return points.subtileArg(v, i, where);
}

function statValue(stat: number, v: number): number {
    return FIXED.get(stat) ? math.floor(v / 256) : v;
}

function statArg(stat: unknown, value: unknown, where: string): [number, number] {
    const s = argInteger(stat, 1, where);
    const v = argNumber(value, 2, where);
    if (s === StatId.Gold || s === StatId.Goldbank) error(`${where}: gold is not set as a stat (the engine zeroes gold past the cap)`, 3);
    const raw = FIXED.get(s) ? math.floor(v * 256) : math.floor(v);
    if (raw < -2147483648 || raw > 2147483647) error(`${where}: ${v} is outside the stat's range`, 3);
    return [s, raw];
}

function gameObject(): object | undefined {
    const ids = games.ids();
    const g = ids.length > 0 ? games.find(ids[0]) : undefined;
    return g ? g.objects.get(native.owner()) : undefined;
}

function fill(cls: string, getters: Record<string, Fn>, methods: Record<string, Fn>, setters: Record<string, Fn>): void {
    const [m, g, s] = W().members(cls);
    for (const [k, f] of pairs(getters)) g.set(k as string, f);
    for (const [k, f] of pairs(methods)) m.set(k as string, f);
    for (const [k, f] of pairs(setters)) s.set(k as string, f);
}

const unitGetters: Record<string, Fn> = {
    id: (u: NativeHandle) => W().unitInfo(u)[1],
    type: (u: NativeHandle) => W().unitInfo(u)[0],
    classId: (u: NativeHandle) => W().unitInfo(u)[2],
    game: () => gameObject(),
    position: (u: NativeHandle) => position(u),
    room: (u: NativeHandle) => W().unitRoom(u),
    level: (u: NativeHandle) => W().levelHandle(W().unitPlace(u)[0]),
    act: (u: NativeHandle) => W().unitPlace(u)[1],
    mode: (u: NativeHandle) => W().unitPlace(u)[2],
    owner: (u: NativeHandle) => W().unitOwner(u),
};

const unitMethods: Record<string, Fn> = {
    distanceTo: (u: NativeHandle, other: unknown) => {
        const a = position(u);
        const b = placeOf(other, 1, "unit.distanceTo");
        return math.sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
    },
    nearby: (u: NativeHandle, radius: unknown, t?: unknown) => {
        const p = position(u);
        return iterate(W().unitsNear(p.x, p.y, argNumber(radius, 1, "unit.nearby"), typeArg(t, 2, "unit.nearby"), u));
    },
    getStat: (u: NativeHandle, stat: unknown, layer?: unknown) => {
        const s = argInteger(stat, 1, "unit.getStat");
        return statValue(s, W().getStat(u, s, typeArg(layer, 2, "unit.getStat"), false));
    },
    getBaseStat: (u: NativeHandle, stat: unknown, layer?: unknown) => {
        const s = argInteger(stat, 1, "unit.getBaseStat");
        return statValue(s, W().getStat(u, s, typeArg(layer, 2, "unit.getBaseStat"), true));
    },
    setStat: (u: NativeHandle, stat: unknown, value: unknown, layer?: unknown) => {
        const [s, v] = statArg(stat, value, "unit.setStat");
        W().setStat(u, s, v, typeArg(layer, 3, "unit.setStat"), false);
    },
    addStat: (u: NativeHandle, stat: unknown, value: unknown, layer?: unknown) => {
        const [s, v] = statArg(stat, value, "unit.addStat");
        W().setStat(u, s, v, typeArg(layer, 3, "unit.addStat"), true);
    },
    hasState: (u: NativeHandle, state: unknown) => W().hasState(u, argInteger(state, 1, "unit.hasState")),
    addState: (u: NativeHandle, state: unknown, frames?: unknown) => W().addState(u, argInteger(state, 1, "unit.addState"), typeArg(frames, 2, "unit.addState")),
    removeState: (u: NativeHandle, state: unknown) => W().removeState(u, argInteger(state, 1, "unit.removeState")),
};

function flag(bit: number): Fn {
    return (u: NativeHandle) => (W().monsterInfo(u)[0] & bit) !== 0;
}

const monsterGetters: Record<string, Fn> = {
    champion: flag(MonsterTypeFlag.Champion),
    unique: flag(MonsterTypeFlag.Unique),
    superUnique: flag(MonsterTypeFlag.Superunique),
    minion: flag(MonsterTypeFlag.Minion),
    boss: (u: NativeHandle) => W().monsterInfo(u)[1],
    life: (u: NativeHandle) => statValue(StatId.Hitpoints, W().getStat(u, StatId.Hitpoints, 0, false)),
    maxLife: (u: NativeHandle) => statValue(StatId.Maxhp, W().getStat(u, StatId.Maxhp, 0, false)),
    target: (u: NativeHandle) => W().target(u),
};

const monsterSetters: Record<string, Fn> = {
    life: (u: NativeHandle, v: unknown) => {
        const n = argNumber(v, 1, "monster.life");
        const max = statValue(StatId.Maxhp, W().getStat(u, StatId.Maxhp, 0, false));
        if (n <= 0) error("monster.life: 0 or less does not kill; use monster.kill()", 2);
        if (n > max) error(`monster.life: ${n} is more than its maxLife ${max}`, 2);
        W().setStat(u, StatId.Hitpoints, math.floor(n * 256), 0, false);
    },
};

type RawMonsterData = { baseId: number; ai: number; aiDelay: number; aiParams: number[]; skills: number[]; skillModes: number[] };

/** MonStats rows as the natives give them, by class (a state serves one game: one difficulty). */
const rows = new LuaTable<number, RawMonsterData | false>();

function rowOf(classId: number): RawMonsterData | undefined {
    let r = rows.get(classId);
    if (r === undefined) {
        r = W().monsterData(classId) ?? false;
        rows.set(classId, r);
    }
    return r === false ? undefined : r;
}

function copy(a: number[]): number[] {
    const out: number[] = [];
    for (const v of a) out.push(v);
    return out;
}

/** A fresh MonsterData: a plain table a mod may keep, change or drop without touching anyone else's. */
function monsterData(classId: number): MonsterData | undefined {
    const r = rowOf(classId);
    if (!r) return undefined;
    return {
        id: classId,
        baseId: r.baseId,
        ai: W().aiName(r.ai) ?? "None",
        aiDelay: r.aiDelay,
        aiParams: copy(r.aiParams),
        skills: copy(r.skills),
        skillModes: copy(r.skillModes),
    } as unknown as MonsterData;
}

/** A unit or a corpse handle, checked. */
function targetArg(v: unknown, i: number, where: string): NativeHandle {
    if (!native.isHandle(v)) error(`bad argument #${i} to '${where}' (a unit or corpse expected, got ${type(v)})`, 3);
    const [, cls] = W().handleId(v);
    if (cls !== "Player" && cls !== "Monster" && cls !== "Unit" && cls !== "Corpse") error(`bad argument #${i} to '${where}' (a unit or corpse expected, got a ${cls})`, 3);
    if (!native.handleIsLive(v)) error(`bad argument #${i} to '${where}' (the ${cls} is not valid)`, 3);
    return v as NativeHandle;
}

const monsterMoreGetters: Record<string, Fn> = {
    data: (u: NativeHandle) => monsterData(W().unitInfo(u)[2]),
    leader: (u: NativeHandle) => W().leader(u),
    alignment: (u: NativeHandle) => W().alignment(u),
};

const monsterMethods: Record<string, Fn> = {
    kill: (u: NativeHandle, killer?: unknown) => W().kill(u, killer as NativeHandle | undefined),
    random: (u: NativeHandle, max: unknown) => {
        const n = argInteger(max, 1, "monster.random");
        if (n < 1) error(`monster.random: max must be 1 or more, got ${n}`, 2);
        return W().unitRoll(u) % n;
    },
    hasStateMask: (u: NativeHandle, mask: unknown) => W().hasStateMask(u, argInteger(mask, 1, "monster.hasStateMask")),
    aiDistanceTo: (u: NativeHandle, other: unknown) => W().aiDistance(u, targetArg(other, 1, "monster.aiDistanceTo")),
    minions: (u: NativeHandle) => iterate(W().minions(u, false)),
    minionCorpses: (u: NativeHandle) => iterate(W().minions(u, true)),
    corpsesNear: (u: NativeHandle, radius: unknown) => iterate(W().corpsesNear(u, argNumber(radius, 1, "monster.corpsesNear"))),
    findTarget: (u: NativeHandle) => {
        const [t, d] = W().aiFindTarget(u);
        return t === undefined ? undefined : { target: t, distance: d };
    },
    canCast: (u: NativeHandle, skill: unknown, at: unknown) =>
        W().aiCanCast(u, argInteger(skill, 1, "monster.canCast"), targetArg(at, 2, "monster.canCast"), 0, 0),
    orderMinions: (u: NativeHandle, order: unknown) => W().aiOrderMinions(u, argInteger(order, 1, "monster.orderMinions")),
    ...ai.actions(),
};

function corpseFlag(bit: number): Fn {
    return (c: NativeHandle) => (W().corpseInfo(c)[6] & bit) !== 0;
}

const corpseGetters: Record<string, Fn> = {
    id: (c: NativeHandle) => W().handleId(c)[0],
    classId: (c: NativeHandle) => W().corpseInfo(c)[0],
    baseId: (c: NativeHandle) => {
        const cls = W().corpseInfo(c)[0];
        return rowOf(cls)?.baseId ?? cls;
    },
    position: (c: NativeHandle) => {
        const [, x, y] = W().corpseInfo(c);
        return points.subtile(x, y);
    },
    mode: (c: NativeHandle) => W().corpseInfo(c)[3],
    alignment: (c: NativeHandle) => W().corpseInfo(c)[4],
    targetable: (c: NativeHandle) => W().corpseInfo(c)[5],
    champion: corpseFlag(MonsterTypeFlag.Champion),
    unique: corpseFlag(MonsterTypeFlag.Unique),
    leader: (c: NativeHandle) => W().leader(c),
};

const corpseMethods: Record<string, Fn> = {
    hasStateMask: (c: NativeHandle, mask: unknown) => W().hasStateMask(c, argInteger(mask, 1, "corpse.hasStateMask")),
    distanceTo: (c: NativeHandle, other: unknown) => {
        const a = placeOf(c, 0, "corpse.distanceTo");
        const b = placeOf(other, 1, "corpse.distanceTo");
        return math.sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
    },
};

const playerGetters: Record<string, Fn> = {
    name: (u: NativeHandle) => W().playerName(u),
    charLevel: (u: NativeHandle) => W().getStat(u, StatId.Level, 0, false),
    experience: (u: NativeHandle) => {
        const v = W().getStat(u, StatId.Experience, 0, false);
        return v < 0 ? v + 4294967296 : v;
    },
    gold: (u: NativeHandle) => W().getStat(u, StatId.Gold, 0, false),
    goldStash: (u: NativeHandle) => W().getStat(u, StatId.Goldbank, 0, false),
    dead: (u: NativeHandle) => W().unitPlace(u)[3],
};

const playerMethods: Record<string, Fn> = {
    sendMessage: (u: NativeHandle, text: unknown) => W().sendMessage(u, argString(text, 1, "player.sendMessage")),
};

function levelId(l: NativeHandle): number {
    return W().levelId(l);
}

const levelGetters: Record<string, Fn> = {
    id: (l: NativeHandle) => levelId(l),
    name: (l: NativeHandle) => W().levelInfo(levelId(l))[0],
    act: (l: NativeHandle) => W().levelInfo(levelId(l))[1],
    isTown: (l: NativeHandle) => W().levelInfo(levelId(l))[2],
    generated: (l: NativeHandle) => W().levelInfo(levelId(l))[3],
    bounds: (l: NativeHandle) => {
        const [, , , gen, x, y, w, h] = W().levelInfo(levelId(l));
        return gen ? points.rect(x!, y!, w!, h!) : undefined;
    },
};

const levelMethods: Record<string, Fn> = {
    rooms: (l: NativeHandle) => iterate(W().levelRooms(l)),
    units: (l: NativeHandle, t?: unknown) => iterate(W().levelUnits(l, typeArg(t, 1, "level.units"))),
    monsters: (l: NativeHandle) => iterate(W().levelUnits(l, UnitType.Monster)),
    players: (l: NativeHandle) => iterate(W().levelUnits(l, UnitType.Player)),
};

const roomGetters: Record<string, Fn> = {
    bounds: (r: NativeHandle) => {
        const [x, y, w, h] = W().roomInfo(r);
        return points.rect(x, y, w, h);
    },
    level: (r: NativeHandle) => W().levelHandle(W().roomInfo(r)[4]),
};

const roomMethods: Record<string, Fn> = {
    neighbours: (r: NativeHandle) => iterate(W().roomNeighbours(r)),
    units: (r: NativeHandle, t?: unknown) => iterate(W().roomUnits(r, typeArg(t, 1, "room.units"))),
    monsters: (r: NativeHandle) => iterate(W().roomUnits(r, UnitType.Monster)),
    players: (r: NativeHandle) => iterate(W().roomUnits(r, UnitType.Player)),
};

/** Fill the native classes' member tables, once per state. */
export function install(): boolean {
    if (installed) return true;
    if (!world) return false;
    installed = true;
    for (const cls of ["Player", "Monster", "Unit"]) fill(cls, unitGetters, unitMethods, {});
    fill("Monster", monsterGetters, monsterMethods, monsterSetters);
    fill("Monster", monsterMoreGetters, {}, {});
    fill("Corpse", corpseGetters, corpseMethods, {});
    fill("Player", playerGetters, playerMethods, {});
    fill("Level", levelGetters, levelMethods, {});
    fill("Room", roomGetters, roomMethods, {});
    return true;
}

/** The game members each mod's Game object gets in a state with a world. */
export function gameMembers(getters: LuaTable<string, (this: void) => unknown>, values: Record<string, unknown>): void {
    if (!install()) return;
    getters.set("difficulty", () => W().gameInfo()[0]);
    getters.set("expansion", () => W().gameInfo()[1]);
    getters.set("name", () => W().gameInfo()[2]);
    getters.set("seed", () => W().gameInfo()[3]);
    getters.set("frame", () => W().gameInfo()[4]);
    values.players = (): LuaIterable<Player> => iterate(W().players());
    values.level = (id: unknown): Level | undefined => W().level(argInteger(id, 1, "game.level")) as Level | undefined;
    values.findUnit = (t: unknown, id: unknown): AnyUnit | undefined =>
        W().findUnit(argInteger(t, 1, "game.findUnit"), argNumber(id, 2, "game.findUnit")) as AnyUnit | undefined;
    values.unitsNear = (at: unknown, radius: unknown, t?: unknown): LuaIterable<AnyUnit> => {
        const p = points.subtileArg(at, 1, "game.unitsNear");
        return iterate(W().unitsNear(p.x, p.y, argNumber(radius, 2, "game.unitsNear"), typeArg(t, 3, "game.unitsNear"), undefined));
    };
    values.spawnMonster = (id: unknown, at: unknown, options?: unknown): Monster | undefined => {
        const cls = argInteger(id, 1, "game.spawnMonster");
        const p = placeOf(at, 2, "game.spawnMonster");
        let radius = 4;
        let minions = false;
        if (options !== undefined) {
            argTable(options, 3, "game.spawnMonster");
            const r = rawget(options as object, "radius");
            if (r !== undefined) radius = argInteger(r, 3, "game.spawnMonster (options.radius)");
            const mi = rawget(options as object, "minions");
            if (mi !== undefined) minions = argBoolean(mi, 3, "game.spawnMonster (options.minions)");
        }
        return W().spawnMonster(cls, p.x, p.y, radius, minions) as Monster | undefined;
    };
    values.message = (text: unknown): void => W().message(argString(text, 1, "game.message"));
}

export type { Room, Unit };
