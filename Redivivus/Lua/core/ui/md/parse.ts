/** Text to tree, or the first reason there is none. */

import type { Document } from './ast';
import { parseBlocks } from './block';
import { splitLines } from './bytes';
import { DEFAULT_LIMITS, type Limits, type ParseError, type State } from './limits';

export interface Parsed {
  ok: boolean;
  /** The tree when `ok`; an empty document otherwise. */
  doc: Document;
  /** Set when not `ok`. */
  error: ParseError | undefined;
}

/**
 * Parses UTF-8 Markdown. Never throws and never gives half a tree: an input over a limit, or
 * one that is not text, is refused whole with the line it went wrong on.
 */
export function parse(src: string, limits?: Limits): Parsed {
  const st: State = { limits: limits === undefined ? DEFAULT_LIMITS : limits, nodes: 0, error: undefined };
  const lines = splitLines(st, src);
  const nums: number[] = [];
  for (let i = 0; i < lines.length; i++) nums.push(i + 1);
  const children = st.error === undefined ? parseBlocks(st, lines, nums, 0) : [];
  if (st.error !== undefined) return { ok: false, doc: { type: 'document', children: [] }, error: st.error };
  return { ok: true, doc: { type: 'document', children }, error: undefined };
}
