// What the native layer calls in the core (d2_lua_core.cpp keeps this module's table). Each function
// is entered from the host, outside any mod's budget, or from inside a call the host made; the mod
// code it reaches it reaches only through native.call and native.resume.
import { CallStatus } from "../native/core";
import type { UIEventType } from "../redivivus/enums";
import * as ai from "./ai";
import * as channels from "./channels";
import { readonly } from "./proxy";
import * as enumtables from "./enumtables";
import * as events from "./events";
import * as games from "./games";
import * as menu from "./menu";
import * as mods from "./mods";
import { native } from "./native";
import { makeClient, makeGame } from "./objects";
import * as persist from "./persist";
import * as registries from "./registries";
import * as scheduler from "./scheduler";
import * as storage from "./storage";

const SIDE_SERVER = 2;
const SIDE_CLIENT = 4;
const side = native.side();

const mains = new LuaTable<number, object>();

// The enums each public module carries at run time, beside what its chunk exports.
const moduleEnums = new LuaTable<string, string[] | true>();
moduleEnums.set("enums", true);
moduleEnums.set("server", ["AiTargeting", "Alignment", "Event", "MinionOrder", "MonsterAiId", "MonsterId", "MonsterMode", "Outcome", "PointKind", "SkillId", "StatId", "StateId", "StateMask", "UnitType"]);
moduleEnums.set("client", ["Event"]);
moduleEnums.set("test", ["TestUnitKind"]);
moduleEnums.set("mods", ["DependencyState", "ModState"]);
moduleEnums.set("ui", ["Align", "BlendMode", "BoxStyle", "ButtonVariant", "Direction", "FontRole", "Justify", "SizeMode", "TextAlign", "TextColour", "UIEventType"]);

/** A public module's exports for one mod, as the mod gets them: with its enums, read-only. */
export function finishModule(entry: string, exports: object, mod: number): object {
    const values = new LuaTable<string, unknown>();
    let [k, v] = next(exports);
    while (k !== undefined) {
        values.set(k as string, v);
        [k, v] = next(exports, k);
    }
    const list = moduleEnums.get(entry);
    const names = list === true ? enumtables.names() : list ?? [];
    for (const name of names) {
        const t = enumtables.table(mod, name);
        if (t) values.set(name, t);
    }
    return readonly(values, `redivivus:${entry}`);
}

/** A mod's control entry (main) has loaded: its exports. */
export function setMain(mod: number, exports: unknown): void {
    if (exports !== undefined && type(exports) !== "table") {
        error(`main must return a table of exports ({ server = function(game) ... end, client = ... }), not a ${type(exports)}`, 0);
    }
    for (const name of ["server", "client", "menu"]) {
        const fn = exports === undefined ? undefined : rawget(exports as object, name);
        if (fn !== undefined && type(fn) !== "function") error(`main's export '${name}' must be a function, not a ${type(fn)}`, 0);
    }
    mains.set(mod, (exports ?? {}) as object);
}

export function newGame(id: number): void {
    if (games.find(id)) error(`game ${id} is already running`, 0);
    games.create(id);
}

export function storageOf(id: number, mod: number): object | undefined {
    const g = games.find(id);
    return g ? storage.of(g, mod) : undefined;
}

export function markLoaded(id: number, mod: number): void {
    games.find(id)?.loaded.set(mod, true);
}

/** Call every mod's export for this side with its object for the game, in load order. */
export function runEntries(id: number): void {
    const g = games.find(id);
    if (!g) return;
    const exportName = side === SIDE_SERVER ? "server" : side === SIDE_CLIENT ? "client" : undefined;
    if (!exportName) return;
    for (const m of mods.all()) {
        const main = mains.get(m.id);
        if (!main) continue;
        const fn = rawget(main, exportName);
        if (fn === undefined) continue;
        const obj = side === SIDE_SERVER ? makeGame(g, m.id) : makeClient(g, m.id);
        const [status, err, trace] = native.call(m.id, fn, obj);
        if (status === CallStatus.Error) native.logError(m.id, err, trace);
        if (g.ended) return;
    }
}

export function endGame(id: number): boolean {
    ai.endGame(id);
    return games.end(id);
}

