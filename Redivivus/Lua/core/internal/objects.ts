// The objects a mod's exports receive: a Game (server) or a Client, one per mod per game. Every
// method closes over the game and the mod, checks its arguments by type, and refuses once the game
// has ended. Nothing registered through one reaches another game.
import type { Client, ClientChannel } from "../redivivus/client";
import type { ExampleRegistry, ExampleSlot, Game, GameRegistries, ServerChannel } from "../redivivus/server";
import * as ai from "./ai";
import type { ChannelDeclaration, Handle, KeyedRegistry, Messages, Script, ScriptTask, SlotRegistry } from "../redivivus/shared";
import * as channels from "./channels";
import { argFunction, argOptionalFunction, argString, argTable, show } from "./check";
import * as events from "./events";
import type { GameState, ScriptRecord } from "./games";
import * as mods from "./mods";
import { native, world } from "./native";
import * as worldClasses from "./world";
import * as persist from "./persist";
import { readonly } from "./proxy";
import * as registries from "./registries";
import * as scheduler from "./scheduler";
import * as storage from "./storage";

type AnyFn = (this: void, ...args: any[]) => any;

function live(g: GameState, where: string): void {
    if (g.ended) error(`${where}: this game has ended`, 3);
}

function eventArg(v: unknown, where: string): string {
    if (!events.isKnown(v)) error(`bad argument #1 to '${where}' (no event ${show(v)} on this side)`, 3);
    return v as string;
}

function task(rec: ScriptRecord): ScriptTask {
    return readonly<ScriptTask>({
        id: rec.id,
        cancel: () => {
            const was = !rec.done;
            rec.done = true;
            return was;
        },
        isRunning: () => !rec.done,
    }, "task");
}

function startScript(g: GameState, mod: number, body: unknown, owner: unknown, where: string): ScriptTask {
    live(g, where);
    argFunction(body, 1, where);
    if (owner !== undefined) {
        if (!native.isHandle(owner)) error(`bad argument #2 to '${where}' (a handle expected as the owner, got ${type(owner)})`, 3);
        if (!native.handleIsLive(owner)) error(`bad argument #2 to '${where}' (the owner is not valid)`, 3);
    }
    return task(scheduler.start(g, mod, body as (this: void) => unknown, owner));
}

function registryFacade(g: GameState, mod: number): GameRegistries {
    const out = new LuaTable<string, object>();
    if (world) out.set("ai", ai.facade(g, mod, (where: string) => live(g, where)));
    for (const [name, kind] of registries.names()) {
        const keyed = kind === registries.KEYED;
        const w = (fn: string) => `registry.${name}.${fn}`;
        if (keyed) {
            out.set(name, readonly<KeyedRegistry<ExampleRegistry>>({
                register: (id: unknown, impl: unknown) => {
                    live(g, w("register"));
                    registries.register(g, mod, name, argString(id, 1, w("register")), argFunction<AnyFn>(impl, 2, w("register")));
                },
                wrap: (id: unknown, fn: unknown) => {
                    live(g, w("wrap"));
                    const sid = argString(id, 1, w("wrap"));
                    if (!registries.wrap(g, mod, name, sid, argFunction<AnyFn>(fn, 2, w("wrap")))) {
                        error(`registry ${name} has no entry ${show(sid)} to wrap`, 2);
                    }
                },
                call: (id: unknown, ...args: unknown[]) => {
                    live(g, w("call"));
                    return registries.call(g, name, argString(id, 1, w("call")), ...args);
                },
                has: (id: unknown) => registries.has(g, name, argString(id, 1, w("has"))),
            } as never, `registry.${name}`));
        } else {
            out.set(name, readonly<SlotRegistry<ExampleSlot>>({
                set: (impl: unknown) => {
                    live(g, w("set"));
                    registries.register(g, mod, name, "", argFunction<AnyFn>(impl, 1, w("set")));
                },
                wrap: (fn: unknown) => {
                    live(g, w("wrap"));
                    registries.wrap(g, mod, name, "", argFunction<AnyFn>(fn, 1, w("wrap")));
                },
                call: (...args: unknown[]) => {
                    live(g, w("call"));
                    return registries.call(g, name, "", ...args);
                },
            } as never, `registry.${name}`));
        }
    }
    return readonly<GameRegistries>(out, "registry");
}

function sendTo(g: GameState, mod: number, kind: number, ep: ReturnType<typeof channels.open>, index: number, peer: number, args: unknown[]): void {
    const entry = (kind === channels.REQUEST ? ep.requests : ep.events)[index];
    native.send(g.id, mod, channels.encode(kind, ep, index, entry, args, args.length), peer);
}

