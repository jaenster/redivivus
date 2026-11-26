// The Mods screen over what the game showed with the example mods installed (Redivivus/Lua/examples
// built into a game's mods/ folder): long titles, no licences, one README among them and a mod whose
// dependency is gone. It is the data of the in-game screenshots, so a preview of it can be laid over
// one pixel for pixel.

import { DependencyState } from "redivivus:enums";
import { ModStatus, type ModInfo, type ModsChanges, type ModsData } from "../../../builtin/mods/model";

function example(name: string, title: string, deps: string[], depState: DependencyState): ModInfo {
    return {
        name,
        title,
        version: "0.1.0",
        author: "Redivivus",
        licence: "",
        enabled: true,
        status: ModStatus.Ok,
        error: "",
        latest: "",
        dependencies: deps.map((spec) => ({ spec, state: depState, installed: depState === DependencyState.Ok ? "0.1.0" : "" })),
    };
}

const installed: ModInfo[] = [
    example("charger", "Charger (a monster AI in TypeScript)", ["redivivus >= 0.1"], DependencyState.Ok),
    example("first-mod", "First mod (TypeScript example)", ["redivivus >= 0.1"], DependencyState.Ok),
    example("second-mod", "Second mod (depends on the first)", ["first-mod >= 0.1"], DependencyState.Ok),
    example("ui-mod", "UI mod (a TSX panel on redivivus:ui)", ["redivivus >= 0.1"], DependencyState.Ok),
    {
        name: "needsgone",
        title: "Needs Gone",
        version: "0.3.0",
        author: "test",
        licence: "",
        enabled: true,
        status: ModStatus.MissingDependency,
        error: "",
        latest: "",
        dependencies: [{ spec: "gone-mod >= 1.0", state: DependencyState.Missing, installed: "" }],
    },
];

const uiModReadme = [
    "# UI mod",
    "",
    "A counter on **redivivus:ui**, the kit every mod draws its UI with.",
    "",
    "- In a game it builds the panel in both states; only the client draws it.",
    "- On the main menu it adds a `UI MOD` button that opens the counter as a full screen.",
    "",
    "Press **+1** to count, **Back** or *Escape* to return to the menu.",
    "",
].join("\n");

/** The example mods as the game's ModsData gives them. */
export function gameModsData(): ModsData & { applied: ModsChanges[]; backs: number } {
    const data = {
        applied: [] as ModsChanges[],
        backs: 0,
        installedMods: () => installed,
        readme: (mod: string) => (mod === "ui-mod" ? uiModReadme : ""),
        changelog: (_mod: string) => "",
        portalMods: () => [],
        settings: () => [],
        apply: (changes: ModsChanges) => {
            data.applied.push(changes);
        },
        back: () => {
            data.backs++;
        },
    };
    return data;
}
