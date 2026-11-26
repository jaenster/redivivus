/**
 * The input as bytes. A source string is read one byte at a time: `charCodeAt` is a byte in
 * Lua, and the JavaScript entry (../index.ts) hands the parser a string with one character
 * per UTF-8 byte, so both see the same numbers. All of Markdown's own characters are ASCII;
 * a multi-byte sequence is checked once here and is an opaque word character after that.
 */

import { fail, type State } from './limits';

export const LF = 10;
export const SPACE = 32;

/** The byte at `i`, or -1 outside the string. */
export function byteAt(s: string, i: number): number {
  if (i < 0 || i >= s.length) return -1;
  return s.charCodeAt(i);
}

export function isPunct(c: number): boolean {
  return (c >= 33 && c <= 47) || (c >= 58 && c <= 64) || (c >= 91 && c <= 96) || (c >= 123 && c <= 126);
}

export function isDigit(c: number): boolean {
  return c >= 48 && c <= 57;
}

export function isAlpha(c: number): boolean {
  return (c >= 65 && c <= 90) || (c >= 97 && c <= 122);
}

/** How many spaces a line starts with. */
export function indentOf(line: string): number {
  let i = 0;
  while (i < line.length && line.charCodeAt(i) === SPACE) i += 1;
  return i;
}

export function isBlank(line: string): boolean {
  return indentOf(line) === line.length;
}

/** `s` without the spaces at its end. */
export function trimEnd(s: string): string {
  let e = s.length;
  while (e > 0 && s.charCodeAt(e - 1) === SPACE) e -= 1;
  return e === s.length ? s : s.substring(0, e);
}

export function trim(s: string): string {
  const t = trimEnd(s);
  const a = indentOf(t);
  return a === 0 ? t : t.substring(a, t.length);
}

/** ASCII letters in lower case; every other byte as it is. */
export function lower(s: string): string {
  let plain = true;
  for (let i = 0; i < s.length; i++) {
    const c = s.charCodeAt(i);
    if (c >= 65 && c <= 90) plain = false;
  }
  if (plain) return s;
  const out: string[] = [];
  for (let i = 0; i < s.length; i++) {
    const c = s.charCodeAt(i);
    out.push(String.fromCharCode(c >= 65 && c <= 90 ? c + 32 : c));
  }
  return out.join('');
}

const SPACES = ['', ' ', '  ', '   ', '    '];

/**
 * The length of the UTF-8 sequence that starts at `i`: 0 when it is not well formed (a stray
 * or missing continuation byte, an overlong form, a surrogate, a value past U+10FFFF) and -1
 * when it is a character the parser refuses: a C1 control or a bidirectional override.
 */
function sequence(s: string, i: number, c: number): number {
  const n = s.length;
  const b1 = i + 1 < n ? s.charCodeAt(i + 1) : -1;
  if (c >= 0xc2 && c <= 0xdf) {
    if (b1 < 0x80 || b1 > 0xbf) return 0;
    return c === 0xc2 && b1 <= 0x9f ? -1 : 2;
  }
  const b2 = i + 2 < n ? s.charCodeAt(i + 2) : -1;
  if (c >= 0xe0 && c <= 0xef) {
    const lo = c === 0xe0 ? 0xa0 : 0x80;
    const hi = c === 0xed ? 0x9f : 0xbf;
    if (b1 < lo || b1 > hi || b2 < 0x80 || b2 > 0xbf) return 0;
    // U+202A to U+202E and U+2066 to U+2069 reorder the text around them.
    if (c === 0xe2 && ((b1 === 0x80 && b2 >= 0xaa && b2 <= 0xae) || (b1 === 0x81 && b2 >= 0xa6 && b2 <= 0xa9))) return -1;
    return 3;
  }
  if (c >= 0xf0 && c <= 0xf4) {
    const b3 = i + 3 < n ? s.charCodeAt(i + 3) : -1;
    const lo = c === 0xf0 ? 0x90 : 0x80;
    const hi = c === 0xf4 ? 0x8f : 0xbf;
    if (b1 < lo || b1 > hi || b2 < 0x80 || b2 > 0xbf || b3 < 0x80 || b3 > 0xbf) return 0;
    return 4;
  }
  return 0;
}

/**
 * The input as lines, checked: well-formed UTF-8, no control characters, no line over the
 * limit. `\n`, `\r\n` and `\r` end a line, a byte order mark at the start is dropped, and a
 * tab becomes the spaces to the next multiple of four columns, a column being one code point.
 */
export function splitLines(st: State, src: string): string[] {
  const lines: string[] = [];
  const n = src.length;
  if (n > st.limits.maxBytes) {
    fail(st, 'too_large', 0, `the text is ${n} bytes; the limit is ${st.limits.maxBytes}`);
    return lines;
  }
  let i = 0;
  if (n >= 3 && src.charCodeAt(0) === 0xef && src.charCodeAt(1) === 0xbb && src.charCodeAt(2) === 0xbf) i = 3;
  let parts: string[] = [];
  let from = i;
  let col = 0;
  let len = 0;
  while (i <= n && st.error === undefined) {
    const c = i < n ? src.charCodeAt(i) : LF;
    if (i === n && from === n && parts.length === 0) {
      // The text ended with its last line's end; there is no line after it.
      i += 1;
    } else if (c === LF || c === 13) {
      let line = src.substring(from, i);
      if (parts.length > 0) {
        parts.push(line);
        line = parts.join('');
        parts = [];
      }
      if (len > st.limits.maxLineBytes) {
        fail(st, 'line_too_long', lines.length + 1, `the line is ${len} bytes; the limit is ${st.limits.maxLineBytes}`);
      }
      lines.push(line);
      if (c === 13 && i + 1 < n && src.charCodeAt(i + 1) === LF) i += 1;
      i += 1;
      from = i;
      col = 0;
      len = 0;
    } else if (c === 9) {
      const pad = 4 - (col % 4);
      parts.push(src.substring(from, i));
      parts.push(SPACES[pad]!);
      i += 1;
      from = i;
      col += pad;
      len += pad;
    } else if (c < 32 || c === 127) {
      fail(st, 'control_character', lines.length + 1, `a control character (byte ${c})`);
    } else if (c < 128) {
      i += 1;
      col += 1;
      len += 1;
    } else {
      const size = sequence(src, i, c);
      if (size === 0) fail(st, 'bad_utf8', lines.length + 1, 'the text is not valid UTF-8');
      else if (size < 0) fail(st, 'control_character', lines.length + 1, 'a control character or a bidirectional override');
      i += size;
      col += 1;
      len += size;
    }
  }
  return lines;
}
