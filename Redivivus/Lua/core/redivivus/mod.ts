/**
 * The mod itself: what a mod's main exports as its default, `export default defineMod({ ... })`.
 * Every entry point is optional and gets its parameter typed from here, so nothing needs importing
 * but `defineMod`; a key that is not an entry point is a compile error.
 *
 * - `patch(engine)` runs once per Lua state (the client's, each server game's), before `menu`,
 *   `server` and `client`, with no game. It is where a mod overrides functions of the game:
 *   `engine.<module>.<namespace>.<function>.override((next, ...args) => ...)`, every named function
 *   of the compiled game, by the names of the source. `engine` holds only the functions of the
 *   state's side (`engine.side`): the server's and the shared ones in a server state, the client's
 *   and the shared ones in the client state.
 * - `server(game)` once for every game this process hosts, `client(client)` once for every game this
 *   client is in, `menu(menu)` once, when the main menu first comes up.
 *
 * Plain Lua: `return require("redivivus:mod").defineMod({ patch = function(engine) ... end })`, or a
 * bare table of the same functions.
 * @side data server client
 */

import type { Client, Menu } from "./client";
import type { Side } from "./enums";
import type { ClientModules, RawClientModules, RawServerModules, ServerModules } from "./engine-functions";
import type { Game } from "./server";

export type {
    ClientModules, EngineFunction, Opaque, Pointer, RawClientModules, RawServerModules, RefusedFunction, ServerModules,
} from "./engine-functions";

/**
 * The raw tier of a granted mod: the functions with pointers as LuaJIT FFI cdata, and `ffi` itself
 * (declare a record's layout with `ffi.cdef` to read its fields; an undeclared record is an
 * incomplete type).
 */
export type RawTier<M> = M & { readonly ffi: any };

/** The engine as a server state's `patch` gets it. */
export interface ServerEngine extends ServerModules {
    readonly side: Side.Server;
    /**
     * The raw tier: the same functions with pointers as LuaJIT FFI cdata. Only there for a mod the
     * player granted it; undefined otherwise.
     */
    readonly unsafe?: RawTier<RawServerModules>;
}

/** The engine as the client state's `patch` gets it. */
export interface ClientEngine extends ClientModules {
    readonly side: Side.Client;
    /** The raw tier (see ServerEngine.unsafe). */
    readonly unsafe?: RawTier<RawClientModules>;
}

/** The engine of the state `patch` runs in: narrow it on `engine.side`. */
export type Engine = ServerEngine | ClientEngine;

/** A mod's entry points. */
export interface ModDefinition {
    /** Once per Lua state, before anything else, with no game: override engine functions here. */
    patch?(this: void, engine: Engine): void;
    /** Once for every game this process hosts. */
    server?(this: void, game: Game): void;
    /** Once for every game this client is in. */
    client?(this: void, client: Client): void;
    /** Once, when the main menu first comes up (the client state). */
    menu?(this: void, menu: Menu): void;
}

/** The mod: returns its argument unchanged; it is there for the types. */
export function defineMod(definition: ModDefinition): ModDefinition {
    return definition;
}
