// Registry ids are the keys of the registry's interface, extended by declaration merging.

registries.example.call(ExampleId.Greet, "x");
registries.example.call(ExampleId.Shout, "x");
registries.example.wrap(ExampleId.Greet, (next, name: string) => next(name));

// @ts-expect-error an id nobody declared
registries.example.call("no-such:id", "x");

// @ts-expect-error wrapping an id nobody declared
registries.example.wrap("base:greeet", (next, name) => next(name));

// @ts-expect-error the entry takes a string
registries.example.call(ExampleId.Greet, 5);

// @ts-expect-error a wrapper must return what the entry returns
registries.example.wrap(ExampleId.Greet, (next, name) => 5);

// @ts-expect-error a slot registry has no ids
registries.example_slot.call(ExampleId.Greet);

// @ts-expect-error remote interfaces are typed too
remote.call("first-mod", "subtract", 1, 2);

// @ts-expect-error registry ids are ExampleId members; the id as a string is refused
registries.example.call("base:greet", "x");

// @ts-expect-error not for wrap either
registries.example.wrap("base:greet", (next, name: string) => next(name));

// @ts-expect-error nor for register
registries.example.register("first-mod:shout", (text: string) => text);
