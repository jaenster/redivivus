// Step-level tests of the helpers in mephisto.ts, without an engine: each runs on a coroutine with a
// stand-in monster whose rolls and action outcomes are scripted, and the test compares the actions
// it starts, in order. An action that starts waits one think (the coroutine yields); one that is
// Blocked comes back at once, as the real ones do. The host test runs run() and checks it is empty.
import { Outcome, type Monster, type MonsterData, type TargetScan, type Think, type Unit } from "redivivus:server";
import type { Script } from "redivivus:shared";
import { castPick, chooseCast, closeIn, decide, melee, reposition, retreat, roam, rotation, rotationStep, type Memory } from "./mephisto";

type Helper = (this: void, ...args: any[]) => unknown;

interface Scene {
    /** The seed's rolls, in order (each must be under the modulo asked for). */
    rolls?: number[];
    /** The outcomes of the actions it starts, in order; Done when the list runs out. */
    outcomes?: Outcome[];
    life?: number;
    difficulty?: number;
    wall?: boolean;
    skills?: number;
    weakest?: string;
    /** Think values for each think after the first (the first uses engaged/distance). */
    thinks?: { engaged: boolean; distance: number }[];
    engaged?: boolean;
    distance?: number;
}

interface Case {
    name: string;
    helper: Helper;
    /** The arguments after the stand-ins (monster, and think when needed). */
    args: (f: Fake) => unknown[];
    scene: Scene;
    want: string[];
    /** The rotation's cadence it leaves (Memory.cadence), when that is part of the case. */
    cadence?: number;
}

interface Fake {
    monster: Monster;
    think: { target: Unit; engaged: boolean; distance: number };
    calls: string[];
    memory: Memory;
    game: { frame: number; difficulty: number };
    /** The scripted rolls nobody made. */
    unrolled: () => number;
}

function unit(name: string): Unit {
    return { name, valid: true } as unknown as Unit;
}

const TARGET = unit("target");
const WEAK = unit("weak");

function nameOf(u: unknown): string {
    return (u as { name: string }).name;
}

function makeFake(scene: Scene): Fake {
    const rolls = [...(scene.rolls ?? [])];
    const outcomes = [...(scene.outcomes ?? [])];
    const calls: string[] = [];
    const skills: number[] = [];
    const modes: number[] = [];
    for (let i = 0; i < 8; i++) {
        skills.push(i < (scene.skills ?? 6) ? 100 + i : -1);
        modes.push(10 + i);
    }
    const data = { aiParams: [60, 35, 50, 0, 0, 0, 0, 0], skills, skillModes: modes } as unknown as MonsterData;
    const game = { frame: 1, difficulty: scene.difficulty ?? 1 };
    const f: Fake = { monster: undefined as unknown as Monster, think: { target: TARGET, engaged: scene.engaged ?? false, distance: scene.distance ?? 10 }, calls, memory: { cadence: 0 }, game, unrolled: () => rolls.length };
    const act = (text: string): Outcome => {
        calls.push(text);
        const o = outcomes.length > 0 ? outcomes.shift()! : Outcome.Done;
        if (o !== Outcome.Blocked) coroutine.yield();
        return o;
    };
    f.monster = {
        data,
        game,
        lifePercent: scene.life ?? 100,
        random(this: void, _self: unknown, max: number): number {
            const r = rolls.shift();
            if (r === undefined) error(`a roll of ${max} nobody scripted (after ${calls.join(", ")})`);
            if (r >= max) error(`a scripted roll ${r} is not under ${max}`);
            return r;
        },
        scanTargets: (): TargetScan => ({ weakest: scene.weakest === undefined ? undefined : WEAK, nearest: undefined, nearestDistance: 0,
            weakestLife: 0, nearestWhenPlayer: undefined, nearestWhenPlayerDistance: 0, count: 1, closeCount: 1 }) as unknown as TargetScan,
        collidesWith: () => scene.wall === true,
        setPathOverrides: (_self: unknown, a: number, b: number, c: number) => {
            calls.push(`path ${a} ${b} ${c}`);
        },
        idle: (_self: unknown, n: number) => act(`idle ${n}`),
        attack: (_self: unknown, t: Unit) => act(`attack ${nameOf(t)}`),
        castSkill: (_self: unknown, s: number, t: Unit, m: number) => act(`cast ${s} ${nameOf(t)} ${m}`),
        walkToward: (_self: unknown, t: Unit, fl: number) => act(`walkToward ${nameOf(t)} ${fl}`),
        circleToward: (_self: unknown, t: Unit, n: number, flank?: boolean) => act(`circle ${nameOf(t)} ${n}${flank ? " flank" : ""}`),
        walkPast: (_self: unknown, t: Unit, n: number, force?: boolean) => act(`walkPast ${nameOf(t)} ${n}${force ? " force" : ""}`),
        wanderAround: (_self: unknown, t: Unit, n: number) => act(`wanderAround ${nameOf(t)} ${n}`),
        walkToDistance: (_self: unknown, t: Unit, a: number, b: number) => act(`walkToDistance ${nameOf(t)} ${a} ${b}`),
        wander: (_self: unknown, n: number) => act(`wander ${n}`),
    } as unknown as Monster;
    return f;
}

