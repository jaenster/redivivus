// Node-only tests: the MPQ reader, DC6, PNG, the raster, the preview renderer, golden images,
// determinism, and the Lua build's draw lists against Node's.

import { spawnSync } from "node:child_process";
import { createHash } from "node:crypto";
import * as fs from "node:fs";
import * as path from "node:path";
import { BlendMode } from "redivivus:ui";
import { FRONT_END_PALETTE, fontFromTbl, installDir, StockAssets, TestAssets } from "../../host/assets";
import { render, parseArgs } from "../../host/cli";
import { decodeDc6 } from "../../host/dc6";
import { explode } from "../../host/explode";
import { MpqSet } from "../../host/mpq";
import { decodePng, encodePng } from "../../host/png";
import { Raster } from "../../host/raster";
import { stockArt, stockFonts } from "../../../core/ui/themes/stock";
import { drawLists } from "../rt/drawlists";
import { eq, ok, test } from "../rt/harness";

// This file runs from out/node/ui/test/host/.
export const UI_ROOT = path.resolve(import.meta.dirname, "../../../../..");
const GOLDEN = path.join(UI_ROOT, "test/golden");
const STOCK_OUT = path.join(UI_ROOT, "out/golden-stock");
const UPDATE = process.env.UPDATE_GOLDEN === "1";
const LUA_OUT = path.join(UI_ROOT, "out/lua");

/** Messages for tests that could not run here (no install, no luajit). */
export const skipped: string[] = [];

test("explode: the reference stream from blast.c", () => {
    const out = explode(new Uint8Array([0x00, 0x04, 0x82, 0x24, 0x25, 0x8f, 0x80, 0x7f]), 13);
    eq(Buffer.from(out).toString("latin1"), "AIAIAIAIAIAIA");
});

function dc6Of(width: number, height: number, rows: number[][]): Uint8Array {
    // rows bottom-up, already RLE-coded.
    const data: number[] = [];
    for (const r of rows) data.push(...r);
    const buf = Buffer.alloc(24 + 4 + 32 + data.length + 3);
    buf.writeUInt32LE(6, 0);
    buf.writeUInt32LE(1, 16);
    buf.writeUInt32LE(1, 20);
    buf.writeUInt32LE(28, 24);
    buf.writeUInt32LE(width, 28 + 4);
    buf.writeUInt32LE(height, 28 + 8);
    buf.writeUInt32LE(data.length, 28 + 28);
    Buffer.from(data).copy(buf, 60);
    return new Uint8Array(buf);
}

test("dc6: runs, skips and end of row, bottom-up", () => {
    // Bottom row: skip 1, two literal pixels 5 6. Top row: literal 7.
    const s = decodeDc6(dc6Of(3, 2, [[0x81, 2, 5, 6, 0x80], [1, 7, 0x80]]));
    const f = s.frames[0];
    eq([f.width, f.height], [3, 2]);
    eq(Array.from(f.pixels), [7, 0, 0, 0, 5, 6]);
    eq(Array.from(f.mask), [1, 0, 0, 0, 1, 1]);
});

test("png: write and read back", () => {
    const img = { width: 3, height: 2, pixels: new Uint8Array([0, 1, 2, 3, 4, 255]), rgb: new Uint8Array(768).map((_, i) => i % 256) };
    const back = decodePng(encodePng(img));
    eq(Array.from(back.pixels), Array.from(img.pixels));
    eq(back.rgb.length, 768);
});

