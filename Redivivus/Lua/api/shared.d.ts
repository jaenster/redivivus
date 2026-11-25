/**
 * What every side has.
 * @side data server client
 */
/// <reference path="./lua/index.d.ts" />
/// <reference path="./core.d.ts" />
/// <reference path="./enums.d.ts" />

/**
 * Every event, server and client. The value is the event's name on the wire and in logs; which
 * events a side raises is its events map (ServerEvents, ClientEvents), and subscribing to one the
 * side does not raise is a type error in TypeScript and an error at run time in Lua.
 */
declare const enum Event {
    /** A game tick (server) or a client frame tick. */
    Tick = "on_tick",
    /** The client draws the UI. */
    Draw = "on_draw",
}

/** The sides a mod runs on, each its own Lua state. */
declare const enum Side {
    Data = 1,
    Server = 2,
    Client = 4,
}

/** How an action or a wait ended. */
declare const enum Outcome {
    /** It did what it set out to. */
    Done = 0,
    /** Something intervened (the target died, the unit was stunned). */
    Interrupted = 1,
    /** It could not start or go on (no path). */
    Blocked = 2,
    /** Its time ran out first. */
    Timeout = 3,
}

/** The value type of a mod setting. */
declare const enum SettingType {
    Bool = "bool",
    Int = "int",
    Number = "number",
    String = "string",
    Choice = "choice",
}

/** Who reads a mod setting, and when it can change. */
declare const enum SettingScope {
    /** Every side, the data stage included; fixed until the game restarts. */
    Startup = "startup",
    /** The server control stage; fixed for a game. */
    Game = "game",
    /** The client control stage: the local player's own value. */
    Player = "player",
}

/** A setting as mod_settings.declare takes it. The kit's settings build these. */
interface SettingDeclaration {
    readonly type: SettingType;
    readonly scope: SettingScope;
    readonly default: boolean | number | string;
    readonly min?: number;
    readonly max?: number;
    readonly choices?: readonly string[];
    readonly title?: string;
    readonly description?: string;
}

/** A declared setting with the value in effect, as mod_settings.list gives it. */
interface SettingInfo extends SettingDeclaration {
    readonly mod: string;
    readonly name: string;
    readonly value: boolean | number | string;
}

/**
 * Mod settings, the plain layer: the kit's settings (`__base__/shared/kit/settings`) give the typed
 * form. Values come from the settings file under the mods root, checked against the declaration.
 * @noSelf
 */
interface LuaModSettings {
    /**
     * Declare a setting of this mod, once per state.
     * @raw D2LuaSettings_Declare
     */
    declare(name: string, declaration: SettingDeclaration): void;
    /**
     * This mod's value of a setting it declared. A setting is read only on its scope's side.
     * @raw D2LuaSettings_Get
     */
    get(name: string): boolean | number | string;
    /**
     * Every setting declared in this state, all mods, in declaration order.
     * @raw D2LuaSettings_List
     */
    list(): SettingInfo[];
}

/** Settings of every mod. */
declare const mod_settings: LuaModSettings;

/**
 * Write a line to the game's log, prefixed with the mod's name. Values are converted with
 * tostring and separated by spaces.
 * @raw D2LuaNative_Log
 */
declare function log(this: void, ...values: unknown[]): void;
