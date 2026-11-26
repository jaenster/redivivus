// Generated from Redivivus/Lua/core/ui/md/serialize.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
/**
 * The tree as text, one node per line, indented by depth: what the tests compare. It is
 * written out field by field so a Lua table and a JavaScript object give the same bytes, and
 * every byte outside printable ASCII is escaped so the result is ASCII in both.
 */
import type { Document } from './ast';
import type { Parsed } from './parse';
export declare function serialize(doc: Document): string;
/** A parse result as text: the tree, or one line naming the error. */
export declare function serializeParsed(res: Parsed): string;
/** Everything the tests compare for one input: the tree, its plain text and its first paragraph. */
export declare function report(res: Parsed): string;
