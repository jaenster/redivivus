// The whole TypeScript side of the Lua mod system, checked in one go:
//
//   node Redivivus/Lua/tools/src/selftest.ts [--out DIR] [--host-test BINARY]
//
//   1. the generated binding glue, docs and LuaLS files are up to date with the declarations
//   2. the example mods build (first-mod, then second-mod importing from it)
//   3. every side of the example mods and the type tests type-checks; each @ts-expect-error in
//      Redivivus/Lua/examples/type-tests must be a real error (an unused one is itself an error)
//   4. the build refuses a Script function used outside yield* (the plugin's own check)
//   5. with --host-test, the host test runs over the built mods (a .exe runs under wine)
// Exit code 0 when everything passes.

import { spawnSync } from "node:child_process";
import { createRequire } from "node:module";
import * as fs from "node:fs";
import * as os from "node:os";
import * as path from "node:path";
import ts from "typescript";
import { engineEnums } from "./enums.ts";

const TOOLS = path.resolve(import.meta.dirname, "..");
const LUA = path.resolve(TOOLS, "..");
const ROOT = path.resolve(LUA, "../..");
const EXAMPLES = path.join(LUA, "examples");

function arg(name: string): string | undefined {
    const i = process.argv.indexOf(name);
    return i >= 0 ? process.argv[i + 1] : undefined;
}

let failed = 0;
function step(what: string, command: string, args: string[], expect: { ok: boolean; output?: string }, cwd = ROOT): string {
    const run = spawnSync(command, args, { cwd, encoding: "utf8", maxBuffer: 64 * 1024 * 1024 });
    const output = (run.stdout ?? "") + (run.stderr ?? "");
    const ok = (run.status === 0) === expect.ok && (!expect.output || output.includes(expect.output));
    console.log(`${ok ? "PASS" : "FAIL"} ${what}`);
    if (!ok) {
        failed++;
        console.log(output.split("\n").slice(-40).join("\n"));
    }
    return output;
}

function expect(what: string, ok: boolean, detail = ""): void {
    console.log(`${ok ? "PASS" : "FAIL"} ${what}`);
    if (!ok) {
        failed++;
        if (detail) console.log(detail);
    }
}

const node = process.execPath;
const out = path.resolve(arg("--out") ?? fs.mkdtempSync(path.join(os.tmpdir(), "rdvmod-out-")));
const cli = path.join(TOOLS, "src/cli.ts");

step("generated glue, docs and LuaLS files are up to date", node, [path.join(TOOLS, "src/gen.ts"), "--check"], { ok: true });
step("example mods build", node, [cli, "build", "--out", out, path.join(EXAMPLES, "first-mod"), path.join(EXAMPLES, "second-mod")], { ok: true });
step("example mods and type tests type-check (every @ts-expect-error is a real error)", node,
    [cli, "check", path.join(EXAMPLES, "first-mod"), path.join(EXAMPLES, "second-mod"), path.join(EXAMPLES, "type-tests")], { ok: true });

// The same type tests with every @ts-expect-error blanked out: each line under one must now be
// reported, so no case passes by being an error of the wrong kind somewhere else.
{
    const copy = fs.mkdtempSync(path.join(os.tmpdir(), "rdvmod-tt-"));
    const tt = path.join(copy, "type-tests");
    fs.cpSync(path.join(EXAMPLES, "type-tests"), tt, { recursive: true });
    const expected: string[] = [];
    const walk = (dir: string) => {
        for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
            const full = path.join(dir, e.name);
            if (e.isDirectory()) walk(full);
            else if (full.endsWith(".ts")) {
                const lines = fs.readFileSync(full, "utf8").split("\n");
                lines.forEach((l, i) => {
                    if (!/^\s*\/\/ @ts-expect-error/.test(l)) return;
                    lines[i] = "//";
                    expected.push(`${path.relative(tt, full).split(path.sep).join("/")}:${i + 2}`);
                });
                fs.writeFileSync(full, lines.join("\n"));
            }
        }
    };
    walk(tt);
    const run = spawnSync(node, [cli, "check", path.join(EXAMPLES, "first-mod"), tt], { cwd: ROOT, encoding: "utf8", maxBuffer: 64 * 1024 * 1024 });
    const text = ((run.stdout ?? "") + (run.stderr ?? "")).replace(/\x1b\[[0-9;]*m/g, "");
    const reported = new Set([...text.matchAll(/type-tests\/([\w/.-]+\.ts):(\d+):\d+/g)].map((m) => `${m[1]}:${m[2]}`));
    const missing = expected.filter((e) => !reported.has(e));
    expect(`without their @ts-expect-error, all ${expected.length} must-fail cases are reported at their line`,
        expected.length > 0 && missing.length === 0, `not reported: ${missing.join(", ")}`);
    fs.rmSync(copy, { recursive: true, force: true });
}

