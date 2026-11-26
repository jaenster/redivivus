// ui-preview: renders a component to a PNG without the game.
//   ui-preview <entry.tsx[#sheet]> --size 800x600 --theme stock|test --out file.png [--state name]
//              [--drawlist file.txt] [--install DIR]
// The entry exports createScreen() -> { view, states }, or view(), or sheets (pick one with #name).
// Entries inside this package are run from the build; others are compiled on the fly and may
// import only "redivivus:ui".

import * as fs from "node:fs";
import * as path from "node:path";
import { pathToFileURL } from "node:url";
import ts from "typescript";
import { mount, Recorder, stockTheme, testTheme, type Child, type UIRoot } from "redivivus:ui";
import { installDir, StockAssets, TestAssets } from "./assets";
import { encodePng } from "./png";
import { Raster } from "./raster";

export interface PreviewOptions {
    entry: string;
    width: number;
    height: number;
    theme: "stock" | "test";
    out: string;
    state: string;
    drawlist: string;
    install: string;
}

export interface Entry {
    view: () => Child;
    states: { [name: string]: (ui: UIRoot) => void };
}

// This file runs from out/node/ui/host/.
const UI_ROOT = path.resolve(import.meta.dirname, "../../../..");
const OUT_NODE = path.join(UI_ROOT, "out/node");

async function loadModule(file: string): Promise<{ [k: string]: unknown }> {
    const abs = path.resolve(file);
    const rel = path.relative(UI_ROOT, abs);
    let js: string;
    if (!rel.startsWith("..") && !path.isAbsolute(rel)) {
        js = path.join(OUT_NODE, "ui", rel.replace(/\.tsx?$/, ".js"));
        if (!fs.existsSync(js)) throw new Error(`${rel} is not in the build (run the build first)`);
    } else {
        const src = fs.readFileSync(abs, "utf8");
        const out = ts.transpileModule(src, {
            compilerOptions: { module: ts.ModuleKind.ESNext, target: ts.ScriptTarget.ES2022, jsx: ts.JsxEmit.React, jsxFactory: "h", jsxFragmentFactory: "Fragment" },
            fileName: abs,
        }).outputText;
        const dir = path.join(OUT_NODE, "_entries");
        fs.mkdirSync(dir, { recursive: true });
        js = path.join(dir, path.basename(abs).replace(/\.tsx?$/, ".js"));
        const index = path.relative(dir, path.join(OUT_NODE, "core/ui/index.js")).split(path.sep).join("/");
        fs.writeFileSync(js, out.replace(/(["'])redivivus:ui\1/g, `"./${index}"`.replace("././", "./")));
    }
    return (await import(pathToFileURL(js).href)) as { [k: string]: unknown };
}

export async function loadEntry(spec: string): Promise<Entry> {
    const hash = spec.indexOf("#");
    const file = hash >= 0 ? spec.substring(0, hash) : spec;
    const pick = hash >= 0 ? spec.substring(hash + 1) : "";
    const mod = await loadModule(file);
    const noStates: { [name: string]: (ui: UIRoot) => void } = { default: () => {} };
    if (typeof mod.createScreen === "function") return (mod.createScreen as () => Entry)();
    if (mod.sheets !== undefined) {
        const sheets = mod.sheets as { [name: string]: () => Child };
        const view = sheets[pick];
        if (view === undefined) throw new Error(`${file}: pick a sheet with #name: ${Object.keys(sheets).join(", ")}`);
        return { view, states: (mod.sheetStates as Entry["states"] | undefined) ?? noStates };
    }
    if (typeof mod.view === "function") return { view: mod.view as () => Child, states: noStates };
    throw new Error(`${file}: exports neither createScreen, sheets nor view`);
}

/** Renders one entry; returns the PNG bytes and the draw list. */
export async function render(o: PreviewOptions): Promise<{ png: Buffer; drawlist: string; missing: string[] }> {
    const entry = await loadEntry(o.entry);
    let assets: StockAssets | TestAssets;
    if (o.theme === "stock") {
        const dir = o.install.length > 0 ? o.install : installDir();
        if (dir === undefined) throw new Error("no Diablo II 1.14d install found (set D2_INSTALL or --install)");
        assets = new StockAssets(dir);
    } else assets = new TestAssets();
    const raster = new Raster(assets);
    const rec = new Recorder(raster);
    const ui = mount(entry.view, rec, o.theme === "stock" ? stockTheme() : testTheme(), o.width, o.height);
    const state = entry.states[o.state];
    if (state === undefined) throw new Error(`no state "${o.state}"; states: ${Object.keys(entry.states).join(", ")}`);
    state(ui);
    ui.frame();
    const png = encodePng(raster.image());
    const drawlist = rec.text();
    ui.dispose();
    if (assets instanceof StockAssets) assets.close();
    return { png, drawlist, missing: raster.missing };
}

export function parseArgs(argv: string[]): PreviewOptions {
    const o: PreviewOptions = { entry: "", width: 800, height: 600, theme: "stock", out: "", state: "default", drawlist: "", install: "" };
    for (let i = 0; i < argv.length; i++) {
        const a = argv[i];
        const next = (): string => {
            const v = argv[++i];
            if (v === undefined) throw new Error(`${a} needs a value`);
            return v;
        };
        if (a === "--size") {
            const m = /^(\d+)x(\d+)$/.exec(next());
            if (m === null) throw new Error("--size is WIDTHxHEIGHT");
            o.width = Number(m[1]);
            o.height = Number(m[2]);
        } else if (a === "--theme") {
            const t = next();
            if (t !== "stock" && t !== "test") throw new Error("--theme is stock or test");
            o.theme = t;
        } else if (a === "--out") o.out = next();
        else if (a === "--state") o.state = next();
        else if (a === "--drawlist") o.drawlist = next();
        else if (a === "--install") o.install = next();
        else if (a.startsWith("--")) throw new Error(`unknown option ${a}`);
        else o.entry = a;
    }
    if (o.entry.length === 0 || o.out.length === 0) throw new Error("usage: ui-preview <entry.tsx[#sheet]> --out file.png [--size 800x600] [--theme stock|test] [--state name] [--drawlist file]");
    return o;
}

export async function main(argv: string[]): Promise<number> {
    const o = parseArgs(argv);
    const r = await render(o);
    fs.mkdirSync(path.dirname(path.resolve(o.out)), { recursive: true });
    fs.writeFileSync(o.out, r.png);
    if (o.drawlist.length > 0) fs.writeFileSync(o.drawlist, r.drawlist + "\n");
    for (const m of r.missing) console.error(`missing: ${m}`);
    console.log(`${o.out}: ${o.width}x${o.height}, theme ${o.theme}, state ${o.state}`);
    return 0;
}
