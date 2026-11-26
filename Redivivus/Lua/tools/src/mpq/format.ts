/**
 * The MPQ container as it is on disk, format version 1: header, hash table, block table and a
 * file's sectors. Nothing here applies the mod profile (profile.ts does); these functions read
 * any version 1 archive, including the ones Blizzard shipped, as far as their compression is
 * zlib or none.
 */

import {
  BLOCK_TABLE_KEY, decryptBlock, HASH_FILE_KEY, HASH_NAME_A, HASH_NAME_B, HASH_TABLE_KEY, HASH_TABLE_OFFSET, hashString,
} from './crypt.ts';
import { MpqError } from './error.ts';
import { inflateZlib } from './inflate.ts';

export const MAGIC = 0x1a51504d; // 'MPQ\x1A'
export const HEADER_SIZE = 32;
export const ENTRY_SIZE = 16;

export const FLAG_IMPLODE = 0x00000100;
export const FLAG_COMPRESS = 0x00000200;
export const FLAG_ENCRYPTED = 0x00010000;
export const FLAG_FIX_KEY = 0x00020000;
export const FLAG_PATCH_FILE = 0x00100000;
export const FLAG_SINGLE_UNIT = 0x01000000;
export const FLAG_DELETE_MARKER = 0x02000000;
export const FLAG_SECTOR_CRC = 0x04000000;
export const FLAG_EXISTS = 0x80000000;

export const BLOCK_EMPTY = 0xffffffff;
export const BLOCK_DELETED = 0xfffffffe;

/** The first byte of a compressed sector: which compressors were applied. Only zlib alone is read. */
export const COMPRESSION_ZLIB = 0x02;

export interface Header {
  /** Where the header starts in the buffer; every position in the archive is relative to it. */
  offset: number;
  headerSize: number;
  archiveSize: number;
  formatVersion: number;
  sectorShift: number;
  sectorSize: number;
  hashTablePos: number;
  blockTablePos: number;
  hashTableEntries: number;
  blockTableEntries: number;
}

export interface HashEntry {
  nameA: number;
  nameB: number;
  locale: number;
  platform: number;
  blockIndex: number;
}

export interface BlockEntry {
  /** Relative to the header. */
  offset: number;
  compressedSize: number;
  size: number;
  flags: number;
}

function view(bytes: Uint8Array): DataView {
  return new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
}

/** The header at `offset`. Only the magic and the room for 32 bytes are checked. */
export function parseHeader(bytes: Uint8Array, offset = 0): Header {
  if (bytes.length < offset + HEADER_SIZE) throw new MpqError('too_small', 'the file is too small to hold an MPQ header');
  const v = view(bytes);
  if (v.getUint32(offset, true) !== MAGIC) throw new MpqError('bad_magic', 'the file does not start with an MPQ header');
  const sectorShift = v.getUint16(offset + 14, true);
  return {
    offset,
    headerSize: v.getUint32(offset + 4, true),
    archiveSize: v.getUint32(offset + 8, true),
    formatVersion: v.getUint16(offset + 12, true),
    sectorShift,
    sectorSize: sectorShift <= 16 ? 512 << sectorShift : 0,
    hashTablePos: v.getUint32(offset + 16, true),
    blockTablePos: v.getUint32(offset + 20, true),
    hashTableEntries: v.getUint32(offset + 24, true),
    blockTableEntries: v.getUint32(offset + 28, true),
  };
}

function table(bytes: Uint8Array, h: Header, pos: number, entries: number, key: number, what: 'bad_hash_table' | 'bad_block_table'): DataView {
  const start = h.offset + pos;
  const length = entries * ENTRY_SIZE;
  if (start + length > bytes.length) throw new MpqError(what, `the ${what === 'bad_hash_table' ? 'hash' : 'block'} table runs past the end of the file`);
  const copy = bytes.slice(start, start + length);
  decryptBlock(copy, 0, length, key);
  return view(copy);
}

export function readHashTable(bytes: Uint8Array, h: Header): HashEntry[] {
  const v = table(bytes, h, h.hashTablePos, h.hashTableEntries, HASH_TABLE_KEY, 'bad_hash_table');
  const out: HashEntry[] = [];
  for (let at = 0; at < v.byteLength; at += ENTRY_SIZE) {
    out.push({
      nameA: v.getUint32(at, true),
      nameB: v.getUint32(at + 4, true),
      locale: v.getUint16(at + 8, true),
      platform: v.getUint16(at + 10, true),
      blockIndex: v.getUint32(at + 12, true),
    });
  }
  return out;
}

export function readBlockTable(bytes: Uint8Array, h: Header): BlockEntry[] {
  const v = table(bytes, h, h.blockTablePos, h.blockTableEntries, BLOCK_TABLE_KEY, 'bad_block_table');
  const out: BlockEntry[] = [];
  for (let at = 0; at < v.byteLength; at += ENTRY_SIZE) {
    out.push({
      offset: v.getUint32(at, true),
      compressedSize: v.getUint32(at + 4, true),
      size: v.getUint32(at + 8, true),
      flags: v.getUint32(at + 12, true),
    });
  }
  return out;
}

/**
 * The hash table slot a name resolves to, probing as Storm does: from the name's table-offset
 * hash, forward, wrapping, until a never-used slot. -1 when the name is not in the archive.
 * The table length must be a power of two.
 */
