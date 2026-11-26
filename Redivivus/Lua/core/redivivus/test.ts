/**
 * The test runner's API. Only a test the test runner starts can import it; it does not resolve for
 * a mod. `world` is the runner's stand-in world (the host test has one; a game has none), `probe`
 * reaches into the core to prove what the native layer promises about it. The test runner runs
 * tests in a server state.
 * @side server
 */

import { argInteger } from "../internal/check";
import { native, testWorld } from "../internal/native";
import { readonly } from "../internal/proxy";
import type { TestUnit, TestUnitKind, TestWorld } from "../native/testworld";

export type { TestUnit, TestUnitDamaged, TestWorld } from "../native/testworld";
export { TestUnitKind } from "../native/testworld";

/** The stand-in world, when the runner has one. */
const w = testWorld;
export const world: TestWorld | undefined = w === undefined ? undefined : readonly<TestWorld>({
    getUnit: (id: number): TestUnit | undefined => w.getUnit(id),
    spawn: (name: string, kind: TestUnitKind, health: number): TestUnit => w.spawn(name, kind, health),
}, "world");

// Core state the probes update in two steps, with work between; consistent when both match.
import { state } from "../internal/probe";

/** Probes into the core, for the runner's tests of the native layer's promises. */
export const probe = readonly<{
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
}>({
    criticalSection: (ms: number): number => {
        argInteger(ms, 1, "probe.criticalSection");
        state.first++;
        const until = native.clock() + ms;
        let spins = 0;
        while (native.clock() < until) spins++;
        state.second++;
        return spins;
    },
    consistent: () => state.first === state.second,
    sections: () => state.second,
    allocate: (kib: number): number => {
        argInteger(kib, 1, "probe.allocate");
        state.first++;
        const keep: string[] = [];
        for (let i = 0; i < kib; i++) keep.push(string.rep("x", 1000) + tostring(i));
        state.second++;
        return keep.length;
    },
}, "probe");
