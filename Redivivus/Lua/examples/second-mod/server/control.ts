import { double, greet } from "__first-mod__/shared/greet";

log(`second-mod imports from first-mod: ${greet("second-mod")} (3 letters doubled: ${double(3)})`);

// Wraps an entry of first-mod's, typed from first-mod's declaration of it.
registries.example.wrap(ExampleId.Shout, (next, text) => next(text).replace(">", "!>"));
registries.example.wrap(ExampleId.Greet, (next, name) => `second-mod{${next(name)}}`);

script.on_load(() => {
    log("second-mod on_load", remote.call("first-mod", "add", 1, 2));
});
