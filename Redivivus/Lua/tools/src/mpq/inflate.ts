/**
 * A strict zlib (RFC 1950/1951) decoder for one MPQ sector. It differs from a general
 * inflate in what it refuses: the output must be exactly the size the archive declared, the
 * stream must use every input byte, and the Adler-32 must match. A sector therefore cannot
 * expand past its bound or carry bytes behind the stream.
 */

import { MpqError } from './error.ts';

const LENGTH_BASE = [3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258];
const LENGTH_EXTRA = [0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0];
const DIST_BASE = [1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577];
const DIST_EXTRA = [0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13];
const CODE_LENGTH_ORDER = [16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15];

interface Table {
  /** Indexed by the next `bits` input bits; an entry is (symbol << 4) | code length, 0 for no code. */
  entries: Uint16Array;
  bits: number;
}

function bad(what: string): never {
  throw new MpqError('bad_zlib', `invalid zlib stream: ${what}`);
}

/**
 * A decoding table for a canonical Huffman code. An over-subscribed set is refused; an
 * incomplete one is refused unless `allowIncomplete` and it has at most one 1-bit code, which
 * is what zlib accepts for literals and distances.
 */
function buildTable(lengths: ArrayLike<number>, count: number, allowIncomplete: boolean): Table {
  const perLength = new Uint16Array(16);
  let max = 0;
  for (let i = 0; i < count; i++) {
    const l = lengths[i]!;
    perLength[l]! += 1;
    if (l > max) max = l;
  }
  if (max === 0) {
    if (!allowIncomplete) bad('a code with no symbols');
    return { entries: new Uint16Array(2), bits: 1 };
  }
  let left = 1;
  for (let l = 1; l <= 15; l++) {
    left = (left << 1) - perLength[l]!;
    if (left < 0) bad('over-subscribed code');
  }
  if (left > 0 && !(allowIncomplete && max === 1)) bad('incomplete code');
  const next = new Uint16Array(16);
  perLength[0] = 0;
  for (let l = 1, code = 0; l <= 15; l++) {
    code = (code + perLength[l - 1]!) << 1;
    next[l] = code;
  }
  const entries = new Uint16Array(1 << max);
  for (let sym = 0; sym < count; sym++) {
    const l = lengths[sym]!;
    if (!l) continue;
    let code = next[l]!;
    next[l]! += 1;
    // Deflate packs codes most significant bit first into a least-significant-first stream.
    let rev = 0;
    for (let i = 0; i < l; i++, code >>>= 1) rev = (rev << 1) | (code & 1);
    const entry = (sym << 4) | l;
    for (let at = rev; at < entries.length; at += 1 << l) entries[at] = entry;
  }
  return { entries, bits: max };
}

let fixed: { lit: Table; dist: Table } | null = null;

function fixedTables(): { lit: Table; dist: Table } {
  if (!fixed) {
    const lit = new Uint8Array(288);
    lit.fill(8, 0, 144);
    lit.fill(9, 144, 256);
    lit.fill(7, 256, 280);
    lit.fill(8, 280, 288);
    // 32 five-bit distance codes; the last two are never valid and are refused where they are used.
    fixed = { lit: buildTable(lit, 288, false), dist: buildTable(new Uint8Array(32).fill(5), 32, false) };
  }
  return fixed;
}

export function adler32(bytes: Uint8Array): number {
  let a = 1;
  let b = 0;
  for (let i = 0; i < bytes.length;) {
    const end = Math.min(bytes.length, i + 3800);
    for (; i < end; i++) {
      a += bytes[i]!;
      b += a;
    }
    a %= 65521;
    b %= 65521;
  }
  return ((b << 16) | a) >>> 0;
}

