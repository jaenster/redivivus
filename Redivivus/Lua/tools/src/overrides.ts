// The override registry: every named function of the compiled tree, read from the tree's own
// definitions, as
//   generated/override/<Source unit>.ov.cpp   the unit itself (#included) and its table: an entry
//                                             per function it defines, built by the templates of
//                                             d2_lua_override_gen.h from the function's own type
//   generated/override/index.gen.cpp          the list of the tables
//   core/redivivus/engine-functions.ts        the engine object's types: engine.<module>.<namespace>.
//                                             <function> with override((next, ...args) => ...)
// A mods build compiles each Source unit through its .ov.cpp (CMakeLists.txt), so static functions
// are in reach and nothing outside the unit's own declarations is needed.
//
// Which functions: the ones each unit's source map (<unit>.cpp.map) lists, i.e. the functions the
// unit defines, plus the inline ones it owns from a header. What the generator reads of each: the
// namespace it is defined in, its parameters and their types, its calling convention, static,
// inline and always_inline, variadic. The kinds it derives (number, boolean, unit, pointer,
// reference, or a record by value) are checked by the compiler: every entry static_asserts them
// against what the templates see, so a wrong guess fails the build instead of a call.

import * as fs from "node:fs";
import * as path from "node:path";
import { type FunctionHead, type Param, scan, type ScanResult } from "./cppscan.ts";

export class OverrideError extends Error {}

// Units the Windows build does not compile (CMakeLists.txt filters the same directories).
const EXCLUDED = /^Source\/(D2OpenGL|MacSpecific|StormMac|D2Rave|D2Uni|D2Server)\//;

// The modules of a dedicated server (CMakeLists.txt D2_SERVER_MODULES) run on both sides; D2Game is
// the server's own; everything else is the client's.
const BOTH = new Set(["Fog", "Storm", "D2Hell", "D2Lang", "D2Net", "D2CMP", "D2Common"]);
const SERVER_ONLY = new Set(["D2Game"]);
export const SIDE_SERVER = 2;
export const SIDE_CLIENT = 4;

export function sideOfModule(module: string): number {
    if (SERVER_ONLY.has(module)) return SIDE_SERVER;
    if (BOTH.has(module)) return SIDE_SERVER | SIDE_CLIENT;
    return SIDE_CLIENT;
}

export const REFUSAL = {
    OK: 0, VARIADIC: 1, BY_VALUE: 2, INLINED: 3, HEADER_STATIC: 4, MEMBER: 5, NO_ENTRY: 6, TOO_MANY: 7,
} as const;
export const REFUSAL_TEXT: Record<number, string> = {
    1: "it takes a variable argument list (...), which a thunk cannot pass on",
    2: "a parameter or its result is a record passed by value",
    3: "it is inlined (always_inline) at its call sites, so an override would miss them",
    4: "it is static in a header: every unit that includes it has its own copy",
    5: "it is a member function",
    7: "it has more parameters than an override frame holds",
};
const MAX_ARGS = 24;

// --- types ---------------------------------------------------------------------------------------------

// Names the Windows headers and the C library define, which the tree's headers do not.
const SYSTEM_NUMBERS = new Set(("int unsigned signed char short long float double size_t ssize_t intptr_t uintptr_t ptrdiff_t "
    + "int8_t int16_t int32_t int64_t uint8_t uint16_t uint32_t uint64_t __int8 __int16 __int32 __int64 wchar_t char16_t char32_t "
    + "BOOL BOOLEAN BYTE WORD DWORD QWORD DWORD64 DWORDLONG UINT ULONG LONG INT SHORT USHORT UCHAR CHAR WCHAR FLOAT DOUBLE TCHAR "
    + "DWORD_PTR UINT_PTR LONG_PTR ULONG_PTR INT_PTR SIZE_T WPARAM LPARAM LRESULT HRESULT ATOM COLORREF LCID LANGID MMRESULT "
    + "SOCKET u_long u_short u_int u_char LONGLONG ULONGLONG time_t clock_t off_t fpos_t errno_t HFILE MCIERROR MCIDEVICEID "
    + "FOURCC HCALL WINBOOL DWORD32 UINT32 INT32 UINT16 INT16 UINT8 INT8 UINT64 INT64 LCTYPE CALID CALTYPE NTSTATUS GLenum GLint "
    + "GLuint GLsizei GLfloat GLboolean GLbitfield GLclampf GLdouble GLubyte GLshort GLushort GLbyte GLclampd").split(/\s+/));
