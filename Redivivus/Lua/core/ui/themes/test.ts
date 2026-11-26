// The test theme: built-in, needs no game files. Flat rectangles, one-pixel lines and the 5x7
// test font, so its images can be committed as golden files.

import { BoxStyle, ButtonVariant, FontRole, TextColour } from "../../redivivus/enums";
import type { Backend, FontHandle } from "../backend";
import type { Theme } from "../theme";
import type { Widget } from "../widget";

const C = {
    screen: 0x0c1014,
    panel: 0x1a2027,
    panelEdge: 0x56616e,
    dialog: 0x20272f,
    dialogEdge: 0x8c98a6,
    inset: 0x0f1317,
    insetEdge: 0x39424c,
    button: 0x2b3440,
    buttonHover: 0x37424f,
    buttonDown: 0x20262e,
    buttonEdge: 0x8c98a6,
    accent: 0xd4b060,
    check: 0x5ac85a,
    rowHover: 0x2a3440,
    rowSelected: 0x3a3320,
    rule: 0x46505a,
    code: 0x101418,
    quote: 0xd4b060,
    tooltip: 0x05070a,
    tooltipEdge: 0x8c98a6,
    slot: 0x14181c,
    slotGrid: 0x2c333b,
    barBack: 0x101418,
    barFill: 0xb03030,
    caret: 0xe8e8e8,
};

const COLOURS: { [role: string]: number } = {
    text: 0,
    muted: 5,
    link: 3,
    code: 7,
    heading: 4,
    ok: 2,
    warning: 8,
    error: 1,
    accent: 4,
    disabled: 5,
    emphasis: 8,
    strong: 9,
    black: 6,
};

const FONT_ROLES: { [role: string]: string } = {
    body: "test",
    small: "test",
    tiny: "test",
    title: "test2x",
    heading: "test2x",
    h1: "test2x",
    h2: "test2x",
    h3: "test",
    button: "test",
    buttonSmall: "test",
    mono: "test",
};

function box(b: Backend, x: number, y: number, w: number, h: number, fill: number, edge: number, alpha: number): void {
    b.fillRect(x, y, w, h, fill, alpha);
    if (edge >= 0 && w > 1 && h > 1) {
        b.drawLine(x, y, x + w - 1, y, edge, 255);
        b.drawLine(x, y + h - 1, x + w - 1, y + h - 1, edge, 255);
        b.drawLine(x, y, x, y + h - 1, edge, 255);
        b.drawLine(x + w - 1, y, x + w - 1, y + h - 1, edge, 255);
    }
}

class TestTheme implements Theme {
    readonly name = "test";
    private fonts: { [name: string]: FontHandle } = {};
    private backend: Backend | undefined = undefined;
    load(b: Backend): void {
        this.backend = b;
        this.fonts = {};
    }
    font(role: FontRole): FontHandle {
        const name = FONT_ROLES[role] ?? role;
        let h = this.fonts[name];
        if (h === undefined && this.backend !== undefined) {
            h = this.backend.loadFont(name);
            this.fonts[name] = h;
        }
        return h ?? 0;
    }
    colour(role: TextColour): number {
        return COLOURS[role] ?? 0;
    }
    // The stock art's sizes and label rooms, so both themes lay a screen out alike.
    buttonSize(variant: ButtonVariant): [number, number] {
        if (variant === ButtonVariant.Wide) return [272, 35];
        if (variant === ButtonVariant.Medium) return [128, 35];
        if (variant === ButtonVariant.Narrow) return [272, 25];
        if (variant === ButtonVariant.Short) return [135, 25];
        if (variant === ButtonVariant.Cancel) return [96, 32];
        return this.buttonSize(this.fitButton(0));
    }
    buttonRoom(variant: ButtonVariant): number {
        if (variant === ButtonVariant.Wide || variant === ButtonVariant.Narrow) return 232;
        if (variant === ButtonVariant.Medium) return 90;
        if (variant === ButtonVariant.Short) return 100;
        if (variant === ButtonVariant.Cancel) return 60;
        return this.buttonRoom(this.fitButton(0));
    }
    fitButton(labelWidth: number): ButtonVariant {
        return labelWidth <= 100 ? ButtonVariant.Short : ButtonVariant.Narrow;
    }
    buttonFont(height: number): FontRole {
        return height >= 35 ? FontRole.Button : FontRole.ButtonSmall;
    }
    tabVariants(): ButtonVariant[] {
        return [ButtonVariant.Cancel, ButtonVariant.Short];
    }
    checkboxSize(): number {
        return 13;
    }
    scrollbarWidth(): number {
        return 10;
    }
    textFieldHeight(): number {
        return 22;
    }
    barHeight(): number {
        return 12;
    }
    frameInset(style: BoxStyle): number {
        if (style === BoxStyle.Panel || style === BoxStyle.Dialog) return 4;
        if (style === BoxStyle.Inset) return 2;
        return 0;
    }
    cellSize(): number {
        return 29;
    }

