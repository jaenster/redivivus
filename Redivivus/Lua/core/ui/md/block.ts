/**
 * Blocks: lines to a tree. Every construct is decided from the line it starts on (a table
 * also reads the next one), and a container hands the lines it owns, with its marker taken
 * off, to the same function one level down. There is no lazy continuation: a line belongs to
 * a block quote only when it starts with `>`, and to a list item only when it is indented to
 * the item's text.
 */

import type { Block, Inline } from './ast';
import { indentOf, isBlank, isDigit, LF, SPACE, trim, trimEnd } from './bytes';
import { parseInline } from './inline';
import { count, fail, type State } from './limits';

interface Fence {
  ch: number;
  length: number;
  lang: string;
}

function isTagByte(c: number): boolean {
  return isDigit(c) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c === 95 || c === 43 || c === 46 || c === 35 || c === 45;
}

/** An opening fence at `ind`: three or more backticks or tildes, then the info string. */
function fenceAt(line: string, ind: number): Fence | undefined {
  const n = line.length;
  const ch = ind < n ? line.charCodeAt(ind) : -1;
  if (ch !== 96 && ch !== 126) return undefined;
  let e = ind;
  while (e < n && line.charCodeAt(e) === ch) e += 1;
  if (e - ind < 3) return undefined;
  const info = trim(line.substring(e, n));
  let w = 0;
  let plain = true;
  while (w < info.length && info.charCodeAt(w) !== SPACE) {
    if (!isTagByte(info.charCodeAt(w))) plain = false;
    w += 1;
  }
  if (ch === 96) {
    for (let i = 0; i < info.length; i++) {
      if (info.charCodeAt(i) === 96) return undefined;
    }
  }
  return { ch, length: e - ind, lang: plain && w <= 32 ? info.substring(0, w) : '' };
}

function closesFence(line: string, f: Fence): boolean {
  const ind = indentOf(line);
  if (ind > 3) return false;
  let e = ind;
  while (e < line.length && line.charCodeAt(e) === f.ch) e += 1;
  if (e - ind < f.length) return false;
  while (e < line.length && line.charCodeAt(e) === SPACE) e += 1;
  return e === line.length;
}

/** 1 to 6 for an ATX heading at `ind`, otherwise 0. */
function headingAt(line: string, ind: number): number {
  let e = ind;
  while (e < line.length && line.charCodeAt(e) === 35) e += 1;
  const level = e - ind;
  if (level < 1 || level > 6) return 0;
  if (e < line.length && line.charCodeAt(e) !== SPACE) return 0;
  return level;
}

/** A heading's text: without the marker and without a closing run of `#`. */
function headingText(line: string, ind: number, level: number): string {
  const text = trim(line.substring(ind + level, line.length));
  let e = text.length;
  while (e > 0 && text.charCodeAt(e - 1) === 35) e -= 1;
  if (e === text.length) return text;
  if (e === 0) return '';
  if (text.charCodeAt(e - 1) !== SPACE) return text;
  return trimEnd(text.substring(0, e));
}

/** Three or more of one of `*`, `-`, `_` and nothing else but spaces. */
function isThematic(line: string, ind: number): boolean {
  const ch = ind < line.length ? line.charCodeAt(ind) : -1;
  if (ch !== 42 && ch !== 45 && ch !== 95) return false;
  let seen = 0;
  for (let i = ind; i < line.length; i++) {
    const c = line.charCodeAt(i);
    if (c === ch) seen += 1;
    else if (c !== SPACE) return false;
  }
  return seen >= 3;
}

interface Marker {
  ordered: boolean;
  /** The bullet, or the `.` or `)` after the number: a list ends where this changes. */
  ch: number;
  start: number;
  /** The marker and the one space after it. */
  width: number;
  /** Nothing follows the marker on its line. */
  empty: boolean;
}

/** A list marker at `ind`: `-`, `*` or `+`, or up to nine digits and `.` or `)`; then a space or the end of the line. */
function markerAt(line: string, ind: number): Marker | undefined {
  const n = line.length;
  if (ind >= n) return undefined;
  const c = line.charCodeAt(ind);
  let e = ind;
  let ordered = false;
  let start = 0;
  let ch = c;
  if (c === 45 || c === 42 || c === 43) {
    e = ind + 1;
  } else if (isDigit(c)) {
    while (e < n && e - ind < 9 && isDigit(line.charCodeAt(e))) {
      start = start * 10 + (line.charCodeAt(e) - 48);
      e += 1;
    }
    if (e >= n) return undefined;
    ch = line.charCodeAt(e);
    if (ch !== 46 && ch !== 41) return undefined;
    e += 1;
    ordered = true;
  } else {
    return undefined;
  }
  if (e < n && line.charCodeAt(e) !== SPACE) return undefined;
  return { ordered, ch, start, width: e - ind + 1, empty: isBlank(line.substring(e, n)) };
}

