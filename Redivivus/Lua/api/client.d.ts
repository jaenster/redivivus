/**
 * The client control stage: UI, drawing, input. Entry file client/control.lua.
 * @side client
 */
/// <reference path="./control.d.ts" />

/**
 * The events the client raises, keyed by Event.
 * @events
 */
interface ClientEvents {
    /** Every client frame tick. */
    [Event.Tick]: EventBase;
    /** When the UI is drawn. */
    [Event.Draw]: EventBase;
}

/** @noSelf */
interface LuaClientScript {
    /**
     * The name of the mod this script object belongs to.
     * @get D2LuaScript_ModName
     */
    readonly mod_name: string;
    /**
     * Subscribe to an event; undefined unsubscribes.
     * @native D2LuaScript_OnEvent
     */
    on_event<K extends keyof ClientEvents>(event: K,
        handler: ((this: void, e: ClientEvents[K]) => void) | undefined): void;
    /**
     * Run once when the client stage starts.
     * @native D2LuaScript_OnInit
     */
    on_init(handler: ((this: void) => void) | undefined): void;
    /**
     * Run when the client stage reloads.
     * @native D2LuaScript_OnLoad
     */
    on_load(handler: ((this: void) => void) | undefined): void;
    /**
     * Which side this state is.
     * @get D2LuaScript_Side
     */
    readonly side: Side;
    /**
     * Send bytes to this mod's other half: to the server. At most 4096 bytes; the kit's channels build typed calls on this.
     * @raw D2LuaScript_Send
     */
    send(payload: string): void;
    /**
     * Receive what this mod's other half sends: the payload and the peer it came from (0, the server). Each delivery is a guarded entry with its own time budget; nothing in the payload is trusted. undefined stops receiving.
     * @native D2LuaScript_OnMessage
     */
    on_message(handler: ((this: void, payload: string, peer: integer) => void) | undefined): void;
}

/** This mod's handle on the control stage: events, lifecycle, scripts. */
declare const script: LuaClientScript;
