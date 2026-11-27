// Pre-classifies every function cmp.ts does not report as instruction-identical, mechanically,
// from the normalised instruction streams of both sides (cmp.ts --insns). The result is a draft
// for reasons.tsv: rows it cannot decide are `unknown` and are settled by reading the diff.
//
//   node asm_matching/mac/cmp.ts --build <dir> --tsv cmp.tsv --insns insns.jsonl
//   node asm_matching/mac/classify.ts cmp.tsv insns.jsonl > draft.tsv
import { existsSync, readdirSync, readFileSync, statSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const [cmpTsv, insnsPath] = process.argv.slice(2);
if (!cmpTsv || !insnsPath) { console.error("usage: classify.ts <cmp.tsv> <insns.jsonl>"); process.exit(2); }

// The address tags, as cmp.ts reads them: a name may carry several (an alias declared under
// another name in a second file), which cmp.ts prints under one of them only.
const REPO = resolve(dirname(fileURLToPath(import.meta.url)), "../..");
const tagAddrs = new Map<string, Set<number>>();
const tagAt = new Map<number, string>();
function walk(d: string): string[] {
    if (!existsSync(d)) return [];
    return readdirSync(d).flatMap((f) => { const p = join(d, f); return statSync(p).isDirectory() ? walk(p) : [p]; });
}
for (const root of ["Source", "Redivivus"]) {
    for (const file of walk(join(REPO, root)).filter((f) => /\.(cpp|c|h|hpp|mm)$/i.test(f))) {
        const text = readFileSync(file, "utf8");
        if (!text.includes("1.14d mac:")) continue;
        let pend: number | null = null;
        for (const l of text.split("\n")) {
            const m = /^\s*\/\/ 1\.14d mac: ([0-9a-f]{8})\b(?:\s+(\S+))?/.exec(l);
            const add = (n: string, a: number) => { if (!tagAddrs.has(n)) tagAddrs.set(n, new Set()); tagAddrs.get(n)!.add(a); tagAt.set(a, n); };
            if (m && m[2] && /[<:~]|^_GLOBAL__/.test(m[2])) { add(m[2], parseInt(m[1], 16)); pend = null; continue; }
            if (m) { pend = parseInt(m[1], 16); continue; }
            if (pend === null || /^\s*(\/\/|$)/.test(l)) continue;
            const n = /([A-Za-z_]\w*)\s*(\(|\[|=|;|\{)/.exec(l.replace(/^\s*(static|extern|const|inline)\s+/g, ""));
            if (n) add(n[1], pend);
            pend = null;
        }
    }
}
// "name+off" / "name-off" as cmp.ts prints a data reference: the absolute address when the
// name is tagged. On our side only "+0" is absolute: any other offset is measured in our
// object's layout, not the binary's.
function refAddr(ref: string, ours: boolean): number | null {
    const m = /^&?([A-Za-z_][\w:<>,~]*)([+-])([0-9a-f]+)$/.exec(ref);
    if (!m) return null;
    const as = tagAddrs.get(m[1]);
    if (!as || as.size !== 1) return null;
    const off = parseInt(m[3], 16);
    if (ours && off !== 0) return null;
    return ([...as][0] + (m[2] === "-" ? -off : off)) >>> 0;
}
const funcAddrs = new Set<number>();
const sameTag = (a: string, b: string) => [...(tagAddrs.get(a) ?? [])].some((x) => tagAddrs.get(b)?.has(x));
const hex = (a: number) => a.toString(16).padStart(8, "0");
// Splits a pic-ref difference into what it is: a real callee or data address difference, an
// alias or neighbour-naming artefact of cmp.ts, a __FILE__ literal, or an unverifiable reference.
function picRef(o: string[], u: string[]): [string, string] {
    const refs = (s: string) => [...s.matchAll(/\[(?:[a-z]+ \+ )?PIC ([^\]]*)\]|^call (\S+)$/g)].map((m) => ({ ref: m[1] ?? m[2], call: m[2] !== undefined }));
    const wrong: string[] = [], data: string[] = [], art: string[] = [], unv: string[] = [], file: string[] = [];
    for (let i = 0; i < o.length; i++) {
        if (o[i] === u[i]) continue;
        const ro = refs(o[i]), ru = refs(u[i]);
        for (let k = 0; k < Math.min(ro.length, ru.length); k++) {
            const a = ro[k].ref, b = ru[k].ref;
            if (a === b) continue;
            const fa = /^"[^"]*\/([^/"]+)"$/.exec(a), fb = /^__FILE__ (\S+)$/.exec(b);
            if (fa && fb && fa[1].toLowerCase() === fb[1].toLowerCase()) { file.push(`${fa[1]} vs ${fb[1]}`); continue; }
            if (a === "?" || b === "?") { unv.push(`${a} / ${b}`); continue; }
            if (ro[k].call) {
                const bn = b.replace(/^_/, "");
                if (sameTag(a, bn) || sameTag(a.replace(/^_/, ""), bn)) art.push(`alias ${a}=${b}`);
                else if (a.startsWith("_") !== b.startsWith("_") && a.replace(/^_/, "") === bn) art.push(`import ${a} named as our tag ${b}`);
                else if (tagAddrs.has(a) && tagAddrs.has(bn)) wrong.push(`calls ${b} where the original calls ${a}`);
                else unv.push(`call ${a} / ${b}`);
                continue;
            }
            const sa = a.replace(/^&/, ""), sb = b.replace(/^&/, "");
            if (a.startsWith("&") && b.startsWith("&") && sameTag(sa, sb)) { art.push(`alias ${a}=${b}`); continue; }
            const xa = refAddr(a, false), xb = refAddr(b, true);
            // our side named off a function symbol: a function-local static, untagged
            if (xb !== null && funcAddrs.has(xb)) { unv.push(`${a} / ${b}`); continue; }
            if (xa !== null && xb !== null) {
                if (xa === xb) art.push(`${hex(xa)} named ${a} / ${b}`);
                else data.push(`original ${hex(xa)} (${a}), ours ${b} tagged ${hex(xb)}`);
            } else unv.push(`${a} / ${b}`);
        }
    }
    const u1 = [...new Set(unv)], a1 = [...new Set(art)], f1 = [...new Set(file)];
    const tail = [f1.length ? `__FILE__ basename ${f1[0]}` : "", a1.length ? `cmp naming: ${a1.slice(0, 3).join("; ")}` : "", u1.length ? `unverifiable ${u1.slice(0, 3).join("; ")}` : ""].filter(Boolean).join("; ");
    if (wrong.length) return ["wrong", `same instructions except the callee: ${[...new Set(wrong)].join("; ")}${tail ? "; " + tail : ""}`];
    if (data.length) return ["data-guess", `same instructions except a data address: ${[...new Set(data)].slice(0, 3).join("; ")}${tail ? "; " + tail : ""}`];
    return ["pic-ref", `same instructions; ${tail || "a PIC data reference or call target resolves to a different symbol/offset"}`];
}
// Frame offsets are normalised away for stack-layout, which also hides a field read at the wrong
// offset inside a frame object. A dword the original reads at a frame offset that is not a
// multiple of 4 is a field of a packed (2-aligned) record, never a local of its own: it must keep
// its distance to the record's preceding offset.
function frameFieldDiff(o: string[], u: string[]): string | null {
    const off = (s: string) => [...s.matchAll(/\[ebp - (0x[0-9a-f]+)\]/g)].map((m) => parseInt(m[1], 16));
    const map = new Map<number, number>(), packed = new Set<number>();
    for (let i = 0; i < o.length; i++) {
        const a = off(o[i]), b = off(u[i]);
        for (let k = 0; k < Math.min(a.length, b.length); k++) {
            if (!map.has(a[k])) map.set(a[k], b[k]);
            if (a[k] % 4 !== 0 && /dword ptr \[ebp - /.test(o[i])) packed.add(a[k]);
        }
    }
    for (const x of packed) {
        const base = [...map.keys()].filter((y) => y > x && y - x < 4).sort((p, q) => p - q)[0];
        if (base !== undefined && map.get(base)! - map.get(x)! !== base - x)
            return `frame field offsets differ: original [ebp - 0x${base.toString(16)}]/[ebp - 0x${x.toString(16)}] (a dword field of a packed record), ours [ebp - 0x${map.get(base)!.toString(16)}]/[ebp - 0x${map.get(x)!.toString(16)}]`;
    }
    return null;
}
const REG = /\b(e?[abcd]x|e?[sd]i|[abcd][lh]|xmm\d|st\(\d\))\b/g;
const stripRegs = (s: string) => s.replace(REG, "R");
const stripPic = (s: string) => s.replace(/\[(?:[a-z]+ \+ )?PIC [^\]]*\]|\bPIC [^\]]*/g, "[P]").replace(/call \S+$/, "call C");
// Locals and outgoing arguments only: [ebp + n] is an incoming argument, and reading a
// different one is a real difference, not a frame layout.
const stripStack = (s: string) => s.replace(/\[(ebp - 0x[0-9a-f]+|esp( \+ 0x[0-9a-f]+)?)\]/g, "[S]").replace(/^(sub|add) esp, 0x[0-9a-f]+/, "$1 esp, N");
const norm = (f: (s: string) => string) => (a: string[]) => a.map(f);
const sortedKey = (a: string[]) => [...a].sort().join("\n");
const calls = (a: string[]) => a.filter((x) => /^call /.test(x) && x !== "call $+5").map((x) => x.replace(/^call /, "")).filter((x) => x !== "?" && !/^(e[abcd]x|e[sd]i)$/.test(x));
const eq = (a: string[], b: string[]) => a.length === b.length && a.every((x, i) => x === b[i]);
const isMem = (s: string) => /\[/.test(s);
const jccStrip = (s: string) => s.replace(/#\d+/g, "#");

// exception landing pads sit after the last return; clang425 emits them for any frame with a destructor
const EH = /__Unwind_Resume|__ZSt9terminatev|___cxa_|\bterminate\b/;
function cutEh(a: string[]): string[] {
    let last = -1;
    for (let i = 0; i < a.length; i++) if (/^(ret|jmp [A-Za-z_])/.test(a[i]) && !/^jmp #/.test(a[i])) last = i;
    const tail = a.slice(last + 1);
    return last >= 0 && tail.some((x) => EH.test(x)) ? a.slice(0, last + 1) : a;
}
function lcsRatio(a: string[], b: string[]): number {
    const n = a.length, m = b.length;
    if (!n || !m) return 0;
    let prev = new Array(m + 1).fill(0);
    for (let i = 1; i <= n; i++) {
        const cur = new Array(m + 1).fill(0);
        for (let j = 1; j <= m; j++) cur[j] = a[i - 1] === b[j - 1] ? prev[j - 1] + 1 : Math.max(prev[j], cur[j - 1]);
        prev = cur;
    }
    return prev[m] / Math.max(n, m);
}
function classify(orig: string[], ours: string[]): [string, string] {
    const r = classify1(orig, ours);
    if (r[0] !== "unknown" && r[0] !== "inline") return r;
    const co = cutEh(orig), cu = cutEh(ours);
    if (co.length !== orig.length || cu.length !== ours.length) {
        const r2 = classify1(co, cu);
        if (r2[0] !== "unknown" && r2[0] !== "inline") return ["eh-pad", `landing-pad tail differs (orig ${orig.length - co.length} insns, ours ${ours.length - cu.length}); body ${r2[0]}: ${r2[1]}`];
    }
    const nrm = (x: string[]) => x.map((v) => stripRegs(stripStack(stripPic(jccStrip(v)))));
    if (r[0] === "inline") return r;
    return ["unknown", `${r[1]}; opcode-skeleton similarity ${lcsRatio(nrm(orig), nrm(ours)).toFixed(2)}`];
}
function classify1(orig: string[], ours: string[]): [string, string] {
    const o = orig.map(jccStrip), u = ours.map(jccStrip);
    const base = (x: string[], ...fs: ((s: string) => string)[]) => x.map((s) => fs.reduce((v, f) => f(v), s));
    // 1. same except where a data reference or callee resolves differently
    if (eq(base(o, stripPic), base(u, stripPic))) return picRef(o, u);
    if (eq(base(o, stripStack), base(u, stripStack))) {
        const f = frameFieldDiff(o, u);
        return f ? ["wrong", f] : ["stack-layout", "same instructions; frame size or local offsets differ"];
    }
    if (eq(base(o, stripRegs), base(u, stripRegs))) return ["regalloc", "same instructions modulo register choice"];
    if (eq(base(o, stripPic, stripStack), base(u, stripPic, stripStack))) return ["stack-layout", "same instructions modulo frame offsets and PIC references"];
    if (eq(base(o, stripPic, stripRegs), base(u, stripPic, stripRegs))) return ["regalloc", "same instructions modulo register choice and PIC references"];
    if (eq(base(o, stripStack, stripRegs), base(u, stripStack, stripRegs))) return ["regalloc", "same instructions modulo register choice and frame offsets"];
    const all = (x: string[]) => base(x, jccStrip, stripPic, stripStack, stripRegs);
    if (eq(all(orig), all(ours))) return ["regalloc", "same instructions modulo registers, frame offsets and PIC references"];
    // 2. same multiset, different order
    if (sortedKey(base(o, stripPic)) === sortedKey(base(u, stripPic)) || sortedKey(base(o, stripPic, stripRegs)) === sortedKey(base(u, stripPic, stripRegs)))
        return ["sched", "same instructions in a different order"];
    if (sortedKey(all(orig)) === sortedKey(all(ours))) return ["sched", "same instructions modulo registers/offsets in a different order"];
    // 3. callee set differs: inlining
    // an alias (one address tagged under two names) or an import cmp.ts names after our tag is the same callee
    const canon = (x: string) => { const b = x.replace(/^_/, ""); const as = tagAddrs.get(b); return as && as.size ? "@" + hex(Math.min(...as)) : b; };
    const co = calls(orig).map(canon).sort().join(","), cu = calls(ours).map(canon).sort().join(",");
    if (co !== cu) {
        const only = (a: string[], b: string[]) => a.filter((x) => !b.includes(x));
        const oc = calls(orig).map(canon), uc = calls(ours).map(canon);
        const oo = calls(orig).filter((x) => !uc.includes(canon(x))), uo = calls(ours).filter((x) => !oc.includes(canon(x)));
        return ["inline", `callees differ: orig-only [${oo.join(" ")}] ours-only [${uo.join(" ")}]`];
    }
    return ["unknown", `orig ${orig.length} insns, ours ${ours.length}`];
}

const tsv = readFileSync(cmpTsv, "utf8").trim().split("\n").slice(1).map((l) => l.split("\t"));
for (const r of tsv) funcAddrs.add(parseInt(r[1], 16));
const insns = new Map<string, { orig: string[]; ours: string[] }>();
for (const l of readFileSync(insnsPath, "utf8").trim().split("\n")) if (l) { const j = JSON.parse(l); insns.set(j.addr, j); }

console.log("addr\tname\treason_class\tdetail\tsize");
for (const [file, addr, name, status, n, unv, detail] of tsv) {
    if (status === "SAME" && unv === "0") continue;
    let cls = "unknown", d = detail;
    const sz = n;
    if (status === "NOOBJ") [cls, d] = ["noobj", `${file} is not built by d2_mac_objs`];
    else if (status === "MISSING") [cls, d] = ["unknown", "tagged but absent from the object (inlined or not compiled)"];
    else if (status === "SAME") [cls, d] = ["unverified-ref", `${unv} reference(s) only one side can resolve; every other instruction identical`];
    else {
        const r = insns.get(addr);
        if (r) [cls, d] = classify(r.orig, r.ours);
    }
    console.log([addr, name, cls, d.replace(/[\t\n]/g, " "), sz].join("\t"));
}
