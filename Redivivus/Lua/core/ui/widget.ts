// The retained widget tree. A widget is a kind, its current prop values and its children; the
// layout pass writes its box (x, y, w, h, absolute screen pixels). What a kind measures, how it
// arranges children, how it draws and how it reacts to input is in kinds.ts.

import { Align, Justify, SizeMode, UIEventType } from "../redivivus/enums";
import { getOwner, type Owner } from "./signal";

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

let nextId = 1;
let layoutVersion = 1;

/** Bumped by every prop write that can change layout; a root lays out again when it moved. */
export function invalidate(): void {
    layoutVersion++;
}
export function currentLayoutVersion(): number {
    return layoutVersion;
}

export type Props = { [key: string]: unknown };
export type Handler = (ev: UIEvent) => void;

export class Widget {
    id: number;
    kind: string;
    props: Props = {};
    handlers: { [name: string]: Handler } = {};
    children: Widget[] = [];
    parent: Widget | undefined = undefined;
    owner: Owner | undefined;
    // Box after layout.
    x = 0;
    y = 0;
    w = 0;
    h = 0;
    // Measured preferred size (for the width it was measured at).
    mw = 0;
    mh = 0;
    // Scroll offset of the content (scrollPane, list).
    scrollX = 0;
    scrollY = 0;
    contentH = 0;
    /** Per-kind state kept across frames (laid-out text, rows of a list, tab rectangles...). */
    state: { [key: string]: unknown } = {};
    constructor(kind: string) {
        this.id = nextId++;
        this.kind = kind;
        this.owner = getOwner();
    }
    set(key: string, value: unknown): void {
        if (this.props[key] === value) return;
        this.props[key] = value;
        invalidate();
    }
    num(key: string, fallback: number): number {
        const v = this.props[key];
        return typeof v === "number" ? v : fallback;
    }
    str(key: string, fallback: string): string {
        const v = this.props[key];
        return typeof v === "string" ? v : fallback;
    }
    bool(key: string): boolean {
        return this.props[key] === true;
    }
    /** Scrolls the content (scroll pane, list); layout clamps it to the content. */
    scrollTo(y: number): void {
        if (y === this.scrollY) return;
        this.scrollY = y;
        invalidate();
    }
    append(child: Widget): void {
        child.parent = this;
        this.children.push(child);
        invalidate();
    }
    setChildren(children: Widget[]): void {
        for (let i = 0; i < this.children.length; i++) {
            const c = this.children[i];
            if (c.parent === this) c.parent = undefined;
        }
        for (let i = 0; i < children.length; i++) children[i].parent = this;
        this.children = children;
        invalidate();
    }
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
export function paddingOf(w: Widget): [number, number, number, number] {
    const p = w.props.padding;
    if (typeof p === "number") return [p, p, p, p];
    if (p !== undefined && typeof p === "object") {
        const a = p as number[];
        if (a.length === 2) return [a[0], a[1], a[0], a[1]];
        if (a.length === 4) return [a[0], a[1], a[2], a[3]];
    }
    return [0, 0, 0, 0];
}

/** Children that take part in layout: regions are opened up, hidden widgets left out. */
export function layoutChildren(w: Widget, out: Widget[]): Widget[] {
    const kids = w.children;
    for (let i = 0; i < kids.length; i++) {
        const k = kids[i];
        if (k.kind === "region") layoutChildren(k, out);
        else if (k.props.visible !== false) out.push(k);
    }
    return out;
}

/** True when `a` is `b` or inside it. */
export function within(a: Widget | undefined, b: Widget): boolean {
    let n = a;
    while (n !== undefined) {
        if (n === b) return true;
        n = n.parent;
    }
    return false;
}
