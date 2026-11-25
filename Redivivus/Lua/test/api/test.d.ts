/**
 * A native class the host test registers (the game does not): a stand-in world of units that
 * exercises the object model engine classes such as Unit will use. Handles, revalidated on every
 * access; read-only and read-write properties; methods; an optional return; an enum; an event
 * carrying a handle; a native registry entry taking a handle.
 * @side server
 * @include "Redivivus/Lua/test/d2_lua_test_world.h"
 */
/// <reference path="../../api/server.d.ts" />

/**
 * What a test unit is.
 * @native TestUnitKind
 */
declare const enum TestUnitKind {
    Monster = 0,
    Npc = 1,
    Player = 2,
}

/** The test world's event. */
declare const enum Event {
    TestUnitDamaged = "on_test_unit_damaged",
}

/**
 * A unit of the test world. A handle: it names the unit by id and looks it up on every use.
 * @handle TestUnitTraits
 */
declare class TestUnit {
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
     * on_test_unit_damaged.
     * @native TestUnit_Damage
     */
    damage(amount: number): boolean;
    /**
     * The nearest other living unit, optionally of one kind.
     * @native TestUnit_FindNearest
     */
    find_nearest(kind?: TestUnitKind): TestUnit | undefined;
}

/** @noSelf */
interface LuaTestWorld {
    /**
     * The unit with this id, if it exists.
     * @native TestWorld_GetUnit
     */
    get_unit(id: integer): TestUnit | undefined;
    /**
     * Make a unit.
     * @native TestWorld_Spawn
     */
    spawn(name: string, kind: TestUnitKind, health: number): TestUnit;
}

/** The test world. */
declare const test_world: LuaTestWorld;

/** Payload of on_test_unit_damaged. */
interface OnTestUnitDamaged extends EventBase {
    /** The unit hit; not valid when the hit killed it. */
    readonly unit: TestUnit;
    readonly amount: number;
    readonly killed: boolean;
}

/** @events */
interface ServerEvents {
    [Event.TestUnitDamaged]: OnTestUnitDamaged;
}

declare const enum ExampleId {
    DamageFormula = "test:damage_formula",
}

/** @registry */
interface ExampleRegistry {
    /**
     * Damage after armour: the native entry halves it.
     * @native TestRegistry_DamageFormula
     */
    [ExampleId.DamageFormula]: (this: void, unit: TestUnit, amount: number) => number;
}
