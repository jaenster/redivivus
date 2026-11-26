// A theme decides what widgets look like and the sizes that follow from their art. The runtime
// asks it for fonts, text colours, fixed sizes, and to paint each widget's chrome; text, layout
// and input stay in the runtime. Two themes ship: stock (the game's own art) and test (built-in,
// no game files).

import type { BoxStyle, ButtonVariant, FontRole, TextColour } from "../redivivus/enums";
import type { Backend, FontHandle } from "./backend";
import type { Widget } from "./widget";


export interface Theme {
    readonly name: string;
    /** Loads fonts and sprites through the backend. Called once per backend. */
    load(b: Backend): void;
    font(role: FontRole): FontHandle;
    /** A text colour index (the game's colour codes) for a role. */
    colour(role: TextColour): number;
    /** Size of a button variant's art (Small: the art fitButton picks for an empty label). */
    buttonSize(variant: ButtonVariant): [number, number];
    /** How wide a label the variant's art has room for between its ends. */
    buttonRoom(variant: ButtonVariant): number;
    /** The art a Small button with a label this wide is drawn with: the smallest it fits in. */
    fitButton(labelWidth: number): ButtonVariant;
    /** The font a button's label is drawn in, by the height of its art. */
    buttonFont(height: number): FontRole;
    /** The art tabs may use, smallest first; a row of tabs uses one for all of them. */
    tabVariants(): ButtonVariant[];
    checkboxSize(): number;
    scrollbarWidth(): number;
    textFieldHeight(): number;
    barHeight(): number;
    /** Inset the frame style's border takes from each side. */
    frameInset(style: BoxStyle): number;
    /** Size of one inventory cell. */
    cellSize(): number;

    paintScreen(b: Backend, w: number, h: number): void;
    paintFrame(b: Backend, w: Widget, style: BoxStyle): void;
    /** A button in the variant's art (resolved: never Small) at its own size, centred in the widget's box; the label is already cut to the art's room. */
    paintButton(b: Backend, w: Widget, variant: ButtonVariant, label: string, pressed: boolean, hover: boolean, disabled: boolean): void;
    paintCheckbox(b: Backend, x: number, y: number, checked: boolean, hover: boolean, disabled: boolean): void;
    paintBar(b: Backend, x: number, y: number, w: number, h: number, value: number, max: number, rgb: number): void;
    /** A vertical scrollbar: `offset` of `content` pixels scrolled, `view` pixels visible. */
    paintScrollbar(b: Backend, x: number, y: number, h: number, offset: number, view: number, content: number): void;
    /** A tab in the variant's art at its own size; the label is already cut to the art's room. */
    paintTab(b: Backend, x: number, y: number, variant: ButtonVariant, label: string, selected: boolean, hover: boolean): void;
    paintTextField(b: Backend, w: Widget, focused: boolean): void;
    paintTooltip(b: Backend, x: number, y: number, w: number, h: number): void;
    paintSlot(b: Backend, x: number, y: number, w: number, h: number, cols: number, rows: number): void;
    paintImagePlaceholder(b: Backend, x: number, y: number, w: number, h: number): void;
    paintRow(b: Backend, x: number, y: number, w: number, h: number, selected: boolean, hover: boolean): void;
    paintRule(b: Backend, x: number, y: number, w: number): void;
    paintCodeBackground(b: Backend, x: number, y: number, w: number, h: number): void;
    paintQuoteBar(b: Backend, x: number, y: number, h: number): void;
    paintCaret(b: Backend, x: number, y: number, h: number): void;
}

/** What layout, drawing and input run against. */
export interface Context {
    backend: Backend;
    theme: Theme;
    width: number;
    height: number;
    /** Widget under the pointer, the one holding the pointer, the one with keyboard focus. */
    hover: Widget | undefined;
    capture: Widget | undefined;
    focus: Widget | undefined;
    pointerX: number;
    pointerY: number;
    /** Frames drawn so far (for the caret blink; the recorder sees it as plain data). */
    tick: number;
}
