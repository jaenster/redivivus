// The palette and the blend tables the game draws with. The game's own come from
// data\global\palette\act1\pal.pl2 (D2Win/Src/D2WinPalette.cpp:156-206): the live palette is the
// first 256 four-byte entries; the 64 KB blend tables are indexed [dst << 8 | src]; text colour n
// (1..12) remaps a glyph's indices through the 256-byte table at 0x6B627 + n * 256.
// The test palette is generated here, with tables computed the same way, so test-theme images
// contain nothing of the game's.

export interface Palette {
    /** 256 x (r, g, b). */
    rgb: Uint8Array;
    /** Blend tables [dst << 8 | src] for the transparency levels: 25%, 50%, 75% opaque. */
    alpha25: Uint8Array;
    alpha50: Uint8Array;
    alpha75: Uint8Array;
    additive: Uint8Array;
    multiply: Uint8Array;
    /** 13 remap tables; index 0 is the identity (colour 0 draws the glyph as it is). */
    text: Uint8Array[];
    nearest(rgb: number): number;
}

function nearestFn(rgb: Uint8Array): (c: number) => number {
    const cache = new Map<number, number>();
    return (c: number): number => {
        const hit = cache.get(c);
        if (hit !== undefined) return hit;
        const r = (c >> 16) & 255;
        const g = (c >> 8) & 255;
        const b = c & 255;
        let best = 0;
        let bestD = Infinity;
        for (let i = 0; i < 256; i++) {
            const dr = rgb[i * 3] - r;
            const dg = rgb[i * 3 + 1] - g;
            const db = rgb[i * 3 + 2] - b;
            // The engine's metric: plain squared distance, the first of equals wins
            // (D2GFX_FindNearestPaletteIndex, D2CMP/Draw.cpp:193).
            const d = dr * dr + dg * dg + db * db;
            if (d < bestD) {
                bestD = d;
                best = i;
            }
        }
        cache.set(c, best);
        return best;
    };
}

function identity(): Uint8Array {
    const t = new Uint8Array(256);
    for (let i = 0; i < 256; i++) t[i] = i;
    return t;
}

/** The game's palette from pal.pl2 bytes. */
export function palettePl2(pl2: Uint8Array): Palette {
    if (pl2.length < 0x6b627 + 13 * 256) throw new Error("pal.pl2: too short");
    const rgb = new Uint8Array(768);
    for (let i = 0; i < 256; i++) {
        // PALETTEENTRY order: r, g, b, flags.
        rgb[i * 3] = pl2[i * 4];
        rgb[i * 3 + 1] = pl2[i * 4 + 1];
        rgb[i * 3 + 2] = pl2[i * 4 + 2];
    }
    const text: Uint8Array[] = [identity()];
    for (let n = 1; n < 13; n++) text.push(pl2.subarray(0x6b627 + n * 256, 0x6b627 + (n + 1) * 256));
    return {
        rgb,
        alpha25: pl2.subarray(0x23500, 0x33500),
        alpha50: pl2.subarray(0x13500, 0x23500),
        alpha75: pl2.subarray(0x3500, 0x13500),
        additive: pl2.subarray(0x33500, 0x43500),
        multiply: pl2.subarray(0x43500, 0x53500),
        text,
        nearest: nearestFn(rgb),
    };
}

/** The test theme's text colours (its own choice, not the game's values). */
export const TEST_TEXT_RGB = [0xe8e8e8, 0xe05a50, 0x5ac85a, 0x7090ff, 0xd4b060, 0x7c7c7c, 0x141414, 0xc0ae88, 0xe8963a, 0xecec50, 0x2c8030, 0xb060e8, 0x60e860];

let testPalette: Palette | undefined;

/** A generated palette: a 6x6x6 colour cube and 40 greys, with blend tables computed from it. */
export function paletteTest(): Palette {
    if (testPalette !== undefined) return testPalette;
    const rgb = new Uint8Array(768);
    let n = 0;
    for (let r = 0; r < 6; r++)
        for (let g = 0; g < 6; g++)
            for (let b = 0; b < 6; b++) {
                rgb[n * 3] = r * 51;
                rgb[n * 3 + 1] = g * 51;
                rgb[n * 3 + 2] = b * 51;
                n++;
            }
    for (let i = 0; i < 40; i++) {
        const v = Math.round(((i + 1) * 255) / 41);
        rgb[n * 3] = v;
        rgb[n * 3 + 1] = v;
        rgb[n * 3 + 2] = v;
        n++;
    }
    const nearest = nearestFn(rgb);
    const table = (f: (d: number, s: number) => number): Uint8Array => {
        const t = new Uint8Array(65536);
        for (let d = 0; d < 256; d++)
            for (let s = 0; s < 256; s++) {
                let c = 0;
                for (let k = 0; k < 3; k++) {
                    const v = Math.max(0, Math.min(255, Math.round(f(rgb[d * 3 + k], rgb[s * 3 + k]))));
                    c = (c << 8) | v;
                }
                t[(d << 8) | s] = nearest(c);
            }
        return t;
    };
    const text: Uint8Array[] = [identity()];
    for (let c = 1; c < 13; c++) {
        const t = new Uint8Array(256);
        const cr = (TEST_TEXT_RGB[c] >> 16) & 255;
        const cg = (TEST_TEXT_RGB[c] >> 8) & 255;
        const cb = TEST_TEXT_RGB[c] & 255;
        for (let i = 0; i < 256; i++) {
            const lum = (rgb[i * 3] + rgb[i * 3 + 1] + rgb[i * 3 + 2]) / (3 * 255);
            t[i] = nearest((Math.round(cr * lum) << 16) | (Math.round(cg * lum) << 8) | Math.round(cb * lum));
        }
        text.push(t);
    }
    testPalette = {
        rgb,
        alpha25: table((d, s) => d * 0.75 + s * 0.25),
        alpha50: table((d, s) => d * 0.5 + s * 0.5),
        alpha75: table((d, s) => d * 0.25 + s * 0.75),
        additive: table((d, s) => d + s),
        multiply: table((d, s) => (d * s) / 255),
        text,
        nearest,
    };
    return testPalette;
}
