// Monster AI: game.registry.ai, the script that drives a monster, and the actions a script runs.
//
// The engine asks for the AI of a monster whose AI id a mod claimed (registered or wrapped) and
// gets the host's entry (d2_lua_world.h): each think then comes here as aiThink, through the
// registry entry of that id in this game. The bottom of the entry is the stock think (a native)
// until a mod registers a script; wrappers sit above as in every registry, and next() runs one think
// of what is below.
//
// A registered script runs as a coroutine per monster, created at the monster's first think (and
// again after the engine attached the AI anew, the script returned, or the monster died and was
// revived). Each think resumes it once:
//   - a bare yield: think again next frame
//   - an action (yield* monster.walkTo(p)): the engine runs it and thinks again when it ends; the
//     action then returns its Outcome
//   - the script returns: it starts over at the next think
//   - it raises: logged, and the monster idles until the engine attaches its AI again
// The C++ side arms a think whenever a think armed none, so a monster never stalls.
import { CallStatus } from "../native/core";
import type { NativeHandle, NativeWorld } from "../native/world";
import type { AiScript, AiWrapper, Monster, Subtile, Think } from "../redivivus/server";
import { MonsterMode, Outcome } from "../redivivus/enums";
import { argFunction, argInteger, argString, show } from "./check";
import type { GameState } from "./games";
import * as mods from "./mods";
import { native, world } from "./native";
import * as points from "./points";
import { readonly } from "./proxy";
import * as registries from "./registries";

type Fn = (this: void, ...args: any[]) => unknown;

const REGISTRY = "ai";
const AI_ACTION = {};
const UNCLAIMED = 0;
const WRAPPED = 1;
const REGISTERED = 2;
const GET_HIT = 3;
const KNOCKBACK = 13;
const BROKEN_IDLE = 100;

type Pending = { kind: string; goal?: NativeHandle; x?: number; y?: number };
type ThinkState = { target?: NativeHandle; distance: number; engaged: boolean; view: Think };
type Driver = { script: AiScript; mod: number };
type UnitAi = { unit: NativeHandle; co?: LuaThread; driver?: Driver; pending?: Pending; broken: boolean; think: ThinkState };

/** game id -> unit id -> its AI state. */
const units = new LuaTable<number, LuaTable<number, UnitAi>>();
/**
 * The coroutine of a running script (or of a body a guard runs inside it) -> its state. Weak both
 * ways: the state holds its coroutine, so with weak keys alone (no ephemerons in LuaJIT) an entry
 * would keep itself alive; the state of a live monster is held by `units`.
 */
const byThread = new LuaTable<LuaThread, UnitAi>();
setmetatable(byThread, { __mode: "kv" });
/** game id -> AI id -> what was claimed in it. */
const claims = new LuaTable<number, LuaTable<number, number>>();

function W(): NativeWorld {
    if (!world) return error("this state has no game world", 3);
    return world;
}

function unitsOf(game: number): LuaTable<number, UnitAi> {
    let t = units.get(game);
    if (!t) {
        t = new LuaTable();
        units.set(game, t);
    }
    return t;
}

function stateOf(game: number, unit: NativeHandle): UnitAi {
    const [id] = W().handleId(unit);
    const t = unitsOf(game);
    let st = t.get(id!);
    if (!st) {
        const think: ThinkState = { distance: 0, engaged: false, view: undefined as unknown as Think };
        const getters = new LuaTable<string, (this: void) => unknown>();
        getters.set("target", () => (think.target !== undefined && native.handleIsLive(think.target) ? think.target : undefined));
        getters.set("distance", () => think.distance);
        getters.set("engaged", () => think.engaged);
        think.view = readonly<Think>({}, "think", getters);
        st = { unit, broken: false, think };
        t.set(id!, st);
    }
    return st;
}

// --- the registry facade ---------------------------------------------------------------------------

function aiIndexArg(v: unknown, where: string): number {
    const name = argString(v, 1, where);
    const index = W().aiIndex(name);
    if (index < 0) error(`${where}: no monster AI ${show(name)} (a MonsterAiId)`, 3);
    return index;
}