/** The cells of a table row: split at each `|` that no backslash stands before, the outer pipes dropped. */
function cellsOf(line: string): string[] {
  const s = trim(line);
  const n = s.length;
  const cells: string[] = [];
  let parts: string[] = [];
  let i = 0;
  if (n > 0 && s.charCodeAt(0) === 124) i = 1;
  let from = i;
  while (i < n) {
    const c = s.charCodeAt(i);
    if (c === 92 && i + 1 < n) {
      if (s.charCodeAt(i + 1) === 124) {
        parts.push(s.substring(from, i));
        from = i + 1;
      }
      i += 2;
    } else if (c === 124) {
      parts.push(s.substring(from, i));
      cells.push(trim(parts.join('')));
      parts = [];
      i += 1;
      from = i;
    } else {
      i += 1;
    }
  }
  if (from < n) {
    parts.push(s.substring(from, n));
    cells.push(trim(parts.join('')));
  }
  return cells;
}

function hasPipe(line: string): boolean {
  for (let i = 0; i < line.length; i++) {
    if (line.charCodeAt(i) === 124) return true;
  }
  return false;
}

/** The alignments a delimiter row gives (`---`, `:--`, `:-:`, `--:`), or undefined when the line is not one. */
function alignsOf(line: string): string[] | undefined {
  if (!hasPipe(line)) return undefined;
  const cells = cellsOf(line);
  if (cells.length === 0) return undefined;
  const out: string[] = [];
  for (const cell of cells) {
    const n = cell.length;
    const left = n > 0 && cell.charCodeAt(0) === 58;
    const right = n > 1 && cell.charCodeAt(n - 1) === 58;
    const a = left ? 1 : 0;
    const b = right ? n - 1 : n;
    if (b - a < 1) return undefined;
    for (let i = a; i < b; i++) {
      if (cell.charCodeAt(i) !== 45) return undefined;
    }
    out.push(left && right ? 'center' : left ? 'left' : right ? 'right' : '');
  }
  return out;
}

/** Whether a line that follows paragraph text starts something else instead of going on with it. */
function interrupts(line: string): boolean {
  const ind = indentOf(line);
  if (ind > 3) return false;
  if (fenceAt(line, ind) !== undefined || headingAt(line, ind) > 0 || isThematic(line, ind)) return true;
  if (ind < line.length && line.charCodeAt(ind) === 62) return true;
  const m = markerAt(line, ind);
  return m !== undefined && !m.empty && (!m.ordered || m.start === 1);
}

function tooDeep(st: State, line: number): void {
  fail(st, 'too_deep', line, `block quotes and lists are nested more than ${st.limits.maxBlockDepth} deep`);
}

/** A list is loose when a blank line stands between two of its items or an item holds two paragraphs. */
function twoParagraphs(item: Block[]): boolean {
  let seen = 0;
  for (const b of item) {
    if (b.type === 'paragraph') seen += 1;
  }
  return seen >= 2;
}

/**
 * The blocks of `lines`. `nums[i]` is the line of the input that `lines[i]` came from, and
 * `depth` is how many block quotes and list items are around them.
 */
