// Generated from Redivivus/Lua/core/ui/signal.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
export type Accessor<T> = () => T;
export type Setter<T> = (value: T) => void;
export declare class Owner {
    parent: Owner | undefined;
    children: Owner[];
    cleanups: (() => void)[];
    disposed: boolean;
    constructor(parent: Owner | undefined);
    disposeChildren(): void;
    dispose(): void;
    detach(): void;
}
export declare const stats: {
    owners: number;
    computations: number;
    watches: number;
    runs: number;
};
/** A value that can change. Reading it inside a computation subscribes that computation. */
export declare function signal<T>(value: T): [Accessor<T>, Setter<T>];
/** A derived value, recomputed lazily and only when a source changed. */
export declare function computed<T>(fn: () => T): Accessor<T>;
/** Runs now and again whenever something it read changed. What a run creates is disposed before the next. */
export declare function effect(fn: () => void): () => void;
/** Groups writes: effects run once, after the outermost batch ends. */
export declare function batch<T>(fn: () => T): T;
/** Reads without subscribing. */
export declare function untrack<T>(fn: () => T): T;
/** Registers a function run when the current owner is disposed or re-run. */
export declare function onCleanup(fn: () => void): void;
/** A new owner, not tracked, disposed by the function it is handed (and by its parent). */
export declare function createRoot<T>(fn: (dispose: () => void) => T, parent?: Owner): T;
export declare function getOwner(): Owner | undefined;
/** Runs fn with `owner` current, so what it creates belongs to that owner. */
export declare function runWithOwner<T>(owner: Owner | undefined, fn: () => T): T;
/** An expression over engine state, re-read once per frame; dependents run only on a change. */
export declare function watch<T>(fn: () => T): Accessor<T>;
/** The per-frame tick: re-reads every watch in one batch. */
export declare function frame(): void;
/** Number of live watches still registered (for leak tests). */
export declare function watchCount(): number;
/** A prop value or an accessor for one. */
export type MaybeAccessor<T> = T | Accessor<T>;
export declare function read<T>(v: MaybeAccessor<T>): T;
