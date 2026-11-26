// DC6: the game's paletted sprite format, decoded as D2CMP does (CelCmp.cpp, FastCmp.cpp).
// Header 0x18 bytes (version 6, flags, format, terminator, directions, frames), then one absolute
// u32 offset per frame. Frame header 0x20 bytes (flip, width, height, offsetX, offsetY, unknown,
// next block, length), then RLE: 0x80 ends a row, a byte with the high bit skips (b & 0x7f)
// transparent pixels, any other byte copies that many literal palette indices. Rows run
// bottom-up unless flip & 1.

export interface Frame {
    width: number;
    height: number;
    offsetX: number;
    offsetY: number;
    /** Palette index per pixel, row-major from the top. */
    pixels: Uint8Array;
    /** 1 where a pixel was written, 0 where the RLE skipped it. */
    mask: Uint8Array;
}

export interface Sprite {
    directions: number;
    frames: Frame[];
}

export function decodeDc6(buf: Uint8Array): Sprite {
    const v = new DataView(buf.buffer, buf.byteOffset, buf.byteLength);
    if (buf.length < 24) throw new Error("dc6: too short");
    const version = v.getUint32(0, true);
    if (version !== 6) throw new Error(`dc6: version ${version}`);
    const directions = v.getUint32(16, true);
    const perDir = v.getUint32(20, true);
    const count = directions * perDir;
    if (count > 4096) throw new Error("dc6: too many frames");
    const frames: Frame[] = [];
    for (let i = 0; i < count; i++) {
        const off = v.getUint32(24 + i * 4, true);
        if (off + 32 > buf.length) throw new Error("dc6: frame offset out of range");
        const flip = v.getUint32(off, true);
        const width = v.getUint32(off + 4, true);
        const height = v.getUint32(off + 8, true);
        const offsetX = v.getInt32(off + 12, true);
        const offsetY = v.getInt32(off + 16, true);
        const length = v.getUint32(off + 28, true);
        if (width > 4096 || height > 4096) throw new Error("dc6: frame too large");
        const pixels = new Uint8Array(width * height);
        const mask = new Uint8Array(width * height);
        const end = Math.min(buf.length, off + 32 + length);
        let p = off + 32;
        let x = 0;
        let row = 0;
        const bottomUp = (flip & 1) === 0;
        while (p < end && row < height) {
            const b = buf[p++];
            if (b === 0x80) {
                x = 0;
                row++;
            } else if ((b & 0x80) !== 0) {
                x += b & 0x7f;
            } else {
                const y = bottomUp ? height - 1 - row : row;
                for (let k = 0; k < b && p < end; k++, x++) {
                    const c = buf[p++];
                    if (x < width) {
                        pixels[y * width + x] = c;
                        mask[y * width + x] = 1;
                    }
                }
            }
        }
        frames.push({ width, height, offsetX, offsetY, pixels, mask });
    }
    return { directions, frames };
}