/** Run the helper to its end, a think at a time; the scene's thinks change what the think says. */
function drive(c: Case): Fake {
    const f = makeFake(c.scene);
    const co = coroutine.create(c.helper);
    const thinks = [...(c.scene.thinks ?? [])];
    let [ok, err] = coroutine.resume(co, f.monster, ...c.args(f));
    while (ok && coroutine.status(co) === "suspended") {
        f.game.frame++;
        const next = thinks.shift();
        if (next !== undefined) {
            f.think.engaged = next.engaged;
            f.think.distance = next.distance;
        }
        [ok, err] = coroutine.resume(co);
    }
    if (!ok) f.calls.push(`error: ${err}`);
    return f;
}

const think = (f: Fake): Think => f.think as unknown as Think;

/** chooseCast then castPick, as the rotation runs them. */
function* castOne(monster: Monster, target: Unit, distance: number): Script<void> {
    yield* castPick(monster, chooseCast(monster, target, distance));
}

const CASES: Case[] = [
    { name: "reposition: wander near the target", helper: reposition, args: () => [TARGET], scene: {},
        want: ["path 0 50 0", "wanderAround target 6"] },
    { name: "reposition: else walk to a distance", helper: reposition, args: () => [TARGET], scene: { outcomes: [Outcome.Blocked] },
        want: ["path 0 50 0", "wanderAround target 6", "walkToDistance target 12 6"] },
    { name: "reposition: else wander where it stands", helper: reposition, args: () => [TARGET], scene: { outcomes: [Outcome.Blocked, Outcome.Blocked] },
        want: ["path 0 50 0", "wanderAround target 6", "walkToDistance target 12 6", "wander 12"] },
    { name: "closeIn: curve in with the flank flag", helper: closeIn, args: () => [TARGET], scene: {}, want: ["circle target 3 flank"] },
    { name: "closeIn: else wander", helper: closeIn, args: () => [TARGET], scene: { outcomes: [Outcome.Blocked] }, want: ["circle target 3 flank", "wander 12"] },
    { name: "melee: hit (life full: 79 < 80, then 79 < 80)", helper: melee, args: () => [TARGET, 0], scene: { rolls: [79, 79] }, want: ["attack target"] },
    { name: "melee: cast Skill3 instead (80 is not under 80)", helper: melee, args: () => [TARGET, 0], scene: { rolls: [10, 80] }, want: ["cast 102 target 12"] },
    { name: "melee: hurt moves both bands (hurt 10: under 90, then under 70)", helper: melee, args: () => [TARGET, 10], scene: { rolls: [89, 70] },
        want: ["cast 102 target 12"] },
    { name: "melee: else close in", helper: melee, args: () => [TARGET, 0], scene: { rolls: [80] }, want: ["path 0 50 0", "circle target 3 flank"] },
    { name: "roam: stand (65 or more)", helper: roam, args: () => [TARGET, 3], scene: { rolls: [65] }, want: ["idle 10"] },
    { name: "roam: walk up when close", helper: roam, args: () => [TARGET, 5], scene: { rolls: [64, 65] }, want: ["path 0 0 4", "walkToward target 7"] },
    { name: "roam: curve in when not close", helper: roam, args: () => [TARGET, 6], scene: { rolls: [64, 65] }, want: ["circle target 4"] },
    { name: "roam: curve in on the second roll", helper: roam, args: () => [TARGET, 2], scene: { rolls: [0, 64] }, want: ["circle target 4"] },
    { name: "castOne: Skill6 at a target behind a wall (Nightmare)", helper: castOne, args: () => [TARGET, 10], scene: { rolls: [99], wall: true },
        want: ["cast 105 target 15"] },
    { name: "castOne: Skill6 at a target past 30", helper: castOne, args: () => [TARGET, 31], scene: { rolls: [99] }, want: ["cast 105 target 15"] },
    { name: "castOne: not in Normal", helper: castOne, args: () => [TARGET, 31], scene: { rolls: [99], difficulty: 0, wall: true }, want: ["cast 100 target 10"] },
    { name: "castOne: Skill5 near (share 16)", helper: castOne, args: () => [TARGET, 14], scene: { rolls: [15] }, want: ["cast 104 target 14"] },
    { name: "castOne: Skill4 below two shares", helper: castOne, args: () => [TARGET, 15], scene: { rolls: [15] }, want: ["cast 103 target 13"] },
    { name: "castOne: Skill2 below three shares", helper: castOne, args: () => [TARGET, 15], scene: { rolls: [32] }, want: ["cast 101 target 11"] },
    { name: "castOne: Skill1 otherwise", helper: castOne, args: () => [TARGET, 15], scene: { rolls: [48] }, want: ["cast 100 target 10"] },
    { name: "castOne: at the weakest, by chance (34 < 35)", helper: castOne, args: () => [TARGET, 15], scene: { rolls: [34, 48], weakest: "weak" },
        want: ["cast 100 weak 10"] },
    { name: "castOne: no skills: wander", helper: castOne, args: () => [TARGET, 15], scene: { skills: 0 }, want: ["wander 6"] },
    { name: "rotationStep: a new cadence approaches first", helper: rotationStep, args: (f) => [think(f), f.memory, 0], scene: {},
        want: ["circle target 3 flank"], cadence: 2 },
    { name: "rotationStep: then casts and rolls to start over (49 < 50)", helper: rotationStep, args: (f) => { f.memory.cadence = 2; return [think(f), f.memory, 0]; },
        scene: { rolls: [48, 49] }, want: ["cast 100 target 10"], cadence: 0 },
    { name: "rotationStep: or goes on (hurt 10: 40 is not under 40)", helper: rotationStep, args: (f) => { f.memory.cadence = 2; return [think(f), f.memory, 10]; },
        scene: { rolls: [48, 40] }, want: ["cast 100 target 10"], cadence: 3 },
    { name: "rotation: 3 thinks (roll 0), approach then two casts", helper: rotation, args: (f) => [think(f), f.memory],
        scene: { rolls: [0, 48, 99, 48, 99] }, want: ["circle target 3 flank", "cast 100 target 10", "cast 100 target 10"], cadence: 4 },
    { name: "rotation: 5 thinks (roll 2)", helper: rotation, args: (f) => [think(f), f.memory],
        scene: { rolls: [2, 48, 99, 48, 99, 48, 99, 48, 99] }, want: ["circle target 3 flank", "cast 100 target 10", "cast 100 target 10", "cast 100 target 10", "cast 100 target 10"] },
    { name: "rotation: the guard ends it when the target comes into melee range", helper: rotation, args: (f) => [think(f), f.memory],
        scene: { rolls: [2, 48, 99], thinks: [{ engaged: false, distance: 10 }, { engaged: true, distance: 1 }] },
        want: ["circle target 3 flank", "cast 100 target 10"] },
    { name: "rotation: and when it gets farther than 20", helper: rotation, args: (f) => [think(f), f.memory],
        scene: { rolls: [2], thinks: [{ engaged: false, distance: 21 }] }, want: ["circle target 3 flank"] },
    { name: "rotation: starts in the think it was chosen, engaged or not", helper: rotation, args: (f) => [think(f), f.memory],
        scene: { rolls: [0], engaged: true, distance: 1, thinks: [{ engaged: true, distance: 1 }] }, want: ["circle target 3 flank"] },
    { name: "retreat: walk on past", helper: retreat, args: (f) => [think(f), f.memory, 18], scene: {}, want: ["path 0 50 0", "walkPast target 8 force"] },
    { name: "retreat: else a rotation step", helper: retreat, args: (f) => [think(f), f.memory, 18], scene: { outcomes: [Outcome.Blocked] },
        want: ["path 0 50 0", "walkPast target 8 force", "circle target 3 flank"] },
    { name: "decide: engaged, the skill chance holds (60 is not over 60): a rotation", helper: decide, args: (f) => [think(f), f.memory],
        scene: { engaged: true, distance: 1, rolls: [60, 0], thinks: [{ engaged: true, distance: 1 }] }, want: ["circle target 3 flank"] },
    { name: "decide: engaged, over it: melee", helper: decide, args: (f) => [think(f), f.memory], scene: { engaged: true, distance: 1, rolls: [61, 0, 0] },
        want: ["attack target"] },
    { name: "decide: far: reposition, no roll", helper: decide, args: (f) => [think(f), f.memory], scene: { distance: 21 }, want: ["path 0 50 0", "wanderAround target 6"] },
    { name: "decide: hurt and close: retreat (39 < 40)", helper: decide, args: (f) => [think(f), f.memory], scene: { life: 20, distance: 4, rolls: [39] },
        want: ["path 0 50 0", "walkPast target 8 force"] },
    { name: "decide: hurt and close, the roll fails: a rotation by the hurt band (65 < 16+50)", helper: decide, args: (f) => [think(f), f.memory],
        scene: { life: 20, distance: 4, rolls: [40, 65, 0], thinks: [{ engaged: true, distance: 1 }] }, want: ["circle target 3 flank"] },
    { name: "decide: else roam", helper: decide, args: (f) => [think(f), f.memory], scene: { distance: 10, rolls: [50, 99] }, want: ["idle 10"] },
];

/** Run every case; the failures, one line each. */
export function run(): string[] {
    const failures: string[] = [];
    for (const c of CASES) {
        const f = drive(c);
        const got = f.calls.join(" | ");
        const want = c.want.join(" | ");
        if (got !== want) failures.push(`${c.name}: got [${got}], want [${want}]`);
        if (f.unrolled() > 0) failures.push(`${c.name}: ${f.unrolled()} scripted roll(s) not made`);
        if (c.cadence !== undefined && f.memory.cadence !== c.cadence) failures.push(`${c.name}: cadence ${f.memory.cadence}, want ${c.cadence}`);
    }
    return failures;
}

/** How many cases there are. */
export const count = CASES.length;
