// Generated from Redivivus/Lua/core/ui/md/block.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
/**
 * Blocks: lines to a tree. Every construct is decided from the line it starts on (a table
 * also reads the next one), and a container hands the lines it owns, with its marker taken
 * off, to the same function one level down. There is no lazy continuation: a line belongs to
 * a block quote only when it starts with `>`, and to a list item only when it is indented to
 * the item's text.
 */
import type { Block } from './ast';
import { type State } from './limits';
/**
 * The blocks of `lines`. `nums[i]` is the line of the input that `lines[i]` came from, and
 * `depth` is how many block quotes and list items are around them.
 */
export declare function parseBlocks(st: State, lines: string[], nums: number[], depth: number): Block[];
