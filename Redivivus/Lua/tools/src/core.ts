// The core (Redivivus/Lua/core, TypeScript) built for the game: compiled to Lua with
// TypeScriptToLua and embedded in the exe as generated C++ (generated/d2_lua_core.gen.cpp), with
// the table of the public modules the resolver serves as `redivivus:<entry>`. The public modules'
// declarations, emitted by the compiler from the same source, are the @redivivus/api package
// (api/redivivus/*.d.ts), so the types mods compile against are the code that runs.
//
// Chunks:
//   internal.* and redivivus.*   the core's modules, by the names TypeScriptToLua's requires use
//   ui.*                         the UI kit (core/ui), which redivivus:ui re-exports
//   lualib_bundle                TypeScriptToLua's runtime: the core's own instance, and the one
//                                every mod gets as redivivus:lualib (an instance per mod, in the
//                                mod's own environment)
// internal.context, the redivivus.* modules and the kit (ui.*) are instantiated per mod
// (core/internal/context.ts).

import * as fs from "node:fs";
import * as path from "node:path";
import ts from "typescript";
import * as tstl from "typescript-to-lua";

const TOOLS = path.resolve(import.meta.dirname, "..");
const LUA = path.resolve(TOOLS, "..");
const ROOT = path.resolve(LUA, "../..");
export const CORE = path.join(LUA, "core");
const LUALIB = path.join(TOOLS, "node_modules/typescript-to-lua/dist/lualib/universal/lualib_bundle.lua");

export class CoreError extends Error {}

function walk(dir: string, out: string[] = []): string[] {
    for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
        const full = path.join(dir, e.name);
        if (e.isDirectory()) walk(full, out);
        else if (e.name.endsWith(".ts")) out.push(full);
    }
    return out.sort();
}

/** Every TypeScript file of the core: sources and the native spec. */
export function coreFiles(): string[] {
    return walk(CORE);
}

export function coreOptions(): tstl.CompilerOptions {
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
        rootDir: CORE,
        luaTarget: tstl.LuaTarget.LuaJIT,
        luaLibImport: tstl.LuaLibImportKind.Require,
        noImplicitSelf: true,
        noHeader: true,
    };
}

function diagnosticsText(diags: readonly ts.Diagnostic[]): string {
    const host: ts.FormatDiagnosticsHost = { getCanonicalFileName: (f) => f, getCurrentDirectory: () => ROOT, getNewLine: () => "\n" };
    return ts.formatDiagnostics(diags, host);
}

/** A public module: what the resolver serves as redivivus:<entry>. */
export type PublicModule = {
    entry: string;
    chunk: string;
    sides: string[];
    grant: "" | "test" | "driver" | "manager";
    values: string[];
    file: string;
};

const GRANTS: Record<string, "test" | "driver" | "manager"> = { test: "test", automation: "driver", manager: "manager" };

function sidesOfFile(sf: ts.SourceFile): string[] {
    const m = /^\s*\/\*\*([\s\S]*?)\*\//.exec(sf.text);
    const side = m && /@side\s+([^\n*]+)/.exec(m[1]);
    if (!side) throw new CoreError(`${path.relative(ROOT, sf.fileName)}: a public module needs @side in its first comment`);
    return side[1].trim().split(/\s+/);
}

/** The public modules, read from the core's sources (core/redivivus/*.ts but engine-enums). */
export function publicModules(program: ts.Program): PublicModule[] {
    const checker = program.getTypeChecker();
    const out: PublicModule[] = [];
    for (const file of fs.readdirSync(path.join(CORE, "redivivus")).filter((f) => f.endsWith(".ts")).sort()) {
        const entry = file.replace(/\.ts$/, "");
        if (entry === "engine-enums") continue;
        const full = path.join(CORE, "redivivus", file);
        const sf = program.getSourceFile(full);
        if (!sf) throw new CoreError(`${file} is not in the core program`);
        const sym = checker.getSymbolAtLocation(sf);
        const values: string[] = [];
        for (const e of sym ? checker.getExportsOfModule(sym) : []) {
            const target = e.flags & ts.SymbolFlags.Alias ? checker.getAliasedSymbol(e) : e;
            if (target.flags & (ts.SymbolFlags.Variable | ts.SymbolFlags.Function | ts.SymbolFlags.Enum)) values.push(e.name);
        }
        values.sort();
        out.push({ entry, chunk: entry === "lualib" ? "lualib_bundle" : `redivivus.${entry}`, sides: sidesOfFile(sf), grant: GRANTS[entry] ?? "", values, file: full });
    }
    return out;
}

export function cLiteral(text: string): string[] {
    const lines = text.split("\n");
    return lines.map((l, i) => "    " + JSON.stringify(l + (i < lines.length - 1 ? "\n" : "")).replace(/\?\?/g, "?\\?"));
}

export type CoreBuild = { outputs: Map<string, string>; modules: PublicModule[] };

