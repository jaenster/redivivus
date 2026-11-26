/**
 * Inlines: the text of one paragraph, heading or table cell to nodes, in one pass from left
 * to right. Nothing here looks back further than a stack: a backtick run remembers per
 * length that it has no closer, a `]` reads its destination only up to the next `(`, and a
 * closing emphasis run pops the openers it skips, so the work is linear in the text.
 */

import type { Inline } from './ast';
import { isAlpha, isPunct, LF, lower, SPACE } from './bytes';
import { count, fail, MAX_URL_BYTES, type State } from './limits';
import { inlineText } from './text';

const NODE = 0;
const DELIM = 1;
const BRACKET = 2;

/** One entry of the working list: a finished node, a run of `*` or `_`, or an open `[` or `![`. */
interface Item {
  kind: number;
  /** For a node. */
  node: Inline | undefined;
  /** How deep the node is: 1 for a leaf. */
  depth: number;
  /** For a run: its character and how many are left of it. */
  ch: number;
  count: number;
  open: boolean;
  close: boolean;
  /** For a bracket: `![`. */
  image: boolean;
}

function nodeItem(node: Inline, depth: number): Item {
  return { kind: NODE, node, depth, ch: 0, count: 0, open: false, close: false, image: false };
}

function textItem(text: string): Item {
  return nodeItem({ type: 'text', text }, 1);
}

function repeat(ch: number, times: number): string {
  const c = String.fromCharCode(ch);
  const out: string[] = [];
  for (let i = 0; i < times; i++) out.push(c);
  return out.join('');
}

/** A run or bracket nothing matched is the text it was written as. */
function literal(it: Item): string {
  if (it.kind === BRACKET) return it.image ? '![' : '[';
  return repeat(it.ch, it.count);
}

interface Nodes {
  nodes: Inline[];
  depth: number;
}

/** Items to nodes: what is left unmatched becomes text, and text next to text is one node. */
function finish(st: State, items: Item[], from: number, line: number): Nodes {
  const nodes: Inline[] = [];
  let depth = 0;
  let pending: string[] = [];
  for (let i = from; i < items.length; i++) {
    const it = items[i]!;
    const node = it.node;
    if (it.kind === NODE && node !== undefined && node.type !== 'text') {
      if (pending.length > 0) {
        nodes.push({ type: 'text', text: pending.join('') });
        count(st, line);
        pending = [];
      }
      nodes.push(node);
      count(st, line);
      if (it.depth > depth) depth = it.depth;
    } else {
      const text = node !== undefined && node.type === 'text' ? node.text : literal(it);
      if (text.length > 0) pending.push(text);
    }
  }
  if (pending.length > 0) {
    nodes.push({ type: 'text', text: pending.join('') });
    count(st, line);
  }
  if (nodes.length > 0 && depth === 0) depth = 1;
  return { nodes, depth };
}

function tooDeep(st: State, line: number): void {
  fail(st, 'too_deep', line, `emphasis and links are nested more than ${st.limits.maxInlineDepth} deep`);
}

/**
 * Emphasis over a list that holds no brackets any more. A closing run takes the nearest
 * opening run of its own character; the runs of the other character it passes over stay
 * text. Two of each make a strong, otherwise one of each an emphasis, until either is used up.
 */
function emphasis(st: State, items: Item[], from: number, line: number): Nodes {
  const out: Item[] = [];
  /** Indexes into `out` of the runs that can still open. */
  const openers: number[] = [];
  let stars = 0;
  let unders = 0;
  for (let i = from; i < items.length && st.error === undefined; i++) {
    const it = items[i]!;
    if (it.kind !== DELIM) {
      out.push(it);
    } else {
      let matching = it.close;
      while (matching && it.count > 0 && st.error === undefined) {
        if ((it.ch === 42 ? stars : unders) === 0) {
          matching = false;
        } else {
          let at = openers[openers.length - 1]!;
          while (out[at]!.ch !== it.ch) {
            openers.pop();
            if (out[at]!.ch === 42) stars -= 1;
            else unders -= 1;
            at = openers[openers.length - 1]!;
          }
          const op = out[at]!;
          const use = op.count >= 2 && it.count >= 2 ? 2 : 1;
          const inner = finish(st, out, at + 1, line);
          while (out.length > at + 1) out.pop();
          op.count -= use;
          it.count -= use;
          if (op.count === 0) {
            out.pop();
            openers.pop();
            if (it.ch === 42) stars -= 1;
            else unders -= 1;
          }
          if (inner.depth + 1 > st.limits.maxInlineDepth) tooDeep(st, line);
          const node: Inline = use === 2 ? { type: 'strong', children: inner.nodes } : { type: 'emphasis', children: inner.nodes };
          out.push(nodeItem(node, inner.depth + 1));
        }
      }
      if (it.count > 0) {
        if (it.open) {
          openers.push(out.length);
          if (it.ch === 42) stars += 1;
          else unders += 1;
          out.push(it);
        } else {
          out.push(textItem(literal(it)));
        }
      }
    }
  }
  return finish(st, out, 0, line);
}

