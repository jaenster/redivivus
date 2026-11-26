#!/usr/bin/env node
// lint-literal-sizes: count decompiler-folded size literals that should be sizeof/offsetof.
//
//  (a) literal sizes passed to memset/memcpy/memmove/ZeroMemory/CopyMemory/FillMemory,
//      SMemZero/SMemFill/SMemCopy/SMemAlloc/SMemReAlloc, D2_ALLOC_AT/D2_ALLOC_POOL_AT/
//      D2_REALLOC_POOL_AT and Fog::Memory::Alloc*Memory (a literal >= 2 outside sizeof()).
//  (b) byte-offset pointer arithmetic whose result is reinterpreted as another type:
//      (T*)((char*)p + 0x24), *(T*)((uint8_t*)&x + 8), (T*)((intptr_t)p + 0x10), ((char*)p)[0x24].
//  (c) byte strides: (char*)p + i * 0x18, and `p += 0x18` on a pointer this file declares as a
//      byte pointer (char/uint8_t/BYTE/unsigned char).
//
// usage (from the repository root):
//   node asm_matching/lint/literal-sizes.mjs [root] [--verbose] [--class a|b|c]
//   node asm_matching/lint/literal-sizes.mjs --baseline asm_matching/lint/literal-sizes.baseline.tsv
//       the gate: exit 1 if any file's count went above its baseline row (a file with no row
//       has a baseline of 0), so the total can only go down
//   node asm_matching/lint/literal-sizes.mjs --write-baseline asm_matching/lint/literal-sizes.baseline.tsv
//       after a change that lowered counts, so the lower numbers become the new ceiling
// root defaults to the current directory; scans root/Source and root/Redivivus, skipping 3rdparty.
// The fix for a hit: sizeof(T), offsetof(T, field), the real field, array indexing or _countof, with
// static_assert(sizeof(void*) != 4 || sizeof(T) == 0xNN) next to the type. A byte offset into a raw
// buffer (packet, file image) that really is one stays and keeps its row in the baseline.
import fs from 'node:fs';
import path from 'node:path';

const argv = process.argv.slice(2);
let root = '.', baseline = null, writeBaseline = null, verbose = false, onlyCls = null;
for (let i = 0; i < argv.length; i++) {
  const a = argv[i];
  if (a === '--baseline') baseline = argv[++i];
  else if (a === '--write-baseline') writeBaseline = argv[++i];
  else if (a === '--verbose' || a === '-v') verbose = true;
  else if (a === '--class') onlyCls = argv[++i];
  else if (a === '-h' || a === '--help') { console.log(fs.readFileSync(new URL(import.meta.url)).toString().split('\n').filter((l) => l.startsWith('//')).map((l) => l.slice(3)).join('\n')); process.exit(0); }
  else root = a;
}

function walk(dir, acc) {
  let ents;
  try { ents = fs.readdirSync(dir, { withFileTypes: true }); } catch { return acc; }
  for (const e of ents) {
    const p = path.join(dir, e.name);
    if (e.isDirectory()) { if (!/^(3rdparty|crt|CRT|vc6)$/.test(e.name)) walk(p, acc); }
    else if (/\.(cpp|h|hpp|inl|c)$/.test(e.name)) acc.push(p);
  }
  return acc;
}

