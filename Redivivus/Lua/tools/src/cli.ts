#!/usr/bin/env node
// rdvmod: build and check Redivivus mods written in TypeScript.
//
//   node Redivivus/Lua/tools/src/cli.ts build --out DIR MOD_DIR...
//   node Redivivus/Lua/tools/src/cli.ts check MOD_DIR...
//   node Redivivus/Lua/tools/src/cli.ts create MOD_DIR --name NAME
//   node Redivivus/Lua/tools/src/cli.ts tsconfig MOD_DIR...
//
// A mod directory holds info.json and TypeScript (or Lua) in the mod layout (see examples/).
// Each side is its own TypeScript program typed by its entry point of @redivivus/api
// (Redivivus/Lua/api), so a server-only API used in client code does not compile:
//   data     settings.ts, data.ts, data-updates.ts, data-final-fixes.ts and any other file
//            outside server/, client/ and shared/          -> api/data.d.ts
//   server   server/**                                     -> api/server.d.ts
//   client   client/**                                     -> api/client.d.ts
//   shared   shared/**, also compiled into every side      -> api/shared.d.ts
// Imports across mods are written "__other-mod__/shared/x". They type-check against the other
// mod's declarations (built into DIR/<mod>/types by its own build, so build dependencies first;
// build sorts the mods it is given) and only for mods named in info.json's dependencies; base's
// declarations sit beside its Lua files.
//
// build writes DIR/<mod>/: the Lua (TypeScriptToLua, luaTarget JIT), source maps for the host's
// error mapping, info.json, every non-TypeScript file of the mod, and types/ for dependents.
// check type-checks every side (no output) and checks that base's copy of the TypeScriptToLua
// runtime matches the pinned TypeScriptToLua.

import * as fs from "node:fs";
import * as os from "node:os";
import * as path from "node:path";
import ts from "typescript";
import * as tstl from "typescript-to-lua";
import { makePlugin } from "./plugin.ts";

const TOOLS = path.resolve(import.meta.dirname, "..");
const LUA = path.resolve(TOOLS, "..");
const API = path.join(LUA, "api");
const BASE = path.join(LUA, "mods/base");
const LUALIB_PINNED = path.join(TOOLS, "node_modules/typescript-to-lua/dist/lualib/universal/lualib_bundle.lua");
const LUALIB_BASE = path.join(BASE, "shared/lualib_bundle.lua");

type Stage = "data" | "server" | "client" | "shared";
const STAGES: Stage[] = ["shared", "data", "server", "client"];

type ModInfo = { name: string; version: string; dependencies?: string[] };

function readInfo(dir: string): ModInfo {
    const file = path.join(dir, "info.json");
    if (!fs.existsSync(file)) throw new Error(`${dir}: no info.json`);
    const info = JSON.parse(fs.readFileSync(file, "utf8")) as ModInfo;
    if (!info.name) throw new Error(`${file}: no name`);
    return info;
}

function dependencyNames(info: ModInfo): string[] {
    const names = (info.dependencies ?? [])
        .map((d) => d.trim())
        .filter((d) => !d.startsWith("!"))
        .map((d) => d.replace(/^\(\?\)\s*|^[?~]\s*/, "").split(/\s|[<>=]/)[0]);
    if (info.name !== "base" && !names.includes("base")) names.unshift("base");
    return names;
}

function walk(dir: string, out: string[] = []): string[] {
    if (!fs.existsSync(dir)) return out;
    for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
        if (entry.name === "node_modules" || entry.name.startsWith(".")) continue;
        const full = path.join(dir, entry.name);
        if (entry.isDirectory()) walk(full, out);
        else out.push(full);
    }
    return out;
}

function isTs(file: string): boolean {
    return file.endsWith(".ts") && !file.endsWith(".d.ts");
}

function stageOf(modDir: string, file: string): Stage {
    const first = path.relative(modDir, file).split(path.sep)[0];
    if (first === "server" || first === "client" || first === "shared") return first;
    return "data";
}

