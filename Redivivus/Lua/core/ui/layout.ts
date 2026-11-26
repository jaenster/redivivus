// Integer layout. Two passes per widget: measure (preferred size for an available width) and
// arrange (the final box, then the children). Rows and columns size children as a number of
// pixels, their content, or a share of the free space ("fill", weighted by `flex`), clamped to
// min/max. Free space is shared by floor and the remainder handed out a pixel at a time from the
// first child, so the result is the same on every machine and in both languages.

import { BoxStyle } from "../redivivus/enums";
import { kinds, inner, type ListRow } from "./kinds";
import { createRoot, runWithOwner, untrack } from "./signal";
import type { Context } from "./theme";
import { layoutChildren, paddingOf, Widget, type Anchor } from "./widget";

type Axis = 0 | 1;

function sizeProp(w: Widget, axis: Axis): unknown {
    return axis === 0 ? w.props.width : w.props.height;
}

function isFill(w: Widget, axis: Axis, parentMain: Axis | -1): boolean {
    const s = sizeProp(w, axis);
    if (s === "fill") return true;
    if (s === undefined && parentMain === axis) {
        const k = kinds[w.kind];
        if (k !== undefined && k.spacer === true) return true;
    }
    return false;
}

/** A widget whose size comes from its art keeps it when its parent stretches (unless it asks for "fill"). */
function rigid(w: Widget, axis: Axis): boolean {
    const k = kinds[w.kind];
    return k !== undefined && k.rigid === true && sizeProp(w, axis) === undefined;
}

function flexOf(w: Widget): number {
    const f = w.props.flex;
    return typeof f === "number" && f > 0 ? f : 1;
}

function clampAxis(w: Widget, axis: Axis, v: number): number {
    const lo = axis === 0 ? w.props.minWidth : w.props.minHeight;
    const hi = axis === 0 ? w.props.maxWidth : w.props.maxHeight;
    let r = v;
    if (typeof hi === "number" && r > hi) r = hi;
    if (typeof lo === "number" && r < lo) r = lo;
    return r < 0 ? 0 : r;
}

function insetOf(ctx: Context, w: Widget): number {
    return w.kind === "frame" ? ctx.theme.frameInset(w.str("style", BoxStyle.None) as BoxStyle) : 0;
}

function anchored(w: Widget): boolean {
    return w.props.anchor !== undefined;
}

function flowKids(w: Widget): Widget[] {
    const all = layoutChildren(w, []);
    const out: Widget[] = [];
    for (let i = 0; i < all.length; i++) if (!anchored(all[i])) out.push(all[i]);
    return out;
}

function gapOf(w: Widget): number {
    return w.num("gap", 0);
}

function modeOf(w: Widget): string {
    const d = w.props.direction;
    if (typeof d === "string" && (w.kind === "flow" || w.kind === "frame")) return d;
    const k = kinds[w.kind];
    return k !== undefined && k.layout !== undefined ? k.layout : "";
}

// --- measure --------------------------------------------------------------------------------------

/** Sets w.mw and w.mh: the size w would like given availW (-1: unbounded). */
export function measure(ctx: Context, w: Widget, availW: number): void {
    const p = paddingOf(w);
    const inset = insetOf(ctx, w);
    const padW = p[1] + p[3] + inset * 2;
    const padH = p[0] + p[2] + inset * 2;
    const fw = w.props.width;
    const fh = w.props.height;
    let innerAvail = -1;
    if (typeof fw === "number") innerAvail = Math.max(0, clampAxis(w, 0, fw) - padW);
    else if (availW >= 0) innerAvail = Math.max(0, clampAxis(w, 0, availW) - padW);
    const c = content(ctx, w, innerAvail);
    w.mw = clampAxis(w, 0, typeof fw === "number" ? fw : c[0] + padW);
    w.mh = clampAxis(w, 1, typeof fh === "number" ? fh : c[1] + padH);
}

