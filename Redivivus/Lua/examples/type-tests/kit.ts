// Channels, settings, scripts, persisted state and storage, typed.
import type { Client } from "redivivus:client";
import { Outcome, SettingScope } from "redivivus:enums";
import type { Game } from "redivivus:server";
import { defineChannel, defineSettings, setting, t, wait, waitUntil, type Handle } from "redivivus:shared";

const enum Colour {
    White = "white",
    Gold = "gold",
}

const chat = defineChannel("chat", {
    requests: { say: [t.text(20), t.oneOf([Colour.White, Colour.Gold])], move: [t.record({ x: t.int(), y: t.int() })] },
    events: { said: [t.int(), t.text(20), t.optional(t.list(t.bool(), 4))] },
});

export function serverSide(game: Game, boss: Handle & { readonly health: number }) {
    const ch = game.channel(chat);
    ch.implement({
        say(peer, text, colour) {
            const p: number = peer;
            const c: Colour = colour;
            void [p, text, c];
        },
        move(peer, pos) {
            const x: number = pos.x;
            void [peer, x];
        },
    });
    ch.emit.said(0, 1, "x", [true]);
    // @ts-expect-error every request needs a handler
    ch.implement({ say() {} });
    // @ts-expect-error an event goes to a peer, given first
    ch.emit.said(1, "x", undefined);

    const kills = game.storage("kills", { count: 0, names: [] as string[] });
    kills.count++;
    // @ts-expect-error the slot has the defaults' shape
    kills.missing;

    const task = game.startScript(function* () {
        const tick: number = yield* wait(2);
        const outcome: Outcome = yield* waitUntil(() => boss.health < 10, 100);
        // @ts-expect-error waitUntil gives an Outcome
        const wrong: string = yield* waitUntil(() => true, 1);
        void [tick, outcome, wrong];
    }, boss);
    const running: boolean = task.isRunning();
    void running;
    // @ts-expect-error startScript takes a Script function
    game.startScript(() => 5);
    // @ts-expect-error waits always have a timeout
    game.startScript(function* () { yield* waitUntil(() => true); });

    const state = game.persist(boss, { charges: 0 });
    state.charges++;
    // @ts-expect-error the state has the defaults' shape
    void state.missing;
    // @ts-expect-error the owner is a handle
    game.persist({ health: 1 }, {});
}

export function clientSide(client: Client) {
    const ch = client.channel(chat);
    ch.call.say("hi", Colour.Gold);
    ch.on.said((from, text, flags) => void [from, text, flags]);
    // @ts-expect-error the argument is declared as text
    ch.call.say(5, Colour.White);
    // @ts-expect-error a choice outside the declared values
    ch.call.say("x", "blue");
    // @ts-expect-error a client implements nothing
    ch.implement;
}

const settings = defineSettings({
    boost: setting.int({ default: 1, min: 0, max: 5, scope: SettingScope.Startup }),
    colour: setting.choice({ choices: [Colour.White, Colour.Gold], default: Colour.White, scope: SettingScope.Game }),
});
const boost: number = settings.boost;
void boost;
// @ts-expect-error settings are read-only
settings.boost = 2;
// @ts-expect-error the default must be one of the choices
setting.choice({ choices: [Colour.White], default: Colour.Gold, scope: SettingScope.Game });
// @ts-expect-error a scope is a SettingScope member, not a string
setting.bool({ default: true, scope: "startup" });
