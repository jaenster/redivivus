/**
 * The writer. The same files give the same bytes on every machine: files are ordered by
 * their upper-cased names, the layout is fixed, nothing carries a time, and compression is one
 * pinned compressor at one setting. The archive's SHA-256 is a mod version's identity, so a
 * change to anything here that moves a byte is a change to that identity.
 */

import { zlibSync } from 'fflate';
import { BLOCK_TABLE_KEY, encryptBlock, HASH_NAME_A, HASH_NAME_B, HASH_TABLE_KEY, HASH_TABLE_OFFSET, hashString } from './crypt.ts';
import { MpqError } from './error.ts';
import { COMPRESSION_ZLIB, ENTRY_SIZE, FLAG_COMPRESS, FLAG_EXISTS, HEADER_SIZE, MAGIC } from './format.ts';
import { checkName, LISTFILE, nameKey } from './names.ts';
import { SECTOR_SHIFT, SECTOR_SIZE } from './profile.ts';

export interface InputFile {
  /** Backslash-separated, relative; see names.ts for what is allowed (normalizeName converts '/'). */
  name: string;
  data: Uint8Array;
}

/** The hash table holds at least 16 entries and is the smallest power of two that stays at most three quarters full. */
export function hashTableSize(entries: number): number {
  let size = 16;
  while (entries * 4 > size * 3) size *= 2;
  return size;
}

interface Packed {
  bytes: Uint8Array[];
  length: number;
  flags: number;
}

/** A file as it lies in the archive: a sector table and zlib sectors when that is smaller, the plain bytes otherwise. */
function pack(data: Uint8Array): Packed {
  const stored: Packed = { bytes: [data], length: data.length, flags: FLAG_EXISTS };
  if (!data.length) return stored;
  const count = Math.ceil(data.length / SECTOR_SIZE);
  const table = new Uint8Array((count + 1) * 4);
  const view = new DataView(table.buffer);
  const sectors: Uint8Array[] = [table];
  let at = table.length;
  for (let i = 0; i < count; i++) {
    view.setUint32(i * 4, at, true);
    const plain = data.subarray(i * SECTOR_SIZE, Math.min(data.length, (i + 1) * SECTOR_SIZE));
    const deflated = zlibSync(plain, { level: 9, mem: 8 });
    if (deflated.length + 1 < plain.length) {
      const sector = new Uint8Array(deflated.length + 1);
      sector[0] = COMPRESSION_ZLIB;
      sector.set(deflated, 1);
      sectors.push(sector);
      at += sector.length;
    } else {
      sectors.push(plain);
      at += plain.length;
    }
  }
  view.setUint32(count * 4, at, true);
  return at < data.length ? { bytes: sectors, length: at, flags: (FLAG_EXISTS | FLAG_COMPRESS) >>> 0 } : stored;
}

/**
 * Build an archive in the mod profile from files; the listfile is written for them. Throws
 * MpqError for a name the profile refuses or two names that differ only by case. The size
 * limits are the reader's: run validate() on the result to apply them.
 */
export function build(files: InputFile[]): Uint8Array {
  const keys = new Map<string, string>();
  for (const f of files) {
    checkName(f.name);
    const key = nameKey(f.name);
    const twin = keys.get(key);
    if (twin !== undefined) {
      throw new MpqError('duplicate_name', twin === f.name ? `${f.name} is given twice` : `${f.name} and ${twin} differ only by case`, f.name);
    }
    keys.set(key, f.name);
  }
  const sorted = [...files].sort((a, b) => (nameKey(a.name) < nameKey(b.name) ? -1 : 1));
  const listfile = new TextEncoder().encode(sorted.map((f) => `${f.name}\r\n`).join(''));
  const entries: InputFile[] = [...sorted, { name: LISTFILE, data: listfile }];

  const packed = entries.map((f) => pack(f.data));
  const hashCount = hashTableSize(entries.length);
  const dataEnd = packed.reduce((n, p) => n + p.length, HEADER_SIZE);
  const total = dataEnd + (hashCount + entries.length) * ENTRY_SIZE;
  if (total > 0xffffffff) throw new MpqError('too_large', 'the archive would be over 4 GiB');

  const out = new Uint8Array(total);
  const view = new DataView(out.buffer);
  view.setUint32(0, MAGIC, true);
  view.setUint32(4, HEADER_SIZE, true);
  view.setUint32(8, total, true);
  view.setUint16(12, 0, true);
  view.setUint16(14, SECTOR_SHIFT, true);
  view.setUint32(16, dataEnd, true);
  view.setUint32(20, dataEnd + hashCount * ENTRY_SIZE, true);
  view.setUint32(24, hashCount, true);
  view.setUint32(28, entries.length, true);

  const hashAt = dataEnd;
  const blockAt = dataEnd + hashCount * ENTRY_SIZE;
  out.fill(0xff, hashAt, blockAt);
  const taken = new Map<number, string>();
  const pairs = new Map<string, string>();
  let at = HEADER_SIZE;
  for (const [i, f] of entries.entries()) {
    const p = packed[i]!;
    const start = at;
    for (const chunk of p.bytes) {
      out.set(chunk, at);
      at += chunk.length;
    }
    view.setUint32(blockAt + i * ENTRY_SIZE, start, true);
    view.setUint32(blockAt + i * ENTRY_SIZE + 4, p.length, true);
    view.setUint32(blockAt + i * ENTRY_SIZE + 8, f.data.length, true);
    view.setUint32(blockAt + i * ENTRY_SIZE + 12, p.flags, true);

    const a = hashString(f.name, HASH_NAME_A);
    const b = hashString(f.name, HASH_NAME_B);
    const clash = pairs.get(`${a}:${b}`);
    if (clash !== undefined) throw new MpqError('name_hash_collision', `${f.name} and ${clash} hash to the same entry`, f.name);
    pairs.set(`${a}:${b}`, f.name);
    let slot = hashString(f.name, HASH_TABLE_OFFSET) & (hashCount - 1);
    while (taken.has(slot)) slot = (slot + 1) & (hashCount - 1);
    taken.set(slot, f.name);
    const e = hashAt + slot * ENTRY_SIZE;
    view.setUint32(e, a, true);
    view.setUint32(e + 4, b, true);
    view.setUint32(e + 8, 0, true);
    view.setUint32(e + 12, i, true);
  }
  encryptBlock(out, hashAt, hashCount * ENTRY_SIZE, HASH_TABLE_KEY);
  encryptBlock(out, blockAt, entries.length * ENTRY_SIZE, BLOCK_TABLE_KEY);
  return out;
}
