// The headless backend: every draw call written down as one line of text, metrics answered by
// another backend (the built-in test metrics, or a real renderer). The draw list is what tests
// compare between Node and LuaJIT, so numbers are printed as integers and names as loaded.

import type { Backend, BlendMode, FontHandle, SpriteHandle } from "./backend";

function hex(rgb: number): string {
    const digits = "0123456789abcdef";
    let s = "";
    let v = rgb < 0 ? 0 : rgb;
    for (let i = 0; i < 6; i++) {
        const d = v % 16;
        s = digits.substring(d, d + 1) + s;
        v = Math.floor(v / 16);
    }
    return s;
}

export class Recorder implements Backend {
    lines: string[] = [];
    private spriteNames: { [h: number]: string } = {};
    private fontNames: { [h: number]: string } = {};
    constructor(private readonly inner: Backend) {}

    loadSprite(path: string): SpriteHandle {
        const h = this.inner.loadSprite(path);
        this.spriteNames[h] = path;
        return h;
    }
    spriteFrames(s: SpriteHandle): number {
        return this.inner.spriteFrames(s);
    }
    spriteWidth(s: SpriteHandle, f: number): number {
        return this.inner.spriteWidth(s, f);
    }
    spriteHeight(s: SpriteHandle, f: number): number {
        return this.inner.spriteHeight(s, f);
    }
    loadFont(name: string): FontHandle {
        const h = this.inner.loadFont(name);
        this.fontNames[h] = name;
        return h;
    }
    fontLineHeight(f: FontHandle): number {
        return this.inner.fontLineHeight(f);
    }
    fontAscent(f: FontHandle): number {
        return this.inner.fontAscent(f);
    }
    hasGlyph(f: FontHandle, code: number): boolean {
        return this.inner.hasGlyph(f, code);
    }
    measureText(f: FontHandle, text: string): number {
        return this.inner.measureText(f, text);
    }
    private sprite(h: SpriteHandle): string {
        const n = this.spriteNames[h];
        return n === undefined ? `#${h}` : n;
    }
    private font(h: FontHandle): string {
        const n = this.fontNames[h];
        return n === undefined ? `#${h}` : n;
    }
    beginFrame(w: number, h: number): void {
        this.lines.push(`begin ${w} ${h}`);
        this.inner.beginFrame(w, h);
    }
    endFrame(): void {
        this.lines.push("end");
        this.inner.endFrame();
    }
    drawSprite(s: SpriteHandle, frame: number, x: number, y: number, mode: BlendMode): void {
        this.lines.push(`sprite ${this.sprite(s)} ${frame} ${x} ${y} ${mode}`);
        this.inner.drawSprite(s, frame, x, y, mode);
    }
    drawNineSlice(s: SpriteHandle, frames: number[], x: number, y: number, w: number, h: number, mode: BlendMode): void {
        this.lines.push(`nine ${this.sprite(s)} ${frames.join(",")} ${x} ${y} ${w} ${h} ${mode}`);
        this.inner.drawNineSlice(s, frames, x, y, w, h, mode);
    }
    fillRect(x: number, y: number, w: number, h: number, rgb: number, alpha: number): void {
        this.lines.push(`rect ${x} ${y} ${w} ${h} ${hex(rgb)} ${alpha}`);
        this.inner.fillRect(x, y, w, h, rgb, alpha);
    }
    drawLine(x0: number, y0: number, x1: number, y1: number, rgb: number, alpha: number): void {
        this.lines.push(`line ${x0} ${y0} ${x1} ${y1} ${hex(rgb)} ${alpha}`);
        this.inner.drawLine(x0, y0, x1, y1, rgb, alpha);
    }
    drawText(f: FontHandle, text: string, x: number, y: number, colour: number): void {
        this.lines.push(`text ${this.font(f)} ${x} ${y} ${colour} ${text}`);
        this.inner.drawText(f, text, x, y, colour);
    }
    pushScissor(x: number, y: number, w: number, h: number): void {
        this.lines.push(`clip ${x} ${y} ${w} ${h}`);
        this.inner.pushScissor(x, y, w, h);
    }
    popScissor(): void {
        this.lines.push("unclip");
        this.inner.popScissor();
    }
    text(): string {
        return this.lines.join("\n");
    }
}
