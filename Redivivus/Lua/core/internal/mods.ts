// The mods of the state, in load order (redivivus first), by the host's id.
import type { NativeMod } from "../native/core";
import { native } from "./native";

const list = native.mods();
const byId = new LuaTable<number, NativeMod>();
for (const m of list) byId.set(m.id, m);

/** Every mod in load order. */
export function all(): readonly NativeMod[] {
    return list;
}

/** A mod's position in load order (-1 for none). */
export function order(mod: number): number {
    const m = byId.get(mod);
    return m ? m.index : -1;
}

export function name(mod: number): string {
    const m = byId.get(mod);
    return m ? m.name : "(host)";
}
