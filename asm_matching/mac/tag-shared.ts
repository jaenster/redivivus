// Writes "// 1.14d mac: <addr>" before the definition of every shared function whose Windows twin
// has a Mac pair in docs/data/mac-win-pairs.tsv, so that cmp.ts compares it with the Mac binary.
//
//   node asm_matching/mac/tag-shared.ts --pairs <mac-win-pairs.tsv> [--unpaired <coverage/unpaired.tsv>] [--module <Dir>[,<Dir>...]] [--dry]
//
// --unpaired adds the rows of coverage/unpaired.tsv classed shared-unpaired that name a Windows address
// (a function the pair table has no row for, matched by hand): they are tagged by the same rules, minus
// the tier test, which a hand verdict replaces.
//
// The Windows side of a function is its address in the generator's *.cpp.map next to the source
// (the sources carry no address comment of their own); the definition is found by name in that file.
// Only a pair whose tier is a measured one (T0, T1*, V, S, K, F, J: each tier of the pair table is
// held to 99% on its holdout; T2 is a structure score and stays out) and whose extents have not
// moved since (stale = no) is tagged. A function whose definition is not unique in its file is left
// alone, and so is a file whose code uses __LINE__ (a tag line would move it). A name that more than one tagged function would carry is keyed by its qualified name,
// the form cmp.ts takes for a template instantiation. Re-running changes nothing.
import { existsSync, readdirSync, readFileSync, statSync, writeFileSync } from "node:fs";
import { dirname, join, relative, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const REPO = resolve(dirname(fileURLToPath(import.meta.url)), "../..");
const TIERS = /^(T0|T1[a-z]|V|S|K|F|J)$/;
let pairsPath = "", unpairedPath = "", dry = false;
let modules: string[] = [];
const argv = process.argv.slice(2);
for (let i = 0; i < argv.length; i++) {
    if (argv[i] === "--pairs") pairsPath = resolve(argv[++i]);
    else if (argv[i] === "--unpaired") unpairedPath = resolve(argv[++i]);
    else if (argv[i] === "--module") modules = argv[++i].split(",");
    else if (argv[i] === "--dry") dry = true;
    else { console.error(`unknown argument ${argv[i]}`); process.exit(2); }
}
if (!pairsPath) { console.error("usage: tag-shared.ts --pairs <mac-win-pairs.tsv> [--unpaired <unpaired.tsv>] [--module A,B] [--dry]"); process.exit(2); }

const walk = (d: string): string[] =>
    existsSync(d) ? readdirSync(d).flatMap((f) => { const p = join(d, f); return statSync(p).isDirectory() ? walk(p) : [p]; }) : [];

// Windows address -> Mac address
const mac = new Map<string, string>();
const rows = readFileSync(pairsPath, "utf8").split("\n").filter(Boolean).map((l) => l.split("\t"));
const col = Object.fromEntries(rows[0].map((h, i) => [h, i]));
const macSeen = new Map<string, number>();
for (const r of rows.slice(1)) macSeen.set(r[col.mac_addr], (macSeen.get(r[col.mac_addr]) ?? 0) + 1);
for (const r of rows.slice(1)) {
    if (!TIERS.test(r[col.tier]) || r[col.stale] !== "no" || macSeen.get(r[col.mac_addr]) !== 1) continue;
    mac.set(r[col.win_addr], r[col.mac_addr]);
}

// hand-matched rows: a Windows address two Mac functions claim is left alone
if (unpairedPath) {
    const byWin = new Map<string, string[]>();
    for (const l of readFileSync(unpairedPath, "utf8").split("\n")) {
        if (!l || l.startsWith("#")) continue;
        const c = l.split("\t");
        if (c[1] !== "shared-unpaired" || !/^[0-9a-f]{8}$/.test(c[2] ?? "") || !/^[0-9a-f]{8}$/.test(c[0])) continue;
        byWin.set(c[2], [...(byWin.get(c[2]) ?? []), c[0]]);
    }
    for (const [w, macs] of byWin) if (macs.length === 1 && !mac.has(w)) mac.set(w, macs[0]);
}

// every Mac address a source already tags (in any file): never claimed twice
const tagged = new Set<string>();
const sources = walk(join(REPO, "Source")).filter((f) => /\.(cpp|c|h|hpp|mm)$/.test(f));
for (const f of sources) {
    const t = readFileSync(f, "utf8");
    if (!t.includes("1.14d mac:")) continue;
    for (const m of t.matchAll(/^\s*\/\/ 1\.14d mac: ([0-9a-f]{8})/gm)) tagged.add(m[1]);
}

interface Job { file: string; line: number; win: string; mac: string; name: string; qual: string }
const jobs: Job[] = [];
const stat = { pairedNoDef: 0, ambiguous: 0, already: 0, lineSensitive: 0 };
const mapFiles = walk(join(REPO, "Source")).filter((f) => f.endsWith(".cpp.map")).sort();
for (const mf of mapFiles) {
    const cpp = mf.slice(0, -4);
    const rel = relative(REPO, cpp);
    if (modules.length && !modules.some((m) => rel.startsWith(`Source/${m}/`) || rel === `Source/globals/${m}.cpp`)) continue;
    if (!existsSync(cpp)) continue;
    const fns = (JSON.parse(readFileSync(mf, "utf8")).functions as { name: string; address: string; namespace?: string }[])
        .filter((f) => mac.has(f.address));
    if (!fns.length) continue;
    const lines = readFileSync(cpp, "utf8").split("\n");
    // an added line moves every __LINE__ below it: a file whose code uses one is left untagged
    if (lines.some((l) => /__LINE__/.test(l) && !/^\s*\/\//.test(l))) { stat.lineSensitive += fns.length; continue; }
    for (const fn of fns) {
        const m = mac.get(fn.address)!;
        if (tagged.has(m)) { stat.already++; continue; }
        const re = new RegExp(`(^|[\\s*&:~])${fn.name.replace(/[$]/g, "\\$")}\\s*\\(`);
        const hits: number[] = [];
        for (let i = 0; i < lines.length; i++) {
            const l = lines[i];
            if (/^\s/.test(l) || /^(#|\/\/|\/\*|\*|})/.test(l) || !re.test(l)) continue;
            // a definition reaches "{" before ";"
            const rest = lines.slice(i, i + 12).join("\n");
            const semi = rest.indexOf(";"), brace = rest.indexOf("{");
            if (brace >= 0 && (semi < 0 || brace < semi) && !/\b(typedef|return)\b/.test(l.split(fn.name)[0])) hits.push(i);
        }
        // the return type on the line above: the definition starts there, not at the name
        const cont = (i: number) => {
            let k = i - 1;
            while (k >= 0 && /^\s*\/\//.test(lines[k])) k--;
            return k >= 0 && !/^(\s*$|#|template|extern|namespace|\[\[)|[{};>]\s*$/.test(lines[k]);
        };
        if (hits.length === 1 && cont(hits[0])) { stat.pairedNoDef++; continue; }
        if (hits.length === 0) { stat.pairedNoDef++; continue; }
        if (hits.length > 1) { stat.ambiguous++; continue; }
        jobs.push({ file: rel, line: hits[0], win: fn.address, mac: m, name: fn.name, qual: fn.namespace ? `${fn.namespace}::${fn.name}` : fn.name });
    }
}

// a name that two tagged functions (new or old) would share is keyed by its qualified name
const nameCount = new Map<string, number>();
for (const j of jobs) nameCount.set(j.name, (nameCount.get(j.name) ?? 0) + 1);
const oldNames = new Set<string>();
for (const f of sources) {
    const t = readFileSync(f, "utf8");
    if (!t.includes("1.14d mac:")) continue;
    const ls = t.split("\n");
    ls.forEach((l, i) => {
        if (!/^\s*\/\/ 1\.14d mac: [0-9a-f]{8}\s*$/.test(l)) return;
        for (let k = i + 1; k < Math.min(ls.length, i + 6); k++) {
            const n = /([A-Za-z_]\w*)\s*(\(|\[|=|;|\{)/.exec(ls[k].replace(/^\s*(static|extern|const|inline)\s+/g, ""));
            if (n && !/^\s*(\/\/|$)/.test(ls[k])) { oldNames.add(n[1]); break; }
        }
    });
}

const byFile = new Map<string, Job[]>();
for (const j of jobs) { if (!byFile.has(j.file)) byFile.set(j.file, []); byFile.get(j.file)!.push(j); }
let n = 0, qualified = 0;
for (const [file, js] of byFile) {
    const path = join(REPO, file);
    const lines = readFileSync(path, "utf8").split("\n");
    for (const j of js.sort((a, b) => b.line - a.line)) {
        const clash = (nameCount.get(j.name) ?? 0) > 1 || oldNames.has(j.name);
        const key = clash && /::/.test(j.qual) ? ` ${j.qual}` : "";
        if (key) qualified++;
        lines.splice(j.line, 0, `// 1.14d mac: ${j.mac}${key}`);
        n++;
    }
    if (!dry) writeFileSync(path, lines.join("\n"));
}
console.log(`${dry ? "would tag" : "tagged"} ${n} functions in ${byFile.size} files (${qualified} by qualified key); ` +
    `already tagged ${stat.already}, paired but no unique definition: ${stat.pairedNoDef} none, ${stat.ambiguous} ambiguous, ${stat.lineSensitive} in files whose code uses __LINE__`);
