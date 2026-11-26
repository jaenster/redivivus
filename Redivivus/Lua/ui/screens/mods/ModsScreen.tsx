// The Mods screen as previews and tests see it: the game's screen (Redivivus/Lua/builtin/mods, the
// built-in redivivus mod's) over its data, the fixtures by default, with named states.

import { h, type Child, type UIRoot, UIEventType } from "redivivus:ui";
import { ModsScreen } from "../../../builtin/mods/ModsScreen";
import { ModsModel, type ModsData } from "../../../builtin/mods/model";
import { fixtureData } from "./fixtures";

/** The screen over its data (the fixtures by default), with named states for previews and tests. */
export function createScreen(data: ModsData = fixtureData()): { view: (this: void) => Child; states: { [name: string]: (this: void, ui: UIRoot) => void } } {
    const model = new ModsModel(data);
    const mods = model.mods();
    return {
        view: () => <ModsScreen model={model} />,
        states: {
            default: () => {},
            pending: () => {
                model.setEnabled(mods[4], true);
                model.setEnabled(mods[3], false);
            },
            changelog: () => model.setDetailTab(1),
            dependencies: () => {
                model.select("act6-maps");
                model.setDetailTab(2);
            },
            broken: () => model.select("broken-hud"),
            search: () => model.setQuery("loot"),
            updates: () => model.setTab(2),
            browse: () => model.setTab(1),
            settings: () => model.setTab(3),
            scrolled: (ui) => {
                ui.layout();
                ui.dispatch({ type: UIEventType.MouseMove, x: 560, y: 420 });
                ui.dispatch({ type: UIEventType.Wheel, x: 560, y: 420, delta: 6 });
            },
            hover: (ui) => {
                ui.layout();
                ui.dispatch({ type: UIEventType.MouseMove, x: 150, y: 150 });
            },
        },
    };
}