export function parseBlocks(st: State, lines: string[], nums: number[], depth: number): Block[] {
  const out: Block[] = [];
  const n = lines.length;
  let i = 0;
  while (i < n && st.error === undefined) {
    const line = lines[i]!;
    const num = nums[i]!;
    const ind = indentOf(line);
    const fence = ind <= 3 ? fenceAt(line, ind) : undefined;
    const level = ind <= 3 ? headingAt(line, ind) : 0;
    const marker = ind <= 3 ? markerAt(line, ind) : undefined;
    const aligns = ind <= 3 && i + 1 < n && hasPipe(line) ? alignsOf(lines[i + 1]!) : undefined;

    if (ind === line.length) {
      i += 1;
    } else if (fence !== undefined) {
      const text: string[] = [];
      i += 1;
      let open = true;
      while (i < n && open) {
        const l = lines[i]!;
        if (closesFence(l, fence)) {
          open = false;
        } else {
          const own = indentOf(l);
          text.push(l.substring(own < ind ? own : ind, l.length));
        }
        i += 1;
      }
      count(st, num);
      out.push({ type: 'code_block', lang: fence.lang, text: text.join('\n') });
    } else if (level > 0) {
      count(st, num);
      out.push({ type: 'heading', level: level > 3 ? 3 : level, children: parseInline(st, headingText(line, ind, level), num) });
      i += 1;
    } else if (ind <= 3 && isThematic(line, ind)) {
      count(st, num);
      out.push({ type: 'thematic_break' });
      i += 1;
    } else if (ind <= 3 && line.charCodeAt(ind) === 62) {
      const inner: string[] = [];
      const innerNums: number[] = [];
      let quoted = true;
      while (i < n && quoted) {
        const l = lines[i]!;
        const at = indentOf(l);
        if (at <= 3 && at < l.length && l.charCodeAt(at) === 62) {
          const skip = at + 1 < l.length && l.charCodeAt(at + 1) === SPACE ? 2 : 1;
          inner.push(l.substring(at + skip, l.length));
          innerNums.push(nums[i]!);
          i += 1;
        } else {
          quoted = false;
        }
      }
      count(st, num);
      if (depth + 1 > st.limits.maxBlockDepth) tooDeep(st, num);
      else out.push({ type: 'blockquote', children: parseBlocks(st, inner, innerNums, depth + 1) });
    } else if (marker !== undefined) {
      const items: Block[][] = [];
      let tight = true;
      let more = true;
      if (depth + 1 > st.limits.maxBlockDepth) tooDeep(st, num);
      while (more && st.error === undefined) {
        const first = lines[i]!;
        const at = indentOf(first);
        const m = markerAt(first, at)!;
        const width = at + m.width;
        const inner: string[] = [m.empty ? '' : first.substring(width, first.length)];
        const innerNums: number[] = [nums[i]!];
        i += 1;
        // The item goes on over blank lines for as long as an indented line follows them.
        let blanks = 0;
        let going = true;
        while (i < n && going) {
          const l = lines[i]!;
          const own = indentOf(l);
          if (own === l.length) {
            blanks += 1;
            i += 1;
          } else if (own >= width) {
            for (let b = blanks; b > 0; b--) {
              inner.push('');
              innerNums.push(nums[i - b]!);
            }
            blanks = 0;
            inner.push(l.substring(width, l.length));
            innerNums.push(nums[i]!);
            i += 1;
          } else {
            going = false;
          }
        }
        const item = parseBlocks(st, inner, innerNums, depth + 1);
        items.push(item);
        if (twoParagraphs(item)) tight = false;
        more = false;
        if (i < n) {
          const l = lines[i]!;
          const own = indentOf(l);
          const next = own <= 3 && !isThematic(l, own) ? markerAt(l, own) : undefined;
          if (next !== undefined && next.ordered === marker.ordered && next.ch === marker.ch) {
            more = true;
            if (blanks > 0) tight = false;
          }
        }
        // Blank lines after the last item belong to whatever comes next.
        if (!more) i -= blanks;
      }
      count(st, num);
      out.push({ type: 'list', ordered: marker.ordered, start: marker.ordered ? marker.start : 1, tight, items });
    } else if (aligns !== undefined && cellsOf(line).length === aligns.length) {
      const columns = aligns.length;
      const row = (source: string, at: number): Inline[][] => {
        const cells = cellsOf(source);
        const cellNodes: Inline[][] = [];
        for (let c = 0; c < columns; c++) cellNodes.push(c < cells.length ? parseInline(st, cells[c]!, at) : []);
        return cellNodes;
      };
      if (columns > st.limits.maxTableColumns) fail(st, 'table_too_large', num, `a table has at most ${st.limits.maxTableColumns} columns`);
      const head = row(line, num);
      const rows: Inline[][][] = [];
      i += 2;
      while (i < n && st.error === undefined && !isBlank(lines[i]!) && hasPipe(lines[i]!)) {
        if (rows.length >= st.limits.maxTableRows) fail(st, 'table_too_large', nums[i]!, `a table has at most ${st.limits.maxTableRows} rows`);
        else rows.push(row(lines[i]!, nums[i]!));
        i += 1;
      }
      count(st, num);
      out.push({ type: 'table', align: aligns, head, rows });
    } else {
      const text: string[] = [line.substring(ind, line.length)];
      i += 1;
      while (i < n && !isBlank(lines[i]!) && !interrupts(lines[i]!)) {
        text.push(lines[i]!.substring(indentOf(lines[i]!), lines[i]!.length));
        i += 1;
      }
      text[text.length - 1] = trimEnd(text[text.length - 1]!);
      count(st, num);
      out.push({ type: 'paragraph', children: parseInline(st, text.join(String.fromCharCode(LF)), num) });
    }
  }
  return out;
}
