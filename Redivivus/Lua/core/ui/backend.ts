// The one seam between the UI runtime and whatever draws it. The engine implements it in C++ on
// D2gfx/D2Win; the host renderer (host/raster.ts) implements it in Node and writes a PNG; the
// recorder (recorder.ts) writes every call down as a line of text, which is what tests compare.
//
// Units are integer pixels in the logical screen, origin top-left, x right, y down. Every call
// draws through the current scissor (the intersection of every pushScissor not yet popped).
// Handles are opaque positive integers; 0 means "could not load" and is safe to draw (nothing,
// or a placeholder the backend chooses).

import { BlendMode } from "../redivivus/enums";

export { BlendMode };

/** A font handle, from loadFont. */
export type FontHandle = number;
/** A sprite handle, from loadSprite. */
export type SpriteHandle = number;

export interface Backend {
    // --- assets -------------------------------------------------------------------------------
    /** A DC6 by game path without extension, e.g. "data\\global\\ui\\FrontEnd\\WideButtonBlank". */
    loadSprite(path: string): SpriteHandle;
    /** Frames in the sprite (0 for handle 0). */
    spriteFrames(sprite: SpriteHandle): number;
    /** Width of one frame in pixels. */
    spriteWidth(sprite: SpriteHandle, frame: number): number;
    /** Height of one frame in pixels. */
    spriteHeight(sprite: SpriteHandle, frame: number): number;
    /** A font by the engine's name ("font16", "fontexocet10", ...). */
    loadFont(name: string): FontHandle;

    // --- text metrics -------------------------------------------------------------------------
    /** Distance from one line's top to the next line's top. */
    fontLineHeight(font: FontHandle): number;
    /** Distance from a line's top to its baseline. */
    fontAscent(font: FontHandle): number;
    /** Whether the font draws this character code (a byte, 0..255). */
    hasGlyph(font: FontHandle, code: number): boolean;
    /** Width of a run with no colour codes and no line breaks, as drawText would advance. */
    measureText(font: FontHandle, text: string): number;

    // --- frame --------------------------------------------------------------------------------
    /** Starts a frame of the given logical size; the scissor is the whole screen. */
    beginFrame(width: number, height: number): void;
    /** Ends the frame; every pushScissor has been popped. */
    endFrame(): void;

    // --- drawing ------------------------------------------------------------------------------
    /** One frame of a sprite with its top-left at (x, y). */
    drawSprite(sprite: SpriteHandle, frame: number, x: number, y: number, mode: BlendMode): void;
    /**
     * A box of any size from nine frames of a sprite: `frames` names the top-left, top, top-right,
     * left, centre, right, bottom-left, bottom, bottom-right frames. Corners are drawn once, edges
     * and centre are tiled (never stretched) and cut at the box.
     */
    drawNineSlice(sprite: SpriteHandle, frames: number[], x: number, y: number, w: number, h: number, mode: BlendMode): void;
    /** A filled rectangle. `rgb` is 0xRRGGBB; `alpha` is 0..255 (the engine rounds it to a BlendMode). */
    fillRect(x: number, y: number, w: number, h: number, rgb: number, alpha: number): void;
    /** A one-pixel line between two points, both included. */
    drawLine(x0: number, y0: number, x1: number, y1: number, rgb: number, alpha: number): void;
    /**
     * A run of text with its line top at (x, y), in one of the game's text colours (the index a
     * colour code names: 0 white, 1 red, 2 green, 3 blue, 4 gold, ...). No codes, no line breaks.
     */
    drawText(font: FontHandle, text: string, x: number, y: number, colour: number): void;
    /** Narrows the drawable area to its intersection with this rectangle. */
    pushScissor(x: number, y: number, w: number, h: number): void;
    popScissor(): void;
}
