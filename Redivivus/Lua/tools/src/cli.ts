#!/usr/bin/env node
// rdvmod: build, check, create and pack Redivivus mods.
//
//   node Redivivus/Lua/tools/src/cli.ts build --out DIR MOD_DIR...
//   node Redivivus/Lua/tools/src/cli.ts check [--types DIR] MOD_DIR...
//   node Redivivus/Lua/tools/src/cli.ts create MOD_DIR [--name NAME] [--ui]
//   node Redivivus/Lua/tools/src/cli.ts tsconfig MOD_DIR...
//   node Redivivus/Lua/tools/src/cli.ts pack [--out DIR] SOURCE
//   node Redivivus/Lua/tools/src/cli.ts pack --check FILE.mpq
//
// A mod is one program: info.json, the data stage entries (settings, data, data-updates,
// data-final-fixes; .ts or .lua), the control entry main (main.ts or main.tsx, or main.lua), and
// any other files those import. The same main is loaded in the server state and in the client
// state; it exports server(game: Game) and/or client(client: Client), which the engine calls.
//
// Modules are imported as "<mod>:<path>": redivivus:server and the rest of the built-in API,
// "other-mod:ids" for a declared dependency; relative imports reach the mod's own files. The
// declarations of redivivus are Redivivus/Lua/api (the @redivivus/api package); a dependency's are
// the .d.ts files its build put beside its Lua (build dependencies first: build sorts the mods it
// is given). redivivus:test and redivivus:automation are not for mods and do not compile in one.
// Every redivivus module resolves in every state, redivivus:ui included: UI a mod builds in the
// server state or the data stage works as anywhere else and is never drawn. TSX compiles with the
// kit's factory, h and Fragment from redivivus:ui.
//
// build writes DIR/<mod>/: the Lua (TypeScriptToLua, luaTarget JIT), source maps for the host's
// error mapping, .d.ts for dependents, info.json and every non-TypeScript file of the mod. That
// folder is the mod's namespace, mods/<mod>/ in the game's file tree.
//
// pack turns a built mod folder (or a tree root holding mods/<mod>/ and data/) into
// <mod>_<version>.mpq: the profile the portal and the game accept (tools/src/mpq), every file
// under mods\<mod>\ but game assets under data\, names case-insensitive and unique, the same bytes
// for the same files. pack --check validates an archive against the profile and the layout rules
// and prints its SHA-256.

import { createHash } from "node:crypto";
import * as fs from "node:fs";
import * as os from "node:os";
import * as path from "node:path";
import ts from "typescript";
import * as tstl from "typescript-to-lua";
import { build as buildMpq, list as listMpq, read as readMpq, validate as validateMpq, type InputFile } from "./mpq/index.ts";
import { makePlugin } from "./plugin.ts";
import { mainChecks, MAIN_DIAGNOSTIC, CROSS_DIAGNOSTIC, scriptChecks } from "./scriptcheck.ts";

const TOOLS = path.resolve(import.meta.dirname, "..");
const LUA = path.resolve(TOOLS, "..");
const API = path.join(LUA, "api");
const LUA_ENV = path.join(API, "lua/index.d.ts");
const INTRINSIC = "redivivus";
const GATED: Record<string, string> = {
    test: "redivivus:test is for tests the test runner starts; a mod cannot import it",
    automation: "redivivus:automation is for driver scripts started from the command line; a mod cannot import it (nothing a server sends may drive a player's client)",
    manager: "redivivus:manager is the game's own redivivus mod's (it turns mods on and off and restarts the game); a mod cannot import it",
};

type ModInfo = { name: string; version: string; dependencies?: string[] };

function readInfo(dir: string): ModInfo {
    const file = path.join(dir, "info.json");
    if (!fs.existsSync(file)) throw new Error(`${dir}: no info.json`);
    const info = JSON.parse(fs.readFileSync(file, "utf8")) as ModInfo;
    if (!info.name) throw new Error(`${file}: no name`);
    return info;
}

function dependencyNames(info: ModInfo): string[] {
    return (info.dependencies ?? [])
        .map((d) => d.trim())
        .filter((d) => !d.startsWith("!"))
        .map((d) => d.replace(/^\(\?\)\s*|^[?~]\s*/, "").split(/\s|[<>=]/)[0])
        .filter((d) => d !== INTRINSIC);
}

