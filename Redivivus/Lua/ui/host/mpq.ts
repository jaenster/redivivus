// A read-only MPQ reader for Blizzard's own Diablo II archives (format version 0/1): hash and
// block tables, encrypted files (with the FIX_KEY rule), single-unit and sectored files, the old
// IMPLODE flag and the COMPRESS mask byte with PKWARE (0x08) and zlib (0x02). Huffman, ADPCM and
// bzip2 are refused with a clear error: they occur only in sound files, which the UI never reads.
//
// Written for this kit from the MPQ format as StormLib documents it; the PKWARE decoder is ported
// from libd2 (explode.ts).

import * as fs from "node:fs";
import * as zlib from "node:zlib";
import { explode } from "./explode";

const FLAG_IMPLODE = 0x100;
const FLAG_COMPRESS = 0x200;
const FLAG_ENCRYPTED = 0x10000;
const FLAG_FIX_KEY = 0x20000;
const FLAG_SINGLE_UNIT = 0x1000000;
const FLAG_SECTOR_CRC = 0x4000000;
const FLAG_EXISTS = 0x80000000;

const CRYPT = (() => {
    const t = new Uint32Array(0x500);
    let seed = 0x00100001;
    for (let i = 0; i < 0x100; i++) {
        let idx = i;
        for (let j = 0; j < 5; j++) {
            seed = (seed * 125 + 3) % 0x2aaaab;
            const t1 = (seed & 0xffff) << 16;
            seed = (seed * 125 + 3) % 0x2aaaab;
            const t2 = seed & 0xffff;
            t[idx] = (t1 | t2) >>> 0;
            idx += 0x100;
        }
    }
    return t;
})();

export function hashString(name: string, type: number): number {
    let seed1 = 0x7fed7fed;
    let seed2 = 0xeeeeeeee;
    for (let i = 0; i < name.length; i++) {
        let ch = name.charCodeAt(i) & 0xff;
        if (ch >= 97 && ch <= 122) ch -= 32;
        if (ch === 47) ch = 92; // "/" is "\"
        seed1 = (CRYPT[type * 0x100 + ch] ^ ((seed1 + seed2) >>> 0)) >>> 0;
        seed2 = (ch + seed1 + seed2 + (seed2 << 5) + 3) >>> 0;
    }
    return seed1;
}

function decrypt(words: Uint32Array, key0: number): void {
    let key = key0 >>> 0;
    let seed = 0xeeeeeeee;
    for (let i = 0; i < words.length; i++) {
        seed = (seed + CRYPT[0x400 + (key & 0xff)]) >>> 0;
        const ch = (words[i] ^ ((key + seed) >>> 0)) >>> 0;
        key = ((((~key << 0x15) >>> 0) + 0x11111111) | (key >>> 0x0b)) >>> 0;
        seed = (ch + seed + (seed << 5) + 3) >>> 0;
        words[i] = ch;
    }
}

function decryptBytes(buf: Uint8Array, key: number): Uint8Array {
    const out = new Uint8Array(buf);
    const n = Math.floor(out.length / 4);
    const words = new Uint32Array(out.buffer, out.byteOffset, n);
    decrypt(words, key);
    return out;
}

interface Block {
    pos: number;
    csize: number;
    fsize: number;
    flags: number;
}

export class Mpq {
    readonly path: string;
    private fd: number;
    private base = 0;
    private sectorSize = 4096;
    private hash: Uint32Array = new Uint32Array(0);
    private blocks: Block[] = [];

    constructor(path: string) {
        this.path = path;
        this.fd = fs.openSync(path, "r");
        const size = fs.fstatSync(this.fd).size;
        const head = Buffer.alloc(32);
        let found = false;
        for (let off = 0; off + 32 <= size; off += 512) {
            fs.readSync(this.fd, head, 0, 32, off);
            if (head.readUInt32LE(0) === 0x1a51504d) {
                this.base = off;
                found = true;
                break;
            }
            if (off > 64 * 1024 * 1024) break;
        }
        if (!found) throw new Error(`${path}: no MPQ header`);
        this.sectorSize = 512 << head.readUInt16LE(14);
        const hashPos = head.readUInt32LE(16);
        const blockPos = head.readUInt32LE(20);
        const hashCount = head.readUInt32LE(24);
        const blockCount = head.readUInt32LE(28);
        const hbuf = this.read(this.base + hashPos, hashCount * 16);
        this.hash = new Uint32Array(hashCount * 4);
        for (let i = 0; i < this.hash.length; i++) this.hash[i] = hbuf.readUInt32LE(i * 4);
        decrypt(this.hash, hashString("(hash table)", 3));
        const bbuf = this.read(this.base + blockPos, blockCount * 16);
        const bw = new Uint32Array(blockCount * 4);
        for (let i = 0; i < bw.length; i++) bw[i] = bbuf.readUInt32LE(i * 4);
        decrypt(bw, hashString("(block table)", 3));
        for (let i = 0; i < blockCount; i++) this.blocks.push({ pos: bw[i * 4], csize: bw[i * 4 + 1], fsize: bw[i * 4 + 2], flags: bw[i * 4 + 3] });
    }

    private read(pos: number, len: number): Buffer {
        const b = Buffer.alloc(len);
        const got = fs.readSync(this.fd, b, 0, len, pos);
        if (got !== len) throw new Error(`${this.path}: short read at ${pos}`);
        return b;
    }

