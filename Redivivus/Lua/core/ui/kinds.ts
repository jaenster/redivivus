// What each widget kind measures, paints and does with input. Containers' child layout is in
// layout.ts; this file has the leaves and the chrome.

import { BlendMode, BoxStyle, ButtonVariant, FontRole, TextColour } from "../redivivus/enums";
import type { FontHandle } from "./backend";
import type { Context } from "./theme";
import { drawLines, ellipsize, layoutSpans, layoutText, linesHeight, linesWidth, truncate, type Span, type TextLine } from "./text";
import { paddingOf, within, type UIEvent, type Widget } from "./widget";

// Plain functions (this: void), so a kind can reuse another kind's function or a top-level one.
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

export const kinds: { [kind: string]: Kind } = {};

const ALIGN_LEFT = 0;
const ALIGN_CENTER = 1;
const ALIGN_RIGHT = 2;

export function alignOf(w: Widget): number {
    const a = w.props.textAlign;
    if (a === "center") return ALIGN_CENTER;
    if (a === "right") return ALIGN_RIGHT;
    return ALIGN_LEFT;
}

export function fontOf(ctx: Context, w: Widget, role: FontRole): FontHandle {
    const f = w.props.font;
    return ctx.theme.font(typeof f === "string" ? (f as FontRole) : role);
}

export function colourOf(ctx: Context, w: Widget, role: TextColour): number {
    const c = w.props.colour;
    if (typeof c === "number") return c;
    return ctx.theme.colour(typeof c === "string" ? (c as TextColour) : role);
}

/** The inner box: x, y, w, h inside padding and frame inset. */
export function inner(ctx: Context, w: Widget): [number, number, number, number] {
    const p = paddingOf(w);
    const inset = w.kind === "frame" ? ctx.theme.frameInset(w.str("style", BoxStyle.None) as BoxStyle) : 0;
    const x = w.x + p[3] + inset;
    const y = w.y + p[0] + inset;
    return [x, y, Math.max(0, w.w - p[1] - p[3] - inset * 2), Math.max(0, w.h - p[0] - p[2] - inset * 2)];
}

function call(w: Widget, name: string, value?: unknown): void {
    const f = w.props[name];
    if (typeof f === "function") (f as (v: unknown) => void)(value);
}

// --- text ---------------------------------------------------------------------------------------

/**
 * A label's lines for a width. Wrapping, they break at the width. Not wrapping, a line wider than
 * the width is tried in each of `fitFonts` in turn, and what still does not fit is cut at a glyph
 * with an ellipsis (unless `ellipsis` is false), so a clip never ends one mid-glyph.
 */
function labelLines(ctx: Context, w: Widget, width: number): TextLine[] {
    const text = w.str("text", "");
    const colour = colourOf(ctx, w, TextColour.Text);
    if (w.props.wrap !== false) return layoutText(ctx.backend, fontOf(ctx, w, FontRole.Body), text, colour, width);
    let lines = layoutText(ctx.backend, fontOf(ctx, w, FontRole.Body), text, colour, -1);
    if (width < 0 || linesWidth(lines) <= width) return lines;
    const fit = w.props.fitFonts as FontRole[] | undefined;
    if (fit !== undefined) {
        for (let i = 0; i < fit.length; i++) {
            lines = layoutText(ctx.backend, ctx.theme.font(fit[i]), text, colour, -1);
            if (linesWidth(lines) <= width) return lines;
        }
    }
    if (w.props.ellipsis === false) return lines;
    const out: TextLine[] = [];
    for (let i = 0; i < lines.length; i++) out.push(ellipsize(ctx.backend, lines[i], width));
    return out;
}

kinds.label = {
    measure(ctx, w, availW) {
        const lines = labelLines(ctx, w, availW);
        return [linesWidth(lines), linesHeight(lines)];
    },
    arrange(ctx, w) {
        const box = inner(ctx, w);
        w.state.lines = labelLines(ctx, w, box[2]);
    },
    paint(ctx, w) {
        const lines = w.state.lines as TextLine[] | undefined;
        if (lines === undefined) return;
        const box = inner(ctx, w);
        drawLines(ctx.backend, lines, box[0], box[1], box[2], alignOf(w));
    },
};

/** A paragraph of spans in several fonts and colours, wrapped as one (Markdown text). */
export interface RichSpan {
    text: string;
    font: FontRole;
    colour: TextColour | number;
}

