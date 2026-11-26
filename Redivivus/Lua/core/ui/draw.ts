// Drawing: one ordered pass over the laid-out tree, emitting backend calls. Siblings draw in
// z order (stable: equal z keeps tree order), a widget with `clip` (and every scroll pane and
// list) scissors its children, and children wholly outside the scissor are skipped. Tooltips are
// drawn last, above everything.

import { FontRole, TextColour } from "../redivivus/enums";
import { kinds, inner } from "./kinds";
import type { Context } from "./theme";
import { drawLines, layoutText, linesHeight, linesWidth } from "./text";
import { within, type Widget } from "./widget";

/** Children in z order; a stable insertion sort, since table.sort is not stable. */
export function zOrdered(w: Widget): Widget[] {
    const kids = w.children;
    let sorted = true;
    for (let i = 1; i < kids.length; i++) if (kids[i].num("z", 0) < kids[i - 1].num("z", 0)) sorted = false;
    if (sorted) return kids;
    const out: Widget[] = [];
    for (let i = 0; i < kids.length; i++) {
        const k = kids[i];
        const z = k.num("z", 0);
        let j = out.length;
        while (j > 0 && out[j - 1].num("z", 0) > z) j--;
        out.splice(j, 0, k);
    }
    return out;
}

export function clips(w: Widget): boolean {
    return w.bool("clip") || w.kind === "scrollPane" || w.kind === "list";
}

interface Clip {
    x0: number;
    y0: number;
    x1: number;
    y1: number;
}

function drawWidget(ctx: Context, w: Widget, clip: Clip, tooltips: Widget[]): void {
    if (w.kind === "region") {
        const kids = zOrdered(w);
        for (let i = 0; i < kids.length; i++) drawWidget(ctx, kids[i], clip, tooltips);
        return;
    }
    if (w.props.visible === false) return;
    if (w.x >= clip.x1 || w.y >= clip.y1 || w.x + w.w <= clip.x0 || w.y + w.h <= clip.y0) return;
    const k = kinds[w.kind];
    if (k !== undefined && k.paint !== undefined) k.paint(ctx, w);
    const kids = zOrdered(w);
    if (kids.length > 0) {
        if (clips(w)) {
            const b = inner(ctx, w);
            ctx.backend.pushScissor(b[0], b[1], b[2], b[3]);
            const c: Clip = { x0: Math.max(clip.x0, b[0]), y0: Math.max(clip.y0, b[1]), x1: Math.min(clip.x1, b[0] + b[2]), y1: Math.min(clip.y1, b[1] + b[3]) };
            for (let i = 0; i < kids.length; i++) drawWidget(ctx, kids[i], c, tooltips);
            ctx.backend.popScissor();
        } else {
            for (let i = 0; i < kids.length; i++) drawWidget(ctx, kids[i], clip, tooltips);
        }
    }
    if (k !== undefined && k.paintAfter !== undefined) k.paintAfter(ctx, w);
    if (w.kind === "tooltip" && ctx.hover !== undefined && within(ctx.hover, w) && w.str("text", "").length > 0) tooltips.push(w);
}

const TIP_PAD = 4;
const TIP_WIDTH = 260;

function drawTooltip(ctx: Context, w: Widget): void {
    const font = ctx.theme.font(FontRole.Small);
    const lines = layoutText(ctx.backend, font, w.str("text", ""), ctx.theme.colour(TextColour.Text), TIP_WIDTH);
    const tw = linesWidth(lines) + TIP_PAD * 2;
    const th = linesHeight(lines) + TIP_PAD * 2;
    let x = ctx.pointerX + 12;
    let y = ctx.pointerY + 16;
    if (x + tw > ctx.width) x = ctx.width - tw;
    if (y + th > ctx.height) y = ctx.pointerY - th - 4;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    ctx.theme.paintTooltip(ctx.backend, x, y, tw, th);
    drawLines(ctx.backend, lines, x + TIP_PAD, y + TIP_PAD, tw - TIP_PAD * 2, 1);
}

/** Draws one frame of the tree through the context's backend. */
export function drawTree(ctx: Context, root: Widget): void {
    ctx.backend.beginFrame(ctx.width, ctx.height);
    const tooltips: Widget[] = [];
    drawWidget(ctx, root, { x0: 0, y0: 0, x1: ctx.width, y1: ctx.height }, tooltips);
    for (let i = 0; i < tooltips.length; i++) drawTooltip(ctx, tooltips[i]);
    ctx.backend.endFrame();
}
