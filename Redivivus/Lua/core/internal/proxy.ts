// Read-only views. Everything the core hands a mod that the mod must not change for others (a
// module's exports, the script and registry objects, enums) is a proxy: an empty table whose
// metatable reads from the values and refuses writes, locked with __metatable so getmetatable
// gives false and setmetatable fails. pairs in a mod environment walks the values (the native
// pairs looks for them under the walk key).
import { native } from "./native";

const walkKey = native.walkKey();

function keyName(key: unknown): string {
    return type(key) === "string" ? (key as string) : type(key);
}

/**
 * A read-only view of values. With getters, reading one of their names calls it, and reading a
 * name that is neither a value nor a getter raises (an object with a fixed set of members);
 * without, a missing name reads as nil (a module, an enum).
 */
export function readonly<T extends object>(values: object, name: string, getters?: LuaTable<string, (this: void) => unknown>,
    what = "read-only"): T {
    const mt = new LuaTable<AnyNotNil, unknown>();
    if (getters) {
        mt.set("__index", (_: unknown, key: unknown) => {
            const getter = rawget(getters, key);
            if (getter !== undefined) return (getter as (this: void) => unknown)();
            const v = rawget(values, key);
            if (v === undefined) error(`${name} has no member '${keyName(key)}'`, 2);
            return v;
        });
    } else {
        mt.set("__index", values);
    }
    mt.set("__newindex", (_: unknown, key: unknown) => {
        error(`${name}.${keyName(key)} is ${what}`, 2);
    });
    mt.set("__metatable", false);
    rawset(mt, walkKey, values);
    return setmetatable({}, mt as unknown as LuaMetatable<object>) as T;
}
