// Enums as plain Lua sees them: a read-only table per enum per mod (so a rawset into a mod's own
// table changes nothing for anyone else), built from the values the native layer holds. TypeScript
// inlines the same values.
import { native } from "./native";
import { readonly } from "./proxy";

const values = native.enums();
const perMod = new LuaTable<number, LuaTable<string, object>>();

export function names(): string[] {
    const out: string[] = [];
    for (const [name] of values) out.push(name);
    out.sort();
    return out;
}

export function table(mod: number, name: string): object | undefined {
    let cache = perMod.get(mod);
    if (!cache) {
        cache = new LuaTable();
        perMod.set(mod, cache);
    }
    let t = cache.get(name);
    if (t) return t;
    const v = values.get(name);
    if (!v) return undefined;
    t = readonly(v, name, undefined, "read-only (an enum)");
    cache.set(name, t);
    return t;
}