    paintScreen(b: Backend, w: number, h: number): void {
        b.fillRect(0, 0, w, h, C.screen, 255);
    }
    paintFrame(b: Backend, w: Widget, style: BoxStyle): void {
        if (style === BoxStyle.Screen) this.paintScreen(b, w.w, w.h);
        else if (style === BoxStyle.Panel) box(b, w.x, w.y, w.w, w.h, C.panel, C.panelEdge, 255);
        else if (style === BoxStyle.Dialog) {
            box(b, w.x, w.y, w.w, w.h, C.dialog, C.dialogEdge, 255);
            box(b, w.x + 2, w.y + 2, w.w - 4, w.h - 4, C.dialog, C.panelEdge, 255);
        } else if (style === BoxStyle.Inset) box(b, w.x, w.y, w.w, w.h, C.inset, C.insetEdge, 255);
        else if (style === BoxStyle.Code) box(b, w.x, w.y, w.w, w.h, C.code, C.insetEdge, 255);
        else if (style === BoxStyle.Quote) this.paintQuoteBar(b, w.x, w.y, w.h);
    }
    paintButton(b: Backend, w: Widget, variant: ButtonVariant, label: string, pressed: boolean, hover: boolean, disabled: boolean): void {
        // The art's size, centred in the box, as the stock theme draws it.
        const [aw, ah] = this.buttonSize(variant);
        const x = w.x + Math.floor((w.w - aw) / 2);
        const y = w.y + Math.floor((w.h - ah) / 2);
        box(b, x, y, aw, ah, pressed ? C.buttonDown : hover ? C.buttonHover : C.button, disabled ? C.insetEdge : C.buttonEdge, 255);
        const font = this.font(this.buttonFont(ah));
        const tw = b.measureText(font, label);
        const lh = b.fontLineHeight(font);
        const off = pressed ? 1 : 0;
        b.drawText(font, label, x + Math.max(0, Math.floor((aw - tw) / 2)) + off, y + Math.floor((ah - lh) / 2) + 1 + off, disabled ? COLOURS.disabled : 0);
    }
    paintCheckbox(b: Backend, x: number, y: number, checked: boolean, hover: boolean, disabled: boolean): void {
        box(b, x, y, 13, 13, C.inset, disabled ? C.insetEdge : hover ? C.accent : C.buttonEdge, 255);
        if (checked) b.fillRect(x + 3, y + 3, 7, 7, disabled ? C.insetEdge : C.check, 255);
    }
    paintBar(b: Backend, x: number, y: number, w: number, h: number, value: number, max: number, rgb: number): void {
        box(b, x, y, w, h, C.barBack, C.insetEdge, 255);
        const inner = Math.max(0, w - 2);
        const fill = max > 0 ? Math.floor((inner * Math.max(0, Math.min(value, max))) / max) : 0;
        if (fill > 0) b.fillRect(x + 1, y + 1, fill, h - 2, rgb >= 0 ? rgb : C.barFill, 255);
    }
    paintScrollbar(b: Backend, x: number, y: number, h: number, offset: number, view: number, content: number): void {
        box(b, x, y, 10, h, C.inset, C.insetEdge, 255);
        const thumb = Math.max(12, Math.floor((h * view) / Math.max(1, content)));
        const travel = h - thumb;
        const pos = content > view ? Math.floor((travel * offset) / (content - view)) : 0;
        box(b, x + 1, y + pos, 8, thumb, C.button, C.buttonEdge, 255);
    }
    paintTab(b: Backend, x: number, y: number, variant: ButtonVariant, label: string, selected: boolean, hover: boolean): void {
        const [w, h] = this.buttonSize(variant);
        box(b, x, y, w, h, selected ? C.dialog : hover ? C.buttonHover : C.button, selected ? C.accent : C.panelEdge, 255);
        const font = this.font(this.buttonFont(h));
        const tw = b.measureText(font, label);
        b.drawText(font, label, x + Math.floor((w - tw) / 2), y + Math.floor((h - b.fontLineHeight(font)) / 2) + 1, selected ? COLOURS.accent : 0);
    }
    paintTextField(b: Backend, w: Widget, focused: boolean): void {
        box(b, w.x, w.y, w.w, w.h, C.inset, focused ? C.accent : C.insetEdge, 255);
    }
    paintTooltip(b: Backend, x: number, y: number, w: number, h: number): void {
        box(b, x, y, w, h, C.tooltip, C.tooltipEdge, 230);
    }
    paintSlot(b: Backend, x: number, y: number, w: number, h: number, cols: number, rows: number): void {
        box(b, x, y, w, h, C.slot, C.insetEdge, 255);
        for (let c = 1; c < cols; c++) b.drawLine(x + Math.floor((w * c) / cols), y + 1, x + Math.floor((w * c) / cols), y + h - 2, C.slotGrid, 255);
        for (let r = 1; r < rows; r++) b.drawLine(x + 1, y + Math.floor((h * r) / rows), x + w - 2, y + Math.floor((h * r) / rows), C.slotGrid, 255);
    }
    paintImagePlaceholder(b: Backend, x: number, y: number, w: number, h: number): void {
        box(b, x, y, w, h, C.inset, C.insetEdge, 255);
        b.drawLine(x, y, x + w - 1, y + h - 1, C.slotGrid, 255);
        b.drawLine(x + w - 1, y, x, y + h - 1, C.slotGrid, 255);
    }
    paintRow(b: Backend, x: number, y: number, w: number, h: number, selected: boolean, _hover: boolean): void {
        b.fillRect(x, y, w, h, selected ? C.rowSelected : C.rowHover, 255);
        if (selected) b.drawLine(x, y, x, y + h - 1, C.accent, 255);
    }
    paintRule(b: Backend, x: number, y: number, w: number): void {
        b.drawLine(x, y, x + w - 1, y, C.rule, 255);
    }
    paintCodeBackground(b: Backend, x: number, y: number, w: number, h: number): void {
        box(b, x, y, w, h, C.code, C.insetEdge, 255);
    }
    paintQuoteBar(b: Backend, x: number, y: number, h: number): void {
        b.fillRect(x, y, 2, h, C.quote, 255);
    }
    paintCaret(b: Backend, x: number, y: number, h: number): void {
        b.fillRect(x, y, 1, h, C.caret, 255);
    }
}

export function testTheme(): Theme {
    return new TestTheme();
}
