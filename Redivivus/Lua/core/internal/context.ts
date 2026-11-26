// The mod a public module instance belongs to. The core's loader (d2_lua_core.cpp) instantiates
// the public modules (and the UI kit, ui.*) once per mod that requires them, with this module and those instances
// running in an environment of their own whose __context names the mod; every other core module
// is one instance for the state. So each mod's redivivus:shared (and the others) acts for that mod
// and no other.

declare const __context: { readonly id: number; readonly name: string; readonly index: number };

/** The host's id of the mod (what every native taking a mod expects). */
export const modId: number = __context.id;
/** The mod's name. */
export const modName: string = __context.name;
/** The mod's position in load order. */
export const modIndex: number = __context.index;
