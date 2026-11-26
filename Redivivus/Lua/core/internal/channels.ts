// Typed channels between a mod's halves: its server side (the `server` export, per game) and its
// client side (the `client` export). One declaration, defineChannel, at module level; each side
// opens it on the object its export received (game.channel / client.channel) and so for one game.
//
// Arguments are encoded by their declared types and decoded and checked against them on the
// receiving side: ranges, lengths, choices, record shape, exact length. The client is never
// trusted; a message that does not decode is dropped and logged, and its handler never runs.
//
// The payload, inside the native message envelope (d2_lua_messages.cpp):
//   byte     version of this format (1)
//   byte     kind: 1 a request (client to server), 2 an event (server to client)
//   varint   length of the channel name, then its bytes
//   varint   the message: its index among the channel's request (or event) names, sorted
//   ...      the arguments in order, each as its type encodes it:
//            int: zigzag varint; number: varint length + "%.17g" text; bool: one byte 0 or 1;
//            text: varint length + bytes; oneOf: varint index into the choices; list: varint count
//            + items; record: the fields sorted by name; optional: byte 0, or byte 1 + the value
// Varints are base 128, low group first.
//
// Everything a mod hands in (argument values, handler tables, records, lists) is read raw; schemas
// and declarations are accepted only when the core made them.
import { CallStatus } from "../native/core";
import { show } from "./check";
import type { ChannelEntry, Endpoint, GameState } from "./games";
import { native } from "./native";

export const enum Kind {
    Int = 1,
    Number = 2,
    Bool = 3,
    Text = 4,
    OneOf = 5,
    List = 6,
    Record = 7,
    Optional = 8,
}

export type SchemaData = {
    readonly kind: Kind;
    readonly min?: number;
    readonly max?: number;
    readonly values?: readonly (string | number)[];
    readonly item?: SchemaData;
    readonly fields?: LuaTable<string, SchemaData>;
    readonly fieldNames?: string[];
};

export type ChannelData = { readonly name: string; readonly requests: ChannelEntry[]; readonly events: ChannelEntry[] };

const schemas = new LuaTable<object, true>();
setmetatable(schemas, { __mode: "k" });
const declarations = new LuaTable<object, ChannelData>();
setmetatable(declarations, { __mode: "k" });

const INT_LIMIT = 2 ** 52;
const TEXT_LIMIT = 4096;
const FORMAT = 1;
export const REQUEST = 1;
export const EVENT = 2;

function made(s: SchemaData): SchemaData {
    schemas.set(s, true);
    return s;
}

function schemaArg(v: unknown, where: string): SchemaData {
    if (type(v) !== "table" || !schemas.has(v as object)) error(`${where}: a type made with t expected`, 3);
    return v as SchemaData;
}

export function int(min: unknown, max: unknown): SchemaData {
    const lo = min === undefined ? -(2 ** 31) : min;
    const hi = max === undefined ? 2 ** 31 - 1 : max;
    if (type(lo) !== "number" || type(hi) !== "number" || !((lo as number) >= -INT_LIMIT && (hi as number) <= INT_LIMIT && (lo as number) <= (hi as number))) {
        error("t.int: the range must lie within +-2^52, min <= max", 3);
    }
    return made({ kind: Kind.Int, min: lo as number, max: hi as number });
}

export function num(min: unknown, max: unknown): SchemaData {
    const lo = min === undefined ? -math.huge : min;
    const hi = max === undefined ? math.huge : max;
    if (type(lo) !== "number" || type(hi) !== "number") error("t.number: min and max are numbers", 3);
    return made({ kind: Kind.Number, min: lo as number, max: hi as number });
}

export function bool(): SchemaData {
    return made({ kind: Kind.Bool });
}

export function text(maxLength: unknown): SchemaData {
    if (type(maxLength) !== "number" || !((maxLength as number) >= 0 && (maxLength as number) <= TEXT_LIMIT)) {
        error(`t.text: a maximum length from 0 to ${TEXT_LIMIT}`, 3);
    }
    return made({ kind: Kind.Text, max: maxLength as number });
}

