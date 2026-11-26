/**
 * What every state has (data, server, client): logging, settings, channel declarations, the
 * waits scripts use, optional imports, and the types the rest of the API is written in. Nothing
 * here registers behaviour: that goes through the object a mod's `server` or `client` export
 * receives (see redivivus:server and redivivus:client).
 * @side data server client
 */

import { CallStatus } from "../native/core";
import * as ai from "../internal/ai";
import * as channels from "../internal/channels";
import { argFunction, argInteger, argString } from "../internal/check";
import { modId, modName as contextName } from "../internal/context";
import { native } from "../internal/native";
import { readonly } from "../internal/proxy";
import * as scheduler from "../internal/scheduler";
import * as settings from "../internal/settings";
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
export type Handle = { readonly valid: boolean };

/** Fields every event payload carries. */
export interface EventBase {
    /** Which event it is. */
    readonly name: Event;
    /** The tick it was raised on. */
    readonly tick: integer;
}

/** What a script yields: nothing (resume next tick), a tick count, or an event to wait for. */
export type ScriptWait = { readonly ticks: integer } | { readonly event: Event };

/**
 * A script or an action a script runs. Declare script functions as `function*` returning
 * `Script<T>`: the mod build compiles them to plain Lua functions that yield on the script's
 * coroutine, so `yield` waits for the next tick and `yield* action()` runs an action and gives
 * its outcome. A `Script` function is only called through `yield*` or handed to startScript.
 */
export interface Script<T = void> extends Generator<ScriptWait | undefined, T, any> {}

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
export type RegistryWrapper<F> = (this: void, next: (this: void, ...args: RegistryArgs<F>) => RegistryResult<F>,
    ...args: RegistryArgs<F>) => RegistryResult<F>;

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
export const modName: string = contextName;

/** Which state this is. */
export const side: Side = native.side() as Side;

/**
 * Write a line to the game's log, prefixed with the mod's name. Values are converted with
 * tostring and separated by spaces.
 */
export function log(...values: unknown[]): void {
    native.log(modId, ...values);
}

/**
 * Another mod's module when that mod is installed, else undefined. The mod must be a declared
 * dependency (typically an optional one, "? other-mod"). Type it with the module's own type:
 * `optionalImport<typeof import("other-mod:api")>("other-mod:api")`.
 */
export function optionalImport<T = unknown>(specifier: string): T | undefined {
    argString(specifier, 1, "optionalImport");
    return native.require(modId, specifier, true) as T | undefined;
}

/** Wait ticks (default 1) inside a script. The outcome is the tick it resumes on. */
export function wait(ticks?: integer): Script<integer> {
    const n = ticks === undefined ? 1 : argInteger(ticks, 1, "wait");
    return scheduler.wait(n, "wait") as unknown as Script<integer>;
}

/**
 * Wait until pred holds, checking it now and then once a tick, for at most timeoutTicks ticks.
 * Outcome.Done when it held, Outcome.Timeout when the time ran out first.
 */
export function waitUntil(pred: (this: void) => boolean, timeoutTicks: integer): Script<Outcome> {
    argFunction(pred, 1, "waitUntil");
    argInteger(timeoutTicks, 2, "waitUntil");
    const rec = scheduler.current();
    if (!rec) error("waitUntil can only be used inside a script (started with startScript)", 2);
    const check = (): boolean => {
        const [status, result] = native.call(rec!.mod, pred);
        if (status !== 1) error(result, 0);
        return !!result;
    };
    if (check()) return Outcome.Done as unknown as Script<Outcome>;
    for (let i = 0; i < timeoutTicks; i++) {
        scheduler.wait(1, "waitUntil");
        if (check()) return Outcome.Done as unknown as Script<Outcome>;
    }
    return Outcome.Timeout as unknown as Script<Outcome>;
}

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
export function guard<T>(check: (this: void) => boolean, body: Script<T>): Script<T | Outcome> {
    argFunction(check, 1, "guard");
    const run = argFunction<(this: void) => unknown>(body, 2, "guard");
    const outer = coroutine.running();
    // A script's mod; in a coroutine of a mod's own (a test driving a Script), this mod.
    const mod = outer === undefined ? undefined : scheduler.modOf(outer) ?? ai.modOf(outer) ?? modId;
    if (mod === undefined) return error("guard can only be used inside a script or a coroutine", 2);
    // The body's own coroutine, counted as this script's: its waits and actions are the script's.
    // Only this call holds it, so a body dropped here is collected.
    const inner = native.spawn(mod, run);
    scheduler.adopt(outer!, inner);
    ai.adopt(outer!, inner);
    return guardStep(inner, mod, check) as Script<T | Outcome>;
}

