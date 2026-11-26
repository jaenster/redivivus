// Asset sources for the raster: the game's own files read from the user's install at run time
// (nothing is copied or cached on disk), or built-in test art.

import * as fs from "node:fs";
import { decodeDc6, type Frame, type Sprite } from "./dc6";
import { MpqSet } from "./mpq";
import { palettePl2, paletteTest, type Palette } from "./palette";
import type { AssetSource } from "./raster";
import { buildTestFont, type BuiltFont } from "./testfont";

/** The 1.14d install D2_INSTALL names, when it holds the game's archives. */
export function installDir(): string | undefined {
    const dir = process.env.D2_INSTALL;
    if (dir === undefined || dir.length === 0) return undefined;
    try {
        const names = fs.readdirSync(dir).map((n) => n.toLowerCase());
        return names.includes("d2data.mpq") ? dir : undefined;
    } catch {
        return undefined;
    }
}

/** Parses a font .tbl (D2Win/Src/D2WinFont.h:38-51) against its DC6. */
export function fontFromTbl(tbl: Uint8Array, sprite: Sprite): BuiltFont {
    const v = new DataView(tbl.buffer, tbl.byteOffset, tbl.byteLength);
    if (v.getUint32(0, true) !== 0x216f6f57) throw new Error("tbl: bad signature");
    const count = v.getUint16(8, true);
    const lh = tbl[10];
    const advance: number[] = new Array(256).fill(0);
    const frameOf: number[] = new Array(256).fill(-1);
    const has: boolean[] = new Array(256).fill(false);
    let cell = 0;
    // Latin fonts index the glyph array by character code (D2WinFont.cpp:163).
    for (let c = 0; c < 256 && c < count; c++) {
        const o = 12 + c * 14;
        advance[c] = tbl[o + 3];
        const fi = v.getUint16(o + 8, true);
        frameOf[c] = fi < sprite.frames.length ? fi : -1;
        has[c] = c >= 32 && advance[c] > 0;
        const fr = frameOf[c] >= 0 ? sprite.frames[frameOf[c]] : undefined;
        if (fr !== undefined && fr.height > cell) cell = fr.height;
    }
    // A line break moves by lineHeight * 16 / 10 (D2WinFont.cpp:453).
    return { sprite, advance, frameOf, lineHeight: Math.trunc((lh * 16) / 10), ascent: cell, has };
}

/**
 * The palette the game's front-end screens draw under: character select's, gaFrontEndPaletteFiles[2]
 * and [3] (D2Launch/Src/CharSel.cpp:2298, the table in globals/D2Launch.cpp), which the Mods screen
 * sets too (Redivivus/Lua/engine/d2_lua_menu.cpp, OpenScreen). The front-end art is made for it:
 * under act1 the button gems turn red and FrontEnd\clickbox (character creation's, made for fechar)
 * is noise under either.
 */
export const FRONT_END_PALETTE = "sky";

export class StockAssets implements AssetSource {
    readonly palette: Palette;
    readonly clearIndex = 0;
    private mpq: MpqSet;
    private cache = new Map<string, Sprite | undefined>();
    constructor(dir: string, palette: string = FRONT_END_PALETTE) {
        this.mpq = new MpqSet(dir);
        const pl2 = this.mpq.get(`data\\global\\palette\\${palette}\\pal.pl2`);
        if (pl2 === undefined) throw new Error(`palette ${palette}: pal.pl2 not found`);
        this.palette = palettePl2(pl2);
    }
    sprite(path: string): Sprite | undefined {
        const key = path.toLowerCase();
        if (this.cache.has(key)) return this.cache.get(key);
        const data = this.mpq.get(path + ".dc6");
        const s = data === undefined ? undefined : decodeDc6(data);
        this.cache.set(key, s);
        return s;
    }
    font(name: string): BuiltFont | undefined {
        const base = `data\\local\\font\\latin\\${name}`;
        const dc6 = this.mpq.get(base + ".dc6");
        const tbl = this.mpq.get(base + ".tbl");
        if (dc6 === undefined || tbl === undefined) return undefined;
        return fontFromTbl(tbl, decodeDc6(dc6));
    }
    close(): void {
        this.mpq.close();
    }
}

function builtin(w: number, h: number, paint: (x: number, y: number) => number): Sprite {
    const pixels = new Uint8Array(w * h);
    const mask = new Uint8Array(w * h);
    for (let y = 0; y < h; y++)
        for (let x = 0; x < w; x++) {
            const c = paint(x, y);
            if (c >= 0) {
                pixels[y * w + x] = c;
                mask[y * w + x] = 1;
            }
        }
    const frame: Frame = { width: w, height: h, offsetX: 0, offsetY: 0, pixels, mask };
    return { directions: 1, frames: [frame] };
}

export class TestAssets implements AssetSource {
    readonly palette = paletteTest();
    readonly clearIndex: number;
    private white: number;
    constructor() {
        this.clearIndex = this.palette.nearest(0x000000);
        this.white = this.palette.nearest(0xffffff);
    }
    sprite(path: string): Sprite | undefined {
        const p = this.palette;
        if (path === "test:icon") {
            const a = p.nearest(0x3366cc);
            const b = p.nearest(0x99ccff);
            return builtin(24, 24, (x, y) => (Math.abs(x - 11.5) + Math.abs(y - 11.5) <= 11 ? ((x + y) % 6 < 3 ? a : b) : -1));
        }
        if (path === "test:thumb") {
            const a = p.nearest(0x996633);
            const b = p.nearest(0x333333);
            return builtin(20, 20, (x, y) => (x === 0 || y === 0 || x === 19 || y === 19 ? b : ((x >> 2) + (y >> 2)) % 2 === 0 ? a : b));
        }
        return undefined;
    }
    font(name: string): BuiltFont | undefined {
        return buildTestFont(name, this.white);
    }
}
