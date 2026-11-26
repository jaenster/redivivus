// The game's own redivivus mod: its program, compiled like any mod's (rdvmod) against the same
// declarations and embedded in the exe. It adds the main menu's MODS button and the Mods screen
// through the menu export every mod has; only redivivus:manager (used in data.ts) is its alone.

import type { Menu } from "redivivus:client";
import { h } from "redivivus:ui";
import { gameData } from "./data";
import { ModsScreen } from "./mods/ModsScreen";
import { ModsModel } from "./mods/model";

export function menu(menu: Menu): void {
    const screen = menu.addScreen((s) => <ModsScreen model={new ModsModel(gameData(() => s.close()))} />);
    menu.addButton("MODS", () => screen.open());
}