    private find(name: string): Block | undefined {
        const count = this.hash.length / 4;
        if (count === 0) return undefined;
        const a = hashString(name, 1);
        const b = hashString(name, 2);
        let i = hashString(name, 0) & (count - 1);
        for (let n = 0; n < count; n++) {
            const block = this.hash[i * 4 + 3];
            if (block === 0xffffffff) return undefined;
            if (block !== 0xfffffffe && this.hash[i * 4] === a && this.hash[i * 4 + 1] === b) {
                const bl = this.blocks[block];
                if (bl !== undefined && (bl.flags & FLAG_EXISTS) !== 0) return bl;
            }
            i = (i + 1) & (count - 1);
        }
        return undefined;
    }

    has(name: string): boolean {
        return this.find(name) !== undefined;
    }

    /** The file's bytes, or undefined when the archive does not hold it. */
    get(name: string): Uint8Array | undefined {
        const bl = this.find(name);
        if (bl === undefined) return undefined;
        let key = 0;
        if ((bl.flags & FLAG_ENCRYPTED) !== 0) {
            const slash = Math.max(name.lastIndexOf("\\"), name.lastIndexOf("/"));
            key = hashString(name.substring(slash + 1), 3);
            if ((bl.flags & FLAG_FIX_KEY) !== 0) key = ((key + bl.pos) ^ bl.fsize) >>> 0;
        }
        const raw = this.read(this.base + bl.pos, bl.csize);
        const compressed = (bl.flags & (FLAG_IMPLODE | FLAG_COMPRESS)) !== 0;
        if ((bl.flags & FLAG_SINGLE_UNIT) !== 0 || !compressed) {
            if (!compressed) {
                if ((bl.flags & FLAG_SINGLE_UNIT) !== 0 || (bl.flags & FLAG_ENCRYPTED) === 0) {
                    const data = (bl.flags & FLAG_ENCRYPTED) !== 0 ? decryptBytes(raw, key) : new Uint8Array(raw);
                    return data.subarray(0, bl.fsize);
                }
                // Stored but encrypted per sector.
                const out = new Uint8Array(bl.fsize);
                for (let s = 0, off = 0; off < bl.fsize; s++, off += this.sectorSize) {
                    const len = Math.min(this.sectorSize, bl.fsize - off);
                    out.set(decryptBytes(raw.subarray(off, off + len), key + s).subarray(0, len), off);
                }
                return out;
            }
            const data = (bl.flags & FLAG_ENCRYPTED) !== 0 ? decryptBytes(raw, key) : new Uint8Array(raw);
            return this.sector(data, bl.fsize, bl.flags);
        }
        const sectors = Math.ceil(bl.fsize / this.sectorSize);
        const entries = sectors + 1 + ((bl.flags & FLAG_SECTOR_CRC) !== 0 ? 1 : 0);
        const table = new Uint32Array(entries);
        for (let i = 0; i < entries; i++) table[i] = raw.readUInt32LE(i * 4);
        if ((bl.flags & FLAG_ENCRYPTED) !== 0) decrypt(table, (key - 1) >>> 0);
        const out = new Uint8Array(bl.fsize);
        for (let s = 0; s < sectors; s++) {
            const start = table[s];
            const end = table[s + 1];
            if (end < start || end > raw.length) throw new Error(`${this.path}: ${name}: bad sector table`);
            let data: Uint8Array = new Uint8Array(raw.subarray(start, end));
            if ((bl.flags & FLAG_ENCRYPTED) !== 0) data = decryptBytes(data, (key + s) >>> 0);
            const size = Math.min(this.sectorSize, bl.fsize - s * this.sectorSize);
            out.set(this.sector(data, size, bl.flags), s * this.sectorSize);
        }
        return out;
    }

    private sector(data: Uint8Array, size: number, flags: number): Uint8Array {
        if (data.length >= size) return data.subarray(0, size);
        if ((flags & FLAG_IMPLODE) !== 0) return explode(data, size);
        const mask = data[0];
        let body = data.subarray(1);
        if ((mask & 0x10) !== 0) throw new Error("mpq: bzip2 sectors are not supported");
        if ((mask & 0x08) !== 0) body = explode(body, size);
        if ((mask & 0x02) !== 0) body = new Uint8Array(zlib.inflateSync(body));
        if ((mask & 0xc1) !== 0) throw new Error(`mpq: compression 0x${mask.toString(16)} (Huffman/ADPCM) is not supported`);
        return body.subarray(0, size);
    }

    close(): void {
        fs.closeSync(this.fd);
    }
}

/** The 1.14d archives in search order, highest priority first, as the game mounts them. */
export const ARCHIVES_114D = ["Patch_D2.mpq", "d2exp.mpq", "d2data.mpq", "d2char.mpq"];

/** Files looked up across several archives, first match wins. */
export class MpqSet {
    readonly archives: Mpq[] = [];
    constructor(dir: string, names: string[] = ARCHIVES_114D) {
        const onDisk = fs.readdirSync(dir);
        for (const n of names) {
            const actual = onDisk.find((f) => f.toLowerCase() === n.toLowerCase());
            if (actual !== undefined) this.archives.push(new Mpq(`${dir}/${actual}`));
        }
        if (this.archives.length === 0) throw new Error(`${dir}: none of ${names.join(", ")}`);
    }
    get(name: string): Uint8Array | undefined {
        for (const a of this.archives) {
            const d = a.get(name);
            if (d !== undefined) return d;
        }
        return undefined;
    }
    close(): void {
        for (const a of this.archives) a.close();
    }
}
