// Generated from Redivivus/Lua/core/ui/control.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
import type { Child } from "./jsx";
import { type Accessor, type MaybeAccessor } from "./signal";
export declare function Show<T>(props: {
    when: MaybeAccessor<T | undefined | null | false>;
    keyed?: boolean;
    fallback?: Child | (() => Child);
    children?: Child | ((value: T) => Child);
}): Child;
/** A list keyed by item (or by `key(item)`): an item that stays keeps its widgets and state. */
export declare function For<T>(props: {
    each: MaybeAccessor<T[] | undefined>;
    key?: (item: T) => unknown;
    fallback?: Child;
    children?: (item: T, index: Accessor<number>) => Child;
}): Child;
/** What Match gives Switch: not a widget. */
export interface MatchCase {
    isMatch: true;
    matchWhen: MaybeAccessor<unknown>;
    matchChildren: unknown;
}
export declare function Match<T>(props: {
    when: MaybeAccessor<T | undefined | null | false>;
    children?: Child | ((value: T) => Child);
}): Child;
/** The first Match whose `when` is shown, else the fallback. */
export declare function Switch(props: {
    fallback?: Child | (() => Child);
    children?: unknown;
}): Child;
