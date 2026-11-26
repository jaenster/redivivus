/** The tree as plain text: for a search index, a summary, an image's alt text. */

import type { Block, Document, Inline } from './ast';

function inlineInto(out: string[], nodes: Inline[], images: boolean): void {
  for (const n of nodes) {
    if (n.type === 'text' || n.type === 'code') out.push(n.text);
    else if (n.type === 'softbreak') out.push(' ');
    else if (n.type === 'hardbreak') out.push('\n');
    else if (n.type === 'image') {
      if (images) out.push(n.alt);
    } else inlineInto(out, n.children, images);
  }
}

/** The text of a run of inlines. A soft break is a space, a hard break a line feed, an image its alt text. */
export function inlineText(nodes: Inline[]): string {
  const out: string[] = [];
  inlineInto(out, nodes, true);
  return out.join('');
}

function blocksInto(out: string[], blocks: Block[]): void {
  for (const b of blocks) {
    if (b.type === 'heading' || b.type === 'paragraph') out.push(inlineText(b.children));
    else if (b.type === 'code_block') out.push(b.text);
    else if (b.type === 'blockquote') blocksInto(out, b.children);
    else if (b.type === 'list') {
      for (const item of b.items) blocksInto(out, item);
    } else if (b.type === 'table') {
      const row = (cells: Inline[][]): void => {
        const texts: string[] = [];
        for (const cell of cells) texts.push(inlineText(cell));
        out.push(texts.join(' '));
      };
      row(b.head);
      for (const r of b.rows) row(r);
    }
  }
}

/** Every block's text, one block per line (a table row is a line, its cells set apart by a space). */
export function plainText(doc: Document): string {
  const out: string[] = [];
  blocksInto(out, doc.children);
  return out.join('\n');
}

/**
 * The first top-level paragraph that says something, on one line: what a listing shows when
 * a mod gives no summary. Images do not count, so a banner or a row of badges is passed over.
 */
export function firstParagraph(doc: Document): string {
  for (const b of doc.children) {
    if (b.type === 'paragraph') {
      const out: string[] = [];
      inlineInto(out, b.children, false);
      const text = out.join('');
      const flat: string[] = [];
      let space = true;
      for (let i = 0; i < text.length; i++) {
        const c = text.charCodeAt(i);
        if (c === 32 || c === 10) {
          if (!space) flat.push(' ');
          space = true;
        } else {
          flat.push(String.fromCharCode(c));
          space = false;
        }
      }
      if (flat.length > 0 && flat[flat.length - 1] === ' ') flat.pop();
      if (flat.length > 0) return flat.join('');
    }
  }
  return '';
}