// One step of a guarded body: check, then resume it with what the script was resumed with.
function guardStep(inner: LuaThread, mod: number, check: (this: void) => boolean, ...sent: unknown[]): unknown {
    const [status, holds] = native.call(mod, check);
    if (status !== CallStatus.Ok) return error(holds, 0);
    if (!holds) return Outcome.Interrupted;
    return guardAfter(inner, mod, check, ...(native.resume(inner, mod, ...sent) as unknown as [CallStatus, ...unknown[]]));
}

// What the step came to: a wait is the script's (its values handed out unchanged), a return is the
// guard's outcome, an error is raised again as it was.
function guardAfter(inner: LuaThread, mod: number, check: (this: void) => boolean, status: CallStatus, ...out: unknown[]): unknown {
    if (status === CallStatus.Yield) return guardStep(inner, mod, check, ...coroutine.yield(...out));
    if (status === CallStatus.Ok) return firstOf(...out);
    return raiseFirst(...out);
}

function firstOf(this: void, value?: unknown): unknown {
    return value;
}

function raiseFirst(this: void, value?: unknown): never {
    return error((value ?? "the mod is disabled") as string, 0);
}

// --- settings ---------------------------------------------------------------------------------------

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

type Common = { readonly scope: SettingScope; readonly title?: string; readonly description?: string };

/** The builders of setting declarations. */
export const setting = readonly<{
    bool(this: void, o: Common & { readonly default: boolean }): SettingSpec<boolean>;
    int(this: void, o: Common & { readonly default: integer; readonly min?: integer; readonly max?: integer }): SettingSpec<integer>;
    number(this: void, o: Common & { readonly default: number; readonly min?: number; readonly max?: number }): SettingSpec<number>;
    text(this: void, o: Common & { readonly default: string }): SettingSpec<string>;
    choice<const C extends string>(this: void, o: Common & { readonly choices: readonly C[]; readonly default: NoInfer<C> }): SettingSpec<C>;
}>({
    bool: (o: unknown) => settings.build(SettingType.Bool, o, "setting.bool"),
    int: (o: unknown) => settings.build(SettingType.Int, o, "setting.int"),
    number: (o: unknown) => settings.build(SettingType.Number, o, "setting.number"),
    text: (o: unknown) => settings.build(SettingType.String, o, "setting.text"),
    choice: (o: unknown) => settings.build(SettingType.Choice, o, "setting.choice"),
}, "setting");

/** The read-only values of a set of declared settings. */
export type Settings<T> = { readonly [K in keyof T]: T[K] extends SettingSpec<infer V> ? V : never };

/**
 * Declare the settings (sorted by name, the order a settings screen lists them in) and get an
 * object that reads their values. Writing to it is an error. Values come from the settings file
 * under the mods root, checked against the declaration; a setting is read on its scope's side.
 */
export function defineSettings<T extends Record<string, SettingSpec<unknown>>>(specs: T): Settings<T> {
    return settings.define(modId, specs) as Settings<T>;
}

/** Every setting declared in this state, all mods, in declaration order. */
export function listSettings(): SettingInfo[] {
    return native.settingsList() as SettingInfo[];
}

// --- channels ---------------------------------------------------------------------------------------

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
export type MessageArgs<A extends readonly Schema<unknown>[]> = { [I in keyof A]: Infer<A[I]> };

/** The argument types. */
export const t = readonly<{
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
    record<const F extends Record<string, Schema<unknown>>>(this: void, fields: F): Schema<{ [K in keyof F]: Infer<F[K]> }>;
    /** The value, or nothing. */
    optional<T>(this: void, inner: Schema<T>): Schema<T | undefined>;
}>({
    int: (min?: number, max?: number) => channels.int(min, max),
    number: (min?: number, max?: number) => channels.num(min, max),
    bool: () => channels.bool(),
    text: (maxLength: number) => channels.text(maxLength),
    oneOf: (values: unknown) => channels.oneOf(values),
    list: (item: unknown, maxLength: unknown) => channels.list(item, maxLength),
    record: (fields: unknown) => channels.record(fields),
    optional: (inner: unknown) => channels.optional(inner),
}, "t");

/**
 * A channel between this mod's server and client sides: requests go from a client to the server,
 * events from the server to clients. Declared once at module level; each side opens it on the
 * object its export received (`game.channel(c)`, `client.channel(c)`).
 */
export interface ChannelDeclaration<R extends Messages, E extends Messages> {
    readonly name: string;
    /** Carries the message types; never set. */
    readonly messages?: { requests: R; events: E };
}

/** Declare a channel of this mod. The same declaration is used on both sides. */
export function defineChannel<const R extends Messages = {}, const E extends Messages = {}>(
    name: string, messages: { readonly requests?: R; readonly events?: E }): ChannelDeclaration<R, E> {
    return channels.define(name, messages) as unknown as ChannelDeclaration<R, E>;
}