function content(ctx: Context, w: Widget, avail: number): [number, number] {
    const k = kinds[w.kind];
    if (k !== undefined && k.measure !== undefined) return k.measure(ctx, w, avail);
    const mode = modeOf(w);
    if (mode === "row") return w.bool("wrap") ? measureWrap(ctx, w, avail) : measureRow(ctx, w, avail);
    if (mode === "column" || mode === "scroll") return measureColumn(ctx, w, avail);
    if (mode === "stack") return measureStack(ctx, w, avail);
    if (mode === "grid") return measureGrid(ctx, w, avail);
    if (mode === "list") {
        const items = (w.props.items as unknown[] | undefined) ?? [];
        return [avail >= 0 ? avail : 0, items.length * w.num("rowHeight", 20)];
    }
    return [0, 0];
}

function measureRow(ctx: Context, w: Widget, avail: number): [number, number] {
    const kids = flowKids(w);
    const gap = gapOf(w);
    let total = 0;
    let tall = 0;
    for (let i = 0; i < kids.length; i++) {
        measure(ctx, kids[i], avail);
        total += kids[i].mw + (i > 0 ? gap : 0);
        if (kids[i].mh > tall) tall = kids[i].mh;
    }
    return [total, tall];
}

function wrapLines(ctx: Context, w: Widget, kids: Widget[], avail: number): Widget[][] {
    const gap = gapOf(w);
    const lines: Widget[][] = [];
    let line: Widget[] = [];
    let used = 0;
    for (let i = 0; i < kids.length; i++) {
        const k = kids[i];
        measure(ctx, k, avail);
        const need = (line.length > 0 ? gap : 0) + k.mw;
        if (avail >= 0 && line.length > 0 && used + need > avail) {
            lines.push(line);
            line = [];
            used = 0;
        }
        used += (line.length > 0 ? gap : 0) + k.mw;
        line.push(k);
    }
    if (line.length > 0) lines.push(line);
    return lines;
}

function measureWrap(ctx: Context, w: Widget, avail: number): [number, number] {
    const lines = wrapLines(ctx, w, flowKids(w), avail);
    const gap = gapOf(w);
    const rowGap = w.num("rowGap", gap);
    let wide = 0;
    let total = 0;
    for (let l = 0; l < lines.length; l++) {
        let lw = 0;
        let lh = 0;
        for (let i = 0; i < lines[l].length; i++) {
            lw += lines[l][i].mw + (i > 0 ? gap : 0);
            if (lines[l][i].mh > lh) lh = lines[l][i].mh;
        }
        if (lw > wide) wide = lw;
        total += lh + (l > 0 ? rowGap : 0);
    }
    return [wide, total];
}

function measureColumn(ctx: Context, w: Widget, avail: number): [number, number] {
    const kids = flowKids(w);
    const gap = gapOf(w);
    let wide = 0;
    let total = 0;
    for (let i = 0; i < kids.length; i++) {
        measure(ctx, kids[i], avail);
        if (kids[i].mw > wide) wide = kids[i].mw;
        total += kids[i].mh + (i > 0 ? gap : 0);
    }
    return [wide, total];
}

function measureStack(ctx: Context, w: Widget, avail: number): [number, number] {
    const kids = flowKids(w);
    let wide = 0;
    let tall = 0;
    for (let i = 0; i < kids.length; i++) {
        measure(ctx, kids[i], avail);
        if (kids[i].mw > wide) wide = kids[i].mw;
        if (kids[i].mh > tall) tall = kids[i].mh;
    }
    return [wide, tall];
}

// Grid columns: `columns` (a count, equal widths) or `columnWidths` (per column a pixel width,
// "content" or "fill").
function gridSpec(w: Widget): unknown[] {
    const cw = w.props.columnWidths;
    if (cw !== undefined) return cw as unknown[];
    const n = w.num("columns", 2);
    const out: unknown[] = [];
    for (let i = 0; i < n; i++) out.push("fill");
    return out;
}