const SYSTEM_POINTERS = /^(H[A-Z0-9]+|LP[A-Z0-9_]+|P[A-Z][A-Z0-9_]*|FARPROC|NEARPROC|PROC|WNDPROC|DLGPROC|TIMERPROC|HOOKPROC|LPTHREAD_START_ROUTINE|va_list|REFIID|REFGUID|REFCLSID)$/;
const SYSTEM_RECORDS = new Set(["RECT", "POINT", "SIZE", "MSG", "GUID", "IID", "CLSID", "LARGE_INTEGER", "ULARGE_INTEGER", "FILETIME",
    "SYSTEMTIME", "CRITICAL_SECTION", "OVERLAPPED", "WIN32_FIND_DATAA", "PALETTEENTRY", "RGBQUAD", "BITMAPINFOHEADER", "sockaddr_in",
    "sockaddr", "in_addr", "fd_set", "timeval", "VARIANT", "DDSURFACEDESC", "DDSURFACEDESC2", "DDPIXELFORMAT", "WAVEFORMATEX", "tm", "FILE"]);

export type TypeIndex = { typedefs: Map<string, string>; enums: Set<string>; records: Set<string> };

function norm(t: string): string {
    return t.replace(/\b(const|volatile|struct|class|union|enum|typename|__restrict|restrict)\b/g, " ")
        .replace(/::/g, "::").replace(/\s+/g, " ").replace(/\s*\*\s*/g, "*").replace(/\s*&\s*/g, "&").trim();
}

function lastName(t: string): string {
    const parts = t.split("::");
    return parts[parts.length - 1];
}

// The kind of a type as the templates see it: v void, b bool, n number, w pointer to a unit,
// p pointer, r reference, x a record by value (or something the generator cannot place).
export function kindOf(type: string, idx: TypeIndex, depth = 0): string {
    const raw = type.trim();
    if (/\(\s*[*&]\s*\)|\(\s*__\w+\s*\*\s*\)/.test(raw)) return /\(\s*&\s*\)/.test(raw) ? "r" : "p";
    const t = norm(raw);
    if (t.endsWith("&")) return "r";
    if (t.endsWith("*")) {
        const base = t.replace(/\*+$/, "");
        return t.endsWith("**") ? "p" : lastName(base) === "D2UnitStrc" ? "w" : "p";
    }
    if (t === "void") return "v";
    if (t === "bool") return "b";
    const words = t.split(" ");
    if (words.every((w) => SYSTEM_NUMBERS.has(w))) return "n";
    const name = lastName(t);
    if (depth < 12 && idx.typedefs.has(name)) {
        const target = idx.typedefs.get(name)!;
        if (target === "(*fn)") return "p";
        return kindOf(target, idx, depth + 1);
    }
    if (idx.enums.has(name)) return "n";
    if (SYSTEM_RECORDS.has(name) || idx.records.has(name)) return "x";
    if (SYSTEM_POINTERS.test(name) && name !== "HRESULT") return "p";
    return "x";
}

// --- the tree ------------------------------------------------------------------------------------------

type MapFunction = { name: string; address: string; namespace?: string; definedIn?: string; paramStorage?: [string, string][] };

export type Entry = {
    id: number;
    path: string;          // engine path, dotted
    unit: string;          // Source/... .cpp
    module: string;
    side: number;
    head: FunctionHead;
    cppNamespace: string[];
    kinds: string;         // result + params
    refused: number;
    paramNames: string[];
    paramTypes: string[];
    address: string;
    inHeader: string | undefined;  // the header defining it (inline functions)
};

function walk(dir: string, root: string, out: string[]): void {
    for (const e of fs.readdirSync(path.join(root, dir), { withFileTypes: true })) {
        const rel = path.posix.join(dir, e.name);
        if (e.isDirectory()) walk(rel, root, out);
        else out.push(rel);
    }
}

