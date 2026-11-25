/**
 * The API's own types, shared by every side. Nothing here is a value: the binding generator reads
 * this file for types only.
 */

/** A Lua number that the host checks is a whole number. */
type integer = number;

/** Fields every event payload carries. */
interface EventBase {
    /** Which event it is. */
    readonly name: Event;
    /** The tick it was raised on. */
    readonly tick: integer;
}

/**
 * Any handle: an engine object named by class and id (handle classes are declared with @handle).
 * It is valid while its object exists.
 */
type Handle = { readonly valid: boolean };

/** What a script yields: nothing (resume next tick), a tick count, or an event to wait for. */
type ScriptWait = { readonly ticks: integer } | { readonly event: Event };

/**
 * A script or an action a script runs. Declare script functions as `function*` returning
 * `Script<T>`: the mod build compiles them to plain Lua functions that yield on the script's
 * coroutine, so `yield` waits for the next tick and `yield* action()` runs an action and gives
 * its outcome. A `Script` function is only called through `yield*` or handed to `script.start`.
 */
interface Script<T = void> extends Generator<ScriptWait | undefined, T, any> {}

/** The arguments of a registry entry's signature. */
type RegistryArgs<F> = F extends (this: void, ...args: infer A) => any ? A : never;
/** The result of a registry entry's signature. */
type RegistryResult<F> = F extends (this: void, ...args: any[]) => infer R ? R : never;
/** A wrapper around a registry entry: `next` runs the entry as it was before this wrapper. */
type RegistryWrapper<F> = (this: void, next: (this: void, ...args: RegistryArgs<F>) => RegistryResult<F>,
    ...args: RegistryArgs<F>) => RegistryResult<F>;

/**
 * A registry of entries keyed by id. Ids are members of a string enum and the signatures are the
 * open interface M, keyed by them: a mod adds an id by declaration merging
 * (`declare global { const enum ExampleId { X = "my-mod:x" } interface ExampleRegistry { [ExampleId.X]: ... } }`).
 * @noSelf
 */
interface KeyedRegistry<M> {
    /** Add the entry, or replace its implementation (wrappers stay). */
    register<K extends keyof M & string>(id: K, impl: M[K]): void;
    /** Wrap the entry. Later mods wrap earlier ones; a wrapper that errors is removed. */
    wrap<K extends keyof M & string>(id: K, wrapper: RegistryWrapper<M[K]>): void;
    /** Run the entry with its wrappers. */
    call<K extends keyof M & string>(id: K, ...args: RegistryArgs<M[K]>): RegistryResult<M[K]>;
    /** Whether the entry exists and has an implementation. */
    has(id: keyof M & string): boolean;
}

/**
 * A registry with a single entry of signature F.
 * @noSelf
 */
interface SlotRegistry<F> {
    /** Replace the implementation (wrappers stay). */
    set(impl: F): void;
    /** Wrap the entry. */
    wrap(wrapper: RegistryWrapper<F>): void;
    /** Run the entry with its wrappers. */
    call(...args: RegistryArgs<F>): RegistryResult<F>;
}
