// Signals in the Solid model: components run once, and only the computations that read a signal
// run again when it changes. Push-pull with three node states, so a computation never sees two
// sources from different moments (glitch-free) and runs at most once per change.
//
// Ownership: every computation, watch and cleanup belongs to the owner that was current when it
// was made. Disposing an owner disposes all of it, and a computation that re-runs first disposes
// what its previous run created.
//
// Lua: this file compiles with TypeScriptToLua. No Map/Set, no regular expressions, no
// truthiness of 0 or "" (both are true in Lua), no arrays with holes.

export type Accessor<T> = () => T;
export type Setter<T> = (value: T) => void;
type Thunk = () => unknown;

const CLEAN = 0;
const CHECK = 1;
const DIRTY = 2;

interface Source {
    observers: Computation[];
}

export class Owner {
    parent: Owner | undefined;
    children: Owner[] = [];
    cleanups: (() => void)[] = [];
    disposed = false;
    constructor(parent: Owner | undefined) {
        this.parent = parent;
        if (parent !== undefined) parent.children.push(this);
        stats.owners++;
        }
    disposeChildren(): void {
        const kids = this.children;
        this.children = [];
        for (let i = kids.length - 1; i >= 0; i--) kids[i].dispose();
        const cleanups = this.cleanups;
        this.cleanups = [];
        for (let i = cleanups.length - 1; i >= 0; i--) cleanups[i]();
    }
    dispose(): void {
        if (this.disposed) return;
        this.disposed = true;
        this.disposeChildren();
        this.detach();
        const p = this.parent;
        if (p !== undefined) {
            const idx = p.children.indexOf(this);
            if (idx >= 0) p.children.splice(idx, 1);
        }
        this.parent = undefined;
        stats.owners--;
    }
    detach(): void {}
}

class SignalNode<T> implements Source {
    value: T;
    observers: Computation[] = [];
    constructor(value: T) {
        this.value = value;
    }
}

class Computation extends Owner implements Source {
    fn: Thunk;
    value: unknown = undefined;
    state = DIRTY;
    sources: Source[] = [];
    observers: Computation[] = [];
    isEffect: boolean;
    constructor(parent: Owner | undefined, fn: Thunk, isEffect: boolean) {
        super(parent);
        this.fn = fn;
        this.isEffect = isEffect;
        stats.computations++;
    }
    override detach(): void {
        unlinkSources(this);
        this.observers = [];
        stats.computations--;
    }
}

export const stats = { owners: 0, computations: 0, watches: 0, runs: 0 };

let currentOwner: Owner | undefined = undefined;
let listener: Computation | undefined = undefined;
let batchDepth = 0;
let pendingEffects: Computation[] = [];
let flushing = false;

function unlinkSources(c: Computation): void {
    const sources = c.sources;
    for (let i = 0; i < sources.length; i++) {
        const obs = sources[i].observers;
        const idx = obs.indexOf(c);
        if (idx >= 0) obs.splice(idx, 1);
    }
    c.sources = [];
}

function track(src: Source): void {
    const l = listener;
    if (l === undefined) return;
    if (l.sources.indexOf(src) >= 0) return;
    l.sources.push(src);
    src.observers.push(l);
}

function markObservers(src: Source, state: number): void {
    const obs = src.observers;
    for (let i = 0; i < obs.length; i++) {
        const o = obs[i];
        if (o.state < state) {
            const wasClean = o.state === CLEAN;
            o.state = state;
            if (wasClean) {
                if (o.isEffect) pendingEffects.push(o);
                markObservers(o, CHECK);
            }
        }
    }
}

function run(c: Computation): void {
    c.disposeChildren();
    unlinkSources(c);
    const prevOwner = currentOwner;
    const prevListener = listener;
    currentOwner = c;
    listener = c;
    stats.runs++;
    let value: unknown;
    try {
        value = c.fn();
    } finally {
        currentOwner = prevOwner;
        listener = prevListener;
    }
    const changed = value !== c.value;
    c.value = value;
    c.state = CLEAN;
    if (changed) {
        const obs = c.observers;
        for (let i = 0; i < obs.length; i++) if (obs[i].state === CHECK) obs[i].state = DIRTY;
    }
}

function update(c: Computation): void {
    if (c.disposed) return;
    if (c.state === CHECK) {
        const sources = c.sources;
        for (let i = 0; i < sources.length; i++) {
            const s = sources[i];
            if (s instanceof Computation) {
                update(s);
                if ((c.state as number) === DIRTY) break;
            }
        }
        if (c.state === CHECK) c.state = CLEAN;
    }
    if (c.state === DIRTY) run(c);
}