/** Inflate a whole zlib stream into exactly `size` bytes. */
export function inflateZlib(src: Uint8Array, size: number): Uint8Array {
  if (src.length < 6) bad('too short');
  const cmf = src[0]!;
  const flg = src[1]!;
  if ((cmf & 0x0f) !== 8 || cmf >>> 4 > 7 || ((cmf << 8) | flg) % 31 !== 0) bad('bad header');
  if (flg & 0x20) bad('preset dictionary');

  const out = new Uint8Array(size);
  const end = src.length - 4;
  let pos = 2;
  let op = 0;
  let buf = 0;
  let cnt = 0;

  const fill = (): void => {
    while (cnt <= 24 && pos < end) {
      buf |= src[pos++]! << cnt;
      cnt += 8;
    }
  };
  const take = (n: number): number => {
    if (n > cnt) bad('truncated');
    const v = buf & ((1 << n) - 1);
    buf >>>= n;
    cnt -= n;
    return v;
  };
  const decode = (t: Table): number => {
    const e = t.entries[buf & ((1 << t.bits) - 1)]!;
    const l = e & 15;
    if (!l) bad('invalid code');
    if (l > cnt) bad('truncated');
    buf >>>= l;
    cnt -= l;
    return e >>> 4;
  };

  for (let last = false; !last;) {
    fill();
    last = take(1) === 1;
    const type = take(2);
    if (type === 3) bad('reserved block type');
    if (type === 0) {
      // Back to a byte boundary; whole bytes still in the bit buffer are returned to the input.
      take(cnt & 7);
      pos -= cnt >>> 3;
      buf = 0;
      cnt = 0;
      if (pos + 4 > end) bad('truncated');
      const len = src[pos]! | (src[pos + 1]! << 8);
      const nlen = src[pos + 2]! | (src[pos + 3]! << 8);
      pos += 4;
      if ((len ^ 0xffff) !== nlen) bad('stored block length');
      if (pos + len > end) bad('truncated');
      if (op + len > size) bad('more output than declared');
      out.set(src.subarray(pos, pos + len), op);
      pos += len;
      op += len;
      continue;
    }

    let lit: Table;
    let dist: Table;
    if (type === 1) {
      ({ lit, dist } = fixedTables());
    } else {
      const hlit = take(5) + 257;
      const hdist = take(5) + 1;
      const hclen = take(4) + 4;
      if (hlit > 286 || hdist > 30) bad('too many codes');
      const clLengths = new Uint8Array(19);
      for (let i = 0; i < hclen; i++) {
        fill();
        clLengths[CODE_LENGTH_ORDER[i]!] = take(3);
      }
      const cl = buildTable(clLengths, 19, false);
      const lengths = new Uint8Array(hlit + hdist);
      for (let i = 0; i < lengths.length;) {
        fill();
        const sym = decode(cl);
        if (sym < 16) {
          lengths[i++] = sym;
          continue;
        }
        let value = 0;
        let repeat: number;
        if (sym === 16) {
          if (i === 0) bad('repeat with no previous length');
          value = lengths[i - 1]!;
          repeat = 3 + take(2);
        } else if (sym === 17) {
          repeat = 3 + take(3);
        } else {
          repeat = 11 + take(7);
        }
        if (i + repeat > lengths.length) bad('too many lengths');
        lengths.fill(value, i, i + repeat);
        i += repeat;
      }
      if (!lengths[256]) bad('no end-of-block code');
      lit = buildTable(lengths, hlit, true);
      dist = buildTable(lengths.subarray(hlit), hdist, true);
    }

    for (;;) {
      fill();
      let sym = decode(lit);
      if (sym < 256) {
        if (op >= size) bad('more output than declared');
        out[op++] = sym;
        continue;
      }
      if (sym === 256) break;
      sym -= 257;
      if (sym > 28) bad('invalid length code');
      const len = LENGTH_BASE[sym]! + take(LENGTH_EXTRA[sym]!);
      fill();
      const dsym = decode(dist);
      if (dsym > 29) bad('invalid distance code');
      fill();
      const d = DIST_BASE[dsym]! + take(DIST_EXTRA[dsym]!);
      if (d > op) bad('distance before the start');
      if (op + len > size) bad('more output than declared');
      for (let from = op - d, to = op + len; op < to;) out[op++] = out[from++]!;
    }
  }

  take(cnt & 7);
  pos -= cnt >>> 3;
  if (pos !== end) bad('data after the stream');
  if (op !== size) bad('less output than declared');
  const sum = ((src[end]! << 24) | (src[end + 1]! << 16) | (src[end + 2]! << 8) | src[end + 3]!) >>> 0;
  if (sum !== adler32(out)) bad('checksum mismatch');
  return out;
}
