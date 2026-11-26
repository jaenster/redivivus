/**
 * The mod profile: the subset of MPQ version 1 a mod archive may use, checked on untrusted
 * bytes before anything else reads them. README.md states the rules; every rule has one error
 * code (error.ts).
 */

import { MpqError, type MpqIssue } from './error.ts';
import {
  BLOCK_DELETED, BLOCK_EMPTY, ENTRY_SIZE, findHashEntry, FLAG_COMPRESS, FLAG_ENCRYPTED, FLAG_EXISTS, FLAG_FIX_KEY, FLAG_IMPLODE,
  FLAG_SINGLE_UNIT, HEADER_SIZE, parseHeader, readBlock, readBlockTable, readHashTable, sectorsEnd,
  type BlockEntry, type HashEntry, type Header,
} from './format.ts';
import { checkName, LISTFILE, MAX_NAME_LENGTH, nameKey } from './names.ts';

/** The only sector size the profile allows: 512 << 3 bytes, what Blizzard's own archives use. */
export const SECTOR_SHIFT = 3;
export const SECTOR_SIZE = 512 << SECTOR_SHIFT;

export interface Limits {
  /** The archive as uploaded. */
  maxArchiveBytes: number;
  /** Files besides the listfile. */
  maxFiles: number;
  /** One file, unpacked. */
  maxFileBytes: number;
  /** All files, unpacked. */
  maxTotalBytes: number;
  /** Unpacked total over archive size, applied once the unpacked total is above ratioFloorBytes. */
  maxRatio: number;
  ratioFloorBytes: number;
  maxNameLength: number;
  maxHashTableEntries: number;
}

export const DEFAULT_LIMITS: Limits = {
  maxArchiveBytes: 64 * 1024 * 1024,
  maxFiles: 4096,
  maxFileBytes: 64 * 1024 * 1024,
  maxTotalBytes: 256 * 1024 * 1024,
  maxRatio: 100,
  ratioFloorBytes: 16 * 1024 * 1024,
  maxNameLength: MAX_NAME_LENGTH,
  maxHashTableEntries: 65536,
};

export interface FileInfo {
  /** As the listfile spells it. */
  name: string;
  /** Unpacked. */
  size: number;
  /** In the archive, sector table included. */
  compressedSize: number;
  compressed: boolean;
}

export type ValidateResult =
  | { ok: true; files: FileInfo[]; totalSize: number }
  | { ok: false; errors: MpqIssue[] };

interface Container {
  header: Header;
  hashTable: HashEntry[];
  blocks: BlockEntry[];
}

/** At most this many name problems are reported per archive. */
const MAX_ISSUES = 50;

function limitsOf(over: Partial<Limits> | undefined): Limits {
  return { ...DEFAULT_LIMITS, ...over };
}

function checkBlock(b: BlockEntry, index: number, h: Header, limits: Limits): void {
  const where = `block ${index}`;
  if (!(b.flags & FLAG_EXISTS)) throw new MpqError('unsupported_flags', `${where} is not marked as an existing file`);
  if (b.flags & (FLAG_ENCRYPTED | FLAG_FIX_KEY)) throw new MpqError('encrypted_file', `${where} is encrypted`);
  if (b.flags & FLAG_IMPLODE) throw new MpqError('unsupported_compression', `${where} is imploded; only zlib is allowed`);
  if (b.flags & FLAG_SINGLE_UNIT) throw new MpqError('single_unit', `${where} is a single-unit file; files are stored in sectors`);
  if ((b.flags & ~(FLAG_EXISTS | FLAG_COMPRESS)) !== 0) {
    throw new MpqError('unsupported_flags', `${where} has flags 0x${b.flags.toString(16)}; only "exists" and "compressed" are allowed`);
  }
  if (b.size > limits.maxFileBytes) throw new MpqError('file_too_large', `${where} unpacks to ${b.size} bytes, over the limit of ${limits.maxFileBytes}`);
  if (b.offset < HEADER_SIZE || b.offset + b.compressedSize > h.archiveSize) throw new MpqError('bad_block', `${where} lies outside the archive`);
  if (b.flags & FLAG_COMPRESS) {
    if (b.size === 0) throw new MpqError('bad_block', `${where} is empty and marked compressed`);
  } else if (b.compressedSize !== b.size) {
    throw new MpqError('bad_block', `${where} is stored and does not take exactly its own size`);
  }
}

