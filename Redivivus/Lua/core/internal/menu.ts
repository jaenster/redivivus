// The main menu: what the mods' `menu` exports add (buttons, full screens) and the one screen that
// is open. The host (engine/d2_lua_menu.cpp in the game) asks for the buttons when it builds the
// main menu, reports presses, and while a screen is open hands it every frame and every input
// event through the host functions in host.ts.
//
// A screen is drawn with the mod's own instance of redivivus:ui, mounted over the engine backend
// below; everything that runs mod code (the view, the kit's frame and dispatch, the handlers) runs
// as that mod, so time, memory and faults are charged to it. A screen whose mod faults, or whose
// view fails, closes.
import type { Menu, MenuScreen } from "../redivivus/client";
import { UIEventType } from "../redivivus/enums";
import type { Backend } from "../ui/backend";
import type { Child } from "../ui/jsx";
import type { InputEvent } from "../ui/input";
import type { UIRoot } from "../ui/root";
import type { Theme } from "../ui/theme";
import { CallStatus } from "../native/core";
import { argFunction, argString } from "./check";
import { native, ui } from "./native";
import { readonly } from "./proxy";

type Fn = (this: void, ...args: any[]) => any;

type Button = { mod: number; label: string; onClick: Fn };
type Screen = { mod: number; view: Fn; handle: MenuScreen };
type Open = { screen: Screen; root: UIRoot; width: number; height: number };

/** A mod's button label: printable bytes, at most this long. */
const MAX_LABEL = 24;
/** Buttons the main menu has room for. */
export const MAX_BUTTONS = 2;

const buttons: Button[] = [];
let wanted: Screen | undefined;
let current: Open | undefined;
let closing = false;
/** Whether the main menu is showing (no screen open): screens may be opened. */
let menuUp = false;

const KEY_ESCAPE = 27;

/** The engine backend over the natives: the kit's Backend (core/ui/backend.ts). */
let engineBackend: Backend | undefined;

function backend(): Backend | undefined {
    if (engineBackend !== undefined || ui === undefined) return engineBackend;
    const n = ui;
    engineBackend = {
        loadSprite: (path: string) => n.loadSprite(path),
        spriteFrames: (s: number) => n.spriteFrames(s),
        spriteWidth: (s: number, f: number) => n.spriteWidth(s, f),
        spriteHeight: (s: number, f: number) => n.spriteHeight(s, f),
        loadFont: (name: string) => n.loadFont(name),
        fontLineHeight: (f: number) => n.fontLineHeight(f),
        fontAscent: (f: number) => n.fontAscent(f),
        hasGlyph: (f: number, code: number) => n.hasGlyph(f, code),
        measureText: (f: number, text: string) => n.measureText(f, text),
        beginFrame: (w: number, h: number) => n.beginFrame(w, h),
        endFrame: () => n.endFrame(),
        drawSprite: (s: number, frame: number, x: number, y: number, mode: number) => n.drawSprite(s, frame, x, y, mode),
        drawNineSlice: (s: number, frames: number[], x: number, y: number, w: number, h: number, mode: number) =>
            n.drawNineSlice(s, frames, x, y, w, h, mode),
        fillRect: (x: number, y: number, w: number, h: number, rgb: number, alpha: number) => n.fillRect(x, y, w, h, rgb, alpha),
        drawLine: (x0: number, y0: number, x1: number, y1: number, rgb: number, alpha: number) => n.drawLine(x0, y0, x1, y1, rgb, alpha),
        drawText: (f: number, text: string, x: number, y: number, colour: number) => n.drawText(f, text, x, y, colour),
        pushScissor: (x: number, y: number, w: number, h: number) => n.pushScissor(x, y, w, h),
        popScissor: () => n.popScissor(),
    };
    return engineBackend;
}

function printable(label: string): boolean {
    for (let i = 0; i < label.length; i++) {
        const c = string.byte(label, i + 1);
        if (c < 32 || c === 127 || c === 255) return false;
    }
    return true;
}

function isOpen(s: Screen): boolean {
    return current !== undefined && current.screen === s && !closing;
}

