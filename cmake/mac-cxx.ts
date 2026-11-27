// The compiler launcher of the Mac build (cmake/mac.cmake): clang-425 is a C++11 compiler and
// the shared tree spells its namespaces the C++17 way (`namespace Fog::Src::ErrorManager {`).
// For each compile the unit is preprocessed with the same compiler and flags; when the
// preprocessed text holds a nested namespace definition, it is rewritten as one namespace per
// name (`namespace Fog { namespace Src { namespace ErrorManager {`, closed by as many braces)
// and that text is compiled. A unit without one compiles from its source as before.
//
// The rewrite changes no declaration: the same names in the same scopes, so the same code and
// the same symbols. It is what the VC8 oracle's translation does for the same reason.
//
//   node cmake/mac-cxx.ts <compiler> <arguments...>      (CXX_COMPILER_LAUNCHER)
import { spawnSync } from 'node:child_process';
import { mkdtempSync, readFileSync, rmSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';

const [compiler, ...args] = process.argv.slice(2);

function run(argv: string[]): number {
    const r = spawnSync(compiler, argv, { stdio: 'inherit' });
    return r.status ?? 1;
}

const compileOnly = args.includes('-c');
const outIndex = args.indexOf('-o');
if (!compileOnly || outIndex < 0) process.exit(run(args));

// The source is the argument after -c (CMake's order: ... -o <obj> -c <src>).
const srcIndex = args.indexOf('-c') + 1;
const depFlags = new Set(['-MD', '-MMD']);
const depArgs = new Set(['-MF', '-MT', '-MQ']);

// Preprocess: everything but -c and -o, with the dependency file written here.
const pre: string[] = [];
for (let i = 0; i < args.length; i++) {
    if (args[i] === '-c') continue;
    if (args[i] === '-o') { i++; continue; }
    pre.push(args[i]);
}
const dir = mkdtempSync(join(tmpdir(), 'd2-mac-cxx-'));
const ii = join(dir, 'unit.ii');
let status = run([...pre, '-E', '-o', ii]);
if (status !== 0) { rmSync(dir, { recursive: true, force: true }); process.exit(status); }

const text = readFileSync(ii, 'latin1');
const lowered = lowerNestedNamespaces(text);
if (lowered === text) {
    rmSync(dir, { recursive: true, force: true });
    process.exit(run(args));
}
writeFileSync(ii, lowered, 'latin1');

// Compile the rewritten text: the same flags, no dependency output (the preprocess wrote it),
// the language as its preprocessed form.
const comp: string[] = [];
for (let i = 0; i < args.length; i++) {
    const a = args[i];
    if (depFlags.has(a)) continue;
    if (depArgs.has(a)) { i++; continue; }
    if (i === srcIndex) { comp.push(ii); continue; }
    if (a === '-x' && i + 1 < args.length) {
        const lang = args[++i];
        comp.push('-x', lang.endsWith('-cpp-output') ? lang : `${lang}-cpp-output`);
        continue;
    }
    comp.push(a);
}
if (!comp.includes('-x')) comp.unshift('-x', 'c++-cpp-output');
status = run(comp);
rmSync(dir, { recursive: true, force: true });
process.exit(status);

// `namespace A::B::C {` -> `namespace A { namespace B { namespace C {`, and its closing brace
// closes all three. Preprocessed text: no comments; string and character literals and
// directive lines are skipped.
function lowerNestedNamespaces(src: string): string {
    const head = /namespace\s+([A-Za-z_]\w*(?:\s*::\s*[A-Za-z_]\w*)+)\s*\{/y;
    const out: string[] = [];
    const extra: number[] = [];
    let i = 0;
    let from = 0;
    while (i < src.length) {
        const c = src[i];
        if (c === '"' || c === "'") {
            let j = i + 1;
            while (j < src.length && src[j] !== c && src[j] !== '\n') j += src[j] === '\\' ? 2 : 1;
            i = j + 1;
            continue;
        }
        if (c === '#' && (i === 0 || src[i - 1] === '\n')) {
            const j = src.indexOf('\n', i);
            i = j < 0 ? src.length : j;
            continue;
        }
        if (c === 'n' && (i === 0 || !/\w/.test(src[i - 1]))) {
            head.lastIndex = i;
            const m = head.exec(src);
            if (m) {
                const names = m[1].split('::').map((s) => s.trim());
                out.push(src.slice(from, i), names.map((n) => `namespace ${n} {`).join(' '));
                extra.push(names.length - 1);
                i = from = head.lastIndex;
                continue;
            }
        }
        if (c === '{') extra.push(0);
        else if (c === '}') {
            const k = extra.pop() ?? 0;
            if (k > 0) {
                out.push(src.slice(from, i + 1), '}'.repeat(k));
                from = i + 1;
            }
        }
        i++;
    }
    if (from === 0) return src;
    out.push(src.slice(from));
    return out.join('');
}
