// The Mods screen's data in the game: what redivivus:mods says about the installed mods, and the
// two things only this mod may do through redivivus:manager, write the mod list and restart.

import { restart, setEnabled } from "redivivus:manager";
import { changelog, installedMods, installedSettings, readme, type InstalledMod, ModState } from "redivivus:mods";
import { ModStatus, type ModInfo, type ModsChanges, type ModsData, type SettingInfo } from "./mods/model";

function status(m: InstalledMod): ModStatus {
    if (m.state === ModState.Disabled) return ModStatus.Disabled;
    if (m.state === ModState.Faulted) return ModStatus.Broken;
    if (m.state === ModState.MissingDependency) return ModStatus.MissingDependency;
    return ModStatus.Ok;
}

function info(m: InstalledMod): ModInfo {
    const dependencies = [];
    for (const d of m.dependencies) dependencies.push({ spec: d.spec, state: d.state, installed: d.installed });
    return {
        name: m.name,
        title: m.title,
        version: m.version,
        author: m.author,
        licence: m.licence,
        enabled: m.enabled,
        status: status(m),
        error: m.error,
        latest: "",
        dependencies,
    };
}

/** The game's ModsData; back leaves the screen. */
export function gameData(back: (this: void) => void): ModsData {
    return {
        installedMods: () => {
            const out: ModInfo[] = [];
            for (const m of installedMods()) out.push(info(m));
            return out;
        },
        readme: (mod: string) => readme(mod),
        changelog: (mod: string) => changelog(mod),
        // The portal is not reached from the game yet.
        portalMods: () => [],
        settings: () => {
            const out: SettingInfo[] = [];
            for (const s of installedSettings()) out.push({ mod: s.mod, name: s.name, title: s.title, type: s.type, scope: s.scope, value: s.value });
            return out;
        },
        apply: (changes: ModsChanges) => {
            setEnabled(changes.enabled);
            restart();
        },
        back,
    };
}
