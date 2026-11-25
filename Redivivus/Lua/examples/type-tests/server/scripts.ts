// Scripts: yield* takes actions; actions only run inside scripts.
import { walk } from "__first-mod__/shared/actions";
import { wait_ticks } from "__base__/shared/actions";

function* good(): Script {
    yield;
    const tick: integer = yield* script.wait(2);
    const walked: integer = yield* walk(3);
    const lua: integer = yield* wait_ticks(1);
    const payload = yield* script.wait_event(Event.Tick);
    void [tick, walked, lua, payload.tick];
}
script.start(good);

function* typed(): Script {
    // @ts-expect-error walk gives an integer, not a string
    const s: string = yield* walk(3);
    void s;
}
void typed;

// @ts-expect-error script.start takes a Script function
script.start(() => 5);