/** A think of a monster whose AI id a mod claimed (d2_lua_world.cpp). True when the core ran it. */
export function aiThink(id: number, monster: object, aiName: string, restart: boolean, target: object | undefined, distance: number,
    engaged: boolean): boolean {
    const g = games.find(id);
    if (!g || g.ended) return false;
    return ai.think(g, monster, aiName, restart, target, distance, engaged);
}

/** A monster's script ends: it died or is gone. */
export function aiDrop(id: number, unitId: number): void {
    ai.drop(id, unitId);
}

/** Drop what was registered through the game's objects, ahead of a reload of its mods. */
export function prepareReload(id: number): boolean {
    const g = games.find(id);
    if (!g) return false;
    games.clearRegistrations(g);
    persist.forget(g);
    return true;
}

export function tick(id: number): void {
    const g = games.find(id);
    if (!g) return;
    if (events.isKnown("tick")) {
        const payload = { name: "tick", tick: native.tick() };
        events.dispatch(g, "tick", payload);
        scheduler.wake(g, "tick", payload);
    }
    scheduler.tick(g);
}

/** Raise an event of the game; the payload is the native layer's table. False when unknown. */
export function dispatchEvent(id: number, name: string, payload: object): boolean {
    if (!events.isKnown(name)) {
        native.logLine(`[lua] the host raised unknown event '${name}'`);
        return false;
    }
    const g = games.find(id);
    if (!g) return false;
    rawset(payload, "name", name);
    rawset(payload, "tick", native.tick());
    events.dispatch(g, name, payload);
    scheduler.wake(g, name, payload);
    return true;
}

export function registryCall(id: number, registry: string, entry: string, ...args: unknown[]): unknown {
    const g = games.find(id);
    if (!g) return error(`no game ${id}`, 0);
    if (!registries.exists(registry)) return error(`no registry '${registry}'`, 0);
    return registries.call(g, registry, entry, ...args);
}

function receive(g: games.GameState, mod: number, payload: string, peer: number): void {
    channels.receive(g, mod, payload, peer, side === SIDE_SERVER);
}

/** A message for a mod's half in this game; false when the mod opened no channel in it. */
export function deliver(id: number, mod: number, payload: string, peer: number): boolean {
    const g = games.find(id);
    if (!g || !g.endpoints.get(mod)) return false;
    const [status, err, trace] = native.call(mod, receive, g, mod, payload, peer);
    if (status === CallStatus.Error) native.logError(mod, err, trace);
    return status !== CallStatus.Disabled;
}

export function gameIds(): number[] {
    return games.ids();
}

// --- the main menu (menu.ts) -------------------------------------------------------------------------

let menuRan = false;

/** Call every mod's `menu` export, in load order (the client state, once, when the main menu first comes up). */
export function runMenu(): void {
    if (menuRan || side !== SIDE_CLIENT) return;
    menuRan = true;
    for (const m of mods.all()) {
        const main = mains.get(m.id);
        const fn = main === undefined ? undefined : rawget(main, "menu");
        if (fn === undefined) continue;
        const [status, err, trace] = native.call(m.id, fn, menu.makeMenu(m.id));
        if (status === CallStatus.Error) native.logError(m.id, err, trace);
    }
}

/** The main menu is up (true) or a screen has taken it over (false). */
export function menuUp(up: boolean): void {
    menu.setMenuUp(up);
}

/** The labels of the main menu buttons the mods added. */
export function menuButtons(): string[] {
    return menu.labels();
}

/** Main menu button i (0-based) was pressed. */
export function menuPress(i: number): void {
    menu.press(i);
}

/** 0 no screen, 1 a screen is open, 2 the open one closes, 3 one asked to open. */
export function screenState(): number {
    return menu.state();
}

/** Mount the screen that asked to open; false when it could not be built. */
export function screenMount(width: number, height: number): boolean {
    return menu.mount(width, height);
}

/** Draw a frame of the open screen (inside the frame the host opened). */
export function screenFrame(width: number, height: number): void {
    menu.frame(width, height);
}

/** An input event for the open screen (UIEventType and its fields); true when handled. */
export function screenInput(eventType: string, x: number, y: number, button: number, delta: number, key: number, text: string,
    shift: boolean, ctrl: boolean): boolean {
    return menu.input({ type: eventType as UIEventType, x, y, button, delta, key, text, shift, ctrl });
}

/** The open screen is gone. */
export function screenDispose(): void {
    menu.dispose();
}
