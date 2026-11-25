// Generated from Redivivus/Lua/kit/shared/channels.ts. Do not edit.
declare const enum Kind {
    Int = 1,
    Number = 2,
    Bool = 3,
    Text = 4,
    OneOf = 5,
    List = 6,
    Record = 7,
    Optional = 8
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
type Args<A extends readonly Schema<unknown>[]> = {
    [I in keyof A]: Infer<A[I]>;
};
/** The argument types. */
export declare const t: {
    /** A whole number, by default a signed 32-bit one. */
    int(this: void, min?: number, max?: number): Schema<integer>;
    /** A finite number. */
    number(this: void, min?: number, max?: number): Schema<number>;
    bool(this: void): Schema<boolean>;
    /** A string of at most maxLength bytes. */
    text(this: void, maxLength: integer): Schema<string>;
    /** One of the values (enum members, typically). */
    oneOf<const V extends string | number>(this: void, values: readonly V[]): Schema<V>;
    /** A list of at most maxLength items. */
    list<T>(this: void, item: Schema<T>, maxLength: integer): Schema<T[]>;
    /** A table with exactly these fields. */
    record<const F extends Record<string, Schema<unknown>>>(this: void, fields: F): Schema<{ [K in keyof F]: Infer<F[K]>; }>;
    /** The value, or nothing. */
    optional<T>(this: void, inner: Schema<T>): Schema<T | undefined>;
};
type Messages = Record<string, readonly Schema<unknown>[]>;
/** A channel: what each side does with it. */
export interface Channel<R extends Messages, E extends Messages> {
    readonly name: string;
    /** Server: handle the requests. Each handler gets the sending client's peer id first. */
    implement(this: void, handlers: {
        [K in keyof R]: (this: void, peer: integer, ...args: Args<R[K]>) => void;
    }): void;
    /** Client: send a request. */
    readonly call: {
        readonly [K in keyof R]: (this: void, ...args: Args<R[K]>) => void;
    };
    /** Server: send an event to one client, or to every client with peer 0. */
    readonly emit: {
        readonly [K in keyof E]: (this: void, peer: integer, ...args: Args<E[K]>) => void;
    };
    /** Client: handle an event. */
    readonly on: {
        readonly [K in keyof E]: (this: void, handler: (this: void, ...args: Args<E[K]>) => void) => void;
    };
}
/** Declare a channel of this mod. The same declaration runs on both sides. */
export declare function defineChannel<const R extends Messages = {}, const E extends Messages = {}>(name: string, messages: {
    readonly requests?: R;
    readonly events?: E;
}): Channel<R, E>;
export {};
