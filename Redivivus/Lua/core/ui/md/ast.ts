/**
 * The tree both renderers consume. Every string in it is UTF-8 text that passed the checks of
 * parse.ts: no control character and no tab anywhere, and a line feed only inside the `text`
 * of a code block. A line break in running text is a node, never a character.
 */

export interface Text { type: 'text'; text: string }
/** A code span; its text is shown as written, in the fixed-width font. */
export interface Code { type: 'code'; text: string }
/** The end of a source line inside a paragraph: a space when the text is reflowed. */
export interface SoftBreak { type: 'softbreak' }
/** A line break the author asked for. */
export interface HardBreak { type: 'hardbreak' }
export interface Emphasis { type: 'emphasis'; children: Inline[] }
export interface Strong { type: 'strong'; children: Inline[] }
/** `url` is as written; a renderer acts on classifyLink(url), never on the string itself. */
export interface Link { type: 'link'; url: string; children: Inline[] }
/** `url` is as written; a renderer acts on classifyImage(url). `alt` is plain text. */
export interface Image { type: 'image'; url: string; alt: string }

export type Inline = Text | Code | SoftBreak | HardBreak | Emphasis | Strong | Link | Image;

export interface Heading { type: 'heading'; level: number; children: Inline[] }
export interface Paragraph { type: 'paragraph'; children: Inline[] }
/** `lang` is the first word of the info string when it is a plain tag, otherwise ''. */
export interface CodeBlock { type: 'code_block'; lang: string; text: string }
export interface BlockQuote { type: 'blockquote'; children: Block[] }
/** In a tight list an item's paragraphs are set without space between them. */
export interface List { type: 'list'; ordered: boolean; start: number; tight: boolean; items: Block[][] }
export interface ThematicBreak { type: 'thematic_break' }
/** `align` has one entry per column: 'left', 'center', 'right' or ''. Every row has that many cells. */
export interface Table { type: 'table'; align: string[]; head: Inline[][]; rows: Inline[][][] }

export type Block = Heading | Paragraph | CodeBlock | BlockQuote | List | ThematicBreak | Table;

export interface Document { type: 'document'; children: Block[] }