export function oneOf(values: unknown): SchemaData {
    if (type(values) !== "table") error("t.oneOf: a list of values", 3);
    const list: (string | number)[] = [];
    const n = (values as unknown[]).length;
    for (let i = 1; i <= n; i++) {
        const v = rawget(values as object, i);
        if (type(v) !== "string" && type(v) !== "number") error("t.oneOf: the values are strings or numbers", 3);
        list.push(v);
    }
    if (list.length === 0) error("t.oneOf: at least one value", 3);
    return made({ kind: Kind.OneOf, values: list });
}

export function list(item: unknown, maxLength: unknown): SchemaData {
    const s = schemaArg(item, "t.list");
    if (type(maxLength) !== "number" || (maxLength as number) < 0) error("t.list: a maximum length", 3);
    return made({ kind: Kind.List, item: s, max: maxLength as number });
}

export function record(fields: unknown): SchemaData {
    if (type(fields) !== "table") error("t.record: a table of field types", 3);
    const table = new LuaTable<string, SchemaData>();
    const names: string[] = [];
    let [k, v] = next(fields as object);
    while (k !== undefined) {
        if (type(k) !== "string") error("t.record: field names are strings", 3);
        table.set(k as string, schemaArg(v, "t.record"));
        names.push(k as string);
        [k, v] = next(fields as object, k);
    }
    names.sort();
    return made({ kind: Kind.Record, fields: table, fieldNames: names });
}

export function optional(inner: unknown): SchemaData {
    return made({ kind: Kind.Optional, item: schemaArg(inner, "t.optional") });
}

// --- encoding ---------------------------------------------------------------------------------------

function varint(parts: string[], n: number): void {
    while (n >= 128) {
        parts.push(string.char((n % 128) + 128));
        n = math.floor(n / 128);
    }
    parts.push(string.char(n));
}

function bytes(parts: string[], s: string): void {
    varint(parts, s.length);
    parts.push(s);
}

function encodeValue(parts: string[], s: SchemaData, v: unknown, path: string): void {
    const t = type(v);
    switch (s.kind) {
        case Kind.Int: {
            if (t !== "number" || (v as number) !== math.floor(v as number) || (v as number) < s.min! || (v as number) > s.max!) {
                error(`${path}: a whole number from ${s.min} to ${s.max} expected, got ${show(v)}`, 0);
            }
            const n = v as number;
            varint(parts, n >= 0 ? n * 2 : -n * 2 - 1);
            return;
        }
        case Kind.Number: {
            const n = v as number;
            if (t !== "number" || n !== n || n === math.huge || n === -math.huge || n < s.min! || n > s.max!) {
                error(`${path}: a finite number in range expected, got ${show(v)}`, 0);
            }
            bytes(parts, string.format("%.17g", n));
            return;
        }
        case Kind.Bool:
            if (t !== "boolean") error(`${path}: a boolean expected, got ${t}`, 0);
            parts.push(string.char(v ? 1 : 0));
            return;
        case Kind.Text:
            if (t !== "string" || (v as string).length > s.max!) error(`${path}: a string of at most ${s.max} bytes expected`, 0);
            bytes(parts, v as string);
            return;
        case Kind.OneOf: {
            const values = s.values!;
            for (let i = 0; i < values.length; i++) {
                if (values[i] === v) {
                    varint(parts, i);
                    return;
                }
            }
            error(`${path}: ${show(v)} is not one of the choices`, 0);
            return;
        }
        case Kind.List: {
            if (t !== "table") error(`${path}: a list expected, got ${t}`, 0);
            const n = (v as unknown[]).length;
            if (n > s.max!) error(`${path}: at most ${s.max} items`, 0);
            varint(parts, n);
            for (let i = 1; i <= n; i++) encodeValue(parts, s.item!, rawget(v as object, i), `${path}[${i}]`);
            return;
        }
        case Kind.Record: {
            if (t !== "table") error(`${path}: a table expected, got ${t}`, 0);
            let [k] = next(v as object);
            while (k !== undefined) {
                if (type(k) !== "string" || s.fields!.get(k as string) === undefined) error(`${path}: no field ${show(k)} is declared`, 0);
                [k] = next(v as object, k);
            }
            for (const key of s.fieldNames!) encodeValue(parts, s.fields!.get(key)!, rawget(v as object, key), `${path}.${key}`);
            return;
        }
        case Kind.Optional:
            if (v === undefined) {
                parts.push(string.char(0));
            } else {
                parts.push(string.char(1));
                encodeValue(parts, s.item!, v, path);
            }
            return;
    }
}