function flush(): void {
    if (flushing) return;
    flushing = true;
    try {
        let guard = 0;
        while (pendingEffects.length > 0) {
            guard++;
            if (guard > 100000) throw new Error("ui: effects did not settle (a cycle of writes)");
            const queue = pendingEffects;
            pendingEffects = [];
            for (let i = 0; i < queue.length; i++) update(queue[i]);
        }
    } finally {
        flushing = false;
    }
}

/** A value that can change. Reading it inside a computation subscribes that computation. */
export function signal<T>(value: T): [Accessor<T>, Setter<T>] {
    const node = new SignalNode<T>(value);
    const read = (): T => {
        track(node);
        return node.value;
    };
    const write = (v: T): void => {
        if (v === node.value) return;
        node.value = v;
        markObservers(node, DIRTY);
        if (batchDepth === 0) flush();
    };
    return [read, write];
}

/** A derived value, recomputed lazily and only when a source changed. */
export function computed<T>(fn: () => T): Accessor<T> {
    const c = new Computation(currentOwner, fn, false);
    return (): T => {
        if (c.state !== CLEAN) update(c);
        track(c);
        return c.value as T;
    };
}

/** Runs now and again whenever something it read changed. What a run creates is disposed before the next. */
export function effect(fn: () => void): () => void {
    const c = new Computation(currentOwner, fn, true);
    if (batchDepth > 0) pendingEffects.push(c);
    else run(c);
    return () => c.dispose();
}

/** Groups writes: effects run once, after the outermost batch ends. */
export function batch<T>(fn: () => T): T {
    batchDepth++;
    let result: T;
    try {
        result = fn();
    } finally {
        batchDepth--;
    }
    if (batchDepth === 0) flush();
    return result;
}

/** Reads without subscribing. */
export function untrack<T>(fn: () => T): T {
    const prev = listener;
    listener = undefined;
    try {
        return fn();
    } finally {
        listener = prev;
    }
}

/** Registers a function run when the current owner is disposed or re-run. */
export function onCleanup(fn: () => void): void {
    if (currentOwner !== undefined) currentOwner.cleanups.push(fn);
}

/** A new owner, not tracked, disposed by the function it is handed (and by its parent). */
export function createRoot<T>(fn: (dispose: () => void) => T, parent?: Owner): T {
    const owner = new Owner(parent === undefined ? currentOwner : parent);
    const prevOwner = currentOwner;
    const prevListener = listener;
    currentOwner = owner;
    listener = undefined;
    try {
        return fn(() => owner.dispose());
    } finally {
        currentOwner = prevOwner;
        listener = prevListener;
    }
}

export function getOwner(): Owner | undefined {
    return currentOwner;
}

/** Runs fn with `owner` current, so what it creates belongs to that owner. */
export function runWithOwner<T>(owner: Owner | undefined, fn: () => T): T {
    const prevOwner = currentOwner;
    const prevListener = listener;
    currentOwner = owner;
    listener = undefined;
    try {
        return fn();
    } finally {
        currentOwner = prevOwner;
        listener = prevListener;
    }
}

// Watches: the engine has no change notifications, so a watched expression is evaluated once per
// frame() and its signal is set only when the value differs.
interface Watch {
    fn: Thunk;
    set: Setter<unknown>;
    alive: boolean;
}
let watches: Watch[] = [];

/** An expression over engine state, re-read once per frame; dependents run only on a change. */
export function watch<T>(fn: () => T): Accessor<T> {
    const [read, set] = signal<T>(untrack(fn));
    const w: Watch = { fn, set: set as Setter<unknown>, alive: true };
    watches.push(w);
    stats.watches++;
    onCleanup(() => {
        w.alive = false;
        stats.watches--;
    });
    return read;
}

/** The per-frame tick: re-reads every watch in one batch. */
export function frame(): void {
    batch(() => {
        let dead = false;
        const list = watches;
        for (let i = 0; i < list.length; i++) {
            const w = list[i];
            if (w.alive) w.set(untrack(w.fn));
            else dead = true;
        }
        if (dead) {
            const keep: Watch[] = [];
            for (let i = 0; i < watches.length; i++) if (watches[i].alive) keep.push(watches[i]);
            watches = keep;
        }
    });
}

/** Number of live watches still registered (for leak tests). */
export function watchCount(): number {
    let n = 0;
    for (let i = 0; i < watches.length; i++) if (watches[i].alive) n++;
    return n;
}

/** A prop value or an accessor for one. */
export type MaybeAccessor<T> = T | Accessor<T>;

export function read<T>(v: MaybeAccessor<T>): T {
    return typeof v === "function" ? (v as Accessor<T>)() : v;
}
