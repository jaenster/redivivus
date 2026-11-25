// Generated from Redivivus/Lua/kit/server/scripts.ts. Do not edit.
/** A running script. */
export interface ScriptTask {
    readonly id: integer;
    /** Stop it; it is not resumed again. True when it was still running. */
    readonly cancel: (this: void) => boolean;
    /** Whether it is still running. */
    readonly isRunning: (this: void) => boolean;
}
/** Start a script, owned by this mod and, when given, by owner. It first runs on the next tick. */
export declare function startScript(body: (this: void) => Script<void>, owner?: Handle): ScriptTask;
/** Wait one tick. */
export declare function wait(): Script<void>;
/** Wait n ticks (at least one). The outcome is the tick the script resumes on. */
export declare function waitTicks(n: integer): Script<integer>;
/**
 * Wait until pred holds, checking it now and then once a tick, for at most timeoutTicks ticks.
 * Outcome.Done when it held, Outcome.Timeout when the time ran out first.
 */
export declare function waitUntil(pred: (this: void) => boolean, timeoutTicks: integer): Script<Outcome>;