test("raster: scissors nest, alpha rounds to the transparency tables, text advances by the font", () => {
    const a = new TestAssets();
    const r = new Raster(a);
    r.beginFrame(20, 10);
    r.pushScissor(2, 2, 10, 10);
    r.pushScissor(0, 0, 5, 5);
    r.fillRect(0, 0, 20, 10, 0xffffff, 255);
    r.popScissor();
    r.popScissor();
    const white = a.palette.nearest(0xffffff);
    const black = a.clearIndex;
    eq([r.pixels[0], r.pixels[2 * 20 + 2], r.pixels[4 * 20 + 4], r.pixels[5 * 20 + 5]], [black, white, white, black]);
    r.fillRect(10, 0, 5, 5, 0xffffff, 128);
    // The software renderer's levels (DrawSolidRectAlpha): >= 192 solid, >= 128 75%, >= 64 50%, > 0 25%.
    eq(r.pixels[12], a.palette.alpha75[(black << 8) | white], "alpha 128 is the 75% table");
    r.fillRect(10, 5, 5, 5, 0xffffff, 64);
    eq(r.pixels[6 * 20 + 12], a.palette.alpha50[(black << 8) | white], "alpha 64 is the 50% table");
    r.fillRect(15, 0, 5, 10, 0xffffff, 0);
    eq(r.pixels[15], black, "alpha 0 draws nothing");
    r.fillRect(15, 0, 5, 10, 0xffffff, 255);
    eq([r.pixels[18], r.pixels[19], r.pixels[9 * 20 + 16]], [white, black, black], "never the last column or row, as DrawSolidRectEx");
    const f = r.loadFont("test");
    eq(r.measureText(f, "abc"), 18);
    r.endFrame();
});

test("raster: colours are the engine's nearest palette entry; a clipped coloured glyph is dropped", () => {
    const a = new TestAssets();
    const rgb = a.palette.rgb;
    for (const c of [0x123456, 0x7f7f7f, 0xc4a46c, 0x302c24]) {
        let best = 0;
        let bestD = Infinity;
        for (let i = 0; i < 256; i++) {
            const d = (rgb[i * 3] - (c >> 16)) ** 2 + (rgb[i * 3 + 1] - ((c >> 8) & 255)) ** 2 + (rgb[i * 3 + 2] - (c & 255)) ** 2;
            if (d < bestD) {
                bestD = d;
                best = i;
            }
        }
        eq(a.palette.nearest(c), best, `nearest ${c.toString(16)}`);
    }
    const r = new Raster(a);
    const f = r.loadFont("test");
    r.beginFrame(40, 20);
    r.pushScissor(0, 0, 40, 4);
    r.drawText(f, "H", 0, 0, 1);
    r.drawText(f, "H", 20, 0, 0);
    r.popScissor();
    r.endFrame();
    let left = 0;
    let right = 0;
    for (let y = 0; y < 4; y++) for (let x = 0; x < 6; x++) {
        if (r.pixels[y * 40 + x] !== a.clearIndex) left++;
        if (r.pixels[y * 40 + 20 + x] !== a.clearIndex) right++;
    }
    eq(left, 0, "a glyph in colour 1 cut by the scissor is dropped");
    ok(right > 0, "a glyph in colour 0 cut by the scissor is drawn clipped");
});

test("raster: a nine-slice tiles its edges and keeps its corners", () => {
    const a = new TestAssets();
    const r = new Raster(a);
    const s = r.loadSprite("test:thumb");
    r.beginFrame(100, 100);
    r.drawNineSlice(s, [0, 0, 0, 0, 0, 0, 0, 0, 0], 0, 0, 100, 60, BlendMode.Normal);
    r.endFrame();
    ok(r.pixels[50 * 100 + 50] !== a.clearIndex && r.pixels[0] !== a.clearIndex && r.pixels[59 * 100 + 99] !== a.clearIndex, "interior and corners drawn");
    eq(r.pixels[70 * 100 + 50], a.clearIndex, "nothing below the box");
});

test("preview: parses its arguments", () => {
    const o = parseArgs(["screens/mods/ModsScreen.tsx", "--size", "640x480", "--theme", "test", "--out", "x.png", "--state", "pending"]);
    eq([o.width, o.height, o.theme, o.state, o.out], [640, 480, "test", "pending", "x.png"]);
});

// --- golden images ------------------------------------------------------------------------------

interface Shot {
    name: string;
    entry: string;
    state: string;
    size: [number, number];
}