// The files a side's program compiles, and the declarations it reads.
function stageFiles(modDir: string, stage: Stage): { sources: string[]; decls: string[] } {
    const files = walk(modDir).filter((f) => !path.relative(modDir, f).startsWith("types" + path.sep));
    const ts_ = files.filter(isTs);
    const decls = files.filter((f) => f.endsWith(".d.ts"));
    const sources = ts_.filter((f) => {
        const s = stageOf(modDir, f);
        return s === stage || (s === "shared" && stage !== "shared");
    });
    const visibleDecls = decls.filter((f) => {
        const s = stageOf(modDir, f);
        return s === stage || s === "shared" || (s === "data" && path.dirname(f) === modDir);
    });
    return { sources, decls: visibleDecls };
}

function compilerOptions(modDir: string, info: ModInfo, stage: Stage, outRoot: string | undefined, typesRoot: string[]): tstl.CompilerOptions {
    const paths: Record<string, string[]> = {};
    for (const dep of dependencyNames(info)) {
        if (dep === "base") {
            paths["__base__/*"] = [path.join(BASE, "*")];
            continue;
        }
        const candidates = typesRoot.map((r) => path.join(r, dep, "types", "*"));
        paths[`__${dep}__/*`] = candidates;
    }
    const options: tstl.CompilerOptions = {
        target: ts.ScriptTarget.ESNext,
        lib: ["lib.esnext.d.ts"],
        module: ts.ModuleKind.Preserve,
        moduleResolution: ts.ModuleResolutionKind.Bundler,
        // Every file is a module, with or without import/export: a top-level const stays a local
        // of its file and never becomes a global of the mod's environment.
        moduleDetection: ts.ModuleDetectionKind.Force,
        strict: true,
        noImplicitOverride: true,
        noFallthroughCasesInSwitch: true,
        skipLibCheck: true,
        types: [],
        typeRoots: [path.join(TOOLS, "node_modules")],
        paths,
        rootDir: modDir,
        sourceMap: true,
        luaTarget: tstl.LuaTarget.LuaJIT,
        luaLibImport: tstl.LuaLibImportKind.Require,
        noImplicitSelf: true,
        noResolvePaths: ["__*__/**"],
        noHeader: true,
        luaPlugins: [{ plugin: makePlugin(modDir) }],
    };
    if (outRoot) {
        options.outDir = path.join(outRoot, info.name);
    } else {
        options.noEmit = true;
    }
    void stage;
    return options;
}

function apiEntry(stage: Stage): string {
    return path.join(API, `${stage}.d.ts`);
}

const formatHost: ts.FormatDiagnosticsHost = {
    getCanonicalFileName: (f) => f,
    getCurrentDirectory: () => process.cwd(),
    getNewLine: () => "\n",
};

function report(diagnostics: readonly ts.Diagnostic[]): number {
    const real = diagnostics.filter((d) => d.category === ts.DiagnosticCategory.Error);
    if (diagnostics.length) process.stderr.write(ts.formatDiagnosticsWithColorAndContext(diagnostics, formatHost));
    return real.length;
}

// Type-check (and with outRoot, emit) one side of a mod. Returns the error count.
function compileStage(modDir: string, info: ModInfo, stage: Stage, outRoot: string | undefined, typesRoot: string[], declarationsOnly = false): number {
    const { sources, decls } = stageFiles(modDir, stage);
    if (!sources.length && stage !== "shared") return 0;
    if (!sources.length) return 0;
    const options = compilerOptions(modDir, info, stage, outRoot, typesRoot);
    const roots = [apiEntry(stage), ...decls, ...sources];
    if (!outRoot || stage === "shared") {
        const program = ts.createProgram(roots, { ...options, noEmit: true });
        const diags = ts.getPreEmitDiagnostics(program);
        const errors = report(diags);
        if (errors || !outRoot) return errors;
        // Declarations of shared/ for mods that import from this one.
        const dts = ts.createProgram(roots.filter((f) => !isTs(f) || stageOf(modDir, f) === "shared"), {
            ...options, noEmit: false, declaration: true, emitDeclarationOnly: true, outDir: path.join(outRoot, info.name, "types"),
        });
        const emitted = dts.emit();
        const dtsErrors = report(emitted.diagnostics);
        if (dtsErrors || declarationsOnly) return dtsErrors;
        return report(tstl.transpileFiles(roots, options).diagnostics);
    }
    const result = tstl.transpileFiles(roots, options);
    return report(result.diagnostics);
}