const ENTITY_NAMES = ['amp;', 'lt;', 'gt;', 'quot;', 'apos;', 'nbsp;'];
// A no-break space is U+00A0: two bytes, spelled out so no source file holds a character outside ASCII.
const ENTITY_TEXTS = ['&', '<', '>', '"', "'", String.fromCharCode(0xc2, 0xa0)];

function startsWith(s: string, at: number, word: string): boolean {
  if (at + word.length > s.length) return false;
  return s.substring(at, at + word.length) === word;
}

/** Whether a byte next to a run of `*` or `_` counts as white space: a space, a line end, either end of the text. */
function isEdge(c: number): boolean {
  return c === SPACE || c === LF || c === -1;
}

/**
 * A code span's text: line ends are spaces, and one space is taken off each end when both
 * ends have one and there is more than spaces.
 */
function codeText(s: string, a: number, b: number): string {
  const out: string[] = [];
  let from = a;
  let blank = true;
  for (let i = a; i < b; i++) {
    const c = s.charCodeAt(i);
    if (c === LF) {
      out.push(s.substring(from, i));
      out.push(' ');
      from = i + 1;
    } else if (c !== SPACE) {
      blank = false;
    }
  }
  out.push(s.substring(from, b));
  const text = out.join('');
  if (!blank && text.length >= 2 && text.charCodeAt(0) === SPACE && text.charCodeAt(text.length - 1) === SPACE) {
    return text.substring(1, text.length - 1);
  }
  return text;
}

/**
 * The destination after a `]`: `(`, then bytes up to `)`. A space, a line end, `(`, `<` or
 * `>` in it means there is no destination; a backslash before punctuation gives that
 * character. `end` is the index after the `)`, or -1.
 */
interface Destination {
  end: number;
  url: string;
}

function destination(s: string, at: number): Destination {
  const none: Destination = { end: -1, url: '' };
  const n = s.length;
  if (at >= n || s.charCodeAt(at) !== 40) return none;
  const out: string[] = [];
  let i = at + 1;
  let from = i;
  while (i < n) {
    const c = s.charCodeAt(i);
    if (c === 41) {
      out.push(s.substring(from, i));
      const url = out.join('');
      if (url.length > MAX_URL_BYTES) return none;
      return { end: i + 1, url };
    }
    if (c === SPACE || c === LF || c === 40 || c === 60 || c === 62) return none;
    if (c === 92 && i + 1 < n && isPunct(s.charCodeAt(i + 1))) {
      out.push(s.substring(from, i));
      from = i + 1;
      i += 2;
    } else {
      i += 1;
    }
  }
  return none;
}

/** `<http://...>` or `<https://...>` at `at`: the index after the `>`, or -1. */
function autolinkEnd(s: string, at: number): number {
  const n = s.length;
  let i = at + 1;
  const head = lower(s.substring(i, i + 8 > n ? n : i + 8));
  if (!startsWith(head, 0, 'http://') && !startsWith(head, 0, 'https://')) return -1;
  while (i < n) {
    const c = s.charCodeAt(i);
    if (c === 62) return i - at - 1 <= MAX_URL_BYTES ? i + 1 : -1;
    if (c === SPACE || c === LF || c === 60) return -1;
    i += 1;
  }
  return -1;
}

