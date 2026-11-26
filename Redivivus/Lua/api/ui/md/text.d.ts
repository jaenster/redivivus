// Generated from Redivivus/Lua/core/ui/md/text.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
/** The tree as plain text: for a search index, a summary, an image's alt text. */
import type { Document, Inline } from './ast';
/** The text of a run of inlines. A soft break is a space, a hard break a line feed, an image its alt text. */
export declare function inlineText(nodes: Inline[]): string;
/** Every block's text, one block per line (a table row is a line, its cells set apart by a space). */
export declare function plainText(doc: Document): string;
/**
 * The first top-level paragraph that says something, on one line: what a listing shows when
 * a mod gives no summary. Images do not count, so a banner or a row of badges is passed over.
 */
export declare function firstParagraph(doc: Document): string;