function walk(dir: string, out: string[] = []): string[] {
    if (!fs.existsSync(dir)) return out;
    for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
        if (entry.name === "node_modules" || entry.name.startsWith(".")) continue;
        const full = path.join(dir, entry.name);
        if (entry.isDirectory()) walk(full, out);
        else out.push(full);
    }
    return out.sort();
}

function isSource(file: string): boolean {
    return (file.endsWith(".ts") || file.endsWith(".tsx")) && !file.endsWith(".d.ts");
}

function redivivusEntries(): string[] {
    return fs.readdirSync(path.join(API, "redivivus")).filter((f) => f.endsWith(".d.ts")).map((f) => f.replace(/\.d\.ts$/, ""))
        .filter((e) => e !== "engine-enums");
}

function compilerOptions(modDir: string, info: ModInfo, outDir: string | undefined, typesRoot: string[]): tstl.CompilerOptions {
    const paths: Record<string, string[]> = { [`${INTRINSIC}:*`]: [path.join(API, "redivivus", "*")] };
    if (info.name !== INTRINSIC) paths[`${info.name}:*`] = [path.join(modDir, "*")];
    const noResolve: string[] = [];
    for (const name of new Set([INTRINSIC, info.name, ...dependencyNames(info)])) noResolve.push(`${name}:*`, `${name}:*/**`);
    for (const dep of dependencyNames(info)) paths[`${dep}:*`] = typesRoot.map((r) => path.join(r, dep, "*"));
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
        noResolvePaths: noResolve,
        noHeader: true,
        luaPlugins: [{ plugin: makePlugin(modDir) }],
        jsx: ts.JsxEmit.React,
        jsxFactory: "h",
        jsxFragmentFactory: "Fragment",
    };
    if (outDir) options.outDir = outDir;
    else options.noEmit = true;
    return options;
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

function diagnostic(node: ts.Node, message: string, code: number, warning = false): ts.Diagnostic {
    return {
        file: node.getSourceFile(), start: node.getStart(), length: node.getWidth(), messageText: message,
        category: warning ? ts.DiagnosticCategory.Warning : ts.DiagnosticCategory.Error, code, source: "rdvmod",
    };
}

// What the resolver would refuse, reported at the import: modules of mods that are no
// dependency, redivivus modules that do not exist or are not for mods.
function importIssues(program: ts.Program, sources: string[], info: ModInfo, driver: boolean, builtin = false): ts.Diagnostic[] {
    const out: ts.Diagnostic[] = [];
    const entries = redivivusEntries();
    const deps = new Set([INTRINSIC, info.name, ...dependencyNames(info)]);
    for (const file of sources) {
        const sf = program.getSourceFile(file);
        if (!sf) continue;
        const visit = (n: ts.Node): void => {
            const spec = (ts.isImportDeclaration(n) || ts.isExportDeclaration(n)) && n.moduleSpecifier && ts.isStringLiteral(n.moduleSpecifier) ? n.moduleSpecifier
                : ts.isImportTypeNode(n) && ts.isLiteralTypeNode(n.argument) && ts.isStringLiteral(n.argument.literal) ? n.argument.literal : undefined;
            if (spec) {
                const m = /^([A-Za-z0-9_-]+):(.*)$/.exec(spec.text);
                if (m) {
                    const [, mod, rest] = m;
                    if (!deps.has(mod)) {
                        out.push(diagnostic(spec, `mod '${info.name}' does not depend on '${mod}' (declare it in info.json)`, MAIN_DIAGNOSTIC));
                    } else if (mod === INTRINSIC) {
                        if (GATED[rest] && !(rest === "automation" && driver) && !(rest === "manager" && builtin)) out.push(diagnostic(spec, GATED[rest], MAIN_DIAGNOSTIC));
                        else if (!entries.includes(rest)) out.push(diagnostic(spec, `redivivus has no module '${rest}' (its modules: ${entries.join(", ")})`, MAIN_DIAGNOSTIC));
                    }
                }
            }
            ts.forEachChild(n, visit);
        };
        visit(sf);
    }
    return out;
}

