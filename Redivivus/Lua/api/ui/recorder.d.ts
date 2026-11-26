// Generated from Redivivus/Lua/core/ui/recorder.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
import type { Backend, BlendMode, FontHandle, SpriteHandle } from "./backend";
export declare class Recorder implements Backend {
    private readonly inner;
    lines: string[];
    private spriteNames;
    private fontNames;
    constructor(inner: Backend);
    loadSprite(path: string): SpriteHandle;
    spriteFrames(s: SpriteHandle): number;
    spriteWidth(s: SpriteHandle, f: number): number;
    spriteHeight(s: SpriteHandle, f: number): number;
    loadFont(name: string): FontHandle;
    fontLineHeight(f: FontHandle): number;
    fontAscent(f: FontHandle): number;
    hasGlyph(f: FontHandle, code: number): boolean;
    measureText(f: FontHandle, text: string): number;
    private sprite;
    private font;
    beginFrame(w: number, h: number): void;
    endFrame(): void;
    drawSprite(s: SpriteHandle, frame: number, x: number, y: number, mode: BlendMode): void;
    drawNineSlice(s: SpriteHandle, frames: number[], x: number, y: number, w: number, h: number, mode: BlendMode): void;
    fillRect(x: number, y: number, w: number, h: number, rgb: number, alpha: number): void;
    drawLine(x0: number, y0: number, x1: number, y1: number, rgb: number, alpha: number): void;
    drawText(f: FontHandle, text: string, x: number, y: number, colour: number): void;
    pushScissor(x: number, y: number, w: number, h: number): void;
    popScissor(): void;
    text(): string;
}
