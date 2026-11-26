// Generated from Redivivus/Lua/core/ui/widget.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
import { Align, Justify, SizeMode, UIEventType } from "../redivivus/enums";
import { type Owner } from "./signal";
export { Align, Justify, SizeMode };
/** A size on one axis: pixels, the content's size, or a share of the free space. */
export type Size = number | SizeMode;
/** Absolute placement against the parent's content box; any edge left out is free. */
export interface Anchor {
    left?: number;
    top?: number;
    right?: number;
    bottom?: number;
    /** Centre on the axes that have neither edge. */
    center?: boolean;
}
/** Bumped by every prop write that can change layout; a root lays out again when it moved. */
export declare function invalidate(): void;
export declare function currentLayoutVersion(): number;
export type Props = {
    [key: string]: unknown;
};
export type Handler = (ev: UIEvent) => void;
export declare class Widget {
    id: number;
    kind: string;
    props: Props;
    handlers: {
        [name: string]: Handler;
    };
    children: Widget[];
    parent: Widget | undefined;
    owner: Owner | undefined;
    x: number;
    y: number;
    w: number;
    h: number;
    mw: number;
    mh: number;
    scrollX: number;
    scrollY: number;
    contentH: number;
    /** Per-kind state kept across frames (laid-out text, rows of a list, tab rectangles...). */
    state: {
        [key: string]: unknown;
    };
    constructor(kind: string);
    set(key: string, value: unknown): void;
    num(key: string, fallback: number): number;
    str(key: string, fallback: string): string;
    bool(key: string): boolean;
    /** Scrolls the content (scroll pane, list); layout clamps it to the content. */
    scrollTo(y: number): void;
    append(child: Widget): void;
    setChildren(children: Widget[]): void;
}
export interface UIEvent {
    type: UIEventType;
    x: number;
    y: number;
    button: number;
    delta: number;
    key: number;
    text: string;
    shift: boolean;
    ctrl: boolean;
    /** The deepest widget the event was aimed at. */
    target: Widget | undefined;
    /** The widget whose handler is running. */
    current: Widget | undefined;
    stopped: boolean;
    handled: boolean;
}
/** Padding as top, right, bottom, left. */
export declare function paddingOf(w: Widget): [number, number, number, number];
/** Children that take part in layout: regions are opened up, hidden widgets left out. */
export declare function layoutChildren(w: Widget, out: Widget[]): Widget[];
/** True when `a` is `b` or inside it. */
export declare function within(a: Widget | undefined, b: Widget): boolean;