// The checks of the control entry: lifecycle exports (errors) and cross-side reads (warnings).
function entryIssues(program: ts.Program, sources: string[]): ts.Diagnostic[] {
    const main = mainChecks(ts);
    const checker = program.getTypeChecker();
    const out: ts.Diagnostic[] = [];
    for (const file of sources) {
        const sf = program.getSourceFile(file);
        if (!sf || !main.isMain(sf)) continue;
        for (const i of main.exportIssues(checker, sf)) out.push(diagnostic(i.node, i.message, i.code));
        for (const i of main.crossSideIssues(checker, sf)) out.push(diagnostic(i.node, i.message, i.code, true));
    }
    return out;
}

// Type-check and run every check over one mod; with outDir, also emit its Lua and declarations.
function compileMod(modDir: string, info: ModInfo, outDir: string | undefined, typesRoot: string[], driver = false, builtin = false): number {
    const files = walk(modDir);
    const sources = files.filter(isSource);
    if (!sources.length) return 0;
    const decls = files.filter((f) => f.endsWith(".d.ts"));
    const roots = [LUA_ENV, ...decls, ...sources];
    const options = compilerOptions(modDir, info, outDir, typesRoot);
    const program = ts.createProgram(roots, { ...options, noEmit: true });
    let errors = report([...ts.getPreEmitDiagnostics(program), ...importIssues(program, sources, info, driver, builtin), ...entryIssues(program, sources)]);
    if (errors) return errors;
    // The plugin's checks (Script functions used outside yield*) run while transpiling.
    const scratch = outDir ?? fs.mkdtempSync(path.join(os.tmpdir(), "rdvmod-check-"));
    errors += report(tstl.transpileFiles(roots, { ...options, outDir: scratch, noEmit: false }).diagnostics);
    if (!outDir) {
        fs.rmSync(scratch, { recursive: true, force: true });
        return errors;
    }
    if (errors) return errors;
    const dts = ts.createProgram(roots, { ...options, noEmit: false, declaration: true, emitDeclarationOnly: true, outDir });
    return report(dts.emit().diagnostics);
}

/**
 * The game's own redivivus mod (Redivivus/Lua/builtin): compiled exactly as a mod is, against the
 * same declarations, so it can use nothing a mod cannot; only redivivus:manager is its alone. Gives
 * its Lua by path in its namespace ("main.lua", "mods/model.lua"), or throws with the diagnostics.
 */
export function compileBuiltin(dir: string): Map<string, string> {
    const info: ModInfo = { name: INTRINSIC, version: "0.0.0" };
    const outDir = fs.mkdtempSync(path.join(os.tmpdir(), "rdvmod-builtin-"));
    try {
        const errors = compileMod(path.resolve(dir), info, outDir, [], false, true);
        if (errors) throw new Error(`the built-in mod (${dir}) does not compile: ${errors} error(s)`);
        const out = new Map<string, string>();
        for (const file of walk(outDir).filter((f) => f.endsWith(".lua"))) {
            out.set(path.relative(outDir, file).split(path.sep).join("/"), fs.readFileSync(file, "utf8"));
        }
        return out;
    } finally {
        fs.rmSync(outDir, { recursive: true, force: true });
    }
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
        if (isSource(file) || path.basename(file) === "tsconfig.json") continue;
        const target = path.join(outDir, rel);
        fs.mkdirSync(path.dirname(target), { recursive: true });
        fs.copyFileSync(file, target);
    }
}