export function findHashEntry(hashTable: HashEntry[], name: string): number {
  const size = hashTable.length;
  if (!size) return -1;
  const a = hashString(name, HASH_NAME_A);
  const b = hashString(name, HASH_NAME_B);
  const start = hashString(name, HASH_TABLE_OFFSET) & (size - 1);
  for (let i = 0; i < size; i++) {
    const slot = (start + i) & (size - 1);
    const e = hashTable[slot]!;
    if (e.blockIndex === BLOCK_EMPTY) return -1;
    if (e.blockIndex !== BLOCK_DELETED && e.nameA === a && e.nameB === b) return slot;
  }
  return -1;
}

/** The key a file's sectors are encrypted with: from the name without its directory. */
export function fileKey(name: string, block: BlockEntry): number {
  const base = name.slice(Math.max(name.lastIndexOf('\\'), name.lastIndexOf('/')) + 1);
  const key = hashString(base, HASH_FILE_KEY);
  return block.flags & FLAG_FIX_KEY ? ((key + block.offset) ^ block.size) >>> 0 : key;
}

export interface ReadOptions {
  /** Needed for an encrypted file (see fileKey); ignored otherwise. */
  key?: number;
  /** Refuse a file whose declared size is above this, before any memory is taken for it. */
  maxSize?: number;
}

/**
 * A file's contents from its block entry. Stored files and files whose sectors are zlib
 * compressed are read; imploded files, other compressors and single-unit files are refused
 * with `unsupported_compression` or `single_unit`.
 */
export function readBlock(bytes: Uint8Array, h: Header, block: BlockEntry, opts: ReadOptions = {}): Uint8Array {
  const { flags, size, compressedSize } = block;
  if (opts.maxSize !== undefined && size > opts.maxSize) throw new MpqError('file_too_large', `a file of ${size} bytes is over the limit of ${opts.maxSize}`);
  if (flags & FLAG_SINGLE_UNIT) throw new MpqError('single_unit', 'single-unit files are not read');
  if (flags & FLAG_IMPLODE) throw new MpqError('unsupported_compression', 'imploded files are not read');
  const encrypted = (flags & FLAG_ENCRYPTED) !== 0;
  if (encrypted && opts.key === undefined) throw new MpqError('encrypted_file', 'the file is encrypted and no key was given');
  const start = h.offset + block.offset;
  if (start + compressedSize > bytes.length) throw new MpqError('bad_block', 'a file runs past the end of the archive');
  if (!h.sectorSize) throw new MpqError('bad_sector_size', 'the sector size is not usable');
  const sectors = Math.ceil(size / h.sectorSize);

  if (!(flags & FLAG_COMPRESS)) {
    if (compressedSize !== size) throw new MpqError('bad_block', 'a stored file must take exactly its own size');
    const out = bytes.slice(start, start + size);
    if (encrypted) {
      for (let i = 0; i < sectors; i++) {
        const at = i * h.sectorSize;
        decryptBlock(out, at, Math.min(h.sectorSize, size - at), (opts.key! + i) >>> 0);
      }
    }
    return out;
  }

  if (size === 0) return new Uint8Array(0);
  const tableEntries = sectors + 1 + (flags & FLAG_SECTOR_CRC ? 1 : 0);
  const tableBytes = tableEntries * 4;
  if (tableBytes > compressedSize) throw new MpqError('bad_sector_table', 'a file is too small to hold its sector table');
  const offsets = bytes.slice(start, start + tableBytes);
  if (encrypted) decryptBlock(offsets, 0, tableBytes, (opts.key! - 1) >>> 0);
  const ov = view(offsets);
  if (ov.getUint32(0, true) !== tableBytes) throw new MpqError('bad_sector_table', 'a sector table does not start where it should');
  if (ov.getUint32(sectors * 4, true) > compressedSize) throw new MpqError('bad_sector_table', 'a sector table points past its file');

  const out = new Uint8Array(size);
  for (let i = 0; i < sectors; i++) {
    const from = ov.getUint32(i * 4, true);
    const to = ov.getUint32(i * 4 + 4, true);
    const expected = Math.min(h.sectorSize, size - i * h.sectorSize);
    if (to <= from || to > compressedSize || to - from > expected) throw new MpqError('bad_sector_table', `sector ${i} has an impossible extent`);
    let sector = bytes.subarray(start + from, start + to);
    if (encrypted) {
      sector = sector.slice();
      decryptBlock(sector, 0, sector.length, (opts.key! + i) >>> 0);
    }
    if (sector.length === expected) {
      out.set(sector, i * h.sectorSize);
      continue;
    }
    if (sector[0] !== COMPRESSION_ZLIB) throw new MpqError('unsupported_compression', `sector ${i} uses compression 0x${sector[0]!.toString(16).padStart(2, '0')}, only zlib (0x02) is read`);
    out.set(inflateZlib(sector.subarray(1), expected), i * h.sectorSize);
  }
  return out;
}

/** Where the sectors of a compressed file end, by its own sector table: its compressed size if nothing is hidden behind them. */
export function sectorsEnd(bytes: Uint8Array, h: Header, block: BlockEntry): number {
  const sectors = Math.ceil(block.size / h.sectorSize);
  return view(bytes).getUint32(h.offset + block.offset + sectors * 4, true);
}
