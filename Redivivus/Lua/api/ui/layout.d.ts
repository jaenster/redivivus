// Generated from Redivivus/Lua/core/ui/layout.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
import type { Context } from "./theme";
import { Widget } from "./widget";
/** Sets w.mw and w.mh: the size w would like given availW (-1: unbounded). */
export declare function measure(ctx: Context, w: Widget, availW: number): void;
/** Gives w its box and lays out its children inside it. */
export declare function arrange(ctx: Context, w: Widget, x: number, y: number, width: number, height: number): void;
/**
 * Lays the whole tree out for a screen of the context's size. The root takes a fixed width or
 * height it asks for, its content size for "content", and the screen otherwise.
 */
export declare function layoutRoot(ctx: Context, root: Widget): void;
