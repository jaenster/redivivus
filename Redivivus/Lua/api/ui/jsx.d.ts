// Generated from Redivivus/Lua/core/ui/jsx.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
import { type Accessor } from "./signal";
import { Widget } from "./widget";
import type { Align, BlendMode, BoxStyle, ButtonVariant, Direction, FontRole, Justify, SizeMode, TextAlign, TextColour } from "../redivivus/enums";
import type { Anchor, Size } from "./widget";
import type { UIEvent } from "./widget";
export type Child = Widget | string | number | boolean | null | undefined | Accessor<unknown> | Child[];
export type Component<P> = (props: P) => Child;
type R<T> = T | (() => T);
export type IntrinsicComponent = (props: {
    [k: string]: unknown;
}) => Child;
/** Components the factory resolves intrinsic names to (markdown registers itself). */
export declare const intrinsicComponents: {
    [name: string]: IntrinsicComponent;
};
/** Turns a child value into widgets, appending to out. Functions are called (inside a region's effect). */
export declare function collect(v: unknown, out: Widget[]): void;
export declare function Fragment(props: {
    children?: unknown;
}): Child;
export declare function h(type: unknown, props: {
    [k: string]: unknown;
} | undefined, ...children: unknown[]): Child;
/** One widget from a child: the widget itself, or a stack holding what it gives. */
export declare function toWidget(c: Child): Widget;
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
    nineSlice?: R<{
        path: string;
        frames: number[];
    }>;
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
            frame: Container & {
                direction?: R<Direction>;
            };
            flow: Container & {
                direction?: R<Direction>;
                wrap?: R<boolean>;
            };
            stack: Container;
            grid: Container & {
                columns?: R<number>;
                columnWidths?: R<(number | SizeMode)[]>;
            };
            label: TextProps;
            rich: Box & {
                spans?: R<{
                    text: string;
                    font: FontRole;
                    colour: TextColour | number;
                }[]>;
                wrap?: R<boolean>;
                textAlign?: R<TextAlign>;
            };
            button: Box & {
                label?: R<string>;
                variant?: R<ButtonVariant>;
                disabled?: R<boolean>;
                pressed?: R<boolean>;
            };
            checkbox: Box & {
                checked?: R<boolean>;
                label?: R<string>;
                disabled?: R<boolean>;
                font?: R<FontRole>;
                onChange?: (this: void, checked: boolean) => void;
            };
            bar: Box & {
                value?: R<number>;
                max?: R<number>;
                rgb?: R<number>;
                text?: R<string>;
            };
            sprite: Box & {
                path?: R<string>;
                frame?: R<number>;
                mode?: R<BlendMode>;
            };
            itemSlot: Box & {
                cols?: R<number>;
                rows?: R<number>;
            };
            scrollPane: Container & {
                wheelStep?: R<number>;
                onScroll?: (this: void, offset: number) => void;
            };
            textField: Box & {
                value?: R<string>;
                placeholder?: R<string>;
                maxLength?: R<number>;
                font?: R<FontRole>;
                onInput?: (this: void, value: string) => void;
                onSubmit?: (this: void, value: string) => void;
            };
            list: Container & {
                items?: R<unknown[]>;
                rowHeight?: R<number>;
                selected?: R<number>;
                render?: (this: void, item: any, index: number) => Child;
                onSelect?: (this: void, index: number) => void;
            };
            tabs: Box & {
                labels?: R<string[]>;
                selected?: R<number>;
                gap?: R<number>;
                justify?: R<Justify>;
                onChange?: (this: void, index: number) => void;
            };
            tooltip: Box & {
                text?: R<string>;
            };
            spacer: Box;
            rule: Box;
            image: Box & {
                alt?: R<string>;
                imageWidth?: R<number>;
                imageHeight?: R<number>;
            };
            markdown: Box & {
                source?: R<string>;
                gap?: number;
            };
        }
    }
}
export {};
