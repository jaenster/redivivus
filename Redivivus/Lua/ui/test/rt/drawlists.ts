// The screens as draw lists on the headless backend, in every state: computed the same way in
// Node and under LuaJIT, and compared line for line by test/host/lua.test.ts.

import { HeadlessMetrics, mount, Recorder, stockTheme, testTheme, type Child, type Theme, type UIRoot } from "redivivus:ui";
import { sheetStates, sheets } from "../../screens/gallery/Gallery";
import { createScreen } from "../../screens/mods/ModsScreen";

export interface DrawList {
    name: string;
    text: string;
}

function record(view: () => Child, theme: Theme, width: number, height: number, state: (ui: UIRoot) => void): string {
    const rec = new Recorder(new HeadlessMetrics());
    const ui = mount(view, rec, theme, width, height);
    state(ui);
    ui.frame();
    ui.dispose();
    return rec.text();
}

const SIZES = [
    [800, 600],
    [640, 480],
    [1024, 768],
];

export function drawLists(): DrawList[] {
    const out: DrawList[] = [];
    const themes: [string, () => Theme][] = [
        ["test", testTheme],
        ["stock", stockTheme],
    ];
    for (let t = 0; t < themes.length; t++) {
        const [themeName, makeTheme] = themes[t];
        const states = ["default", "pending", "changelog", "dependencies", "broken", "search", "updates", "browse", "settings", "scrolled", "hover"];
        for (let s = 0; s < states.length; s++) {
            const screen = createScreen();
            out.push({ name: `mods/${themeName}/${states[s]}`, text: record(screen.view, makeTheme(), 800, 600, screen.states[states[s]]) });
        }
        for (let z = 1; z < SIZES.length; z++) {
            const screen = createScreen();
            out.push({ name: `mods/${themeName}/default@${SIZES[z][0]}x${SIZES[z][1]}`, text: record(screen.view, makeTheme(), SIZES[z][0], SIZES[z][1], screen.states.default) });
        }
        const sheetNames = ["controls", "layout", "list"];
        const stateNames = ["default", "tooltip", "focus", "scrolled"];
        for (let i = 0; i < sheetNames.length; i++)
            for (let s = 0; s < stateNames.length; s++)
                out.push({ name: `gallery/${themeName}/${sheetNames[i]}/${stateNames[s]}`, text: record(sheets[sheetNames[i]], makeTheme(), 800, 600, sheetStates[stateNames[s]]) });
    }
    return out;
}
