// The kit, typed: channels, settings, scripts, persisted state.
import { defineChannel, t } from "__base__/shared/kit/channels";
import { defineSettings, setting } from "__base__/shared/kit/settings";
import { startScript, waitTicks, waitUntil } from "__base__/server/kit/scripts";
import { persist } from "__base__/server/kit/persist";

const enum Colour {
    White = "white",
    Gold = "gold",
}

const chat = defineChannel("chat", {
    requests: { say: [t.text(20), t.oneOf([Colour.White, Colour.Gold])], move: [t.record({ x: t.int(), y: t.int() })] },
    events: { said: [t.int(), t.text(20), t.optional(t.list(t.bool(), 4))] },
});

chat.implement({
    say(peer, text, colour) {
        const p: integer = peer;
        const s: string = text;
        const c: Colour = colour;
        void [p, s, c];
    },
    move(peer, pos) {
        const x: number = pos.x;
        void [peer, x];
    },
});
chat.emit.said(0, 1, "x", [true]);
chat.emit.said(3, 1, "x", undefined);

// @ts-expect-error every request needs a handler
chat.implement({ say() {} });

// @ts-expect-error a request handler gets the declared types
chat.implement({ say(peer, text: number) { void [peer, text]; }, move() {} });

// @ts-expect-error the argument is declared as text
chat.call.say(5, Colour.White);

// @ts-expect-error a choice outside the declared values
chat.call.say("x", "blue");

// @ts-expect-error no such request
chat.call.shout("x");

// @ts-expect-error an event goes to a peer, given first
chat.emit.said(1, "x", undefined);

// @ts-expect-error a record with a field missing
chat.call.move({ x: 1 });

chat.on.said((from, text, flags) => {
    const f: integer = from;
    const s: string = text;
    const l: boolean[] | undefined = flags;
    void [f, s, l];
});

const settings = defineSettings({
    boost: setting.int({ default: 1, min: 0, max: 5, scope: SettingScope.Startup }),
    colour: setting.choice({ choices: [Colour.White, Colour.Gold], default: Colour.White, scope: SettingScope.Game }),
    hud: setting.bool({ default: true, scope: SettingScope.Player }),
});
const boost: number = settings.boost;
const colour: Colour = settings.colour;
const hud: boolean = settings.hud;
void [boost, colour, hud];

// @ts-expect-error settings are read-only
settings.boost = 2;

// @ts-expect-error a boolean setting is not a string
const hudText: string = settings.hud;

// @ts-expect-error no such setting
void settings.speed;

// @ts-expect-error the default must be one of the choices
setting.choice({ choices: [Colour.White], default: Colour.Gold, scope: SettingScope.Game });

// @ts-expect-error a scope is a SettingScope member, not a string
setting.bool({ default: true, scope: "startup" });
void hudText;

declare const boss: TestUnitLike;
interface TestUnitLike { readonly valid: boolean; readonly health: number }

const task = startScript(function* () {
    const tick: integer = yield* waitTicks(2);
    const outcome: Outcome = yield* waitUntil(() => boss.health < 10, 100);
    // @ts-expect-error waitUntil gives an Outcome
    const wrong: string = yield* waitUntil(() => true, 1);
    void [tick, outcome, wrong];
}, boss);
const running: boolean = task.isRunning();
const cancelled: boolean = task.cancel();
void [running, cancelled];

// @ts-expect-error startScript takes a Script function
startScript(() => 5);

// @ts-expect-error waits always have a timeout
startScript(function* () { yield* waitUntil(() => true); });

const state = persist(boss, { charges: 0, names: ["a"] });
state.charges++;
const first: string = state.names[0];
void first;

// @ts-expect-error the state has the defaults' shape
void state.missing;

// @ts-expect-error the owner is a handle
persist({ health: 1 }, {});
