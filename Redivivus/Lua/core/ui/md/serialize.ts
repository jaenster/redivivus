/**
 * The tree as text, one node per line, indented by depth: what the tests compare. It is
 * written out field by field so a Lua table and a JavaScript object give the same bytes, and
 * every byte outside printable ASCII is escaped so the result is ASCII in both.
 */

import type { Block, Document, Inline } from './ast';
import type { Parsed } from './parse';
import { firstParagraph, plainText } from './text';

const HEX = '0123456789abcdef';

function quote(s: string): string {
  const out: string[] = ['"'];
  let from = 0;
  for (let i = 0; i < s.length; i++) {
    const c = s.charCodeAt(i);
    if (c < 32 || c > 126 || c === 34 || c === 92) {
      out.push(s.substring(from, i));
      from = i + 1;
      if (c === 10) out.push('\\n');
      else if (c === 34) out.push('\\"');
      else if (c === 92) out.push('\\\\');
      else {
        const hi = Math.floor(c / 16);
        const lo = c % 16;
        out.push(`\\x${HEX.substring(hi, hi + 1)}${HEX.substring(lo, lo + 1)}`);
      }
    }
  }
  out.push(s.substring(from, s.length));
  out.push('"');
  return out.join('');
}

function inlines(out: string[], pad: string, nodes: Inline[]): void {
  for (const n of nodes) {
    if (n.type === 'text' || n.type === 'code') out.push(`${pad}${n.type} ${quote(n.text)}`);
    else if (n.type === 'softbreak' || n.type === 'hardbreak') out.push(`${pad}${n.type}`);
    else if (n.type === 'image') out.push(`${pad}image ${quote(n.url)} ${quote(n.alt)}`);
    else {
      out.push(n.type === 'link' ? `${pad}link ${quote(n.url)}` : `${pad}${n.type}`);
      inlines(out, `${pad}  `, n.children);
    }
  }
}

function blocks(out: string[], pad: string, nodes: Block[]): void {
  const deeper = `${pad}  `;
  for (const b of nodes) {
    if (b.type === 'heading') {
      out.push(`${pad}heading ${b.level}`);
      inlines(out, deeper, b.children);
    } else if (b.type === 'paragraph') {
      out.push(`${pad}paragraph`);
      inlines(out, deeper, b.children);
    } else if (b.type === 'code_block') {
      out.push(`${pad}code_block ${quote(b.lang)} ${quote(b.text)}`);
    } else if (b.type === 'blockquote') {
      out.push(`${pad}blockquote`);
      blocks(out, deeper, b.children);
    } else if (b.type === 'list') {
      out.push(`${pad}list ${b.ordered ? `ordered ${b.start}` : 'bullet'} ${b.tight ? 'tight' : 'loose'}`);
      for (const item of b.items) {
        out.push(`${deeper}item`);
        blocks(out, `${deeper}  `, item);
      }
    } else if (b.type === 'thematic_break') {
      out.push(`${pad}thematic_break`);
    } else {
      const names: string[] = [];
      for (const a of b.align) names.push(a === '' ? 'none' : a);
      out.push(`${pad}table ${names.join(' ')}`);
      const row = (name: string, cells: Inline[][]): void => {
        out.push(`${deeper}${name}`);
        for (const cell of cells) {
          out.push(`${deeper}  cell`);
          inlines(out, `${deeper}    `, cell);
        }
      };
      row('head', b.head);
      for (const r of b.rows) row('row', r);
    }
  }
}

export function serialize(doc: Document): string {
  const out: string[] = ['document'];
  blocks(out, '  ', doc.children);
  out.push('');
  return out.join('\n');
}

/** A parse result as text: the tree, or one line naming the error. */
export function serializeParsed(res: Parsed): string {
  if (res.ok) return serialize(res.doc);
  const e = res.error;
  if (e === undefined) return 'error\n';
  return `error ${e.code} line ${e.line}\n`;
}

/** Everything the tests compare for one input: the tree, its plain text and its first paragraph. */
export function report(res: Parsed): string {
  if (!res.ok) return serializeParsed(res);
  return `${serialize(res.doc)}plain ${quote(plainText(res.doc))}\nfirst ${quote(firstParagraph(res.doc))}\n`;
}