function gridColumns(ctx: Context, w: Widget, kids: Widget[], avail: number): number[] {
    const spec = gridSpec(w);
    const n = spec.length;
    const gap = gapOf(w);
    const widths: number[] = [];
    let used = 0;
    let fills = 0;
    for (let c = 0; c < n; c++) {
        const s = spec[c];
        let cw = 0;
        if (typeof s === "number") cw = s;
        else if (s === "content" || avail < 0) {
            for (let i = c; i < kids.length; i += n) {
                measure(ctx, kids[i], -1);
                if (kids[i].mw > cw) cw = kids[i].mw;
            }
        } else fills++;
        if (s === "fill" && avail >= 0) cw = 0;
        widths.push(cw);
        used += cw;
    }
    if (fills > 0 && avail >= 0) {
        const free = Math.max(0, avail - used - gap * (n - 1));
        const share = Math.floor(free / fills);
        let rem = free - share * fills;
        for (let c = 0; c < n; c++) {
            if (spec[c] === "fill") {
                widths[c] = share + (rem > 0 ? 1 : 0);
                if (rem > 0) rem--;
            }
        }
    }
    return widths;
}

function gridRows(ctx: Context, w: Widget, kids: Widget[], widths: number[]): number[] {
    const n = widths.length;
    const heights: number[] = [];
    for (let i = 0; i < kids.length; i++) {
        const r = Math.floor(i / n);
        measure(ctx, kids[i], widths[i % n]);
        if (heights.length <= r) heights.push(0);
        if (kids[i].mh > heights[r]) heights[r] = kids[i].mh;
    }
    return heights;
}

function measureGrid(ctx: Context, w: Widget, avail: number): [number, number] {
    const kids = flowKids(w);
    const widths = gridColumns(ctx, w, kids, avail);
    const heights = gridRows(ctx, w, kids, widths);
    const gap = gapOf(w);
    const rowGap = w.num("rowGap", gap);
    let tw = 0;
    for (let c = 0; c < widths.length; c++) tw += widths[c] + (c > 0 ? gap : 0);
    let th = 0;
    for (let r = 0; r < heights.length; r++) th += heights[r] + (r > 0 ? rowGap : 0);
    return [avail >= 0 && tw < avail && hasFill(w) ? avail : tw, th];
}

function hasFill(w: Widget): boolean {
    const spec = gridSpec(w);
    for (let i = 0; i < spec.length; i++) if (spec[i] === "fill") return true;
    return false;
}

// --- arrange ------------------------------------------------------------------------------------

/** Gives w its box and lays out its children inside it. */
export function arrange(ctx: Context, w: Widget, x: number, y: number, width: number, height: number): void {
    w.x = x;
    w.y = y;
    w.w = width;
    w.h = height;
    const k = kinds[w.kind];
    if (k !== undefined && k.arrange !== undefined) k.arrange(ctx, w);
    const mode = modeOf(w);
    if (mode === "") return;
    const box = inner(ctx, w);
    if (mode === "row") {
        if (w.bool("wrap")) arrangeWrap(ctx, w, box);
        else arrangeLine(ctx, w, box, 0);
    } else if (mode === "column") arrangeLine(ctx, w, box, 1);
    else if (mode === "stack") arrangeStack(ctx, w, box);
    else if (mode === "grid") arrangeGrid(ctx, w, box);
    else if (mode === "scroll") arrangeScroll(ctx, w, box);
    else if (mode === "list") arrangeList(ctx, w, box);
    arrangeAnchored(ctx, w, box);
}

/** Shares `free` pixels over weights: floor each share, then a pixel each from the first. */
function share(free: number, weights: number[]): number[] {
    let total = 0;
    for (let i = 0; i < weights.length; i++) total += weights[i];
    const out: number[] = [];
    let given = 0;
    for (let i = 0; i < weights.length; i++) {
        const s = total > 0 ? Math.floor((free * weights[i]) / total) : 0;
        out.push(s);
        given += s;
    }
    let rem = free - given;
    for (let i = 0; i < out.length && rem > 0; i++) {
        out[i]++;
        rem--;
    }
    return out;
}

function alignOffset(align: string, room: number): number {
    if (align === "center") return Math.floor(room / 2);
    if (align === "end") return room;
    return 0;
}

