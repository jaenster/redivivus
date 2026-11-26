// Control flow: Show, For (keyed), Switch/Match. Each returns a function, which the factory turns
// into a region. A child given as a function is created lazily and disposed when it goes away; a
// child given as an element was created by the caller and is only attached and detached.
//
// "Shown" means not undefined, null or false. 0 and "" count as shown (they are true in Lua, and
// the rule has to be the same in both languages).

import type { Child } from "./jsx";
import { collect } from "./jsx";
import { computed, createRoot, getOwner, onCleanup, read, signal, untrack, type Accessor, type MaybeAccessor, type Setter } from "./signal";
import type { Widget } from "./widget";

function present(v: unknown): boolean {
    return v !== undefined && v !== null && v !== false;
}

function resolve(c: unknown, arg?: unknown): Child {
    if (typeof c === "function") return (c as (a: unknown) => Child)(arg);
    return c as Child;
}

export function Show<T>(props: { when: MaybeAccessor<T | undefined | null | false>; keyed?: boolean; fallback?: Child | (() => Child); children?: Child | ((value: T) => Child) }): Child {
    const value = computed(() => {
        const v = read(props.when);
        return present(v) ? v : undefined;
    });
    // Re-create the children only when the value goes between shown and hidden; with `keyed`, on
    // every new value.
    const keyed = props.keyed === true;
    const shown = computed(() => (keyed ? value() : present(value()) ? true : undefined));
    return () => {
        const s = shown();
        if (s === undefined) return resolve(props.fallback);
        return resolve(props.children, untrack(value));
    };
}

interface Entry {
    nodes: Widget[];
    dispose: () => void;
    setIndex: Setter<number>;
}

/** A list keyed by item (or by `key(item)`): an item that stays keeps its widgets and state. */
export function For<T>(props: { each: MaybeAccessor<T[] | undefined>; key?: (item: T) => unknown; fallback?: Child; children?: (item: T, index: Accessor<number>) => Child }): Child {
    const owner = getOwner();
    let cache = new Map<unknown, Entry>();
    onCleanup(() => {
        cache.forEach((e) => e.dispose());
        cache = new Map<unknown, Entry>();
    });
    return () => {
        const list = read(props.each) ?? [];
        const next = new Map<unknown, Entry>();
        const out: Widget[] = [];
        const extra: Entry[] = [];
        for (let i = 0; i < list.length; i++) {
            const item = list[i];
            const k = props.key !== undefined ? props.key(item) : item;
            let e = cache.get(k);
            if (e !== undefined) {
                cache.delete(k);
                e.setIndex(i);
            } else {
                const idx = i;
                e = untrack(() =>
                    createRoot((dispose) => {
                        const [index, setIndex] = signal(idx);
                        const nodes: Widget[] = [];
                        if (props.children !== undefined) collect(props.children(item, index), nodes);
                        return { nodes, dispose, setIndex };
                    }, owner),
                );
            }
            if (next.has(k)) extra.push(e);
            else next.set(k, e);
            for (let j = 0; j < e.nodes.length; j++) out.push(e.nodes[j]);
        }
        cache.forEach((e) => e.dispose());
        for (let i = 0; i < extra.length; i++) next.set({}, extra[i]);
        cache = next;
        if (list.length === 0 && props.fallback !== undefined) return props.fallback;
        return out;
    };
}

/** What Match gives Switch: not a widget. */
export interface MatchCase {
    isMatch: true;
    matchWhen: MaybeAccessor<unknown>;
    matchChildren: unknown;
}

export function Match<T>(props: { when: MaybeAccessor<T | undefined | null | false>; children?: Child | ((value: T) => Child) }): Child {
    const c: MatchCase = { isMatch: true, matchWhen: props.when, matchChildren: props.children };
    return c as unknown as Child;
}

/** The first Match whose `when` is shown, else the fallback. */
export function Switch(props: { fallback?: Child | (() => Child); children?: unknown }): Child {
    const raw = props.children;
    const cases: MatchCase[] = [];
    const gather = (v: unknown): void => {
        if (Array.isArray(v)) for (let i = 0; i < v.length; i++) gather(v[i]);
        else if (v !== undefined && v !== null && typeof v === "object" && (v as MatchCase).isMatch === true) cases.push(v as MatchCase);
    };
    gather(raw);
    const chosen = computed(() => {
        for (let i = 0; i < cases.length; i++) if (present(read(cases[i].matchWhen))) return i;
        return -1;
    });
    return () => {
        const i = chosen();
        if (i < 0) return resolve(props.fallback);
        return resolve(cases[i].matchChildren, untrack(() => read(cases[i].matchWhen)));
    };
}
