// Text: the game's inline colour codes, missing glyphs, measurement and word wrapping.
//
// Strings are bytes, as in the game: a Lua string is its bytes and the game's fonts index by
// byte. In JavaScript a string here holds one character per byte (codes 0..255). A colour code is
// byte 255, the letter "c", and one code character: "\xFFc1" switches to red. The code
// character's value minus 48 is the colour index ("0" white, "1" red, ... ";" purple).

import type { Backend, FontHandle } from "./backend";

export const colourEscape = 255;
const LETTER_C = 99;
const SPACE = 32;
const NEWLINE = 10;
/** Drawn in place of a character the font lacks. */
export const placeholderGlyph = 63; // "?"

export interface Run {
    text: string;
    colour: number;
}

/** A colour code as a string, for building text: colourCode(1) + "red". */
export function colourCode(index: number): string {
    return String.fromCharCode(colourEscape) + "c" + String.fromCharCode(48 + index);
}

/** Splits text into lines (at "\n") of coloured runs. A colour carries over line ends. */
export function parseRuns(text: string, colour: number): Run[][] {
    const lines: Run[][] = [];
    let runs: Run[] = [];
    let current = colour;
    let start = 0;
    let i = 0;
    const n = text.length;
    const push = (end: number): void => {
        if (end > start) runs.push({ text: text.substring(start, end), colour: current });
    };
    while (i < n) {
        const c = text.charCodeAt(i);
        if (c === colourEscape && i + 2 < n && text.charCodeAt(i + 1) === LETTER_C) {
            push(i);
            current = text.charCodeAt(i + 2) - 48;
            i += 3;
            start = i;
        } else if (c === NEWLINE) {
            push(i);
            lines.push(runs);
            runs = [];
            i++;
            start = i;
        } else {
            i++;
        }
    }
    push(n);
    lines.push(runs);
    return lines;
}

/** The text with every character the font lacks replaced by the placeholder. */
export function sanitize(backend: Backend, font: FontHandle, text: string): string {
    let out: string | undefined = undefined;
    for (let i = 0; i < text.length; i++) {
        const c = text.charCodeAt(i);
        if (!backend.hasGlyph(font, c)) {
            if (out === undefined) out = text.substring(0, i);
            out += String.fromCharCode(placeholderGlyph);
        } else if (out !== undefined) {
            out += text.substring(i, i + 1);
        }
    }
    return out === undefined ? text : out;
}

/** Width of a string that may hold colour codes (one line). */
export function measure(backend: Backend, font: FontHandle, text: string): number {
    const lines = parseRuns(text, 0);
    let widest = 0;
    for (let l = 0; l < lines.length; l++) {
        let w = 0;
        const runs = lines[l];
        for (let r = 0; r < runs.length; r++) w += backend.measureText(font, sanitize(backend, font, runs[r].text));
        if (w > widest) widest = w;
    }
    return widest;
}

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

interface Piece {
    text: string;
    font: FontHandle;
    colour: number;
    width: number;
    space: boolean;
    newline: boolean;
}

function piecesOf(backend: Backend, spans: Span[]): Piece[] {
    const pieces: Piece[] = [];
    for (let s = 0; s < spans.length; s++) {
        const span = spans[s];
        const lines = parseRuns(span.text, span.colour);
        for (let l = 0; l < lines.length; l++) {
            if (l > 0) pieces.push({ text: "", font: span.font, colour: span.colour, width: 0, space: false, newline: true });
            const runs = lines[l];
            for (let r = 0; r < runs.length; r++) {
                const t = sanitize(backend, span.font, runs[r].text);
                let i = 0;
                while (i < t.length) {
                    const space = t.charCodeAt(i) === SPACE;
                    let j = i + 1;
                    while (j < t.length && (t.charCodeAt(j) === SPACE) === space) j++;
                    const text = t.substring(i, j);
                    pieces.push({ text, font: span.font, colour: runs[r].colour, width: backend.measureText(span.font, text), space, newline: false });
                    i = j;
                }
            }
        }
    }
    return pieces;
}