function claim(g: GameState, index: number, kind: number, targeting: number): void {
    let t = claims.get(g.id);
    if (!t) {
        t = new LuaTable();
        claims.set(g.id, t);
    }
    const was = t.get(index) ?? UNCLAIMED;
    if (kind < was) return;
    t.set(index, kind);
    W().aiClaim(index, kind, targeting);
}

function stockThink(this: void, monster: NativeHandle): void {
    W().aiStock(monster);
}

function ensureEntry(g: GameState, id: string): void {
    registries.ensure(g, REGISTRY, id, stockThink);
}

/** The game's AI registry as one mod sees it. */
export function facade(g: GameState, mod: number, live: (where: string) => void): object {
    return readonly({
        register: (id: unknown, script: unknown, targeting?: unknown) => {
            live("registry.ai.register");
            const index = aiIndexArg(id, "registry.ai.register");
            const fn = argFunction<AiScript>(script, 2, "registry.ai.register");
            let t = 0;
            if (targeting !== undefined) {
                t = argInteger(targeting, 3, "registry.ai.register");
                if (t < 0 || t > 2) error("registry.ai.register: targeting must be AiTargeting.Default, Basic or Advanced", 2);
            }
            const driver: Driver = { script: fn, mod };
            registries.register(g, mod, REGISTRY, id as string, (monster: NativeHandle, think: Think) => drive(g, driver, monster, think));
            claim(g, index, REGISTERED, t);
        },
        wrap: (id: unknown, wrapper: unknown) => {
            live("registry.ai.wrap");
            const index = aiIndexArg(id, "registry.ai.wrap");
            const fn = argFunction<AiWrapper>(wrapper, 2, "registry.ai.wrap");
            ensureEntry(g, id as string);
            registries.wrap(g, mod, REGISTRY, id as string, (next: Fn, monster: Monster, think: Think) => {
                fn(() => {
                    next(monster, think);
                }, monster, think);
            });
            claim(g, index, WRAPPED, 0);
        },
        has: (id: unknown) => {
            const index = aiIndexArg(id, "registry.ai.has");
            return (claims.get(g.id)?.get(index) ?? UNCLAIMED) !== UNCLAIMED;
        },
    }, "registry.ai");
}

// --- a think -----------------------------------------------------------------------------------------

function distance(a: Subtile, x: number, y: number): number {
    return math.sqrt((a.x - x) * (a.x - x) + (a.y - y) * (a.y - y));
}

function outcomeOf(st: UnitAi): Outcome {
    const p = st.pending!;
    if (p.kind === "idle") return Outcome.Done;
    if (p.goal !== undefined && !native.handleIsLive(p.goal)) return Outcome.Interrupted;
    const [, , aiState] = W().aiUnit(st.unit);
    if (aiState === GET_HIT || aiState === KNOCKBACK) return Outcome.Interrupted;
    if (p.kind === "move") {
        const [mx, my] = W().unitPosition(st.unit);
        if (mx === undefined) return Outcome.Interrupted;
        const here = points.subtile(mx, my);
        if (p.goal !== undefined) {
            const [gx, gy] = W().unitPosition(p.goal);
            return gx !== undefined && distance(here, gx, gy) <= 4 ? Outcome.Done : Outcome.Blocked;
        }
        return distance(here, p.x!, p.y!) <= 2 ? Outcome.Done : Outcome.Blocked;
    }
    return Outcome.Done;
}

/** One think of a registered script (the bottom of the entry). */
function drive(g: GameState, driver: Driver, monster: NativeHandle, think: Think): void {
    void think;
    const st = stateOf(g.id, monster);
    if (st.driver !== driver || (st.co === undefined && !st.broken)) {
        st.driver = driver;
        st.broken = false;
        st.pending = undefined;
        const view = st.think.view;
        st.co = native.spawn(driver.mod, () => driver.script(monster as unknown as Monster, view));
        byThread.set(st.co, st);
    }
    if (st.broken) {
        W().aiIdle(monster, BROKEN_IDLE);
        return;
    }
    const outcome = st.pending !== undefined ? outcomeOf(st) : undefined;
    st.pending = undefined;
    if (st.co === undefined) return;
    const [status, a, b] = native.resume(st.co, driver.mod, outcome);
    if (status === CallStatus.Yield) {
        if (a === AI_ACTION) return;
        if (a === undefined) {
            W().aiIdle(monster, 1);
            return;
        }
        native.logLine(`[lua] ${mods.name(driver.mod)}: a monster AI yielded a ${type(a)}; yield bare or yield* an action`);
        st.broken = true;
        W().aiIdle(monster, BROKEN_IDLE);
        return;
    }
    st.co = undefined;
    if (status === CallStatus.Error) {
        native.logError(driver.mod, a, b as string | undefined);
        st.broken = true;
        W().aiIdle(monster, BROKEN_IDLE);
    } else if (status === CallStatus.Fatal) {
        error(a, 0);
    }
    // Ok: the script returned and starts over at the next think (the host arms it).
}

