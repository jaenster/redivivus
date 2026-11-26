// The host pixel renderer: the Backend drawn into an 8-bit paletted framebuffer, as the game's
// software path does, with the game's blend tables (or the test palette's). Node only.
//
// Engine semantics followed (paths under Source/):
// - A DC6 frame drawn at (x, y) has its left edge at x + offsetX and its bottom row at
//   y + offsetY (D2CMP/Src/CelCmp.cpp:2064-2096). The Backend contract places a frame's box top-left
//   at (x, y), so here a frame lands at (x, y) and its own offsets are not applied (UI art has none).
// - Text: each glyph is the font DC6 frame named by its .tbl entry, drawn with SOLID and the colour
//   remap, then x += the glyph's advance (D2Win/Src/D2WinFont.cpp:390-458). The engine's y is the
//   bottom row of the glyph cell; the contract's y is the line top, so the cell top is at y and
//   ascent is the cell height. Line height is lineHeight * 16 / 10 (the .tbl byte at +0xA).
// - Blends: [dst << 8 | src] through the pl2 tables; a remapped glyph is blend[dst << 8 | remap[src]]
//   (D2DDraw/Src/ddSprite.cpp:108-180, FastCmp.cpp:1267-1320).
// - Colours: the palette index nearest to the RGB by the engine's own metric (D2CMP/Draw.cpp:193).
// - fillRect: the software renderer's four levels (DrawSolidRectAlpha, DirectDraw.cpp:1128):
//   alpha >= 192 solid, >= 128 the 75% table, >= 64 the 50% table, > 0 the 25% table, 0 nothing;
//   and, as DrawSolidRectEx (DirectDraw.cpp:1033) does, never the screen's last column or row.
//   drawLine: Bresenham, both ends, alpha ignored (DirectDraw.cpp:1162).
// - Clipping, as the game's backend does (engine/d2_lua_menu.cpp): a sprite is cut at the scissor
//   (DrawSprite6); a glyph whose ink (its opaque pixels, not its cell, which is wider than its
//   advance) is inside is drawn whole; one whose ink is partly outside is cut only in colour 0 and
//   dropped in any other colour, since the clipped draw takes no palette shift.

import { BlendMode, type Backend, type FontHandle, type SpriteHandle } from "redivivus:ui";
import type { Frame, Sprite } from "./dc6";
import type { Palette } from "./palette";
import type { BuiltFont } from "./testfont";
import type { Indexed } from "./png";

/** Where art comes from: the game's MPQs, or built-in test art. */
export interface AssetSource {
    palette: Palette;
    sprite(path: string): Sprite | undefined;
    font(name: string): BuiltFont | undefined;
    /** Palette index the screen is cleared to. */
    clearIndex: number;
}

interface Rect {
    x0: number;
    y0: number;
    x1: number;
    y1: number;
}

const inkCache = new WeakMap<Frame, Rect>();

/** The box of a frame's opaque pixels, relative to its top-left; empty (x0 >= x1) for a blank one. */
export function inkOf(fr: Frame): Rect {
    const hit = inkCache.get(fr);
    if (hit !== undefined) return hit;
    const r: Rect = { x0: fr.width, y0: fr.height, x1: 0, y1: 0 };
    for (let y = 0; y < fr.height; y++)
        for (let x = 0; x < fr.width; x++) {
            if (fr.mask[y * fr.width + x] === 0) continue;
            if (x < r.x0) r.x0 = x;
            if (x + 1 > r.x1) r.x1 = x + 1;
            if (y < r.y0) r.y0 = y;
            if (y + 1 > r.y1) r.y1 = y + 1;
        }
    inkCache.set(fr, r);
    return r;
}

export class Raster implements Backend {
    width = 0;
    height = 0;
    pixels = new Uint8Array(0);
    private sprites: (Sprite | undefined)[] = [];
    private spritePaths: string[] = [];
    private fonts: BuiltFont[] = [];
    private fontNames: string[] = [];
    private clip: Rect[] = [];
    /** Paths asked for that the source did not have. */
    missing: string[] = [];

    constructor(readonly src: AssetSource) {}

