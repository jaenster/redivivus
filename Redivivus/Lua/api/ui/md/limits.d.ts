// Generated from Redivivus/Lua/core/ui/md/limits.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
/**
 * What the parser refuses. Sizes are bytes of UTF-8, so they are the same number in
 * JavaScript and in Lua.
 */
/** Raised when the syntax changes in a way a renderer has to know about. */
export declare const SYNTAX_VERSION = 1;
export interface Limits {
    /** The whole input. */
    maxBytes: number;
    /** One line, after its tabs became spaces. */
    maxLineBytes: number;
    /** Block quotes and lists inside one another. */
    maxBlockDepth: number;
    /** Emphasis, strong and links inside one another. */
    maxInlineDepth: number;
    /** Blocks and inlines in the whole tree. */
    maxNodes: number;
    maxTableColumns: number;
    /** Body rows of one table. */
    maxTableRows: number;
}
export declare const DEFAULT_LIMITS: Limits;
/** A link or image destination longer than this is not one: the text stays as written. */
export declare const MAX_URL_BYTES = 2048;
export type ErrorCode = 'too_large' | 'bad_utf8' | 'control_character' | 'line_too_long' | 'too_deep' | 'too_many_nodes' | 'table_too_large';
export interface ParseError {
    code: ErrorCode;
    /** 1-based; 0 when the error is about the whole input. */
    line: number;
    message: string;
}
/** What one parse carries along: the limits, the node count so far and the first error. */
export interface State {
    limits: Limits;
    nodes: number;
    error: ParseError | undefined;
}
export declare function fail(st: State, code: ErrorCode, line: number, message: string): void;
/** Counts one node of the tree. */
export declare function count(st: State, line: number): void;
