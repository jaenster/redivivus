// Generated from Redivivus/Lua/core/redivivus/client.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
/**
 * The client: what a mod's `client` export receives, once for every game this client is in.
 * The module holds types and the enums the client object uses; everything a mod registers goes
 * through the Client it is handed and lasts as long as that game.
 * @side data server client
 */
import { Event } from "./enums";
import type { Child } from "../ui/jsx";
import type { ChannelDeclaration, EventBase, Handle, MessageArgs, Messages, Script, ScriptTask } from "./shared";
export { Event } from "./enums";
/**
 * The events the client raises, keyed by Event.
 * @events client
 */
export interface ClientEvents {
    /** Every client frame tick. */
    [Event.Tick]: EventBase;
    /** When the UI is drawn. */
    [Event.Draw]: EventBase;
}
/** A channel as a client has it, for one game. */
export interface ClientChannel<R extends Messages, E extends Messages> {
    readonly name: string;
    /** Send a request to the server. */
    readonly call: {
        readonly [K in keyof R]: (this: void, ...args: MessageArgs<R[K]>) => void;
    };
    /** Handle an event the server sends. */
    readonly on: {
        readonly [K in keyof E]: (this: void, handler: (this: void, ...args: MessageArgs<E[K]>) => void) => void;
    };
}
/** One game this client is in, as one mod sees it. Everything registered through it ends with the game. */
export interface Client {
    /** The game's id on this client. */
    readonly gameId: number;
    /** The current client tick. */
    readonly tick: number;
    /** Subscribe to an event; undefined unsubscribes. One handler per event per mod. */
    on<K extends keyof ClientEvents>(this: void, event: K, handler: ((this: void, e: ClientEvents[K]) => void) | undefined): void;
    /** Start a script for this game, resumed each client tick it is due. */
    startScript(this: void, body: (this: void) => Script<void>, owner?: Handle): ScriptTask;
    /** Wait for an event of this game inside one of its scripts. The outcome is the payload. */
    waitEvent<K extends keyof ClientEvents>(this: void, event: K): Script<ClientEvents[K]>;
    /** Open a channel of this mod for this game. */
    channel<R extends Messages, E extends Messages>(this: void, declaration: ChannelDeclaration<R, E>): ClientChannel<R, E>;
}
/** The type of a mod's `client` export. */
export type ClientEntry = (this: void, client: Client) => void;
/**
 * A full screen of the main menu, made with menu.addScreen. While it is open it is the whole
 * menu: it draws every frame through redivivus:ui and gets the mouse and the keyboard; closing it
 * returns to the main menu. Escape closes it when nothing in it took the key.
 */
export interface MenuScreen {
    /** Open the screen (from a main menu button, or from another screen of the same mod). */
    open(this: void): void;
    /** Close it and return to the main menu. */
    close(this: void): void;
    isOpen(this: void): boolean;
}
/**
 * The main menu, as one mod sees it: what a mod's `menu` export receives, once, in the client
 * state, when the main menu first comes up. What a mod adds stays for the whole session.
 */
export interface Menu {
    /**
     * Add a button to the main menu, below the stock ones, labelled in the game's button font.
     * The menu has room for two; a mod's button that does not fit is left out and logged.
     */
    addButton(this: void, label: string, onClick: (this: void) => void): void;
    /**
     * A full screen. `view` builds it (a redivivus:ui component) each time it opens, and gets the
     * screen so it can close itself.
     */
    addScreen(this: void, view: (this: void, screen: MenuScreen) => Child): MenuScreen;
}
/** The type of a mod's `menu` export. */
export type MenuEntry = (this: void, menu: Menu) => void;
