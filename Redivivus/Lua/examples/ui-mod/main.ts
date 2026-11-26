// Every redivivus module loads in every state, the UI kit too: main imports the panel directly.
// The server state has no renderer and no input, so the panel the server export builds is never
// drawn by the game; it is made, laid out and recorded all the same. In the client the menu
// export adds a main menu button that opens the same counter as a full screen.
import type { Client, Menu } from "redivivus:client";
import type { Game } from "redivivus:server";
import { CounterScreen, open } from "./panel";

export function server(game: Game) {
    open("server", game.id);
}

export function client(client: Client) {
    open("client", client.gameId);
}

export function menu(menu: Menu) {
    const screen = menu.addScreen((s) => CounterScreen({ close: () => s.close() }));
    menu.addButton("UI MOD", () => screen.open());
}