/** A row (axis 0) or column (axis 1) without wrapping. */
function arrangeLine(ctx: Context, w: Widget, box: [number, number, number, number], axis: Axis): void {
    const kids = flowKids(w);
    const n = kids.length;
    if (n === 0) return;
    const gap = gapOf(w);
    const mainSize = axis === 0 ? box[2] : box[3];
    const crossSize = axis === 0 ? box[3] : box[2];
    const defAlign = w.str("align", axis === 0 ? "center" : "stretch");
    // Cross sizes first for columns, because a child's height depends on its width.
    const crossOf: number[] = [];
    const base: number[] = [];
    const fillIdx: number[] = [];
    const fillW: number[] = [];
    let used = gap * (n - 1);
    for (let i = 0; i < n; i++) {
        const k = kids[i];
        const align = k.str("alignSelf", defAlign);
        if (axis === 1) {
            measure(ctx, k, crossSize);
            const fixed = typeof k.props.width === "number" || rigid(k, 0);
            const cw = fixed ? k.mw : isFill(k, 0, axis) || align === "stretch" ? clampAxis(k, 0, crossSize) : Math.min(k.mw, crossSize);
            crossOf.push(cw);
            measure(ctx, k, cw);
        } else {
            measure(ctx, k, mainSize);
            crossOf.push(0);
        }
        if (isFill(k, axis, axis)) {
            base.push(0);
            fillIdx.push(i);
            fillW.push(flexOf(k));
        } else {
            const m = axis === 0 ? k.mw : k.mh;
            base.push(m);
            used += m;
        }
    }
    const free = mainSize - used;
    if (fillIdx.length > 0) {
        // A share that min/max changes is fixed at the clamped size and the rest is shared again
        // among the others, until no share is clamped.
        let idx = fillIdx;
        let weights = fillW;
        let room = Math.max(0, free);
        for (;;) {
            const shares = share(room, weights);
            const nextIdx: number[] = [];
            const nextW: number[] = [];
            let clamped = false;
            for (let j = 0; j < idx.length; j++) {
                const v = clampAxis(kids[idx[j]], axis, shares[j]);
                if (v !== shares[j]) {
                    base[idx[j]] = v;
                    room = Math.max(0, room - v);
                    clamped = true;
                } else {
                    nextIdx.push(idx[j]);
                    nextW.push(weights[j]);
                }
            }
            if (!clamped) {
                for (let j = 0; j < idx.length; j++) base[idx[j]] = shares[j];
                break;
            }
            idx = nextIdx;
            weights = nextW;
            if (idx.length === 0) break;
        }
    }
    let pos = axis === 0 ? box[0] : box[1];
    let gaps: number[] = [];
    const justify = w.str("justify", "start");
    if (fillIdx.length === 0 && free > 0) {
        if (justify === "between" && n > 1) {
            const ws: number[] = [];
            for (let i = 0; i < n - 1; i++) ws.push(1);
            gaps = share(free, ws);
        } else pos += alignOffset(justify, free);
    }
    for (let i = 0; i < n; i++) {
        const k = kids[i];
        const align = k.str("alignSelf", defAlign);
        const main = base[i];
        let crossLen: number;
        if (axis === 1) crossLen = crossOf[i];
        else {
            if (typeof k.props.height === "number" || rigid(k, 1)) crossLen = k.mh;
            else if (isFill(k, 1, axis) || align === "stretch") crossLen = clampAxis(k, 1, crossSize);
            else {
                measure(ctx, k, main);
                crossLen = k.mh;
            }
        }
        const crossPos = (axis === 0 ? box[1] : box[0]) + alignOffset(align === "stretch" ? "start" : align, crossSize - crossLen);
        if (axis === 0) arrange(ctx, k, pos, crossPos, main, crossLen);
        else arrange(ctx, k, crossPos, pos, crossLen, main);
        pos += main + gap + (i < gaps.length ? gaps[i] : 0);
    }
}

