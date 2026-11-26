// Input: abstract events in, routed to widgets. Pointer events go to the deepest widget under
// the pointer (respecting z order, clipping and `hit={false}`), or to the widget holding the
// pointer since a press. Keys and text go to the focused widget. Every event bubbles from its
// target to the root: at each widget the kind's own behaviour runs, then the user's handler
// (onClick, onMouseDown, ...), until one stops it.

import { UIEventType } from "../redivivus/enums";
import { inner, kinds } from "./kinds";
import { clips, zOrdered } from "./draw";
import { batch } from "./signal";
import type { Context } from "./theme";
import { within, type UIEvent, type Widget } from "./widget";

/** An input event as the host delivers it. Pointer events carry x, y; wheel a delta (lines, down positive). */
export interface InputEvent {
    type: UIEventType;
    x?: number;
    y?: number;
    button?: number;
    delta?: number;
    key?: number;
    text?: string;
    shift?: boolean;
    ctrl?: boolean;
}

const KEY_TAB = 9;

const HANDLER: { [type: string]: string } = {
    click: "onClick",
    mousedown: "onMouseDown",
    mouseup: "onMouseUp",
    mousemove: "onMouseMove",
    wheel: "onWheel",
    keydown: "onKeyDown",
    keyup: "onKeyUp",
    text: "onText",
};

function hitWidget(ctx: Context, w: Widget, x: number, y: number): Widget | undefined {
    if (w.kind === "region") {
        const kids = zOrdered(w);
        for (let i = kids.length - 1; i >= 0; i--) {
            const r = hitWidget(ctx, kids[i], x, y);
            if (r !== undefined) return r;
        }
        return undefined;
    }
    if (w.props.visible === false) return undefined;
    const inside = x >= w.x && y >= w.y && x < w.x + w.w && y < w.y + w.h;
    let childrenReachable = true;
    if (clips(w)) {
        const b = inner(ctx, w);
        childrenReachable = x >= b[0] && y >= b[1] && x < b[0] + b[2] && y < b[1] + b[3];
    }
    if (childrenReachable) {
        const kids = zOrdered(w);
        for (let i = kids.length - 1; i >= 0; i--) {
            const r = hitWidget(ctx, kids[i], x, y);
            if (r !== undefined) return r;
        }
    }
    if (inside && w.props.hit !== false) return w;
    return undefined;
}

/** The deepest widget at (x, y), or undefined. */
export function hitTest(ctx: Context, root: Widget, x: number, y: number): Widget | undefined {
    return hitWidget(ctx, root, x, y);
}

function makeEvent(src: InputEvent, type: UIEventType): UIEvent {
    return {
        type,
        x: src.x ?? 0,
        y: src.y ?? 0,
        button: src.button ?? 0,
        delta: src.delta ?? 0,
        key: src.key ?? 0,
        text: src.text ?? "",
        shift: src.shift === true,
        ctrl: src.ctrl === true,
        target: undefined,
        current: undefined,
        stopped: false,
        handled: false,
    };
}

/** Bubbles ev from target to the root. */
export function bubble(ctx: Context, ev: UIEvent, target: Widget | undefined): void {
    ev.target = target;
    let n = target;
    const name = HANDLER[ev.type];
    while (n !== undefined) {
        ev.current = n;
        const k = kinds[n.kind];
        if (k !== undefined && k.event !== undefined && !k.event(ctx, n, ev)) return;
        const h = name === undefined ? undefined : n.props[name];
        if (typeof h === "function") {
            (h as (e: UIEvent) => void)(ev);
            ev.handled = true;
        }
        if (ev.stopped) return;
        n = n.parent;
    }
}

function focusableOf(w: Widget | undefined): Widget | undefined {
    let n = w;
    while (n !== undefined) {
        const k = kinds[n.kind];
        if ((k !== undefined && k.focusable === true && !n.bool("disabled")) || n.bool("focusable")) return n;
        n = n.parent;
    }
    return undefined;
}

function collectFocusable(w: Widget, out: Widget[]): void {
    if (w.props.visible === false) return;
    const k = kinds[w.kind];
    if (w.kind !== "region" && ((k !== undefined && k.focusable === true && !w.bool("disabled")) || w.bool("focusable"))) out.push(w);
    for (let i = 0; i < w.children.length; i++) collectFocusable(w.children[i], out);
}

/** Routes one host event. Returns true when a widget handled it (the host then skips its own handlers). */
export function dispatch(ctx: Context, root: Widget, src: InputEvent): boolean {
    let handled = false;
    batch(() => {
        if (src.type === "mousemove" || src.type === "mousedown" || src.type === "mouseup" || src.type === "wheel") {
            const x = src.x ?? ctx.pointerX;
            const y = src.y ?? ctx.pointerY;
            ctx.pointerX = x;
            ctx.pointerY = y;
            const hit = hitTest(ctx, root, x, y);
            ctx.hover = hit;
            const ev = makeEvent(src, src.type);
            ev.x = x;
            ev.y = y;
            if (src.type === "mousedown") {
                ctx.capture = hit;
                ctx.focus = focusableOf(hit);
                bubble(ctx, ev, hit);
            } else if (src.type === "mouseup") {
                const held = ctx.capture;
                bubble(ctx, ev, held !== undefined ? held : hit);
                ctx.capture = undefined;
                if (held !== undefined && within(hit, held)) {
                    const click = makeEvent(src, UIEventType.Click);
                    click.x = x;
                    click.y = y;
                    bubble(ctx, click, held);
                    if (click.handled) handled = true;
                }
            } else {
                bubble(ctx, ev, ctx.capture !== undefined ? ctx.capture : hit);
            }
            if (ev.handled || hit !== undefined) handled = true;
        } else {
            const ev = makeEvent(src, src.type);
            bubble(ctx, ev, ctx.focus !== undefined ? ctx.focus : root);
            if (!ev.handled && src.type === "keydown" && src.key === KEY_TAB) {
                const list: Widget[] = [];
                collectFocusable(root, list);
                if (list.length > 0) {
                    let idx = ctx.focus === undefined ? -1 : list.indexOf(ctx.focus);
                    idx = src.shift === true ? (idx <= 0 ? list.length - 1 : idx - 1) : (idx + 1) % list.length;
                    ctx.focus = list[idx];
                    ev.handled = true;
                }
            }
            handled = ev.handled;
        }
    });
    return handled;
}
