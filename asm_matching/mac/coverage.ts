// Measures how much of the 1.14d Intel Mac build's own code has source here, function by function.
//
//   node asm_matching/mac/coverage.ts [--cmp <cmp.ts --tsv output, default build/cmp.tsv>] [--out <tsv>] [-v]
//   node asm_matching/mac/coverage.ts --import <ghidra dump> --docs <docs/data dir>
//
// Every function of DiabloII_macho inside the Mac-only regions (coverage/units.tsv), and every unpaired
// function outside them that coverage/unpaired.tsv classifies, gets one row: its
// original source file, the path that file has here, a class (blizzard-mac, third-party, shared,
// shared-unpaired) and a status: missing, body (a definition tagged "// 1.14d mac: <addr>" exists),
// match or nonmatch (cmp.ts's SAME or DIFF, read from its --tsv output). The summary is the coverage,
// blizzard-mac functions with a body over all blizzard-mac functions, and the match count.
//
// Nothing here reads Ghidra: the clean program's functions are cached in coverage/functions.tsv, which
// --import rebuilds from a fresh read-only dump (coverage/dump.java) and the pairing tables. See README.md.
import { existsSync, readdirSync, readFileSync, statSync, writeFileSync } from "node:fs";
import { dirname, join, relative, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = resolve(HERE, "../..");
const FUNCTIONS = join(HERE, "coverage/functions.tsv");
const UNITS = join(HERE, "coverage/units.tsv");
const REASONS = join(HERE, "reasons.tsv");
const UNPAIRED = join(HERE, "coverage/unpaired.tsv");
// where a "// 1.14d mac:" tag may stand: the Mac trees and the shared code's D2_OS_MAC blocks
const SOURCE_ROOTS = ["Source", "Redivivus"];

const argv = process.argv.slice(2);
// cmp.ts --tsv writes here by the README's recipe; build/ is not checked in
let cmpTsv = existsSync(join(HERE, "build/cmp.tsv")) ? join(HERE, "build/cmp.tsv") : "", out = join(HERE, "coverage.tsv"), importDump = "", docsDir = "", verbose = false;
for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    if (a === "--cmp") cmpTsv = resolve(argv[++i]);
    else if (a === "--out") out = resolve(argv[++i]);
    else if (a === "--import") importDump = resolve(argv[++i]);
    else if (a === "--docs") docsDir = resolve(argv[++i]);
    else if (a === "-v") verbose = true;
    else { console.error(`unknown argument ${a}`); process.exit(2); }
}

const hex = (n: number) => n.toString(16).padStart(8, "0");
const readTsv = (path: string) => readFileSync(path, "utf8").split("\n").filter((l) => l && !l.startsWith("#")).map((l) => l.split("\t"));

// ---- --import: rebuild coverage/functions.tsv ----
// The dump is dump.java's output: "#program" line, then addr, end, bytes, thunk, name, file_literal.
// The pairing comes from docs/data: mac-win-pairs.tsv (the Windows twin) and mac-win-unpaired.tsv (how
// many body candidates the pairer found on Windows for an unpaired function), and mac-file-map.tsv
// (its real_path where the tier is own-file, bracketed or pair at certain or high confidence: for a
// paired function that is its Windows twin's file).
if (importDump) {
    if (!docsDir) { console.error("--import needs --docs <docs/data>"); process.exit(2); }
    const lines = readFileSync(importDump, "utf8").split("\n");
    const program = lines.find((l) => l.startsWith("#program")) ?? "#program ?";
    const twin = new Map<string, string>();
    for (const r of readTsv(join(docsDir, "mac-win-pairs.tsv")).slice(1)) twin.set(r[1], r[0]);
    const cand = new Map<string, string>();
    for (const r of readTsv(join(docsDir, "mac-win-unpaired.tsv")).slice(1)) if (r[1] === "mac") cand.set(r[2], r[6]);
    const fileMap = new Map<string, string>();
    for (const r of readTsv(join(docsDir, "mac-file-map.tsv")).slice(1)) {
        if (r[3] && /^(own-file|bracketed|pair)$/.test(r[5]) && /^(certain|high)$/.test(r[6])) fileMap.set(r[0], r[3]);
    }
    const rows = ["addr\tend\tbytes\tname\tfile_literal\twin_twin\twin_candidates\tfile_map"];
    for (const l of lines.slice(1)) {
        const r = l.split("\t");
        if (r.length < 6 || r[0] === "addr" || r[3] === "T") continue;  // header, thunks (import stubs)
        rows.push([r[0], r[1], r[2], r[4], r[5], twin.get(r[0]) ?? "", twin.has(r[0]) ? "" : (cand.get(r[0]) ?? ""), fileMap.get(r[0]) ?? ""].join("\t"));
    }
    writeFileSync(FUNCTIONS, [
        `# The clean Mac program's functions, read-only dump (coverage/dump.java) of ${program.slice(9)}.`,
        "# Thunks dropped. win_twin: the Windows Game.exe twin from docs/data/mac-win-pairs.tsv;",
        "# win_candidates: for an unpaired function, cand_n from docs/data/mac-win-unpaired.tsv;",
        "# file_map: real_path from docs/data/mac-file-map.tsv at tiers own-file/bracketed/pair, certain or high.",
        ...rows].join("\n") + "\n");
    console.log(`${FUNCTIONS}: ${rows.length - 1} functions`);
    process.exit(0);
}