/** Header, tables and layout: everything that can be checked without unpacking a file. */
function container(bytes: Uint8Array, limits: Limits): Container {
  if (bytes.length > limits.maxArchiveBytes) throw new MpqError('too_large', `the archive is ${bytes.length} bytes, over the limit of ${limits.maxArchiveBytes}`);
  const h = parseHeader(bytes, 0);
  if (h.headerSize !== HEADER_SIZE) throw new MpqError('bad_header_size', `the header says it is ${h.headerSize} bytes; version 1 headers are ${HEADER_SIZE}`);
  if (h.formatVersion !== 0) throw new MpqError('unsupported_version', `format version ${h.formatVersion + 1} is not allowed; only version 1 is`);
  if (h.sectorShift !== SECTOR_SHIFT) throw new MpqError('bad_sector_size', `the sector size must be ${SECTOR_SIZE} bytes`);
  if (h.archiveSize !== bytes.length) throw new MpqError('bad_archive_size', `the header says the archive is ${h.archiveSize} bytes and the file is ${bytes.length}`);

  const hashCount = h.hashTableEntries;
  if (hashCount < 4 || hashCount > limits.maxHashTableEntries || (hashCount & (hashCount - 1)) !== 0) {
    throw new MpqError('bad_hash_table', `a hash table of ${hashCount} entries is not allowed: a power of two from 4 to ${limits.maxHashTableEntries}`);
  }
  if (h.hashTablePos < HEADER_SIZE || h.hashTablePos + hashCount * ENTRY_SIZE > h.archiveSize) throw new MpqError('bad_hash_table', 'the hash table lies outside the archive');
  const blockCount = h.blockTableEntries;
  if (blockCount < 1) throw new MpqError('missing_listfile', `the archive has no files, so no ${LISTFILE}`);
  if (blockCount > limits.maxFiles + 1) throw new MpqError('too_many_files', `the archive holds ${blockCount - 1} files, over the limit of ${limits.maxFiles}`);
  if (blockCount >= hashCount) throw new MpqError('bad_block_table', 'the hash table has no free entry left');
  if (h.blockTablePos < HEADER_SIZE || h.blockTablePos + blockCount * ENTRY_SIZE > h.archiveSize) throw new MpqError('bad_block_table', 'the block table lies outside the archive');

  const hashTable = readHashTable(bytes, h);
  const blocks = readBlockTable(bytes, h);

  const referenced = new Uint8Array(blockCount);
  for (const [slot, e] of hashTable.entries()) {
    if (e.blockIndex === BLOCK_EMPTY) {
      if (e.nameA !== 0xffffffff || e.nameB !== 0xffffffff || e.locale !== 0xffff || e.platform !== 0xffff) {
        throw new MpqError('bad_hash_entry', `hash entry ${slot} is free and not blank`);
      }
      continue;
    }
    if (e.blockIndex === BLOCK_DELETED) throw new MpqError('deleted_entry', `hash entry ${slot} is a deleted file`);
    if (e.blockIndex >= blockCount) throw new MpqError('bad_hash_entry', `hash entry ${slot} points at block ${e.blockIndex}, which does not exist`);
    if (e.locale !== 0 || e.platform !== 0) throw new MpqError('bad_locale', `hash entry ${slot} is for a locale or platform; only neutral files are allowed`);
    if (referenced[e.blockIndex]) throw new MpqError('duplicate_block_ref', `block ${e.blockIndex} is named by more than one hash entry`);
    referenced[e.blockIndex] = 1;
  }
  const orphan = referenced.indexOf(0);
  if (orphan !== -1) throw new MpqError('unreferenced_block', `block ${orphan} is not named by any hash entry`);

  let total = 0;
  for (const [i, b] of blocks.entries()) {
    checkBlock(b, i, h, limits);
    total += b.size;
  }
  if (total > limits.maxTotalBytes) throw new MpqError('total_too_large', `the files unpack to ${total} bytes, over the limit of ${limits.maxTotalBytes}`);
  if (total > limits.ratioFloorBytes && total > limits.maxRatio * bytes.length) {
    throw new MpqError('ratio_too_high', `the files unpack to more than ${limits.maxRatio} times the archive`);
  }

  // Every byte belongs to the header, a table or exactly one file.
  const regions: Array<{ from: number; to: number; what: string }> = [
    { from: 0, to: HEADER_SIZE, what: 'the header' },
    { from: h.hashTablePos, to: h.hashTablePos + hashCount * ENTRY_SIZE, what: 'the hash table' },
    { from: h.blockTablePos, to: h.blockTablePos + blockCount * ENTRY_SIZE, what: 'the block table' },
  ];
  for (const [i, b] of blocks.entries()) {
    if (b.compressedSize) regions.push({ from: b.offset, to: b.offset + b.compressedSize, what: `block ${i}` });
  }
  regions.sort((a, b) => a.from - b.from || a.to - b.to);
  let end = 0;
  let before = '';
  for (const r of regions) {
    if (r.from < end) throw new MpqError('overlap', `${r.what} overlaps ${before}`);
    if (r.from > end) throw new MpqError('unreferenced_data', `${r.from - end} bytes before ${r.what} belong to nothing`);
    end = r.to;
    before = r.what;
  }
  if (end !== h.archiveSize) throw new MpqError('unreferenced_data', `${h.archiveSize - end} bytes at the end of the archive belong to nothing`);
  return { header: h, hashTable, blocks };
}

