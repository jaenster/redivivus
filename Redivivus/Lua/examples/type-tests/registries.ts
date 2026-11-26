// Registry ids are the keys of the registry's interface, extended by augmentation.
import { ExampleId } from "redivivus:enums";
import type { Game } from "redivivus:server";

export function registries(game: Game) {
    game.registry.example.call(ExampleId.Greet, "x");
    game.registry.example.call(ExampleId.Shout, "x");
    game.registry.example.wrap(ExampleId.Greet, (next, name: string) => next(name));

    // @ts-expect-error an id nobody declared
    game.registry.example.call("no-such:id", "x");

    // @ts-expect-error the entry takes a string
    game.registry.example.call(ExampleId.Greet, 5);

    // @ts-expect-error a wrapper must return what the entry returns
    game.registry.example.wrap(ExampleId.Greet, (next, name) => 5);

    // @ts-expect-error a slot registry has no ids
    game.registry.exampleSlot.call(ExampleId.Greet);

    // @ts-expect-error registry ids are ExampleId members; the id as a string is refused
    game.registry.example.call("redivivus:greet", "x");
}
