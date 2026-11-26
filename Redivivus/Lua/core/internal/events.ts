// Event handlers of a game: one per mod per event, called in load order, each as its mod (a call
// through the native layer: the mod is the owner, its own budget when the host raised the event).
import { CallStatus } from "../native/core";
import type { GameState, Handler } from "./games";
import { native } from "./native";

const known = native.events();

/** Whether name is an event this side raises. */
export function isKnown(name: unknown): boolean {
    return type(name) === "string" && known.get(name as string) !== undefined;
}

export function subscribe(g: GameState, mod: number, order: number, event: string, fn: ((this: void, e: object) => void) | undefined): void {
    const old = g.handlers.get(event) ?? [];
    const list: Handler[] = [];
    let placed = fn === undefined;
    for (const h of old) {
        if (h.mod === mod) continue;
        if (!placed && h.order > order) {
            list.push({ mod, order, fn: fn! });
            placed = true;
        }
        list.push(h);
    }
    if (!placed) list.push({ mod, order, fn: fn! });
    g.handlers.set(event, list);
}

/**
 * Call every handler of the event with the payload. An error is logged and the next handler runs;
 * a budget that ran out inside an entry still running stops the dispatch and goes up.
 */
export function dispatch(g: GameState, name: string, payload: object): void {
    const list = g.handlers.get(name);
    if (!list) return;
    for (const h of list) {
        if (g.ended) return;
        const [status, err, trace] = native.call(h.mod, h.fn, payload);
        if (status === CallStatus.Error) {
            native.logError(h.mod, err, trace);
        } else if (status === CallStatus.Fatal) {
            error(err, 0);
        }
    }
}
