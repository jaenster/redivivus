// Events: Event members, checked against the side's events map; handlers are typed by it.
import { Event, type Game } from "redivivus:server";
import type { Script } from "redivivus:shared";

export function onServer(game: Game) {
    game.on(Event.Tick, (e) => {
        const tick: number = e.tick;
        void tick;
    });

    // @ts-expect-error a misspelled event name
    game.on("tik", () => {});

    // @ts-expect-error the payload has no such field
    game.on(Event.Tick, (e) => e.unit);

    // @ts-expect-error handlers take no self
    game.on(Event.Tick, function (this: { x: number }) {});

    // @ts-expect-error event names are Event members; the name as a string is refused
    game.on("tick", () => {});

    // @ts-expect-error the client's draw event is not a game event
    game.on(Event.Draw, () => {});

    game.startScript(function* (): Script {
        const payload = yield* game.waitEvent(Event.Tick);
        void payload.tick;
        // @ts-expect-error waitEvent takes an Event member too
        yield* game.waitEvent("tick");
    });

    // @ts-expect-error a game has no client channel calls
    game.channel(undefined as never).call;
}
