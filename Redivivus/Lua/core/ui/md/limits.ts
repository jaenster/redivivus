/**
 * What the parser refuses. Sizes are bytes of UTF-8, so they are the same number in
 * JavaScript and in Lua.
 */

/** Raised when the syntax changes in a way a renderer has to know about. */
export const SYNTAX_VERSION = 1;

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

export const DEFAULT_LIMITS: Limits = {
  maxBytes: 65536,
  maxLineBytes: 4096,
  maxBlockDepth: 8,
  maxInlineDepth: 8,
  maxNodes: 10000,
  maxTableColumns: 16,
  maxTableRows: 200,
};

/** A link or image destination longer than this is not one: the text stays as written. */
export const MAX_URL_BYTES = 2048;

export type ErrorCode =
  | 'too_large'
  | 'bad_utf8'
  | 'control_character'
  | 'line_too_long'
  | 'too_deep'
  | 'too_many_nodes'
  | 'table_too_large';

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

export function fail(st: State, code: ErrorCode, line: number, message: string): void {
  if (st.error === undefined) st.error = { code, line, message };
}

/** Counts one node of the tree. */
export function count(st: State, line: number): void {
  st.nodes += 1;
  if (st.nodes > st.limits.maxNodes) fail(st, 'too_many_nodes', line, `more than ${st.limits.maxNodes} nodes`);
}