function newLine(): TextLine {
    return { runs: [], width: 0, height: 0, ascent: 0 };
}

function addToLine(line: TextLine, p: Piece): void {
    const runs = line.runs;
    const last = runs.length > 0 ? runs[runs.length - 1] : undefined;
    if (last !== undefined && last.colour === p.colour && last.font === p.font) {
        last.text += p.text;
        last.width += p.width;
    } else {
        runs.push({ text: p.text, font: p.font, colour: p.colour, width: p.width });
    }
    line.width += p.width;
}

function breakWord(backend: Backend, p: Piece, first: number, maxWidth: number): Piece[] {
    const out: Piece[] = [];
    let start = 0;
    let room = first;
    while (start < p.text.length) {
        let end = start + 1;
        while (end < p.text.length && backend.measureText(p.font, p.text.substring(start, end + 1)) <= room) end++;
        const text = p.text.substring(start, end);
        out.push({ text, font: p.font, colour: p.colour, width: backend.measureText(p.font, text), space: false, newline: false });
        start = end;
        room = maxWidth;
    }
    return out;
}

function finish(backend: Backend, line: TextLine, font: FontHandle): void {
    let height = 0;
    let ascent = 0;
    const runs = line.runs;
    if (runs.length === 0) {
        height = backend.fontLineHeight(font);
        ascent = backend.fontAscent(font);
    }
    for (let i = 0; i < runs.length; i++) {
        const h = backend.fontLineHeight(runs[i].font);
        const a = backend.fontAscent(runs[i].font);
        if (h > height) height = h;
        if (a > ascent) ascent = a;
    }
    line.height = height;
    line.ascent = ascent;
}

/**
 * Lays spans out into lines no wider than maxWidth (maxWidth < 0: no wrapping). Words wrap at
 * spaces; a word wider than a whole line is broken between characters. Spaces at a wrap point
 * are dropped; spaces at the start of a paragraph are kept.
 */
export function layoutSpans(backend: Backend, spans: Span[], maxWidth: number): TextLine[] {
    const out: TextLine[] = [];
    const fallbackFont = spans.length > 0 ? spans[0].font : 0;
    const pieces = piecesOf(backend, spans);
    let line = newLine();
    let lastFont = fallbackFont;
    let pending: Piece | undefined = undefined;
    let wrapped = false;
    const push = (): void => {
        finish(backend, line, lastFont);
        out.push(line);
        line = newLine();
        pending = undefined;
    };
    for (let i = 0; i < pieces.length; i++) {
        const p = pieces[i];
        lastFont = p.font;
        if (p.newline) {
            push();
            wrapped = false;
            continue;
        }
        if (p.space) {
            if (line.runs.length === 0 && wrapped) continue;
            if (pending === undefined) pending = p;
            else pending = { text: pending.text + p.text, font: p.font, colour: p.colour, width: pending.width + p.width, space: true, newline: false };
            continue;
        }
        const spaceW = pending === undefined ? 0 : (pending as Piece).width;
        if (maxWidth >= 0 && line.runs.length > 0 && line.width + spaceW + p.width > maxWidth) {
            push();
            wrapped = true;
        } else if (pending !== undefined) {
            addToLine(line, pending);
            pending = undefined;
        }
        if (maxWidth >= 0 && line.width + p.width > maxWidth) {
            const parts = breakWord(backend, p, maxWidth - line.width, maxWidth);
            for (let k = 0; k < parts.length; k++) {
                if (k > 0) {
                    push();
                    wrapped = true;
                }
                addToLine(line, parts[k]);
            }
        } else {
            addToLine(line, p);
        }
    }
    if (pending !== undefined && line.runs.length > 0) addToLine(line, pending);
    finish(backend, line, lastFont);
    out.push(line);
    return out;
}

/** One font, one default colour: layoutSpans for plain label text. */
export function layoutText(backend: Backend, font: FontHandle, text: string, colour: number, maxWidth: number): TextLine[] {
    return layoutSpans(backend, [{ text, font, colour }], maxWidth);
}