// ---- inputs ----
interface Fn { addr: number; bytes: number; name: string; literals: string[]; twin: string; cands: number; fileMap: string }
const fns: Fn[] = readTsv(FUNCTIONS).slice(1).map((r) => ({
    addr: parseInt(r[0], 16), bytes: Number(r[2]), name: r[3],
    // a __FILE__ path below Diablo2/Source; a header's literal says nothing of the unit
    literals: (r[4] ?? "").split("|").filter((s) => s && !/\.(h|hpp|H)$/.test(s)),
    twin: r[5] ?? "", cands: Number(r[6] || 0), fileMap: r[7] ?? "",
}));
interface Unit { lo: number; hi: number; kind: string; cls: string; file: string; redivivus: string; evidence: string }
const unitRows = readTsv(UNITS).slice(1).map((r): Unit => ({
    lo: parseInt(r[0], 16), hi: r[1] ? parseInt(r[1], 16) : parseInt(r[0], 16) + 1, kind: r[2], cls: r[3],
    file: r[4], redivivus: r[5] ?? "", evidence: r[6] ?? "",
}));
const units = unitRows.filter((u) => u.kind !== "override");
const overrides = new Map(unitRows.filter((u) => u.kind === "override").map((u) => [u.lo, u]));

// ---- our sources: the address tags, as cmp.ts reads them ----
interface Tag { addr: number; file: string; defined: boolean }
const tags = new Map<number, Tag[]>();
function walk(d: string): string[] {
    if (!existsSync(d)) return [];
    return readdirSync(d).flatMap((f) => { const p = join(d, f); return statSync(p).isDirectory() ? walk(p) : [p]; });
}
for (const root of SOURCE_ROOTS) {
    for (const file of walk(join(REPO, root)).filter((f) => /\.(cpp|c|h|hpp|mm|m)$/i.test(f)).sort()) {
        const text = readFileSync(file, "utf8");
        if (!text.includes("1.14d mac:")) continue;
        const lines = text.split("\n");
        const isSource = /\.(cpp|c|mm|m)$/i.test(file);
        for (let li = 0; li < lines.length; li++) {
            const m = /^\s*\/\/ 1\.14d mac: ([0-9a-f]{8})\b(?:\s+(\S+))?/.exec(lines[li]);
            if (!m) continue;
            const addr = parseInt(m[1], 16);
            if (!tags.has(addr)) tags.set(addr, []);
            // an explicit key names a template instantiation or a ctor/dtor variant the file's object holds
            if (m[2] && /[<:~]|^_GLOBAL__/.test(m[2])) { tags.get(addr)!.push({ addr, file: relative(REPO, file), defined: isSource }); continue; }
            // the definition follows; a declaration reaches ";" before "{" or "="
            let k = li + 1;
            while (k < lines.length && /^\s*(\/\/|$)/.test(lines[k])) k++;
            const rest = lines.slice(k, k + 12).join("\n");
            const semi = rest.indexOf(";"), brace = rest.search(/[{=]/);
            const defined = isSource && brace >= 0 && (semi < 0 || brace < semi) && !/^\s*extern\b/.test(lines[k] ?? "");
            tags.get(addr)!.push({ addr, file: relative(REPO, file), defined });
        }
    }
}

// ---- cmp.ts results ----
const cmp = new Map<number, string>();
const unverified = new Set<number>();  // SAME, but with references only one side could resolve
// reasons.tsv: why a function that has a body is not instruction-identical (addr, name, reason_class, detail)
const reasons = new Map<number, string>();
if (existsSync(REASONS)) for (const r of readTsv(REASONS).slice(1)) if (r[0]) reasons.set(parseInt(r[0], 16), r[2]);
if (cmpTsv) {
    for (const r of readTsv(cmpTsv).slice(1)) { cmp.set(parseInt(r[1], 16), r[3]); if (r[3] === "SAME" && Number(r[5]) > 0) unverified.add(parseInt(r[1], 16)); }
}

// ---- the original path here: Source/ plus the original with its spaces dropped ----
function redivivusPath(orig: string): string {
    if (orig === "?") return "?";
    return "Source/" + orig.replace(/^StormMac\/Storm Mac\//, "StormMac/").replace(/ /g, "");
}

// ---- one row per function in a unit ----
interface Row { reason: string; fn: Fn; unit: Unit; file: string; fileEvidence: string; cls: string; redivivus: string; status: string; bodyFile: string; atPath: string }
const rows: Row[] = [];
for (const u of units) {
    const inUnit = fns.filter((f) => f.addr >= u.lo && f.addr < u.hi);
    // anchors: a function's own __FILE__ literal, else the file mac-file-map gives it (for a paired
    // function, its Windows twin's file; inside the Mac Storm tree that is the same file of StormMac)
    const literal = (f: Fn) => (f.literals.length ? f.literals.find((s) => u.file !== "?" && s.startsWith(u.file.replace(/\?$/, ""))) ?? f.literals[0] : null);
    const mapped = (f: Fn) => {
        if (!f.fileMap || /\.(h|hpp)$/i.test(f.fileMap)) return null;
        return u.file.startsWith("StormMac/") ? f.fileMap.replace(/^Storm\/SOURCE\//, "StormMac/Storm Mac/SOURCE/") : f.fileMap;
    };
    const anchorFile = (f: Fn) => literal(f) ?? mapped(f);
    inUnit.forEach((f, i) => {
        let file = u.file, ev = u.kind === "fixed" ? "unit" : "";
        if (u.kind === "anchors") {
            const own = anchorFile(f);
            if (own) { file = own; ev = literal(f) ? "__FILE__" : "file-map"; }
            else {
                let before: string | null = null, after: string | null = null;
                for (let j = i - 1; j >= 0 && !before; j--) before = anchorFile(inUnit[j]);
                for (let j = i + 1; j < inUnit.length && !after; j++) after = anchorFile(inUnit[j]);
                if (before && before === after) { file = before; ev = "bracketed"; }
                else { file = `${before ?? u.file}|${after ?? u.file}`; ev = "between"; }
            }
        }
        let cls = u.cls;
        if (cls === "shared") cls = f.twin ? "shared" : f.cands > 0 ? "shared-unpaired" : "blizzard-mac";
        const ov = overrides.get(f.addr);
        if (ov) { cls = ov.cls; file = ov.file; ev = "override"; }
        const red = cls === "third-party" ? "-" : (ov ?? u).redivivus || file.split("|").map(redivivusPath).join("|");
        const ts = tags.get(f.addr) ?? [];
        const def = ts.find((t) => t.defined);
        let status = def ? "body" : "missing";
        const c = cmp.get(f.addr);
        if (def && c === "SAME") status = "match";
        else if (def && c === "DIFF") status = "nonmatch";
        const atPath = !def ? "-" : red.split("|").includes(def.file) ? "yes" : "no";
        rows.push({ reason: reasons.get(f.addr) ?? "", fn: f, unit: u, file, fileEvidence: ev, cls, redivivus: red, status, bodyFile: def?.file ?? "", atPath });
    });
}

// ---- functions outside the Mac-only regions with no Windows twin (coverage/unpaired.tsv) ----
// Each was classified by hand against the Mac and Windows programs: mac-only (no Windows equivalent,
// counted as blizzard-mac), shared-unpaired (a Windows function does the same, or VC inlined it),
// third-party, compiler (thunks, stubs, static initialisers). Anything not listed there is not a row.
const unpairedClass: Record<string, string> = { "mac-only": "blizzard-mac", "shared-unpaired": "shared-unpaired", "third-party": "third-party", "compiler": "compiler" };
const seen = new Set(rows.map((r) => r.fn.addr));
const unpaired = new Map<number, { cls: string; win: string; evidence: string }>();
if (existsSync(UNPAIRED)) for (const r of readTsv(UNPAIRED).slice(1)) {
    if (!unpairedClass[r[1]]) { console.error(`unpaired.tsv: unknown class ${r[1]} for ${r[0]}`); process.exit(2); }
    unpaired.set(parseInt(r[0], 16), { cls: unpairedClass[r[1]], win: r[2] ?? "", evidence: r[3] ?? "" });
}
for (const f of fns) {
    const u = unpaired.get(f.addr);
    if (!u || seen.has(f.addr) || f.twin) continue;
    // the file: its own __FILE__ literal (not a header), else the file map, else unknown
    const file = f.literals[0] ?? (f.fileMap && !/\.(h|hpp)$/i.test(f.fileMap) ? f.fileMap : "?");
    const ev = f.literals.length ? "__FILE__" : file !== "?" ? "file-map" : "";
    const red = u.cls === "third-party" || u.cls === "compiler" ? "-" : redivivusPath(file);
    const def = (tags.get(f.addr) ?? []).find((t) => t.defined);
    let status = def ? "body" : "missing";
    const c = cmp.get(f.addr);
    if (def && c === "SAME") status = "match";
    else if (def && c === "DIFF") status = "nonmatch";
    const atPath = !def ? "-" : red.split("|").includes(def.file) ? "yes" : "no";
    const unit: Unit = { lo: f.addr, hi: f.addr + 1, kind: "unpaired", cls: u.cls, file, redivivus: red, evidence: u.evidence };
    rows.push({ reason: reasons.get(f.addr) ?? "", fn: { ...f, twin: u.win }, unit, file, fileEvidence: ev || "unpaired", cls: u.cls, redivivus: red, status, bodyFile: def?.file ?? "", atPath });
}
rows.sort((a, b) => a.fn.addr - b.fn.addr);

const header = ["addr", "name", "bytes", "orig_file", "file_evidence", "redivivus", "class", "status", "body_file", "at_path", "win_twin", "reason"];
writeFileSync(out, [header.join("\t"), ...rows.map((r) => [hex(r.fn.addr), r.fn.name, r.fn.bytes, r.file, r.fileEvidence,
    r.redivivus, r.cls, r.status, r.bodyFile, r.atPath, r.fn.twin, r.reason].join("\t"))].join("\n") + "\n");

// ---- summary ----
const pct = (n: number, d: number) => (d ? (100 * n / d).toFixed(1) : "0.0") + "%";
const has = (r: Row) => r.status !== "missing";
const byClass = new Map<string, Row[]>();
for (const r of rows) { if (!byClass.has(r.cls)) byClass.set(r.cls, []); byClass.get(r.cls)!.push(r); }
for (const [cls, rs] of [...byClass].sort()) {
    const bytes = rs.reduce((s, r) => s + r.fn.bytes, 0);
    console.log(`${cls.padEnd(16)} ${String(rs.length).padStart(5)} functions ${String(bytes).padStart(7)} bytes  body ${rs.filter(has).length}`);
}
const bm = byClass.get("blizzard-mac") ?? [];
const perFile = new Map<string, { n: number; body: number; match: number }>();
for (const r of bm) {
    const k = r.file;
    if (!perFile.has(k)) perFile.set(k, { n: 0, body: 0, match: 0 });
    const p = perFile.get(k)!;
    p.n++; if (has(r)) p.body++; if (r.status === "match") p.match++;
}
const files = [...perFile].sort((a, b) => (b[1].n - b[1].body) - (a[1].n - a[1].body) || a[0].localeCompare(b[0]));
for (const [f, p] of verbose ? files : files.slice(0, 15)) {
    console.log(`  ${String(p.body).padStart(4)}/${String(p.n).padEnd(4)} missing ${String(p.n - p.body).padStart(4)}  ${f}`);
}
if (!verbose && files.length > 15) console.log(`  ... ${files.length - 15} more files (-v lists them)`);
const misplaced = bm.filter((r) => r.atPath === "no").length;
const matches = rows.filter((r) => r.status === "match").length;
console.log(`== coverage ${bm.filter(has).length}/${bm.length} blizzard-mac functions with a body (${pct(bm.filter(has).length, bm.length)})` +
    (misplaced ? `, ${misplaced} of them outside the mapped path` : ""));
console.log(`== match ${matches} (${cmpTsv ? "from " + relative(REPO, cmpTsv) : "no cmp.ts output: match and nonmatch read as body"})`);
// a row that has a body but is not instruction-identical (nonmatch, not compared, or SAME with
// unverified references) must carry a reason from reasons.tsv
const nonIdentical = rows.filter((r) => r.status === "nonmatch" || r.status === "body" || (r.status === "match" && unverified.has(r.fn.addr)));
const noReason = nonIdentical.filter((r) => !r.reason);
console.log(`== nonmatch ${nonIdentical.length}, ${nonIdentical.length - noReason.length} with a reason, ${noReason.length} WITHOUT a reason`);
if (verbose) for (const r of noReason) console.log(`   no reason: ${hex(r.fn.addr)} ${r.fn.name} (${r.status})`);
const stale = [...reasons.keys()].filter((a) => !nonIdentical.some((r) => r.fn.addr === a));
if (stale.length) console.log(`== ${stale.length} reasons.tsv rows name a function that is now identical or has no body: ${stale.slice(0, 5).map(hex).join(" ")}${stale.length > 5 ? " ..." : ""}`);
console.log(`== wrote ${relative(process.cwd(), out)}: ${rows.length} rows`);
