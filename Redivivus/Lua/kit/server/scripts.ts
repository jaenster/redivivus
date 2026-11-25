// General scripts: sequential code that waits, resumed by the host's tick.
// A script belongs to the mod that starts it, and optionally to an owner
// handle: once the owner's object is gone the script is dropped. Every tick, after the on_tick
// handlers, the host resumes the scripts that are due in the order they were started.
//
//   const task = startScript(function* () {
//       yield* waitTicks(25);
//       const outcome = yield* waitUntil(() => boss.health < 100, 250);
//       if (outcome === Outcome.Timeout) return;
//   }, boss);
//   task.cancel();
//
// From Lua the same functions are plain calls inside the script body: kit.waitTicks(25).

/** A running script. */
export interface ScriptTask {
    readonly id: integer;
    /** Stop it; it is not resumed again. True when it was still running. */
    readonly cancel: (this: void) => boolean;
    /** Whether it is still running. */
    readonly isRunning: (this: void) => boolean;
}

/** Start a script, owned by this mod and, when given, by owner. It first runs on the next tick. */
export function startScript(body: (this: void) => Script<void>, owner?: Handle): ScriptTask {
    const id = script.start(body, owner);
    return {
        id,
        cancel: () => script.cancel(id),
        isRunning: () => script.running(id),
    };
}

/** Wait one tick. */
export function* wait(): Script<void> {
    yield;
}

/** Wait n ticks (at least one). The outcome is the tick the script resumes on. */
export function* waitTicks(n: integer): Script<integer> {
    return yield* script.wait(n);
}

/**
 * Wait until pred holds, checking it now and then once a tick, for at most timeoutTicks ticks.
 * Outcome.Done when it held, Outcome.Timeout when the time ran out first.
 */
export function* waitUntil(pred: (this: void) => boolean, timeoutTicks: integer): Script<Outcome> {
    if (pred()) return Outcome.Done;
    for (let i = 0; i < timeoutTicks; i++) {
        yield;
        if (pred()) return Outcome.Done;
    }
    return Outcome.Timeout;
}
