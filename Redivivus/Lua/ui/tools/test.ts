// Builds, then runs every test: the runtime tests in Node and under LuaJIT (same sources), the
// Node-only host tests, the draw lists compared between the two, and the golden images.
// UPDATE_GOLDEN=1 rewrites the test-theme golden PNGs and the stock-theme hashes.

import { spawnSync } from "node:child_process";
import * as path from "node:path";
import { pathToFileURL } from "node:url";
import { buildLua, buildNode, OUT_LUA, OUT_NODE, ROOT } from "./build.ts";

interface Report {
    passed: number;
    failed: number;
    lines: string[];
}

const t0 = Date.now();
buildNode();
buildLua();
let failed = 0;
const summary: string[] = [];

function show(title: string, r: Report, quiet: boolean): void {
    for (const l of r.lines) if (!quiet || l.startsWith("FAIL")) console.log(`  ${l}`);
    console.log(`${title}: ${r.passed} passed, ${r.failed} failed`);
    summary.push(`${title}: ${r.passed} passed, ${r.failed} failed`);
    failed += r.failed;
}

const quiet = !process.argv.includes("-v");
const imp = async <T>(rel: string): Promise<T> => (await import(pathToFileURL(path.join(OUT_NODE, rel)).href)) as T;

// 1. Runtime tests in Node.
const rt = await imp<{ runAll(): Report }>("ui/test/rt/main.js");
show("runtime (node)", rt.runAll(), quiet);

// 2. The same tests under LuaJIT.
const lua = spawnSync("luajit", [path.join(ROOT, "test/lua/run.lua"), OUT_LUA, "tests"], { encoding: "utf8" });
if (lua.error !== undefined) {
    console.log(`runtime (luajit): SKIPPED (${lua.error.message})`);
    summary.push("runtime (luajit): skipped");
} else {
    const lines = lua.stdout.trim().split("\n");
    const tail = lines.pop() ?? "";
    const m = /(\d+) passed, (\d+) failed/.exec(tail);
    const r: Report = { passed: m ? Number(m[1]) : 0, failed: m ? Number(m[2]) : 1, lines };
    if (lua.status !== 0 && r.failed === 0) {
        r.failed = 1;
        r.lines.push(`FAIL luajit exited ${lua.status}: ${lua.stderr}`);
    }
    show("runtime (luajit)", r, quiet);
}

// 3. Host tests (Node only), draw lists Node vs LuaJIT, golden images.
const harness = await imp<{ clear(): void; runAll(): Report }>("ui/test/rt/harness.js");
harness.clear();
const host = await imp<{ goldenTests(): Promise<Report>; luaDrawListTest(): Report; skipped: string[] }>("ui/test/host/host.test.js");
show("host", harness.runAll(), quiet);
show("draw lists node = luajit", host.luaDrawListTest(), false);
show("golden images", await host.goldenTests(), quiet);
for (const s of host.skipped) console.log(`SKIPPED ${s}`);

console.log(`\n${summary.join("\n")}\n${failed === 0 ? "all passed" : `${failed} FAILED`} in ${Date.now() - t0} ms`);
process.exitCode = failed === 0 ? 0 : 1;
