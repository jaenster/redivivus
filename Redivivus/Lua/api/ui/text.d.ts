// Generated from Redivivus/Lua/core/ui/text.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
import type { Backend, FontHandle } from "./backend";
export declare const colourEscape = 255;
/** Drawn in place of a character the font lacks. */
export declare const placeholderGlyph = 63;
export interface Run {
    text: string;
    colour: number;
}
/** A colour code as a string, for building text: colourCode(1) + "red". */
export declare function colourCode(index: number): string;
/** Splits text into lines (at "\n") of coloured runs. A colour carries over line ends. */
export declare function parseRuns(text: string, colour: number): Run[][];
/** The text with every character the font lacks replaced by the placeholder. */
export declare function sanitize(backend: Backend, font: FontHandle, text: string): string;
/** Width of a string that may hold colour codes (one line). */
export declare function measure(backend: Backend, font: FontHandle, text: string): number;
/** A run of text in one font and one colour, before wrapping. "\n" in its text is a hard break. */
export interface Span {
    text: string;
    font: FontHandle;
    colour: number;
}
/** A laid-out run on one line. */
export interface LineRun {
    text: string;
    font: FontHandle;
    colour: number;
    width: number;
}
/** One laid-out line: runs share a baseline at `ascent` below the line top. */
export interface TextLine {
    runs: LineRun[];
    width: number;
    height: number;
    ascent: number;
}
/**
 * Lays spans out into lines no wider than maxWidth (maxWidth < 0: no wrapping). Words wrap at
 * spaces; a word wider than a whole line is broken between characters. Spaces at a wrap point
 * are dropped; spaces at the start of a paragraph are kept.
 */
export declare function layoutSpans(backend: Backend, spans: Span[], maxWidth: number): TextLine[];
/** One font, one default colour: layoutSpans for plain label text. */
export declare function layoutText(backend: Backend, font: FontHandle, text: string, colour: number, maxWidth: number): TextLine[];
/** Total height of laid-out lines. */
export declare function linesHeight(lines: TextLine[]): number;
/** Widest of laid-out lines. */
export declare function linesWidth(lines: TextLine[]): number;
/**
 * Draws laid-out lines in a box: `align` 0 left, 1 centre, 2 right. Lines past the box are still
 * drawn; the caller clips.
 */
export declare function drawLines(backend: Backend, lines: TextLine[], x: number, y: number, w: number, align: number): void;
/** What a cut line ends with: three full stops, which every font of the game has. */
export declare const ellipsis = "...";
/**
 * A laid-out line no wider than maxWidth: as it is when it fits, otherwise cut after the last whole
 * glyph that leaves room for the ellipsis (drawn in the font and colour of the run it ends), with
 * the spaces before the cut dropped. A line too narrow for the ellipsis itself comes back empty.
 */
export declare function ellipsize(backend: Backend, line: TextLine, maxWidth: number): TextLine;
/**
 * A single line of plain text (no colour codes) cut to maxWidth with the ellipsis, at a glyph
 * boundary; as it is when it fits.
 */
export declare function truncate(backend: Backend, font: FontHandle, text: string, maxWidth: number): string;
/** Strips colour codes. */
export declare function plain(text: string): string;
