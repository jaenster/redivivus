/**
 * The test runner's stand-in world (the host test registers it; a game does not): units that
 * exercise the object model engine classes such as Unit will use. Handles, revalidated on every
 * access; read-only and read-write properties; methods; an optional return; an enum; an event
 * carrying a handle; a native registry entry taking a handle. Mods reach it through
 * `redivivus:test` (`world`), which only the test runner grants.
 * @side server
 * @include "Redivivus/Lua/test/d2_lua_test_world.h"
 */

import type { EventBase, integer } from "../redivivus/shared";

/**
 * What a test unit is.
 * @native TestUnitKind
 */
export declare const enum TestUnitKind {
    Monster = 0,
    Npc = 1,
    Player = 2,
}

/**
 * A unit of the test world. A handle: it names the unit by id and looks it up on every use.
 * @handle TestUnitTraits
 */
export declare class TestUnit {
    private constructor();
    /** False once the unit is gone; every other member then raises an error. */
    readonly valid: boolean;
    /**
     * The unit's id.
     * @get TestUnit_GetId
     */
    readonly id: integer;
    /**
     * The unit's name.
     * @get TestUnit_GetName
     */
    readonly name: string;
    /**
     * What the unit is.
     * @get TestUnit_GetKind
     */
    readonly kind: TestUnitKind;
    /**
     * Hit points. Setting it to 0 or less does not kill the unit; damage does.
     * @get TestUnit_GetHealth
     * @set TestUnit_SetHealth
     */
    health: number;
    /**
     * Deal damage; the unit is removed when its health reaches 0. True when it died. Raises
     * Event.TestUnitDamaged.
     * @native TestUnit_Damage
     */
    damage(amount: number): boolean;
    /**
     * The nearest other living unit, optionally of one kind.
     * @native TestUnit_FindNearest
     */
    findNearest(kind?: TestUnitKind): TestUnit | undefined;
}

/** @noSelf */
export interface TestWorld {
    /**
     * The unit with this id, if it exists.
     * @native TestWorld_GetUnit
     */
    getUnit(id: integer): TestUnit | undefined;
    /**
     * Make a unit.
     * @native TestWorld_Spawn
     */
    spawn(name: string, kind: TestUnitKind, health: number): TestUnit;
}

/** The test world's natives. */
export declare const testWorld: TestWorld;

/** Payload of Event.TestUnitDamaged. */
export interface TestUnitDamaged extends EventBase {
    /** The unit hit; not valid when the hit killed it. */
    readonly unit: TestUnit;
    readonly amount: number;
    readonly killed: boolean;
}

declare module "../redivivus/enums" {
    /** The test world's event and registry id. */
    export const enum Event {
        /** A test unit took damage (the test world only). */
        TestUnitDamaged = "testUnitDamaged",
    }
    export const enum ExampleId {
        DamageFormula = "test:damageFormula",
    }
}

declare module "../redivivus/server" {
    /** @events server */
    export interface ServerEvents {
        [Event.TestUnitDamaged]: TestUnitDamaged;
    }
    /** @registry */
    export interface ExampleRegistry {
        /**
         * Damage after armour: the native entry halves it.
         * @native TestRegistry_DamageFormula
         */
        [ExampleId.DamageFormula]: (this: void, unit: TestUnit, amount: number) => number;
    }
}

import type { Event, ExampleId } from "../redivivus/enums";
