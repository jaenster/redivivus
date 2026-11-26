// The whole TypeScript side of the mod system, checked in one go:
//
//   node Redivivus/Lua/tools/src/selftest.ts [--out DIR] [--host-test BINARY]
//
//   - the generated outputs (glue, embedded core, declarations, docs, LuaLS) are up to date
//   - the example mods build (first-mod, second-mod importing from it, the plain-Lua lua-mod, ui-mod
//     with a TSX panel)
//   - every example and the type tests check; each @ts-expect-error in examples/type-tests is a
//     real error, and with them blanked out every one is reported at its own line
//   - the build refuses Script misuse, a wrong lifecycle export of main (at the export), imports
//     the resolver would refuse; it warns about a variable written in server() and read in client()
//   - the editor plugin reports the same, in a language service and in a real tsserver that loads
//     it from a mod project's node_modules
//   - pack: deterministic bytes, the layout rules, pack --check
//   - with --host-test, the host test runs over the built mods (a .exe runs under wine)
// Exit code 0 when everything passes.

import { spawn, spawnSync } from "node:child_process";
import { createHash } from "node:crypto";
import { createRequire } from "node:module";
import * as fs from "node:fs";
import * as os from "node:os";
import * as path from "node:path";
import ts from "typescript";
import { engineEnums } from "./enums.ts";
import { layoutProblems } from "./cli.ts";
import { build as buildMpq } from "./mpq/index.ts";

const TOOLS = path.resolve(import.meta.dirname, "..");
const LUA = path.resolve(TOOLS, "..");
const ROOT = path.resolve(LUA, "../..");
const EXAMPLES = path.join(LUA, "examples");

function arg(name: string): string | undefined {
    const i = process.argv.indexOf(name);
    return i >= 0 ? process.argv[i + 1] : undefined;
}

