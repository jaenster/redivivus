// storage: each mod's saved state, one table per mod per game. The native layer turns it into
// bytes and back (d2_lua_storage.cpp); restoring fills the existing table, so a reference a mod
// took to it stays good.
import type { GameState } from "./games";

export function of(g: GameState, mod: number): object {
    let t = g.storage.get(mod);
    if (!t) {
        t = {};
        g.storage.set(mod, t);
    }
    return t;
}

/** The table under key in the mod's storage, made from a copy of defaults the first time. */
export function slot(g: GameState, mod: number, key: string, defaults: object): object {
    const root = of(g, mod);
    const v = rawget(root, key);
    if (type(v) === "table") return v;
    const made = copy(defaults, new LuaTable());
    rawset(root, key, made);
    return made as object;
}

// A copy of plain data: tables are copied raw and once each (shared tables stay shared, cycles end).
export function copy(value: unknown, seen: LuaTable<object, object>): unknown {
    if (type(value) !== "table") return value;
    const done = seen.get(value as object);
    if (done) return done;
    const out = {};
    seen.set(value as object, out);
    let [k, v] = next(value as object);
    while (k !== undefined) {
        rawset(out, copy(k, seen), copy(v, seen));
        [k, v] = next(value as object, k);
    }
    return out;
}
