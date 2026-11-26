/**
 * The installed mods, as the game found them when it started: every mod in the mods folder, whether
 * it is on, how it stands and why, its dependencies as they are met, its README and changelog, and
 * the settings the mods declared. Read-only, and the same in every state; a state the game gave no
 * catalogue (a test runtime) lists nothing. Turning mods on and off is not here: that is the game's
 * own redivivus mod's (its Mods screen).
 * @side data server client
 */

import { argString } from "../internal/check";
import { catalog } from "../internal/native";
import type { DependencyState, ModState, SettingScope, SettingType } from "./enums";

export { DependencyState, ModState } from "./enums";

/** One dependency of an installed mod. */
export interface InstalledDependency {
    /** As written in info.json: "redivivus >= 0.1", "? other", "! enemy". */
    readonly spec: string;
    readonly name: string;
    readonly state: DependencyState;
    /** The installed version, or "" when it is not installed. */
    readonly installed: string;
}

/** An installed mod. */
export interface InstalledMod {
    readonly name: string;
    /** The title, or the name when info.json has none. */
    readonly title: string;
    readonly version: string;
    readonly author: string;
    readonly licence: string;
    /** On in the player's mod list (it may still not load: see state). */
    readonly enabled: boolean;
    readonly state: ModState;
    /** Why it is not loaded, or "". */
    readonly error: string;
    readonly dependencies: readonly InstalledDependency[];
}

/** A setting an installed mod declared, with the value in effect. */
export interface InstalledSetting {
    readonly mod: string;
    readonly name: string;
    /** The declared title, or the name. */
    readonly title: string;
    readonly type: SettingType;
    readonly scope: SettingScope;
    readonly value: boolean | number | string;
}

/** Every installed mod: the ones that load in load order, then the rest by name. Fresh tables. */
export function installedMods(): InstalledMod[] {
    return catalog.mods() as unknown as InstalledMod[];
}

/** An installed mod's README.md (Markdown, at most 256 KiB), or "" when it has none. */
export function readme(mod: string): string {
    return catalog.readme(argString(mod, 1, "readme"));
}

/** An installed mod's CHANGELOG.md, or "". */
export function changelog(mod: string): string {
    return catalog.changelog(argString(mod, 1, "changelog"));
}

/** Every setting the installed mods declared, with its value. Fresh tables. */
export function installedSettings(): InstalledSetting[] {
    return catalog.settings() as unknown as InstalledSetting[];
}
