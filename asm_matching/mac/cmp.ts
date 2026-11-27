// Compares the Mac reconstruction with the shipped 1.14d Intel Mac binary, function by function.
//
//   node asm_matching/mac/cmp.ts --build <mac build dir> [--binary <DiabloII_macho>] [--tsv <out>] [-v] [filter...]
//
// Every function or global in a Mac source carrying "// 1.14d mac: <addr>" on the line before its
// definition is paired with that address. For each function, our object's instruction stream is
// compared with the original's over the original's whole extent (LC_FUNCTION_STARTS bounds it).
// Branch targets compare as instruction indices, calls by callee, PIC data references by what they
// point at: a symbol plus offset, a string's text, or an imported symbol. A reference that only one
// side can name is counted as unverified rather than failed. See README.md.
import { execFileSync } from "node:child_process";
import { createHash } from "node:crypto";
import { existsSync, mkdirSync, readdirSync, readFileSync, statSync, writeFileSync } from "node:fs";
import { dirname, join, relative, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = resolve(HERE, "../..");
const BINARY_SHA256 = "1e8dd6dd3d19019cbaa29270f4abfb04960fcf1ce5d341fa520863b0d640d177";
// Every tree a Mac tag may stand in: StormMac and MacSpecific, the Mac renderer (Source/D2OpenGL/Src)
// and the D2_OS_MAC blocks of the shared code. A tagged file the Mac build does not compile has no
// object to compare; it still names its functions and globals for the references of the rest.
const SOURCE_ROOTS = ["Source", "Redivivus"];

// ---- arguments ----
const argv = process.argv.slice(2);
let buildDir = "";
let binary = process.env.D2_MAC_BINARY ?? "";
let tsvOut = "";
let insnsOut = "";
let verbose = false;
const filters: string[] = [];
for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    if (a === "--build") buildDir = resolve(argv[++i]);
    else if (a === "--binary") binary = resolve(argv[++i]);
    else if (a === "--tsv") tsvOut = resolve(argv[++i]);
    else if (a === "--insns") insnsOut = resolve(argv[++i]);
    else if (a === "-v") verbose = true;
    else filters.push(a);
}
if (!buildDir || !binary) {
    console.error("usage: node asm_matching/mac/cmp.ts --build <dir> [--binary <DiabloII_macho>] [--tsv out] [--insns out.jsonl] [-v] [filter...]");
    console.error("       the binary may also come from D2_MAC_BINARY");
    process.exit(2);
}

function tool(name: string): string {
    for (const d of (process.env.PATH ?? "").split(":").concat(["/opt/homebrew/opt/llvm/bin", "/usr/local/opt/llvm/bin"])) {
        if (d && existsSync(join(d, name))) return join(d, name);
    }
    throw new Error(`${name} not found (Homebrew llvm provides it)`);
}
const OBJDUMP = tool("llvm-objdump");
const NM = tool("llvm-nm");
const CXXFILT = tool("llvm-cxxfilt");
const run = (exe: string, args: string[], input?: string) =>
    execFileSync(exe, args, { encoding: "utf8", maxBuffer: 1 << 30, input });

