// Link probe of the Mac build: how far is d2_mac_objs + d2_mac_shared_objs from linking into a
// Mac executable like the original DiabloII_macho?
//
//   node asm_matching/mac/link-probe.ts --build <mac build dir> [--out <dir>]
//
// Links every object of the three targets (d2_mac_link_deps holds the vendored third-party code and
// the link stubs, see cmake/mac.cmake) with the Xcode 4.6.2 ld (i386, 10.7 SDK, the frameworks the
// original links) and reads what ld says. Two links are needed because ld64 stops at the first
// failing phase: `-undefined error` yields the undefined symbols, `-undefined dynamic_lookup`
// lets it get as far as the duplicate-symbol pass. Nothing is written into the tree; the result
// is <out>/link-probe.tsv (one row per undefined symbol, with its class), <out>/duplicates.tsv and
// a summary on stdout. See README.md.
//
// Environment (defaults match cmake/mac-i386-clang425.cmake):
//   D2_MAC_PROBE  payload dir, default ~/code/re/diablo2/oracle/mac-probe (cli/Payload/usr/bin/ld, sdk107/)
//   D2_MAC_LD     the ld to run (x86_64, runs under Rosetta on Apple silicon)
//   D2_MAC_SDK    the MacOSX10.7.sdk
//
// Undefined-symbol classes:
//   a  third-party the Mac binary embeds (Bink, crashy, blast, zlib, MoreFiles) or that the shared
//      Windows code needs (Intel JPEG Library); d2_mac_link_deps defines all of them, so this
//      class is 0 unless a new reference appears
//   b  a function the Mac binary has (Ghidra name or FUN_ address) and the tree has no body for
//   b2 no Mac function of that name: Windows-only code the shared unit still calls, or unnamed
//   c  data or a global that is declared and never defined
//   d  the function is defined, under a different mangling (namespace, parameter types)
//   e  entry, startup or compiler/CRT runtime
import { execFileSync } from "node:child_process";
import { existsSync, mkdirSync, readdirSync, readFileSync, statSync, writeFileSync } from "node:fs";
import { homedir } from "node:os";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const probe = process.env.D2_MAC_PROBE ?? join(homedir(), "code/re/diablo2/oracle/mac-probe");
const ld = process.env.D2_MAC_LD ?? join(probe, "cli/Payload/usr/bin/ld");
// crt1.o: the executable's entry (start), which calls main as the original binary's entry does.
const crt1 = join(probe, "sdk/Payload/usr/lib/crt1.10.6.o");
const sdk = process.env.D2_MAC_SDK ?? join(probe, "sdk107/MacOSX10.7.sdk");
const LLVM = existsSync("/opt/homebrew/opt/llvm/bin") ? "/opt/homebrew/opt/llvm/bin/" : "";
const FRAMEWORKS = ["Carbon", "Security", "CoreAudio", "DrawSprocket", "OpenGL", "AGL", "ApplicationServices",
    "AudioToolbox", "AudioUnit", "Cocoa", "CoreFoundation", "QuickTime", "OpenAL", "ScriptingBridge",
    "CoreServices", "Foundation", "AppKit"];

const argv = process.argv.slice(2);
let buildDir = "";
let outDir = "";
for (let i = 0; i < argv.length; i++) {
    if (argv[i] === "--build") buildDir = resolve(argv[++i]);
    else if (argv[i] === "--out") outDir = resolve(argv[++i]);
}
if (!buildDir) {
    console.error("usage: node asm_matching/mac/link-probe.ts --build <mac build dir> [--out <dir>]");
    process.exit(2);
}
outDir ||= join(buildDir, "link-probe");
mkdirSync(outDir, { recursive: true });

// ---- the objects of both targets ----
function walk(dir: string, out: string[]) {
    for (const e of readdirSync(dir)) {
        const p = join(dir, e);
        if (statSync(p).isDirectory()) walk(p, out);
        else if (e.endsWith(".o")) out.push(p);
    }
}
const objs: string[] = [];
for (const t of ["d2_mac_objs", "d2_mac_shared_objs", "d2_mac_link_deps"]) {
    const d = join(buildDir, "CMakeFiles", `${t}.dir`);
    if (!existsSync(d)) { console.error(`no ${d}: build ${t} first`); process.exit(2); }
    walk(d, objs);
}
objs.sort();

