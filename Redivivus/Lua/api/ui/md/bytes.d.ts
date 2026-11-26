// Generated from Redivivus/Lua/core/ui/md/bytes.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
/**
 * The input as bytes. A source string is read one byte at a time: `charCodeAt` is a byte in
 * Lua, and the JavaScript entry (../index.ts) hands the parser a string with one character
 * per UTF-8 byte, so both see the same numbers. All of Markdown's own characters are ASCII;
 * a multi-byte sequence is checked once here and is an opaque word character after that.
 */
import { type State } from './limits';
export declare const LF = 10;
export declare const SPACE = 32;
/** The byte at `i`, or -1 outside the string. */
export declare function byteAt(s: string, i: number): number;
export declare function isPunct(c: number): boolean;
export declare function isDigit(c: number): boolean;
export declare function isAlpha(c: number): boolean;
/** How many spaces a line starts with. */
export declare function indentOf(line: string): number;
export declare function isBlank(line: string): boolean;
/** `s` without the spaces at its end. */
export declare function trimEnd(s: string): string;
export declare function trim(s: string): string;
/** ASCII letters in lower case; every other byte as it is. */
export declare function lower(s: string): string;
/**
 * The input as lines, checked: well-formed UTF-8, no control characters, no line over the
 * limit. `\n`, `\r\n` and `\r` end a line, a byte order mark at the start is dropped, and a
 * tab becomes the spaces to the next multiple of four columns, a column being one code point.
 */
export declare function splitLines(st: State, src: string): string[];