let failed = 0;
let passed = 0;
function run(command: string, args: string[], cwd = ROOT): { ok: boolean; output: string } {
    const r = spawnSync(command, args, { cwd, encoding: "utf8", maxBuffer: 64 * 1024 * 1024 });
    return { ok: r.status === 0, output: ((r.stdout ?? "") + (r.stderr ?? "")).replace(/\x1b\[[0-9;]*m/g, "") };
}

function expect(what: string, ok: boolean, detail = ""): void {
    console.log(`${ok ? "PASS" : "FAIL"} ${what}`);
    if (ok) passed++;
    else {
        failed++;
        if (detail) console.log(detail.split("\n").slice(-40).join("\n"));
    }
}

function step(what: string, command: string, args: string[], want: { ok: boolean; output?: string | RegExp }, cwd = ROOT): string {
    const r = run(command, args, cwd);
    const outputOk = !want.output || (typeof want.output === "string" ? r.output.includes(want.output) : want.output.test(r.output));
    expect(what, r.ok === want.ok && outputOk, r.output);
    return r.output;
}

const node = process.execPath;
const out = path.resolve(arg("--out") ?? fs.mkdtempSync(path.join(os.tmpdir(), "rdvmod-out-")));
const cli = path.join(TOOLS, "src/cli.ts");
const scratch = fs.mkdtempSync(path.join(os.tmpdir(), "rdvmod-self-"));

function mod(name: string, files: Record<string, string>, deps: string[] = []): string {
    const dir = path.join(scratch, name);
    fs.rmSync(dir, { recursive: true, force: true });
    for (const [rel, text] of Object.entries({ "info.json": JSON.stringify({ name, version: "0.1.0", dependencies: deps }), ...files })) {
        fs.mkdirSync(path.dirname(path.join(dir, rel)), { recursive: true });
        fs.writeFileSync(path.join(dir, rel), text);
    }
    return dir;
}

step("generated outputs are up to date", node, [path.join(TOOLS, "src/gen.ts"), "--check"], { ok: true });
step("the example mods build", node, [cli, "build", "--out", out, path.join(EXAMPLES, "first-mod"), path.join(EXAMPLES, "second-mod"), path.join(EXAMPLES, "lua-mod"), path.join(EXAMPLES, "ui-mod"), path.join(EXAMPLES, "charger"), path.join(EXAMPLES, "fallen-shaman")], { ok: true });
step("the examples and the type tests check (every @ts-expect-error is a real error)", node,
    [cli, "check", path.join(EXAMPLES, "first-mod"), path.join(EXAMPLES, "second-mod"), path.join(EXAMPLES, "lua-mod"), path.join(EXAMPLES, "ui-mod"), path.join(EXAMPLES, "charger"), path.join(EXAMPLES, "fallen-shaman"), path.join(EXAMPLES, "type-tests")], { ok: true });

// The type tests with every @ts-expect-error blanked out: each line under one must be reported.
{
    const tt = path.join(scratch, "type-tests");
    fs.cpSync(path.join(EXAMPLES, "type-tests"), tt, { recursive: true });
    const expected: string[] = [];
    for (const f of fs.readdirSync(tt).filter((x) => x.endsWith(".ts") || x.endsWith(".tsx"))) {
        const full = path.join(tt, f);
        const lines = fs.readFileSync(full, "utf8").split("\n");
        lines.forEach((l, i) => {
            if (!/^\s*\/\/ @ts-expect-error/.test(l)) return;
            lines[i] = "//";
            expected.push(`${f}:${i + 2}`);
        });
        fs.writeFileSync(full, lines.join("\n"));
    }
    const r = run(node, [cli, "check", path.join(EXAMPLES, "first-mod"), tt]);
    const reported = new Set([...r.output.matchAll(/type-tests\/([\w/.-]+\.tsx?):(\d+):\d+/g)].map((m) => `${m[1]}:${m[2]}`));
    const missing = expected.filter((e) => !reported.has(e));
    expect(`without their @ts-expect-error, all ${expected.length} must-fail cases are reported at their line`, expected.length > 40 && missing.length === 0, `not reported: ${missing.join(", ")}`);
}

const G = 'import type { Game } from "redivivus:server";\nimport type { Client } from "redivivus:client";\nimport type { Script } from "redivivus:shared";\n';

// Script misuse.
step("a Script function called outside yield* is refused", node, [cli, "check", mod("bad-call", {
    "main.ts": `${G}function* act(): Script<number> { yield; return 1; }\nexport function server(game: Game) { game.on("tick" as never, () => { act(); }); }\n`,
})], { ok: false, output: "A Script function runs only as" });
step("yield* of something other than an action is refused", node, [cli, "check", mod("bad-delegate", {
    "main.ts": `${G}function* act(): Script<number> { yield* [undefined]; return 1; }\nexport function server(game: Game) { game.startScript(act as never); }\n`,
})], { ok: false, output: "yield* takes a call of a Script function" });

step("a Script call handed to guard with a spread argument is refused", node, [cli, "check", mod("bad-defer", {
    "main.ts": `${G}import { guard } from "redivivus:shared";\nfunction* act(n: number): Script<number> { yield; return n; }\nconst xs: [number] = [1];\nexport function server(game: Game) { game.startScript(function* (): Script { yield* guard(() => true, act(...xs)); }); }\n`,
})], { ok: false, output: "cannot take a spread argument" });
{
    const dir = mod("defer-emit", {
        "main.ts": `${G}import { guard } from "redivivus:shared";\nfunction* act(n: number): Script<number> { yield; return n; }\nexport function server(game: Game) { game.startScript(function* (): Script { const o = yield* guard(() => true, act(2)); void o; }); }\n`,
    });
    step("a Script call handed to guard builds", node, [cli, "build", "--out", path.join(scratch, "defer-out"), dir], { ok: true });
    const built = path.join(scratch, "defer-out", "defer-emit", "main.lua");
    const lua = fs.existsSync(built) ? fs.readFileSync(built, "utf8") : "";
    expect("  as a function that runs it, its arguments evaluated in place (not a call that runs at once)",
        lua.includes("(function(____body, ____arg1) return function() return ____body(____arg1) end end)(act, 2)"), lua);
}

// The control entry, reported at the export.
function atLine(output: string, file: string, line: number, text: string): boolean {
    return output.split("\n").some((l) => l.includes(`${file}:${line}:`) && l.includes(text));
}
{
    const r = run(node, [cli, "check", mod("bad-entry", {
        "main.ts": `${G}export function server(game: Client) { void game; }\nexport function client() {}\nexport function sever(game: Game) { void game; }\nexport const helper = 1;\n`,
    })]);
    expect("server with the wrong parameter type is reported at the export", !r.ok && atLine(r.output, "main.ts", 4, "server takes a Game"), r.output);
    expect("client without its parameter is reported at the export", atLine(r.output, "main.ts", 5, "the client parameter is missing"), r.output);
    expect("an unknown lifecycle export (a typo) is reported at the export", atLine(r.output, "main.ts", 6, "'sever' is not a lifecycle export") && r.output.includes("did you mean 'server'"), r.output);
    expect("  and a plain export is fine", !r.output.includes("'helper'"), r.output);
}
{
    const r = run(node, [cli, "check", mod("cross-side", {
        "main.ts": `${G}let shared = 0;\nexport function server(game: Game) { game.startScript(function* (): Script { shared = 1; }); }\nexport function client(client: Client) { void client; void shared; }\n`,
    })]);
    expect("a variable written in server() and read in client() is warned about, at the read, and does not fail the build",
        r.ok && atLine(r.output, "main.ts", 6, "'shared' is written in server() and read in client()"), r.output);
}

// Imports the resolver would refuse.
step("redivivus:test does not compile in a mod", node, [cli, "check", mod("gate-test", { "main.ts": 'import { probe } from "redivivus:test";\nvoid probe;\n' })],
    { ok: false, output: "redivivus:test is for tests the test runner starts; a mod cannot import it" });
step("redivivus:automation does not compile in a mod", node, [cli, "check", mod("gate-auto", { "main.ts": 'import { isDriver } from "redivivus:automation";\nvoid isDriver;\n' })],
    { ok: false, output: "redivivus:automation is for driver scripts" });
step("  but does in a driver script (--driver)", node, [cli, "check", "--driver", mod("gate-driver", { "main.ts": 'import { isDriver } from "redivivus:automation";\nvoid isDriver;\n' })], { ok: true });
step("an undeclared redivivus module does not compile", node, [cli, "check", mod("gate-none", { "main.ts": 'import * as nothing from "redivivus:nothing";\nvoid nothing;\n' })],
    { ok: false, output: "redivivus has no module 'nothing'" });

// redivivus:ui resolves in every state: main and the data stage entries may import it.
{
    const ui = 'import { h, type Child } from "redivivus:ui";\nexport function view(): Child { return <label text="x" />; }\n';
    step("redivivus:ui imported by main compiles", node, [cli, "check", mod("ui-main", { "main.tsx": 'import { h } from "redivivus:ui";\nexport const view = () => <label text="x" />;\n' })], { ok: true });
    step("  and through a module main imports", node, [cli, "check", mod("ui-chain", { "main.ts": `${G}import { view } from "./panel";\nexport function client(client: Client) { view(); void client; }\n`, "panel.tsx": ui })], { ok: true });
    step("  and from a data stage entry", node, [cli, "check", mod("ui-data", { "data.tsx": 'import { h } from "redivivus:ui";\nvoid (<label />);\n' })], { ok: true });
    step("lazyImport is gone from redivivus:shared", node, [cli, "check", mod("ui-lazy", { "main.ts": `${G}import { lazyImport } from "redivivus:shared";\nvoid lazyImport;\n` })],
        { ok: false, output: "lazyImport" });
    const created = path.join(scratch, "created-ui");
    step("rdvmod create --ui makes a mod with a TSX panel", node, [cli, "create", created, "--ui"], { ok: true });
    step("  which checks", node, [cli, "check", created], { ok: true });
}
step("a module of a mod that is no dependency does not compile", node, [cli, "check", mod("gate-dep", { "main.ts": 'import { greet } from "first-mod:greet";\nvoid greet;\n' })],
    { ok: false, output: "mod 'gate-dep' does not depend on 'first-mod'" });
step("there are no API globals", node, [cli, "check", mod("no-globals", { "main.ts": "log(1);\n" })], { ok: false, output: "Cannot find name 'log'" });

// A file without import or export is still a module: its top-level declarations stay locals.
{
    const dir = mod("no-imports", { "main.ts": 'import { log } from "redivivus:shared";\nconst leak = 5;\nfunction helper(): number { return leak; }\nlog(helper());\n' });
    step("a mod builds", node, [cli, "build", "--out", path.join(scratch, "out"), dir], { ok: true });
    const lua = fs.readFileSync(path.join(scratch, "out/no-imports/main.lua"), "utf8");
    expect("  its top-level const and function are locals, not globals of the mod", /^local leak = 5$/m.test(lua) && /^local function helper\(\)$/m.test(lua), lua);
    expect("  and it requires the TypeScriptToLua runtime from redivivus only when it uses it", !lua.includes("lualib"), lua);
}
expect("Script functions leave no __TS__Generator import behind", !fs.readFileSync(path.join(out, "first-mod/main.lua"), "utf8").includes("__TS__Generator"));
expect("TypeScriptToLua's runtime is required as redivivus:lualib", fs.readFileSync(path.join(out, "first-mod/main.lua"), "utf8").includes('require("redivivus:lualib")'));
expect("a dependency's declarations sit beside its Lua (mods/<name>/greet.d.ts)", fs.existsSync(path.join(out, "first-mod/greet.d.ts")));

// The engine enum generator on a small header of its own.
{
    const dir = fs.mkdtempSync(path.join(os.tmpdir(), "rdvmod-enums-"));
    const header = path.join(dir, "enums.h");
    const allow = path.join(dir, "allow.json");
    fs.writeFileSync(header, ["typedef uint32_t eFoo;", "namespace eFoo_ns {", "constexpr eFoo FOO_ONE = 1;", "constexpr eFoo FOO_TWO_WORDS = 2;",
        "constexpr eFoo FOO_COUNT = 4;", "}", ""].join("\n"));
    fs.writeFileSync(allow, JSON.stringify({ exported: [{ cpp: "eFoo", ts: "Foo", exclude: ["FOO_COUNT"] }], leftOut: {} }));
    const text = engineEnums(allow, header, dir);
    expect("enum generator: export const enum, prefix stripped, PascalCase, sentinel excluded",
        /export const enum Foo/.test(text) && /One = 1,/.test(text) && /TwoWords = 2,/.test(text) && !/Count/.test(text), text);
    fs.rmSync(dir, { recursive: true, force: true });
}

// The editor plugin, in a language service.
const pluginInit = createRequire(import.meta.url)(path.join(LUA, "api/ls-plugin/index.js"));
{
    const file = path.join(scratch, "ls", "main.ts");
    fs.mkdirSync(path.dirname(file), { recursive: true });
    const text = [
        'import type { Game } from "redivivus:server";',
        'import { type Script } from "redivivus:shared";',
        "function* act(): Script<number> { yield; return 1; }",
        "function* main(): Script {",
        "    act();",                   // 5: a bare call
        "    yield* [1, 2];",           // 6: yield* of no action
        "    const n: number = yield* act();",
        "    void n;",
        "}",
        "export function server(game: number) {",  // 10: wrong lifecycle parameter
        "    void game; void main;",
        "}",
        "",
    ].join("\n");
    fs.writeFileSync(file, text);
    const opts: ts.CompilerOptions = {
        strict: true, target: ts.ScriptTarget.ESNext, lib: ["lib.esnext.d.ts"], types: [], typeRoots: [path.join(TOOLS, "node_modules")],
        moduleDetection: ts.ModuleDetectionKind.Force, module: ts.ModuleKind.Preserve, moduleResolution: ts.ModuleResolutionKind.Bundler,
        paths: { "redivivus:*": [path.join(LUA, "api/redivivus/*")] },
    };
    const roots = [path.join(LUA, "api/lua/index.d.ts"), file];
    const host: ts.LanguageServiceHost = {
        getScriptFileNames: () => roots,
        getScriptVersion: () => "1",
        getScriptSnapshot: (f) => (fs.existsSync(f) ? ts.ScriptSnapshot.fromString(fs.readFileSync(f, "utf8")) : undefined),
        getCurrentDirectory: () => LUA,
        getCompilationSettings: () => opts,
        getDefaultLibFileName: (o) => ts.getDefaultLibFilePath(o),
        fileExists: (f) => fs.existsSync(f),
        readFile: (f) => fs.readFileSync(f, "utf8"),
    };
    const wrapped = pluginInit({ typescript: ts }).create({ languageService: ts.createLanguageService(host) } as unknown as ts.server.PluginCreateInfo) as ts.LanguageService;
    const ours = wrapped.getSemanticDiagnostics(file).filter((d) => d.source === "rdvmod");
    const lines = ours.map((d) => ts.getLineAndCharacterOfPosition(d.file!, d.start!).line + 1).sort((a, b) => a - b);
    expect("the editor plugin reports Script misuse and a wrong lifecycle export at the place", JSON.stringify(lines) === "[5,6,10]", `lines ${JSON.stringify(lines)}`);
}

// The editor plugin in a real tsserver, loaded from the mod project's own node_modules.
async function tsserverCheck(): Promise<void> {
    const project = path.join(scratch, "editor-project");
    fs.cpSync(path.join(EXAMPLES, "first-mod"), project, { recursive: true });
    fs.mkdirSync(path.join(project, "node_modules/@redivivus"), { recursive: true });
    fs.symlinkSync(path.join(LUA, "api"), path.join(project, "node_modules/@redivivus/api"), "dir");
    run(node, [cli, "tsconfig", project]);
    const mainFile = path.join(project, "main.ts");
    fs.writeFileSync(mainFile, fs.readFileSync(mainFile, "utf8") + "\nfunction* act(): Script<number> { yield; return 1; }\nexport function sever(game: Game) { act(); void game; }\n");
    const line = fs.readFileSync(mainFile, "utf8").split("\n").length - 1;
    const server = spawn(node, [path.join(TOOLS, "node_modules/typescript/lib/tsserver.js"), "--allowLocalPluginLoads", "--disableAutomaticTypingAcquisition"], { cwd: project });
    let buffer = "";
    const responses: Record<string, unknown>[] = [];
    server.stdout.on("data", (d: Buffer) => {
        buffer += d.toString("utf8");
        let m: RegExpExecArray | null;
        while ((m = /Content-Length: (\d+)\r\n\r\n/.exec(buffer))) {
            const start = m.index + m[0].length;
            const length = Number(m[1]);
            if (Buffer.byteLength(buffer.slice(start)) < length) break;
            const body = Buffer.from(buffer.slice(start)).subarray(0, length).toString("utf8");
            buffer = Buffer.from(buffer.slice(start)).subarray(length).toString("utf8");
            try {
                responses.push(JSON.parse(body));
            } catch {
                // not a message
            }
        }
    });
    const send = (seq: number, command: string, args: object) => server.stdin.write(JSON.stringify({ seq, type: "request", command, arguments: args }) + "\n");
    send(1, "open", { file: mainFile, projectRootPath: project });
    send(2, "semanticDiagnosticsSync", { file: mainFile });
    const deadline = Date.now() + 60000;
    while (Date.now() < deadline && !responses.some((r) => r.request_seq === 2)) await new Promise((r) => setTimeout(r, 100));
    server.kill();
    const diag = responses.find((r) => r.request_seq === 2) as { body?: { code: number; start: { line: number }; text: string }[] } | undefined;
    const ours = (diag?.body ?? []).filter((d) => d.code === 91001 || d.code === 91002);
    expect("tsserver loads @redivivus/api/ls-plugin from the mod project and reports at the line (Script misuse, an unknown lifecycle export)",
        ours.some((d) => d.code === 91001 && d.start.line === line) && ours.some((d) => d.code === 91002 && d.start.line === line),
        JSON.stringify(diag?.body ?? responses.slice(-3)).slice(0, 2000));
}

// pack.
function sha(file: string): string {
    return createHash("sha256").update(fs.readFileSync(file)).digest("hex");
}
function packChecks(): void {
    const packOut = path.join(scratch, "packs");
    step("pack a built mod folder", node, [cli, "pack", "--out", packOut, path.join(out, "first-mod")], { ok: true, output: "first-mod_0.1.0.mpq" });
    const file = path.join(packOut, "first-mod_0.1.0.mpq");
    const first = fs.existsSync(file) ? sha(file) : "";
    step("  again", node, [cli, "pack", "--out", packOut, path.join(out, "first-mod")], { ok: true });
    expect("  the same bytes both times (deterministic)", first !== "" && first === sha(file), first);
    const checked = step("pack --check validates it and prints its SHA-256", node, [cli, "pack", "--check", file], { ok: true, output: "profile and layout ok" });
    expect("  the hash it prints is the file's", checked.includes(first));
    const listed = run(node, ["-e", `import("${path.join(TOOLS, "src/mpq/index.ts")}").then(m => console.log(m.list(new Uint8Array(require("fs").readFileSync("${file}"))).map(f => f.name).join("\\n")))`]).output;
    expect("  every file is under mods\\first-mod\\ with backslash paths, the listfile complete", listed.includes("mods\\first-mod\\info.json") && listed.includes("mods\\first-mod\\main.lua")
        && listed.split("\n").filter((l) => l.trim()).every((l) => l.startsWith("mods\\first-mod\\")), listed);
    step("pack a plain-Lua mod", node, [cli, "pack", "--out", packOut, path.join(EXAMPLES, "lua-mod")], { ok: true, output: "lua-mod_0.1.0.mpq" });
    // A tree root with game assets.
    const tree = path.join(scratch, "tree");
    fs.cpSync(path.join(EXAMPLES, "lua-mod"), path.join(tree, "mods/lua-mod"), { recursive: true });
    fs.mkdirSync(path.join(tree, "data/global/ui"), { recursive: true });
    fs.writeFileSync(path.join(tree, "data/global/ui/panel.txt"), "asset");
    step("pack a tree root with a game asset under data\\", node, [cli, "pack", "--out", path.join(scratch, "packs2"), tree], { ok: true });
    // Rejections.
    const bad = (what: string, files: Record<string, string>, text: string) => {
        const dir = path.join(scratch, `badtree-${Object.keys(files).length}-${what.length}`);
        fs.rmSync(dir, { recursive: true, force: true });
        for (const [rel, body] of Object.entries(files)) {
            fs.mkdirSync(path.dirname(path.join(dir, rel)), { recursive: true });
            fs.writeFileSync(path.join(dir, rel), body);
        }
        const archive = path.join(scratch, `bad-${what.replace(/\W+/g, "-")}.mpq`);
        fs.writeFileSync(archive, buildMpq(Object.entries(files).map(([name, body]) => ({ name: name.split("/").join("\\"), data: new TextEncoder().encode(body) }))));
        step(`pack --check refuses ${what}`, node, [cli, "pack", "--check", archive], { ok: false, output: text });
        if (fs.existsSync(path.join(dir, "mods"))) {
            const spaces = fs.readdirSync(path.join(dir, "mods"));
            if (spaces.length === 1) step(`  and pack refuses to make it`, node, [cli, "pack", "--out", path.join(scratch, "packs3"), dir], { ok: false, output: text });
        }
    };
    const info = (n: string) => JSON.stringify({ name: n, version: "1.0.0" });
    bad("a file in another mod's namespace", { "mods/a/info.json": info("a"), "mods/b/x.lua": "" }, "more than one mod's namespace");
    bad("two info.json", { "mods/a/info.json": info("a"), "mods/b/info.json": info("b") }, "more than one info.json");
    bad("an info.json whose name is not the namespace", { "mods/a/info.json": info("b") }, "names the mod 'b', but its namespace is 'a'");
    bad("a file outside the allowed roots", { "mods/a/info.json": info("a"), "scripts/x.lua": "" }, "outside mods\\<name>\\ and outside the game asset roots");
    bad("files in mods\\redivivus\\", { "mods/redivivus/info.json": info("redivivus") }, "redivivus is built into the game");
    // Case collisions on a disk that has case.
    const caseDir = path.join(scratch, "case-mod");
    fs.mkdirSync(caseDir, { recursive: true });
    fs.writeFileSync(path.join(caseDir, "info.json"), info("case-mod"));
    fs.writeFileSync(path.join(caseDir, "Main.lua"), "");
    fs.writeFileSync(path.join(caseDir, "main.lua"), "");
    if (fs.readdirSync(caseDir).length === 3) {
        step("pack refuses two files that differ only by case", node, [cli, "pack", "--out", path.join(scratch, "packs4"), caseDir], { ok: false, output: "differ only by case" });
    } else {
        console.log("SKIP pack over a folder with two names differing by case: this disk does not keep them");
    }
    const caseProblems = layoutProblems(["mods\\a\\info.json", "mods\\a\\Main.lua", "mods\\a\\main.lua"], () => JSON.stringify({ name: "a" }));
    expect("the layout check refuses two names that differ only by case", caseProblems.some((p) => p.includes("differ only by case")), caseProblems.join("; "));
}

async function main(): Promise<void> {
    await tsserverCheck();
    packChecks();
    const hostTest = arg("--host-test");
    if (hostTest) {
        const exe = path.resolve(hostTest);
        const useWine = exe.endsWith(".exe");
        const output = step(`host test ${path.basename(exe)}${useWine ? " under wine" : ""}`, useWine ? "wine" : exe, [...(useWine ? [exe] : []), "--ts-mods", out], { ok: true });
        const summary = output.split("\n").filter((l) => /^\d+ passed, \d+ failed/.test(l));
        console.log(`  ${summary.join(" ") || "(no summary line)"}`);
    }
    fs.rmSync(scratch, { recursive: true, force: true });
    console.log(failed ? `${failed} step(s) failed, ${passed} passed` : `all ${passed} steps passed`);
    process.exit(failed ? 1 : 0);
}

void main();
