// Generated from Redivivus/Lua/core/ui/draw.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
import type { Context } from "./theme";
import { type Widget } from "./widget";
/** Children in z order; a stable insertion sort, since table.sort is not stable. */
export declare function zOrdered(w: Widget): Widget[];
export declare function clips(w: Widget): boolean;
/** Draws one frame of the tree through the context's backend. */
export declare function drawTree(ctx: Context, root: Widget): void;