function richLines(ctx: Context, w: Widget, width: number): TextLine[] {
    const src = (w.props.spans as RichSpan[] | undefined) ?? [];
    const spans: Span[] = [];
    for (let i = 0; i < src.length; i++) {
        const s = src[i];
        spans.push({ text: s.text, font: ctx.theme.font(s.font), colour: typeof s.colour === "number" ? s.colour : ctx.theme.colour(s.colour) });
    }
    return layoutSpans(ctx.backend, spans, w.props.wrap === false ? -1 : width);
}

kinds.rich = {
    measure(ctx, w, availW) {
        const lines = richLines(ctx, w, availW);
        return [linesWidth(lines), linesHeight(lines)];
    },
    arrange(ctx, w) {
        w.state.lines = richLines(ctx, w, inner(ctx, w)[2]);
    },
    paint: kinds.label.paint,
};

// --- controls -----------------------------------------------------------------------------------

function buttonLabel(w: Widget): string {
    return w.str("label", w.str("text", ""));
}

/**
 * The art a button is drawn with and its label cut to the art's room. Art is drawn at its own size,
 * never stretched: a Small button takes the smallest art its label fits in, the others their own.
 */
function buttonArt(ctx: Context, w: Widget): [ButtonVariant, string] {
    let variant = w.str("variant", ButtonVariant.Medium) as ButtonVariant;
    const label = buttonLabel(w);
    if (variant === ButtonVariant.Small) {
        // Fonts follow the art's height, so measure in each candidate's font until one fits.
        variant = ctx.theme.fitButton(0);
        for (let i = 0; i < 4; i++) {
            const font = ctx.theme.font(ctx.theme.buttonFont(ctx.theme.buttonSize(variant)[1]));
            const next = ctx.theme.fitButton(ctx.backend.measureText(font, label));
            if (next === variant) break;
            variant = next;
        }
    }
    const font = ctx.theme.font(ctx.theme.buttonFont(ctx.theme.buttonSize(variant)[1]));
    return [variant, truncate(ctx.backend, font, label, ctx.theme.buttonRoom(variant))];
}

kinds.button = {
    rigid: true,
    measure(ctx, w) {
        return ctx.theme.buttonSize(buttonArt(ctx, w)[0]);
    },
    paint(ctx, w) {
        const disabled = w.bool("disabled");
        const pressed = !disabled && (w.bool("pressed") || (ctx.capture !== undefined && within(ctx.capture, w) && within(ctx.hover, w)));
        const art = buttonArt(ctx, w);
        ctx.theme.paintButton(ctx.backend, w, art[0], art[1], pressed, !disabled && within(ctx.hover, w), disabled);
    },
    event(_ctx, w, ev) {
        if (w.bool("disabled") && (ev.type === "click" || ev.type === "mousedown")) return false;
        return true;
    },
};

const CHECK_GAP = 6;

kinds.checkbox = {
    rigid: true,
    measure(ctx, w) {
        const box = ctx.theme.checkboxSize();
        const label = w.str("label", "");
        if (label.length === 0) return [box, box];
        const font = fontOf(ctx, w, FontRole.Body);
        const lw = ctx.backend.measureText(font, label);
        return [box + CHECK_GAP + lw, Math.max(box, ctx.backend.fontLineHeight(font))];
    },
    paint(ctx, w) {
        const box = ctx.theme.checkboxSize();
        const b = inner(ctx, w);
        const by = b[1] + Math.floor((b[3] - box) / 2);
        const disabled = w.bool("disabled");
        ctx.theme.paintCheckbox(ctx.backend, b[0], by, w.bool("checked"), within(ctx.hover, w), disabled);
        const label = w.str("label", "");
        if (label.length > 0) {
            const font = fontOf(ctx, w, FontRole.Body);
            const ty = b[1] + Math.floor((b[3] - ctx.backend.fontLineHeight(font)) / 2);
            const lines = layoutText(ctx.backend, font, label, colourOf(ctx, w, disabled ? TextColour.Disabled : TextColour.Text), -1);
            drawLines(ctx.backend, lines, b[0] + box + CHECK_GAP, ty, b[2], ALIGN_LEFT);
        }
    },
    event(_ctx, w, ev) {
        if (w.bool("disabled")) return ev.type !== "click";
        if (ev.type === "click") call(w, "onChange", !w.bool("checked"));
        return true;
    },
};

kinds.bar = {
    measure(ctx, w, availW) {
        return [availW >= 0 ? availW : 100, ctx.theme.barHeight()];
    },
    paint(ctx, w) {
        const b = inner(ctx, w);
        ctx.theme.paintBar(ctx.backend, b[0], b[1], b[2], b[3], w.num("value", 0), w.num("max", 100), w.num("rgb", -1));
        const text = w.str("text", "");
        if (text.length > 0) {
            const lines = layoutText(ctx.backend, ctx.theme.font(FontRole.Small), text, colourOf(ctx, w, TextColour.Text), -1);
            drawLines(ctx.backend, lines, b[0], b[1] + Math.floor((b[3] - linesHeight(lines)) / 2), b[2], ALIGN_CENTER);
        }
    },
};

