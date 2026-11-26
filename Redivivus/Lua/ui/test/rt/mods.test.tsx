// The Mods screen reads everything through ModsData and hands its changes back through it.

import { UIEventType } from "redivivus:ui";
import { fixtureData } from "../../screens/mods/fixtures";
import { createScreen } from "../../screens/mods/ModsScreen";
import type { ModsData } from "../../../builtin/mods/model";
import { byId, eq, find, mountTest, ok, show, test } from "./harness";

function click(ui: { dispatch: (e: { type: UIEventType; x: number; y: number }) => boolean }, x: number, y: number): void {
    ui.dispatch({ type: UIEventType.MouseDown, x, y });
    ui.dispatch({ type: UIEventType.MouseUp, x, y });
}

test("the Mods screen asks its data for the README of the selected mod", () => {
    const base = fixtureData();
    const asked: string[] = [];
    const data: ModsData = {
        installedMods: base.installedMods,
        readme: (mod: string) => {
            asked.push(mod);
            return "# From the host";
        },
        changelog: base.changelog,
        portalMods: base.portalMods,
        settings: base.settings,
        apply: base.apply,
        back: base.back,
    };
    const screen = createScreen(data);
    const { ui } = mountTest(screen.view, 800, 600);
    ui.layout();
    eq(asked, ["charger"]);
    ok(find(ui.root, (w) => show(w.props.spans).indexOf("From the host") >= 0) !== undefined, "the README shown is the data's");
});

test("Apply hands the pending changes to the data; Back asks it to leave", () => {
    const data = fixtureData();
    const screen = createScreen(data);
    const { ui } = mountTest(screen.view, 800, 600);
    screen.states.pending(ui);
    ui.layout();
    const apply = byId(ui.root, "mods-apply");
    click(ui, apply.x + 4, apply.y + 4);
    eq(data.applied, [{ enabled: { "broken-hud": false, "stash-plus": true } }]);
    const back = byId(ui.root, "mods-back");
    click(ui, back.x + 4, back.y + 4);
    eq(data.backs, 1);
});
