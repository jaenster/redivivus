// Typed mod settings over the native settings (the file under the mods root, checked against each
// declaration natively). A setting is declared once per state, from the module that defines the
// set; reading one goes to the native layer every time, so a value is whatever the host settled.
import { native } from "./native";

const specs = new LuaTable<object, object>();
setmetatable(specs, { __mode: "k" });

function optString(o: object, field: string, where: string): string | undefined {
    const v = rawget(o, field);
    if (v !== undefined && type(v) !== "string") error(`${where}: '${field}' must be a string`, 4);
    return v as string | undefined;
}

function optNumber(o: object, field: string, where: string): number | undefined {
    const v = rawget(o, field);
    if (v !== undefined && type(v) !== "number") error(`${where}: '${field}' must be a number`, 4);
    return v as number | undefined;
}

/** A declaration from a builder's options (read raw), recorded as made by the core. */
export function build(kind: string, o: unknown, where: string): object {
    if (type(o) !== "table") error(`${where}: a table of options`, 3);
    const opts = o as object;
    const decl = new LuaTable<string, unknown>();
    decl.set("type", kind);
    decl.set("scope", rawget(opts, "scope"));
    decl.set("default", rawget(opts, "default"));
    decl.set("min", optNumber(opts, "min", where));
    decl.set("max", optNumber(opts, "max", where));
    decl.set("title", optString(opts, "title", where));
    decl.set("description", optString(opts, "description", where));
    const choices = rawget(opts, "choices");
    if (choices !== undefined) {
        if (type(choices) !== "table") error(`${where}: 'choices' must be a list of strings`, 3);
        const list: unknown[] = [];
        const n = (choices as unknown[]).length;
        for (let i = 1; i <= n; i++) list.push(rawget(choices as object, i));
        decl.set("choices", list);
    }
    const spec = { declaration: decl };
    specs.set(spec, decl);
    return spec;
}

/** Declare every setting of the set (sorted by name) and give the read-only view of their values. */
export function define(mod: number, set: unknown): object {
    if (type(set) !== "table") error("defineSettings: a table of settings made with setting.*", 3);
    const names: string[] = [];
    let [k] = next(set as object);
    while (k !== undefined) {
        if (type(k) !== "string") error("defineSettings: settings are named by strings", 3);
        names.push(k as string);
        [k] = next(set as object, k);
    }
    names.sort();
    const declared = new LuaTable<string, true>();
    for (const name of names) {
        const decl = specs.get(rawget(set as object, name));
        if (!decl) error(`defineSettings: '${name}' is not a setting made with setting.*`, 3);
        native.settingsDeclare(mod, name, decl!);
        declared.set(name, true);
    }
    const mt = new LuaTable<string, unknown>();
    mt.set("__index", (_: unknown, key: unknown) => {
        if (type(key) !== "string" || !declared.has(key as string)) error(`no setting '${type(key) === "string" ? key : type(key)}' in this set`, 2);
        return native.settingsGet(mod, key as string);
    });
    mt.set("__newindex", (_: unknown, key: unknown) => {
        error(`settings are read-only (setting '${type(key) === "string" ? key : type(key)}')`, 2);
    });
    mt.set("__metatable", false);
    return setmetatable({}, mt as unknown as LuaMetatable<object>);
}
