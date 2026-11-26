/**
 * The drawing backend and the installed mods, as the core sees them (Redivivus/Lua/d2_lua_ui.cpp,
 * tables "ui" and "catalog" of the natives). "ui" is present in a client state that has a renderer;
 * "catalog" in every state (empty when the host gave the state no catalogue). Mods never see either
 * table: redivivus:ui's engine backend, the main menu, redivivus:mods and redivivus:manager are
 * built on them in the core. Drawing outside a frame the host opened is ignored.
 * Hand-written: these natives are registered by hand, not by the generator.
 */

/** @noSelf */
export interface NativeUi {
    loadSprite(path: string): number;
    spriteFrames(sprite: number): number;
    spriteWidth(sprite: number, frame: number): number;
    spriteHeight(sprite: number, frame: number): number;
    loadFont(name: string): number;
    fontLineHeight(font: number): number;
    fontAscent(font: number): number;
    hasGlyph(font: number, code: number): boolean;
    measureText(font: number, text: string): number;
    beginFrame(width: number, height: number): void;
    endFrame(): void;
    drawSprite(sprite: number, frame: number, x: number, y: number, mode: number): void;
    drawNineSlice(sprite: number, frames: number[], x: number, y: number, w: number, h: number, mode: number): void;
    fillRect(x: number, y: number, w: number, h: number, rgb: number, alpha: number): void;
    drawLine(x0: number, y0: number, x1: number, y1: number, rgb: number, alpha: number): void;
    drawText(font: number, text: string, x: number, y: number, colour: number): void;
    pushScissor(x: number, y: number, w: number, h: number): void;
    popScissor(): void;
}

/** One dependency of an installed mod, as the catalogue has it. */
export interface NativeCatalogDependency {
    spec: string;
    name: string;
    state: string;
    installed: string;
}

/** One installed mod, as the catalogue has it. */
export interface NativeCatalogMod {
    name: string;
    title: string;
    version: string;
    author: string;
    licence: string;
    enabled: boolean;
    state: string;
    error: string;
    dependencies: NativeCatalogDependency[];
}

/** One declared setting, as the settings stage settled it. */
export interface NativeCatalogSetting {
    mod: string;
    name: string;
    title: string;
    type: string;
    scope: string;
    value: boolean | number | string;
}

/** @noSelf */
export interface NativeCatalog {
    mods(): NativeCatalogMod[];
    settings(): NativeCatalogSetting[];
    readme(mod: string): string;
    changelog(mod: string): string;
    /** Set mods on or off in the mod list; true, or false and why. */
    writeModList(changes: LuaTable<string, boolean>): LuaMultiReturn<[boolean, string?]>;
    /** Restart the game; true, or false and why. */
    restart(): LuaMultiReturn<[boolean, string?]>;
}
