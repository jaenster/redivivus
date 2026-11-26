// Scripts: coroutines the core resumes, each resume an entry of the script's mod with a budget of
// its own. A script belongs to a game and a mod, and
// optionally to an owner handle: once the owner's object is gone the script is dropped. Every tick,
// after the tick handlers, the scripts of that game that are due resume in the order they started.
//
// What a script yields: nothing (resume next tick), a wait the core made (wait, waitEvent: private
// markers no mod can forge), or a table { ticks = n } / { event = name } from plain Lua, read raw.
import { CallStatus } from "../native/core";
import * as events from "./events";
import type { GameState, ScriptRecord } from "./games";
import * as mods from "./mods";
import { native } from "./native";

const WAIT_TICKS = {};
const WAIT_EVENT = {};

// Running coroutine -> its record: the script's own, and those a guard runs its body on (adopt).
// Weak both ways: the record holds its coroutine, so with weak keys alone (no ephemerons in LuaJIT)
// an entry would keep itself alive. A live record is held by its game's list.
const records = new LuaTable<LuaThread, ScriptRecord>();
setmetatable(records, { __mode: "kv" });
const gameOf = new LuaTable<ScriptRecord, GameState>();
setmetatable(gameOf, { __mode: "k" });
let seq = 0;

export function start(g: GameState, mod: number, body: (this: void) => unknown, holder?: unknown): ScriptRecord {
    const co = native.spawn(mod, body);
    seq++;
    const rec: ScriptRecord = { co, mod, due: native.tick() + 1, id: seq, holder, done: false };
    records.set(co, rec);
    gameOf.set(rec, g);
    g.scripts.push(rec);
    return rec;
}

/** The script the running coroutine is, when it is one of this state's. */
export function current(): ScriptRecord | undefined {
    const co = coroutine.running();
    if (co === undefined) return undefined;
    return records.get(co);
}

/** The mod of the script the coroutine is (or runs a guarded body of), when it is one. */
export function modOf(co: LuaThread): number | undefined {
    return records.get(co)?.mod;
}

/**
 * A coroutine started inside a script to run part of it (a guard's body) counts as that script:
 * its waits are the script's. Nothing holds it but the guard running it.
 */
export function adopt(outer: LuaThread, inner: LuaThread): void {
    const rec = records.get(outer);
    if (rec) records.set(inner, rec);
}

export function gameOfCurrent(): GameState | undefined {
    const rec = current();
    return rec ? gameOf.get(rec) : undefined;
}

/** Wait ticks inside a script; the outcome is the tick it resumes on. */
export function wait(ticks: number, where: string): number {
    if (!current()) error(`${where} can only be used inside a script (started with startScript)`, 3);
    const [tick] = coroutine.yield(WAIT_TICKS, ticks);
    return tick as number;
}

/** Wait for an event of the script's game; the outcome is the payload. */
export function waitEvent(g: GameState, name: string, where: string): object {
    const rec = current();
    if (!rec) error(`${where} can only be used inside a script (started with startScript)`, 3);
    if (gameOf.get(rec!) !== g) error(`${where}: this script belongs to another game`, 3);
    const [payload] = coroutine.yield(WAIT_EVENT, name);
    return payload as object;
}

function stop(rec: ScriptRecord, why: string): void {
    native.logLine(`[lua] ${mods.name(rec.mod)}: a script: ${why}; the script is stopped`);
    rec.done = true;
}

function resume(rec: ScriptRecord, arg: unknown): void {
    const now = native.tick();
    const [status, a, b] = native.resume(rec.co, rec.mod, arg);
    rec.event = undefined;
    if (status === CallStatus.Yield) {
        rec.due = now + 1;
        if (a === WAIT_TICKS) {
            const n = b as number;
            rec.due = now + (n < 1 ? 1 : n);
        } else if (a === WAIT_EVENT) {
            rec.event = b as string;
        } else if (a === undefined) {
            // next tick
        } else if (type(a) === "table") {
            const ticks = rawget(a, "ticks");
            const ev = rawget(a, "event");
            if (type(ticks) === "number") {
                rec.due = now + ((ticks as number) < 1 ? 1 : math.floor(ticks as number));
            } else if (type(ev) === "string" && events.isKnown(ev)) {
                rec.event = ev as string;
            } else if (type(ev) === "string") {
                stop(rec, `it waits for an unknown event '${ev as string}'`);
            } else {
                stop(rec, "it yielded a table that is not a wait ({ ticks = n } or { event = name })");
            }
        } else {
            stop(rec, `it yielded a ${type(a)}, which is not a wait`);
        }
        return;
    }
    rec.done = true;
    if (status === CallStatus.Error) {
        native.logError(rec.mod, a, b as string | undefined);
    } else if (status === CallStatus.Fatal) {
        error(a, 0);
    }
}

function compact(g: GameState): void {
    const live: ScriptRecord[] = [];
    for (const s of g.scripts) {
        if (!s.done) live.push(s);
        // A task handle a mod keeps holds the record: let go of the coroutine (and so of the
        // guarded bodies on its stack) once the script is over.
        else s.co = undefined as unknown as LuaThread;
    }
    g.scripts = live;
}

/** Resume the game's scripts that are due. Scripts started during the pass wait for the next one. */
export function tick(g: GameState): void {
    const now = native.tick();
    const list = g.scripts;
    const n = list.length;
    for (let i = 0; i < n; i++) {
        const rec = list[i];
        if (g.ended) return;
        if (rec.done) continue;
        if (rec.holder !== undefined && !native.handleIsLive(rec.holder)) {
            rec.done = true;
            continue;
        }
        if (rec.event === undefined && rec.due <= now) resume(rec, now);
    }
    compact(g);
}

/** Resume the game's scripts waiting for the event, with its payload. */
export function wake(g: GameState, name: string, payload: object): void {
    const list = g.scripts;
    const n = list.length;
    for (let i = 0; i < n; i++) {
        const rec = list[i];
        if (g.ended) return;
        if (rec.done || rec.event !== name) continue;
        if (rec.holder !== undefined && !native.handleIsLive(rec.holder)) {
            rec.done = true;
            continue;
        }
        resume(rec, payload);
    }
    compact(g);
}