function serverChannel(g: GameState, mod: number, decl: unknown): ServerChannel<Messages, Messages> {
    live(g, "game.channel");
    const d = channels.declarationArg(decl, "game.channel");
    const ep = channels.open(g, mod, d, "game.channel");
    const emit = new LuaTable<string, AnyFn>();
    ep.events.forEach((entry, i) => {
        emit.set(entry.name, (peer: unknown, ...args: unknown[]) => {
            live(g, `${d.name}.emit.${entry.name}`);
            if (type(peer) !== "number") error(`bad argument #1 to '${d.name}.emit.${entry.name}' (a peer id expected, got ${type(peer)})`, 2);
            sendTo(g, mod, channels.EVENT, ep, i, peer as number, args);
        });
    });
    return readonly<ServerChannel<Messages, Messages>>({
        name: d.name,
        implement: (handlers: unknown) => {
            live(g, `${d.name}.implement`);
            argTable(handlers, 1, `${d.name}.implement`);
            const found: AnyFn[] = [];
            for (const entry of ep.requests) {
                const h = rawget(handlers as object, entry.name);
                if (type(h) !== "function") error(`${d.name}.implement: no handler for request '${entry.name}'`, 2);
                found.push(h);
            }
            ep.requests.forEach((entry, i) => {
                entry.handler = found[i];
            });
        },
        emit: readonly(emit, `${d.name}.emit`),
    }, `channel ${d.name}`);
}

function clientChannel(g: GameState, mod: number, decl: unknown): ClientChannel<Messages, Messages> {
    live(g, "client.channel");
    const d = channels.declarationArg(decl, "client.channel");
    const ep = channels.open(g, mod, d, "client.channel");
    const call = new LuaTable<string, AnyFn>();
    const on = new LuaTable<string, AnyFn>();
    ep.requests.forEach((entry, i) => {
        call.set(entry.name, (...args: unknown[]) => {
            live(g, `${d.name}.call.${entry.name}`);
            sendTo(g, mod, channels.REQUEST, ep, i, 0, args);
        });
    });
    ep.events.forEach((entry) => {
        on.set(entry.name, (handler: unknown) => {
            live(g, `${d.name}.on.${entry.name}`);
            entry.handler = argFunction<AnyFn>(handler, 1, `${d.name}.on.${entry.name}`);
        });
    });
    return readonly<ClientChannel<Messages, Messages>>({ name: d.name, call: readonly(call, `${d.name}.call`), on: readonly(on, `${d.name}.on`) },
        `channel ${d.name}`);
}

function persistOwner(owner: unknown, where: string): void {
    if (!native.handleIsLive(owner)) error(`${where}: the owner must be a valid handle`, 3);
}

export function makeGame(g: GameState, mod: number): Game {
    const order = mods.order(mod);
    const registry = registryFacade(g, mod);
    const getters = new LuaTable<string, (this: void) => unknown>();
    getters.set("tick", () => native.tick());
    getters.set("loaded", () => g.loaded.get(mod) === true);
    const values: Record<string, unknown> = {
        id: g.id,
        storage: (key: unknown, defaults: unknown) => {
            live(g, "game.storage");
            const k = argString(key, 1, "game.storage");
            if (string.sub(k, 1, 2) === "__") error("game.storage: keys starting with __ are the core's", 2);
            argTable(defaults, 2, "game.storage");
            return storage.slot(g, mod, k, defaults as object);
        },
        registry,
        on: (event: unknown, handler: unknown) => {
            live(g, "game.on");
            events.subscribe(g, mod, order, eventArg(event, "game.on"), argOptionalFunction(handler, 2, "game.on"));
        },
        startScript: (body: unknown, owner?: unknown) => startScript(g, mod, body, owner, "game.startScript"),
        waitEvent: (event: unknown) => scheduler.waitEvent(g, eventArg(event, "game.waitEvent"), "game.waitEvent") as unknown as Script<never>,
        channel: <R extends Messages, E extends Messages>(decl: ChannelDeclaration<R, E>) => serverChannel(g, mod, decl) as unknown as ServerChannel<R, E>,
        persist: <T extends object>(owner: Handle, defaults: T): T => {
            live(g, "game.persist");
            persistOwner(owner, "game.persist");
            argTable(defaults, 2, "game.persist");
            return persist.persist(g, mod, owner, defaults) as T;
        },
        hasPersisted: (owner: Handle) => {
            persistOwner(owner, "game.hasPersisted");
            return persist.hasPersisted(g, mod, owner);
        },
    };
    worldClasses.gameMembers(getters, values);
    const game = readonly<Game>(values, "game", getters);
    g.objects.set(mod, game);
    return game;
}

export function makeClient(g: GameState, mod: number): Client {
    const order = mods.order(mod);
    const getters = new LuaTable<string, (this: void) => unknown>();
    getters.set("tick", () => native.tick());
    return readonly<Client>({
        gameId: g.id,
        on: (event: unknown, handler: unknown) => {
            live(g, "client.on");
            events.subscribe(g, mod, order, eventArg(event, "client.on"), argOptionalFunction(handler, 2, "client.on"));
        },
        startScript: (body: unknown, owner?: unknown) => startScript(g, mod, body, owner, "client.startScript"),
        waitEvent: (event: unknown) => scheduler.waitEvent(g, eventArg(event, "client.waitEvent"), "client.waitEvent") as unknown as Script<never>,
        channel: <R extends Messages, E extends Messages>(decl: ChannelDeclaration<R, E>) => clientChannel(g, mod, decl) as unknown as ClientChannel<R, E>,
    }, "client", getters);
}