    loadSprite(path: string): SpriteHandle {
        const i = this.spritePaths.indexOf(path);
        if (i >= 0) return this.sprites[i] === undefined ? 0 : i + 1;
        const s = this.src.sprite(path);
        this.spritePaths.push(path);
        this.sprites.push(s);
        if (s === undefined) {
            this.missing.push(path);
            return 0;
        }
        return this.sprites.length;
    }
    private frame(s: SpriteHandle, f: number): Frame | undefined {
        const sp = s > 0 ? this.sprites[s - 1] : undefined;
        return sp === undefined ? undefined : sp.frames[f];
    }
    spriteFrames(s: SpriteHandle): number {
        const sp = s > 0 ? this.sprites[s - 1] : undefined;
        return sp === undefined ? 0 : sp.frames.length;
    }
    spriteWidth(s: SpriteHandle, f: number): number {
        return this.frame(s, f)?.width ?? 0;
    }
    spriteHeight(s: SpriteHandle, f: number): number {
        return this.frame(s, f)?.height ?? 0;
    }
    loadFont(name: string): FontHandle {
        const i = this.fontNames.indexOf(name);
        if (i >= 0) return i + 1;
        const f = this.src.font(name);
        if (f === undefined) {
            this.missing.push(`font ${name}`);
            return 0;
        }
        this.fontNames.push(name);
        this.fonts.push(f);
        return this.fonts.length;
    }
    private font(f: FontHandle): BuiltFont | undefined {
        return f > 0 ? this.fonts[f - 1] : undefined;
    }
    fontLineHeight(f: FontHandle): number {
        return this.font(f)?.lineHeight ?? 0;
    }
    fontAscent(f: FontHandle): number {
        return this.font(f)?.ascent ?? 0;
    }
    hasGlyph(f: FontHandle, code: number): boolean {
        const ft = this.font(f);
        return ft !== undefined && code >= 0 && code < 256 && ft.has[code];
    }
    measureText(f: FontHandle, text: string): number {
        const ft = this.font(f);
        if (ft === undefined) return 0;
        let w = 0;
        for (let i = 0; i < text.length; i++) {
            const c = text.charCodeAt(i);
            w += c < 256 ? ft.advance[c] : ft.advance[0];
        }
        return w;
    }

    beginFrame(w: number, h: number): void {
        if (w !== this.width || h !== this.height) {
            this.width = w;
            this.height = h;
            this.pixels = new Uint8Array(w * h);
        }
        this.pixels.fill(this.src.clearIndex);
        this.clip = [{ x0: 0, y0: 0, x1: w, y1: h }];
    }
    endFrame(): void {
        if (this.clip.length !== 1) throw new Error(`raster: ${this.clip.length - 1} scissor(s) left pushed at endFrame`);
    }
    private top(): Rect {
        return this.clip[this.clip.length - 1];
    }
    pushScissor(x: number, y: number, w: number, h: number): void {
        const c = this.top();
        this.clip.push({ x0: Math.max(c.x0, x), y0: Math.max(c.y0, y), x1: Math.min(c.x1, x + w), y1: Math.min(c.y1, y + h) });
    }
    popScissor(): void {
        if (this.clip.length <= 1) throw new Error("raster: popScissor without push");
        this.clip.pop();
    }

    private table(mode: BlendMode): Uint8Array | undefined {
        const p = this.src.palette;
        if (mode === BlendMode.Trans25) return p.alpha25;
        if (mode === BlendMode.Trans50) return p.alpha50;
        if (mode === BlendMode.Trans75) return p.alpha75;
        if (mode === BlendMode.Additive) return p.additive;
        if (mode === BlendMode.AlphaBlack) return p.multiply;
        return undefined;
    }

    private blit(fr: Frame, x: number, y: number, mode: BlendMode, remap: Uint8Array | undefined, cut: Rect): void {
        const t = this.table(mode);
        const x0 = Math.max(cut.x0, x);
        const y0 = Math.max(cut.y0, y);
        const x1 = Math.min(cut.x1, x + fr.width);
        const y1 = Math.min(cut.y1, y + fr.height);
        for (let py = y0; py < y1; py++) {
            const srow = (py - y) * fr.width;
            const drow = py * this.width;
            for (let px = x0; px < x1; px++) {
                const si = srow + px - x;
                if (fr.mask[si] === 0) continue;
                const s = remap === undefined ? fr.pixels[si] : remap[fr.pixels[si]];
                const di = drow + px;
                this.pixels[di] = t === undefined ? s : t[(this.pixels[di] << 8) | s];
            }
        }
    }

    private placeholder(x: number, y: number, w: number, h: number): void {
        const c = this.src.palette.nearest(0xb040b0);
        this.fillRect(x, y, w, h, 0x301030, 255);
        this.drawLine(x, y, x + w - 1, y + h - 1, 0xb040b0, 255);
        this.drawLine(x + w - 1, y, x, y + h - 1, 0xb040b0, 255);
        void c;
    }

    drawSprite(s: SpriteHandle, frame: number, x: number, y: number, mode: BlendMode): void {
        const fr = this.frame(s, frame);
        if (fr === undefined) {
            this.placeholder(x, y, 16, 16);
            return;
        }
        this.blit(fr, x, y, mode, undefined, this.top());
    }

