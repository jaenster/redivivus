/**
 * What both control stages (server and client) have.
 * @side server client
 */
/// <reference path="./shared.d.ts" />

/**
 * Cross-mod calls bound late. Interfaces are typed by declaration merging into RemoteInterfaces:
 * `declare global { interface RemoteInterfaces { "my-mod": { greet(this: void, who: string): string } } }`.
 */
interface RemoteInterfaces {}

/** @noSelf */
interface LuaRemote {
    /**
     * Publish an interface: a table of functions other mods call through remote.call.
     * @native D2LuaRemote_AddInterface
     */
    add_interface(name: string, functions: { [name: string]: (this: void, ...args: any[]) => any }): void;
    /**
     * Remove an interface. True when it existed.
     * @native D2LuaRemote_RemoveInterface
     */
    remove_interface(name: string): boolean;
    /**
     * Call a function of an interface. It runs as the mod that added the interface; its errors
     * reach the caller.
     * @raw D2LuaRemote_Call
     */
    call<I extends keyof RemoteInterfaces & string, F extends keyof RemoteInterfaces[I] & string>(
        iface: I, fn: F, ...args: RegistryArgs<RemoteInterfaces[I][F]>): RegistryResult<RemoteInterfaces[I][F]>;
    /**
     * Every interface and its function names: `{ [iface]: { [fn]: true } }`.
     * @get @raw D2LuaRemote_Interfaces
     */
    readonly interfaces: LuaTable<string, LuaTable<string, true>>;
}

/** Late-bound calls between mods. */
declare const remote: LuaRemote;
