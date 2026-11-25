import { wait_ticks } from "__base__/shared/actions";
import { startScript, waitUntil } from "__base__/server/kit/scripts";
import { greet } from "../shared/greet";
import { net } from "../shared/net";
import { settings } from "../shared/settings";

// The TypeScriptToLua runtime comes from base: the map below needs it.
const doubled = [1, 2, 3].map((x) => x * 2);
log(`first-mod control loaded, lualib from base`, doubled.length);

function* inner(n: integer): Script<integer> {
    for (let i = 0; i < n; i++) {
        yield;
    }
    return n;
}

function* middle(n: integer): Script<integer> {
    return yield* inner(n);
}

function* outer(n: integer): Script<integer> {
    const result = yield* middle(n);
    return result;
}

function* main(): Script {
    log(`ts script: started at ${script.tick}`);
    yield;
    log(`ts script: bare yield resumed at ${script.tick}`);
    const resumedAt: integer = yield { ticks: 2 };
    log(`ts script: wait value outcome ${resumedAt}`);
    const n = yield* outer(3);
    log(`ts script: nested actions gave ${n} ticks, ended at ${script.tick}`);
    const luaTick = yield* wait_ticks(2);
    log(`ts script: lua action gave ${luaTick}`);
    log("ts script: done");
}

script.on_init(() => {
    log("first-mod on_init");
    // Engine enums are inlined: the Lua has the numbers.
    log("engine enums:", UnitType.Monster, SkillId.Teleport, ItemQuality.Unique);
    storage.greeting = greet("storage");
    script.start(main);
    log("kit settings:", settings.greetings, settings.pace);
    startScript(function* () {
        const outcome = yield* waitUntil(() => script.tick >= 3, 10);
        log("kit waitUntil:", outcome === Outcome.Done, script.tick);
    });
});

script.on_event(Event.Tick, (e) => {
    if (e.tick === 12) {
        error("first-mod fails on tick 12"); // THROW-ON-TICK-12
    }
});

registries.example.wrap(ExampleId.Greet, (next, name) => `first-mod(${next(name)})`);
registries.example.register(ExampleId.Shout, (text) => `<${text.toUpperCase()}>`);

net.implement({
    wave: (peer, text, pace) => net.emit.waved(0, peer, `${text} (${pace})`),
});

remote.add_interface("first-mod", {
    add: (a: number, b: number) => a + b,
});