const SHOTS: Shot[] = [
    { name: "mods-800x600", entry: "screens/mods/ModsScreen.tsx", state: "default", size: [800, 600] },
    { name: "mods-pending", entry: "screens/mods/ModsScreen.tsx", state: "pending", size: [800, 600] },
    { name: "mods-dependencies", entry: "screens/mods/ModsScreen.tsx", state: "dependencies", size: [800, 600] },
    { name: "mods-browse", entry: "screens/mods/ModsScreen.tsx", state: "browse", size: [800, 600] },
    { name: "mods-640x480", entry: "screens/mods/ModsScreen.tsx", state: "default", size: [640, 480] },
    { name: "mods-1024x768", entry: "screens/mods/ModsScreen.tsx", state: "default", size: [1024, 768] },
    { name: "gallery-controls", entry: "screens/gallery/Gallery.tsx#controls", state: "tooltip", size: [800, 600] },
    { name: "gallery-layout", entry: "screens/gallery/Gallery.tsx#layout", state: "default", size: [800, 600] },
    { name: "gallery-list", entry: "screens/gallery/Gallery.tsx#list", state: "default", size: [800, 600] },
    // The example mods as the game shows them: long titles, no README, no licence.
    { name: "mods-game-first-mod", entry: "screens/mods/GameMods.tsx", state: "first-mod", size: [800, 600] },
    { name: "mods-game-ui-mod", entry: "screens/mods/GameMods.tsx", state: "ui-mod", size: [800, 600] },
    { name: "mods-game-640x480", entry: "screens/mods/GameMods.tsx", state: "needsgone", size: [640, 480] },
];

async function shot(s: Shot, theme: "stock" | "test"): Promise<Buffer> {
    const [file, sheet] = s.entry.split("#");
    const entry = path.join(UI_ROOT, file) + (sheet === undefined ? "" : "#" + sheet);
    const r = await render({ entry, width: s.size[0], height: s.size[1], theme, out: "", state: s.state, drawlist: "", install: "" });
    return r.png;
}

function pixelHash(png: Uint8Array): string {
    const img = decodePng(png);
    return createHash("sha256").update(img.pixels).update(img.rgb).update(`${img.width}x${img.height}`).digest("hex");
}

export async function goldenTests(): Promise<{ passed: number; failed: number; lines: string[] }> {
    const res = { passed: 0, failed: 0, lines: [] as string[] };
    const pass = (m: string): void => {
        res.passed++;
        res.lines.push(`ok   ${m}`);
    };
    const fail = (m: string): void => {
        res.failed++;
        res.lines.push(`FAIL ${m}`);
    };
    fs.mkdirSync(GOLDEN, { recursive: true });
    // Test theme: the images themselves are the golden files (no game art in them).
    for (const s of SHOTS) {
        const png = await shot(s, "test");
        const again = await shot(s, "test");
        if (!png.equals(again)) fail(`determinism ${s.name}: two renders differ`);
        else pass(`determinism ${s.name}`);
        const file = path.join(GOLDEN, `test-${s.name}.png`);
        if (UPDATE || !fs.existsSync(file)) {
            fs.writeFileSync(file, png);
            pass(`golden test-${s.name} written`);
        } else if (pixelHash(fs.readFileSync(file)) === pixelHash(png)) pass(`golden test-${s.name}`);
        else {
            fs.mkdirSync(path.join(UI_ROOT, "out/golden-diff"), { recursive: true });
            fs.writeFileSync(path.join(UI_ROOT, "out/golden-diff", `test-${s.name}.png`), png);
            fail(`golden test-${s.name}: pixels differ (new render in out/golden-diff/)`);
        }
    }
    // Stock theme: only hashes are committed; the images go to out/golden-stock (gitignored).
    const dir = installDir();
    if (dir === undefined) {
        skipped.push("stock golden images: no 1.14d install (set D2_INSTALL)");
        return res;
    }
    const hashFile = path.join(GOLDEN, "stock-hashes.json");
    const known: { [name: string]: string } = fs.existsSync(hashFile) ? JSON.parse(fs.readFileSync(hashFile, "utf8")) : {};
    const next: { [name: string]: string } = {};
    fs.mkdirSync(STOCK_OUT, { recursive: true });
    for (const s of SHOTS) {
        const png = await shot(s, "stock");
        fs.writeFileSync(path.join(STOCK_OUT, `stock-${s.name}.png`), png);
        const hash = pixelHash(png);
        next[s.name] = hash;
        if (UPDATE || known[s.name] === undefined) pass(`stock hash ${s.name} recorded`);
        else if (known[s.name] === hash) pass(`stock hash ${s.name}`);
        else fail(`stock hash ${s.name}: pixels differ (image in out/golden-stock/)`);
    }
    if (UPDATE || Object.keys(known).length === 0) fs.writeFileSync(hashFile, JSON.stringify(next, null, 2) + "\n");
    return res;
}