function spriteHandle(ctx: Context, w: Widget): number {
    const path = w.str("path", "");
    if (w.state.path !== path) {
        w.state.path = path;
        w.state.handle = path.length > 0 ? ctx.backend.loadSprite(path) : 0;
    }
    return w.state.handle as number;
}

kinds.sprite = {
    rigid: true,
    measure(ctx, w) {
        const h = spriteHandle(ctx, w);
        const f = w.num("frame", 0);
        return [ctx.backend.spriteWidth(h, f), ctx.backend.spriteHeight(h, f)];
    },
    paint(ctx, w) {
        const b = inner(ctx, w);
        ctx.backend.drawSprite(spriteHandle(ctx, w), w.num("frame", 0), b[0], b[1], w.num("mode", BlendMode.Normal) as BlendMode);
    },
};

kinds.itemSlot = {
    rigid: true,
    measure(ctx, w) {
        const c = ctx.theme.cellSize();
        return [w.num("cols", 1) * c, w.num("rows", 1) * c];
    },
    paint(ctx, w) {
        const b = inner(ctx, w);
        ctx.theme.paintSlot(ctx.backend, b[0], b[1], b[2], b[3], w.num("cols", 1), w.num("rows", 1));
    },
};

kinds.spacer = {
    measure() {
        return [0, 0];
    },
    spacer: true,
};

kinds.rule = {
    measure(_ctx, _w, availW) {
        return [availW >= 0 ? availW : 0, 1];
    },
    paint(ctx, w) {
        const b = inner(ctx, w);
        ctx.theme.paintRule(ctx.backend, b[0], b[1], b[2]);
    },
};

kinds.image = {
    rigid: true,
    measure(ctx, w, availW) {
        const iw = w.num("imageWidth", 160);
        return [availW >= 0 ? Math.min(iw, availW) : iw, w.num("imageHeight", 48)];
    },
    paint(ctx, w) {
        const b = inner(ctx, w);
        ctx.theme.paintImagePlaceholder(ctx.backend, b[0], b[1], b[2], b[3]);
        const lines = layoutText(ctx.backend, ctx.theme.font(FontRole.Small), w.str("alt", ""), ctx.theme.colour(TextColour.Muted), b[2] - 8);
        // One line is centred by its glyph cell: a font's line advance can be far taller (font8's 22
        // against a 14-pixel cell), which put a one-letter stand-in near the top.
        const th = lines.length === 1 ? lines[0].ascent : linesHeight(lines);
        drawLines(ctx.backend, lines, b[0] + 4, b[1] + Math.floor((b[3] - th) / 2), b[2] - 8, ALIGN_CENTER);
    },
};

// --- text field -----------------------------------------------------------------------------------

const KEY_BACKSPACE = 8;
const KEY_ENTER = 13;
const KEY_END = 35;
const KEY_HOME = 36;
const KEY_LEFT = 37;
const KEY_RIGHT = 39;
const KEY_DELETE = 46;
const FIELD_PAD = 6;

function caretOf(w: Widget): number {
    const value = w.str("value", "");
    const c = w.state.caret;
    if (typeof c !== "number" || c > value.length) return value.length;
    return c;
}

