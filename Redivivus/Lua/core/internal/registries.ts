// Behaviour registries of a game. An entry is a chain: an implementation at the bottom (the
// native one the declarations give, or what a mod registered last) and wrappers above it, ordered
// by the wrapping mod's load order (then by the order one mod added them), so later mods wrap
// earlier ones whatever the moment they wrap. A call runs the top wrapper with (next, ...args);
// next runs the level below. Every level runs as its own mod through the native layer.
//
// A wrapper that raises an error of its own is removed from the chain, logged, and the call goes on
// with the level below and the original arguments. An error that comes up out of next (a level
// below failed) passes through the wrappers above unchanged and is held against none of them: the
// core remembers the value next raised and compares the wrapper's error with it. When the budget
// or the memory limit runs out, nothing falls through: the error goes up to the entry's start and
// the native layer faults the mod it was charged to.
import { CallStatus } from "../native/core";
import { show } from "./check";
import type { Entry, GameState, Wrapper } from "./games";
import * as mods from "./mods";
import { native } from "./native";

type AnyFn = (this: void, ...args: any[]) => any;

const defs = native.registries();
let wrapperSeq = 0;

export const KEYED = 0;
export const SLOT = 1;

/** The registries of this side by name, with their kind. */
export function names(): LuaTable<string, number> {
    const out = new LuaTable<string, number>();
    for (const [name, def] of defs) out.set(name, def.kind);
    return out;
}

export function exists(registry: string): boolean {
    return defs.get(registry) !== undefined;
}

function entryOf(g: GameState, registry: string, id: string, create: boolean): Entry | undefined {
    let table = g.entries.get(registry);
    if (!table) {
        table = new LuaTable();
        g.entries.set(registry, table);
    }
    let entry = table.get(id);
    if (entry) return entry;
    const impl = defs.get(registry)?.natives.get(id);
    if (!impl && !create) return undefined;
    entry = { impl, owner: -1, wrappers: [] };
    table.set(id, entry);
    return entry;
}

/**
 * Make the entry with impl at its bottom, run without changing the owner, when it does not exist
 * yet (a registry whose entries the core provides, such as the monster AIs' stock thinks).
 */
export function ensure(g: GameState, registry: string, id: string, impl: AnyFn): void {
    const entry = entryOf(g, registry, id, true)!;
    if (entry.impl === undefined) entry.impl = impl;
}

export function register(g: GameState, mod: number, registry: string, id: string, fn: AnyFn): void {
    const entry = entryOf(g, registry, id, true)!;
    entry.impl = fn;
    entry.owner = mod;
}

/** False when the entry does not exist. */
export function wrap(g: GameState, mod: number, registry: string, id: string, fn: AnyFn): boolean {
    const entry = entryOf(g, registry, id, false);
    if (!entry) return false;
    const order = mods.order(mod);
    wrapperSeq++;
    const w: Wrapper = { fn, mod, order, seq: wrapperSeq, removed: false };
    const list = entry.wrappers;
    let at = list.length;
    for (let i = 0; i < list.length; i++) {
        if (list[i].order > order) {
            at = i;
            break;
        }
    }
    list.splice(at, 0, w);
    return true;
}

export function has(g: GameState, registry: string, id: string): boolean {
    const entry = entryOf(g, registry, id, false);
    return entry !== undefined && entry.impl !== undefined;
}

function label(registry: string, id: string): string {
    return id === "" ? registry : `${registry} ${string.format("%q", id)}`;
}

function errorText(e: unknown): string {
    return type(e) === "string" ? (e as string) : `(error object is a ${type(e)} value)`;
}

function runLevel(entry: Entry, snapshot: Wrapper[], level: number, where: string, ...args: unknown[]): unknown {
    if (level === 0) {
        const impl = entry.impl;
        if (impl === undefined) error(`${where} has no implementation`, 0);
        const [status, result] = native.call(entry.owner, impl, ...args);
        if (status === CallStatus.Ok) return result;
        if (status === CallStatus.Disabled) error(`${where}: its implementation belongs to ${mods.name(entry.owner)}, which is disabled`, 0);
        return error(result, 0);
    }
    const w = snapshot[level - 1];
    if (w.removed) return runLevel(entry, snapshot, level - 1, where, ...args);
    let below: unknown = undefined;
    let failed = false;
    const next = (...a: unknown[]): unknown => {
        const [ok, value] = pcall(runLevel, entry, snapshot, level - 1, where, ...a);
        if (!ok) {
            below = value;
            failed = true;
            error(value, 0);
        }
        return value;
    };
    const [status, result] = native.call(w.mod, w.fn, next, ...args);
    if (status === CallStatus.Ok) return result;
    if (status === CallStatus.Disabled) return runLevel(entry, snapshot, level - 1, where, ...args);
    if (status !== CallStatus.Error || (failed && rawequal(result, below))) return error(result, 0);
    native.logLine(`[lua] ${mods.name(w.mod)}: a wrapper on ${where} failed and is removed from the chain: ${errorText(result)}`);
    w.removed = true;
    const list = entry.wrappers;
    for (let i = 0; i < list.length; i++) {
        if (list[i] === w) {
            list.splice(i, 1);
            break;
        }
    }
    return runLevel(entry, snapshot, level - 1, where, ...args);
}

/** Run the entry through its chain; raises the error of the bottom or a passing one. */
export function call(g: GameState, registry: string, id: string, ...args: unknown[]): unknown {
    const entry = entryOf(g, registry, id, false);
    if (!entry) return error(`registry ${registry} has no entry ${show(id)}`, 0);
    const snapshot: Wrapper[] = [];
    for (const w of entry.wrappers) snapshot.push(w);
    return runLevel(entry, snapshot, snapshot.length, label(registry, id), ...args);
}