// Order mod directories so that each comes after the mods it depends on (by name, within the set).
function sortMods(dirs: string[]): { dir: string; info: ModInfo }[] {
    const mods = dirs.map((dir) => ({ dir: path.resolve(dir), info: readInfo(dir) }));
    const byName = new Map(mods.map((m) => [m.info.name, m]));
    const done = new Set<string>();
    const out: typeof mods = [];
    const visit = (m: (typeof mods)[number], stack: string[]) => {
        if (done.has(m.info.name)) return;
        if (stack.includes(m.info.name)) throw new Error(`dependency cycle: ${[...stack, m.info.name].join(" -> ")}`);
        for (const d of dependencyNames(m.info)) {
            const dep = byName.get(d);
            if (dep) visit(dep, [...stack, m.info.name]);
        }
        done.add(m.info.name);
        out.push(m);
    };
    for (const m of [...mods].sort((a, b) => (a.info.name < b.info.name ? -1 : 1))) visit(m, []);
    return out;
}

function copyNonTs(modDir: string, outDir: string) {
    for (const file of walk(modDir)) {
        const rel = path.relative(modDir, file);
        if (file.endsWith(".ts") || rel.startsWith("types" + path.sep) || path.basename(file) === "tsconfig.json") continue;
        const target = path.join(outDir, rel);
        fs.mkdirSync(path.dirname(target), { recursive: true });
        fs.copyFileSync(file, target);
    }
}

function checkLualib(): number {
    const pinned = fs.readFileSync(LUALIB_PINNED, "utf8");
    const ours = fs.existsSync(LUALIB_BASE) ? fs.readFileSync(LUALIB_BASE, "utf8") : "";
    if (pinned !== ours) {
        console.error(`rdvmod: ${path.relative(process.cwd(), LUALIB_BASE)} differs from the pinned TypeScriptToLua's runtime; copy ${LUALIB_PINNED} over it`);
        return 1;
    }
    return 0;
}

function build(args: string[]): number {
    const outIndex = args.indexOf("--out");
    if (outIndex < 0 || !args[outIndex + 1]) {
        console.error("rdvmod build: --out DIR is required");
        return 2;
    }
    const outRoot = path.resolve(args[outIndex + 1]);
    const dirs = args.filter((_, i) => i !== outIndex && i !== outIndex + 1);
    let errors = checkLualib();
    for (const { dir, info } of sortMods(dirs)) {
        const outDir = path.join(outRoot, info.name);
        fs.rmSync(outDir, { recursive: true, force: true });
        fs.mkdirSync(outDir, { recursive: true });
        let modErrors = 0;
        for (const stage of STAGES) modErrors += compileStage(dir, info, stage, outRoot, [outRoot]);
        copyNonTs(dir, outDir);
        console.log(`rdvmod: ${info.name} ${modErrors ? `FAILED (${modErrors} errors)` : "built"} -> ${path.relative(process.cwd(), outDir)}`);
        errors += modErrors;
    }
    return errors ? 1 : 0;
}

function check(args: string[]): number {
    const typesIndex = args.indexOf("--types");
    // Declarations of the mods checked so far, so later ones can import from them.
    const scratch = fs.mkdtempSync(path.join(os.tmpdir(), "rdvmod-check-"));
    const typesRoot = [scratch, ...(typesIndex >= 0 ? [path.resolve(args[typesIndex + 1])] : [])];
    const dirs = args.filter((_, i) => typesIndex < 0 || (i !== typesIndex && i !== typesIndex + 1));
    let errors = checkLualib();
    for (const { dir, info } of sortMods(dirs)) {
        let modErrors = 0;
        // Shared: declarations for later mods. The sides: transpiled to a scratch directory, so the
        // plugin's checks (Script functions used outside yield*) run too.
        for (const stage of STAGES) modErrors += compileStage(dir, info, stage, stage === "shared" ? scratch : path.join(scratch, "lua"), typesRoot, stage === "shared");
        console.log(`rdvmod: ${info.name} ${modErrors ? `has ${modErrors} type errors` : "type-checks"}`);
        errors += modErrors;
    }
    fs.rmSync(scratch, { recursive: true, force: true });
    return errors ? 1 : 0;
}