// ---- Mach-O sections, enough to read string literals ----
interface Section { seg: string; name: string; addr: number; size: number; offset: number }
function machoSections(buf: Buffer): Section[] {
    if (buf.readUInt32LE(0) !== 0xfeedface) throw new Error("not a 32-bit little-endian Mach-O");
    const ncmds = buf.readUInt32LE(16);
    const out: Section[] = [];
    let p = 28;
    for (let c = 0; c < ncmds; c++) {
        const cmd = buf.readUInt32LE(p), size = buf.readUInt32LE(p + 4);
        if (cmd === 1) {  // LC_SEGMENT
            const nsects = buf.readUInt32LE(p + 48);
            for (let s = 0; s < nsects; s++) {
                const q = p + 56 + s * 68;
                const str = (o: number) => buf.toString("latin1", q + o, q + o + 16).replace(/\0.*$/s, "");
                out.push({ name: str(0), seg: str(16), addr: buf.readUInt32LE(q + 32), size: buf.readUInt32LE(q + 36), offset: buf.readUInt32LE(q + 40) });
            }
        }
        p += size;
    }
    return out;
}
function cstringAt(buf: Buffer, secs: Section[], addr: number): string | null {
    const s = secs.find((x) => x.name === "__cstring" && addr >= x.addr && addr < x.addr + x.size);
    if (!s) return null;
    const o = s.offset + addr - s.addr;
    const text = buf.toString("latin1", o, buf.indexOf(0, o));
    // a __FILE__ literal compares by its file name: the build machine's directory is not the source's
    // and by name only up to case: the original's files are partly upper case (SSTR.CPP), ours not
    const file = /^[^\n]*\/([^/\n]+\.(?:cpp|c|h|hpp|mm))$/i.exec(text);
    return file ? `__FILE__ ${file[1].toLowerCase()}` : JSON.stringify(text);
}
// A Pascal string literal ("\\p...", -fpascal-strings) lands in __TEXT,__const: length byte, then text.
function pascalAt(buf: Buffer, secs: Section[], addr: number): string | null {
    const s = secs.find((x) => x.seg === "__TEXT" && x.name === "__const" && addr >= x.addr && addr < x.addr + x.size);
    if (!s) return null;
    const o = s.offset + addr - s.addr, n = buf[o];
    if (addr + 1 + n > s.addr + s.size) return null;
    return "p" + JSON.stringify(buf.toString("latin1", o + 1, o + 1 + n));
}
function readU32At(buf: Buffer, secs: Section[], addr: number): number {
    const s = secs.find((x) => addr >= x.addr && addr + 4 <= x.addr + x.size && x.offset);
    return s ? buf.readUInt32LE(s.offset + addr - s.addr) : 0;
}
function indirectSymbols(path: string): Map<number, string> {
    const m = new Map<number, string>();
    for (const l of run(OBJDUMP, ["--macho", "--indirect-symbols", path]).split("\n")) {
        // "0x<addr> <index> <name>", or "0x<addr> LOCAL" / "ABSOLUTE" with no index
        const r = /^0x([0-9a-f]+)\s+(?:\d+\s+)?(\S+)/.exec(l);
        if (r) m.set(parseInt(r[1], 16), r[2]);
    }
    return m;
}

// ---- instructions ----
interface Insn { addr: number; text: string; reloc?: string; note?: string }
const DIS_ARGS = ["-d", "--no-show-raw-insn", "--x86-asm-syntax=intel"];
function parseDisasm(text: string): Insn[] {
    const out: Insn[] = [];
    for (const l of text.split("\n")) {
        let r = /^\s*([0-9a-f]+):\s+(.*)$/.exec(l);
        if (r && !/RELOC/.test(r[2])) {
            const [body, note] = r[2].split("##");
            out.push({ addr: parseInt(r[1], 16), text: body.replace(/\s+/g, " ").trim(), note: note?.trim() });
            continue;
        }
        r = /^\s+[0-9a-f]+:\s+\S*RELOC\S*\s+(\S+)/.exec(l);
        if (r && out.length) out[out.length - 1].reloc = r[1];
    }
    // llvm-objdump prints a lock prefix as its own line
    for (let i = out.length - 2; i >= 0; i--) {
        if (out[i].text === "lock" || out[i].text.startsWith("rep")) {
            if (out[i].text.split(" ").length === 1) { out[i + 1].text = out[i].text + " " + out[i + 1].text; out.splice(i, 1); }
        }
    }
    return out;
}