/** The inlines of `s`, whose lines are joined by line feeds. `line` is where it starts, for an error. */
export function parseInline(st: State, s: string, line: number): Inline[] {
  const n = s.length;
  const items: Item[] = [];
  /** Indexes into `items` of the open brackets. */
  const brackets: number[] = [];
  /** The brackets below this height of the stack opened before a link closed, and a link cannot hold a link. */
  let dead = 0;
  /** Per run length, the index from which no closing run of that length exists. */
  const noCloser: { [length: number]: number } = {};
  let from = 0;
  let i = 0;

  while (i < n && st.error === undefined) {
    const c = s.charCodeAt(i);
    let special = true;
    let flushTo = i;
    let item: Item | undefined;
    let next = i + 1;

    if (c === 92) {
      const d = i + 1 < n ? s.charCodeAt(i + 1) : -1;
      if (d === LF) {
        item = nodeItem({ type: 'hardbreak' }, 1);
        next = i + 2;
      } else if (d !== -1 && isPunct(d)) {
        // The backslash goes; the character after it joins the text that follows.
        items.push(textItem(s.substring(from, i)));
        from = i + 1;
        i += 2;
        special = false;
      } else {
        special = false;
        i += 1;
      }
    } else if (c === LF) {
      let e = i;
      while (e > from && s.charCodeAt(e - 1) === SPACE) e -= 1;
      flushTo = e;
      item = nodeItem(i - e >= 2 ? { type: 'hardbreak' } : { type: 'softbreak' }, 1);
    } else if (c === 96) {
      let e = i;
      while (e < n && s.charCodeAt(e) === 96) e += 1;
      const length = e - i;
      next = e;
      let close = -1;
      const known = noCloser[length];
      if (length <= 32 && (known === undefined || known > e)) {
        let j = e;
        while (j < n && close === -1) {
          if (s.charCodeAt(j) === 96) {
            let k = j;
            while (k < n && s.charCodeAt(k) === 96) k += 1;
            if (k - j === length) close = j;
            j = k;
          } else {
            j += 1;
          }
        }
        if (close === -1) noCloser[length] = e;
      }
      if (close === -1) {
        special = false;
        i = e;
      } else {
        item = nodeItem({ type: 'code', text: codeText(s, e, close) }, 1);
        next = close + length;
      }
    } else if (c === 42 || c === 95) {
      let e = i;
      while (e < n && s.charCodeAt(e) === c) e += 1;
      const before = i > 0 ? s.charCodeAt(i - 1) : -1;
      const after = e < n ? s.charCodeAt(e) : -1;
      const left = !isEdge(after) && (!isPunct(after) || isEdge(before) || isPunct(before));
      const right = !isEdge(before) && (!isPunct(before) || isEdge(after) || isPunct(after));
      const open = c === 42 ? left : left && (!right || isPunct(before));
      const close = c === 42 ? right : right && (!left || isPunct(after));
      item = { kind: DELIM, node: undefined, depth: 0, ch: c, count: e - i, open, close, image: false };
      next = e;
    } else if (c === 91) {
      item = { kind: BRACKET, node: undefined, depth: 0, ch: 0, count: 0, open: false, close: false, image: false };
    } else if (c === 33 && i + 1 < n && s.charCodeAt(i + 1) === 91) {
      item = { kind: BRACKET, node: undefined, depth: 0, ch: 0, count: 0, open: false, close: false, image: true };
      next = i + 2;
    } else if (c === 93) {
      if (brackets.length === 0) {
        special = false;
        i += 1;
      } else {
        items.push(textItem(s.substring(from, i)));
        from = i;
        const at = brackets.pop()!;
        const opener = items[at]!;
        const live = opener.image || brackets.length >= dead;
        if (dead > brackets.length) dead = brackets.length;
        const dest: Destination = live ? destination(s, i + 1) : { end: -1, url: '' };
        if (dest.end === -1) {
          items[at] = textItem(literal(opener));
          special = false;
          i += 1;
        } else {
          const inner = emphasis(st, items, at + 1, line);
          while (items.length > at) items.pop();
          if (opener.image) {
            item = nodeItem({ type: 'image', url: dest.url, alt: inlineText(inner.nodes) }, 1);
          } else {
            if (inner.depth + 1 > st.limits.maxInlineDepth) tooDeep(st, line);
            item = nodeItem({ type: 'link', url: dest.url, children: inner.nodes }, inner.depth + 1);
            dead = brackets.length;
          }
          next = dest.end;
        }
      }
    } else if (c === 60) {
      const e = autolinkEnd(s, i);
      if (e === -1) {
        special = false;
        i += 1;
      } else {
        const url = s.substring(i + 1, e - 1);
        item = nodeItem({ type: 'link', url, children: [{ type: 'text', text: url }] }, 2);
        dead = brackets.length;
        next = e;
      }
    } else if (c === 38) {
      let found = -1;
      for (let k = 0; k < ENTITY_NAMES.length; k++) {
        if (found === -1 && startsWith(s, i + 1, ENTITY_NAMES[k]!)) found = k;
      }
      if (found === -1) {
        special = false;
        i += 1;
      } else {
        item = textItem(ENTITY_TEXTS[found]!);
        next = i + 1 + ENTITY_NAMES[found]!.length;
      }
    } else {
      special = false;
      i += 1;
    }

    if (special && item !== undefined) {
      if (flushTo > from) items.push(textItem(s.substring(from, flushTo)));
      if (item.kind === BRACKET) brackets.push(items.length);
      items.push(item);
      from = next;
      i = next;
    }
  }

  if (from < n) items.push(textItem(s.substring(from, n)));
  if (st.error !== undefined) return [];
  return emphasis(st, items, 0, line).nodes;
}
