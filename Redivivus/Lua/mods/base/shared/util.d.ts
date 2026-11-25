/** @noSelfInFile */
// Declarations of util.lua, for TypeScript mods.

/** A shallow copy of a table. */
export function copy<T extends object>(t: T): T;
/** The number of entries in a table, array part or not. */
export function count(t: object): number;
