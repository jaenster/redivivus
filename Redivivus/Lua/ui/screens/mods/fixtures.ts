// Fixture data for the Mods screen: a working mod with a long README, one with an update, one
// with a missing dependency, a broken one and a disabled one.

import { DependencyState, SettingScope, SettingType } from "redivivus:enums";
import { ModStatus, type ModInfo, type ModsChanges, type ModsData, type PortalMod, type SettingInfo } from "../../../builtin/mods/model";

const chargerReadme = [
    "# Charger",
    "",
    "Adds the **Charger**, a monster that winds up and then rushes its target in a straight",
    "line. Written as a TypeScript mod against `redivivus:server`; see the [mod guide](https://redivivus.app/mods/guide).",
    "",
    "## Features",
    "",
    "- A new monster with its own AI script",
    "- Works in single player *and* multiplayer",
    "- Drops from its own treasure class:",
    "  1. charms at normal",
    "  2. runes at nightmare and hell",
    "",
    "## Settings",
    "",
    "| Setting | Default | Meaning |",
    "|:--|:-:|--:|",
    "| `charger.speed` | 12 | Rush speed in subtiles per frame |",
    "| `charger.windup` | 25 | Frames before the rush |",
    "| `charger.enabled` | true | Spawn in the Cold Plains |",
    "",
    "## Example",
    "",
    "```ts",
    "export function server(game: Game) {",
    "  game.registry.ai.register(MonsterAiId.Charger, charger);",
    "}",
    "```",
    "",
    "> The rush ignores collision with other monsters, which is intended:",
    "> it is how the Charger breaks a crowd.",
    "",
    "![The Charger mid-rush](docs/rush.png)",
    "",
    "---",
    "",
    "### Known issues",
    "",
    "A Charger that rushes into a closed door stops there and waits; it does not open doors. Long words",
    "like Superlongwordwithoutanyspacesatallthatmustbreakacrosslines are broken between characters.",
    "",
].join("\n");

const chargerChangelog = ["## 1.1.0", "", "- Rush distance follows the `charger.speed` setting", "- Fix: the wind-up sound played twice", "", "## 1.0.0", "", "- First release"].join("\n");

const installed: ModInfo[] = [
    {
        name: "charger",
        title: "Charger",
        version: "1.1.0",
        author: "redivivus",
        licence: "MIT",
        enabled: true,
        status: ModStatus.Ok,
        error: "",
        latest: "",
        dependencies: [
            { spec: "redivivus >= 1.0", state: DependencyState.Ok, installed: "1.0.0" },
            { spec: "? loot-filter >= 1.2", state: DependencyState.Optional, installed: "1.2.0" },
        ],
    },
    {
        name: "loot-filter",
        title: "Loot Filter",
        version: "1.2.0",
        author: "nightfall",
        licence: "GPL-3.0-or-later",
        enabled: true,
        status: ModStatus.Update,
        error: "",
        latest: "1.3.0",
        dependencies: [{ spec: "redivivus >= 1.0", state: DependencyState.Ok, installed: "1.0.0" }],
    },
    {
        name: "act6-maps",
        title: "Act VI Maps",
        version: "0.4.2",
        author: "tilesmith",
        licence: "CC-BY-4.0",
        enabled: true,
        status: ModStatus.MissingDependency,
        error: "",
        latest: "",
        dependencies: [
            { spec: "redivivus >= 1.0", state: DependencyState.Ok, installed: "1.0.0" },
            { spec: "tile-kit >= 2.0", state: DependencyState.Missing, installed: "" },
            { spec: "! old-maps", state: DependencyState.Ok, installed: "" },
        ],
    },
    {
        name: "broken-hud",
        title: "Broken HUD",
        version: "0.1.0",
        author: "tester",
        licence: "MIT",
        enabled: true,
        status: ModStatus.Broken,
        error: "main.lua:12: attempt to index a nil value (field 'hud')",
        latest: "",
        dependencies: [{ spec: "redivivus >= 1.0", state: DependencyState.Ok, installed: "1.0.0" }],
    },
    {
        name: "stash-plus",
        title: "Stash Plus",
        version: "2.0.1",
        author: "kolbot",
        licence: "MIT",
        enabled: false,
        status: ModStatus.Disabled,
        error: "",
        latest: "",
        dependencies: [{ spec: "redivivus >= 1.1", state: DependencyState.Version, installed: "1.0.0" }],
    },
];

const portal: PortalMod[] = [
    { name: "charger", title: "Charger", version: "1.1.0", author: "redivivus", summary: "A monster that rushes its target.", downloads: 1520, installed: true },
    { name: "tile-kit", title: "Tile Kit", version: "2.1.0", author: "tilesmith", summary: "Shared tiles for map mods.", downloads: 880, installed: false },
    { name: "item-sets-plus", title: "Item Sets Plus", version: "0.9.0", author: "setfan", summary: "Twelve new item sets.", downloads: 412, installed: false },
    { name: "loot-filter", title: "Loot Filter", version: "1.3.0", author: "nightfall", summary: "Hide the items you do not want.", downloads: 9820, installed: true },
];

const settings: SettingInfo[] = [
    { mod: "charger", name: "enabled", title: "Spawn Chargers in the Cold Plains", type: SettingType.Bool, scope: SettingScope.Startup, value: true },
    { mod: "charger", name: "speed", title: "Rush speed", type: SettingType.Int, scope: SettingScope.Game, value: 12 },
    { mod: "loot-filter", name: "hideJunk", title: "Hide junk items", type: SettingType.Bool, scope: SettingScope.Player, value: false },
];

const readmes: { [mod: string]: string } = {
    "charger": chargerReadme,
    "loot-filter": "# Loot Filter\n\nHides items you do not want to see on the ground.\n\n- Rules per item quality\n- `Alt` shows everything",
    "act6-maps": "# Act VI Maps\n\nNew outdoor areas for Act VI, built from the tile kit.",
    "broken-hud": "# Broken HUD\n\nA HUD replacement that fails to load.",
    "stash-plus": "# Stash Plus\n\nA bigger shared stash with tabs.",
};

const changelogs: { [mod: string]: string } = {
    "charger": chargerChangelog,
    "loot-filter": "## 1.3.0\n\n- Rules per difficulty\n\n## 1.2.0\n\n- Charms count as magic items",
    "act6-maps": "## 0.4.2\n\n- Fixed a seam between two tiles",
    "broken-hud": "",
    "stash-plus": "## 2.0.1\n\n- Tabs keep their names",
};

/** The fixtures as the screen reads them. apply and back record what they were given. */
export function fixtureData(): ModsData & { applied: ModsChanges[]; backs: number } {
    const data = {
        applied: [] as ModsChanges[],
        backs: 0,
        installedMods: () => installed,
        readme: (mod: string) => readmes[mod] ?? "",
        changelog: (mod: string) => changelogs[mod] ?? "",
        portalMods: () => portal,
        settings: () => settings,
        apply: (changes: ModsChanges) => {
            data.applied.push(changes);
        },
        back: () => {
            data.backs++;
        },
    };
    return data;
}
