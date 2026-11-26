// State that belongs to an engine object, per game: kept in the mod's storage for that game (so it
// is saved with it) under the owner handle, and removed once the owner's object is gone.
//
// storage.__persist holds { owner = handle, state = table } entries. A script of the mod, started
// on first use in a game, drops the entries whose owner is gone, once a tick. Everything here reads
// the mod's tables raw: storage is the mod's, and a metatable it set must not run in here.
import type { GameState } from "./games";
import { native } from "./native";
import * as scheduler from "./scheduler";
import * as storage from "./storage";

const KEY = "__persist";

type Index = { list: object; byOwner: LuaTable<AnyNotNil, object>; sweeping: boolean };
// (game, mod) -> the index of the entry list it was built from.
const indexes = new LuaTable<GameState, LuaTable<number, Index>>();
setmetatable(indexes, { __mode: "k" });

function entries(g: GameState, mod: number): Index {
    const s = storage.of(g, mod);
    let list = rawget(s, KEY);
    if (type(list) !== "table") {
        list = {};
        rawset(s, KEY, list);
    }
    let perGame = indexes.get(g);
    if (!perGame) {
        perGame = new LuaTable();
        indexes.set(g, perGame);
    }
    let index = perGame.get(mod);
    if (!index || index.list !== list) {
        const byOwner = new LuaTable<AnyNotNil, object>();
        const n = (list as unknown[]).length;
        for (let i = 1; i <= n; i++) {
            const e = rawget(list, i);
            if (type(e) === "table") byOwner.set(rawget(e, "owner"), e);
        }
        const sweeping = index !== undefined;
        index = { list, byOwner, sweeping: true };
        perGame.set(mod, index);
        if (!sweeping) scheduler.start(g, mod, sweeper(g, mod));
    }
    return index;
}

export function persist(g: GameState, mod: number, owner: unknown, defaults: object): object {
    const index = entries(g, mod);
    const found = index.byOwner.get(owner as AnyNotNil);
    if (found) return rawget(found, "state");
    const entry = { owner, state: storage.copy(defaults, new LuaTable()) };
    const list = index.list as unknown[];
    rawset(list, list.length + 1, entry);
    index.byOwner.set(owner as AnyNotNil, entry);
    return entry.state as object;
}

export function hasPersisted(g: GameState, mod: number, owner: unknown): boolean {
    return entries(g, mod).byOwner.get(owner as AnyNotNil) !== undefined;
}

/** Drop the entries whose owner is gone; how many. */
export function sweep(g: GameState, mod: number): number {
    const s = g.storage.get(mod);
    if (!s) return 0;
    const list = rawget(s, KEY);
    if (type(list) !== "table") return 0;
    const index = entries(g, mod);
    let removed = 0;
    for (let i = (list as unknown[]).length; i >= 1; i--) {
        const e = rawget(list, i);
        const owner = type(e) === "table" ? rawget(e, "owner") : undefined;
        if (!native.handleIsLive(owner)) {
            index.byOwner.delete(owner as AnyNotNil);
            table.remove(list as unknown[], i);
            removed++;
        }
    }
    return removed;
}

function sweeper(g: GameState, mod: number): (this: void) => void {
    return () => {
        for (;;) {
            coroutine.yield();
            sweep(g, mod);
        }
    };
}

/** Forget the indexes of a game whose registrations were dropped (its sweepers stopped with them). */
export function forget(g: GameState): void {
    indexes.delete(g);
}
