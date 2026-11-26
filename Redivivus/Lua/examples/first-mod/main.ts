// first-mod's control entry: one program, loaded in the server state and in the client state. The
// engine calls server(game) for every game this process hosts and client(client) for every game
// this client is in; everything is registered through the object each one gets.
import type { Client } from "redivivus:client";
import { ExampleId, ItemQuality, Outcome, SkillId, UnitType } from "redivivus:enums";
import { Event, type Game } from "redivivus:server";
import { log, waitUntil, wait, type Script } from "redivivus:shared";
import { walk } from "./actions";
import { greet } from "./greet";
import { net } from "./net";
import { options, Pace } from "./options";

// The TypeScriptToLua runtime comes from redivivus: the map below needs it.
const doubled = [1, 2, 3].map((x) => x * 2);
log("first-mod main loaded, lualib from redivivus", doubled.length);

function* inner(n: number): Script<number> {
    for (let i = 0; i < n; i++) {
        yield;
    }
    return n;
}

function* middle(n: number): Script<number> {
    return yield* inner(n);
}

function* outer(n: number): Script<number> {
    const result = yield* middle(n);
    return result;
}

export function server(game: Game) {
    const state = game.storage("greeting", { text: "" });
    log(`first-mod server(game ${game.id}), ${game.loaded ? "loaded" : "new game"}`);
    // Engine enums are inlined: the Lua has the numbers.
    log("engine enums:", UnitType.Monster, SkillId.Teleport, ItemQuality.Unique);
    log("kit settings:", options.greetings, options.pace);
    state.text = greet("storage");

    game.startScript(function* (): Script {
        log(`ts script: started at ${game.tick}`);
        yield;
        log(`ts script: bare yield resumed at ${game.tick}`);
        const resumedAt: number = yield { ticks: 2 };
        log(`ts script: wait value outcome ${resumedAt}`);
        const n = yield* outer(3);
        log(`ts script: nested actions gave ${n} ticks, ended at ${game.tick}`);
        yield* walk(1);
        yield* wait(1);
        log("ts script: done");
    });
    game.startScript(function* () {
        const outcome = yield* waitUntil(() => game.tick >= 3, 10);
        log("waitUntil:", outcome === Outcome.Done, game.tick);
    });

    game.on(Event.Tick, (e) => {
        if (e.tick === 12) {
            error("first-mod fails on tick 12"); // THROW-ON-TICK-12
        }
    });

    game.registry.example.wrap(ExampleId.Greet, (next, name) => `first-mod(${next(name)})`);
    game.registry.example.register(ExampleId.Shout, (text) => `<${text.toUpperCase()}>`);

    const channel = game.channel(net);
    channel.implement({
        wave: (peer, text, pace) => channel.emit.waved(0, peer, `${text} (${pace})`),
    });
}

export function client(client: Client) {
    const channel = client.channel(net);
    channel.on.waved((from, text) => log(`peer ${from} waved: ${text}`));
    channel.call.wave("hi", Pace.Fast);
    client.on(Event.Draw, () => {
        void greet("client");
    });
}
