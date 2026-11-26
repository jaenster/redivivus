/**
 * Turning mods on and off and restarting the game: what the game's own redivivus mod does from its
 * Mods screen. Only that mod can import this module; a mod cannot (a mod must never switch the
 * player's mods or restart the game).
 * @side data server client
 */

import { argBoolean, argString, argTable } from "../internal/check";
import { catalog } from "../internal/native";

/**
 * Set mods on or off in the player's mod list (mods/mod-list.json), keyed by name; the others keep
 * their entries. Takes effect when the game restarts. Raises when the file cannot be written.
 */
export function setEnabled(changes: { readonly [name: string]: boolean }): void {
    argTable(changes, 1, "setEnabled");
    const out = new LuaTable<string, boolean>();
    for (const [k, v] of pairs(changes)) {
        out.set(argString(k, 1, "setEnabled"), argBoolean(v, 1, "setEnabled"));
    }
    const [ok, why] = catalog.writeModList(out);
    if (!ok) error(`setEnabled: ${why ?? "the mod list cannot be written"}`, 2);
}

/**
 * Restart the game with the mod list as it is now: this process quits as the main menu's Exit does
 * and the same executable starts again with the same command line. Raises when the game cannot
 * restart itself.
 */
export function restart(): void {
    const [ok, why] = catalog.restart();
    if (!ok) error(`restart: ${why ?? "the game cannot restart itself"}`, 2);
}
