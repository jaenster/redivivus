// Pre-classifies every function cmp.ts does not report as instruction-identical, mechanically,
// from the normalised instruction streams of both sides (cmp.ts --insns). The result is a draft
// for reasons.tsv: rows it cannot decide are `unknown` and are settled by reading the diff.
//
//   node asm_matching/mac/cmp.ts --build <dir> --tsv cmp.tsv --insns insns.jsonl
//   node asm_matching/mac/classify.ts cmp.tsv insns.jsonl > draft.tsv
import { readFileSync } from "node:fs";

const [cmpTsv, insnsPath] = process.argv.slice(2);
if (!cmpTsv || !insnsPath) { console.error("usage: classify.ts <cmp.tsv> <insns.jsonl>"); process.exit(2); }

const REG = /\b(e?[abcd]x|e?[sd]i|[abcd][lh]|xmm\d|st\(\d\))\b/g;
const stripRegs = (s: string) => s.replace(REG, "R");
const stripPic = (s: string) => s.replace(/\[(?:[a-z]+ \+ )?PIC [^\]]*\]|\bPIC [^\]]*/g, "[P]").replace(/call \S+$/, "call C");
const stripStack = (s: string) => s.replace(/\[(ebp|esp)( [+-] 0x[0-9a-f]+)?\]/g, "[S]").replace(/^(sub|add) esp, 0x[0-9a-f]+/, "$1 esp, N");
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
    if (eq(base(o, stripPic), base(u, stripPic))) return ["pic-ref", "same instructions; a PIC data reference or call target resolves to a different symbol/offset"];
    if (eq(base(o, stripStack), base(u, stripStack))) return ["stack-layout", "same instructions; frame size or local offsets differ"];
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
    const co = calls(orig).sort().join(","), cu = calls(ours).sort().join(",");
    if (co !== cu) {
        const only = (a: string[], b: string[]) => a.filter((x) => !b.includes(x));
        const oo = only(calls(orig), calls(ours)), uo = only(calls(ours), calls(orig));
        return ["inline", `callees differ: orig-only [${oo.join(" ")}] ours-only [${uo.join(" ")}]`];
    }
    return ["unknown", `orig ${orig.length} insns, ours ${ours.length}`];
}

const tsv = readFileSync(cmpTsv, "utf8").trim().split("\n").slice(1).map((l) => l.split("\t"));
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