function runLd(undefinedMode: string): string {
    const args = ["-arch", "i386", "-macosx_version_min", "10.7", "-syslibroot", sdk, "-undefined", undefinedMode,
        "-o", join(outDir, "probe.out"), ...(existsSync(crt1) ? [crt1] : []), ...objs];
    for (const f of FRAMEWORKS) args.push("-framework", f);
    args.push("-lc++", "-lSystem", "-lobjc");
    try {
        return execFileSync(ld, args, { encoding: "utf8", maxBuffer: 1 << 30, stdio: ["ignore", "pipe", "pipe"] });
    } catch (e: any) {
        return String(e.stdout ?? "") + String(e.stderr ?? "");
    }
}
function pipeLines(cmd: string, args: string[], input: string): string[] {
    return execFileSync(cmd, args, { encoding: "utf8", input, maxBuffer: 1 << 30 }).split("\n");
}
const demangle = (syms: string[]) => pipeLines(`${LLVM}llvm-cxxfilt`, [], syms.join("\n")).slice(0, syms.length);

// ---- undefined symbols ----
const errOut = runLd("error");
writeFileSync(join(outDir, "ld-undefined.txt"), errOut);
const undef = new Map<string, { n: number; ref: string; file: string }>();
{
    let cur = "";
    for (const l of errOut.split("\n")) {
        let m = l.match(/^ {2}"(.*)", referenced from:$/);
        if (m) { cur = m[1]; undef.set(cur, { n: 0, ref: "", file: "" }); continue; }
        m = l.match(/^ {6}(.*) in (.*)$/);
        if (m && cur) {
            const u = undef.get(cur)!;
            if (!u.n) { u.ref = m[1]; u.file = m[2]; }
            u.n++;
        }
    }
}