function strip(src) {
  const o = src.split('');
  const n = src.length;
  const blank = (a, b) => { for (let k = a; k < b; k++) if (o[k] !== '\n') o[k] = ' '; };
  let i = 0;
  while (i < n) {
    const c = src[i], d = src[i + 1];
    if (c === '/' && d === '/') { const e = src.indexOf('\n', i); const end = e < 0 ? n : e; blank(i, end); i = end; continue; }
    if (c === '/' && d === '*') { const e = src.indexOf('*/', i + 2); const end = e < 0 ? n : e + 2; blank(i, end); i = end; continue; }
    if (c === '"' || c === "'") { let j = i + 1; while (j < n && src[j] !== c && src[j] !== '\n') { if (src[j] === '\\') j++; j++; } blank(i + 1, j); i = j + 1; continue; }
    i++;
  }
  // preprocessor lines (macro definitions already spell sizeof)
  const lines = o.join('').split('\n');
  let cont = false;
  for (let k = 0; k < lines.length; k++) if (cont || /^\s*#/.test(lines[k])) { cont = /\\\s*$/.test(lines[k]); lines[k] = ' '.repeat(lines[k].length); }
  return lines.join('\n');
}

const LIT = '(?:0[xX][0-9a-fA-F]+|[0-9]+)(?:[uU]?[lL]{0,2}|[lL]{1,2}[uU]?)?';
const litVal = (t) => { t = t.replace(/[uUlL]+$/, ''); return /^0[xX]/.test(t) ? parseInt(t, 16) : parseInt(t, 10); };
const BYTE_T = '(?:const\\s+)?(?:char|signed\\s+char|unsigned\\s+char|uint8_t|int8_t|BYTE|byte|u8|undefined1|uchar|UCHAR)';
const INT_T = '(?:intptr_t|uintptr_t|INT_PTR|UINT_PTR|DWORD_PTR|LONG_PTR|ULONG_PTR|size_t|int|unsigned\\s+int|uint|DWORD|uint32_t|int32_t|long|ULONG)';
const BYTE_PTR_RE = new RegExp(`^${BYTE_T}\\s*\\*$`);

function matchParen(s, open) {
  let depth = 0;
  for (let k = open; k < s.length; k++) {
    const ch = s[k];
    if (ch === '(' || ch === '[' || ch === '{') depth++;
    else if (ch === ')' || ch === ']' || ch === '}') { depth--; if (depth === 0) return k; }
  }
  return -1;
}
function splitArgs(s, open, close) {
  const args = [];
  let depth = 0, st = open + 1;
  for (let k = open + 1; k < close; k++) {
    const ch = s[k];
    if (ch === '(' || ch === '[' || ch === '{') depth++;
    else if (ch === ')' || ch === ']' || ch === '}') depth--;
    else if (ch === ',' && depth === 0) { args.push(s.slice(st, k)); st = k + 1; }
  }
  args.push(s.slice(st, close));
  return args;
}
function postfixEnd(s, i) {
  let k = i;
  while (/\s/.test(s[k])) k++;
  const start = k;
  if (s[k] === '&' || s[k] === '*') { k++; while (/\s/.test(s[k])) k++; }
  if (s[k] === '(') { const e = matchParen(s, k); if (e < 0) return -1; k = e + 1; }
  else { const m = /^(?:::)?[A-Za-z_]\w*(?:::[A-Za-z_~]\w*)*/.exec(s.slice(k, k + 400)); if (!m) return -1; k += m[0].length; }
  for (;;) {
    const r = /^\s*(->|\.)\s*([A-Za-z_]\w*)/.exec(s.slice(k, k + 200));
    if (r) { k += r[0].length; continue; }
    let j = k; while (s[j] === ' ' || s[j] === '\t') j++;
    if (s[j] === '[' || (s[j] === '(' && /[\w\]]$/.test(s.slice(start, k)))) { const e = matchParen(s, j); if (e < 0) break; k = e + 1; continue; }
    break;
  }
  return k;
}
function sizeHasLiteral(expr) {
  let e = expr;
  for (;;) {
    const m = /\bsizeof\s*\(/.exec(e);
    if (!m) break;
    const c = matchParen(e, m.index + m[0].length - 1);
    if (c < 0) break;
    e = e.slice(0, m.index) + ' '.repeat(c + 1 - m.index) + e.slice(c + 1);
  }
  const re = new RegExp(`(?<![\\w.])(${LIT})(?![\\w.])`, 'g');
  let m;
  while ((m = re.exec(e))) if (litVal(m[1]) >= 2) return m[1];
  return null;
}

const SIZE_ARG = {
  memset: 2, memcpy: 2, memmove: 2, ZeroMemory: 1, RtlZeroMemory: 1, CopyMemory: 2, MoveMemory: 2, FillMemory: 1,
  SMemZero: 1, SMemFill: 1, SMemCopy: 2, SMemMove: 2, SMemAlloc: 0, SMemReAlloc: 1,
  D2_ALLOC_AT: 0, D2_ALLOC_POOL_AT: 1, D2_REALLOC_POOL_AT: 2, AllocClientMemory: 0, AllocServerMemory: 1, ReAllocServerMemory: 2,
};

function lintFile(raw) {
  const s = strip(raw);
  const hits = [];
  const starts = [0];
  for (let k = 0; k < s.length; k++) if (s[k] === '\n') starts.push(k + 1);
  const lineOf = (p) => { let lo = 0, hi = starts.length - 1; while (lo < hi) { const m = (lo + hi + 1) >> 1; if (starts[m] <= p) lo = m; else hi = m - 1; } return lo + 1; };
  const seen = new Set();
  const hit = (cls, pos, lit, what) => { const ln = lineOf(pos); const key = `${cls}:${pos}`; if (seen.has(key)) return; seen.add(key); hits.push({ cls, line: ln, lit, what }); };

  // (a)
  {
    const re = new RegExp(`\\b(${Object.keys(SIZE_ARG).join('|')})\\s*\\(`, 'g');
    let m;
    while ((m = re.exec(s))) {
      const pre = s.slice(Math.max(0, m.index - 40), m.index);
      if (/(?:\bvoid\s*\*?|\bBOOL|\bint|\bLPVOID)\s*(?:__\w+\s+)?$/.test(pre) && !/(=|return|\(|,)\s*$/.test(pre)) continue; // declaration
      const open = m.index + m[0].length - 1;
      const close = matchParen(s, open);
      if (close < 0) continue;
      const args = splitArgs(s, open, close);
      const ai = SIZE_ARG[m[1]];
      if (args.length <= ai) continue;
      const lit = sizeHasLiteral(args[ai]);
      if (lit) hit('a', m.index, lit, m[1]);
    }
  }
  // (b)/(c) byte casts
  {
    const re = new RegExp(`\\(\\s*${BYTE_T}\\s*\\*\\s*\\)`, 'g');
    let m;
    while ((m = re.exec(s))) {
      const opStart = m.index + m[0].length;
      const opEnd = postfixEnd(s, opStart);
      if (opEnd < 0) continue;
      const op = s.slice(opStart, opEnd).trim();
      if (!op || /^\d/.test(op)) continue;
      const tail = s.slice(opEnd, opEnd + 120);
      const before = s.slice(Math.max(0, m.index - 80), m.index);
      const outer = /\(\s*((?:const\s+)?(?:struct\s+|class\s+)?[A-Za-z_][\w:<>]*\s*\*+)\s*\)\s*\(\s*$/.exec(before);
      const outerNonByte = outer && !BYTE_PTR_RE.test(outer[1].replace(/\*+$/, '*').trim());
      let r;
      if ((r = new RegExp(`^\\s*\\+\\s*\\(?\\s*(?:[A-Za-z_][\\w.\\->]*\\s*\\*\\s*(${LIT})|(${LIT})\\s*\\*\\s*[A-Za-z_][\\w.\\->]*)(?![\\w.])`).exec(tail))) {
        const t = r[1] || r[2];
        if (litVal(t) >= 2) hit('c', m.index, t, `(byte*)${op} + i*${t}`);
        continue;
      }
      if ((r = new RegExp(`^\\s*[+-]\\s*(${LIT})(?![\\w.*])(?!\\s*\\*)`).exec(tail))) {
        if (litVal(r[1]) === 0) continue;
        if (outerNonByte || /^&/.test(op)) hit('b', m.index, r[1], `(byte*)${op} +- ${r[1]}`);
        continue;
      }
      if (/\(\s*$/.test(s.slice(Math.max(0, m.index - 2), m.index))) {
        const rr = new RegExp(`^\\s*\\)\\s*\\[\\s*(${LIT})\\s*\\]`).exec(tail);
        if (rr && litVal(rr[1]) > 0) hit('b', m.index, rr[1], `((byte*)${op})[${rr[1]}]`);
      }
    }
  }
  // (b) (T*)((intptr_t)p + LIT)
  {
    const re = new RegExp(`\\(\\s*(?:const\\s+)?(?:struct\\s+|class\\s+)?[A-Za-z_][\\w:<>]*\\s*\\*+\\s*\\)\\s*\\(\\s*\\(\\s*${INT_T}\\s*\\)`, 'g');
    let m;
    while ((m = re.exec(s))) {
      const opStart = m.index + m[0].length;
      const opEnd = postfixEnd(s, opStart);
      if (opEnd < 0) continue;
      const r = new RegExp(`^\\s*[+-]\\s*(${LIT})(?![\\w.])\\s*\\)`).exec(s.slice(opEnd, opEnd + 60));
      if (r && litVal(r[1]) > 0) hit('b', m.index, r[1], 'int-cast address arithmetic');
      const r2 = new RegExp(`^\\s*\\+\\s*\\(?\\s*[A-Za-z_][\\w.\\->]*\\s*\\*\\s*(${LIT})(?![\\w.])`).exec(s.slice(opEnd, opEnd + 60));
      if (r2 && litVal(r2[1]) >= 2) hit('c', m.index, r2[1], 'int-cast index stride');
    }
  }
  // (c) p += LIT on a byte pointer declared in this file
  {
    const bytePtrs = new Set();
    const dre = new RegExp(`\\b${BYTE_T}\\s*\\*\\s*([A-Za-z_]\\w*)\\s*(?=[=;,)\\[])`, 'g');
    let m;
    while ((m = dre.exec(s))) bytePtrs.add(m[1]);
    const re = new RegExp(`(?<![\\w.>])([A-Za-z_]\\w*)\\s*(?:\\+=|-=)\\s*(${LIT})(?![\\w.])\\s*;`, 'g');
    while ((m = re.exec(s))) if (bytePtrs.has(m[1]) && litVal(m[2]) >= 2) hit('c', m.index, m[2], `${m[1]} += ${m[2]}`);
    const re2 = new RegExp(`(?<![\\w.>])([A-Za-z_]\\w*)\\s*=\\s*\\1\\s*([+-])\\s*(${LIT})(?![\\w.])\\s*;`, 'g');
    while ((m = re2.exec(s))) if (bytePtrs.has(m[1]) && litVal(m[3]) >= 2) hit('c', m.index, m[3], `${m[1]} = ${m[1]} ${m[2]} ${m[3]}`);
  }
  return onlyCls ? hits.filter((h) => h.cls === onlyCls) : hits;
}

const files = walk(path.join(root, 'Source'), []).concat(walk(path.join(root, 'Redivivus'), [])).sort();
const counts = {};
const tot = { a: 0, b: 0, c: 0 };
for (const f of files) {
  const rel = path.relative(root, f);
  const hits = lintFile(fs.readFileSync(f, 'utf8'));
  if (!hits.length) continue;
  counts[rel] = hits.length;
  for (const h of hits) tot[h.cls]++;
  if (verbose) for (const h of hits) console.log(`${rel}:${h.line}\t${h.cls}\t${h.lit}\t${h.what}`);
}
const total = tot.a + tot.b + tot.c;
if (!verbose && !baseline) for (const [f, n] of Object.entries(counts).sort((x, y) => y[1] - x[1])) console.log(`${n}\t${f}`);
console.log(`TOTAL\t${total}\ta=${tot.a}\tb=${tot.b}\tc=${tot.c}\tfiles=${Object.keys(counts).length}`);

if (writeBaseline) {
  fs.writeFileSync(writeBaseline, Object.entries(counts).sort().map(([f, n]) => `${n}\t${f}`).join('\n') + '\n');
}
if (baseline) {
  const base = {};
  for (const l of fs.readFileSync(baseline, 'utf8').split('\n')) { const m = /^(\d+)\t(.+)$/.exec(l); if (m) base[m[2]] = +m[1]; }
  let bad = 0, down = 0;
  for (const [f, n] of Object.entries(counts)) {
    const b = base[f] ?? 0;
    if (n > b) { console.log(`UP\t${f}\t${b} -> ${n}`); bad++; }
    else if (n < b) down++;
  }
  for (const f of Object.keys(base)) if (!(f in counts)) down++;
  console.log(bad ? `FAIL: ${bad} file(s) gained literal sizes` : `OK: no file gained literal sizes (${down} went down; refresh the baseline with --write-baseline)`);
  process.exit(bad ? 1 : 0);
}
