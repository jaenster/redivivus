// Generated from Redivivus/Lua/core/redivivus/test.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
/**
 * The test runner's API. Only a test the test runner starts can import it; it does not resolve for
 * a mod. `world` is the runner's stand-in world (the host test has one; a game has none), `probe`
 * reaches into the core to prove what the native layer promises about it. The test runner runs
 * tests in a server state.
 * @side server
 */
import type { TestWorld } from "../native/testworld";
export type { TestUnit, TestUnitDamaged, TestWorld } from "../native/testworld";
export { TestUnitKind } from "../native/testworld";
export declare const world: TestWorld | undefined;
/** Probes into the core, for the runner's tests of the native layer's promises. */
export declare const probe: {
    /**
     * Update core state in two steps with ms milliseconds of core work between them (a critical
     * section). The watchdog must not cut it: a deadline that passes inside is acted on once
     * control is back in mod code.
     */
    criticalSection(this: void, ms: number): number;
    /** Whether the probe's core state is consistent (both steps of every section done). */
    consistent(this: void): boolean;
    /** Sections completed. */
    sections(this: void): number;
    /** Allocate about kib KiB in core tables in one critical section, then drop it. */
    allocate(this: void, kib: number): number;
};
