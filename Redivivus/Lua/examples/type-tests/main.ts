// The control entry, correct: the checks of lifecycle exports find nothing here.
import type { Client } from "redivivus:client";
import type { Game } from "redivivus:server";
import { onClient } from "./client";
import { onServer } from "./events";

export function server(game: Game) {
    onServer(game);
}

export function client(client: Client) {
    onClient(client);
}

// Other exports are fine: other mods import them.
export const version = 1;
