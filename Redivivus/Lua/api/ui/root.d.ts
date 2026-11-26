// Generated from Redivivus/Lua/core/ui/root.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
import type { Backend } from "./backend";
import { type InputEvent } from "./input";
import { type Child } from "./jsx";
import type { Context, Theme } from "./theme";
import { Widget } from "./widget";
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
export declare function mount(view: () => Child, backend: Backend, theme: Theme, width: number, height: number): UIRoot;