/** A think from the engine: run the entry of the monster's AI in this game. True when handled. */
export function think(g: GameState, monster: NativeHandle, aiName: string, restart: boolean, target: NativeHandle | undefined,
    dist: number, engaged: boolean): boolean {
    const [id] = W().handleId(monster);
    if (restart) unitsOf(g.id).delete(id!);
    const st = stateOf(g.id, monster);
    st.think.target = target;
    st.think.distance = dist;
    st.think.engaged = engaged;
    ensureEntry(g, aiName);
    const [ok, err] = pcall(registries.call, g, REGISTRY, aiName, monster, st.think.view);
    if (!ok) native.logLine(`[lua] monster AI ${aiName}: ${type(err) === "string" ? err : type(err)}`);
    return true;
}

/** The monster died or is gone: its script ends (a revived monster starts it over). */
export function drop(game: number, unitId: number): void {
    const t = units.get(game);
    if (t) t.delete(unitId);
}

export function endGame(game: number): void {
    units.delete(game);
    claims.delete(game);
}

/** The mod of the AI script the coroutine is (or runs a guarded body of), when it is one. */
export function modOf(co: LuaThread): number | undefined {
    return byThread.get(co)?.driver?.mod;
}

/** A coroutine running part of a monster's script (a guard's body) acts as that script. */
export function adopt(outer: LuaThread, inner: LuaThread): void {
    const st = byThread.get(outer);
    if (st) byThread.set(inner, st);
}

// --- the actions -------------------------------------------------------------------------------------

/** Start an action of the monster whose script is running and wait for the engine to finish it. */
function act(self: NativeHandle, where: string, kind: string, start: () => boolean, goal?: NativeHandle, x?: number, y?: number): Outcome {
    const co = coroutine.running();
    const st = co === undefined ? undefined : byThread.get(co);
    if (st === undefined || st.unit !== self) error(`${where} can only be used inside this monster's own AI script`, 3);
    if (!start()) return Outcome.Blocked;
    st!.pending = { kind, goal, x, y };
    const [outcome] = coroutine.yield(AI_ACTION);
    return outcome as Outcome;
}

function unitArg(v: unknown, i: number, where: string): NativeHandle {
    if (!native.isHandle(v)) error(`bad argument #${i} to '${where}' (a unit expected, got ${type(v)})`, 3);
    if (!native.handleIsLive(v)) error(`bad argument #${i} to '${where}' (the unit is not valid)`, 3);
    return v as NativeHandle;
}

/** A unit or a corpse: what a skill can be cast at besides a point. */
function isTarget(v: unknown): boolean {
    if (!native.isHandle(v)) return false;
    const [, cls] = W().handleId(v);
    return cls === "Player" || cls === "Monster" || cls === "Unit" || cls === "Corpse";
}

/**
 * Cast at a unit, a corpse or a point. A unit target is the action's goal (gone or dead by the end:
 * Interrupted); a corpse is not, since a resurrect makes it a living monster, which ends the corpse.
 */
function cast(self: NativeHandle, where: string, skill: unknown, at: unknown, mode: number, sequence: boolean): Outcome {
    const s = argInteger(skill, 1, where);
    if (isTarget(at)) {
        if (!native.handleIsLive(at)) error(`bad argument #2 to '${where}' (the target is not valid)`, 3);
        const t = at as NativeHandle;
        const [, cls] = W().handleId(t);
        return act(self, where, "cast", () => W().aiCast(self, s, t, 0, 0, mode, sequence), cls === "Corpse" ? undefined : t);
    }
    const p = points.subtileArg(at, 2, where);
    return act(self, where, "cast", () => W().aiCast(self, s, undefined, p.x, p.y, mode, sequence));
}

