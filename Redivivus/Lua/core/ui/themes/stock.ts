// The stock Diablo II theme: the front end's own art and fonts. Paths and the rules for using
// them are read from the engine (paths under Source/):
//   D2Launch/Src/MainMenus.cpp:4297-4440   front-end art loaded at boot (buttons, backgrounds, popups)
//   D2Launch/Src/MainMenus.cpp:3480        TileableDialog tiled to fill a 410x245 dialog (FORM_ImageEx)
//   D2Multi/Src/ChatDlg.cpp:1762           BIGMENU\joingameclickbox, the create-game screen's checkbox
//                                          (frame 0 off, 1 on); FrontEnd\clickbox is character
//                                          creation's, drawn for the fechar palette, noise under sky
//   D2Win/Src/D2WinScrollbar.cpp:343-359   joingamescrollbars frames: 0/2 up arrow, 1/3 down, 4 thumb, 5 track (every 10 px)
//   D2Win/Src/D2WinButton.cpp:206-378      button frames (pressed = up + tiles per frame) and the label rule
//   D2Win/Src/D2WinFont.cpp:36-50          the font names
// Art wider than 256 px is cut into 256-px tiles: one state of a button is `tiles` frames side by side.
// Every piece of art here is the front end's and is drawn under its palette (palette\sky, which
// character select and the Mods screen set: D2Launch/Src/CharSel.cpp:2298).
// Buttons are drawn as the game draws them: each art at its own size, never stretched or cut.

import { BlendMode, BoxStyle, ButtonVariant, FontRole, TextColour } from "../../redivivus/enums";
import type { Backend, FontHandle, SpriteHandle } from "../backend";
import { spriteSizes } from "../headless";
import type { Theme } from "../theme";
import type { Widget } from "../widget";

const UI = "data\\global\\ui\\";

/** Every stock asset the theme uses, by role. */
export const stockArt: { [role: string]: string } = {
    screen: UI + "FrontEnd\\gameselectscreenEXP",
    dialog: UI + "FrontEnd\\TileableDialog",
    wide: UI + "FrontEnd\\WideButtonBlank",
    medium: UI + "FrontEnd\\MediumButtonBlank",
    narrow: UI + "FrontEnd\\NarrowButtonBlank",
    short: UI + "CharSelect\\ShortButtonBlank",
    cancel: UI + "FrontEnd\\CancelButtonBlank",
    checkbox: UI + "BIGMENU\\joingameclickbox",
    scrollbar: UI + "FrontEnd\\joingamescrollbars",
    textField: UI + "FrontEnd\\textbox2",
};

// Frame sizes of that art (dimensions only), so headless runs lay out as the real art does.
spriteSizes[stockArt.screen] = [12, 256, 256, 256, 256, 256, 256, 32, 256, 256, 256, 256, 256, 256, 256, 32, 256, 256, 88, 256, 88, 256, 88, 32, 88];
spriteSizes[stockArt.dialog] = [1, 256, 256];
spriteSizes[stockArt.wide] = [4, 256, 35, 16, 35, 256, 35, 16, 35];
spriteSizes[stockArt.medium] = [2, 128, 35];
spriteSizes[stockArt.narrow] = [4, 256, 25, 16, 25, 256, 25, 16, 25];
spriteSizes[stockArt.short] = [2, 135, 25];
spriteSizes[stockArt.cancel] = [2, 96, 32];
spriteSizes[stockArt.checkbox] = [2, 15, 16];
spriteSizes[stockArt.scrollbar] = [6, 12, 14];
spriteSizes[stockArt.textField] = [1, 169, 26];

/**
 * The label room of each button art: the width between its end pieces (knots and gems), measured
 * on the art; a label wider than this is cut with an ellipsis rather than drawn over the ends.
 */
const BUTTON_ROOM: { [variant: string]: number } = {
    wide: 232,
    medium: 90,
    narrow: 232,
    short: 100,
    cancel: 60,
};

/** The art a Small button picks from, smallest first: the 25-pixel-high family, so a column of them lines up. */
const SMALL_ART = [ButtonVariant.Short, ButtonVariant.Narrow];
/** The art a row of tabs picks from, smallest first. */
const TAB_ART = [ButtonVariant.Cancel, ButtonVariant.Short];

function isArt(variant: string): boolean {
    return variant === ButtonVariant.Wide || variant === ButtonVariant.Medium || variant === ButtonVariant.Narrow || variant === ButtonVariant.Short || variant === ButtonVariant.Cancel;
}

