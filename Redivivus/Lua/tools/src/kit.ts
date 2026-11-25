// The kit (Redivivus/Lua/kit, TypeScript) compiled into the base mod: kit/<side>/x.ts becomes
// mods/base/<side>/kit/x.lua and, for TypeScript mods, x.d.ts beside it. gen.ts writes these with
// its other outputs (and checks them with --check), so the committed Lua is what the sources give.
//
// Kit modules run in the environment of the mod that requires them, so they name each other by
// absolute module name ("__base__/shared/kit/channels"); here those names resolve to the kit's
// sources.

import * as fs from "node:fs";
import * as path from "node:path";
import ts from "typescript";
import * as tstl from "typescript-to-lua";
import { makePlugin } from "./plugin.ts";

const TOOLS = path.resolve(import.meta.dirname, "..");
const LUA = path.resolve(TOOLS, "..");
const KIT = path.join(LUA, "kit");
const BASE = path.join(LUA, "mods/base");
const SIDES = ["shared", "server", "client"] as const;

export class KitError extends Error {}

function sources(side: string): string[] {
    const dir = path.join(KIT, side);
    if (!fs.existsSync(dir)) return [];
    return fs.readdirSync(dir).filter((f) => f.endsWith(".ts") && !f.endsWith(".d.ts")).sort().map((f) => path.join(dir, f));
}

function options(outDir: string): tstl.CompilerOptions {
    return {
        target: ts.ScriptTarget.ESNext,
        lib: ["lib.esnext.d.ts"],
        module: ts.ModuleKind.Preserve,
        moduleResolution: ts.ModuleResolutionKind.Bundler,
        moduleDetection: ts.ModuleDetectionKind.Force,
        strict: true,
        skipLibCheck: true,
        types: [],
        typeRoots: [path.join(TOOLS, "node_modules")],
        paths: Object.fromEntries([
            ...SIDES.map((s) => [`__base__/${s}/kit/*`, [path.join(KIT, s, "*")]]),
            ["__base__/*", [path.join(BASE, "*")]],
        ]),
        rootDir: KIT,
        outDir,
        luaTarget: tstl.LuaTarget.LuaJIT,
        luaLibImport: tstl.LuaLibImportKind.Require,
        noImplicitSelf: true,
        noResolvePaths: ["__*__/**"],
        noHeader: true,
        luaPlugins: [{ plugin: makePlugin(KIT) }],
    };
}

function diagnosticsText(diags: readonly ts.Diagnostic[]): string {
    const host: ts.FormatDiagnosticsHost = { getCanonicalFileName: (f) => f, getCurrentDirectory: () => LUA, getNewLine: () => "\n" };
    return ts.formatDiagnostics(diags, host);
}

// Every output for the base mod, by absolute path.
export function kitOutputs(): Map<string, string> {
    const out = new Map<string, string>();
    for (const side of SIDES) {
        const files = sources(side);
        if (!files.length) continue;
        const roots = [path.join(LUA, "api", `${side}.d.ts`), ...files];
        const opts = options("/kit-out");
        const program = ts.createProgram(roots, { ...opts, noEmit: true });
        const diags = ts.getPreEmitDiagnostics(program).filter((d) => d.category === ts.DiagnosticCategory.Error);
        if (diags.length) throw new KitError(`the kit does not type-check:\n${diagnosticsText(diags)}`);
        const own = new Set(files.map((f) => path.resolve(f)));
        const collected = new Map<string, string>();
        const result = tstl.transpileFiles(roots, { ...opts, sourceMap: false }, (p, text) => collected.set(p, text));
        const errors = result.diagnostics.filter((d) => d.category === ts.DiagnosticCategory.Error);
        if (errors.length) throw new KitError(`the kit does not compile:\n${diagnosticsText(errors)}`);
        for (const [p, text] of collected) {
            const rel = path.relative("/kit-out", p);
            const [first, ...rest] = rel.split(path.sep);
            if (first !== side || !p.endsWith(".lua")) continue;
            const source = path.join(KIT, side, rest.join(path.sep).replace(/\.lua$/, ".ts"));
            if (!own.has(path.resolve(source))) continue;
            const banner = `-- Generated from Redivivus/Lua/kit/${side}/${path.basename(source)} by Redivivus/Lua/tools/src/gen.ts. Do not edit.\n`;
            out.set(path.join(BASE, side, "kit", ...rest), banner + text);
        }
        // Declarations for TypeScript mods.
        const dtsProgram = ts.createProgram(roots, { ...opts, noEmit: false, declaration: true, emitDeclarationOnly: true, outDir: "/kit-dts" });
        dtsProgram.emit(undefined, (p, text) => {
            const rel = path.relative("/kit-dts", p);
            const [first, ...rest] = rel.split(path.sep);
            if (first !== side) return;
            const source = path.join(KIT, side, rest.join(path.sep).replace(/\.d\.ts$/, ".ts"));
            if (!own.has(path.resolve(source))) return;
            out.set(path.join(BASE, side, "kit", ...rest), `// Generated from Redivivus/Lua/kit/${side}/${path.basename(source)}. Do not edit.\n${text}`);
        });
    }
    return out;
}

// Files under mods/base/<side>/kit that no source gives any more.
export function staleKitFiles(outputs: Map<string, string>): string[] {
    const stale: string[] = [];
    for (const side of SIDES) {
        const dir = path.join(BASE, side, "kit");
        if (!fs.existsSync(dir)) continue;
        for (const f of fs.readdirSync(dir)) {
            const full = path.join(dir, f);
            if (!outputs.has(full)) stale.push(full);
        }
    }
    return stale;
}