// ---- the original ----
const binBuf = readFileSync(binary);
const sha = createHash("sha256").update(binBuf).digest("hex");
if (sha !== BINARY_SHA256) { console.error(`${binary}: sha256 ${sha}, expected ${BINARY_SHA256}`); process.exit(2); }
const cacheDir = join(HERE, "build");
mkdirSync(cacheDir, { recursive: true });
const disCache = join(cacheDir, `orig-${sha.slice(0, 8)}.dis`);
if (!existsSync(disCache)) writeFileSync(disCache, run(OBJDUMP, ["--macho", ...DIS_ARGS, binary]));
const origInsns = parseDisasm(readFileSync(disCache, "utf8"));
const origIndex = new Map<number, number>();
origInsns.forEach((x, i) => origIndex.set(x.addr, i));
const starts = run(OBJDUMP, ["--macho", "--function-starts", binary]).split("\n")
    .filter((l) => /^[0-9a-f]{8}$/.test(l.trim())).map((l) => parseInt(l, 16)).sort((a, b) => a - b);
const binSecs = machoSections(binBuf);
const binIndirect = indirectSymbols(binary);
function nextStart(a: number): number {
    let lo = 0, hi = starts.length;
    while (lo < hi) { const m = (lo + hi) >> 1; if (starts[m] <= a) lo = m + 1; else hi = m; }
    return lo < starts.length ? starts[lo] : Infinity;
}