/** Font roles to the engine's font names (D2Win/Src/D2WinFont.cpp:36-50). */
export const stockFonts: { [role: string]: string } = {
    body: "font16",
    small: "font8",
    tiny: "font6",
    title: "font42",
    heading: "font30",
    h1: "font30",
    h2: "font24",
    h3: "fontexocet10",
    button: "fontexocet10",
    buttonSmall: "fontridiculous",
    // The game has no fixed-width font; code is told apart by colour.
    mono: "font16",
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

// Colours for rectangles and lines, close to palette entries of act1's pal.pl2.
const C = {
    black: 0x000000,
    edge: 0x6c5c3c,
    edgeDark: 0x2c2418,
    gold: 0xc4a46c,
    grey: 0x484848,
    rowHover: 0x302c24,
    rowSelected: 0x584c30,
    slot: 0x0c0c0c,
    slotGrid: 0x282420,
    bar: 0xa01010,
    white: 0xd0d0d0,
};

const TILE = 256;

class StockTheme implements Theme {
    readonly name = "stock";
    private b: Backend | undefined = undefined;
    private sprites: { [role: string]: SpriteHandle } = {};
    private fonts: { [name: string]: FontHandle } = {};

    load(b: Backend): void {
        this.b = b;
        this.sprites = {};
        this.fonts = {};
    }
    private sprite(role: string): SpriteHandle {
        let h = this.sprites[role];
        if (h === undefined && this.b !== undefined) {
            h = this.b.loadSprite(stockArt[role]);
            this.sprites[role] = h;
        }
        return h ?? 0;
    }
    font(role: FontRole): FontHandle {
        const name = stockFonts[role] ?? role;
        let h = this.fonts[name];
        if (h === undefined && this.b !== undefined) {
            h = this.b.loadFont(name);
            this.fonts[name] = h;
        }
        return h ?? 0;
    }
    colour(role: TextColour): number {
        return COLOURS[role] ?? 0;
    }

    /** Tiles per state and the full size of one state of a multi-tile sprite. */
    private art(role: string): [number, number, number] {
        const b = this.b;
        const s = this.sprite(role);
        if (b === undefined || s === 0) return [1, 0, 0];
        const frames = b.spriteFrames(s);
        let tiles = 1;
        while (tiles < frames && b.spriteWidth(s, tiles - 1) >= TILE) tiles++;
        if (b.spriteWidth(s, 0) < TILE) tiles = 1;
        let w = 0;
        for (let i = 0; i < tiles; i++) w += b.spriteWidth(s, i);
        return [tiles, w, b.spriteHeight(s, 0)];
    }

    /** One state of a tiled sprite, drawn at (x, y). */
    private drawState(b: Backend, role: string, state: number, x: number, y: number, mode: BlendMode): void {
        const s = this.sprite(role);
        const a = this.art(role);
        let cx = x;
        for (let i = 0; i < a[0]; i++) {
            const f = state * a[0] + i;
            b.drawSprite(s, f, cx, y, mode);
            cx += b.spriteWidth(s, f);
        }
    }

    /**
     * A state of horizontal art fitted to width w (the text field only; buttons are never fitted): narrower, the left half from the art's start and
     * the right half from its end; wider, both ends as caps and the middle repeated. Each piece is
     * scissored, so caps stay intact. Art is never stretched.
     */
    private drawFitted(b: Backend, role: string, state: number, x: number, y: number, w: number, h: number, mode: BlendMode): void {
        const a = this.art(role);
        if (w === a[1]) {
            this.drawState(b, role, state, x, y, mode);
            return;
        }
        if (w < a[1]) {
            const half = Math.floor(w / 2);
            b.pushScissor(x, y, half, h);
            this.drawState(b, role, state, x, y, mode);
            b.popScissor();
            b.pushScissor(x + half, y, w - half, h);
            this.drawState(b, role, state, x + w - a[1], y, mode);
            b.popScissor();
            return;
        }
        // Wider than the art: a quarter of it as each cap, its middle repeated between them.
        const cap = Math.floor(a[1] / 4);
        const mid = a[1] - cap * 2;
        b.pushScissor(x, y, cap, h);
        this.drawState(b, role, state, x, y, mode);
        b.popScissor();
        b.pushScissor(x + w - cap, y, cap, h);
        this.drawState(b, role, state, x + w - a[1], y, mode);
        b.popScissor();
        // Each repeat only shows its own middle piece: drawn whole, its left cap would land inside
        // the middle as a seam.
        b.pushScissor(x + cap, y, w - cap * 2, h);
        for (let mx = x + cap; mx < x + w - cap; mx += mid) {
            b.pushScissor(mx, y, mid, h);
            this.drawState(b, role, state, mx - cap, y, mode);
            b.popScissor();
        }
        b.popScissor();
    }

    buttonSize(variant: ButtonVariant): [number, number] {
        const a = this.art(isArt(variant) ? variant : this.fitButton(0));
        return [a[1], a[2]];
    }
    buttonRoom(variant: ButtonVariant): number {
        return BUTTON_ROOM[isArt(variant) ? variant : this.fitButton(0)] ?? 0;
    }
    fitButton(labelWidth: number): ButtonVariant {
        for (let i = 0; i < SMALL_ART.length; i++) if (labelWidth <= BUTTON_ROOM[SMALL_ART[i]]) return SMALL_ART[i];
        return SMALL_ART[SMALL_ART.length - 1];
    }
    buttonFont(height: number): FontRole {
        // D2Win/Src/D2WinButton.cpp:206-378: FontExocet10 when the art is 35 high or more, else FontRidiculous.
        return height >= 35 ? FontRole.Button : FontRole.ButtonSmall;
    }
    tabVariants(): ButtonVariant[] {
        return TAB_ART;
    }
    checkboxSize(): number {
        const b = this.b;
        const s = this.sprite("checkbox");
        return b === undefined ? 15 : Math.max(b.spriteWidth(s, 0), b.spriteHeight(s, 0));
    }
    scrollbarWidth(): number {
        const b = this.b;
        return b === undefined ? 12 : b.spriteWidth(this.sprite("scrollbar"), 0);
    }
    textFieldHeight(): number {
        return this.art("textField")[2];
    }
    barHeight(): number {
        return 10;
    }
    frameInset(style: BoxStyle): number {
        if (style === BoxStyle.Panel || style === BoxStyle.Dialog) return 6;
        if (style === BoxStyle.Inset) return 2;
        return 0;
    }
    cellSize(): number {
        return 29;
    }

    paintScreen(b: Backend, w: number, h: number): void {
        b.fillRect(0, 0, w, h, C.black, 255);
        // The 800x600 front-end background, centred: 4x3 tiles (256, 256, 256, 32 by 256, 256, 88).
        const s = this.sprite("screen");
        if (s === 0) return;
        let total = 0;
        for (let i = 0; i < 4; i++) total += b.spriteWidth(s, i);
        let totalH = 0;
        for (let r = 0; r < 3; r++) totalH += b.spriteHeight(s, r * 4);
        const ox = Math.floor((w - total) / 2);
        const oy = Math.floor((h - totalH) / 2);
        let y = oy;
        for (let r = 0; r < 3; r++) {
            let x = ox;
            for (let c = 0; c < 4; c++) {
                b.drawSprite(s, r * 4 + c, x, y, BlendMode.Normal);
                x += b.spriteWidth(s, c);
            }
            y += b.spriteHeight(s, r * 4);
        }
    }

    /** One piece of the dialog art: the sprite drawn at (sx, sy), cut to the rectangle. */
    private piece(b: Backend, s: SpriteHandle, sx: number, sy: number, x: number, y: number, w: number, h: number): void {
        if (w <= 0 || h <= 0) return;
        b.pushScissor(x, y, w, h);
        b.drawSprite(s, 0, sx, sy, BlendMode.Normal);
        b.popScissor();
    }

    /**
     * TileableDialog (256x256, a 2-pixel gold border round a dark texture) fitted to any box: the
     * interior tiled without its border, the edges tiled along the sides, the corners once. The
     * engine fills a 410x245 dialog from it (MainMenus.cpp:3480); how it hides the seams there is
     * not read yet, so this cut is ours.
     */
    private dialogBox(b: Backend, x: number, y: number, w: number, h: number): void {
        const s = this.sprite("dialog");
        const t = b.spriteWidth(s, 0);
        const th = b.spriteHeight(s, 0);
        const e = 2;
        if (t <= e * 2 || th <= e * 2 || w < e * 2 || h < e * 2) return;
        const sw = t - e * 2;
        const sh = th - e * 2;
        for (let y0 = y + e; y0 < y + h - e; y0 += sh) {
            const segH = Math.min(sh, y + h - e - y0);
            for (let x0 = x + e; x0 < x + w - e; x0 += sw) this.piece(b, s, x0 - e, y0 - e, x0, y0, Math.min(sw, x + w - e - x0), segH);
            this.piece(b, s, x, y0 - e, x, y0, e, segH);
            this.piece(b, s, x + w - t, y0 - e, x + w - e, y0, e, segH);
        }
        for (let x0 = x + e; x0 < x + w - e; x0 += sw) {
            const segW = Math.min(sw, x + w - e - x0);
            this.piece(b, s, x0 - e, y, x0, y, segW, e);
            this.piece(b, s, x0 - e, y + h - th, x0, y + h - e, segW, e);
        }
        this.piece(b, s, x, y, x, y, e, e);
        this.piece(b, s, x + w - t, y, x + w - e, y, e, e);
        this.piece(b, s, x, y + h - th, x, y + h - e, e, e);
        this.piece(b, s, x + w - t, y + h - th, x + w - e, y + h - e, e, e);
    }

    private border(b: Backend, x: number, y: number, w: number, h: number, rgb: number): void {
        if (w < 2 || h < 2) return;
        b.drawLine(x, y, x + w - 1, y, rgb, 255);
        b.drawLine(x, y + h - 1, x + w - 1, y + h - 1, rgb, 255);
        b.drawLine(x, y, x, y + h - 1, rgb, 255);
        b.drawLine(x + w - 1, y, x + w - 1, y + h - 1, rgb, 255);
    }

    paintFrame(b: Backend, w: Widget, style: BoxStyle): void {
        if (style === BoxStyle.Screen) this.paintScreen(b, w.w, w.h);
        else if (style === BoxStyle.Panel || style === BoxStyle.Dialog) this.dialogBox(b, w.x, w.y, w.w, w.h); else if (style === BoxStyle.Inset) {
            // Popup fills are black at the engine's lighter transparency (D2Win/Src/D2WinPopup.cpp:173).
            b.fillRect(w.x, w.y, w.w, w.h, C.black, 192);
            this.border(b, w.x, w.y, w.w, w.h, C.grey);
        } else if (style === BoxStyle.Code) {
            b.fillRect(w.x, w.y, w.w, w.h, C.black, 192);
            this.border(b, w.x, w.y, w.w, w.h, C.edgeDark);
        } else if (style === BoxStyle.Quote) this.paintQuoteBar(b, w.x, w.y, w.h);
    }

    /** A button's label as D2WinButton draws it (D2WinButton.cpp:206-378), in art h high at (x, y). */
    private buttonLabel(b: Backend, label: string, x: number, y: number, w: number, h: number, pressed: boolean, colour: number): void {
        // FontExocet10 when h >= 35, else FontRidiculous; x = left + max(0, (w - textW) / 2); baseline
        // y = bottom - (h - lineH) / 2 + off, off 4 when h >= 35, 3 when h == 32, 2 when h > 20, else 1;
        // pressed moves it 2 left and 2 down. lineH is the .tbl line height, which is
        // fontLineHeight * 10 / 16 rounded up.
        const font = this.font(this.buttonFont(h));
        const tw = b.measureText(font, label);
        const lineH = Math.ceil((b.fontLineHeight(font) * 10) / 16);
        const off = h >= 35 ? 4 : h === 32 ? 3 : h > 20 ? 2 : 1;
        const bottom = y + h - 1;
        const baseline = bottom - Math.floor((h - lineH) / 2) + off + (pressed ? 2 : 0);
        b.drawText(font, label, x + Math.max(0, Math.floor((w - tw) / 2)) - (pressed ? 2 : 0), baseline - b.fontAscent(font) + 1, colour);
    }

    paintButton(b: Backend, w: Widget, variant: ButtonVariant, label: string, pressed: boolean, _hover: boolean, disabled: boolean): void {
        const a = this.art(variant);
        // The art at its own size, centred in the box when the box was given another size.
        const x = w.x + Math.floor((w.w - a[1]) / 2);
        const y = w.y + Math.floor((w.h - a[2]) / 2);
        // Disabled buttons draw with mode 1, TRANSPARENT (D2Win/Src/D2WinButton.cpp).
        this.drawState(b, variant, pressed ? 1 : 0, x, y, disabled ? BlendMode.Trans50 : BlendMode.Normal);
        this.buttonLabel(b, label, x, y, a[1], a[2], pressed, disabled ? COLOURS.disabled : 0);
    }

    paintCheckbox(b: Backend, x: number, y: number, checked: boolean, _hover: boolean, disabled: boolean): void {
        b.drawSprite(this.sprite("checkbox"), checked ? 1 : 0, x, y, disabled ? BlendMode.Trans50 : BlendMode.Normal);
    }

    paintBar(b: Backend, x: number, y: number, w: number, h: number, value: number, max: number, rgb: number): void {
        b.fillRect(x, y, w, h, C.black, 255);
        this.border(b, x, y, w, h, C.edge);
        const inner = Math.max(0, w - 2);
        const fill = max > 0 ? Math.floor((inner * Math.max(0, Math.min(value, max))) / max) : 0;
        if (fill > 0) b.fillRect(x + 1, y + 1, fill, h - 2, rgb >= 0 ? rgb : C.bar, 255);
    }

    paintScrollbar(b: Backend, x: number, y: number, h: number, offset: number, view: number, content: number): void {
        const s = this.sprite("scrollbar");
        const ah = b.spriteHeight(s, 0);
        // Track every 10 px between the arrows, then the arrows, then the thumb (D2WinScrollbar.cpp:343-359).
        b.pushScissor(x, y + ah, 12, Math.max(0, h - ah * 2));
        for (let ty = y + ah; ty < y + h - ah; ty += 10) b.drawSprite(s, 5, x, ty, BlendMode.Normal);
        b.popScissor();
        b.drawSprite(s, 0, x, y, BlendMode.Normal);
        b.drawSprite(s, 1, x, y + h - ah, BlendMode.Normal);
        const travel = Math.max(0, h - ah * 3);
        const pos = content > view ? Math.floor((travel * offset) / (content - view)) : 0;
        b.drawSprite(s, 4, x, y + ah + pos, BlendMode.Normal);
    }

    paintTab(b: Backend, x: number, y: number, variant: ButtonVariant, label: string, selected: boolean, _hover: boolean): void {
        // A tab is a button: the selected one shows the pressed state, its label in the accent colour.
        const a = this.art(variant);
        this.drawState(b, variant, selected ? 1 : 0, x, y, BlendMode.Normal);
        this.buttonLabel(b, label, x, y, a[1], a[2], selected, selected ? COLOURS.accent : 0);
    }

    paintTextField(b: Backend, w: Widget, focused: boolean): void {
        this.drawFitted(b, "textField", 0, w.x, w.y, w.w, w.h, BlendMode.Normal);
        if (focused) this.border(b, w.x, w.y, w.w, w.h, C.edge);
    }

    paintTooltip(b: Backend, x: number, y: number, w: number, h: number): void {
        b.fillRect(x, y, w, h, C.black, 192);
    }

    paintSlot(b: Backend, x: number, y: number, w: number, h: number, cols: number, rows: number): void {
        b.fillRect(x, y, w, h, C.slot, 192);
        this.border(b, x, y, w, h, C.edgeDark);
        for (let c = 1; c < cols; c++) b.drawLine(x + Math.floor((w * c) / cols), y + 1, x + Math.floor((w * c) / cols), y + h - 2, C.slotGrid, 255);
        for (let r = 1; r < rows; r++) b.drawLine(x + 1, y + Math.floor((h * r) / rows), x + w - 2, y + Math.floor((h * r) / rows), C.slotGrid, 255);
    }

    paintImagePlaceholder(b: Backend, x: number, y: number, w: number, h: number): void {
        // A sunken tile in the frame colours, so a missing picture reads as a slot, not a hole.
        b.fillRect(x, y, w, h, C.rowHover, 255);
        this.border(b, x, y, w, h, C.edgeDark);
        this.border(b, x + 1, y + 1, w - 2, h - 2, C.edge);
    }

    paintRow(b: Backend, x: number, y: number, w: number, h: number, selected: boolean, _hover: boolean): void {
        b.fillRect(x, y, w, h, selected ? C.rowSelected : C.rowHover, selected ? 160 : 128);
    }

    paintRule(b: Backend, x: number, y: number, w: number): void {
        b.drawLine(x, y, x + w - 1, y, C.edge, 255);
    }

    paintCodeBackground(b: Backend, x: number, y: number, w: number, h: number): void {
        b.fillRect(x, y, w, h, C.black, 192);
    }

    paintQuoteBar(b: Backend, x: number, y: number, h: number): void {
        b.fillRect(x, y, 2, h, C.gold, 255);
    }

    paintCaret(b: Backend, x: number, y: number, h: number): void {
        b.fillRect(x, y, 1, h, C.white, 255);
    }
}

export function stockTheme(): Theme {
    return new StockTheme();
}