class Reader {
    pos = 1;
    constructor(public readonly data: string) {}
}

class DecodeError {
    constructor(public readonly why: string) {}
}

function fail(why: string): never {
    throw new DecodeError(why);
}

function rbyte(r: Reader): number {
    if (r.pos > r.data.length) fail("the message ends early");
    const b = string.byte(r.data, r.pos);
    r.pos++;
    return b;
}

function rvarint(r: Reader): number {
    let n = 0;
    let scale = 1;
    for (;;) {
        const b = rbyte(r);
        n += (b % 128) * scale;
        if (b < 128) return n;
        scale *= 128;
        if (scale > 2 ** 56) fail("a number is too long");
    }
}

function rbytes(r: Reader, max: number): string {
    const length = rvarint(r);
    if (length > max) fail(`a string of ${length} bytes, at most ${max} allowed`);
    if (r.pos + length - 1 > r.data.length) fail("the message ends early");
    const s = string.sub(r.data, r.pos, r.pos + length - 1);
    r.pos += length;
    return s;
}

function decodeValue(r: Reader, s: SchemaData): unknown {
    switch (s.kind) {
        case Kind.Int: {
            const z = rvarint(r);
            const v = z % 2 === 0 ? z / 2 : -(z + 1) / 2;
            if (v < s.min! || v > s.max!) fail(`${v} is outside ${s.min}..${s.max}`);
            return v;
        }
        case Kind.Number: {
            const v = tonumber(rbytes(r, 32));
            if (v === undefined || v !== v || v === math.huge || v === -math.huge || v < s.min! || v > s.max!) fail("not a finite number in range");
            return v;
        }
        case Kind.Bool: {
            const b = rbyte(r);
            if (b > 1) fail("not a boolean");
            return b === 1;
        }
        case Kind.Text:
            return rbytes(r, s.max!);
        case Kind.OneOf: {
            const index = rvarint(r);
            if (index >= s.values!.length) fail("not one of the choices");
            return s.values![index];
        }
        case Kind.List: {
            const count = rvarint(r);
            if (count > s.max!) fail(`${count} items, at most ${s.max} allowed`);
            const out: unknown[] = [];
            for (let i = 0; i < count; i++) out.push(decodeValue(r, s.item!));
            return out;
        }
        case Kind.Record: {
            const out = new LuaTable<string, unknown>();
            for (const key of s.fieldNames!) out.set(key, decodeValue(r, s.fields!.get(key)!));
            return out;
        }
        case Kind.Optional: {
            const flag = rbyte(r);
            if (flag > 1) fail("not an optional's flag");
            return flag === 1 ? decodeValue(r, s.item!) : undefined;
        }
    }
    return fail("unknown type");
}

// --- declarations and endpoints ----------------------------------------------------------------

function messageEntries(messages: unknown, what: string): ChannelEntry[] {
    if (messages === undefined) return [];
    if (type(messages) !== "table") error(`defineChannel: ${what} is a table of message names to argument lists`, 4);
    const names: string[] = [];
    let [k] = next(messages as object);
    while (k !== undefined) {
        if (type(k) !== "string") error(`defineChannel: ${what} are named by strings`, 4);
        names.push(k as string);
        [k] = next(messages as object, k);
    }
    names.sort();
    const out: ChannelEntry[] = [];
    for (const name of names) {
        const args = rawget(messages as object, name);
        if (type(args) !== "table") error(`defineChannel: ${what}.${name} is a list of types made with t`, 4);
        const list: object[] = [];
        const n = (args as unknown[]).length;
        for (let i = 1; i <= n; i++) list.push(schemaArg(rawget(args as object, i), `defineChannel: ${what}.${name}`));
        out.push({ name, args: list });
    }
    return out;
}

