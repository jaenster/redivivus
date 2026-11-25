// Typed channels between a mod's client and server halves. One declaration in shared code:
//
//   import { defineChannel, t } from "__base__/shared/kit/channels";
//   export const chat = defineChannel("chat", {
//       requests: { say: [t.text(200), t.oneOf([Colour.White, Colour.Gold])] },   // client -> server
//       events: { said: [t.int(), t.text(200)] },                                 // server -> client
//   });
//
// The server implements the requests and emits events; the client calls and listens:
//
//   chat.implement({ say(peer, text, colour) { chat.emit.said(0, peer, text); } });   // server
//   chat.call.say("hello", Colour.Gold);  chat.on.said((from, text) => ...);           // client
//
// Arguments are encoded by their declared types and decoded and checked against them on the
// receiving side: ranges, lengths, choices, record shape, exact length. The client is never
// trusted; a message that does not decode is dropped and logged, and its handler never runs.
// Delivery is the host's script.send / script.on_message, so each received message is a guarded
// entry of the receiving mod with its own time budget.
//
// The payload, inside the host's message envelope (d2_lua_messages.cpp):
//   byte     version of this format (1)
//   byte     kind: 1 a request (client to server), 2 an event (server to client)
//   varint   length of the channel name, then its bytes
//   varint   the message: its index among the channel's request (or event) names, sorted
//   ...      the arguments in order, each as its type encodes it:
//            int: zigzag varint; number: varint length + "%.17g" text; bool: one byte 0 or 1;
//            text: varint length + bytes; oneOf: varint index into the choices; list: varint count
//            + items; record: the fields sorted by name; optional: byte 0, or byte 1 + the value
// Varints are base 128, low group first. A channel's schema is the same on both ends because the
// mod set is.

// The control stages' script object, as far as channels use it; the data stage has none.
interface KitScript {
    readonly side: Side;
    send(this: void, payload: string, peer?: integer): void;
    on_message(this: void, handler: ((this: void, payload: string, peer: integer) => void) | undefined): void;
}
declare const script: KitScript;

const enum Kind {
    Int = 1,
    Number = 2,
    Bool = 3,
    Text = 4,
    OneOf = 5,
    List = 6,
    Record = 7,
    Optional = 8,
}

/** A declared argument type whose values are T. Built with t; read by the encoder. */
export interface Schema<T> {
    readonly kind: Kind;
    readonly min?: number;
    readonly max?: number;
    readonly values?: readonly (string | number)[];
    readonly item?: Schema<unknown>;
    readonly fields?: Record<string, Schema<unknown>>;
    /** Carries T for the types; never set. */
    readonly type?: T;
}

/** The value type a schema declares. */
export type Infer<S> = S extends Schema<infer T> ? T : never;
type Args<A extends readonly Schema<unknown>[]> = { [I in keyof A]: Infer<A[I]> };

const INT_LIMIT = 2 ** 52;
const TEXT_LIMIT = 4096;

/** The argument types. */
export const t = {
    /** A whole number, by default a signed 32-bit one. */
    int(this: void, min = -(2 ** 31), max = 2 ** 31 - 1): Schema<integer> {
        if (!(min >= -INT_LIMIT && max <= INT_LIMIT && min <= max)) error("t.int: the range must lie within +-2^52, min <= max", 2);
        return { kind: Kind.Int, min, max };
    },
    /** A finite number. */
    number(this: void, min = -math.huge, max = math.huge): Schema<number> {
        return { kind: Kind.Number, min, max };
    },
    bool(this: void): Schema<boolean> {
        return { kind: Kind.Bool };
    },
    /** A string of at most maxLength bytes. */
    text(this: void, maxLength: integer): Schema<string> {
        if (!(maxLength >= 0 && maxLength <= TEXT_LIMIT)) error(`t.text: a maximum length from 0 to ${TEXT_LIMIT}`, 2);
        return { kind: Kind.Text, max: maxLength };
    },
    /** One of the values (enum members, typically). */
    oneOf<const V extends string | number>(this: void, values: readonly V[]): Schema<V> {
        if (values.length === 0) error("t.oneOf: at least one value", 2);
        return { kind: Kind.OneOf, values };
    },
    /** A list of at most maxLength items. */
    list<T>(this: void, item: Schema<T>, maxLength: integer): Schema<T[]> {
        return { kind: Kind.List, item, max: maxLength };
    },
    /** A table with exactly these fields. */
    record<const F extends Record<string, Schema<unknown>>>(this: void, fields: F): Schema<{ [K in keyof F]: Infer<F[K]> }> {
        return { kind: Kind.Record, fields };
    },
    /** The value, or nothing. */
    optional<T>(this: void, inner: Schema<T>): Schema<T | undefined> {
        return { kind: Kind.Optional, item: inner };
    },
};

