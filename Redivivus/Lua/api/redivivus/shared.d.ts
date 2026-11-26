// Generated from Redivivus/Lua/core/redivivus/shared.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
/**
 * What every state has (data, server, client): logging, settings, channel declarations, the
 * waits scripts use, optional imports, and the types the rest of the API is written in. Nothing
 * here registers behaviour: that goes through the object a mod's `server` or `client` export
 * receives (see redivivus:server and redivivus:client).
 * @side data server client
 */
import type { Client } from "./client";
import { Outcome, SettingScope, SettingType, Side } from "./enums";
import type { Event } from "./enums";
import type { Game } from "./server";
/** A Lua number that the host checks is a whole number. */
export type integer = number;
/**
 * Any handle: an engine object named by class and id. It is valid while its object exists; any
 * other use of an invalid handle raises an error.
 */
export type Handle = {
    readonly valid: boolean;
};
/** Fields every event payload carries. */
export interface EventBase {
    /** Which event it is. */
    readonly name: Event;
    /** The tick it was raised on. */
    readonly tick: integer;
}
/** What a script yields: nothing (resume next tick), a tick count, or an event to wait for. */
export type ScriptWait = {
    readonly ticks: integer;
} | {
    readonly event: Event;
};
/**
 * A script or an action a script runs. Declare script functions as `function*` returning
 * `Script<T>`: the mod build compiles them to plain Lua functions that yield on the script's
 * coroutine, so `yield` waits for the next tick and `yield* action()` runs an action and gives
 * its outcome. A `Script` function is only called through `yield*` or handed to startScript.
 */
export interface Script<T = void> extends Generator<ScriptWait | undefined, T, any> {
}
/**
 * What a mod's `main` entry exports. The engine calls `server` once for every game this process
 * hosts and `client` once for every game this client is in; a side that does not run here never
 * has its export called. Everything a mod does in a game it registers through the object it gets,
 * and it is dropped when that game ends.
 */
export interface ModMain {
    /** Called once for every game this process hosts, with that game. */
    server?(this: void, game: Game): void;
    /** Called once for every game this client is in. */
    client?(this: void, client: Client): void;
}
/** A registry entry's arguments. */
export type RegistryArgs<F> = F extends (this: void, ...args: infer A) => any ? A : never;
/** A registry entry's result. */
export type RegistryResult<F> = F extends (this: void, ...args: any[]) => infer R ? R : never;
/** A wrapper around a registry entry: `next` runs the entry as it was before this wrapper. */
export type RegistryWrapper<F> = (this: void, next: (this: void, ...args: RegistryArgs<F>) => RegistryResult<F>, ...args: RegistryArgs<F>) => RegistryResult<F>;
/**
 * A registry of entries keyed by id, for one game. Ids are members of a string enum and the
 * signatures are the open interface M, keyed by them; a mod adds an id by augmenting both.
 */
export interface KeyedRegistry<M> {
    /** Add the entry, or replace its implementation (wrappers stay). */
    register<K extends keyof M & string>(this: void, id: K, impl: M[K]): void;
    /** Wrap the entry. Later mods wrap earlier ones; a wrapper that errors is removed. */
    wrap<K extends keyof M & string>(this: void, id: K, wrapper: RegistryWrapper<M[K]>): void;
    /** Run the entry with its wrappers. */
    call<K extends keyof M & string>(this: void, id: K, ...args: RegistryArgs<M[K]>): RegistryResult<M[K]>;
    /** Whether the entry exists and has an implementation. */
    has(this: void, id: keyof M & string): boolean;
}
/** A registry with a single entry of signature F, for one game. */
export interface SlotRegistry<F> {
    /** Replace the implementation (wrappers stay). */
    set(this: void, impl: F): void;
    /** Wrap the entry. */
    wrap(this: void, wrapper: RegistryWrapper<F>): void;
    /** Run the entry with its wrappers. */
    call(this: void, ...args: RegistryArgs<F>): RegistryResult<F>;
}
/** A running script. */
export interface ScriptTask {
    readonly id: integer;
    /** Stop it; it is not resumed again. True when it was still running. */
    cancel(this: void): boolean;
    /** Whether it is still running. */
    isRunning(this: void): boolean;
}
/** This mod's name. */
export declare const modName: string;
/** Which state this is. */
export declare const side: Side;
/**
 * Write a line to the game's log, prefixed with the mod's name. Values are converted with
 * tostring and separated by spaces.
 */
export declare function log(...values: unknown[]): void;
/**
 * Another mod's module when that mod is installed, else undefined. The mod must be a declared
 * dependency (typically an optional one, "? other-mod"). Type it with the module's own type:
 * `optionalImport<typeof import("other-mod:api")>("other-mod:api")`.
 */
