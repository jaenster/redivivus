// Scripts: yield* takes actions; actions only run inside scripts.
import { walk } from "first-mod:actions";
import type { Game, Monster, Outcome } from "redivivus:server";
import { guard, wait, type Script } from "redivivus:shared";

export function scripts(game: Game) {
    function* good(): Script {
        yield;
        const tick: number = yield* wait(2);
        const walked: number = yield* walk(3);
        void [tick, walked];
    }
    game.startScript(good);

    function* typed(): Script {
        // @ts-expect-error walk gives a number, not a string
        const s: string = yield* walk(3);
        void s;
    }
    void typed;

    // guard: the body is a Script call, handed over unstarted; the outcome is the body's or Interrupted.
    function* chase(monster: Monster, target: Monster): Script<string> {
        yield* monster.walkToward(target);
        return "caught";
    }
    function* guarded(monster: Monster, target: Monster): Script {
        const o: string | Outcome = yield* guard(() => target.valid, chase(monster, target));
        const p: Outcome = yield* guard(() => true, monster.idle(5));
        const nested: string | Outcome = yield* guard(() => true, guard(() => monster.valid, chase(monster, target)));
        void [o, p, nested];
        // @ts-expect-error the outcome may be Interrupted, not only the body's string
        const s: string = yield* guard(() => true, chase(monster, target));
        // @ts-expect-error the check gives a boolean
        yield* guard(() => 1, chase(monster, target));
        // @ts-expect-error the body is a Script, not a function that makes one
        yield* guard(() => true, chase);
        void s;
    }
    void guarded;
}