// ---- our sources: name <-> address ----
interface Entry { name: string; addr: number; file: string; defined: boolean }
const entries: Entry[] = [];
const addrName = new Map<number, string>();
function walk(d: string): string[] {
    if (!existsSync(d)) return [];
    return readdirSync(d).flatMap((f) => { const p = join(d, f); return statSync(p).isDirectory() ? walk(p) : [p]; });
}
for (const root of SOURCE_ROOTS) {
    for (const file of walk(join(REPO, root)).filter((f) => /\.(cpp|c|h|hpp|mm)$/i.test(f)).sort()) {
        let pend: number | null = null;
        const text = readFileSync(file, "utf8");
        if (!text.includes("1.14d mac:")) continue;
        const lines = text.split("\n");
        for (let li = 0; li < lines.length; li++) {
            const l = lines[li];
            const m = /^\s*\/\/ 1\.14d mac: ([0-9a-f]{8})\b(?:\s+(\S+))?/.exec(l);
            if (m && m[2] && /[<:~]|^_GLOBAL__/.test(m[2])) {
                // an explicit key: a template instantiation or a ctor/dtor variant this file's object holds
                entries.push({ name: m[2], addr: parseInt(m[1], 16), file: relative(REPO, file), defined: true });
                addrName.set(parseInt(m[1], 16), m[2]);
                pend = null;
                continue;
            }
            if (m) { pend = parseInt(m[1], 16); continue; }
            if (pend === null || /^\s*(\/\/|$)/.test(l)) continue;
            // a function-pointer global, "void (*name)(...)", is named by its declarator
            const n = /^[^(]*\(\s*\*\s*([A-Za-z_]\w*)\s*\)\s*\(/.exec(l) ??
                /([A-Za-z_]\w*)\s*(\(|\[|=|;|\{)/.exec(l.replace(/^\s*(static|extern|const|inline)\s+/g, ""));
            if (n) {
                // a definition reaches "{" (or "=" for data) before ";"; a declaration does not
                const rest = lines.slice(li, li + 12).join("\n");
                const semi = rest.indexOf(";"), brace = rest.search(/[{=]/);
                const defined = brace >= 0 && (semi < 0 || brace < semi) && !/^\s*extern\b/.test(l);
                entries.push({ name: n[1], addr: pend, file: relative(REPO, file), defined });
                addrName.set(pend, n[1]);
            }
            pend = null;
        }
    }
}
const nameAddr = new Map(entries.map((e) => [e.name, e.addr]));

// Whether a tag's definition has C linkage: its object defines "_name". A tag with no definition
// here (a declaration only) or no object is taken as C, as before.
const cDefs = new Map<string, Set<string> | null>();
function cLinkageTag(name: string): boolean {
    const defs = entries.filter((e) => e.name === name && e.defined);
    if (!defs.length) return true;
    return defs.some((e) => {
        const o = join(buildDir, "CMakeFiles/d2_mac_objs.dir", e.file + ".o");
        if (!cDefs.has(o)) cDefs.set(o, existsSync(o) ? new Set(run(NM, ["--defined-only", "-j", o]).split("\n")) : null);
        const set = cDefs.get(o);
        return set === null || set!.has("_" + name);
    });
}
// ---- one object ----
interface ObjInfo { funcs: Map<string, Insn[]>; syms: { addr: number; name: string }[]; buf: Buffer; secs: Section[]; indirect: Map<number, string>; relocs: number[] }
const demangleCache = new Map<string, string>();
function baseName(sym: string): string {
    let d = demangleCache.get(sym);
    if (d === undefined) {
        d = run(CXXFILT, ["-_"], sym + "\n").trim();
        // an unmangled C-linkage name (a global variable) comes back as is: drop the Mach-O underscore
        if (d === sym && sym.startsWith("_") && !sym.startsWith("__Z")) d = sym.slice(1);
        demangleCache.set(sym, d);
    }
    // globalopt shrinks a static bool to i1 and renames it "<name>.b", demangled "<name> (.b)"
    return d.replace(/ \(\.b\)$/, "").replace(/\(.*$/s, "").replace(/^.*::/, "").replace(/^.*\s/, "");
}
// The qualified name without the parameter list and spaces, "TSLink<FileLocation>::Unlink"; a
// constructor or destructor gets its variant, "TSHashTable<A,B>::~TSHashTable[D0]". A source names a
// template instantiation by this key on its address comment, since the plain name is ambiguous.
const keyCache = new Map<string, string>();
function qualKey(sym: string): string {
    let k = keyCache.get(sym);
    if (k !== undefined) return k;
    baseName(sym);
    let d = demangleCache.get(sym)!;
    if (d.endsWith(")")) {
        let depth = 0, i = d.length - 1;
        for (; i >= 0; i--) { if (d[i] === ")") depth++; else if (d[i] === "(" && --depth === 0) break; }
        if (i > 0) d = d.slice(0, i);
    }
    k = d.replace(/\s+/g, "");
    const v = /([CD][0-3])E\w*$/.exec(sym);
    const last = k.replace(/^.*::/, ""), cls = k.replace(/::[^:]*$/, "").replace(/<.*$/, "").replace(/^.*::/, "");
    if (v && (last.startsWith("~") || last === cls)) k += `[${v[1]}]`;
    keyCache.set(sym, k);
    return k;
}
// The name a symbol of ours is paired by: its qualified key when a source names that, else its plain name.
// Our symbol's tag name, spelled as the original side spells that address: one address tagged
// under two names (a declaration in a second file under another name) is the same function or global.
const canonical = (n: string): string => addrName.get(nameAddr.get(n)!) ?? n;
const known = (sym: string): string | null => {
    const q = qualKey(sym);
    if (nameAddr.has(q)) return canonical(q);
    const b = baseName(sym);
    return nameAddr.has(b) ? canonical(b) : null;
};
function loadObject(path: string): ObjInfo {
    const insns = parseDisasm(run(OBJDUMP, [...DIS_ARGS, "-r", path]));
    const syms = run(NM, ["-n", "--defined-only", path]).split("\n")
        .map((l) => /^([0-9a-f]{8}) \w (\S+)$/.exec(l)).filter((r): r is RegExpExecArray => !!r)
        .map((r) => ({ addr: parseInt(r[1], 16), name: r[2] }));
    const labels = run(OBJDUMP, [...DIS_ARGS, path]).split("\n")
        .map((l) => /^([0-9a-f]+) <(\S+)>:$/.exec(l)).filter((r): r is RegExpExecArray => !!r)
        .map((r) => ({ addr: parseInt(r[1], 16), name: r[2] }));
    const funcs = new Map<string, Insn[]>();
    labels.forEach((lb, i) => {
        const end = i + 1 < labels.length ? labels[i + 1].addr : Infinity;
        const body = insns.filter((x) => x.addr >= lb.addr && x.addr < end);
        funcs.set(baseName(lb.name), body);
        funcs.set(qualKey(lb.name), body);
    });
    const buf = readFileSync(path);
    const secs = machoSections(buf);
    return { funcs, syms, buf, secs, indirect: indirectSymbols(path), relocs: textRelocs(path, secs) };
}
// Where code carries a relocation: a PIC data reference in an object always has one. A relocation's
// address is an offset into its section.
function textRelocs(path: string, secs: Section[]): number[] {
    const out: number[] = [];
    let base: number | null = null;
    for (const l of run(OBJDUMP, ["--macho", "-r", path]).split("\n")) {
        const h = /^Relocation information \(__TEXT,(__(?:text|textcoal_nt|StaticInit))\)/.exec(l);
        if (/^Relocation information/.test(l)) { base = h ? secs.find((x) => x.name === h[1])?.addr ?? null : null; continue; }
        const r = /^([0-9a-f]{8})\s/.exec(l);
        if (base !== null && r) out.push(base + parseInt(r[1], 16));
    }
    return out;
}

// ---- normalisation ----
type Resolve = (target: number) => string | null;
function normalise(insns: Insn[], inText: (a: number) => boolean, resolveCode: Resolve, resolveData: Resolve,
    resolveCall: (x: Insn, target: number) => string | null, readU32: (a: number) => number,
    relocated?: (at: number, next: number) => boolean) {
    const index = new Map(insns.map((x, i) => [x.addr, i]));
    const pic = new Map<string, number>();
    const picSlots = new Map<string, number>();
    let unverified = 0;
    // A switch's jump table sits in __text after the code; what follows its start is data.
    let tableAt = Infinity;
    const out = insns.map((x, i) => {
        let t = x.text;
        const jt = /^mov (e\w\w), dword ptr \[(e\w\w) \+ 4\*(e\w\w) \+ (0x[0-9a-f]+)\]$/.exec(t);
        if (jt && pic.has(jt[2])) {
            const base = pic.get(jt[2])!, table = base + parseInt(jt[4], 16);
            let n = -1;
            for (let j = i - 1; j >= 0 && n < 0; j--) {
                const c = new RegExp(`^cmp ${jt[3]}, (0x[0-9a-f]+|\\d+)$`).exec(insns[j].text);
                if (c) n = Number(c[1]) + 1;
            }
            if (n < 0 || n > 4096) return t;
            tableAt = Math.min(tableAt, table);
            const ks = Array.from({ length: n }, (_, k) => {
                const ix = index.get((base + readU32(table + 4 * k)) >>> 0);
                return ix === undefined ? "out" : "#" + ix;
            });
            return `mov ${jt[1]}, jumptable[${ks.join(",")}]`;
        }
        const call = /^call (0x[0-9a-f]+)/.exec(t);
        if (call) {
            const target = parseInt(call[1], 16);
            if (i + 1 < insns.length && target === insns[i + 1].addr) return "call $+5";
            const who = resolveCall(x, target);
            if (who === null) unverified++;
            return "call " + (who ?? "?");
        }
        const pop = /^pop (e\w\w)$/.exec(t);
        const afterCallNext = i > 0 && new RegExp(`^call 0x${x.addr.toString(16)}\\b`).test(insns[i - 1].text);
        const br = /^(j\w+|loop\w*) (0x[0-9a-f]+)/.exec(t);
        if (br) {
            const k = index.get(parseInt(br[2], 16));
            // A tail call leaves the function. In an object its target is not linked yet (0 for
            // the section's first function, which reads as a jump to its own start), so a jmp
            // that carries a relocation is named by it, as a call is, and leaves ("out") when
            // the callee is not one of ours.
            const next = i + 1 < insns.length ? insns[i + 1].addr : x.addr + 16;
            if (br[1] === "jmp" && (relocated ? relocated(x.addr, next) : k === undefined)) {
                const who = resolveCall(x, parseInt(br[2], 16));
                return "jmp " + (who ?? "out");
            }
            return `${br[1]} ${k === undefined ? "out" : "#" + k}`;
        }
        // The pic base stays with its register for the rest of the function: the scan is linear
        // and a write on one path says nothing about another. A reference resolves only when it
        // lands outside __text, so a register reused for something else stays as written.
        if (pop && afterCallNext) { pic.set(pop[1], x.addr); return t; }
        // A pic base spilled to a frame slot and loaded back into another register.
        const spill = /^mov dword ptr \[ebp - (0x[0-9a-f]+)\], (e\w\w)$/.exec(t);
        if (spill && pic.has(spill[2])) picSlots.set(spill[1], pic.get(spill[2])!);
        const reload = /^mov (e\w\w), dword ptr \[ebp - (0x[0-9a-f]+)\]$/.exec(t);
        if (reload && picSlots.has(reload[2])) { pic.set(reload[1], picSlots.get(reload[2])!); return t; }
        // In an object, an operand off a register that no longer holds the pic base has no relocation.
        if (relocated && !relocated(x.addr, i + 1 < insns.length ? insns[i + 1].addr : x.addr + 16)) return t;
        // [pic + disp], or [r + pic + disp] for an array the pic-relative address indexes
        t = t.replace(/\[(e\w\w)(?: \+ (e\w\w))? ([+-]) (0x[0-9a-f]+)\]/g, (m, r1: string, r2: string | undefined, sign: string, disp: string) => {
            const reg = r2 !== undefined && pic.has(r2) ? r2 : r1;
            const index = r2 === undefined ? "" : (reg === r1 ? r2 : r1) + " + ";
            const base = pic.get(reg);
            if (base === undefined) return m;
            const target = (base + (sign === "-" ? -1 : 1) * parseInt(disp, 16)) >>> 0;
            if (inText(target)) {
                const fn = resolveCode(target);
                return fn === null ? m : `[${index}PIC fn ${fn}]`;
            }
            const what = resolveData(target);
            if (what === null) { unverified++; return `[${index}PIC ?]`; }
            return `[${index}PIC ${what}]`;
        });
        return t;
    });
    return { out: out.filter((_, i) => insns[i].addr < tableAt), unverified };
}
const stripTail = (a: string[]) => { while (a.length && /^(nop|int3|xchg ax, ax)/.test(a[a.length - 1])) a.pop(); return a; };

const insnLog: string[] = [];
function compareOne(e: Entry, obj: ObjInfo) {
    const ours = obj.funcs.get(e.name);
    if (!ours) return { status: "MISSING", detail: "not in the object (inlined or not compiled)", unverified: 0, n: 0 };
    const end = nextStart(e.addr);
    const i0 = origIndex.get(e.addr);
    let orig: Insn[] = [];
    if (i0 !== undefined) {
        for (let i = i0; i < origInsns.length && origInsns[i].addr < end; i++) orig.push(origInsns[i]);
    }
    if (i0 === undefined || (orig.length && orig[orig.length - 1].addr + 16 < end && end !== Infinity)) {
        // The linear sweep lost step (data in __text before it): disassemble the function alone.
        orig = parseDisasm(run(OBJDUMP, [...DIS_ARGS, `--start-address=0x${e.addr.toString(16)}`,
            `--stop-address=0x${end.toString(16)}`, binary])).filter((x) => x.addr >= e.addr);
        if (!orig.length) return { status: "BADADDR", detail: "no instruction at that address", unverified: 0, n: 0 };
    }

    const origRes = normalise(orig, (a) => binSecs.some((x) => x.name === "__text" && a >= x.addr && a < x.addr + x.size),
        (a) => addrName.get(a) ?? null,
        (a) => {
            const imp = binIndirect.get(a);
            if (imp === "LOCAL") {
                // a pointer to a global of the image itself: the slot holds its address
                const n = addrName.get(readU32At(binBuf, binSecs, a));
                return n === undefined ? null : "&" + n;
            }
            if (imp) return "&" + imp;
            // a paired global in __TEXT,__const would otherwise read as a Pascal string
            const exact = addrName.get(a);
            if (exact !== undefined && !entries.find((x) => x.addr === a && obj.funcs.has(x.name))) return `${exact}+0`;
            const s = cstringAt(binBuf, binSecs, a) ?? pascalAt(binBuf, binSecs, a);
            if (s) return s;
            // our globals: the nearest named address at or below, within 64 KB
            let best: number | null = null;
            for (const ad of addrName.keys()) if (ad <= a && a - ad < 0x10000 && (best === null || ad > best)) best = ad;
            const isData = (ad: number) => nameAddr.has(addrName.get(ad)!) && !entries.find((x) => x.addr === ad && obj.funcs.has(x.name));
            // just below one of our globals: an array indexed from 1 (&a[i - 1]); the nearer of the two wins,
            // since the padding between globals differs between the binary and our object
            let above: number | null = null;
            for (const ad of addrName.keys()) if (ad > a && ad - a <= 0x40 && (above === null || ad < above)) above = ad;
            if (above !== null && isData(above) && (best === null || !isData(best) || above - a < a - best))
                return `${addrName.get(above)}-${(above - a).toString(16)}`;
            return best !== null && isData(best) ? `${addrName.get(best)}+${(a - best).toString(16)}` : null;
        },
        (x, target) => {
            const stub = /symbol stub for: (\S+)/.exec(x.note ?? "");
            if (stub) return stub[1];
            const imp = binIndirect.get(target);
            if (imp) return imp;
            const n = addrName.get(target);
            return n ?? null;
        },
        (a) => readU32At(binBuf, binSecs, a));
    // clang puts a weak function (a template instantiation, an inline member) in __textcoal_nt, a
    // file's static initialiser in __StaticInit
    const ourRes = normalise(ours, (a) => obj.secs.some((x) => /^__(text|textcoal_nt|StaticInit)$/.test(x.name) && a >= x.addr && a < x.addr + x.size),
        (a) => {
            const s2 = obj.syms.find((x) => x.addr === a);
            return s2 ? known(s2.name) : null;
        },
        (a) => {
            const imp = obj.indirect.get(a);
            if (imp) return "&" + (known(imp) ?? imp);
            const exact = obj.syms.find((s2) => s2.addr === a && !/^[Ll]/.test(s2.name) && known(s2.name));
            if (exact) return `${known(exact.name)}+0`;
            const s = cstringAt(obj.buf, obj.secs, a) ?? pascalAt(obj.buf, obj.secs, a);
            if (s) return s;
            let best: { addr: number; name: string } | null = null;
            for (const s2 of obj.syms) if (s2.addr <= a) best = s2;
            const below = best && !best.name.startsWith("L") && !best.name.startsWith("l") && known(best.name) ? best : null;
            const above = obj.syms.find((s2) => s2.addr > a && s2.addr - a <= 0x40 && !/^[Ll]/.test(s2.name));
            if (above && known(above.name) && (!below || above.addr - a < a - below.addr))
                return `${known(above.name)}-${(above.addr - a).toString(16)}`;
            return below ? `${known(below.name)}+${(a - below.addr).toString(16)}` : null;
        },
        (x, target) => {
            const sym = x.reloc && !/^__(text|textcoal_nt|StaticInit)$/.test(x.reloc) ? x.reloc : obj.syms.find((s) => s.addr === target)?.name;
            if (!sym) return null;
            if (sym.startsWith("__Z") || !sym.startsWith("_")) return known(sym);
            // an extern "C" symbol: the stub name is the symbol itself. It is our tag only when the
            // tagged definition has C linkage too: Win32Mac's ShowWindow is C++, and "_ShowWindow"
            // is Carbon's, an import.
            return nameAddr.has(sym.slice(1)) && cLinkageTag(sym.slice(1)) ? canonical(sym.slice(1)) : sym;
        },
        (a) => readU32At(obj.buf, obj.secs, a),
        (at, next) => obj.relocs.some((r) => r > at && r < next));
    const a = stripTail(origRes.out), b = stripTail(ourRes.out);
    // unresolved on one side only: accept, but count
    let unverified = 0;
    const unverifiedAt: string[] = [];
    let k = 0;
    for (; k < Math.min(a.length, b.length); k++) {
        if (a[k] === b[k]) continue;
        const wa = a[k].replace(/\[PIC [^\]]*\]|call \S+$/g, "?"), wb = b[k].replace(/\[PIC [^\]]*\]|call \S+$/g, "?");
        if (wa === wb && (/\?/.test(a[k]) || /\?/.test(b[k]))) { unverified++; unverifiedAt.push(`    ${k} orig '${a[k]}' ours '${b[k]}'`); continue; }
        break;
    }
    if (k === a.length && k === b.length) {
        return { status: "SAME", detail: verbose && unverifiedAt.length ? "\n" + unverifiedAt.join("\n") : "", unverified, n: b.length };
    }
    const ctx = (arr: string[]) => (k < arr.length ? arr[k] : "<end>");
    let detail = `insn ${k}/${a.length}(orig) ${b.length}(ours): ours '${ctx(b)}' orig '${ctx(a)}'`;
    if (verbose) {
        const lo = Math.max(0, k - 3), hi = Math.max(a.length, b.length);
        detail += "\n" + Array.from({ length: hi - lo }, (_, j) => j + lo)
            .map((j) => `   ${j.toString().padStart(4)} ${(a[j] ?? "").padEnd(48)} ${b[j] ?? ""}${a[j] === b[j] ? "" : "   <"}`).join("\n");
    }
    if (insnsOut) insnLog.push(JSON.stringify({ addr: e.addr.toString(16).padStart(8, "0"), name: e.name, orig: a, ours: b }));
    return { status: "DIFF", detail, unverified, n: b.length };
}

// ---- run ----
const byFile = new Map<string, Entry[]>();
for (const e of entries) {
    if (!/\.(cpp|c|mm)$/i.test(e.file) || !e.defined) continue;
    if (filters.length && !filters.some((f) => e.file.includes(f) || e.name === f)) continue;
    if (!byFile.has(e.file)) byFile.set(e.file, []);
    byFile.get(e.file)!.push(e);
}
const rows: string[] = ["file\taddr\tname\tstatus\tinsns\tunverified\tdetail"];
let same = 0, total = 0;
const noObject: string[] = [];
const pct = (n: number, d: number) => (d ? (100 * n / d).toFixed(1) : "0.0") + "%";
for (const [file, es] of byFile) {
    const objPath = join(buildDir, "CMakeFiles/d2_mac_objs.dir", file + ".o");
    if (!existsSync(objPath)) {
        noObject.push(file);
        if (verbose) console.log(`${file}: no object at ${objPath}`);
        for (const e of es) rows.push([file, e.addr.toString(16).padStart(8, "0"), e.name, "NOOBJ", 0, 0, "not built by d2_mac_objs"].join("\t"));
        continue;
    }
    const obj = loadObject(objPath);
    let fs = 0, ft = 0;
    for (const e of es) {
        if (!obj.funcs.has(e.name) && !starts.includes(e.addr)) continue;  // a global, not a function
        const r = compareOne(e, obj);
        ft++; total++;
        if (r.status === "SAME") { fs++; same++; }
        const unv = r.unverified ? ` (${r.unverified} unverified refs)` : "";
        console.log(`${e.addr.toString(16).padStart(8, "0")} ${e.name} ${r.status}${r.n ? ` ${r.n} insns` : ""}${unv}${r.detail ? " " + r.detail : ""}`);
        rows.push([file, e.addr.toString(16).padStart(8, "0"), e.name, r.status, r.n, r.unverified, r.detail.split("\n")[0]].join("\t"));
    }
    console.log(`== ${file}: ${fs}/${ft} same (${pct(fs, ft)})`);
}
if (noObject.length) console.log(`== ${noObject.length} tagged files have no Mac object (not built by d2_mac_objs; -v lists them)`);
console.log(`== total ${same}/${total} instruction-identical (${pct(same, total)})`);
if (tsvOut) writeFileSync(tsvOut, rows.join("\n") + "\n");
if (insnsOut) writeFileSync(insnsOut, insnLog.join("\n") + "\n");