export declare function optionalImport<T = unknown>(specifier: string): T | undefined;
/** Wait ticks (default 1) inside a script. The outcome is the tick it resumes on. */
export declare function wait(ticks?: integer): Script<integer>;
/**
 * Wait until pred holds, checking it now and then once a tick, for at most timeoutTicks ticks.
 * Outcome.Done when it held, Outcome.Timeout when the time ran out first.
 */
export declare function waitUntil(pred: (this: void) => boolean, timeoutTicks: integer): Script<Outcome>;
/**
 * Run body as a step of this script while check holds. check is called before each step of the
 * body: before it starts and before every resume after it waited. While it holds the body runs as
 * if it were called with `yield*` (what the script is resumed with reaches the body, what the body
 * waits for the script waits for); once it does not, the body is dropped where it waits, never
 * resumed again, and the outcome is Outcome.Interrupted. Otherwise the outcome is the body's.
 *
 * A body waits as a whole: a `wait(100)` or an action the engine runs is not cut short, and an
 * action the body started is left to the engine as it runs it. Errors in check or body propagate.
 * Guards nest. Outside a script, in a coroutine of the mod's own (a test driving a Script), the
 * body's waits reach whoever resumes it. `yield* guard(() => target.valid && !target.dead, chase(monster, target))`: the
 * build hands the body over unstarted. In Lua the body is a function that runs it:
 * `guard(check, function() return chase(monster, target) end)`.
 */
export declare function guard<T>(check: (this: void) => boolean, body: Script<T>): Script<T | Outcome>;
/** A setting as the host takes it. The setting builders make these. */
export interface SettingDeclaration {
    readonly type: SettingType;
    readonly scope: SettingScope;
    readonly default: boolean | number | string;
    readonly min?: number;
    readonly max?: number;
    readonly choices?: readonly string[];
    readonly title?: string;
    readonly description?: string;
}
/** A declared setting with the value in effect, as listSettings gives it. */
export interface SettingInfo extends SettingDeclaration {
    readonly mod: string;
    readonly name: string;
    readonly value: boolean | number | string;
}
/** A declared setting whose value is a T. */
export interface SettingSpec<T> {
    readonly declaration: SettingDeclaration;
    /** Carries T for the types; never set. */
    readonly value?: T;
}
type Common = {
    readonly scope: SettingScope;
    readonly title?: string;
    readonly description?: string;
};
/** The builders of setting declarations. */
export declare const setting: {
    bool(this: void, o: Common & {
        readonly default: boolean;
    }): SettingSpec<boolean>;
    int(this: void, o: Common & {
        readonly default: integer;
        readonly min?: integer;
        readonly max?: integer;
    }): SettingSpec<integer>;
    number(this: void, o: Common & {
        readonly default: number;
        readonly min?: number;
        readonly max?: number;
    }): SettingSpec<number>;
    text(this: void, o: Common & {
        readonly default: string;
    }): SettingSpec<string>;
    choice<const C extends string>(this: void, o: Common & {
        readonly choices: readonly C[];
        readonly default: NoInfer<C>;
    }): SettingSpec<C>;
};
/** The read-only values of a set of declared settings. */
export type Settings<T> = {
    readonly [K in keyof T]: T[K] extends SettingSpec<infer V> ? V : never;
};
/**
 * Declare the settings (sorted by name, the order a settings screen lists them in) and get an
 * object that reads their values. Writing to it is an error. Values come from the settings file
 * under the mods root, checked against the declaration; a setting is read on its scope's side.
 */
export declare function defineSettings<T extends Record<string, SettingSpec<unknown>>>(specs: T): Settings<T>;
/** Every setting declared in this state, all mods, in declaration order. */
export declare function listSettings(): SettingInfo[];
/** A declared argument type whose values are T. Made with t. */
export interface Schema<T> {
    /** Carries T for the types; never set. */
    readonly type?: T;
}
/** The value type a schema declares. */
export type Infer<S> = S extends Schema<infer T> ? T : never;
/** A channel's messages: name -> argument types. */
export type Messages = Record<string, readonly Schema<unknown>[]>;
/** The argument values of a message. */
export type MessageArgs<A extends readonly Schema<unknown>[]> = {
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
/**
 * A channel between this mod's server and client sides: requests go from a client to the server,
 * events from the server to clients. Declared once at module level; each side opens it on the
 * object its export received (`game.channel(c)`, `client.channel(c)`).
 */
export interface ChannelDeclaration<R extends Messages, E extends Messages> {
    readonly name: string;
    /** Carries the message types; never set. */
    readonly messages?: {
        requests: R;
        events: E;
    };
}
/** Declare a channel of this mod. The same declaration is used on both sides. */
export declare function defineChannel<const R extends Messages = {}, const E extends Messages = {}>(name: string, messages: {
    readonly requests?: R;
    readonly events?: E;
}): ChannelDeclaration<R, E>;
export {};
