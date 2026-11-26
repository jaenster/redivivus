// The Mods screen over the example mods as the game shows them (game.ts), for previews laid over
// in-game screenshots: ui-preview screens/mods/GameMods.tsx --state first-mod.

import { h, type Child, type UIRoot } from "redivivus:ui";
import { ModsScreen } from "../../../builtin/mods/ModsScreen";
import { ModsModel } from "../../../builtin/mods/model";
import { gameModsData } from "./game";

export function createScreen(): { view: (this: void) => Child; states: { [name: string]: (this: void, ui: UIRoot) => void } } {
    const model = new ModsModel(gameModsData());
    return {
        view: () => <ModsScreen model={model} />,
        states: {
            default: () => {},
            "first-mod": () => model.select("first-mod"),
            "ui-mod": () => model.select("ui-mod"),
            "needsgone": () => {
                model.select("needsgone");
                model.setDetailTab(2);
            },
        },
    };
}