// --- the MPQ reader on the real archives ----------------------------------------------------------

test("mpq: reads encrypted and imploded files from the 1.14d archives", () => {
    const dir = installDir();
    if (dir === undefined) {
        skipped.push("mpq: no 1.14d install");
        return;
    }
    const set = new MpqSet(dir);
    try {
        const pl2 = set.get("data\\global\\palette\\act1\\pal.pl2");
        eq(pl2?.length, 443175, "pal.pl2 size");
        const dat = set.get("data\\global\\palette\\act1\\pal.dat") as Uint8Array;
        eq(dat.length, 768);
        // pal.dat is BGR; pal.pl2's first entries are RGBA: they must agree.
        const p = pl2 as Uint8Array;
        let same = 0;
        for (let i = 0; i < 256; i++) if (p[i * 4] === dat[i * 3 + 2] && p[i * 4 + 1] === dat[i * 3 + 1] && p[i * 4 + 2] === dat[i * 3]) same++;
        ok(same >= 250, `palette entries agree with pal.dat (${same}/256)`);
        const tbl = set.get("data\\local\\font\\latin\\font16.tbl") as Uint8Array;
        const font = fontFromTbl(tbl, decodeDc6(set.get("data\\local\\font\\latin\\font16.dc6") as Uint8Array));
        eq([font.advance[65], font.advance[105], font.lineHeight, font.ascent], [12, 4, 16, 16], "font16: A, i, line height, cell");
        const btn = decodeDc6(set.get("data\\global\\ui\\FrontEnd\\WideButtonBlank.dc6") as Uint8Array);
        eq(
            btn.frames.map((f) => `${f.width}x${f.height}`),
            ["256x35", "16x35", "256x35", "16x35"],
        );
    } finally {
        set.close();
    }
});

test("stock assets: every path the stock theme names exists", () => {
    const dir = installDir();
    if (dir === undefined) return;
    const a = new StockAssets(dir);
    const r = new Raster(a);
    for (const role in stockArt) ok(r.loadSprite(stockArt[role]) > 0, `sprite ${stockArt[role]}`);
    for (const role in stockFonts) ok(r.loadFont(stockFonts[role]) > 0, `font ${stockFonts[role]}`);
    a.close();
});

// --- the preview draws what the game draws ---------------------------------------------------------

const LUA_ROOT = path.resolve(UI_ROOT, "..");
const TREE_ROOT = path.resolve(LUA_ROOT, "../..");

test("preview palette: the one the game's screen sets (palette\\sky, not act1)", () => {
    // The screen draws under gaFrontEndPaletteFiles[n] (engine/d2_lua_menu.cpp, OpenScreen); the
    // table names the files (Source/globals/D2Launch.cpp). Under act1 the previews showed red gems
    // the game does not draw, and hid a checkbox that was noise in the game.
    const menu = fs.readFileSync(path.join(LUA_ROOT, "engine/d2_lua_menu.cpp"), "latin1");
    const open = menu.substring(menu.indexOf("void OpenScreen()"));
    const m = /D2GFX_SetPaletteForDraw\([^;]*gaFrontEndPaletteFiles\[(\d)\],[^;]*gaFrontEndPaletteFiles\[(\d)\]\)/.exec(open);
    ok(m !== null, "OpenScreen sets a front-end palette");
    const table = fs.readFileSync(path.join(TREE_ROOT, "Source/globals/D2Launch.cpp"), "latin1");
    const t = /gaFrontEndPaletteFiles\[4\] = \{([^}]*)\}/.exec(table);
    ok(t !== null, "the palette table");
    const names = (t as RegExpExecArray)[1].split(",").map((x) => x.trim());
    const dat = names[Number((m as RegExpExecArray)[1])];
    const pl2 = names[Number((m as RegExpExecArray)[2])];
    eq([dat, pl2], [`s_palette_${FRONT_END_PALETTE}_pal_dat_006d39bc`, `s_palette_${FRONT_END_PALETTE}_pal_pl2_006d39a8`]);
});

