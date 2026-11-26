// Generated from Redivivus/Lua/core/ui/md/parse.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
/** Text to tree, or the first reason there is none. */
import type { Document } from './ast';
import { type Limits, type ParseError } from './limits';
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
export declare function parse(src: string, limits?: Limits): Parsed;