kinds.textField = {
    focusable: true,
    measure(ctx, w, availW) {
        return [availW >= 0 ? Math.min(availW, 200) : 200, ctx.theme.textFieldHeight()];
    },
    paint(ctx, w) {
        const focused = ctx.focus === w;
        ctx.theme.paintTextField(ctx.backend, w, focused);
        const b = inner(ctx, w);
        const font = fontOf(ctx, w, FontRole.Body);
        const value = w.str("value", "");
        const lh = ctx.backend.fontLineHeight(font);
        const ty = b[1] + Math.floor((b[3] - lh) / 2);
        ctx.backend.pushScissor(b[0] + FIELD_PAD - 2, b[1], Math.max(0, b[2] - FIELD_PAD * 2 + 4), b[3]);
        if (value.length === 0 && !focused) {
            const ph = w.str("placeholder", "");
            if (ph.length > 0) drawLines(ctx.backend, layoutText(ctx.backend, font, ph, ctx.theme.colour(TextColour.Muted), -1), b[0] + FIELD_PAD, ty, b[2], ALIGN_LEFT);
        } else {
            const caret = caretOf(w);
            const before = ctx.backend.measureText(font, value.substring(0, caret));
            const room = b[2] - FIELD_PAD * 2;
            const shift = before > room ? before - room : 0;
            drawLines(ctx.backend, layoutText(ctx.backend, font, value, colourOf(ctx, w, TextColour.Text), -1), b[0] + FIELD_PAD - shift, ty, b[2], ALIGN_LEFT);
            if (focused) ctx.theme.paintCaret(ctx.backend, b[0] + FIELD_PAD - shift + before, ty, lh);
        }
        ctx.backend.popScissor();
    },
    event(_ctx, w, ev) {
        const value = w.str("value", "");
        let caret = caretOf(w);
        let next: string | undefined = undefined;
        if (ev.type === "text") {
            const max = w.num("maxLength", 256);
            let t = "";
            for (let i = 0; i < ev.text.length; i++) {
                const c = ev.text.charCodeAt(i);
                if (c >= 32 && c !== 127 && value.length + t.length < max) t += ev.text.substring(i, i + 1);
            }
            if (t.length > 0) {
                next = value.substring(0, caret) + t + value.substring(caret);
                caret += t.length;
            }
        } else if (ev.type === "keydown") {
            if (ev.key === KEY_BACKSPACE && caret > 0) {
                next = value.substring(0, caret - 1) + value.substring(caret);
                caret--;
            } else if (ev.key === KEY_DELETE && caret < value.length) {
                next = value.substring(0, caret) + value.substring(caret + 1);
            } else if (ev.key === KEY_LEFT) caret = Math.max(0, caret - 1);
            else if (ev.key === KEY_RIGHT) caret = Math.min(value.length, caret + 1);
            else if (ev.key === KEY_HOME) caret = 0;
            else if (ev.key === KEY_END) caret = value.length;
            else if (ev.key === KEY_ENTER) call(w, "onSubmit", value);
            else return true;
            ev.handled = true;
        } else {
            return true;
        }
        w.state.caret = caret;
        if (next !== undefined) {
            if (w.props.onInput === undefined) w.set("value", next);
            call(w, "onInput", next);
        }
        ev.handled = true;
        return true;
    },
};

// --- tabs -----------------------------------------------------------------------------------------

function tabLabels(w: Widget): string[] {
    return (w.props.labels as string[] | undefined) ?? [];
}

/**
 * The one art every tab of a row is drawn with, for a width (< 0: unbounded): the smallest of the
 * theme's tab art that every label fits in and whose row fits the width; failing that the largest
 * whose row fits, with the labels cut to its room; failing that the smallest.
 */
function tabArt(ctx: Context, w: Widget, width: number): ButtonVariant {
    const labels = tabLabels(w);
    const variants = ctx.theme.tabVariants();
    const gap = w.num("gap", 2);
    let widest: ButtonVariant | undefined = undefined;
    for (let v = 0; v < variants.length; v++) {
        const variant = variants[v];
        const size = ctx.theme.buttonSize(variant);
        const row = labels.length * size[0] + Math.max(0, labels.length - 1) * gap;
        if (width >= 0 && row > width) continue;
        widest = variant;
        const font = ctx.theme.font(ctx.theme.buttonFont(size[1]));
        const room = ctx.theme.buttonRoom(variant);
        let fits = true;
        for (let i = 0; i < labels.length; i++) if (ctx.backend.measureText(font, labels[i]) > room) fits = false;
        if (fits) return variant;
    }
    return widest ?? variants[0];
}

/** Each tab's x and width, in the art tabArt picks for the inner width, placed by `justify`. */
function tabRects(ctx: Context, w: Widget): number[] {
    const labels = tabLabels(w);
    const b = inner(ctx, w);
    const tw = ctx.theme.buttonSize(tabArt(ctx, w, b[2]))[0];
    const gap = w.num("gap", 2);
    const row = labels.length * tw + Math.max(0, labels.length - 1) * gap;
    const justify = w.str("justify", "start");
    let x = b[0] + (justify === "end" ? Math.max(0, b[2] - row) : justify === "center" ? Math.max(0, Math.floor((b[2] - row) / 2)) : 0);
    const out: number[] = [];
    for (let i = 0; i < labels.length; i++) {
        out.push(x, tw);
        x += tw + gap;
    }
    return out;
}

