// Paletted PNG: write and read back (only what this writer produces: colour type 3, bit depth 8,
// filter 0 on every row). Tests compare decoded pixels and palette, so a different zlib build
// producing different compressed bytes does not break a golden image.

import * as zlib from "node:zlib";

const CRC = (() => {
    const t = new Uint32Array(256);
    for (let n = 0; n < 256; n++) {
        let c = n;
        for (let k = 0; k < 8; k++) c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1;
        t[n] = c >>> 0;
    }
    return t;
})();

function crc32(bufs: Uint8Array[]): number {
    let c = 0xffffffff;
    for (const b of bufs) for (let i = 0; i < b.length; i++) c = CRC[(c ^ b[i]) & 0xff] ^ (c >>> 8);
    return (c ^ 0xffffffff) >>> 0;
}

function chunk(type: string, data: Uint8Array): Buffer {
    const head = Buffer.alloc(8);
    head.writeUInt32BE(data.length, 0);
    head.write(type, 4, "ascii");
    const crc = Buffer.alloc(4);
    crc.writeUInt32BE(crc32([head.subarray(4), data]), 0);
    return Buffer.concat([head, data, crc]);
}

export interface Indexed {
    width: number;
    height: number;
    /** One palette index per pixel. */
    pixels: Uint8Array;
    /** 256 x (r, g, b). */
    rgb: Uint8Array;
}

export function encodePng(img: Indexed): Buffer {
    const ihdr = Buffer.alloc(13);
    ihdr.writeUInt32BE(img.width, 0);
    ihdr.writeUInt32BE(img.height, 4);
    ihdr[8] = 8;
    ihdr[9] = 3;
    const raw = Buffer.alloc((img.width + 1) * img.height);
    for (let y = 0; y < img.height; y++) raw.set(img.pixels.subarray(y * img.width, (y + 1) * img.width), y * (img.width + 1) + 1);
    const idat = zlib.deflateSync(raw, { level: 9 });
    return Buffer.concat([Buffer.from([0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a]), chunk("IHDR", ihdr), chunk("PLTE", img.rgb), chunk("IDAT", idat), chunk("IEND", new Uint8Array(0))]);
}

export function decodePng(buf: Uint8Array): Indexed {
    const b = Buffer.from(buf.buffer, buf.byteOffset, buf.byteLength);
    let p = 8;
    let width = 0;
    let height = 0;
    let rgb = new Uint8Array(768);
    const idat: Buffer[] = [];
    while (p < b.length) {
        const len = b.readUInt32BE(p);
        const type = b.toString("ascii", p + 4, p + 8);
        const data = b.subarray(p + 8, p + 8 + len);
        if (type === "IHDR") {
            width = data.readUInt32BE(0);
            height = data.readUInt32BE(4);
            if (data[8] !== 8 || data[9] !== 3) throw new Error("png: not an 8-bit paletted image");
        } else if (type === "PLTE") rgb = new Uint8Array(data);
        else if (type === "IDAT") idat.push(data);
        p += 12 + len;
    }
    const raw = zlib.inflateSync(Buffer.concat(idat));
    const pixels = new Uint8Array(width * height);
    for (let y = 0; y < height; y++) {
        if (raw[y * (width + 1)] !== 0) throw new Error("png: filtered rows are not supported");
        pixels.set(raw.subarray(y * (width + 1) + 1, (y + 1) * (width + 1)), y * width);
    }
    return { width, height, pixels, rgb };
}
