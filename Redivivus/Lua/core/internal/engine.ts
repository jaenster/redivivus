// The engine object a mod's patch(engine) gets: the functions of the compiled game as
// engine.<module>.<namespace>.<function>, resolved by name in the override registry
// (d2_lua_override.cpp) the first time a mod reads one, and only those of this state's side. Each
// function's override(fn) adds fn for the mod; the registry does the rest.
//
// engine.unsafe is the raw tier, there only for a mod the player granted it: the same functions,
// but pointers arrive as LuaJIT FFI cdata (the mod reads and writes engine memory through ffi), and
// next() takes cdata back. Without the grant it is nil.
import { Side } from "../redivivus/enums";
import { native } from "./native";

const SIDE = native.side() as Side;

type Fn = (this: void, ...args: any[]) => any;

type FfiModule = {
    cast(this: void, ct: unknown, v: unknown): unknown;
    cdef(this: void, def: string): void;
    typeof(this: void, ct: string): unknown;
    istype(this: void, ct: unknown, v: unknown): boolean;
};

let ffiModule: FfiModule | undefined;
const ctypes = new LuaTable<string, unknown>();
let uintptr: unknown;

function ffi(): FfiModule {
    if (!ffiModule) {
        ffiModule = native.ovFfi() as FfiModule;
        uintptr = ffiModule.typeof("uintptr_t");
    }
    return ffiModule;
}

const PRIMITIVE = new LuaTable<string, boolean>();
for (const t of ["void", "char", "short", "int", "long", "float", "double", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t",
    "uint16_t", "uint32_t", "uint64_t", "size_t", "intptr_t", "uintptr_t", "bool"]) PRIMITIVE.set(t, true);

// The cdata type of a pointer to the named type: a declared record (incomplete unless a layout was
// given to ffi), a primitive, or bytes.
function pointerType(name: string): unknown {
    let ct = ctypes.get(name);
    if (ct !== undefined) return ct;
    const f = ffi();
    if (PRIMITIVE.get(name)) {
        ct = f.typeof(`${name} *`);
    } else {
        const [declared] = pcall(() => f.typeof(`struct ${name} *`));
        if (!declared) pcall(() => f.cdef(`struct ${name};`));
        const [ok, t] = pcall(() => f.typeof(`struct ${name} *`));
        ct = ok ? t : f.typeof("uint8_t *");
    }
    ctypes.set(name, ct);
    return ct;
}

// A raw layer: pointers in (lightuserdata or handles from the native side) become cdata, cdata the
// mod passes on or returns become addresses the native side reads as they are.
function rawLayer(kinds: string, pointees: string[], fn: Fn): Fn {
    const f = ffi();
    const nParams = kinds.length - 1;
    const toCdata = (k: string, name: string, v: unknown) => {
        if (v === undefined) return undefined;
        if (k === "n" || k === "b") return v;
        return f.cast(pointerType(name), v);
    };
    const fromCdata = (k: string, v: unknown): unknown => {
        if (k === "n" || k === "b" || v === undefined) return v;
        if ((type(v) as string) === "cdata") return tonumber(f.cast(uintptr, v));
        return v;
    };
    // pointee names in order of the pointer kinds
    const names: string[] = [];
    let pi = 0;
    for (let i = 0; i < kinds.length; i++) {
        const k = kinds.charAt(i);
        names[i] = k === "p" || k === "w" || k === "r" ? (pointees[pi++] ?? "uint8_t") : "";
    }
    return (next: Fn, ...args: unknown[]) => {
        const cargs: unknown[] = [];
        for (let i = 0; i < nParams; i++) cargs[i] = toCdata(kinds.charAt(i + 1), names[i + 1], args[i]);
        const rawNext = (...nargs: unknown[]) => {
            const back: unknown[] = [];
            for (let i = 0; i < nParams; i++) back[i] = fromCdata(kinds.charAt(i + 1), nargs[i]);
            return toCdata(kinds.charAt(0), names[0], next(...unpack(back, 1, nParams)));
        };
        return fromCdata(kinds.charAt(0), fn(rawNext, ...unpack(cargs, 1, nParams)));
    };
}

function makeFunction(mod: number, id: number, raw: boolean): object {
    const [path, kinds, refused, pointees] = native.ovInfo(id);
    const values = new LuaTable<string, unknown>();
    values.set("path", path);
    if (refused !== undefined) {
        values.set("refused", refused);
        values.set("override", () => error(`${path} cannot be overridden: ${refused}`, 2));
    } else {
        values.set("override", (fn: unknown) => {
            if (type(fn) !== "function") error(`${path}.override: expected a function, got a ${type(fn)}`, 2);
            let layer = fn as Fn;
            if (raw) layer = rawLayer(kinds, pointees.split("|"), layer);
            const why = native.ovAdd(mod, id, layer, raw);
            if (why !== undefined) error(`${path}: ${why}`, 2);
        });
    }
    return locked(values, path);
}

function locked(values: LuaTable<string, unknown>, name: string): object {
    const mt = new LuaTable<string, unknown>();
    mt.set("__index", values);
    mt.set("__newindex", () => error(`${name} is read-only`, 2));
    mt.set("__metatable", false);
    return setmetatable({}, mt as unknown as LuaMetatable<object>);
}

function makeNamespace(mod: number, prefix: string, raw: boolean, extra?: LuaTable<string, unknown>): object {
    const cache = new LuaTable<string, unknown>();
    const mt = new LuaTable<string, unknown>();
    mt.set("__index", (_: unknown, key: unknown) => {
        if (extra) {
            const v = rawget(extra, key);
            if (v !== undefined) return v;
        }
        if (type(key) !== "string") return undefined;
        const cached = cache.get(key as string);
        if (cached !== undefined) return cached;
        const p = prefix === "" ? (key as string) : `${prefix}.${key as string}`;
        const [kind, id] = native.ovLookup(p);
        let v: unknown;
        if (kind === 2) v = makeFunction(mod, id!, raw);
        else if (kind === 1) v = makeNamespace(mod, p, raw);
        else return undefined;
        cache.set(key as string, v);
        return v;
    });
    mt.set("__newindex", () => error(prefix === "" ? "the engine is read-only" : `${prefix} is read-only`, 2));
    mt.set("__metatable", false);
    return setmetatable({}, mt as unknown as LuaMetatable<object>);
}

/** The engine object for mod `mod` in this state. */
export function makeEngine(mod: number): object {
    const extra = new LuaTable<string, unknown>();
    extra.set("side", SIDE);
    if (native.ovRawGranted(mod)) {
        const raw = new LuaTable<string, unknown>();
        raw.set("ffi", ffi());
        extra.set("unsafe", makeNamespace(mod, "", true, raw));
    }
    return makeNamespace(mod, "", false, extra);
}