function arrangeWrap(ctx: Context, w: Widget, box: [number, number, number, number]): void {
    const lines = wrapLines(ctx, w, flowKids(w), box[2]);
    const gap = gapOf(w);
    const rowGap = w.num("rowGap", gap);
    const align = w.str("align", "center");
    let y = box[1];
    for (let l = 0; l < lines.length; l++) {
        const line = lines[l];
        let lh = 0;
        let lw = 0;
        for (let i = 0; i < line.length; i++) {
            if (line[i].mh > lh) lh = line[i].mh;
            lw += line[i].mw + (i > 0 ? gap : 0);
        }
        let x = box[0] + alignOffset(w.str("justify", "start"), box[2] - lw);
        for (let i = 0; i < line.length; i++) {
            const k = line[i];
            const kh = align === "stretch" ? lh : k.mh;
            arrange(ctx, k, x, y + alignOffset(align === "stretch" ? "start" : align, lh - kh), k.mw, kh);
            x += k.mw + gap;
        }
        y += lh + rowGap;
    }
}

function arrangeStack(ctx: Context, w: Widget, box: [number, number, number, number]): void {
    const kids = flowKids(w);
    const align = w.str("align", "stretch");
    for (let i = 0; i < kids.length; i++) {
        const k = kids[i];
        measure(ctx, k, box[2]);
        const kw = typeof k.props.width === "number" || rigid(k, 0) || align !== "stretch" ? Math.min(k.mw, box[2]) : box[2];
        const kh = typeof k.props.height === "number" || rigid(k, 1) || align !== "stretch" ? k.mh : box[3];
        const a = align === "stretch" ? "start" : align;
        arrange(ctx, k, box[0] + alignOffset(a, box[2] - kw), box[1] + alignOffset(a, box[3] - kh), kw, kh);
    }
}

function arrangeGrid(ctx: Context, w: Widget, box: [number, number, number, number]): void {
    const kids = flowKids(w);
    const widths = gridColumns(ctx, w, kids, box[2]);
    const heights = gridRows(ctx, w, kids, widths);
    const n = widths.length;
    const gap = gapOf(w);
    const rowGap = w.num("rowGap", gap);
    const align = w.str("align", "stretch");
    let y = box[1];
    for (let r = 0; r < heights.length; r++) {
        let x = box[0];
        for (let c = 0; c < n; c++) {
            const i = r * n + c;
            if (i >= kids.length) break;
            const k = kids[i];
            const kh = align === "stretch" ? heights[r] : k.mh;
            arrange(ctx, k, x, y + alignOffset(align === "stretch" ? "start" : align, heights[r] - kh), widths[c], kh);
            x += widths[c] + gap;
        }
        y += heights[r] + rowGap;
    }
}

function arrangeScroll(ctx: Context, w: Widget, box: [number, number, number, number]): void {
    const kids = flowKids(w);
    const gap = gapOf(w);
    let cw = box[2];
    let total = columnHeight(ctx, kids, cw, gap);
    if (total > box[3]) {
        cw = Math.max(0, box[2] - ctx.theme.scrollbarWidth() - w.num("scrollGap", 2));
        total = columnHeight(ctx, kids, cw, gap);
    }
    w.contentH = total;
    const max = Math.max(0, total - box[3]);
    if (w.scrollY > max) w.scrollY = max;
    if (w.scrollY < 0) w.scrollY = 0;
    let y = box[1] - w.scrollY;
    for (let i = 0; i < kids.length; i++) {
        arrange(ctx, kids[i], box[0], y, cw, kids[i].mh);
        y += kids[i].mh + gap;
    }
}

function columnHeight(ctx: Context, kids: Widget[], width: number, gap: number): number {
    let total = 0;
    for (let i = 0; i < kids.length; i++) {
        measure(ctx, kids[i], width);
        if (typeof kids[i].props.width !== "number") kids[i].mw = width;
        total += kids[i].mh + (i > 0 ? gap : 0);
    }
    return total;
}