// --- encoding -------------------------------------------------------------------------------------

class Writer {
    parts: string[] = [];
    byte(b: number): void {
        this.parts.push(string.char(b));
    }
    varint(n: number): void {
        while (n >= 128) {
            this.byte((n % 128) + 128);
            n = math.floor(n / 128);
        }
        this.byte(n);
    }
    bytes(s: string): void {
        this.varint(s.length);
        this.parts.push(s);
    }
}

class Reader {
    pos = 1;
    constructor(public readonly data: string) {}
    fail(why: string): never {
        throw new DecodeError(why);
    }
    byte(): number {
        if (this.pos > this.data.length) this.fail("the message ends early");
        const b = string.byte(this.data, this.pos);
        this.pos++;
        return b;
    }
    varint(): number {
        let n = 0;
        let scale = 1;
        for (;;) {
            const b = this.byte();
            n += (b % 128) * scale;
            if (b < 128) return n;
            scale *= 128;
            if (scale > 2 ** 56) this.fail("a number is too long");
        }
    }
    bytes(max: number): string {
        const length = this.varint();
        if (length > max) this.fail(`a string of ${length} bytes, at most ${max} allowed`);
        if (this.pos + length - 1 > this.data.length) this.fail("the message ends early");
        const s = string.sub(this.data, this.pos, this.pos + length - 1);
        this.pos += length;
        return s;
    }
}

class DecodeError {
    constructor(public readonly why: string) {}
}

function sortedKeys(o: object): string[] {
    return Object.keys(o).sort();
}

function encodeValue(w: Writer, s: Schema<unknown>, v: unknown, path: string): void {
    switch (s.kind) {
        case Kind.Int: {
            if (typeof v !== "number" || v !== math.floor(v) || v < s.min! || v > s.max!) error(`${path}: a whole number from ${s.min} to ${s.max} expected, got ${tostring(v)}`, 0);
            w.varint(v >= 0 ? v * 2 : -v * 2 - 1);
            return;
        }
        case Kind.Number: {
            if (typeof v !== "number" || v !== v || v === math.huge || v === -math.huge || v < s.min! || v > s.max!) error(`${path}: a finite number in range expected, got ${tostring(v)}`, 0);
            w.bytes(string.format("%.17g", v));
            return;
        }
        case Kind.Bool:
            if (typeof v !== "boolean") error(`${path}: a boolean expected, got ${type(v)}`, 0);
            w.byte(v ? 1 : 0);
            return;
        case Kind.Text:
            if (typeof v !== "string" || v.length > s.max!) error(`${path}: a string of at most ${s.max} bytes expected`, 0);
            w.bytes(v);
            return;
        case Kind.OneOf: {
            const index = s.values!.indexOf(v as string | number);
            if (index < 0) error(`${path}: ${tostring(v)} is not one of the choices`, 0);
            w.varint(index);
            return;
        }
        case Kind.List: {
            if (type(v) !== "table") error(`${path}: a list expected, got ${type(v)}`, 0);
            const list = v as unknown[];
            if (list.length > s.max!) error(`${path}: at most ${s.max} items`, 0);
            w.varint(list.length);
            for (let i = 0; i < list.length; i++) encodeValue(w, s.item!, list[i], `${path}[${i + 1}]`);
            return;
        }
        case Kind.Record: {
            if (type(v) !== "table") error(`${path}: a table expected, got ${type(v)}`, 0);
            const record = v as Record<string, unknown>;
            for (const key of sortedKeys(record)) {
                if (!(key in s.fields!)) error(`${path}: no field '${key}' is declared`, 0);
            }
            for (const key of sortedKeys(s.fields!)) encodeValue(w, s.fields![key], record[key], `${path}.${key}`);
            return;
        }
        case Kind.Optional:
            if (v === undefined) {
                w.byte(0);
            } else {
                w.byte(1);
                encodeValue(w, s.item!, v, path);
            }
            return;
    }
}