/** A declaration: pure data, the same on both sides. */
export function define(name: unknown, messages: unknown): ChannelData {
    if (type(name) !== "string" || (name as string).length === 0 || (name as string).length > 64) error("defineChannel: a name of 1 to 64 bytes", 3);
    if (type(messages) !== "table") error("defineChannel: a table { requests?, events? }", 3);
    const d: ChannelData = {
        name: name as string,
        requests: messageEntries(rawget(messages as object, "requests"), "requests"),
        events: messageEntries(rawget(messages as object, "events"), "events"),
    };
    declarations.set(d, d);
    return d;
}

export function declarationArg(v: unknown, where: string): ChannelData {
    const d = type(v) === "table" ? declarations.get(v as object) : undefined;
    if (!d) error(`${where}: a channel made with defineChannel expected`, 3);
    return d!;
}

/** The game's endpoint of a mod's channel, opened once. */
export function open(g: GameState, mod: number, d: ChannelData, where: string): Endpoint {
    let perMod = g.endpoints.get(mod);
    if (!perMod) {
        perMod = new LuaTable();
        g.endpoints.set(mod, perMod);
    }
    if (perMod.get(d.name)) error(`${where}: channel '${d.name}' is already open in this game`, 3);
    const copyEntries = (list: ChannelEntry[]) => list.map((e): ChannelEntry => ({ name: e.name, args: e.args }));
    const ep: Endpoint = { name: d.name, requests: copyEntries(d.requests), events: copyEntries(d.events) };
    perMod.set(d.name, ep);
    return ep;
}

export function encode(kind: number, ep: Endpoint, index: number, entry: ChannelEntry, args: unknown[], count: number): string {
    const parts: string[] = [];
    parts.push(string.char(FORMAT), string.char(kind));
    bytes(parts, ep.name);
    varint(parts, index);
    const n = entry.args.length;
    if (count > n) error(`${ep.name}.${entry.name}: ${n} arguments expected, got ${count}`, 0);
    for (let i = 0; i < n; i++) encodeValue(parts, entry.args[i] as SchemaData, args[i], `${ep.name}.${entry.name} argument ${i + 1}`);
    return table.concat(parts);
}

function logDrop(mod: number, peer: number, where: string, why: string): void {
    native.log(mod, `channel message from peer ${peer} dropped${where !== "" ? ` (${where})` : ""}: ${why}`);
}

/**
 * Decode a message for the mod's endpoints in this game and run its handler, as the mod. Runs as a
 * call of the mod (deliver puts it there), so the decoding is billed to the receiver.
 */
export function receive(g: GameState, mod: number, payload: string, peer: number, server: boolean): void {
    const r = new Reader(payload);
    let entry: ChannelEntry | undefined;
    const args: unknown[] = [];
    let where = "";
    try {
        if (rbyte(r) !== FORMAT) fail("unknown channel format");
        const kind = rbyte(r);
        const name = rbytes(r, 64);
        const ep = g.endpoints.get(mod)?.get(name);
        if (!ep) fail(`no channel '${name}'`);
        const expect = server ? REQUEST : EVENT;
        if (kind !== expect) fail(server ? "only requests reach the server" : "only events reach the client");
        const list = kind === REQUEST ? ep.requests : ep.events;
        const index = rvarint(r);
        entry = list[index];
        if (!entry) fail(`channel '${name}' has no message ${index}`);
        where = `${name}.${entry.name}`;
        for (const s of entry.args) args.push(decodeValue(r, s as SchemaData));
        if (r.pos !== r.data.length + 1) fail("bytes after the last argument");
    } catch (e) {
        if (e instanceof DecodeError) {
            logDrop(mod, peer, where, e.why);
            return;
        }
        throw e;
    }
    const handler = entry.handler;
    if (!handler) {
        native.log(mod, `channel message ${where} from peer ${peer} has no handler`);
        return;
    }
    const [status, err] = server
        ? native.call(mod, handler, peer, ...unpack(args, 1, entry.args.length))
        : native.call(mod, handler, ...unpack(args, 1, entry.args.length));
    if (status === CallStatus.Error) error(err, 0);
    if (status === CallStatus.Fatal) error(err, 0);
}
