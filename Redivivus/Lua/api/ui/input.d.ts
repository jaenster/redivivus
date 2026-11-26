// Generated from Redivivus/Lua/core/ui/input.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
import { UIEventType } from "../redivivus/enums";
import type { Context } from "./theme";
import { type UIEvent, type Widget } from "./widget";
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
/** The deepest widget at (x, y), or undefined. */
export declare function hitTest(ctx: Context, root: Widget, x: number, y: number): Widget | undefined;
/** Bubbles ev from target to the root. */
export declare function bubble(ctx: Context, ev: UIEvent, target: Widget | undefined): void;
/** Routes one host event. Returns true when a widget handled it (the host then skips its own handlers). */
export declare function dispatch(ctx: Context, root: Widget, src: InputEvent): boolean;
