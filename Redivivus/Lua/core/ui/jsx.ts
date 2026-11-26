// The JSX factory. Compile .tsx with jsx "react", jsxFactory "h", jsxFragmentFactory "Fragment"
// (TypeScriptToLua supports exactly that form). A component is a function that runs once; an
// intrinsic element becomes a Widget whose props are set once, or kept up to date by an effect
// when the prop is a function. Props named on<Event>, `render` and `key` are never reactive.
//
// Children: a widget, text (becomes a label), an array, or a function, which becomes a region
// re-evaluated when what it reads changes. In Lua a child expression must not be nil
// (`{cond && <x/>}` with cond nil leaves a hole): use <Show>.

import { effect, untrack, type Accessor } from "./signal";
import { Widget } from "./widget";
import type { Align, BlendMode, BoxStyle, ButtonVariant, Direction, FontRole, Justify, SizeMode, TextAlign, TextColour } from "../redivivus/enums";
import type { Anchor, Size } from "./widget";
import type { UIEvent } from "./widget";

export type Child = Widget | string | number | boolean | null | undefined | Accessor<unknown> | Child[];
export type Component<P> = (props: P) => Child;
type R<T> = T | (() => T);

export type IntrinsicComponent = (props: { [k: string]: unknown }) => Child;

/** Components the factory resolves intrinsic names to (markdown registers itself). */
export const intrinsicComponents: { [name: string]: IntrinsicComponent } = {};

function isHandlerKey(key: string): boolean {
    if (key === "render" || key === "key" || key === "ref") return true;
    if (key.length < 3 || key.charCodeAt(0) !== 111 || key.charCodeAt(1) !== 110) return false;
    const c = key.charCodeAt(2);
    return c >= 65 && c <= 90;
}

function labelOf(text: string): Widget {
    const w = new Widget("label");
    w.props.text = text;
    return w;
}

/** Turns a child value into widgets, appending to out. Functions are called (inside a region's effect). */
export function collect(v: unknown, out: Widget[]): void {
    if (v === undefined || v === null || v === true || v === false) return;
    if (v instanceof Widget) out.push(v);
    else if (typeof v === "string") out.push(labelOf(v));
    else if (typeof v === "number") out.push(labelOf(`${v}`));
    else if (typeof v === "function") collect((v as () => unknown)(), out);
    else if (Array.isArray(v)) for (let i = 0; i < v.length; i++) collect(v[i], out);
}

function appendChild(parent: Widget, c: unknown): void {
    if (typeof c === "function") {
        const region = new Widget("region");
        effect(() => {
            const list: Widget[] = [];
            collect((c as () => unknown)(), list);
            region.setChildren(list);
        });
        parent.append(region);
    } else if (Array.isArray(c)) {
        for (let i = 0; i < c.length; i++) appendChild(parent, c[i]);
    } else {
        const list: Widget[] = [];
        collect(c, list);
        for (let i = 0; i < list.length; i++) parent.append(list[i]);
    }
}

export function Fragment(props: { children?: unknown }): Child {
    return props.children as Child;
}

export function h(type: unknown, props: { [k: string]: unknown } | undefined, ...children: unknown[]): Child {
    if (typeof type === "function") {
        const p: { [k: string]: unknown } = {};
        if (props !== undefined && props !== null) for (const k in props) p[k] = props[k];
        if (children.length === 1) p.children = children[0];
        else if (children.length > 1) p.children = children;
        return untrack(() => (type as (p: unknown) => Child)(p));
    }
    const name = type as string;
    const comp = intrinsicComponents[name];
    if (comp !== undefined) return h(comp, props, ...children);
    const w = new Widget(name);
    let ref: unknown = undefined;
    if (props !== undefined && props !== null) {
        for (const key in props) {
            const v = props[key];
            if (key === "children") continue;
            if (key === "ref") ref = v;
            else if (isHandlerKey(key)) w.props[key] = v;
            else if (typeof v === "function") {
                const get = v as Accessor<unknown>;
                effect(() => w.set(key, get()));
            } else w.props[key] = v;
        }
    }
    for (let i = 0; i < children.length; i++) appendChild(w, children[i]);
    if (typeof ref === "function") (ref as (w: Widget) => void)(w);
    return w;
}

/** One widget from a child: the widget itself, or a stack holding what it gives. */
export function toWidget(c: Child): Widget {
    if (c instanceof Widget) return c;
    const w = new Widget("stack");
    appendChild(w, c);
    return w;
}