/** Compile the core: the embedded chunks, the module table, the public declarations. */
export function buildCore(luaEnv: string): CoreBuild {
    const files = coreFiles();
    const options = coreOptions();
    const roots = [luaEnv, ...files];
    const program = ts.createProgram(roots, { ...options, noEmit: true });
    const diags = ts.getPreEmitDiagnostics(program).filter((d) => d.category === ts.DiagnosticCategory.Error);
    if (diags.length) throw new CoreError(`the core does not type-check:\n${diagnosticsText(diags)}`);
    const modules = publicModules(program);

    // Lua.
    const chunks = new Map<string, string>();
    const result = tstl.transpileFiles(roots, { ...options, outDir: "/core-out", sourceMap: false }, (p, text) => {
        if (!p.endsWith(".lua")) return;
        const rel = path.relative("/core-out", p).split(path.sep).join("/");
        if (rel === "lualib_bundle.lua") return;
        chunks.set(rel.replace(/\.lua$/, "").replace(/\//g, "."), text);
    });
    const errors = result.diagnostics.filter((d) => d.category === ts.DiagnosticCategory.Error);
    if (errors.length) throw new CoreError(`the core does not compile:\n${diagnosticsText(errors)}`);
    chunks.set("lualib_bundle", fs.readFileSync(LUALIB, "utf8"));
    for (const name of chunks.keys()) {
        if (!/^((internal|redivivus)\.[a-z-]+|ui(\.[a-z0-9-]+)+)$/.test(name) && name !== "lualib_bundle") throw new CoreError(`unexpected core chunk ${name}`);
    }

    const cpp: string[] = [
        "// Generated by Redivivus/Lua/tools/src/gen.ts from Redivivus/Lua/core (TypeScript, compiled with",
        "// TypeScriptToLua). Do not edit; change the core and run the generator.",
        "#if D2_FEATURE_MODS",
        '#include "Redivivus/Lua/d2_lua_core.h"',
        "",
    ];
    const names = [...chunks.keys()].sort();
    names.forEach((name, i) => {
        cpp.push(`// ${name}`, `static const char kChunk${i}[] =`, ...cLiteral(chunks.get(name)!), "    ;", "");
    });
    cpp.push("const D2LuaCoreChunk g_aD2LuaCoreChunks[] = {");
    names.forEach((name, i) => cpp.push(`    { ${JSON.stringify(name)}, kChunk${i}, sizeof(kChunk${i}) - 1 },`));
    cpp.push("    { nullptr, nullptr, 0 },", "};", "");
    const sideBits: Record<string, string> = { data: "D2LUA_SIDE_DATA", server: "D2LUA_SIDE_SERVER", client: "D2LUA_SIDE_CLIENT" };
    const grantBits: Record<string, string> = { "": "0", test: "D2LUA_GRANT_TEST", driver: "D2LUA_GRANT_DRIVER", manager: "D2LUA_GRANT_MANAGER" };
    modules.forEach((m, i) => {
        cpp.push(`static const char* const kValues${i}[] = { ${[...m.values.map((v) => JSON.stringify(v)), "nullptr"].join(", ")} };`);
    });
    cpp.push("", "// The public modules: redivivus:<entry>, the states it resolves in, the grant it needs, its exported values.");
    cpp.push("const D2LuaCoreModule g_aD2LuaCoreModules[] = {");
    modules.forEach((m, i) => {
        if (!chunks.has(m.chunk)) throw new CoreError(`public module ${m.entry} has no chunk ${m.chunk}`);
        cpp.push(`    { ${JSON.stringify(m.entry)}, ${JSON.stringify(m.chunk)}, ${m.sides.map((s) => sideBits[s]).join(" | ")}, ${grantBits[m.grant]}, kValues${i} },`);
    });
    cpp.push("    { nullptr, nullptr, 0, 0, nullptr },", "};", "#endif", "");

    const outputs = new Map<string, string>();
    outputs.set(path.join(LUA, "generated/d2_lua_core.gen.cpp"), cpp.join("\n"));

    // Declarations of the public modules, and the native spec files they refer to.
    const dts = ts.createProgram(roots, { ...options, noEmit: false, declaration: true, emitDeclarationOnly: true, outDir: "/core-dts" });
    const publicFiles = new Set(modules.map((m) => path.resolve(m.file)));
    const referenced = new Set<string>();
    dts.emit(undefined, (p, text) => {
        const rel = path.relative("/core-dts", p).split(path.sep).join("/");
        const source = path.join(CORE, rel.replace(/\.d\.ts$/, ".ts"));
        if (!publicFiles.has(path.resolve(source)) && !rel.startsWith("redivivus/engine-enums") && !rel.startsWith("ui/")) return;
        for (const m of text.matchAll(/from "(\.\.?\/[^"]+)"/g)) {
            const target = path.posix.normalize(path.posix.join(path.posix.dirname(rel), m[1]));
            if (target.startsWith("internal/")) throw new CoreError(`${rel}: the public declarations refer to ${target}; keep internal types out of the API`);
            if (target.startsWith("native/")) referenced.add(target);
        }
        outputs.set(path.join(LUA, "api", rel), `// Generated from Redivivus/Lua/core/${rel.replace(/\.d\.ts$/, ".ts")} by Redivivus/Lua/tools/src/gen.ts. Do not edit.\n${text}`);
    });
    for (const rel of referenced) {
        const source = path.join(CORE, `${rel}.d.ts`);
        if (!fs.existsSync(source)) throw new CoreError(`the public declarations refer to ${rel}, which is not a native spec file`);
        outputs.set(path.join(LUA, "api", `${rel}.d.ts`), `// Copied from Redivivus/Lua/core/${rel}.d.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.\n${fs.readFileSync(source, "utf8")}`);
    }
    return { outputs, modules };
}
