/**
 * The MPQ cipher and name hashes. Both come from one 1280-entry table; the hash and block
 * tables of every archive are encrypted with it under fixed keys, which is part of the format
 * and not a secret.
 */

export const HASH_TABLE_OFFSET = 0;
export const HASH_NAME_A = 1;
export const HASH_NAME_B = 2;
export const HASH_FILE_KEY = 3;

export type HashType = typeof HASH_TABLE_OFFSET | typeof HASH_NAME_A | typeof HASH_NAME_B | typeof HASH_FILE_KEY;

export const cryptTable: Uint32Array = (() => {
  const table = new Uint32Array(0x500);
  let seed = 0x00100001;
  for (let i = 0; i < 0x100; i++) {
    for (let j = 0, at = i; j < 5; j++, at += 0x100) {
      seed = (seed * 125 + 3) % 0x2aaaab;
      const hi = (seed & 0xffff) << 16;
      seed = (seed * 125 + 3) % 0x2aaaab;
      table[at] = (hi | (seed & 0xffff)) >>> 0;
    }
  }
  return table;
})();

/** Storm's name hash: ASCII letters fold to upper case and '/' is '\'. Characters above 0xFF are not names. */
export function hashString(name: string, type: HashType): number {
  let seed1 = 0x7fed7fed;
  let seed2 = 0xeeeeeeee;
  for (let i = 0; i < name.length; i++) {
    let ch = name.charCodeAt(i) & 0xff;
    if (ch >= 0x61 && ch <= 0x7a) ch -= 0x20;
    else if (ch === 0x2f) ch = 0x5c;
    seed1 = (cryptTable[type * 0x100 + ch]! ^ (seed1 + seed2)) >>> 0;
    seed2 = (ch + seed1 + seed2 + (seed2 << 5) + 3) >>> 0;
  }
  return seed1;
}

export const HASH_TABLE_KEY = hashString('(hash table)', HASH_FILE_KEY);
export const BLOCK_TABLE_KEY = hashString('(block table)', HASH_FILE_KEY);

/** Decrypt little-endian 32-bit words in place; `length` is rounded down to whole words. */
export function decryptBlock(bytes: Uint8Array, offset: number, length: number, key: number): void {
  const view = new DataView(bytes.buffer, bytes.byteOffset + offset, length & ~3);
  let seed = 0xeeeeeeee;
  for (let i = 0; i + 4 <= view.byteLength; i += 4) {
    seed = (seed + cryptTable[0x400 + (key & 0xff)]!) >>> 0;
    const plain = (view.getUint32(i, true) ^ (key + seed)) >>> 0;
    key = (((~key << 21) + 0x11111111) | (key >>> 11)) >>> 0;
    seed = (plain + seed + (seed << 5) + 3) >>> 0;
    view.setUint32(i, plain, true);
  }
}

/** Encrypt little-endian 32-bit words in place; `length` is rounded down to whole words. */
export function encryptBlock(bytes: Uint8Array, offset: number, length: number, key: number): void {
  const view = new DataView(bytes.buffer, bytes.byteOffset + offset, length & ~3);
  let seed = 0xeeeeeeee;
  for (let i = 0; i + 4 <= view.byteLength; i += 4) {
    seed = (seed + cryptTable[0x400 + (key & 0xff)]!) >>> 0;
    const plain = view.getUint32(i, true);
    view.setUint32(i, (plain ^ (key + seed)) >>> 0, true);
    key = (((~key << 21) + 0x11111111) | (key >>> 11)) >>> 0;
    seed = (plain + seed + (seed << 5) + 3) >>> 0;
  }
}
