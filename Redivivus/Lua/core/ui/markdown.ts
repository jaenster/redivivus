// Markdown to widgets: the tree of the README parser (md/, vendored from the portal) rendered
// with the game's fonts. Headings, paragraphs with emphasis, strong, code and links as coloured
// runs in one wrapped paragraph; lists, block quotes, fenced code (never wrapped, clipped), tables
// as grids, rules, and images as a placeholder box with the alt text (the kit loads no images
// from a README). Links are coloured, not followed; `onLink` receives the classified target.

import { BoxStyle, FontRole, TextColour } from "../redivivus/enums";
import { h, intrinsicComponents, type Child, type IntrinsicComponent } from "./jsx";
import type { Block, Document, Inline } from "./md/ast";
import { parse } from "./md/parse";
import { classifyLink } from "./md/url";
import { computed, effect, read, type MaybeAccessor } from "./signal";
import type { RichSpan } from "./kinds";
import { Widget } from "./widget";

interface Style {
    font: FontRole;
    colour: TextColour | number;
}

function spansOf(inlines: Inline[], style: Style, out: RichSpan[]): void {
    for (let i = 0; i < inlines.length; i++) {
        const n = inlines[i];
        if (n.type === "text") out.push({ text: n.text, font: style.font, colour: style.colour });
        else if (n.type === "code") out.push({ text: n.text, font: FontRole.Mono, colour: TextColour.Code });
        else if (n.type === "softbreak") out.push({ text: " ", font: style.font, colour: style.colour });
        else if (n.type === "hardbreak") out.push({ text: "\n", font: style.font, colour: style.colour });
        else if (n.type === "emphasis") spansOf(n.children, { font: style.font, colour: style.colour === TextColour.Text ? TextColour.Emphasis : style.colour }, out);
        else if (n.type === "strong") spansOf(n.children, { font: style.font, colour: style.colour === TextColour.Text || style.colour === TextColour.Emphasis ? TextColour.Strong : style.colour }, out);
        else if (n.type === "link") {
            const target = classifyLink(n.url);
            spansOf(n.children, { font: style.font, colour: target.kind === "blocked" ? TextColour.Muted : TextColour.Link }, out);
        } else if (n.type === "image") out.push({ text: "[" + n.alt + "]", font: style.font, colour: TextColour.Muted });
    }
}

function rich(spans: RichSpan[], align?: string): Widget {
    const w = new Widget("rich");
    w.props.spans = spans;
    if (align !== undefined && align.length > 0) w.props.textAlign = align;
    return w;
}

function column(children: Widget[], gap: number): Widget {
    const w = new Widget("flow");
    w.props.direction = "column";
    w.props.gap = gap;
    for (let i = 0; i < children.length; i++) w.append(children[i]);
    return w;
}

function block(b: Block, gap: number): Widget {
    if (b.type === "heading") {
        const spans: RichSpan[] = [];
        spansOf(b.children, { font: b.level === 1 ? FontRole.H1 : b.level === 2 ? FontRole.H2 : FontRole.H3, colour: TextColour.Heading }, spans);
        const w = rich(spans);
        w.props.padding = [b.level === 1 ? 4 : 2, 0, 0, 0];
        return w;
    }
    if (b.type === "paragraph") {
        if (b.children.length === 1 && b.children[0].type === "image") {
            const img = new Widget("image");
            img.props.alt = b.children[0].alt;
            return img;
        }
        const spans: RichSpan[] = [];
        spansOf(b.children, { font: FontRole.Body, colour: TextColour.Text }, spans);
        return rich(spans);
    }
    if (b.type === "code_block") {
        const f = new Widget("frame");
        f.props.style = BoxStyle.Code;
        f.props.padding = 4;
        f.props.clip = true;
        const l = new Widget("label");
        l.props.text = b.text;
        l.props.font = FontRole.Mono;
        l.props.colour = TextColour.Code;
        l.props.wrap = false;
        f.append(l);
        return f;
    }
    if (b.type === "blockquote") {
        const kids: Widget[] = [];
        for (let i = 0; i < b.children.length; i++) kids.push(block(b.children[i], gap));
        const f = new Widget("frame");
        f.props.style = "quote";
        f.props.padding = [0, 0, 0, 10];
        f.append(column(kids, gap));
        return f;
    }
    if (b.type === "list") {
        const rows: Widget[] = [];
        for (let i = 0; i < b.items.length; i++) {
            const row = new Widget("flow");
            row.props.align = "start";
            row.props.gap = 4;
            const marker = new Widget("label");
            marker.props.text = b.ordered ? `${b.start + i}.` : "-";
            marker.props.colour = TextColour.Muted;
            marker.props.width = b.ordered ? 22 : 12;
            marker.props.wrap = false;
            row.append(marker);
            const kids: Widget[] = [];
            const item = b.items[i];
            for (let j = 0; j < item.length; j++) kids.push(block(item[j], gap));
            const body = column(kids, b.tight ? 0 : gap);
            body.props.width = "fill";
            row.append(body);
            rows.push(row);
        }
        return column(rows, b.tight ? 1 : gap);
    }
    if (b.type === "thematic_break") {
        const r = new Widget("rule");
        r.props.padding = [3, 0, 3, 0];
        return r;
    }
    // table
    const g = new Widget("grid");
    const n = b.align.length;
    const widths: string[] = [];
    for (let c = 0; c < n; c++) widths.push(c === n - 1 ? "fill" : "content");
    g.props.columnWidths = widths;
    g.props.gap = 10;
    g.props.rowGap = 2;
    for (let c = 0; c < n; c++) {
        const spans: RichSpan[] = [];
        spansOf(b.head[c], { font: FontRole.Body, colour: TextColour.Heading }, spans);
        g.append(rich(spans, b.align[c]));
    }
    for (let r = 0; r < b.rows.length; r++) {
        for (let c = 0; c < n; c++) {
            const spans: RichSpan[] = [];
            spansOf(b.rows[r][c], { font: FontRole.Body, colour: TextColour.Text }, spans);
            g.append(rich(spans, b.align[c]));
        }
    }
    return g;
}

/** Widgets for a parsed document. */
export function renderDocument(doc: Document, gap: number): Widget {
    const kids: Widget[] = [];
    for (let i = 0; i < doc.children.length; i++) kids.push(block(doc.children[i], gap));
    return column(kids, gap);
}

/** Markdown source (UTF-8 bytes, one character per byte in JavaScript) as widgets. */
export function Markdown(props: { source?: MaybeAccessor<string>; gap?: number; [k: string]: unknown }): Child {
    const gap = typeof props.gap === "number" ? props.gap : 6;
    const parsed = computed(() => parse(read(props.source ?? "")));
    const boxProps: { [k: string]: unknown } = { direction: "column" };
    for (const k in props) if (k !== "source" && k !== "gap" && k !== "children") boxProps[k] = props[k];
    const box = h("flow", boxProps) as Widget;
    effect(() => {
        const p = parsed();
        if (p.ok) box.setChildren([renderDocument(p.doc, gap)]);
        else {
            const l = new Widget("label");
            l.props.text = p.error === undefined ? "markdown error" : `README line ${p.error.line}: ${p.error.message}`;
            l.props.colour = "error";
            box.setChildren([l]);
        }
    });
    return box;
}

intrinsicComponents.markdown = Markdown as IntrinsicComponent;
