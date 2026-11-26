// Generated from Redivivus/Lua/core/ui/kinds.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
import { FontRole, TextColour } from "../redivivus/enums";
import type { FontHandle } from "./backend";
import type { Context } from "./theme";
import { type UIEvent, type Widget } from "./widget";
export interface Kind {
    /** Content size (inside padding) for an inner width; availW < 0 means unbounded. */
    measure?(this: void, ctx: Context, w: Widget, availW: number): [number, number];
    /** After the box is set: lay out what depends on the final size. */
    arrange?(this: void, ctx: Context, w: Widget): void;
    /** Before the children. */
    paint?(this: void, ctx: Context, w: Widget): void;
    /** After the children (scrollbars). */
    paintAfter?(this: void, ctx: Context, w: Widget): void;
    /** Built-in reaction while an event bubbles through; false cancels the user's handler and bubbling. */
    event?(this: void, ctx: Context, w: Widget, ev: UIEvent): boolean;
    focusable?: boolean;
    /** How a container lays out its children: row, column, stack (all on top of each other), grid, scroll, list. */
    layout?: string;
    /** The axis a "fill"-less spacer fills, decided by the parent; true for the spacer kind. */
    spacer?: boolean;
    /** Sized by its art: not stretched by a parent's align (button, checkbox, sprite, slot, tabs, image). */
    rigid?: boolean;
}
export declare const kinds: {
    [kind: string]: Kind;
};
export declare function alignOf(w: Widget): number;
export declare function fontOf(ctx: Context, w: Widget, role: FontRole): FontHandle;
export declare function colourOf(ctx: Context, w: Widget, role: TextColour): number;
/** The inner box: x, y, w, h inside padding and frame inset. */
export declare function inner(ctx: Context, w: Widget): [number, number, number, number];
/** A paragraph of spans in several fonts and colours, wrapped as one (Markdown text). */
export interface RichSpan {
    text: string;
    font: FontRole;
    colour: TextColour | number;
}
export interface ListRow {
    index: number;
    item: unknown;
    widget: Widget;
    dispose: () => void;
}
