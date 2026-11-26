// PKWARE Data Compression Library "implode" decoder, the compression Blizzard's own MPQs use.
// Ported from libd2's packages/formats/src/pkware.zig (itself after Mark Adler's blast.c): a
// two-byte header (literals coded or not; log2 of the dictionary size), then an LSB-first bit
// stream of literals and (length, distance) pairs coded with three static Huffman tables.

const MAX_BITS = 13;

// The static tables as (repeat-1) << 4 | bits runs: these ARE the format.
const LIT_LEN = [
    11, 124, 8, 7, 28, 7, 188, 13, 76, 4, 10, 8, 12, 10, 12, 10, 8, 23, 8, 9, 7, 6, 7, 8, 7, 6, 55, 8, 23, 24, 12, 11, 7, 9, 11, 12, 6, 7, 22, 5, 7, 24, 6, 11, 9, 6, 7, 22, 7,
    11, 38, 7, 9, 8, 25, 11, 8, 11, 9, 12, 8, 12, 5, 38, 5, 38, 5, 11, 7, 5, 6, 21, 6, 10, 53, 8, 7, 24, 10, 27, 44, 253, 253, 253, 252, 252, 252, 13, 12, 45, 12, 45, 12, 61, 12,
    45, 44, 173,
];
const LEN_LEN = [2, 35, 36, 53, 38, 23];
const DIST_LEN = [2, 20, 53, 230, 247, 151, 248];
const LEN_BASE = [3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264];
const LEN_EXTRA = [0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8];

interface Huffman {
    count: number[];
    symbol: number[];
}

function construct(rep: number[], alphabet: number): Huffman {
    const lengths: number[] = [];
    for (const b of rep) for (let left = (b >> 4) + 1; left > 0; left--) lengths.push(b & 15);
    if (lengths.length !== alphabet) throw new Error(`explode: table expands to ${lengths.length}, not ${alphabet}`);
    const count = new Array<number>(MAX_BITS + 1).fill(0);
    for (const l of lengths) count[l]++;
    const offs = new Array<number>(MAX_BITS + 2).fill(0);
    for (let len = 1; len <= MAX_BITS; len++) offs[len + 1] = offs[len] + count[len];
    const symbol = new Array<number>(256).fill(0);
    lengths.forEach((l, sym) => {
        if (l !== 0) symbol[offs[l]++] = sym;
    });
    return { count, symbol };
}

const LIT = construct(LIT_LEN, 256);
const LENGTH = construct(LEN_LEN, 16);
const DIST = construct(DIST_LEN, 64);

class Bits {
    pos = 0;
    buf = 0;
    cnt = 0;
    constructor(private readonly src: Uint8Array) {}
    bits(need: number): number {
        if (need === 0) return 0;
        while (this.cnt < need) {
            if (this.pos >= this.src.length) throw new Error("explode: truncated stream");
            this.buf |= this.src[this.pos++] << this.cnt;
            this.cnt += 8;
        }
        const val = this.buf & ((1 << need) - 1);
        this.buf >>>= need;
        this.cnt -= need;
        return val;
    }
}

function decode(br: Bits, h: Huffman): number {
    let code = 0;
    let first = 0;
    let index = 0;
    for (let len = 1; len <= MAX_BITS; len++) {
        code |= br.bits(1) ^ 1;
        const count = h.count[len];
        if (code < first + count) return h.symbol[index + (code - first)];
        index += count;
        first = (first + count) << 1;
        code <<= 1;
    }
    throw new Error("explode: bad code");
}

/** Decodes src into a buffer of `size` bytes; stops at the end code or when full. */
export function explode(src: Uint8Array, size: number): Uint8Array {
    const dst = new Uint8Array(size);
    const br = new Bits(src);
    const coded = br.bits(8);
    if (coded > 1) throw new Error("explode: bad header");
    const dict = br.bits(8);
    if (dict < 4 || dict > 6) throw new Error("explode: bad dictionary size");
    let n = 0;
    while (n < size) {
        if (br.bits(1) !== 0) {
            const sym = decode(br, LENGTH);
            const len = LEN_BASE[sym] + br.bits(LEN_EXTRA[sym]);
            if (len === 519) break;
            const dbits = len === 2 ? 2 : dict;
            let dist = decode(br, DIST) << dbits;
            dist += br.bits(dbits) + 1;
            if (dist > n) throw new Error("explode: distance too far");
            let copy = Math.min(len, size - n);
            while (copy-- > 0) {
                dst[n] = dst[n - dist];
                n++;
            }
        } else {
            dst[n++] = coded !== 0 ? decode(br, LIT) : br.bits(8);
        }
    }
    return n === size ? dst : dst.subarray(0, n);
}