/** Total height of laid-out lines. */
export function linesHeight(lines: TextLine[]): number {
    let h = 0;
    for (let i = 0; i < lines.length; i++) h += lines[i].height;
    return h;
}

/** Widest of laid-out lines. */
export function linesWidth(lines: TextLine[]): number {
    let w = 0;
    for (let i = 0; i < lines.length; i++) if (lines[i].width > w) w = lines[i].width;
    return w;
}

/**
 * Draws laid-out lines in a box: `align` 0 left, 1 centre, 2 right. Lines past the box are still
 * drawn; the caller clips.
 */
export function drawLines(backend: Backend, lines: TextLine[], x: number, y: number, w: number, align: number): void {
    let top = y;
    for (let i = 0; i < lines.length; i++) {
        const line = lines[i];
        let lx = x;
        if (align === 1) lx = x + Math.floor((w - line.width) / 2);
        else if (align === 2) lx = x + w - line.width;
        for (let r = 0; r < line.runs.length; r++) {
            const run = line.runs[r];
            const trimmed = run.text;
            if (trimmed.length > 0) backend.drawText(run.font, trimmed, lx, top + line.ascent - backend.fontAscent(run.font), run.colour);
            lx += run.width;
        }
        top += line.height;
    }
}

/** What a cut line ends with: three full stops, which every font of the game has. */
export const ellipsis = "...";

/**
 * A laid-out line no wider than maxWidth: as it is when it fits, otherwise cut after the last whole
 * glyph that leaves room for the ellipsis (drawn in the font and colour of the run it ends), with
 * the spaces before the cut dropped. A line too narrow for the ellipsis itself comes back empty.
 */
export function ellipsize(backend: Backend, line: TextLine, maxWidth: number): TextLine {
    if (maxWidth < 0 || line.width <= maxWidth) return line;
    const out: TextLine = { runs: [], width: 0, height: line.height, ascent: line.ascent };
    let x = 0;
    for (let r = 0; r < line.runs.length; r++) {
        const run = line.runs[r];
        const ellW = backend.measureText(run.font, ellipsis);
        // The longest prefix of this run that still leaves room for the ellipsis after it.
        let n = 0;
        let w = 0;
        while (n < run.text.length) {
            const cw = backend.measureText(run.font, run.text.substring(n, n + 1));
            if (x + w + cw + ellW > maxWidth) break;
            w += cw;
            n++;
        }
        if (n < run.text.length || r === line.runs.length - 1) {
            let text = run.text.substring(0, n);
            while (text.length > 0 && text.charCodeAt(text.length - 1) === SPACE) text = text.substring(0, text.length - 1);
            // The run fits whole only when it is the last one, which then did not need cutting.
            if (x + ellW > maxWidth && text.length === 0) {
                // Not even the ellipsis fits after the runs before this one: end them there.
                return out;
            }
            out.runs.push({ text: text + ellipsis, font: run.font, colour: run.colour, width: backend.measureText(run.font, text) + ellW });
            out.width = x + out.runs[out.runs.length - 1].width;
            return out;
        }
        out.runs.push(run);
        x += run.width;
        out.width = x;
    }
    return out;
}

/**
 * A single line of plain text (no colour codes) cut to maxWidth with the ellipsis, at a glyph
 * boundary; as it is when it fits.
 */
export function truncate(backend: Backend, font: FontHandle, text: string, maxWidth: number): string {
    if (backend.measureText(font, text) <= maxWidth) return text;
    const line = ellipsize(backend, { runs: [{ text, font, colour: 0, width: backend.measureText(font, text) }], width: backend.measureText(font, text), height: 0, ascent: 0 }, maxWidth);
    return line.runs.length > 0 ? line.runs[0].text : "";
}

/** Strips colour codes. */
export function plain(text: string): string {
    const lines = parseRuns(text, 0);
    let s = "";
    for (let l = 0; l < lines.length; l++) {
        if (l > 0) s += "\n";
        for (let r = 0; r < lines[l].length; r++) s += lines[l][r].text;
    }
    return s;
}