const bad = fs.mkdtempSync(path.join(os.tmpdir(), "rdvmod-bad-"));
const badMod = path.join(bad, "bad-mod");
fs.mkdirSync(path.join(badMod, "server"), { recursive: true });
fs.writeFileSync(path.join(badMod, "info.json"), JSON.stringify({ name: "bad-mod", version: "0.1.0" }));
fs.writeFileSync(path.join(badMod, "server/control.ts"),
    "function* act(): Script<integer> { yield; return 1; }\nscript.on_event(\"on_tick\", () => { act(); });\n");
step("a Script function called outside yield* is refused", node, [cli, "build", "--out", path.join(bad, "out"), badMod],
    { ok: false, output: "A Script function runs only as" });
fs.writeFileSync(path.join(badMod, "server/control.ts"),
    "function* act(): Script<integer> { yield* [1, 2]; return 1; }\nscript.start(act);\n");
step("yield* of something other than an action is refused in a Script function", node, [cli, "build", "--out", path.join(bad, "out"), badMod],
    { ok: false, output: "yield* takes a call of a Script function" });

// A file without import or export is still a module: its top-level declarations stay locals.
const noImports = path.join(bad, "no-imports");
fs.mkdirSync(path.join(noImports, "server"), { recursive: true });
fs.writeFileSync(path.join(noImports, "info.json"), JSON.stringify({ name: "no-imports", version: "0.1.0" }));
fs.writeFileSync(path.join(noImports, "server/control.ts"), "const leak = 5;\nfunction helper(): number { return leak; }\nlog(helper());\n");
step("a mod file without imports builds", node, [cli, "build", "--out", path.join(bad, "out"), noImports], { ok: true });
const lua = fs.readFileSync(path.join(bad, "out/no-imports/server/control.lua"), "utf8");
expect("  its top-level const and function are locals, not globals of the mod", /^local leak = 5$/m.test(lua) && /^local function helper\(\)$/m.test(lua), lua);

// Script functions need no generator runtime; an ordinary generator in the same file still gets it.
const builtLua = (rel: string) => fs.readFileSync(path.join(out, rel), "utf8");
expect("Script functions leave no __TS__Generator import behind",
    !builtLua("first-mod/server/control.lua").includes("__TS__Generator") && !builtLua("first-mod/shared/actions.lua").includes("__TS__Generator"));
fs.writeFileSync(path.join(noImports, "server/control.ts"),
    "function* act(): Script<integer> { yield; return 1; }\nfunction* plain() { yield 1; }\nscript.start(function* (): Script { yield* act(); });\nlog([...plain()].length);\n");
step("a mod with both kinds of generator builds", node, [cli, "build", "--out", path.join(bad, "out"), noImports], { ok: true });
expect("  the ordinary generator keeps its runtime", fs.readFileSync(path.join(bad, "out/no-imports/server/control.lua"), "utf8").includes("__TS__Generator("));

// require is not part of a mod's Lua library (modules are imported); it is not even declared.
fs.writeFileSync(path.join(noImports, "server/control.ts"), "require(\"x\");\n");
step("require called as a Lua function does not compile", node, [cli, "check", noImports], { ok: false, output: "Cannot find name 'require'" });
fs.rmSync(bad, { recursive: true, force: true });

