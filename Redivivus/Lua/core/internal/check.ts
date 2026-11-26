// Argument checks for the functions mods call. Every check looks at type() only, so no metamethod
// of a value a mod passed in runs here, and messages name types, never the values themselves
// (tostring would run a __tostring of the mod's).
//
// Level 3 in the errors: the check, the API function, the mod's call.

/** The type of v as a message names it. */
export function typeName(v: unknown): string {
    return type(v);
}

/** v for a message: strings and numbers as they are, anything else by its type. */
export function show(v: unknown): string {
    const t = type(v);
    if (t === "string") return string.format("%q", v as string);
    if (t === "number") return tostring(v);
    return t;
}

function fail(i: number, where: string, expected: string, v: unknown): never {
    return error(`bad argument #${i} to '${where}' (${expected} expected, got ${v === undefined ? "no value" : type(v)})`, 4);
}

export function argString(v: unknown, i: number, where: string): string {
    if (type(v) !== "string") fail(i, where, "string", v);
    return v as string;
}

export function argNumber(v: unknown, i: number, where: string): number {
    if (type(v) !== "number") fail(i, where, "number", v);
    return v as number;
}

export function argInteger(v: unknown, i: number, where: string): number {
    if (type(v) !== "number" || (v as number) !== math.floor(v as number)) fail(i, where, "integer", v);
    return v as number;
}

export function argBoolean(v: unknown, i: number, where: string): boolean {
    if (type(v) !== "boolean") fail(i, where, "boolean", v);
    return v as boolean;
}

export function argFunction<F>(v: unknown, i: number, where: string): F {
    if (type(v) !== "function") fail(i, where, "function", v);
    return v as F;
}

export function argOptionalFunction<F>(v: unknown, i: number, where: string): F | undefined {
    if (v !== undefined && type(v) !== "function") fail(i, where, "function or nil", v);
    return v as F | undefined;
}

export function argTable<T extends object>(v: unknown, i: number, where: string): T {
    if (type(v) !== "table") fail(i, where, "table", v);
    return v as T;
}