/** A file's bytes, with nothing left behind its last sector. */
function unpack(bytes: Uint8Array, c: Container, block: BlockEntry, name: string): Uint8Array {
  try {
    if (block.flags & FLAG_COMPRESS) {
      if (block.compressedSize < (Math.ceil(block.size / SECTOR_SIZE) + 1) * 4) throw new MpqError('bad_sector_table', 'the file is too small to hold its sector table');
      if (sectorsEnd(bytes, c.header, block) !== block.compressedSize) throw new MpqError('bad_sector_table', 'the sectors do not end where the file does');
    }
    return readBlock(bytes, c.header, block);
  } catch (err) {
    if (err instanceof MpqError) throw new MpqError(err.code, `${name}: ${err.message}`, name);
    throw err;
  }
}

interface Listed {
  files: Array<FileInfo & { block: BlockEntry }>;
  issues: MpqIssue[];
}

/** The listfile, and that it names every file exactly once. */
function listed(bytes: Uint8Array, c: Container, limits: Limits): Listed {
  const slot = findHashEntry(c.hashTable, LISTFILE);
  if (slot === -1) throw new MpqError('missing_listfile', `the archive has no ${LISTFILE}`);
  const listBlock = c.blocks[c.hashTable[slot]!.blockIndex]!;
  if (listBlock.size > (limits.maxFiles + 1) * (limits.maxNameLength + 2)) throw new MpqError('bad_listfile', `the ${LISTFILE} is larger than its names could make it`);
  const raw = unpack(bytes, c, listBlock, LISTFILE);
  let text = '';
  for (const byte of raw) {
    if (byte !== 0x0a && byte !== 0x0d && (byte < 0x20 || byte > 0x7e)) throw new MpqError('bad_listfile', `the ${LISTFILE} holds a byte that is not printable ASCII`);
    text += String.fromCharCode(byte);
  }
  const lines = text.length ? text.replace(/\r?\n$/, '').split(/\r?\n/) : [];
  if (lines.some((l) => l === '' || l.includes('\r'))) throw new MpqError('bad_listfile', `the ${LISTFILE} holds an empty line or a stray carriage return`);

  const issues: MpqIssue[] = [];
  const files: Listed['files'] = [];
  const seenKeys = new Map<string, string>();
  const seenSlots = new Map<number, string>();
  for (const name of lines) {
    try {
      checkName(name, limits.maxNameLength);
      const key = nameKey(name);
      const twin = seenKeys.get(key);
      if (twin !== undefined) {
        throw new MpqError('duplicate_name', twin === name ? `${name} is listed twice` : `${name} and ${twin} differ only by case`, name);
      }
      seenKeys.set(key, name);
      const at = findHashEntry(c.hashTable, name);
      if (at === -1) throw new MpqError('listed_name_missing', `${name} is listed and not in the archive`, name);
      const other = seenSlots.get(at);
      if (other !== undefined) throw new MpqError('name_hash_collision', `${name} and ${other} hash to the same entry`, name);
      seenSlots.set(at, name);
      const block = c.blocks[c.hashTable[at]!.blockIndex]!;
      files.push({ name, size: block.size, compressedSize: block.compressedSize, compressed: (block.flags & FLAG_COMPRESS) !== 0, block });
    } catch (err) {
      if (!(err instanceof MpqError)) throw err;
      if (issues.length < MAX_ISSUES) issues.push(err.issue());
    }
  }
  if (!issues.length && files.length + 1 !== c.blocks.length) {
    issues.push({ code: 'unlisted_files', message: `the archive holds ${c.blocks.length - 1} files and the ${LISTFILE} names ${files.length}` });
  }
  return { files, issues };
}

