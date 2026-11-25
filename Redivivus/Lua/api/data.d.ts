/**
 * The data stage: settings.lua, data.lua, data-updates.lua, data-final-fixes.lua.
 * @side data
 */
/// <reference path="./shared.d.ts" />

/** A prototype: a row of a data table. */
interface Prototype {
    readonly type: string;
    readonly name: string;
    [field: string]: unknown;
}

/** @noSelf */
interface LuaData {
    /** Every prototype by type and name. */
    readonly raw: { [type: string]: { [name: string]: Prototype } };
    /** Add prototypes. */
    extend(prototypes: Prototype[]): void;
}

/**
 * The data stage's shared table. Every mod sees the same one.
 * @raw D2LuaData_Push
 */
declare const data: LuaData;