function arrangeList(ctx: Context, w: Widget, box: [number, number, number, number]): void {
    const items = (w.props.items as unknown[] | undefined) ?? [];
    const rowH = Math.max(1, w.num("rowHeight", 20));
    const n = items.length;
    const total = n * rowH;
    w.contentH = total;
    const rowW = total > box[3] ? Math.max(0, box[2] - ctx.theme.scrollbarWidth() - w.num("scrollGap", 2)) : box[2];
    const max = Math.max(0, total - box[3]);
    if (w.scrollY > max) w.scrollY = max;
    if (w.scrollY < 0) w.scrollY = 0;
    const first = Math.floor(w.scrollY / rowH);
    const last = Math.min(n - 1, Math.floor((w.scrollY + box[3] - 1) / rowH));
    const old = (w.state.rows as ListRow[] | undefined) ?? [];
    const rows: ListRow[] = [];
    const render = w.props.render as ((item: unknown, index: number) => unknown) | undefined;
    for (let i = 0; i < old.length; i++) {
        const r = old[i];
        if (r.index < first || r.index > last || r.index >= n || items[r.index] !== r.item) r.dispose();
    }
    for (let idx = first; idx <= last; idx++) {
        let found: ListRow | undefined = undefined;
        for (let i = 0; i < old.length; i++) {
            const r = old[i];
            if (r.index === idx && items[idx] === r.item) found = r;
        }
        if (found === undefined && render !== undefined) {
            const item = items[idx];
            let widget: Widget | undefined = undefined;
            let dispose: () => void = () => {};
            untrack(() =>
                runWithOwner(w.owner, () =>
                    createRoot((d) => {
                        dispose = d;
                        const el = render(item, idx);
                        if (!(el instanceof Widget)) throw new Error("ui: a list's render must return one element");
                        widget = el;
                    }),
                ),
            );
            found = { index: idx, item, widget: widget as unknown as Widget, dispose };
        }
        if (found !== undefined) rows.push(found);
    }
    w.state.rows = rows;
    // Children change during layout; this must not ask for another layout.
    const children: Widget[] = [];
    for (let i = 0; i < rows.length; i++) {
        rows[i].widget.parent = w;
        children.push(rows[i].widget);
    }
    w.children = children;
    for (let i = 0; i < rows.length; i++) arrange(ctx, rows[i].widget, box[0], box[1] + rows[i].index * rowH - w.scrollY, rowW, rowH);
}

function arrangeAnchored(ctx: Context, w: Widget, box: [number, number, number, number]): void {
    const all = layoutChildren(w, []);
    for (let i = 0; i < all.length; i++) {
        const k = all[i];
        if (!anchored(k)) continue;
        const a = k.props.anchor as Anchor;
        const hasL = a.left !== undefined;
        const hasR = a.right !== undefined;
        const hasT = a.top !== undefined;
        const hasB = a.bottom !== undefined;
        const l = hasL ? (a.left as number) : 0;
        const r = hasR ? (a.right as number) : 0;
        const t = hasT ? (a.top as number) : 0;
        const b = hasB ? (a.bottom as number) : 0;
        const availW = hasL && hasR ? Math.max(0, box[2] - l - r) : box[2];
        measure(ctx, k, availW);
        const kw = hasL && hasR && typeof k.props.width !== "number" ? availW : k.mw;
        if (hasT && hasB && typeof k.props.height !== "number") measure(ctx, k, kw);
        const kh = hasT && hasB && typeof k.props.height !== "number" ? Math.max(0, box[3] - t - b) : k.mh;
        let x = box[0];
        if (hasL) x = box[0] + l;
        else if (hasR) x = box[0] + box[2] - r - kw;
        else if (a.center === true) x = box[0] + Math.floor((box[2] - kw) / 2);
        let y = box[1];
        if (hasT) y = box[1] + t;
        else if (hasB) y = box[1] + box[3] - b - kh;
        else if (a.center === true) y = box[1] + Math.floor((box[3] - kh) / 2);
        arrange(ctx, k, x, y, kw, kh);
    }
}

/**
 * Lays the whole tree out for a screen of the context's size. The root takes a fixed width or
 * height it asks for, its content size for "content", and the screen otherwise.
 */
export function layoutRoot(ctx: Context, root: Widget): void {
    const fw = root.props.width;
    const fh = root.props.height;
    let w = ctx.width;
    let h = ctx.height;
    if (typeof fw === "number" || fw === "content" || typeof fh === "number" || fh === "content") {
        measure(ctx, root, ctx.width);
        if (typeof fw === "number" || fw === "content") w = root.mw;
        if (typeof fh === "number" || fh === "content") h = root.mh;
    }
    arrange(ctx, root, 0, 0, w, h);
}