/** The members a Monster handle gets for its actions. */
export function actions(): Record<string, Fn> {
    return {
        idle: (self: NativeHandle, frames?: unknown) => {
            const n = frames === undefined ? W().aiUnit(self)[3] : argInteger(frames, 1, "monster.idle");
            return act(self, "monster.idle", "idle", () => {
                W().aiIdle(self, n < 1 ? 1 : n);
                return true;
            });
        },
        walkTo: (self: NativeHandle, to: unknown) => {
            const p = points.subtileArg(to, 1, "monster.walkTo");
            return act(self, "monster.walkTo", "move", () => W().aiMove(self, undefined, p.x, p.y, false), undefined, p.x, p.y);
        },
        runTo: (self: NativeHandle, to: unknown) => {
            const p = points.subtileArg(to, 1, "monster.runTo");
            return act(self, "monster.runTo", "move", () => W().aiMove(self, undefined, p.x, p.y, true), undefined, p.x, p.y);
        },
        walkToward: (self: NativeHandle, target: unknown, flags?: unknown) => {
            const t = unitArg(target, 1, "monster.walkToward");
            const f = flags === undefined ? 0 : argInteger(flags, 2, "monster.walkToward");
            return act(self, "monster.walkToward", "move", () => W().aiMove(self, t, 0, 0, false, f), t);
        },
        runToward: (self: NativeHandle, target: unknown) => {
            const t = unitArg(target, 1, "monster.runToward");
            return act(self, "monster.runToward", "move", () => W().aiMove(self, t, 0, 0, true), t);
        },
        attack: (self: NativeHandle, target: unknown, secondary?: unknown) => {
            const t = unitArg(target, 1, "monster.attack");
            return act(self, "monster.attack", "attack", () => W().aiAttack(self, t, secondary === true), t);
        },
        castSkill: (self: NativeHandle, skill: unknown, at: unknown, mode?: unknown) => {
            const m = mode === undefined ? MonsterMode.Cast : argInteger(mode, 3, "monster.castSkill");
            return cast(self, "monster.castSkill", skill, at, m, false);
        },
        castSequence: (self: NativeHandle, skill: unknown, at: unknown) => cast(self, "monster.castSequence", skill, at, MonsterMode.Sequence, true),
        circleToward: (self: NativeHandle, target: unknown, steps: unknown, flank?: unknown) => {
            const t = unitArg(target, 1, "monster.circleToward");
            const n = argInteger(steps, 2, "monster.circleToward");
            return act(self, "monster.circleToward", "circle", () => W().aiCircle(self, t, n, flank === true), t);
        },
        walkPast: (self: NativeHandle, target: unknown, offset: unknown, force?: unknown) => {
            const t = unitArg(target, 1, "monster.walkPast");
            const n = argInteger(offset, 2, "monster.walkPast");
            return act(self, "monster.walkPast", "walk", () => W().aiWalkPast(self, t, n, force === true), t);
        },
        wanderAround: (self: NativeHandle, target: unknown, radius: unknown) => {
            const t = unitArg(target, 1, "monster.wanderAround");
            const r = argInteger(radius, 2, "monster.wanderAround");
            return act(self, "monster.wanderAround", "wander", () => W().aiWanderAround(self, t, r));
        },
        walkToDistance: (self: NativeHandle, target: unknown, maxStep: unknown, dist: unknown) => {
            const t = unitArg(target, 1, "monster.walkToDistance");
            const m = argInteger(maxStep, 2, "monster.walkToDistance");
            const d = argInteger(dist, 3, "monster.walkToDistance");
            return act(self, "monster.walkToDistance", "walk", () => W().aiWalkToDistance(self, t, m, d), t);
        },
        wander: (self: NativeHandle, radius: unknown) => {
            const r = argInteger(radius, 1, "monster.wander");
            return act(self, "monster.wander", "wander", () => W().aiWander(self, r));
        },
        flee: (self: NativeHandle, from: unknown, dist: unknown) => {
            const f = unitArg(from, 1, "monster.flee");
            const d = argInteger(dist, 2, "monster.flee");
            return act(self, "monster.flee", "flee", () => W().aiFlee(self, f, d));
        },
    };
}