// --- typing ---------------------------------------------------------------------------------------
// Callback props say `this: void`: TypeScriptToLua otherwise compiles the arrow given for them
// with a self parameter, and the runtime calls them without one.

interface Box {
    width?: R<Size>;
    height?: R<Size>;
    minWidth?: R<number>;
    maxWidth?: R<number>;
    minHeight?: R<number>;
    maxHeight?: R<number>;
    flex?: R<number>;
    padding?: R<number | number[]>;
    anchor?: R<Anchor>;
    z?: R<number>;
    clip?: R<boolean>;
    visible?: R<boolean>;
    hit?: R<boolean>;
    focusable?: R<boolean>;
    alignSelf?: R<Align>;
    /** A name tests find the widget by. */
    testId?: string;
    ref?: (this: void, w: Widget) => void;
    key?: unknown;
    children?: unknown;
    onClick?: (this: void, e: UIEvent) => void;
    onMouseDown?: (this: void, e: UIEvent) => void;
    onMouseUp?: (this: void, e: UIEvent) => void;
    onMouseMove?: (this: void, e: UIEvent) => void;
    onWheel?: (this: void, e: UIEvent) => void;
    onKeyDown?: (this: void, e: UIEvent) => void;
    onKeyUp?: (this: void, e: UIEvent) => void;
    onText?: (this: void, e: UIEvent) => void;
}

interface Container extends Box {
    gap?: R<number>;
    rowGap?: R<number>;
    align?: R<Align>;
    justify?: R<Justify>;
    /** A fill colour 0xRRGGBB behind the box, and its alpha. */
    bg?: R<number>;
    bgAlpha?: R<number>;
    /** The theme's chrome behind the box. */
    style?: R<BoxStyle>;
    /** A sprite drawn as a nine-slice behind the box. */
    nineSlice?: R<{ path: string; frames: number[] }>;
}

interface TextProps extends Box {
    text?: R<string>;
    font?: R<FontRole>;
    /** A colour the theme names, or a colour code of the game's. */
    colour?: R<number | TextColour>;
    textAlign?: R<TextAlign>;
    wrap?: R<boolean>;
    /** Not wrapping: smaller fonts to try, in order, when the text is wider than the label. */
    fitFonts?: R<FontRole[]>;
    /** Not wrapping: cut text that still does not fit at a glyph and end it with "..." (default true). */
    ellipsis?: R<boolean>;
}

declare global {
    namespace JSX {
        type Element = Child;
        interface ElementChildrenAttribute {
            children: {};
        }
        interface IntrinsicElements {
            frame: Container & { direction?: R<Direction> };
            flow: Container & { direction?: R<Direction>; wrap?: R<boolean> };
            stack: Container;
            grid: Container & { columns?: R<number>; columnWidths?: R<(number | SizeMode)[]> };
            label: TextProps;
            rich: Box & { spans?: R<{ text: string; font: FontRole; colour: TextColour | number }[]>; wrap?: R<boolean>; textAlign?: R<TextAlign> };
            button: Box & { label?: R<string>; variant?: R<ButtonVariant>; disabled?: R<boolean>; pressed?: R<boolean> };
            checkbox: Box & { checked?: R<boolean>; label?: R<string>; disabled?: R<boolean>; font?: R<FontRole>; onChange?: (this: void, checked: boolean) => void };
            bar: Box & { value?: R<number>; max?: R<number>; rgb?: R<number>; text?: R<string> };
            sprite: Box & { path?: R<string>; frame?: R<number>; mode?: R<BlendMode> };
            itemSlot: Box & { cols?: R<number>; rows?: R<number> };
            scrollPane: Container & { wheelStep?: R<number>; onScroll?: (this: void, offset: number) => void };
            textField: Box & { value?: R<string>; placeholder?: R<string>; maxLength?: R<number>; font?: R<FontRole>; onInput?: (this: void, value: string) => void; onSubmit?: (this: void, value: string) => void };
            list: Container & { items?: R<unknown[]>; rowHeight?: R<number>; selected?: R<number>; render?: (this: void, item: any, index: number) => Child; onSelect?: (this: void, index: number) => void };
            tabs: Box & { labels?: R<string[]>; selected?: R<number>; gap?: R<number>; justify?: R<Justify>; onChange?: (this: void, index: number) => void };
            tooltip: Box & { text?: R<string> };
            spacer: Box;
            rule: Box;
            image: Box & { alt?: R<string>; imageWidth?: R<number>; imageHeight?: R<number> };
            markdown: Box & { source?: R<string>; gap?: number };
        }
    }
}