function readScan(file: string, cache: Map<string, ScanResult>): ScanResult {
    let s = cache.get(file);
    if (!s) {
        s = scan(fs.readFileSync(file, "utf8"));
        cache.set(file, s);
    }
    return s;
}

// The conditions a function's table entry repeats: everything but the generator's include guards.
function keptConditions(conds: string[]): string[] {
    return conds.filter((c) => !/D2_DEFINED_|_H_?\b|_INCLUDED|#ifndef D2_/.test(c) && c !== "#if 1");
}

function sanitize(name: string): string {
    return name.replace(/<.*$/, (m) => "_" + m.replace(/[^A-Za-z0-9_]+/g, "_").replace(/^_+|_+$/g, "")).replace(/[^A-Za-z0-9_]/g, "_");
}

export type Registry = { entries: Entry[]; units: string[]; index: TypeIndex; unmatched: string[]; declared: Map<string, number> };

/** Read the tree under root (the repository): every function of every Windows-built unit. */
export function readTree(root: string): Registry {
    const files: string[] = [];
    walk("Source", root, files);
    const headers = files.filter((f) => f.endsWith(".h") && !EXCLUDED.test(f)).sort();
    const units = files.filter((f) => f.endsWith(".cpp") && !EXCLUDED.test(f) && files.includes(`${f}.map`)).sort();
    const cache = new Map<string, ScanResult>();
    const index: TypeIndex = { typedefs: new Map(), enums: new Set(), records: new Set() };
    // Inline definitions in headers, by name.
    const headerDefs = new Map<string, { file: string; head: FunctionHead }[]>();
    for (const h of headers) {
        const s = readScan(path.join(root, h), cache);
        for (const [k, v] of s.typedefs) if (!index.typedefs.has(k)) index.typedefs.set(k, v);
        for (const e of s.enums) index.enums.add(e);
        for (const r of s.records) index.records.add(r);
        for (const f of s.functions) {
            if (!f.definition) continue;
            const a = headerDefs.get(f.name) ?? [];
            a.push({ file: h, head: f });
            headerDefs.set(f.name, a);
        }
    }
    // How many functions of each name the tree declares or defines: a name with several needs a cast
    // to the one an entry means.
    const declared = new Map<string, number>();
    const seenHeads = new Set<string>();
    const countHead = (f: FunctionHead) => {
        const sig = `${f.name}(${f.params.map((p) => norm(p.type)).join(",")})`;
        if (seenHeads.has(sig)) return;
        seenHeads.add(sig);
        declared.set(f.name, (declared.get(f.name) ?? 0) + 1);
    };
    for (const h of headers) for (const f of readScan(path.join(root, h), cache).functions) countHead(f);
    for (const u of units) for (const f of readScan(path.join(root, u), cache).functions) countHead(f);
    const entries: Entry[] = [];
    const unmatched: string[] = [];
    const taken = new Set<string>();  // header inline functions already registered (one unit owns each)
    for (const unit of units) {
        const s = readScan(path.join(root, unit), cache);
        for (const [k, v] of s.typedefs) if (!index.typedefs.has(k)) index.typedefs.set(k, v);
        for (const e of s.enums) index.enums.add(e);
        for (const r of s.records) index.records.add(r);
    }
    for (const unit of units) {
        const module = unit.split("/")[1];
        const map = JSON.parse(fs.readFileSync(path.join(root, `${unit}.map`), "utf8")) as { functions: MapFunction[] };
        const s = readScan(path.join(root, unit), cache);
        const defs = new Map<string, FunctionHead[]>();
        for (const f of s.functions) {
            if (!f.definition) continue;
            const key = f.qualified ? f.name.split("::").pop()! : f.name;
            const a = defs.get(key) ?? [];
            a.push(f);
            defs.set(key, a);
        }
        const used = new Set<FunctionHead>();
        for (const mf of map.functions) {
            let candidates = (defs.get(mf.name) ?? []).filter((h) => !used.has(h));
            let inHeader: string | undefined;
            if (!candidates.length) {
                const hs = (headerDefs.get(mf.name) ?? []).filter((x) => !taken.has(`${x.file}:${x.head.line}`));
                if (hs.length) {
                    // the header the map names, else the first definition
                    const want = mf.definedIn ? hs.find((x) => x.file.endsWith(mf.definedIn!.split("/").pop()!)) : undefined;
                    const pick = want ?? hs[0];
                    taken.add(`${pick.file}:${pick.head.line}`);
                    candidates = [pick.head];
                    inHeader = pick.file;
                }
            }
            if (!candidates.length) {
                unmatched.push(`${unit}: ${mf.name}`);
                continue;
            }
            // Several definitions of one name: pick by the parameter names the map recorded.
            let head = candidates[0];
            if (candidates.length > 1 && mf.paramStorage) {
                const names = mf.paramStorage.map((p) => p[0]);
                const exact = candidates.find((c) => c.params.length === names.length && c.params.every((p, i) => p.name === names[i]));
                const sameCount = candidates.find((c) => c.params.length === names.length);
                head = exact ?? sameCount ?? head;
            }
            used.add(head);
            entries.push(makeEntry(unit, module, head, mf, inHeader, index));
        }
    }
    // Engine paths: a name that two functions share in one namespace (static helpers of two units,
    // overloads) gets its address appended.
    const byPath = new Map<string, Entry[]>();
    for (const e of entries) {
        const a = byPath.get(e.path) ?? [];
        a.push(e);
        byPath.set(e.path, a);
    }
    for (const [, list] of byPath) {
        if (list.length > 1) for (const e of list) e.path = `${e.path}_${e.address}`;
    }
    // A path that is also a namespace prefix of another (a function named like a namespace) keeps
    // its name: lookup tries the function first.
    entries.sort((a, b) => (a.unit < b.unit ? -1 : a.unit > b.unit ? 1 : a.head.line - b.head.line));
    entries.forEach((e, i) => (e.id = i));
    return { entries, units, index, unmatched, declared };
}

function makeEntry(unit: string, module: string, head: FunctionHead, mf: MapFunction, inHeader: string | undefined, index: TypeIndex): Entry {
    const ns = head.namespace;
    const qualifiedParts = head.qualified ? head.name.split("::") : [head.name];
    const fname = qualifiedParts.pop()!;
    const nsParts = [...ns, ...qualifiedParts.map(sanitize)];
    const pathParts = nsParts[0] === module ? nsParts.slice(1) : nsParts;
    const kinds = [kindOf(head.ret, index), ...head.params.map((p) => kindOf(p.type, index))].join("");
    let refused: number = REFUSAL.OK;
    if (head.qualified) refused = REFUSAL.MEMBER;
    else if (head.variadic) refused = REFUSAL.VARIADIC;
    else if (head.forceInline) refused = REFUSAL.INLINED;
    else if (inHeader && head.isStatic) refused = REFUSAL.HEADER_STATIC;
    else if (head.params.length > MAX_ARGS) refused = REFUSAL.TOO_MANY;
    else if (kinds.slice(1).includes("v") || kinds.includes("x")) refused = REFUSAL.BY_VALUE;
    return {
        id: -1,
        path: [module, ...pathParts, fname].join("."),
        unit,
        module,
        side: sideOfModule(module),
        head,
        cppNamespace: ns,
        kinds,
        refused,
        paramNames: head.params.map((p, i) => tsName(p.name, i)),
        paramTypes: head.params.map((p) => p.type),
        address: mf.address,
        inHeader,
    };
}

const TS_RESERVED = new Set(("break case catch class const continue debugger default delete do else enum export extends false finally for "
    + "function if import in instanceof new null return super switch this throw true try typeof var void while with yield let static "
    + "implements interface package private protected public await any boolean number string symbol type of next").split(" "));

function tsName(name: string, i: number): string {
    if (!name) return `arg${i + 1}`;
    return TS_RESERVED.has(name) ? `${name}_` : name;
}

// --- C++ -------------------------------------------------------------------------------------------------

const HEADER = "// Generated by Redivivus/Lua/tools/src/overrides.ts from the tree's own definitions. Do not edit;\n// run node Redivivus/Lua/tools/src/gen.ts.";

function cString(s: string): string {
    return JSON.stringify(s);
}

function symbolOf(unit: string): string {
    return unit.replace(/^Source\//, "").replace(/\.cpp$/, "").replace(/[^A-Za-z0-9]/g, "_");
}

// The function as an expression in its own namespace: &Name, or a cast that picks the overload.
function addressExpr(e: Entry, overloaded: boolean): string {
    if (!overloaded) return `&${e.head.name}`;
    const conv = e.head.conv ? `${e.head.conv} ` : "";
    return `static_cast<${e.head.ret} (${conv}*)(${e.paramTypes.join(", ")})>(&${e.head.name})`;
}

function wrapConds(conds: string[], body: string[]): string[] {
    if (!conds.length) return body;
    const open = conds.map((c) => c.startsWith("#if") ? c : `#if ${c}`);
    return [...open, ...body, ...conds.map(() => "#endif")];
}

export function unitCpp(unit: string, entries: Entry[], declared: Map<string, number>): string {
    const sym = symbolOf(unit);
    const out: string[] = [HEADER, `#include ${cString(unit)}`, "#if D2_FEATURE_MODS", '#include "Redivivus/Lua/d2_lua_override_gen.h"', ""];
    // Names that need a cast: the same name defined twice in one namespace under the same conditions.
    const count = new Map<string, number>();
    for (const e of entries) {
        const k = `${e.cppNamespace.join("::")}::${e.head.name}|${keptConditions(e.head.conditions).join("&")}`;
        count.set(k, (count.get(k) ?? 0) + 1);
    }
    // One constant per function, in the function's own namespace, so its name and its parameter types
    // resolve as they do in the unit.
    const ref = (e: Entry) => `${e.cppNamespace.length ? `::${e.cppNamespace.join("::")}::` : "::"}d2lua_ov_${e.id}`;
    for (const e of entries) {
        if (e.refused === REFUSAL.MEMBER || e.refused === REFUSAL.VARIADIC || e.refused === REFUSAL.HEADER_STATIC) continue;
        const conds = keptConditions(e.head.conditions);
        const k = `${e.cppNamespace.join("::")}::${e.head.name}|${conds.join("&")}`;
        const decl = `constexpr auto d2lua_ov_${e.id} = ${addressExpr(e, (count.get(k) ?? 0) > 1 || (declared.get(e.head.name) ?? 0) > 1)};`;
        const body = e.cppNamespace.length ? [`namespace ${e.cppNamespace.join("::")} { ${decl} }`] : [decl];
        out.push(...wrapConds(conds, body));
    }
    out.push("");
    // The kinds, checked.
    for (const e of entries) {
        const conds = keptConditions(e.head.conditions);
        if (e.refused === REFUSAL.OK) {
            out.push(...wrapConds(conds, [`D2LUA_OV_CHECK(${cString(e.path)}, ${ref(e)}, ${cString(e.kinds)});`]));
        } else if (e.refused === REFUSAL.BY_VALUE) {
            out.push(...wrapConds(conds, [`static_assert(!D2LuaOv::AllSupported(D2LUA_OV_SIGOF(${ref(e)})::Kinds()), ${cString(`${e.path} takes or returns a record by value`)});`]));
        }
    }
    out.push("", `static const D2LuaOvEntry s_aEntries[] = {`);
    for (const e of entries) {
        const conds = keptConditions(e.head.conditions);
        const side = e.side;
        let line: string;
        if (e.refused === REFUSAL.OK) {
            line = `    D2LUA_OV(${e.id}, ${cString(e.path)}, ${ref(e)}, ${side}, ${cString(e.kinds)}, ${cString(pointees(e))}),`;
        } else {
            const addr = e.refused === REFUSAL.MEMBER || e.refused === REFUSAL.VARIADIC || e.refused === REFUSAL.HEADER_STATIC
                ? "nullptr" : `D2LuaOv::Address(${ref(e)})`;
            line = `    D2LUA_OV_REFUSED(${e.id}, ${cString(e.path)}, ${addr}, ${side}, ${cString(e.kinds)}, ${e.refused}),`;
        }
        out.push(...wrapConds(conds, [line]));
    }
    out.push("    D2LUA_OV_REFUSED(-1, nullptr, nullptr, 0, nullptr, 0),", "};");
    out.push(`extern const D2LuaOvTable g_tD2LuaOv_${sym} = { s_aEntries, (int)(sizeof(s_aEntries) / sizeof(s_aEntries[0])) - 1 };`, "#endif", "");
    return out.join("\n");
}

// The types the pointers of the result and the parameters point to, in kinds order (the raw tier
// makes its cdata types from them).
function pointees(e: Entry): string {
    const types = [e.head.ret, ...e.paramTypes];
    const out: string[] = [];
    for (let i = 0; i < e.kinds.length; i++) if ("wpr".includes(e.kinds[i])) out.push(pointee(types[i]));
    return out.join("|");
}

export function indexCpp(units: string[]): string {
    const out = [HEADER, "#if D2_FEATURE_MODS", '#include "Redivivus/Lua/d2_lua_override.h"', ""];
    for (const u of units) out.push(`extern const D2LuaOvTable g_tD2LuaOv_${symbolOf(u)};`);
    out.push("", "const D2LuaOvTable* const g_apD2LuaOvTables[] = {");
    for (const u of units) out.push(`    &g_tD2LuaOv_${symbolOf(u)},`);
    out.push("    nullptr,", "};", "#endif", "");
    return out.join("\n");
}

// --- TypeScript -------------------------------------------------------------------------------------------

function pointee(type: string): string {
    if (/\(/.test(type)) return "void";  // a function pointer
    const t = norm(type).replace(/[*&]+$/, "");
    const n = lastName(t).replace(/[^A-Za-z0-9_]/g, "_");
    return n || "void";
}

function tsType(kind: string, type: string, shared: boolean): string {
    switch (kind) {
    case "n":
        return "number";
    case "b":
        return "boolean";
    case "v":
        return "void";
    case "w":
        return "U<T> | undefined";
    case "p":
        return `P<T, ${JSON.stringify(pointee(type))}> | undefined`;
    case "r":
        return `P<T, ${JSON.stringify(pointee(type) + "&")}>`;
    }
    return "never";
}

type NsNode = { children: Map<string, NsNode>; functions: Entry[] };

function tree(entries: Entry[]): Map<string, NsNode> {
    const roots = new Map<string, NsNode>();
    for (const e of entries) {
        const parts = e.path.split(".");
        let map = roots;
        let node: NsNode | undefined;
        for (const p of parts.slice(0, -1)) {
            node = map.get(p);
            if (!node) {
                node = { children: new Map(), functions: [] };
                map.set(p, node);
            }
            map = node.children;
        }
        node!.functions.push(e);
    }
    return roots;
}

function ifaceName(parts: string[]): string {
    return `NS_${parts.join("_")}`;
}

/** core/redivivus/engine-functions.ts: the engine object's types. */
export function engineTs(entries: Entry[]): string {
    const out: string[] = [
        "/**",
        " * The functions of the compiled game, as `patch(engine)` reaches them: `engine.<module>.<namespace>.<function>`,",
        " * named as in the source. Each one's `override` takes a function of `next` and the arguments;",
        " * `next(...)` runs what is below (other mods' overrides, at the bottom the original).",
        " * Generated by Redivivus/Lua/tools/src/overrides.ts from the tree's own definitions. Do not edit;",
        " * run node Redivivus/Lua/tools/src/gen.ts.",
        " * @side data server client",
        " */",
        "",
        "/** Which tier a namespace is seen through: the typed tiers of the two sides, or the raw one. */",
        "export interface TierServer { readonly tier: \"server\" }",
        "export interface TierClient { readonly tier: \"client\" }",
        "export interface TierRaw { readonly tier: \"raw\" }",
        "export type Tier = TierServer | TierClient | TierRaw;",
        "",
        "import type { Unit } from \"./server\";",
        "",
        "declare const opaqueBrand: unique symbol;",
        "/** A pointer of the engine's, passed through untouched: a mod can hand it on to `next` or return it, never make or read one. */",
        "export interface Opaque<N extends string> { readonly [opaqueBrand]: N }",
        "declare const pointerBrand: unique symbol;",
        "/** A pointer in the raw tier: LuaJIT FFI cdata, read and written through ffi. */",
        "export interface Pointer<N extends string> { readonly [pointerBrand]: N; [field: string]: any }",
        "",
        "/** A unit as each tier hands it over: a server state's checked handle (redivivus:server), the client's opaque value, the raw tier's cdata. */",
        "export type U<T extends Tier> = T extends TierServer ? Unit : T extends TierRaw ? Pointer<\"D2UnitStrc\"> : Opaque<\"D2UnitStrc\">;",
        "export type P<T extends Tier, N extends string> = T extends TierRaw ? Pointer<N> : Opaque<N>;",
        "",
        "/** One function of the engine. */",
        "export interface EngineFunction<F extends (this: void, ...args: any[]) => any> {",
        "    /** Its engine path. */",
        "    readonly path: string;",
        "    /**",
        "     * Replace it for this state, from `patch` only. `fn` gets `next` (what is below: other mods'",
        "     * overrides, at the bottom the original) and the arguments, and returns the result.",
        "     */",
        "    override(this: void, fn: (this: void, next: F, ...args: Parameters<F>) => ReturnType<F>): void;",
        "}",
        "/** A function the registry lists but that cannot be overridden; `refused` says why. */",
        "export interface RefusedFunction {",
        "    readonly path: string;",
        "    readonly refused: string;",
        "}",
        "",
    ];
    const roots = tree(entries);
    const emitNode = (parts: string[], node: NsNode) => {
        out.push(`export interface ${ifaceName(parts)}<T extends Tier> {`);
        for (const [name] of [...node.children].sort((a, b) => (a[0] < b[0] ? -1 : 1))) {
            out.push(`    readonly ${JSON.stringify(name)}: ${ifaceName([...parts, name])}<T>;`);
        }
        const fns = [...node.functions].sort((a, b) => (a.path < b.path ? -1 : 1));
        for (const e of fns) {
            const fname = e.path.split(".").pop()!;
            if (node.children.has(fname)) continue;  // a namespace of the same name wins in the types
            if (e.refused !== REFUSAL.OK) {
                out.push(`    /** Cannot be overridden: ${REFUSAL_TEXT[e.refused] ?? "refused"}. */`);
                out.push(`    readonly ${JSON.stringify(fname)}: RefusedFunction;`);
                continue;
            }
            const shared = (e.side & SIDE_SERVER) !== 0 && (e.side & SIDE_CLIENT) !== 0;
            const params = ["this: void", ...e.paramNames.map((n, i) => `${n}: ${tsType(e.kinds[i + 1], e.paramTypes[i], shared)}`)].join(", ");
            const ret = tsType(e.kinds[0], e.head.ret, shared);
            out.push(`    readonly ${JSON.stringify(fname)}: EngineFunction<(${params}) => ${ret}>;`);
        }
        out.push("}");
        for (const [name, child] of [...node.children].sort((a, b) => (a[0] < b[0] ? -1 : 1))) emitNode([...parts, name], child);
    };
    const modules = [...roots.keys()].sort();
    for (const m of modules) emitNode([m], roots.get(m)!);
    const sideOf = (m: string) => sideOfModule(m);
    const fields = (tier: string, side: number) => modules.filter((m) => sideOf(m) & side).map((m) => `    readonly ${m}: ${ifaceName([m])}<${tier}>;`);
    out.push("", "/** The engine of a server state: the server's and the shared modules. */");
    out.push("export interface ServerModules {", ...fields("TierServer", SIDE_SERVER), "}");
    out.push("/** The engine of a client state: the client's and the shared modules. */");
    out.push("export interface ClientModules {", ...fields("TierClient", SIDE_CLIENT), "}");
    out.push("/** The raw tier of a server state: pointers as cdata. */");
    out.push("export interface RawServerModules {", ...fields("TierRaw", SIDE_SERVER), "}");
    out.push("/** The raw tier of a client state. */");
    out.push("export interface RawClientModules {", ...fields("TierRaw", SIDE_CLIENT), "}");
    out.push("");
    return out.join("\n");
}

// --- all outputs --------------------------------------------------------------------------------------------

export function overrideOutputs(root: string, lua: string): { outputs: Map<string, string>; registry: Registry } {
    const registry = readTree(root);
    const outputs = new Map<string, string>();
    const byUnit = new Map<string, Entry[]>();
    for (const e of registry.entries) {
        const a = byUnit.get(e.unit) ?? [];
        a.push(e);
        byUnit.set(e.unit, a);
    }
    const units = [...byUnit.keys()].sort();
    for (const u of units) {
        outputs.set(path.join(lua, "generated/override", u.replace(/\.cpp$/, ".ov.cpp")), unitCpp(u, byUnit.get(u)!, registry.declared));
    }
    outputs.set(path.join(lua, "generated/override/index.gen.cpp"), indexCpp(units));
    outputs.set(path.join(lua, "core/redivivus/engine-functions.ts"), engineTs(registry.entries));
    return { outputs, registry };
}

// The host test's fixture (test/override/): the same generator over a unit written like the tree's,
// every function it and its header define, sides by namespace.
const FIXTURE_SIDES: Record<string, number> = { TestShared: SIDE_SERVER | SIDE_CLIENT, TestServer: SIDE_SERVER, TestClient: SIDE_CLIENT };

export function fixtureOutputs(root: string, lua: string): Map<string, string> {
    const unit = "Redivivus/Lua/test/override/d2_lua_override_fixture.cpp";
    const header = "Redivivus/Lua/test/override/d2_lua_override_fixture.h";
    const sc = scan(fs.readFileSync(path.join(root, unit), "utf8"));
    const sh = scan(fs.readFileSync(path.join(root, header), "utf8"));
    const index: TypeIndex = { typedefs: new Map([...sh.typedefs, ...sc.typedefs]), enums: new Set([...sh.enums, ...sc.enums]),
        records: new Set([...sh.records, ...sc.records]) };
    const heads = [...sc.functions.filter((f) => f.definition), ...sh.functions.filter((f) => f.definition)];
    const entries: Entry[] = heads.map((h) => {
        const module = h.namespace[0];
        const e = makeEntry(unit, module, h, { name: h.name, address: "0" }, undefined, index);
        e.side = FIXTURE_SIDES[module] ?? (SIDE_SERVER | SIDE_CLIENT);
        return e;
    });
    entries.forEach((e, i) => (e.id = i));
    const out = new Map<string, string>();
    out.set(path.join(lua, "test/generated/d2_lua_override_fixture.ov.cpp"), unitCpp(unit, entries, new Map()));
    out.set(path.join(lua, "test/generated/d2_lua_override_index.gen.cpp"), indexCpp([unit]));
    return out;
}

export function summary(r: Registry): string {
    const by = new Map<number, number>();
    for (const e of r.entries) by.set(e.refused, (by.get(e.refused) ?? 0) + 1);
    const parts = [...by].sort((a, b) => a[0] - b[0]).map(([k, v]) => `${k === 0 ? "overridable" : REFUSAL_TEXT[k] ? Object.keys(REFUSAL).find((n) => (REFUSAL as any)[n] === k)!.toLowerCase() : k} ${v}`);
    return `overrides: ${r.entries.length} functions in ${r.units.length} units (${parts.join(", ")}); ${r.unmatched.length} listed functions not found`;
}

// node src/overrides.ts [--list-unmatched]: the registry's numbers, nothing written.
if (process.argv[1] && path.resolve(process.argv[1]) === path.resolve(import.meta.filename)) {
    const ROOT = path.resolve(import.meta.dirname, "../../../..");
    const r = readTree(ROOT);
    console.log(summary(r));
    if (process.argv.includes("--list-unmatched")) for (const u of r.unmatched) console.log(`  ${u}`);
    if (process.argv.includes("--list-refused")) for (const e of r.entries) if (e.refused) console.log(`  ${e.refused} ${e.path} ${e.kinds} (${e.paramTypes.join(", ")}) -> ${e.head.ret}`);
}