function build(args: string[]): number {
    const outIndex = args.indexOf("--out");
    if (outIndex < 0 || !args[outIndex + 1]) {
        console.error("rdvmod build: --out DIR is required");
        return 2;
    }
    const outRoot = path.resolve(args[outIndex + 1]);
    const driver = args.includes("--driver");
    const dirs = args.filter((a, i) => i !== outIndex && i !== outIndex + 1 && a !== "--driver");
    let errors = 0;
    for (const { dir, info } of sortMods(dirs)) {
        const outDir = path.join(outRoot, info.name);
        fs.rmSync(outDir, { recursive: true, force: true });
        fs.mkdirSync(outDir, { recursive: true });
        const modErrors = compileMod(dir, info, outDir, [outRoot], driver);
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
    const driver = args.includes("--driver");
    const dirs = args.filter((a, i) => (typesIndex < 0 || (i !== typesIndex && i !== typesIndex + 1)) && a !== "--driver");
    let errors = 0;
    for (const { dir, info } of sortMods(dirs)) {
        const modErrors = compileMod(dir, info, path.join(scratch, info.name), typesRoot, driver);
        console.log(`rdvmod: ${info.name} ${modErrors ? `has ${modErrors} errors` : "checks"}`);
        errors += modErrors;
    }
    fs.rmSync(scratch, { recursive: true, force: true });
    return errors ? 1 : 0;
}

function tsconfigs(args: string[]): number {
    for (const dir of args) {
        const where = path.resolve(dir);
        const info = readInfo(where);
        const options = compilerOptions(where, info, undefined, [path.resolve(where, "../build")]);
        const rel = (p: string) => path.relative(where, p).split(path.sep).join("/") || ".";
        const config = {
            compilerOptions: {
                target: "ESNext", lib: ["ESNext"], module: "preserve", moduleResolution: "bundler", moduleDetection: "force",
                strict: true, skipLibCheck: true, noEmit: true, types: [], typeRoots: [rel(path.join(TOOLS, "node_modules"))],
                jsx: "react", jsxFactory: "h", jsxFragmentFactory: "Fragment",
                paths: Object.fromEntries(Object.entries(options.paths ?? {}).map(([k, v]) => [k, v.map(rel)])),
                plugins: [{ name: "@redivivus/api/ls-plugin" }],
            },
            files: [rel(LUA_ENV)],
            include: ["**/*.ts", "**/*.tsx"],
            tstl: { luaTarget: "JIT", noImplicitSelf: true },
        };
        fs.writeFileSync(path.join(where, "tsconfig.json"), JSON.stringify(config, null, 4) + "\n");
        console.log(`rdvmod: wrote ${path.relative(process.cwd(), path.join(where, "tsconfig.json"))} (editor support; rdvmod build ignores it)`);
    }
    return 0;
}

function create(args: string[]): number {
    const withUi = args.includes("--ui");
    args = args.filter((a) => a !== "--ui");
    const dir = args[0];
    const nameIndex = args.indexOf("--name");
    const name = nameIndex >= 0 ? args[nameIndex + 1] : path.basename(dir ?? "");
    if (!dir || !/^[A-Za-z0-9_-]{1,64}$/.test(name) || /^(mod|base|core|redivivus.*)$/i.test(name)) {
        console.error("rdvmod create: MOD_DIR [--name NAME] [--ui]; names use letters, digits, '_' and '-' (mod, base, core and redivivus* are reserved)");
        return 2;
    }
    if (fs.existsSync(dir)) {
        console.error(`rdvmod create: ${dir} exists`);
        return 1;
    }
    const files: Record<string, string> = {
        "info.json": JSON.stringify({ name, version: "0.1.0", title: name, author: "", dependencies: ["redivivus >= 0.1"] }, null, 2) + "\n",
        "label.ts": "export function label(text: string): string {\n    return `[" + name + "] ${text}`;\n}\n",
        "main.ts": [
            "import { log } from \"redivivus:shared\";",
            "import { Event, type Game } from \"redivivus:server\";",
            "import type { Client } from \"redivivus:client\";",
            "import { label } from \"./label\";",
            "",
            "// Called once for every game this process hosts. Register everything through game.",
            "export function server(game: Game) {",
            "    const state = game.storage(\"state\", { started: 0 });",
            "    if (!game.loaded) state.started = game.tick;",
            "    game.on(Event.Tick, () => {});",
            "    log(label(\"server started\"));",
            "}",
            "",
            "// Called once for every game this client is in.",
            "export function client(client: Client) {",
            "    client.on(Event.Draw, () => {});",
            "}",
            "",
        ].join("\n"),
        "data.ts": "import { data } from \"redivivus:data\";\n\ndata.extend([]);\n",
    };
    if (withUi) {
        // A panel in TSX, imported by main like any other module.
        files["main.ts"] = files["main.ts"]
            .replace('import { log } from "redivivus:shared";', 'import { log } from "redivivus:shared";\nimport { open } from "./panel";')
            .replace("    client.on(Event.Draw, () => {});\n", [
                "    open(client);",
                "",
            ].join("\n"));
        files["panel.tsx"] = [
            "import type { Client } from \"redivivus:client\";",
            "import { log } from \"redivivus:shared\";",
            "import { BoxStyle, ButtonVariant, Direction, h, HeadlessMetrics, mount, Recorder, signal, testTheme } from \"redivivus:ui\";",
            "",
            "function Counter() {",
            "    const [clicks, setClicks] = signal(0);",
            "    return (",
            "        <frame style={BoxStyle.Panel} direction={Direction.Column} padding={8} gap={6}>",
            `            <label text="${name}" />`,
            "            <label text={() => `clicked ${clicks()} times`} />",
            "            <button variant={ButtonVariant.Small} label=\"+1\" onClick={() => setClicks(clicks() + 1)} />",
            "        </frame>",
            "    );",
            "}",
            "",
            "// Until the game draws mod UI, the panel is laid out and drawn once on the kit's recorder,",
            "// which writes every draw call down.",
            "export function open(client: Client) {",
            "    const recorder = new Recorder(new HeadlessMetrics());",
            "    const ui = mount(() => <Counter />, recorder, testTheme(), 320, 200);",
            "    ui.draw();",
            "    log(`panel drawn for game ${client.gameId}: ${recorder.lines.length} draw calls`);",
            "}",
            "",
        ].join("\n");
    }
    for (const [rel, text] of Object.entries(files)) {
        fs.mkdirSync(path.dirname(path.join(dir, rel)), { recursive: true });
        fs.writeFileSync(path.join(dir, rel), text);
    }
    tsconfigs([dir]);
    console.log(`rdvmod: created ${dir}`);
    return 0;
}

// --- pack -------------------------------------------------------------------------------------------

const ASSET_ROOTS = ["data"];

/** Why the archive's names break the layout rules, empty when they hold. */
export function layoutProblems(names: string[], infoText: (name: string) => string | undefined): string[] {
    const problems: string[] = [];
    const namespaces = new Set<string>();
    const lower = new Map<string, string>();
    for (const n of names) {
        const k = n.toLowerCase();
        if (lower.has(k)) problems.push(`'${n}' and '${lower.get(k)}' differ only by case; names in an archive are case-insensitive`);
        lower.set(k, n);
        const parts = n.split("\\");
        if (parts[0].toLowerCase() === "mods") {
            if (parts.length < 3) {
                problems.push(`'${n}': files under mods\\ live in a mod's namespace, mods\\<name>\\...`);
                continue;
            }
            namespaces.add(parts[1]);
        } else if (!ASSET_ROOTS.includes(parts[0].toLowerCase())) {
            problems.push(`'${n}' is outside mods\\<name>\\ and outside the game asset roots (${ASSET_ROOTS.map((r) => `${r}\\`).join(", ")})`);
        }
    }
    const spaces = [...namespaces];
    if (spaces.some((s) => s.toLowerCase() === INTRINSIC)) problems.push("the archive carries files in mods\\redivivus\\; redivivus is built into the game and nothing may replace or extend it");
    const infos = names.filter((n) => /^mods\\[^\\]+\\info\.json$/i.test(n));
    if (infos.length === 0) problems.push("no mods\\<name>\\info.json");
    if (infos.length > 1) problems.push(`more than one info.json: ${infos.join(", ")}`);
    if (spaces.length > 1) problems.push(`files in more than one mod's namespace: ${spaces.map((s) => `mods\\${s}\\`).join(", ")}; an archive carries one mod`);
    if (infos.length === 1) {
        const space = infos[0].split("\\")[1];
        const text = infoText(infos[0]);
        let name: unknown;
        try {
            name = JSON.parse(text ?? "").name;
        } catch {
            problems.push(`${infos[0]} is not JSON`);
        }
        if (name !== undefined && name !== space) problems.push(`${infos[0]} names the mod '${String(name)}', but its namespace is '${space}'`);
    }
    return problems;
}

function sha256(bytes: Uint8Array): string {
    return createHash("sha256").update(bytes).digest("hex");
}

function pack(args: string[]): number {
    const checkIndex = args.indexOf("--check");
    if (checkIndex >= 0) {
        const file = args[checkIndex + 1];
        if (!file) {
            console.error("rdvmod pack --check FILE.mpq");
            return 2;
        }
        const bytes = new Uint8Array(fs.readFileSync(file));
        const result = validateMpq(bytes);
        if (!result.ok) {
            for (const e of result.errors) console.error(`rdvmod pack --check: ${e.code}${e.name ? ` (${e.name})` : ""}: ${e.message}`);
            return 1;
        }
        const names = listMpq(bytes).map((f) => f.name);
        const problems = layoutProblems(names, (n) => {
            const data = readMpq(bytes, n);
            return data ? new TextDecoder().decode(data) : undefined;
        });
        for (const p of problems) console.error(`rdvmod pack --check: ${p}`);
        console.log(`${sha256(bytes)}  ${file}  (${names.length} files${problems.length ? `, ${problems.length} layout problem(s)` : ", profile and layout ok"})`);
        return problems.length ? 1 : 0;
    }
    const outIndex = args.indexOf("--out");
    const outDir = path.resolve(outIndex >= 0 ? args[outIndex + 1] : ".");
    const source = args.filter((_, i) => outIndex < 0 || (i !== outIndex && i !== outIndex + 1))[0];
    if (!source) {
        console.error("rdvmod pack [--out DIR] SOURCE  (a built mod folder, or a tree root holding mods/<name>/)");
        return 2;
    }
    const root = path.resolve(source);
    const files: InputFile[] = [];
    let info: ModInfo;
    if (fs.existsSync(path.join(root, "info.json"))) {
        info = readInfo(root);
        for (const f of walk(root)) {
            if (path.basename(f) === "tsconfig.json") continue;
            files.push({ name: ["mods", info.name, ...path.relative(root, f).split(path.sep)].join("\\"), data: new Uint8Array(fs.readFileSync(f)) });
        }
    } else {
        const mods = path.join(root, "mods");
        const spaces = fs.existsSync(mods) ? fs.readdirSync(mods) : [];
        if (spaces.length !== 1) {
            console.error(`rdvmod pack: ${source} is neither a mod folder (with info.json) nor a tree root with exactly one mods/<name>/`);
            return 1;
        }
        info = readInfo(path.join(mods, spaces[0]));
        for (const f of walk(root)) files.push({ name: path.relative(root, f).split(path.sep).join("\\"), data: new Uint8Array(fs.readFileSync(f)) });
    }
    const byName = new Map(files.map((f) => [f.name, f.data]));
    const problems = layoutProblems(files.map((f) => f.name), (n) => {
        const d = byName.get(n);
        return d ? new TextDecoder().decode(d) : undefined;
    });
    if (problems.length) {
        for (const p of problems) console.error(`rdvmod pack: ${p}`);
        return 1;
    }
    const bytes = buildMpq(files);
    const check = validateMpq(bytes);
    if (!check.ok) {
        for (const e of check.errors) console.error(`rdvmod pack: the archive fails its own profile: ${e.code}: ${e.message}`);
        return 1;
    }
    fs.mkdirSync(outDir, { recursive: true });
    const out = path.join(outDir, `${info.name}_${info.version}.mpq`);
    fs.writeFileSync(out, bytes);
    console.log(`${sha256(bytes)}  ${path.relative(process.cwd(), out)}  (${files.length} files, ${bytes.length} bytes)`);
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
            case "pack": return pack(args);
            default:
                console.error("usage: rdvmod build --out DIR MOD_DIR... | check [--types DIR] MOD_DIR... | create MOD_DIR [--name NAME] | tsconfig MOD_DIR... | pack [--out DIR] SOURCE | pack --check FILE.mpq");
                return 2;
        }
    } catch (e) {
        console.error(`rdvmod: ${(e as Error).message}`);
        return 1;
    }
}

void CROSS_DIAGNOSTIC;
void scriptChecks;
if (process.argv[1] && path.resolve(process.argv[1]) === path.resolve(import.meta.filename)) process.exit(main(process.argv.slice(2)));
