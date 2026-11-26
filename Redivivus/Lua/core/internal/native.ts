// The native layer's table (core/native/core.d.ts), handed to the core when the state starts. It
// lives in the core's own environment, which no mod reaches; nothing here is ever given to a mod.
import type { NativeCore } from "../native/core";
import type { TestWorld } from "../native/testworld";
import type { NativeCatalog, NativeUi } from "../native/ui";
import type { NativeWorld } from "../native/world";

declare const __native: {
    readonly core: NativeCore;
    readonly testWorld?: TestWorld;
    readonly world?: NativeWorld;
    readonly ui?: NativeUi;
    readonly catalog: NativeCatalog;
};

export const native = __native.core;
/** The test runner's world, when the state has one (the host test). */
export const testWorld = __native.testWorld;
/** The game world, in a server state that runs a game (d2_lua_world.h). */
export const world = __native.world;
/** The drawing backend, in a client state that has a renderer (d2_lua_ui.h). */
export const ui = __native.ui;
/** The installed mods, as the boot found them (d2_lua_ui.h). */
export const catalog = __native.catalog;