// ---- duplicate symbols ----
const dupOut = runLd("dynamic_lookup");
writeFileSync(join(outDir, "ld-duplicates.txt"), dupOut);
const dups = new Map<string, Set<string>>();
{
    let cur = "";
    for (const l of dupOut.split("\n")) {
        const m = l.match(/^duplicate symbol (\S+) in:$/);
        if (m) { cur = m[1]; if (!dups.has(cur)) dups.set(cur, new Set()); continue; }
        if (l.startsWith("    ") && cur) dups.get(cur)!.add(l.trim().replace(/^.*\/CMakeFiles\//, ""));
    }
}

// ---- what we define, and what the Mac binary has ----
const defined: string[] = [];
for (let i = 0; i < objs.length; i += 200) {
    const out = execFileSync(`${LLVM}llvm-nm`, ["-m", "--defined-only", ...objs.slice(i, i + 200)],
        { encoding: "utf8", maxBuffer: 1 << 30 });
    for (const l of out.split("\n")) {
        const m = l.match(/ external (\S+)$/);
        if (m && !m[1].endsWith(".eh")) defined.push(m[1]);
    }
}
const defSyms = [...new Set(defined)];
const defDem = demangle(defSyms);

// the base name of a demangled function: everything before its parameter list
function baseName(d: string): string {
    let depth = 0;
    for (let i = 0; i < d.length; i++) {
        if (d[i] === "(" && depth === 0 && !d.startsWith("(anonymous", i)) return d.slice(0, i);
    }
    return d;
}
function argc(d: string): number | null {
    let depth = 0, n = 0, seen = false, start = -1;
    for (let i = 0; i < d.length; i++) {
        const c = d[i];
        if (c === "(") { if (depth === 0) { start = i; n = 0; seen = false; } depth++; }
        else if (c === ")") depth--;
        else if (depth === 1 && c === ",") n++;
        else if (depth === 1 && !/\s/.test(c)) seen = true;
    }
    return start < 0 ? null : seen ? n + 1 : 0;
}
const lastId = (b: string) => b.split("::").pop()!;
const defByBase = new Map<string, string[]>();
const defByLast = new Map<string, string[]>();
defDem.forEach((d, i) => {
    const b = baseName(d);
    (defByBase.get(b) ?? defByBase.set(b, []).get(b)!).push(d);
    (defByLast.get(lastId(b)) ?? defByLast.set(lastId(b), []).get(lastId(b))!).push(d);
});
// the clean Mac program's functions: addr, name
const macByName = new Map<string, string[]>();
const macByLast = new Map<string, { addr: string; name: string }[]>();
for (const l of readFileSync(join(HERE, "coverage/functions.tsv"), "utf8").split("\n")) {
    if (!l || l[0] === "#" || l.startsWith("addr\t")) continue;
    const p = l.split("\t");
    (macByName.get(p[3]) ?? macByName.set(p[3], []).get(p[3])!).push(p[0]);
    const li = lastId(p[3]);
    (macByLast.get(li) ?? macByLast.set(li, []).get(li)!).push({ addr: p[0], name: p[3] });
}

const WIN32 = /^_(DeleteCriticalSection|EnterCriticalSection|FreeLibrary|GetCurrentThreadId|GetLastError|GetModuleHandleA|GetProcAddress|InitializeCriticalSection|LeaveCriticalSection|LoadLibraryA|MessageBoxA|OutputDebugStringA|QueryPerformance\w+|VirtualAlloc|VirtualFree|wsprintfA)$/;
const CRT = new Set(["__eh_vector_constructor_iterator_", "__eh_vector_destructor_iterator_", "__except_handler4",
    "__purecall", "__strrev"]);

const undefSyms = [...undef.keys()].sort();
const undefDem = demangle(undefSyms);
const rows: string[][] = [];
undefSyms.forEach((s, i) => {
    const d = undefDem[i];
    const u = undef.get(s)!;
    let cls = "", det = "";
    const b = baseName(d), li = lastId(b);
    if (s === "start") { cls = "e"; det = "crt1.o entry; the tree defines no main or start"; }
    else if (/^_Bink/.test(s)) { cls = "a"; det = "RAD Bink"; }
    else if (/^_ijl/.test(s)) { cls = "a"; det = "Intel JPEG Library (Windows screenshot path in a shared unit)"; }
    else if (s === "_uncompress") { cls = "a"; det = "zlib"; }
    else if (s === "_D2CRASH_ReportHalt") { cls = "a"; det = "crashy"; }
    else if (s === "_FSpIterateDirectory") { cls = "a"; det = "MoreFiles (Apple sample)"; }
    else if (s === "___atomic_compare_exchange") { cls = "e"; det = "clang runtime libcall for a 64-bit atomic (no libatomic on 10.7)"; }
    else if (CRT.has(s)) { cls = "e"; det = "MSVC runtime symbol reached from a shared unit"; }
    else if (WIN32.test(s)) { cls = "b"; det = "Win32 API; the Mac binary has it in Win32Mac under another name (linux-server/win32-on-mac.tsv)"; }
    else if (!s.startsWith("__Z")) { cls = "c"; det = "data or C symbol, declared and never defined"; }
    else if (!d.includes("(")) { cls = "c"; det = "data (C++ name), declared and never defined"; }
    else {
        const sameBase = defByBase.get(b);
        const sameLast = (defByLast.get(li) ?? []).filter((x) => argc(x) === argc(d));
        const fun = b.match(/(?:^|::)FUN_([0-9a-f]{8})$/);
        if (sameBase) { cls = "d"; det = "defined as: " + sameBase.slice(0, 3).join(" | "); }
        else if (!fun && li.length > 3 && sameLast.length) { cls = "d"; det = "namespace or signature: " + sameLast.slice(0, 3).join(" | ").slice(0, 220); }
        else if (/crashy|::CRASHY_|ErrorReportingSetSupportPath/.test(d)) { cls = "a"; det = "crashy"; }
        else if (d.startsWith("blast(")) { cls = "a"; det = "blast (zlib contrib)"; }
        else if (fun) { cls = "b"; det = `mac ${fun[1]}`; }
        else if (macByName.has(b)) { cls = "b"; det = `mac ${macByName.get(b)!.slice(0, 3).join(",")}`; }
        else {
            const m = macByLast.get(li) ?? macByLast.get(li.replace(/[AW]$/, ""));
            if (m) { cls = "b"; det = `mac ${m.slice(0, 3).map((x) => x.addr).join(",")} (${m[0].name})`; }
            else { cls = "b2"; det = "no Mac function of that name: Windows-only code or unnamed in Ghidra"; }
        }
    }
    rows.push([cls, s, d, String(u.n), u.ref, u.file, det]);
});
rows.sort((x, y) => x[0].localeCompare(y[0]) || x[1].localeCompare(y[1]));
writeFileSync(join(outDir, "link-probe.tsv"),
    "class\tsymbol\tdemangled\trefs\tfirst_referrer\tfirst_object\tdetail\n" + rows.map((r) => r.join("\t")).join("\n") + "\n");

const dupRows = [...dups].map(([s, f]) => `${s}\t${f.size}\t${[...f].slice(0, 3).join(" ")}`);
writeFileSync(join(outDir, "duplicates.tsv"), "symbol\tdefinitions\tfirst_objects\n" + dupRows.join("\n") + "\n");

const count = new Map<string, number>();
for (const r of rows) count.set(r[0], (count.get(r[0]) ?? 0) + 1);
console.log(`objects: ${objs.length}`);
console.log(`undefined symbols: ${undefSyms.length}  (${[...count].sort().map(([k, v]) => `${k}=${v}`).join(" ")})`);
console.log(`duplicate symbols: ${dups.size}`);
console.log(`wrote ${outDir}/{link-probe.tsv,duplicates.tsv,ld-undefined.txt,ld-duplicates.txt}`);
