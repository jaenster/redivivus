// The client object: its own events, no registries, no persisted state.
import { Event, type Client } from "redivivus:client";

export function onClient(client: Client) {
    client.on(Event.Draw, (e) => void e.tick);
    client.on(Event.Tick, () => {});

    // @ts-expect-error a client has no registries
    client.registry;

    // @ts-expect-error nor storage
    client.storage("x", {});

    // @ts-expect-error a string is not an Event
    client.on("draw", () => {});
}
