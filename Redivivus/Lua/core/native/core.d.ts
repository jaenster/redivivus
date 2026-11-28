/**
 * The native layer as the core sees it: the functions Redivivus/Lua/d2_lua_core.cpp and its
 * neighbours put into the table the core is started with. Mods never see this table, any function
 * in it or any value it hands out that is not plain data; the resolver serves no module that leads
 * here. Hand-written; the generator checks every @native/@raw name against the glue it emits.
 *
 * Everything that enforces the sandbox stays on this side: entering mod code (owner, time budget,
 * memory limit, fault marking), the resolver and its capability rules, handles, storage bytes,
 * the settings file, the message envelope. The core decides what to call and when; the native
 * layer decides what it costs and whose fault it was.
 * @side data server client
 */

/** What an entry into mod code came to (`call`, `resume`). The values are D2LUA_CALL_* in d2_lua_api.h. */
export const enum CallStatus {
    /** It returned (or, for `resume`, finished); its results follow. */
    Ok = 1,
    /** It yielded (`resume` only); the yielded values follow. */
    Yield = 2,
    /** It raised an error; the error value and a traceback string follow. The mod stays enabled. */
    Error = 0,
    /**
     * The time budget or the memory limit ran out inside an entry that is still running (the core
     * was called from mod code). The error value follows; the core must stop what it does and
     * raise it again, so that it reaches the entry's start.
     */
    Fatal = -1,
    /** The entry was the outermost and ran out of time or memory: the mod is faulted (and logged). */
    Faulted = -2,
    /** The mod was faulted earlier; nothing ran. */
    Disabled = -3,
}

/** A mod as the core knows it. */
export interface NativeMod {
    /** The host's id for the mod; what every native taking a mod expects. */
    readonly id: number;
    readonly name: string;
    /** Position in load order (0 is redivivus itself). */
    readonly index: number;
}

/** A registry the declarations define for this side, with its native entries. */
export interface NativeRegistry {
    /** 0 keyed, 1 a single slot. */
    readonly kind: number;
    /** Native implementations at the bottom of entries, by id ("" for a slot). */
    readonly natives: LuaTable<string, (this: void, ...args: any[]) => any>;
}

