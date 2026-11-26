// Generated from Redivivus/Lua/core/ui/md/inline.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
/**
 * Inlines: the text of one paragraph, heading or table cell to nodes, in one pass from left
 * to right. Nothing here looks back further than a stack: a backtick run remembers per
 * length that it has no closer, a `]` reads its destination only up to the next `(`, and a
 * closing emphasis run pops the openers it skips, so the work is linear in the text.
 */
import type { Inline } from './ast';
import { type State } from './limits';
/** The inlines of `s`, whose lines are joined by line feeds. `line` is where it starts, for an error. */
export declare function parseInline(st: State, s: string, line: number): Inline[];