function decodeValue(r: Reader, s: Schema<unknown>): unknown {
    switch (s.kind) {
        case Kind.Int: {
            const z = r.varint();
            const v = z % 2 === 0 ? z / 2 : -(z + 1) / 2;
            if (v < s.min! || v > s.max!) r.fail(`${v} is outside ${s.min}..${s.max}`);
            return v;
        }
        case Kind.Number: {
            const v = tonumber(r.bytes(32));
            if (v === undefined || v !== v || v === math.huge || v === -math.huge || v < s.min! || v > s.max!) r.fail("not a finite number in range");
            return v;
        }
        case Kind.Bool: {
            const b = r.byte();
            if (b > 1) r.fail("not a boolean");
            return b === 1;
        }
        case Kind.Text:
            return r.bytes(s.max!);
        case Kind.OneOf: {
            const index = r.varint();
            if (index >= s.values!.length) r.fail("not one of the choices");
            return s.values![index];
        }
        case Kind.List: {
            const count = r.varint();
            if (count > s.max!) r.fail(`${count} items, at most ${s.max} allowed`);
            const out: unknown[] = [];
            for (let i = 0; i < count; i++) out.push(decodeValue(r, s.item!));
            return out;
        }
        case Kind.Record: {
            const out: Record<string, unknown> = {};
            for (const key of sortedKeys(s.fields!)) out[key] = decodeValue(r, s.fields![key]);
            return out;
        }
        case Kind.Optional: {
            const flag = r.byte();
            if (flag > 1) r.fail("not an optional's flag");
            return flag === 1 ? decodeValue(r, s.item!) : undefined;
        }
    }
    return r.fail("unknown type");
}

// --- channels ---------------------------------------------------------------------------------------

const FORMAT = 1;
const REQUEST = 1;
const EVENT = 2;

type Messages = Record<string, readonly Schema<unknown>[]>;

type Entry = { name: string; args: readonly Schema<unknown>[]; handler?: (this: void, ...args: unknown[]) => void };
type ChannelState = { name: string; requests: Entry[]; events: Entry[] };

// This mod's channels on this side, by name. The kit runs in the requiring mod's environment, so
// this table is the mod's own.
const channels = new LuaTable<string, ChannelState>();
let receiving = false;

function encode(kind: number, channel: string, index: number, entry: Entry, args: unknown[]): string {
    const w = new Writer();
    w.byte(FORMAT);
    w.byte(kind);
    w.bytes(channel);
    w.varint(index);
    const n = entry.args.length;
    if (args.length > n) error(`${channel}.${entry.name}: ${n} arguments expected, got ${args.length}`, 0);
    for (let i = 0; i < n; i++) encodeValue(w, entry.args[i], args[i], `${channel}.${entry.name} argument ${i + 1}`);
    return table.concat(w.parts);
}

