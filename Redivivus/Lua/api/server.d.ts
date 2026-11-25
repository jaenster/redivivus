/**
 * The server control stage: game logic. Entry file server/control.lua.
 * @side server
 */
/// <reference path="./control.d.ts" />

/**
 * The events the server raises, keyed by Event. A mod subscribes with script.on_event.
 * @events
 */
interface ServerEvents {
    /** Every game tick, before the scripts that are due resume. */
    [Event.Tick]: EventBase;
}

/** The ids of the example registry. A mod adds its own by declaring members of this enum. */
declare const enum ExampleId {
    /** "hello, <name>". */
    Greet = "base:greet",
}

/**
 * The example registry: shows keyed entries, a native implementation at the bottom of an
 * entry's chain, and ids added by mods through declaration merging (of ExampleId and of this interface).
 * @registry
 */
interface ExampleRegistry {
    /**
     * Greets: "hello, <name>".
     * @native D2LuaExample_Greet
     */
    [ExampleId.Greet]: (this: void, name: string) => string;
}

/**
 * The example single-slot registry: doubles a number.
 * @native D2LuaExample_Slot
 */
type ExampleSlot = (this: void, value: number) => number;

/** The behaviour registries of the server side. */
interface ServerRegistries {
    readonly example: KeyedRegistry<ExampleRegistry>;
    readonly example_slot: SlotRegistry<ExampleSlot>;
}

/** @noSelf */
interface LuaServerScript {
    /**
     * The name of the mod this script object belongs to.
     * @get D2LuaScript_ModName
     */
    readonly mod_name: string;
    /**
     * The current tick.
     * @get D2LuaScript_Tick
     */
    readonly tick: integer;
    /**
     * Subscribe to an event; undefined unsubscribes. One handler per event per mod.
     * @native D2LuaScript_OnEvent
     */
    on_event<K extends keyof ServerEvents>(event: K,
        handler: ((this: void, e: ServerEvents[K]) => void) | undefined): void;
    /**
     * Run once when a new game starts, after every mod's control file. storage is fresh.
     * @native D2LuaScript_OnInit
     */
    on_init(handler: ((this: void) => void) | undefined): void;
    /**
     * Run when a game is loaded or the control stage reloads, after storage is restored.
     * @native D2LuaScript_OnLoad
     */
    on_load(handler: ((this: void) => void) | undefined): void;
    /**
     * Start a script: the host runs body in a coroutine of its own from the next tick on,
     * resuming it each time what it waits for has come. A script ends when body returns or
     * raises, when it is cancelled, or when the object of its owner handle stops existing.
     * Scripts do not survive a reload. Every tick resumes the scripts that are due in the order
     * they were started. The result is the script's id.
     * @raw D2LuaScript_Start
     */
    start(body: (this: void) => Script<void>, owner?: Handle): integer;
    /**
     * Stop a script this mod started. It is not resumed again (a script that cancels itself
     * stops at its next wait). True when it was still running.
     * @native D2LuaScript_Cancel
     */
    cancel(id: integer): boolean;
    /**
     * Whether a script this mod started is still running.
     * @native D2LuaScript_Running
     */
    running(id: integer): boolean;
    /**
     * Which side this state is.
     * @get D2LuaScript_Side
     */
    readonly side: Side;
    /**
     * Send bytes to this mod's other half: to the client with this peer id, or to every client (0, the default). At most 4096 bytes; the kit's channels build typed calls on this.
     * @raw D2LuaScript_Send
     */
    send(payload: string, peer?: integer): void;
    /**
     * Receive what this mod's other half sends: the payload and the peer it came from (the client's id). Each delivery is a guarded entry with its own time budget; nothing in the payload is trusted. undefined stops receiving.
     * @native D2LuaScript_OnMessage
     */
    on_message(handler: ((this: void, payload: string, peer: integer) => void) | undefined): void;
    /**
     * Wait ticks (default 1) inside a script. The outcome is the tick it resumes on.
     * @raw D2LuaScript_Wait
     */
    wait(ticks?: integer): Script<integer>;
    /**
     * Wait for an event inside a script. The outcome is the event's payload.
     * @raw D2LuaScript_WaitEvent
     */
    wait_event<K extends keyof ServerEvents>(event: K): Script<ServerEvents[K]>;
}

/** This mod's handle on the control stage: events, lifecycle, scripts. */
declare const script: LuaServerScript;

/**
 * The mod's persistent state: plain data and handles, saved with the game (see the storage
 * rules in the reference).
 * @raw D2LuaStorage_Push
 */
declare let storage: { [key: string]: any };

/**
 * The behaviour registries.
 * @raw D2LuaRegistries_Push
 */
declare const registries: ServerRegistries;
