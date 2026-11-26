// Actions for scripts, written in TypeScript. A Script function compiles to a plain Lua function
// that yields, so a Lua script calls it as an ordinary function.
import type { Script } from "redivivus:shared";

/** Walk `steps` ticks, one step a tick; the outcome is twice the steps walked. */
export function* walk(steps: number): Script<number> {
    let walked = 0;
    for (let i = 0; i < steps; i++) {
        yield;
        walked++;
    }
    return walked * 2;
}
