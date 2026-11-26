// The Mods screen's data: what is installed, what the portal offers, the settings, and what the
// player changed but has not applied yet. The screen reads it through ModsData, which the game fills
// from redivivus:mods (../data.ts) and the previews and tests fill from fixtures
// (Redivivus/Lua/ui/screens/mods/fixtures.ts).

import { computed, signal, type Accessor, type Setter } from "redivivus:ui";
import type { DependencyState, SettingScope, SettingType } from "redivivus:enums";

/** How an installed mod stands. */
export const enum ModStatus {
    Ok = "ok",
    Disabled = "disabled",
    /** It failed to load; ModInfo.error says why. */
    Broken = "broken",
    /** The portal has a newer version (ModInfo.latest). */
    Update = "update",
    MissingDependency = "missingDependency",
}

export interface Dependency {
    /** As written in info.json: "redivivus >= 1.0", "? optional-mod", "! other". */
    spec: string;
    state: DependencyState;
    /** What is installed, when anything is. */
    installed: string;
}

export interface ModInfo {
    name: string;
    title: string;
    version: string;
    author: string;
    licence: string;
    enabled: boolean;
    status: ModStatus;
    /** The load error of a broken mod. */
    error: string;
    /** Newer version on the portal, or "". */
    latest: string;
    dependencies: Dependency[];
}

export interface PortalMod {
    name: string;
    title: string;
    version: string;
    author: string;
    summary: string;
    downloads: number;
    installed: boolean;
}

/** A setting a mod declared, with the value it has now. */
export interface SettingInfo {
    mod: string;
    name: string;
    /** The declared title, or the name. */
    title: string;
    type: SettingType;
    scope: SettingScope;
    value: boolean | number | string;
}

/** What "Apply and restart" hands back: the enabled state of every mod the player changed. */
export interface ModsChanges {
    enabled: { [name: string]: boolean };
}

/**
 * Where the screen's data comes from. The game implements it over the mod runtime; fixtures.ts
 * implements it for previews and tests. Read when the screen opens; nothing here changes under it.
 */
export interface ModsData {
    /** The installed mods, in load order, the broken and disabled ones included. */
    installedMods(this: void): ModInfo[];
    /** A mod's README.md (Markdown), or "" when it has none. */
    readme(this: void, mod: string): string;
    /** A mod's changelog (Markdown), or "". */
    changelog(this: void, mod: string): string;
    /** The portal's index, or nothing when it has not been reached. */
    portalMods(this: void): PortalMod[];
    /** The settings every enabled mod declared. */
    settings(this: void): SettingInfo[];
    /** Write the changes and restart the game. */
    apply(this: void, changes: ModsChanges): void;
    /** Leave the screen. */
    back(this: void): void;
}

export const tabLabels = ["Installed", "Browse", "Updates", "Settings"];
export const detailTabs = ["Description", "Changelog", "Dependencies"];

export class ModsModel {
    readonly data: ModsData;
    readonly mods: Accessor<ModInfo[]>;
    readonly portal: PortalMod[];
    readonly settings: SettingInfo[];
    readonly tab: Accessor<number>;
    readonly setTab: Setter<number>;
    readonly detailTab: Accessor<number>;
    readonly setDetailTab: Setter<number>;
    readonly query: Accessor<string>;
    readonly setQuery: Setter<string>;
    readonly selected: Accessor<string>;
    readonly select: Setter<string>;
    private readonly enabledOverride: Accessor<{ [name: string]: boolean }>;
    private readonly setEnabledOverride: Setter<{ [name: string]: boolean }>;
    readonly visible: Accessor<ModInfo[]>;
    readonly current: Accessor<ModInfo | undefined>;
    readonly pending: Accessor<number>;

    constructor(data: ModsData) {
        this.data = data;
        const mods = data.installedMods();
        const [modsSig] = signal(mods);
        this.mods = modsSig;
        this.portal = data.portalMods();
        this.settings = data.settings();
        [this.tab, this.setTab] = signal(0);
        [this.detailTab, this.setDetailTab] = signal(0);
        [this.query, this.setQuery] = signal("");
        [this.selected, this.select] = signal(mods.length > 0 ? mods[0].name : "");
        [this.enabledOverride, this.setEnabledOverride] = signal<{ [name: string]: boolean }>({});
        this.visible = computed(() => {
            const q = lower(this.query());
            const all = this.mods();
            const updatesOnly = this.tab() === 2;
            const out: ModInfo[] = [];
            for (let i = 0; i < all.length; i++) {
                const m = all[i];
                if (updatesOnly && m.latest.length === 0) continue;
                if (q.length === 0 || contains(lower(m.title), q) || contains(lower(m.name), q)) out.push(m);
            }
            return out;
        });
        this.current = computed(() => {
            const name = this.selected();
            const all = this.mods();
            for (let i = 0; i < all.length; i++) if (all[i].name === name) return all[i];
            return undefined;
        });
        this.pending = computed(() => {
            const o = this.enabledOverride();
            const all = this.mods();
            let n = 0;
            for (let i = 0; i < all.length; i++) {
                const v = o[all[i].name];
                if (v !== undefined && v !== all[i].enabled) n++;
            }
            return n;
        });
    }

    /** The enabled state of every mod the player changed and has not applied. */
    changes(): ModsChanges {
        const o = this.enabledOverride();
        const all = this.mods();
        const enabled: { [name: string]: boolean } = {};
        for (let i = 0; i < all.length; i++) {
            const v = o[all[i].name];
            if (v !== undefined && v !== all[i].enabled) enabled[all[i].name] = v;
        }
        return { enabled };
    }

    isEnabled(m: ModInfo): boolean {
        const v = this.enabledOverride()[m.name];
        return v === undefined ? m.enabled : v;
    }

    setEnabled(m: ModInfo, on: boolean): void {
        const prev = this.enabledOverride();
        const next: { [name: string]: boolean } = {};
        const all = this.mods();
        for (let i = 0; i < all.length; i++) {
            const v = prev[all[i].name];
            if (v !== undefined) next[all[i].name] = v;
        }
        next[m.name] = on;
        this.setEnabledOverride(next);
    }
}

function lower(s: string): string {
    let out = "";
    for (let i = 0; i < s.length; i++) {
        const c = s.charCodeAt(i);
        out += c >= 65 && c <= 90 ? String.fromCharCode(c + 32) : s.substring(i, i + 1);
    }
    return out;
}

function contains(hay: string, needle: string): boolean {
    return hay.indexOf(needle) >= 0;
}
