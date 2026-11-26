// Generated from Redivivus/Lua/core/redivivus/manager.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
/**
 * Turning mods on and off and restarting the game: what the game's own redivivus mod does from its
 * Mods screen. Only that mod can import this module; a mod cannot (a mod must never switch the
 * player's mods or restart the game).
 * @side data server client
 */
/**
 * Set mods on or off in the player's mod list (mods/mod-list.json), keyed by name; the others keep
 * their entries. Takes effect when the game restarts. Raises when the file cannot be written.
 */
export declare function setEnabled(changes: {
    readonly [name: string]: boolean;
}): void;
/**
 * Restart the game with the mod list as it is now: this process quits as the main menu's Exit does
 * and the same executable starts again with the same command line. Raises when the game cannot
 * restart itself.
 */
export declare function restart(): void;
