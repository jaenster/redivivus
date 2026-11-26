// Generated from Redivivus/Lua/core/ui/headless.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
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
export declare const testFonts: FontMetrics[];
/**
 * Stand-in sizes for sprites by path: [frames, width, height] for uniform frames, or [frames, w0,
 * h0, w1, h1, ...] per frame. Unknown paths are 1 frame of 32x32.
 */
export declare const spriteSizes: {
    [path: string]: number[];
};
export declare class HeadlessMetrics implements Backend {
    private fonts;
    private sprites;
    loadSprite(path: string): SpriteHandle;
    private size;
    spriteFrames(s: SpriteHandle): number;
    private dim;
    spriteWidth(s: SpriteHandle, f: number): number;
    spriteHeight(s: SpriteHandle, f: number): number;
    loadFont(name: string): FontHandle;
    private font;
    fontLineHeight(f: FontHandle): number;
    fontAscent(f: FontHandle): number;
    hasGlyph(f: FontHandle, code: number): boolean;
    measureText(f: FontHandle, text: string): number;
    beginFrame(_w: number, _h: number): void;
    endFrame(): void;
    drawSprite(_s: SpriteHandle, _f: number, _x: number, _y: number, _m: BlendMode): void;
    drawNineSlice(_s: SpriteHandle, _fr: number[], _x: number, _y: number, _w: number, _h: number, _m: BlendMode): void;
    fillRect(_x: number, _y: number, _w: number, _h: number, _rgb: number, _a: number): void;
    drawLine(_x0: number, _y0: number, _x1: number, _y1: number, _rgb: number, _a: number): void;
    drawText(_f: FontHandle, _t: string, _x: number, _y: number, _c: number): void;
    pushScissor(_x: number, _y: number, _w: number, _h: number): void;
    popScissor(): void;
}