// The engine enum generator on a small header of its own: names, values, and the cases that must fail.
{
    const dir = fs.mkdtempSync(path.join(os.tmpdir(), "rdvmod-enums-"));
    const header = path.join(dir, "enums.h");
    const allow = path.join(dir, "allow.json");
    const writeHeader = (fooValue: number) => fs.writeFileSync(header, [
        "typedef uint32_t eFoo;", "namespace eFoo_ns {", "constexpr eFoo FOO_ONE = 1;", `constexpr eFoo FOO_TWO_WORDS = ${fooValue};`,
        "constexpr eFoo FOO_MixedCase = 3;", "constexpr eFoo FOO_COUNT = 4;", "}",
        "typedef int eBar;", "namespace eBar_ns {", "constexpr eBar BAR_A = -1;", "constexpr eBar BAR_0x10 = 16;", "}", "",
    ].join("\n"));
    const run = (list: object): string => {
        fs.writeFileSync(allow, JSON.stringify(list));
        try {
            return engineEnums(allow, header, dir);
        } catch (e) {
            return `ERROR ${(e as Error).message}`;
        }
    };
    writeHeader(2);
    const good = run({ exported: [{ cpp: "eFoo", ts: "Foo", exclude: ["FOO_COUNT"] }], leftOut: { eBar: "test" } });
    expect("enum generator: prefix stripped, PascalCase, values from the header, sentinel excluded",
        /One = 1,/.test(good) && /TwoWords = 2,/.test(good) && /MixedCase = 3,/.test(good) && !/Count/.test(good), good);
    writeHeader(20);
    expect("  a value is never renumbered: the output follows the header",
        /TwoWords = 20,/.test(run({ exported: [{ cpp: "eFoo", ts: "Foo", exclude: ["FOO_COUNT"] }], leftOut: {} })));
    expect("  a member that is no identifier fails", run({ exported: [{ cpp: "eBar", ts: "Bar" }], leftOut: {} }).includes("not an identifier"));
    expect("  two members with one name fail",
        run({ exported: [{ cpp: "eFoo", ts: "Foo", rename: { FOO_ONE: "MixedCase" } }], leftOut: {} }).includes("both become Foo.MixedCase"));
    expect("  two enums with one name fail",
        run({ exported: [{ cpp: "eFoo", ts: "Foo" }, { cpp: "eBar", ts: "Foo", exclude: ["BAR_0x10"] }], leftOut: {} }).includes("two enums are called Foo"));
    expect("  an enum the header lacks fails", run({ exported: [{ cpp: "eNope", ts: "Nope" }], leftOut: {} }).includes("is not an enum"));
    expect("  a left-out enum the header lacks fails", run({ exported: [], leftOut: { eNope: "x" } }).includes("does not have"));
    expect("  negative values stay negative", /A = -1,/.test(run({ exported: [{ cpp: "eBar", ts: "Bar", exclude: ["BAR_0x10"] }], leftOut: {} })));
    fs.rmSync(dir, { recursive: true, force: true });
}

// The language service plugin shows the build's Script errors in an editor, at the call.
{
    const file = path.join(os.tmpdir(), "rdvmod-ls-check.ts");
    const text = [
        "function* act(): Script<integer> { yield; return 1; }",
        "function* main(): Script {",
        "    act();",                   // line 3: a bare call
        "    yield act();",             // line 4: yield without the star
        "    yield* [1, 2];",           // line 5: yield* of something that is no action
        "    const n: integer = yield* act();",
        "    void n;",
        "}",
        "script.start(main);",
        "script.start(function* () { act(); });", // line 10, contextually typed
        "",
    ].join("\n");
    const roots = [path.join(LUA, "api/server.d.ts"), file];
    const host: ts.LanguageServiceHost = {
        getScriptFileNames: () => roots,
        getScriptVersion: () => "1",
        getScriptSnapshot: (f) => ts.ScriptSnapshot.fromString(f === file ? text : fs.readFileSync(f, "utf8")),
        getCurrentDirectory: () => LUA,
        getCompilationSettings: () => ({ strict: true, target: ts.ScriptTarget.ESNext, lib: ["lib.esnext.d.ts"], types: [], typeRoots: [path.join(TOOLS, "node_modules")], moduleDetection: ts.ModuleDetectionKind.Force }),
        getDefaultLibFileName: (o) => ts.getDefaultLibFilePath(o),
        fileExists: (f) => f === file || fs.existsSync(f),
        readFile: (f) => (f === file ? text : fs.readFileSync(f, "utf8")),
    };
    const ls = ts.createLanguageService(host);
    const init = createRequire(import.meta.url)(path.join(LUA, "api/ls-plugin/index.js"));
    const wrapped = init({ typescript: ts }).create({ languageService: ls } as unknown as ts.server.PluginCreateInfo) as ts.LanguageService;
    const ours = wrapped.getSemanticDiagnostics(file).filter((d) => d.code === 91001);
    const lines = ours.map((d) => ts.getLineAndCharacterOfPosition(d.file!, d.start!).line + 1).sort((a, b) => a - b);
    expect("the language service plugin reports Script misuse at the call (bare call, yield without star, yield* of no action, in a callback)",
        JSON.stringify(lines) === "[3,4,5,10]", `lines ${JSON.stringify(lines)}`);
    expect("  and nothing for correct use", !ours.some((d) => ts.getLineAndCharacterOfPosition(d.file!, d.start!).line + 1 === 6));
}

const hostTest = arg("--host-test");
if (hostTest) {
    const exe = path.resolve(hostTest);
    const useWine = exe.endsWith(".exe");
    const output = step(`host test ${path.basename(exe)}${useWine ? " under wine" : ""}`, useWine ? "wine" : exe,
        [...(useWine ? [exe] : []), "--ts-mods", out], { ok: true });
    const summary = output.split("\n").filter((l) => /^\d+ passed, \d+ failed/.test(l));
    console.log(`  ${summary.join(" ") || "(no summary line)"}`);
}

console.log(failed ? `${failed} step(s) failed` : "all steps passed");
process.exit(failed ? 1 : 0);