function info(f: FileInfo): FileInfo {
  return { name: f.name, size: f.size, compressedSize: f.compressedSize, compressed: f.compressed };
}

/** Called by `validate` with each file as it is unpacked, in listfile order. The bytes are the caller's to keep. */
export type Visit = (file: FileInfo, data: Uint8Array) => void;

/**
 * Check a whole archive against the profile, unpacking every file once. A structural problem
 * is reported alone, since nothing after it can be trusted; problems with names are reported
 * together.
 *
 * `visit` sees every file's contents on that one pass, so a caller that checks what the files
 * hold does not unpack them a second time. It is called only once the container and the
 * listfile are sound, but before the files after the one it is shown: whatever it gathers
 * counts only if the answer is ok.
 */
export function validate(bytes: Uint8Array, limits?: Partial<Limits>, visit?: Visit): ValidateResult {
  const lim = limitsOf(limits);
  try {
    const c = container(bytes, lim);
    const l = listed(bytes, c, lim);
    if (l.issues.length) return { ok: false, errors: l.issues };
    let totalSize = 0;
    for (const f of l.files) {
      const data = unpack(bytes, c, f.block, f.name);
      if (visit) visit(info(f), data);
      totalSize += f.size;
    }
    return { ok: true, files: l.files.map(info), totalSize };
  } catch (err) {
    if (err instanceof MpqError) return { ok: false, errors: [err.issue()] };
    throw err;
  }
}

/**
 * The files of an archive, in listfile order. The container and the listfile are checked; the
 * files themselves are not unpacked. Throws MpqError.
 */
export function list(bytes: Uint8Array, limits?: Partial<Limits>): FileInfo[] {
  const lim = limitsOf(limits);
  const l = listed(bytes, container(bytes, lim), lim);
  const first = l.issues[0];
  if (first) throw new MpqError(first.code, first.message, first.name);
  return l.files.map(info);
}

/**
 * One file by name (case-insensitive, either separator), or null when the archive has no such
 * file. The container is checked and the file unpacked strictly; the listfile is not read.
 * Throws MpqError.
 */
export function read(bytes: Uint8Array, name: string, limits?: Partial<Limits>): Uint8Array | null {
  const c = container(bytes, limitsOf(limits));
  const slot = findHashEntry(c.hashTable, name);
  if (slot === -1) return null;
  return unpack(bytes, c, c.blocks[c.hashTable[slot]!.blockIndex]!, name);
}
