// Generated from Redivivus/Lua/core/ui/md/index.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
/**
 * Everything the game runs: this directory compiles with TypeScriptToLua to Lua 5.1, and the
 * same files are what the site runs. It keeps to what both give the same answer for: bytes
 * read with charCodeAt, substring, arrays, plain objects; no regular expressions, no bitwise
 * operators, no truthiness of '' or 0, and no character outside ASCII in a source file.
 */
export type { Block, BlockQuote, Code, CodeBlock, Document, Emphasis, HardBreak, Heading, Image, Inline, Link, List, Paragraph, SoftBreak, Strong, Table, Text, ThematicBreak, } from './ast';
export { DEFAULT_LIMITS, MAX_URL_BYTES, SYNTAX_VERSION, type ErrorCode, type Limits, type ParseError } from './limits';
export { parse, type Parsed } from './parse';
export { firstParagraph, inlineText, plainText } from './text';
export { classifyImage, classifyLink, type Target } from './url';
export { report, serialize, serializeParsed } from './serialize';