function receive(this: void, payload: string, peer: integer): void {
    const r = new Reader(payload);
    let entry: Entry | undefined;
    let args: unknown[] = [];
    let where = "";
    try {
        if (r.byte() !== FORMAT) r.fail("unknown channel format");
        const kind = r.byte();
        const name = r.bytes(64);
        const state = channels.get(name);
        if (!state) r.fail(`no channel '${name}'`);
        const expect = script.side === Side.Server ? REQUEST : EVENT;
        if (kind !== expect) r.fail(script.side === Side.Server ? "only requests reach the server" : "only events reach the client");
        const list = kind === REQUEST ? state.requests : state.events;
        const index = r.varint();
        entry = list[index];
        if (!entry) r.fail(`channel '${name}' has no message ${index}`);
        where = `${name}.${entry.name}`;
        for (const s of entry.args) args.push(decodeValue(r, s));
        if (r.pos !== r.data.length + 1) r.fail("bytes after the last argument");
    } catch (e) {
        if (e instanceof DecodeError) {
            log(`channel message from peer ${peer} dropped${where ? ` (${where})` : ""}: ${e.why}`);
            return;
        }
        throw e;
    }
    if (!entry.handler) {
        log(`channel message ${where} from peer ${peer} has no handler`);
        return;
    }
    if (script.side === Side.Server) {
        entry.handler(peer, ...args);
    } else {
        entry.handler(...args);
    }
}

function entries(messages: Messages | undefined): Entry[] {
    if (!messages) return [];
    return sortedKeys(messages).map((name) => ({ name, args: messages[name] }));
}

function onSide(side: Side, what: string): void {
    const here = typeof script === "undefined" ? Side.Data : script.side;
    if (here !== side) error(`${what} is for the ${side === Side.Server ? "server" : "client"} side`, 3);
}

/** A channel: what each side does with it. */
export interface Channel<R extends Messages, E extends Messages> {
    readonly name: string;
    /** Server: handle the requests. Each handler gets the sending client's peer id first. */
    implement(this: void, handlers: { [K in keyof R]: (this: void, peer: integer, ...args: Args<R[K]>) => void }): void;
    /** Client: send a request. */
    readonly call: { readonly [K in keyof R]: (this: void, ...args: Args<R[K]>) => void };
    /** Server: send an event to one client, or to every client with peer 0. */
    readonly emit: { readonly [K in keyof E]: (this: void, peer: integer, ...args: Args<E[K]>) => void };
    /** Client: handle an event. */
    readonly on: { readonly [K in keyof E]: (this: void, handler: (this: void, ...args: Args<E[K]>) => void) => void };
}

/** Declare a channel of this mod. The same declaration runs on both sides. */
export function defineChannel<const R extends Messages = {}, const E extends Messages = {}>(
    name: string, messages: { readonly requests?: R; readonly events?: E }): Channel<R, E> {
    if (name.length === 0 || name.length > 64) error("defineChannel: a name of 1 to 64 bytes", 2);
    if (channels.has(name)) error(`defineChannel: this mod already has a channel '${name}'`, 2);
    const state: ChannelState = { name, requests: entries(messages.requests), events: entries(messages.events) };
    channels.set(name, state);
    if (!receiving && typeof script !== "undefined") {
        script.on_message(receive);
        receiving = true;
    }
    const call: Record<string, (this: void, ...args: unknown[]) => void> = {};
    const emit: Record<string, (this: void, peer: integer, ...args: unknown[]) => void> = {};
    const on: Record<string, (this: void, handler: (this: void, ...args: unknown[]) => void) => void> = {};
    state.requests.forEach((entry, i) => {
        call[entry.name] = (...args: unknown[]) => {
            onSide(Side.Client, `${name}.call.${entry.name}`);
            script.send(encode(REQUEST, name, i, entry, args));
        };
    });
    state.events.forEach((entry, i) => {
        emit[entry.name] = (peer: integer, ...args: unknown[]) => {
            onSide(Side.Server, `${name}.emit.${entry.name}`);
            script.send(encode(EVENT, name, i, entry, args), peer);
        };
        on[entry.name] = (handler) => {
            onSide(Side.Client, `${name}.on.${entry.name}`);
            entry.handler = handler;
        };
    });
    return {
        name,
        implement(this: void, handlers: Record<string, (this: void, ...args: unknown[]) => void>) {
            onSide(Side.Server, `${name}.implement`);
            for (const entry of state.requests) {
                const handler = handlers[entry.name];
                if (typeof handler !== "function") error(`${name}.implement: no handler for request '${entry.name}'`, 2);
                entry.handler = handler;
            }
        },
        call, emit, on,
    } as unknown as Channel<R, E>;
}
