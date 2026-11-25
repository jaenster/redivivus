// Events are Event members, checked against ServerEvents.

script.on_event(Event.Tick, (e) => {
    const tick: number = e.tick;
    void tick;
});

// @ts-expect-error a misspelled event name
script.on_event("on_tik", () => {});

// @ts-expect-error the payload has no such field
script.on_event(Event.Tick, (e) => e.unit);

// @ts-expect-error handlers take no self
script.on_event(Event.Tick, function (this: { x: number }) {});

// @ts-expect-error event names are Event members; the name as a string is refused
script.on_event("on_tick", () => {});

// @ts-expect-error the client's draw event is not raised on the server
script.on_event(Event.Draw, () => {});

function* waits(): Script {
    // @ts-expect-error wait_event takes an Event member too
    yield* script.wait_event("on_tick");
}
script.start(waits);
