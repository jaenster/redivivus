// Server-only API in client code does not compile.

script.on_event(Event.Draw, () => {});

// @ts-expect-error storage is server-only
storage.x = 1;

// @ts-expect-error registries are server-only
registries.example.call(ExampleId.Greet, "x");

// @ts-expect-error scripts are server-only
script.start(function* (): Script {});

// @ts-expect-error on_test... and server events are not client events
script.on_event("on_tick_server", () => {});

// @ts-expect-error a string is not an Event, on the client either
script.on_event("on_draw", () => {});