test("preview art: the checkbox is a gold box under the screen's palette, empty off and ticked on", () => {
    const dir = installDir();
    if (dir === undefined) {
        skipped.push("preview art: no 1.14d install");
        return;
    }
    const a = new StockAssets(dir);
    try {
        const s = a.sprite(stockArt.checkbox);
        ok(s !== undefined, "checkbox art");
        const rgb = (f: number, x: number, y: number): number[] => {
            const fr = (s as NonNullable<typeof s>).frames[f];
            const i = fr.pixels[y * fr.width + x] * 3;
            return [a.palette.rgb[i], a.palette.rgb[i + 1], a.palette.rgb[i + 2]];
        };
        const gold = (c: number[]): boolean => c[0] > 60 && c[0] > c[2] + 25 && c[1] > c[2];
        const dark = (c: number[]): boolean => c[0] + c[1] + c[2] < 90;
        // The border is gold in both frames; off is empty inside, on is not.
        ok(gold(rgb(0, 0, 8)) && gold(rgb(1, 0, 8)), `border ${rgb(0, 0, 8)} ${rgb(1, 0, 8)}`);
        let off = 0;
        let on = 0;
        for (let y = 4; y < 12; y++)
            for (let x = 4; x < 11; x++) {
                if (dark(rgb(0, x, y))) off++;
                if (dark(rgb(1, x, y))) on++;
            }
        ok(off === 56 && on < 50, `inside off ${off}/56 dark, on ${on}/56 dark`);
    } finally {
        a.close();
    }
});

test("raster: a coloured glyph is drawn when its ink is inside the scissor, though its cell is not", () => {
    const a = new TestAssets();
    const r = new Raster(a);
    const f = r.loadFont("test");
    r.beginFrame(40, 20);
    // "." is drawn at x 10; the clip ends at 13: the 6-wide cell crosses it, the dot (one pixel
    // column, at 2) does not.
    r.pushScissor(0, 0, 13, 20);
    r.drawText(f, ".", 10, 2, 1);
    r.popScissor();
    r.endFrame();
    let lit = 0;
    for (const v of r.pixels) if (v !== a.clearIndex) lit++;
    ok(lit > 0, "the dot is drawn");
});

// --- Lua against Node -----------------------------------------------------------------------------

export function luaDrawListTest(): { passed: number; failed: number; lines: string[] } {
    const res = { passed: 0, failed: 0, lines: [] as string[] };
    const run = spawnSync("luajit", [path.join(UI_ROOT, "test/lua/run.lua"), LUA_OUT, "drawlists"], { encoding: "latin1", maxBuffer: 1 << 30 });
    if (run.error !== undefined) {
        skipped.push(`draw lists under LuaJIT: ${run.error.message}`);
        return res;
    }
    if (run.status !== 0) {
        res.failed++;
        res.lines.push(`FAIL luajit drawlists exited ${run.status}: ${run.stderr.slice(0, 2000)}`);
        return res;
    }
    const lua = new Map<string, string>();
    const parts = run.stdout.split("@@@ ");
    for (const p of parts) {
        const nl = p.indexOf("\n");
        if (nl < 0) continue;
        const name = p.substring(0, nl);
        if (name === "end") continue;
        lua.set(name, p.substring(nl + 1).replace(/\n$/, ""));
    }
    const node = drawLists();
    let lines = 0;
    for (const d of node) {
        const l = lua.get(d.name);
        if (l === undefined) {
            res.failed++;
            res.lines.push(`FAIL drawlist ${d.name}: missing from the Lua run`);
            continue;
        }
        if (l !== d.text) {
            const a = d.text.split("\n");
            const b = l.split("\n");
            let i = 0;
            while (i < a.length && a[i] === b[i]) i++;
            res.failed++;
            res.lines.push(`FAIL drawlist ${d.name}: line ${i + 1}: node "${a[i]}" lua "${b[i]}"`);
        } else {
            res.passed++;
            lines += d.text.split("\n").length;
        }
    }
    res.lines.push(`ok   drawlists: ${res.passed} of ${node.length} identical in Node and LuaJIT (${lines} draw calls)`);
    return res;
}