    drawNineSlice(s: SpriteHandle, frames: number[], x: number, y: number, w: number, h: number, mode: BlendMode): void {
        const f = frames.map((i) => this.frame(s, i));
        if (f.some((v) => v === undefined) || w <= 0 || h <= 0) {
            this.placeholder(x, y, Math.max(1, w), Math.max(1, h));
            return;
        }
        const [tl, tm, tr, ml, mm, mr, bl, bm, br] = f as Frame[];
        const outer = this.top();
        const box: Rect = { x0: Math.max(outer.x0, x), y0: Math.max(outer.y0, y), x1: Math.min(outer.x1, x + w), y1: Math.min(outer.y1, y + h) };
        const left = tl.width;
        const right = tr.width;
        const topH = tl.height;
        const botH = bl.height;
        const inner: Rect = { x0: Math.max(box.x0, x + left), y0: Math.max(box.y0, y + topH), x1: Math.min(box.x1, x + w - right), y1: Math.min(box.y1, y + h - botH) };
        for (let ty = y + topH; ty < y + h - botH; ty += mm.height) for (let tx = x + left; tx < x + w - right; tx += mm.width) this.blit(mm, tx, ty, mode, undefined, inner);
        const hband: Rect = { x0: inner.x0, y0: box.y0, x1: inner.x1, y1: box.y1 };
        for (let tx = x + left; tx < x + w - right; tx += tm.width) {
            this.blit(tm, tx, y, mode, undefined, hband);
            this.blit(bm, tx, y + h - bm.height, mode, undefined, hband);
        }
        const vband: Rect = { x0: box.x0, y0: inner.y0, x1: box.x1, y1: inner.y1 };
        for (let ty = y + topH; ty < y + h - botH; ty += ml.height) {
            this.blit(ml, x, ty, mode, undefined, vband);
            this.blit(mr, x + w - mr.width, ty, mode, undefined, vband);
        }
        this.blit(tl, x, y, mode, undefined, box);
        this.blit(tr, x + w - tr.width, y, mode, undefined, box);
        this.blit(bl, x, y + h - bl.height, mode, undefined, box);
        this.blit(br, x + w - br.width, y + h - br.height, mode, undefined, box);
    }

    fillRect(x: number, y: number, w: number, h: number, rgb: number, alpha: number): void {
        if (alpha <= 0) return;
        const idx = this.src.palette.nearest(rgb);
        const t = alpha >= 192 ? undefined : alpha >= 128 ? this.src.palette.alpha75 : alpha >= 64 ? this.src.palette.alpha50 : this.src.palette.alpha25;
        const c = this.top();
        const x0 = Math.max(c.x0, x);
        const y0 = Math.max(c.y0, y);
        const x1 = Math.min(c.x1, x + w, this.width - 1);
        const y1 = Math.min(c.y1, y + h, this.height - 1);
        for (let py = y0; py < y1; py++) {
            const row = py * this.width;
            for (let px = x0; px < x1; px++) this.pixels[row + px] = t === undefined ? idx : t[(this.pixels[row + px] << 8) | idx];
        }
    }

    drawLine(x0: number, y0: number, x1: number, y1: number, rgb: number, _alpha: number): void {
        const idx = this.src.palette.nearest(rgb);
        const c = this.top();
        let x = x0;
        let y = y0;
        const dx = Math.abs(x1 - x0);
        const dy = -Math.abs(y1 - y0);
        const sx = x0 < x1 ? 1 : -1;
        const sy = y0 < y1 ? 1 : -1;
        let err = dx + dy;
        for (;;) {
            if (x >= c.x0 && x < c.x1 && y >= c.y0 && y < c.y1) this.pixels[y * this.width + x] = idx;
            if (x === x1 && y === y1) break;
            const e2 = 2 * err;
            if (e2 >= dy) {
                err += dy;
                x += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y += sy;
            }
        }
    }

    drawText(f: FontHandle, text: string, x: number, y: number, colour: number): void {
        const ft = this.font(f);
        if (ft === undefined) return;
        const remap = colour > 0 && colour < 13 ? this.src.palette.text[colour] : undefined;
        const cut = this.top();
        let cx = x;
        for (let i = 0; i < text.length; i++) {
            let c = text.charCodeAt(i);
            if (c >= 256) c = 0;
            const fi = ft.frameOf[c];
            const fr = fi >= 0 ? ft.sprite.frames[fi] : undefined;
            if (fr !== undefined) {
                // The engine's y is the cell's bottom row: top + ascent - 1; the frame's bottom lands there.
                const bottom = y + ft.ascent - 1 + fr.offsetY;
                const gx = cx + fr.offsetX;
                const gy = bottom - fr.height + 1;
                const ink = inkOf(fr);
                const inside = gx + ink.x0 >= cut.x0 && gy + ink.y0 >= cut.y0 && gx + ink.x1 <= cut.x1 && gy + ink.y1 <= cut.y1;
                if (ink.x0 < ink.x1 && (inside || remap === undefined)) this.blit(fr, gx, gy, BlendMode.Normal, remap, cut);
            }
            cx += ft.advance[c];
        }
    }

    image(): Indexed {
        return { width: this.width, height: this.height, pixels: this.pixels, rgb: this.src.palette.rgb };
    }
}