function tsconfigs(args: string[]): number {
    for (const dir of args) {
        const info = readInfo(dir);
        for (const stage of STAGES) {
            const { sources } = stageFiles(path.resolve(dir), stage);
            if (!sources.length) continue;
            const options = compilerOptions(path.resolve(dir), info, stage, undefined, [path.resolve(dir, "../build")]);
            const where = stage === "data" ? path.resolve(dir) : path.resolve(dir, stage);
            const rel = (p: string) => path.relative(where, p).split(path.sep).join("/");
            const config = {
                compilerOptions: {
                    target: "ESNext", lib: ["ESNext"], module: "preserve", moduleResolution: "bundler", moduleDetection: "force", strict: true,
                    skipLibCheck: true, noEmit: true, types: [], typeRoots: [rel(path.join(TOOLS, "node_modules"))],
                    paths: Object.fromEntries(Object.entries(options.paths ?? {}).map(([k, v]) => [k, v.map(rel)])),
                    plugins: [{ name: "@redivivus/api/ls-plugin" }],
                },
                files: [rel(apiEntry(stage))],
                include: stage === "data" ? ["*.ts", "prototypes/**/*.ts", "shared/**/*.ts"] : ["**/*.ts", "../shared/**/*.ts"],
                tstl: { luaTarget: "JIT", noImplicitSelf: true },
            };
            fs.writeFileSync(path.join(where, "tsconfig.json"), JSON.stringify(config, null, 4) + "\n");
            console.log(`rdvmod: wrote ${path.relative(process.cwd(), path.join(where, "tsconfig.json"))} (editor support; rdvmod build ignores it)`);
        }
    }
    return 0;
}

function create(args: string[]): number {
    const dir = args[0];
    const nameIndex = args.indexOf("--name");
    const name = nameIndex >= 0 ? args[nameIndex + 1] : path.basename(dir ?? "");
    if (!dir || !/^[A-Za-z0-9_-]{1,64}$/.test(name)) {
        console.error("rdvmod create: MOD_DIR [--name NAME]; names use letters, digits, '_' and '-'");
        return 2;
    }
    if (fs.existsSync(dir)) {
        console.error(`rdvmod create: ${dir} exists`);
        return 1;
    }
    const files: Record<string, string> = {
        "info.json": JSON.stringify({ name, version: "0.1.0", title: name, author: "", redivivus_version: "0.1", dependencies: ["base >= 0.1"] }, null, 2) + "\n",
        "shared/util.ts": "export function label(text: string): string {\n    return `[" + name + "] ${text}`;\n}\n",
        "server/control.ts": "import { label } from \"../shared/util\";\n\nscript.on_init(() => {\n    storage.started = script.tick;\n    log(label(\"started\"));\n});\n",
        "client/control.ts": "script.on_event(Event.Draw, () => {});\n",
        "data.ts": "data.extend([]);\n",
    };
    for (const [rel, text] of Object.entries(files)) {
        fs.mkdirSync(path.dirname(path.join(dir, rel)), { recursive: true });
        fs.writeFileSync(path.join(dir, rel), text);
    }
    tsconfigs([dir]);
    console.log(`rdvmod: created ${dir}`);
    return 0;
}

function main(argv: string[]): number {
    const [command, ...args] = argv;
    try {
        switch (command) {
            case "build": return build(args);
            case "check": return check(args);
            case "create": return create(args);
            case "tsconfig": return tsconfigs(args);
            default:
                console.error("usage: rdvmod build --out DIR MOD_DIR... | check [--types DIR] MOD_DIR... | create MOD_DIR [--name NAME] | tsconfig MOD_DIR...");
                return 2;
        }
    } catch (e) {
        console.error(`rdvmod: ${(e as Error).message}`);
        return 1;
    }
}

process.exit(main(process.argv.slice(2)));
