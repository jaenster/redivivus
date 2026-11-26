// Builds the kit twice from the same sources:
//   out/node  - TypeScript to ESM JavaScript (tsc), every file, for tests and the preview CLI
//   out/lua   - TypeScriptToLua (LuaJIT target) of what runs in the game: the kit (../core/ui, which
//               the game embeds as redivivus:ui), the Mods screen (../builtin/mods), screens/, test/rt/
// Both mirror Redivivus/Lua: out/node/core/ui/index.js, out/node/ui/host/cli.js, ...
// Run directly: node tools/build.ts [node|lua]

import * as fs from "node:fs";
import * as path from "node:path";
import ts from "typescript";
import * as tstl from "typescript-to-lua";

export const ROOT = path.resolve(import.meta.dirname, "..");
/** Redivivus/Lua: the root both builds mirror. */
export const LUA = path.resolve(ROOT, "..");
export const OUT_NODE = path.join(ROOT, "out/node");
export const OUT_LUA = path.join(ROOT, "out/lua");

/** Sources that compile to Lua: everything the game would run. */
const LUA_DIRS = ["core/ui", "builtin/mods", "ui/screens", "ui/test/rt"];

function walk(dir: string, out: string[]): string[] {
    if (!fs.existsSync(dir)) return out;
    for (const e of fs.readdirSync(dir, { withFileTypes: true }).sort((a, b) => (a.name < b.name ? -1 : 1))) {
        const p = path.join(dir, e.name);
        if (e.isDirectory()) walk(p, out);
        else if ((e.name.endsWith(".ts") || e.name.endsWith(".tsx")) && !e.name.endsWith(".d.ts")) out.push(p);
    }
    return out;
}

function format(diags: readonly ts.Diagnostic[]): string {
    return ts.formatDiagnostics(diags, { getCanonicalFileName: (f) => f, getCurrentDirectory: () => ROOT, getNewLine: () => "\n" });
}

function config(): ts.ParsedCommandLine {
    const parsed = ts.getParsedCommandLineOfConfigFile(path.join(ROOT, "tsconfig.json"), {}, {
        ...ts.sys,
        onUnRecoverableConfigFileDiagnostic: (d) => {
            throw new Error(format([d]));
        },
    });
    if (parsed === undefined) throw new Error("tsconfig.json did not parse");
    return parsed;
}

// ESM in Node needs file extensions; the sources leave them out so TypeScriptToLua can resolve the
// same imports. Rewrite each relative specifier (and the redivivus:ui alias) to a real file.
function fixImports(file: string): void {
    let text = fs.readFileSync(file, "utf8");
    const dir = path.dirname(file);
    text = text.replace(/(\bfrom\s+|\bimport\s*\(\s*|\bimport\s+)(["'])([^"']+)\2/g, (m, pre: string, q: string, spec: string) => {
        let target: string | undefined;
        if (spec === "redivivus:ui") target = path.join(OUT_NODE, "core/ui/index.js");
        else if (spec.startsWith("./") || spec.startsWith("../")) {
            const abs = path.resolve(dir, spec);
            if (fs.existsSync(abs + ".js")) target = abs + ".js";
            else if (fs.existsSync(path.join(abs, "index.js"))) target = path.join(abs, "index.js");
        }
        if (target === undefined) return m;
        let rel = path.relative(dir, target).split(path.sep).join("/");
        if (!rel.startsWith(".")) rel = "./" + rel;
        return `${pre}${q}${rel}${q}`;
    });
    fs.writeFileSync(file, text);
}

export function buildNode(): void {
    const cfg = config();
    const options: ts.CompilerOptions = { ...cfg.options, noEmit: false, outDir: OUT_NODE, rootDir: LUA, declaration: false, sourceMap: false };
    const program = ts.createProgram(cfg.fileNames, options);
    const diags = ts.getPreEmitDiagnostics(program).filter((d) => d.category === ts.DiagnosticCategory.Error);
    if (diags.length) throw new Error(`type errors:\n${format(diags)}`);
    const written: string[] = [];
    const res = program.emit(undefined, (f, text) => {
        fs.mkdirSync(path.dirname(f), { recursive: true });
        fs.writeFileSync(f, text);
        written.push(f);
    });
    if (res.emitSkipped) throw new Error("tsc emit skipped");
    for (const f of written) if (f.endsWith(".js")) fixImports(f);
}

export function luaOptions(): tstl.CompilerOptions {
    return {
        target: ts.ScriptTarget.ESNext,
        lib: ["lib.esnext.d.ts"],
        module: ts.ModuleKind.Preserve,
        moduleResolution: ts.ModuleResolutionKind.Bundler,
        moduleDetection: ts.ModuleDetectionKind.Force,
        strict: true,
        skipLibCheck: true,
        types: [],
        jsx: ts.JsxEmit.React,
        jsxFactory: "h",
        jsxFragmentFactory: "Fragment",
        // TypeScriptToLua resolves paths against the config file directory when there is no baseUrl.
        configFilePath: path.join(ROOT, "tsconfig.json"),
        paths: { "redivivus:ui": ["../core/ui/index.ts"], "redivivus:enums": ["../core/redivivus/enums.ts"] },
        rootDir: LUA,
        outDir: OUT_LUA,
        luaTarget: tstl.LuaTarget.LuaJIT,
        luaLibImport: tstl.LuaLibImportKind.Require,
        noImplicitSelf: true,
        noHeader: true,
        sourceMap: false,
    };
}

export function luaSources(): string[] {
    const files: string[] = [];
    for (const d of LUA_DIRS) walk(path.join(LUA, d), files);
    return files;
}

export function buildLua(): void {
    const files = luaSources();
    const opts = luaOptions();
    const program = ts.createProgram(files, { ...opts, noEmit: true });
    const diags = ts.getPreEmitDiagnostics(program).filter((d) => d.category === ts.DiagnosticCategory.Error);
    if (diags.length) throw new Error(`the Lua sources do not type-check without Node:\n${format(diags)}`);
    const res = tstl.transpileFiles(files, opts, (f, text) => {
        fs.mkdirSync(path.dirname(f), { recursive: true });
        fs.writeFileSync(f, text);
    });
    const errors = res.diagnostics.filter((d) => d.category === ts.DiagnosticCategory.Error);
    if (errors.length) throw new Error(`TypeScriptToLua:\n${format(errors)}`);
}

if (import.meta.main) {
    const what = process.argv[2] ?? "all";
    const t0 = Date.now();
    if (what === "all" || what === "node") buildNode();
    if (what === "all" || what === "lua") buildLua();
    console.log(`build ${what}: ${Date.now() - t0} ms`);
}
