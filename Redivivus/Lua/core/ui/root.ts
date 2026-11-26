// A mounted UI: a component tree under one owner, laid out for a screen size and drawn through a
// backend with a theme. The host calls frame() once per frame and dispatch() for each input event.

import type { Backend } from "./backend";
import { drawTree } from "./draw";
import { dispatch, type InputEvent } from "./input";
import { toWidget, type Child } from "./jsx";
import { layoutRoot } from "./layout";
import { createRoot, frame as tickWatches } from "./signal";
import type { Context, Theme } from "./theme";
import { currentLayoutVersion, Widget } from "./widget";

export interface UIRoot {
    readonly root: Widget;
    readonly ctx: Context;
    /** One frame: re-read watches, lay out when something changed, draw. */
    frame(): void;
    /** Lays out (when needed) and draws, without re-reading watches. */
    draw(): void;
    /** Lays out now if anything changed. */
    layout(): void;
    /** Routes a host input event; true when the UI handled it. */
    dispatch(ev: InputEvent): boolean;
    resize(width: number, height: number): void;
    /** Disposes every computation, watch and widget the tree created. */
    dispose(): void;
}

export function mount(view: () => Child, backend: Backend, theme: Theme, width: number, height: number): UIRoot {
    theme.load(backend);
    const ctx: Context = { backend, theme, width, height, hover: undefined, capture: undefined, focus: undefined, pointerX: -1, pointerY: -1, tick: 0 };
    let disposeAll: () => void = () => {};
    let root: Widget = new Widget("stack");
    createRoot((dispose) => {
        disposeAll = dispose;
        root = toWidget(view());
    });
    let laidOut = -1;
    let laidW = -1;
    let laidH = -1;
    const layout = (): void => {
        if (laidOut === currentLayoutVersion() && laidW === ctx.width && laidH === ctx.height) return;
        layoutRoot(ctx, root);
        laidOut = currentLayoutVersion();
        laidW = ctx.width;
        laidH = ctx.height;
    };
    const draw = (): void => {
        layout();
        drawTree(ctx, root);
        ctx.tick++;
    };
    return {
        root,
        ctx,
        frame() {
            tickWatches();
            draw();
        },
        draw() {
            draw();
        },
        layout() {
            layout();
        },
        dispatch(ev) {
            layout();
            const handled = dispatch(ctx, root, ev);
            return handled;
        },
        resize(w, h) {
            ctx.width = w;
            ctx.height = h;
        },
        dispose() {
            disposeAll();
            ctx.hover = undefined;
            ctx.capture = undefined;
            ctx.focus = undefined;
        },
    };
}
