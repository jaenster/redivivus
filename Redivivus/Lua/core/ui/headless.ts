// Metrics without art: what the recorder asks when no renderer is behind it (tests, and the Lua
// build under LuaJIT). The test theme's fonts are defined here exactly as the host renderer draws
// them (host/testfont.ts: 5x7 glyphs, scale 1 or 2). Other fonts and sprites get fixed stand-in
// metrics, so a stock-theme draw list is deterministic headless but says nothing about pixels.

import type { Backend, BlendMode, FontHandle, SpriteHandle } from "./backend";

export interface FontMetrics {
    name: string;
    advance: number;
    lineHeight: number;
    ascent: number;
    first: number;
    last: number;
}

/** The test theme's fonts: the built-in 5x7 glyphs at scale 1 and 2, codes 32..126. */
export const testFonts: FontMetrics[] = [
    { name: "test", advance: 6, lineHeight: 11, ascent: 8, first: 32, last: 126 },
    { name: "test2x", advance: 12, lineHeight: 20, ascent: 15, first: 32, last: 126 },
];

/**
 * Stand-in sizes for sprites by path: [frames, width, height] for uniform frames, or [frames, w0,
 * h0, w1, h1, ...] per frame. Unknown paths are 1 frame of 32x32.
 */
export const spriteSizes: { [path: string]: number[] } = {};

export class HeadlessMetrics implements Backend {
    private fonts: FontMetrics[] = [];
    private sprites: string[] = [];
    loadSprite(path: string): SpriteHandle {
        for (let i = 0; i < this.sprites.length; i++) if (this.sprites[i] === path) return i + 1;
        this.sprites.push(path);
        return this.sprites.length;
    }
    private size(s: SpriteHandle): number[] {
        if (s <= 0 || s > this.sprites.length) return [0, 0, 0];
        const known = spriteSizes[this.sprites[s - 1]];
        return known !== undefined ? known : [1, 32, 32];
    }
    spriteFrames(s: SpriteHandle): number {
        return this.size(s)[0];
    }
    private dim(s: SpriteHandle, f: number, k: number): number {
        const z = this.size(s);
        if (f < 0 || f >= z[0]) return 0;
        const i = z.length > 3 ? 1 + f * 2 + k : 1 + k;
        return i < z.length ? z[i] : 0;
    }
    spriteWidth(s: SpriteHandle, f: number): number {
        return this.dim(s, f, 0);
    }
    spriteHeight(s: SpriteHandle, f: number): number {
        return this.dim(s, f, 1);
    }
    loadFont(name: string): FontHandle {
        for (let i = 0; i < this.fonts.length; i++) if (this.fonts[i].name === name) return i + 1;
        let m: FontMetrics | undefined = undefined;
        for (let i = 0; i < testFonts.length; i++) if (testFonts[i].name === name) m = testFonts[i];
        if (m === undefined) {
            // A stand-in for an engine font: a fixed advance from the size in its name.
            let size = 0;
            for (let i = 0; i < name.length; i++) {
                const c = name.charCodeAt(i);
                if (c >= 48 && c <= 57) size = size * 10 + (c - 48);
            }
            if (size <= 0) size = 12;
            m = { name, advance: Math.max(4, Math.floor(size / 2)), lineHeight: size, ascent: Math.floor((size * 3) / 4), first: 32, last: 255 };
        }
        this.fonts.push(m);
        return this.fonts.length;
    }
    private font(f: FontHandle): FontMetrics {
        return this.fonts[f - 1];
    }
    fontLineHeight(f: FontHandle): number {
        return f <= 0 ? 0 : this.font(f).lineHeight;
    }
    fontAscent(f: FontHandle): number {
        return f <= 0 ? 0 : this.font(f).ascent;
    }
    hasGlyph(f: FontHandle, code: number): boolean {
        if (f <= 0) return false;
        const m = this.font(f);
        return code >= m.first && code <= m.last;
    }
    measureText(f: FontHandle, text: string): number {
        return f <= 0 ? 0 : text.length * this.font(f).advance;
    }
    beginFrame(_w: number, _h: number): void {}
    endFrame(): void {}
    drawSprite(_s: SpriteHandle, _f: number, _x: number, _y: number, _m: BlendMode): void {}
    drawNineSlice(_s: SpriteHandle, _fr: number[], _x: number, _y: number, _w: number, _h: number, _m: BlendMode): void {}
    fillRect(_x: number, _y: number, _w: number, _h: number, _rgb: number, _a: number): void {}
    drawLine(_x0: number, _y0: number, _x1: number, _y1: number, _rgb: number, _a: number): void {}
    drawText(_f: FontHandle, _t: string, _x: number, _y: number, _c: number): void {}
    pushScissor(_x: number, _y: number, _w: number, _h: number): void {}
    popScissor(): void {}
}