/** The Menu a mod's `menu` export gets. */
export function makeMenu(mod: number): Menu {
    return readonly<Menu>({
        addButton: (label: unknown, onClick: unknown) => {
            const text = argString(label, 1, "addButton");
            const fn = argFunction<Fn>(onClick, 2, "addButton");
            if (text.length === 0 || text.length > MAX_LABEL || !printable(text)) {
                error(`bad argument #1 to 'addButton' (a label of 1 to ${MAX_LABEL} printable characters expected)`, 2);
            }
            if (buttons.length >= MAX_BUTTONS) {
                native.log(mod, `the main menu has room for ${MAX_BUTTONS} buttons; '${text}' is left out`);
                return;
            }
            buttons.push({ mod, label: text, onClick: fn });
        },
        addScreen: (view: unknown) => {
            const fn = argFunction<Fn>(view, 1, "addScreen");
            const screen: Screen = { mod, view: fn, handle: undefined as unknown as MenuScreen };
            screen.handle = readonly<MenuScreen>({
                open: () => {
                    if (current !== undefined && current.screen.mod !== mod) error("open: another mod's screen is open", 2);
                    if (current === undefined && !menuUp) error("open: the main menu is not up", 2);
                    if (current !== undefined && current.screen === screen && !closing) return;
                    wanted = screen;
                    if (current !== undefined) closing = true;
                },
                close: () => {
                    if (current !== undefined && current.screen === screen) closing = true;
                    if (wanted === screen) wanted = undefined;
                },
                isOpen: () => isOpen(screen),
            }, "MenuScreen");
            return screen.handle;
        },
    }, "Menu");
}

// --- the host's side --------------------------------------------------------------------------------

/** The main menu is up (true) or a screen has taken it over (false). */
export function setMenuUp(up: boolean): void {
    menuUp = up;
}

/** The labels of the buttons the mods added, in order. */
export function labels(): string[] {
    const out: string[] = [];
    for (const b of buttons) out.push(b.label);
    return out;
}

/** Button i (0-based) was pressed: its handler runs as its mod. */
export function press(i: number): void {
    const b = buttons[i];
    if (b === undefined) return;
    const [status, err, trace] = native.call(b.mod, b.onClick);
    if (status === CallStatus.Error) native.logError(b.mod, err, trace);
}

/** Whether a screen asked to open (and none is open), or the open one asked to close. */
export function state(): number {
    if (current !== undefined) return closing ? 2 : 1;
    return wanted !== undefined ? 3 : 0;
}

function fail(s: Screen, status: CallStatus, err: unknown, trace: string | undefined): void {
    if (status === CallStatus.Error) native.logError(s.mod, err as string, trace);
    closing = true;
}

/** Mount the screen that asked to open, at the screen size. False when it could not be built. */
export function mount(width: number, height: number): boolean {
    const s = wanted;
    wanted = undefined;
    closing = false;
    const b = backend();
    if (s === undefined || b === undefined) return false;
    const kit = native.require(s.mod, "redivivus:ui", false) as {
        mount: (this: void, view: () => Child, backend: Backend, theme: Theme, w: number, h: number) => UIRoot;
        stockTheme: (this: void) => Theme;
    };
    let root: UIRoot | undefined;
    const [status, err, trace] = native.call(s.mod, () => {
        root = kit.mount(() => s.view(s.handle) as Child, b, kit.stockTheme(), width, height);
    });
    if (status !== CallStatus.Ok || root === undefined) {
        if (status === CallStatus.Error) native.logError(s.mod, err, trace);
        return false;
    }
    current = { screen: s, root, width, height };
    return true;
}

/** Draw a frame of the open screen; the host has the frame open. */
export function frame(width: number, height: number): void {
    const c = current;
    if (c === undefined || closing) return;
    const [status, err, trace] = native.call(c.screen.mod, () => {
        if (c.width !== width || c.height !== height) {
            c.width = width;
            c.height = height;
            c.root.resize(width, height);
        }
        c.root.frame();
    });
    if (status !== CallStatus.Ok) fail(c.screen, status, err, trace);
}

/** An input event for the open screen; true when it was handled. Escape closes an unhandled one. */
export function input(ev: InputEvent): boolean {
    const c = current;
    if (c === undefined || closing) return false;
    let handled = false;
    const [status, err, trace] = native.call(c.screen.mod, () => {
        handled = c.root.dispatch(ev);
    });
    if (status !== CallStatus.Ok) {
        fail(c.screen, status, err, trace);
        return true;
    }
    if (!handled && ev.type === UIEventType.KeyDown && ev.key === KEY_ESCAPE) {
        closing = true;
        return true;
    }
    return handled;
}

/** The open screen is gone: dispose of its tree. */
export function dispose(): void {
    const c = current;
    current = undefined;
    closing = false;
    if (c === undefined) return;
    const [status, err, trace] = native.call(c.screen.mod, () => c.root.dispose());
    if (status === CallStatus.Error) native.logError(c.screen.mod, err, trace);
}
