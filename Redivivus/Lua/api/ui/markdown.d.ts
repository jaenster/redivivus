// Generated from Redivivus/Lua/core/ui/markdown.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
import { type Child } from "./jsx";
import type { Document } from "./md/ast";
import { type MaybeAccessor } from "./signal";
import { Widget } from "./widget";
/** Widgets for a parsed document. */
export declare function renderDocument(doc: Document, gap: number): Widget;
/** Markdown source (UTF-8 bytes, one character per byte in JavaScript) as widgets. */
export declare function Markdown(props: {
    source?: MaybeAccessor<string>;
    gap?: number;
    [k: string]: unknown;
}): Child;