kinds.tabs = {
    rigid: true,
    measure(ctx, w, availW) {
        const labels = tabLabels(w);
        const size = ctx.theme.buttonSize(tabArt(ctx, w, availW));
        return [labels.length * size[0] + Math.max(0, labels.length - 1) * w.num("gap", 2), size[1]];
    },
    paint(ctx, w) {
        const r = tabRects(ctx, w);
        const labels = tabLabels(w);
        const b = inner(ctx, w);
        const variant = tabArt(ctx, w, b[2]);
        const font = ctx.theme.font(ctx.theme.buttonFont(ctx.theme.buttonSize(variant)[1]));
        const room = ctx.theme.buttonRoom(variant);
        const sel = w.num("selected", 0);
        for (let i = 0; i < labels.length; i++) {
            const tx = r[i * 2];
            const tw = r[i * 2 + 1];
            const hover = within(ctx.hover, w) && ctx.pointerX >= tx && ctx.pointerX < tx + tw;
            ctx.theme.paintTab(ctx.backend, tx, b[1], variant, truncate(ctx.backend, font, labels[i], room), i === sel, hover);
        }
    },
    event(ctx, w, ev) {
        if (ev.type !== "click") return true;
        const r = tabRects(ctx, w);
        for (let i = 0; i * 2 < r.length; i++) {
            if (ev.x >= r[i * 2] && ev.x < r[i * 2] + r[i * 2 + 1]) {
                if (i !== w.num("selected", 0)) call(w, "onChange", i);
                ev.handled = true;
                return true;
            }
        }
        return true;
    },
};

// --- containers' chrome -------------------------------------------------------------------------

kinds.frame = {
    layout: "column",
    paint(ctx, w) {
        const bg = w.props.bg;
        if (typeof bg === "number") ctx.backend.fillRect(w.x, w.y, w.w, w.h, bg, w.num("bgAlpha", 255));
        const nine = w.props.nineSlice as { path: string; frames: number[] } | undefined;
        if (nine !== undefined) {
            if (w.state.ninePath !== nine.path) {
                w.state.ninePath = nine.path;
                w.state.nineHandle = ctx.backend.loadSprite(nine.path);
            }
            ctx.backend.drawNineSlice(w.state.nineHandle as number, nine.frames, w.x, w.y, w.w, w.h, BlendMode.Normal);
        }
        const style = w.str("style", BoxStyle.None) as BoxStyle;
        if (style !== BoxStyle.None) ctx.theme.paintFrame(ctx.backend, w, style);
    },
};

kinds.flow = { layout: "row", paint: kinds.frame.paint };
kinds.grid = { layout: "grid", paint: kinds.frame.paint };
kinds.stack = { layout: "stack", paint: kinds.frame.paint };
kinds.tooltip = { layout: "column" };

const WHEEL_STEP = 24;

function scrollEvent(ctx: Context, w: Widget, ev: UIEvent): boolean {
    if (ev.type !== "wheel") return true;
    const view = inner(ctx, w)[3];
    const max = Math.max(0, w.contentH - view);
    const step = w.num("wheelStep", WHEEL_STEP);
    const next = Math.max(0, Math.min(max, w.scrollY + ev.delta * step));
    if (next !== w.scrollY) {
        w.scrollTo(next);
        call(w, "onScroll", next);
    }
    ev.handled = true;
    return false;
}

function paintScrollbar(ctx: Context, w: Widget): void {
    const b = inner(ctx, w);
    if (w.contentH <= b[3]) return;
    const sw = ctx.theme.scrollbarWidth();
    ctx.theme.paintScrollbar(ctx.backend, b[0] + b[2] - sw, b[1], b[3], w.scrollY, b[3], w.contentH);
}

kinds.scrollPane = { layout: "scroll", paint: kinds.frame.paint, paintAfter: paintScrollbar, event: scrollEvent };

kinds.list = {
    layout: "list",
    paint(ctx, w) {
        kinds.frame.paint!(ctx, w);
        const rows = (w.state.rows as ListRow[] | undefined) ?? [];
        const sel = w.props.selected;
        for (let i = 0; i < rows.length; i++) {
            const r = rows[i].widget;
            const isSel = typeof sel === "number" && sel === rows[i].index;
            const hover = within(ctx.hover, r);
            if (isSel || hover) ctx.theme.paintRow(ctx.backend, r.x, r.y, r.w, r.h, isSel, hover);
        }
    },
    paintAfter: paintScrollbar,
    event(ctx, w, ev) {
        if (ev.type === "click") {
            const rows = (w.state.rows as ListRow[] | undefined) ?? [];
            for (let i = 0; i < rows.length; i++) if (within(ev.target, rows[i].widget)) call(w, "onSelect", rows[i].index);
            return true;
        }
        return scrollEvent(ctx, w, ev);
    },
};

export interface ListRow {
    index: number;
    item: unknown;
    widget: Widget;
    dispose: () => void;
}