/** @noSelf */
export interface NativeCore {
    /**
     * Run fn as mod `mod` (an id) with the arguments: the mod is pushed as the owner, so time,
     * memory and faults are charged to it, and the call is a C boundary, so nothing in it can yield
     * past it. Outside any entry the call is an entry of its own with a fresh time budget. Gives
     * the status and then the single result, or the error value and a traceback.
     * A mod id of -1 runs a native implementation without changing the owner.
     * @raw D2LuaCore_Call
     */
    call(mod: number, fn: (this: void, ...args: any[]) => any, ...args: unknown[]): LuaMultiReturn<[CallStatus, any, string?]>;
    /**
     * Resume a script's coroutine as its mod: always an entry of its own, with its own budget.
     * Gives the status and the yielded values (Yield), or the error value (Error).
     * @raw D2LuaCore_Resume
     */
    resume(thread: LuaThread, mod: number, ...args: unknown[]): LuaMultiReturn<[CallStatus, ...any[]]>;
    /**
     * A coroutine for a script of mod `mod`, recorded as that mod's (a fault inside it is charged to
     * the mod, whoever resumes it).
     * @raw D2LuaCore_Spawn
     */
    spawn(mod: number, body: (this: void, ...args: any[]) => any): LuaThread;
    /**
     * Log `[lua] [<mod name>] values...`, each value through tostring (which may run a __tostring of
     * the mod's, as that mod).
     * @raw D2LuaCore_Log
     */
    log(mod: number, ...values: unknown[]): void;
    /**
     * Log an error of mod `mod` with its traceback, mapped back to TypeScript sources.
     * @raw D2LuaCore_LogError
     */
    logError(mod: number, message: string, traceback?: string): void;
    /**
     * Log a line as it is.
     * @raw D2LuaCore_LogLine
     */
    logLine(text: string): void;
    /**
     * Whether the mod has been faulted.
     * @raw D2LuaCore_IsFaulted
     */
    isFaulted(mod: number): boolean;
    /**
     * Every mod of the state in load order, redivivus first.
     * @raw D2LuaCore_Mods
     */
    mods(): NativeMod[];
    /**
     * The mod whose code is running (the owner of the innermost entry), -1 when none.
     * @raw D2LuaCore_Owner
     */
    owner(): number;
    /**
     * The current tick.
     * @raw D2LuaCore_Tick
     */
    tick(): number;
    /**
     * Which side this state is: 1 data, 2 server, 4 client.
     * @raw D2LuaCore_Side
     */
    side(): number;
    /**
     * The events of this side: name -> the events map that declares it.
     * @raw D2LuaCore_Events
     */
    events(): LuaTable<string, string>;
    /**
     * The registries of this side.
     * @raw D2LuaCore_Registries
     */
    registries(): LuaTable<string, NativeRegistry>;
    /**
     * Every enum of this state: name -> member -> value. Fresh tables.
     * @raw D2LuaCore_Enums
     */
    enums(): LuaTable<string, LuaTable<string, number | string>>;
    /**
     * Whether v is a handle (of any class) and its object exists. Reads the userdata itself; runs no
     * metamethod.
     * @raw D2LuaCore_HandleIsLive
     */
    handleIsLive(v: unknown): boolean;
    /**
     * Whether v is a handle at all.
     * @raw D2LuaCore_IsHandle
     */
    isHandle(v: unknown): boolean;
    /**
     * Declare a setting of mod `mod` (checked against the declaration rules, value settled from the
     * settings file).
     * @raw D2LuaCore_SettingsDeclare
     */
    settingsDeclare(mod: number, name: string, declaration: object): void;
    /**
     * A setting's value for mod `mod`; raises when it is not declared or not readable on this side.
     * @raw D2LuaCore_SettingsGet
     */
    settingsGet(mod: number, name: string): boolean | number | string;
    /**
     * Every setting of the state, fresh tables.
     * @raw D2LuaCore_SettingsList
     */
    settingsList(): object[];
    /**
     * Send a message of game `game` as mod `mod` to its other half (peer on the server: the client,
     * 0 all).
     * @raw D2LuaCore_Send
     */
    send(game: number, mod: number, payload: string, peer: number): void;
    /**
     * The key a read-only proxy's metatable holds its values under, so pairs in a mod environment
     * walks the values.
     * @raw D2LuaCore_WalkKey
     */
    walkKey(): unknown;
    /**
     * Resolve a module for mod `mod` exactly as its own require would (same rules, same cache);
     * with optional, a declared dependency that is not installed gives nil instead of an error.
     * @raw D2LuaCore_Require
     */
    require(mod: number, specifier: string, optional: boolean): unknown;
    /**
     * A monotonic clock in milliseconds.
     * @raw D2LuaCore_Clock
     */
    clock(): number;
    /**
     * What an engine path ("D2Common.Skills.GetSkillLevel") names on this state's side: 0 nothing,
     * 1 a namespace, 2 a function, with its id.
     * @raw D2LuaCore_OvLookup
     */
    ovLookup(path: string): LuaMultiReturn<[number, number?]>;
    /**
     * A function of the override registry: its engine path, its kinds (result, then parameters:
     * v n b w p r), why it cannot be overridden (undefined when it can), and the names of the types
     * its pointers point to, in order, separated by "|".
     * @raw D2LuaCore_OvInfo
     */
    ovInfo(id: number): LuaMultiReturn<[string, string, string | undefined, string]>;
    /**
     * Add an override of function `id` for mod `mod` in this state (the newest is outermost). raw:
     * the mod's raw tier. Gives why it was refused, or undefined.
     * @raw D2LuaCore_OvAdd
     */
    ovAdd(mod: number, id: number, fn: (this: void, ...args: any[]) => any, raw: boolean): string | undefined;
    /**
     * Drop every override of mod `mod` in this state (its patch failed).
     * @raw D2LuaCore_OvDropMod
     */
    ovDropMod(mod: number): void;
    /**
     * Whether mod `mod` was granted the raw tier (engine.unsafe).
     * @raw D2LuaCore_OvRawGranted
     */
    ovRawGranted(mod: number): boolean;
    /**
     * Overrides may be added (patch() runs) or not.
     * @raw D2LuaCore_OvPatching
     */
    ovPatching(on: boolean): void;
    /**
     * LuaJIT's ffi module, for the raw tier only (never handed to a mod without the grant).
     * @raw D2LuaCore_OvFfi
     */
    ovFfi(): unknown;
}

/** The functions above. */
export declare const core: NativeCore;
