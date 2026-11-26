import { double, greet } from "first-mod:greet";
import { ExampleId } from "redivivus:enums";
import type { Game } from "redivivus:server";
import { log } from "redivivus:shared";

log(`second-mod imports from first-mod: ${greet("second-mod")} (3 letters doubled: ${double(3)})`);

export function server(game: Game) {
    // Wraps an entry of first-mod's, typed from first-mod's declaration of it.
    game.registry.example.wrap(ExampleId.Shout, (next, text) => next(text).replace(">", "!>"));
    game.registry.example.wrap(ExampleId.Greet, (next, name) => `second-mod{${next(name)}}`);
    // For the host test, which looks at the game from outside.
    (_G as unknown as Record<string, unknown>).ts_game = game;
}
