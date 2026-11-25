// The Lua API generator: the TypeScript declarations in Redivivus/Lua/api are the spec, and this
// turns them into the C++ binding glue compiled into the host, a reference in markdown, and LuaLS
// ---@meta files for authors who write plain Lua.
//
//   node src/gen.ts            write every output
//   node src/gen.ts --check    write nothing; fail when an output on disk differs from what the
//                              declarations give (a stale commit)
//
// The binding tags (JSDoc), the whole set:
//   @side <data|server|client ...>   file level (first comment): the file is bound for these sides.
//                                    A file without it holds types only.
//   @include "<header>"              file level: a header the glue includes (handle traits, enums)
//   @assertInclude "<header>"        file level: the header the static_asserts of its @native enums need,
//                                    which then go to a unit of their own (*_asserts.gen.cpp)
//   @native <C++ function>           a function or method; the native the glue calls
//   @get <C++ function>              a property's getter
//   @set <C++ function>              a mutable property's setter
//   @raw <C++ function>              the native takes the Lua stack itself (int f(D2LuaCtx&)); on a
//                                    variable it pushes the value installed into each environment
//   @handle <C++ traits type>        a class whose instances are handles (d2_lua_api.h)
//   @native <C++ enum type>          on a numeric `declare const enum`: the C++ type natives take
//   @scope <C++ qualifier>           with it: where the C++ members live (default: the type), for
//                                    the static_asserts that tie each value to the C++ one
//   @events                          an interface mapping event ids (members of a string enum, as
//                                    computed keys: [Event.Tick]) to payload types
//   @registry                        an interface mapping registry ids to entry signatures
// (TypeScriptToLua's own @noSelf is required on interfaces bound as singletons, so calls compile
// to `script.on_event(...)` rather than `script:on_event(...)`.)
//
// The supported subset, rejected with a message naming the declaration otherwise: string,
// number, integer, boolean, enums (top-level `declare const enum`), handle classes, function types (with
// `this: void` first), tables (interfaces of data, type literals, arrays, LuaTable), unknown/any,
// unions with undefined or null (optional) or of string literals, type parameters constrained by
// `keyof <@events map>`. Anything else needs @raw.
//
// Enums. Every top-level `declare const enum` of a bound file is an API enum: TypeScript inlines its
// values, and each mod environment of the file's sides gets a read-only table of the same name for
// plain Lua (UnitType.Player). Members have explicit values, numbers or strings, never mixed.
// Declarations of one enum in several files merge (the test set adds events to Event).

import * as fs from "node:fs";
import * as path from "node:path";
import ts from "typescript";
import { EnumError, engineEnums } from "./enums.ts";
import { KitError, kitOutputs, staleKitFiles } from "./kit.ts";
import { luaEnvTypings } from "./luaenv.ts";

const TOOLS = path.resolve(import.meta.dirname, "..");
const LUA = path.resolve(TOOLS, "..");
const ROOT = path.resolve(LUA, "../..");

type SetConfig = {
    name: string;
    bindDirs: string[];
    contextDirs: string[];
    cpp: string;
    header: string;
    docs?: string;
    luals?: string;
};

const SETS: SetConfig[] = [
    {
        name: "main",
        bindDirs: [path.join(LUA, "api")],
        contextDirs: [path.join(LUA, "api")],
        cpp: path.join(LUA, "generated/d2_lua_api_main.gen.cpp"),
        header: path.join(LUA, "generated/d2_lua_api_main.gen.h"),
        docs: path.join(LUA, "api/docs/reference.md"),
        luals: path.join(LUA, "api/luals"),
    },
    {
        name: "test",
        bindDirs: [path.join(LUA, "test/api")],
        contextDirs: [path.join(LUA, "api"), path.join(LUA, "test/api")],
        cpp: path.join(LUA, "test/generated/d2_lua_api_test.gen.cpp"),
        header: path.join(LUA, "test/generated/d2_lua_api_test.gen.h"),
    },
];

const SIDES = ["data", "server", "client"] as const;
type Side = (typeof SIDES)[number];
const SIDE_BIT: Record<Side, string> = { data: "D2LUA_SIDE_DATA", server: "D2LUA_SIDE_SERVER", client: "D2LUA_SIDE_CLIENT" };

class GenError extends Error {}

function fail(node: ts.Node, what: string): never {
    const sf = node.getSourceFile();
    const { line } = sf.getLineAndCharacterOfPosition(node.getStart());
    throw new GenError(`${path.relative(ROOT, sf.fileName)}:${line + 1}: ${what}`);
}

// --- tags ------------------------------------------------------------------------------------------

type Tags = Map<string, string>;

function tagsOf(node: ts.Node): Tags {
    const out: Tags = new Map();
    for (const tag of ts.getJSDocTags(node)) {
        const text = (ts.getTextOfJSDocComment(tag.comment) ?? "").trim();
        out.set(tag.tagName.text, text);
    }
    return out;
}

function docOf(node: ts.Node): string {
    const parts: string[] = [];
    for (const doc of ts.getJSDocCommentsAndTags(node)) {
        if (ts.isJSDoc(doc)) {
            const text = ts.getTextOfJSDocComment(doc.comment);
            if (text) parts.push(text.trim());
        }
    }
    return parts.join("\n");
}

const KNOWN_TAGS = new Set(["side", "include", "native", "get", "set", "raw", "handle", "events", "registry", "noSelf"]);

type FileInfo = { sides: Side[]; includes: string[]; assertInclude?: string };

function fileInfo(sf: ts.SourceFile): FileInfo | undefined {
    const m = /^\s*\/\*\*([\s\S]*?)\*\//.exec(sf.text);
    if (!m) return undefined;
    const side = /@side\s+([^\n*]+)/.exec(m[1]);
    if (!side) return undefined;
    const sides = side[1].trim().split(/\s+/);
    for (const s of sides) {
        if (!(SIDES as readonly string[]).includes(s)) {
            throw new GenError(`${path.relative(ROOT, sf.fileName)}: @side names an unknown side '${s}'`);
        }
    }
    const includes = [...m[1].matchAll(/@include\s+"([^"]+)"/g)].map((x) => x[1]);
    const assertInclude = /@assertInclude\s+"([^"]+)"/.exec(m[1])?.[1];
    return { sides: sides as Side[], includes, assertInclude };
}

// --- types ------------------------------------------------------------------------------------------

type Kind =
    | { k: "string" }
    | { k: "number" }
    | { k: "integer" }
    | { k: "boolean" }
    | { k: "void" }
    | { k: "any" }
    | { k: "function" }
    | { k: "table" }
    | { k: "enum"; e: EnumInfo }
    | { k: "handle"; c: ClassInfo }
    | { k: "oneof"; values: string[]; enumName?: string }
    | { k: "event"; map: string };
type Ty = Kind & { optional: boolean; text: string };

type EnumMember = { name: string; value: number | string; cpp: string; doc: string };
// A numeric enum without @native is passed to natives as lua_Integer; a string enum as const char*.
// assertInclude: the header the value checks need, compiled in their own unit (the engine's headers do not
// build beside the host test's system headers).
type EnumInfo = { name: string; kind: "number" | "string"; cpp: string; scope?: string; assertInclude?: string; members: EnumMember[]; sides: Side[]; doc: string };
type ClassInfo = { name: string; traits: string; members: Member[]; sides: Side[]; doc: string; decl: ts.ClassDeclaration };
type Param = { name: string; ty: Ty };
type Member = {
    name: string;
    kind: "method" | "property";
    doc: string;
    params: Param[];
    ret: Ty;
    native?: string;
    getter?: string;
    setter?: string;
    raw?: string;
    readonly: boolean;
    sig: string;
};

class Model {
    checker: ts.TypeChecker;
    program: ts.Program;
    enums = new Map<string, EnumInfo>();     // "UnitType" -> the enum, every declaration merged
    classes = new Map<string, ClassInfo>();  // handle classes
    eventMaps = new Set<string>();           // names of @events interfaces (all files)
    registryOf = new Map<string, { name: string; kind: "keyed" | "slot" }>(); // map interface / slot alias -> registry

    constructor(program: ts.Program) {
        this.program = program;
        this.checker = program.getTypeChecker();
    }
}

function typeText(node: ts.Node | undefined): string {
    return node ? node.getText().replace(/\s+/g, " ") : "void";
}

function isThisVoid(p: ts.ParameterDeclaration): boolean {
    return ts.isIdentifier(p.name) && p.name.text === "this" && !!p.type && p.type.kind === ts.SyntaxKind.VoidKeyword;
}

function resolveType(model: Model, node: ts.TypeNode | undefined, where: ts.Node, what: string): Ty {
    const text = typeText(node);
    const mk = (k: Kind, optional = false): Ty => ({ ...k, optional, text });
    if (!node) return mk({ k: "void" });
    switch (node.kind) {
        case ts.SyntaxKind.StringKeyword: return mk({ k: "string" });
        case ts.SyntaxKind.NumberKeyword: return mk({ k: "number" });
        case ts.SyntaxKind.BooleanKeyword: return mk({ k: "boolean" });
        case ts.SyntaxKind.VoidKeyword: return mk({ k: "void" });
        case ts.SyntaxKind.UnknownKeyword:
        case ts.SyntaxKind.AnyKeyword: return mk({ k: "any" });
        case ts.SyntaxKind.TypeLiteral:
        case ts.SyntaxKind.ArrayType:
        case ts.SyntaxKind.TupleType: return mk({ k: "table" });
    }
    if (ts.isParenthesizedTypeNode(node)) return { ...resolveType(model, node.type, where, what), text };
    if (ts.isFunctionTypeNode(node)) {
        const first = node.parameters[0];
        if (!first || !isThisVoid(first)) {
            fail(where, `${what}: function types must declare 'this: void' first, so TypeScriptToLua calls them without a self argument`);
        }
        return mk({ k: "function" });
    }
    if (ts.isUnionTypeNode(node)) {
        let optional = false;
        const rest: ts.TypeNode[] = [];
        for (const t of node.types) {
            if (t.kind === ts.SyntaxKind.UndefinedKeyword || (ts.isLiteralTypeNode(t) && t.literal.kind === ts.SyntaxKind.NullKeyword)) {
                optional = true;
            } else {
                rest.push(t);
            }
        }
        if (rest.every((t) => ts.isLiteralTypeNode(t) && ts.isStringLiteral(t.literal))) {
            const values = rest.map((t) => ((t as ts.LiteralTypeNode).literal as ts.StringLiteral).text);
            return mk({ k: "oneof", values }, optional);
        }
        if (rest.length === 1) {
            const inner = resolveType(model, rest[0], where, what);
            return { ...inner, optional: true, text };
        }
        fail(where, `${what}: the union '${text}' is not bindable (only T | undefined, T | null, or string literals)`);
    }
    if (ts.isTypeReferenceNode(node)) {
        const name = node.typeName.getText();
        if (name === "integer") return mk({ k: "integer" });
        if (name === "LuaTable") return mk({ k: "table" });
        const sym = model.checker.getSymbolAtLocation(node.typeName);
        const target = sym && sym.flags & ts.SymbolFlags.Alias ? model.checker.getAliasedSymbol(sym) : sym;
        if (target) {
            if (target.flags & ts.SymbolFlags.TypeParameter) {
                const decl = target.declarations?.[0];
                if (decl && ts.isTypeParameterDeclaration(decl) && decl.constraint) {
                    const map = keyofTarget(decl.constraint);
                    if (map && model.eventMaps.has(map)) return mk({ k: "event", map });
                    if (map) return mk({ k: "string" });
                }
                fail(where, `${what}: type parameter '${name}' is bindable only when constrained by keyof <interface>`);
            }
            if (model.classes.has(name)) return mk({ k: "handle", c: model.classes.get(name)! });
            const e = model.enums.get(name);
            if (e && e.kind === "string") return mk({ k: "oneof", values: e.members.map((m) => String(m.value)), enumName: e.name });
            if (e) return mk({ k: "enum", e });
            if (target.flags & ts.SymbolFlags.EnumMember || target.flags & ts.SymbolFlags.Enum) {
                fail(where, `${what}: '${name}' is not a bindable enum (a top-level 'declare const enum'; a single member is not a type here)`);
            }
            if (target.flags & ts.SymbolFlags.Class) {
                fail(where, `${what}: class '${name}' is not a handle class (@handle)`);
            }
            if (name === "Script") fail(where, `${what}: Script<T> results need @raw (the native yields)`);
            if (target.flags & ts.SymbolFlags.TypeAlias) {
                const decl = target.declarations?.find(ts.isTypeAliasDeclaration);
                if (decl) {
                    if (decl.typeParameters?.length) fail(where, `${what}: generic alias '${name}' is not bindable; use @raw`);
                    return { ...resolveType(model, decl.type, where, what), text };
                }
            }
            if (target.flags & ts.SymbolFlags.Interface) {
                if (node.typeArguments?.length) fail(where, `${what}: generic interface '${text}' is not bindable; use @raw`);
                return mk({ k: "table" });
            }
        }
        fail(where, `${what}: cannot resolve type '${text}'`);
    }
    fail(where, `${what}: type '${text}' is outside the bindable subset (string, number, integer, boolean, enums, handle classes, functions, tables, optional, string-literal unions); use @raw`);
}

function keyofTarget(constraint: ts.TypeNode): string | undefined {
    const parts = ts.isIntersectionTypeNode(constraint) ? constraint.types : [constraint];
    for (const p of parts) {
        if (ts.isTypeOperatorNode(p) && p.operator === ts.SyntaxKind.KeyOfKeyword && ts.isTypeReferenceNode(p.type)) {
            return p.type.typeName.getText();
        }
    }
    return undefined;
}

// --- enums ------------------------------------------------------------------------------------------

function memberValue(init: ts.Expression | undefined): number | string | undefined {
    if (!init) return undefined;
    if (ts.isNumericLiteral(init)) return Number(init.text);
    if (ts.isPrefixUnaryExpression(init) && init.operator === ts.SyntaxKind.MinusToken && ts.isNumericLiteral(init.operand)) {
        return -Number(init.operand.text);
    }
    if (ts.isStringLiteral(init)) return init.text;
    return undefined;
}

function readEnum(e: ts.EnumDeclaration, sides: Side[]): EnumInfo {
    const name = e.name.text;
    const mods = e.modifiers ?? [];
    if (!mods.some((m) => m.kind === ts.SyntaxKind.DeclareKeyword) || !mods.some((m) => m.kind === ts.SyntaxKind.ConstKeyword)) {
        fail(e, `enum ${name}: write 'declare const enum' (TypeScript inlines the values; plain Lua reads the table)`);
    }
    const tags = tagsOf(e);
    const members = e.members.map((mm) => {
        const value = memberValue(mm.initializer);
        if (value === undefined) fail(mm, `${name}.${mm.name.getText()}: give every member an explicit number or string`);
        return { name: mm.name.getText(), value, cpp: tagsOf(mm).get("native") || mm.name.getText(), doc: docOf(mm) };
    });
    const kinds = new Set(members.map((m) => typeof m.value));
    if (kinds.size > 1) fail(e, `enum ${name}: members are all numbers or all strings`);
    const kind = kinds.has("string") ? "string" : "number";
    const cpp = tags.get("native");
    if (cpp && kind === "string") fail(e, `enum ${name}: a string enum has no C++ type (@native is for numeric enums)`);
    if (tags.has("scope") && !cpp) fail(e, `enum ${name}: @scope needs @native`);
    return { name, kind, cpp: cpp || "lua_Integer", scope: cpp ? tags.get("scope") || cpp : undefined, members, sides, doc: docOf(e) };
}

// Merge a declaration into the program-wide enum of that name.
function mergeEnum(model: Model, e: EnumInfo): void {
    const prev = model.enums.get(e.name);
    if (!prev) {
        model.enums.set(e.name, { ...e, members: [...e.members] });
        return;
    }
    if (prev.kind !== e.kind) throw new GenError(`enum ${e.name}: declared once with numbers and once with strings`);
    for (const m of e.members) {
        if (prev.members.some((p) => p.name === m.name)) throw new GenError(`enum ${e.name}: member ${m.name} declared twice`);
        prev.members.push(m);
    }
}

// The name an @events or @registry key gives: [Event.Tick] -> "on_tick". Keys are enum members, so
// TypeScript code can only name them through the enum.
function enumKey(model: Model, m: ts.PropertySignature, map: string): { name: string; member: string } {
    if (!ts.isComputedPropertyName(m.name) || !ts.isPropertyAccessExpression(m.name.expression)) {
        fail(m, `${map}: a key is a member of a string enum, [Enum.Member], not a string`);
    }
    const value = model.checker.getConstantValue(m.name.expression);
    if (typeof value !== "string") fail(m, `${map}: ${m.name.getText()} is not a member of a string enum`);
    return { name: value, member: m.name.expression.getText() };
}

// --- reading the declarations -------------------------------------------------------------------

type Global =
    | { kind: "singleton"; name: string; iface: string; members: Member[]; sides: Side[]; doc: string; noSelf: boolean; node: ts.Node }
    | { kind: "raw"; name: string; native: string; sides: Side[]; doc: string; type: string; node: ts.Node }
    | { kind: "function"; name: string; member: Member; sides: Side[]; node: ts.Node };

type EventMap = { name: string; sides: Side[]; model: Model; events: { name: string; member: string; payload: string; doc: string; node: ts.PropertySignature }[] };
type RegistryNative = { registry: string; id: string | null; native: string; params: Param[]; ret: Ty; sides: Side[]; doc: string; sig: string };
type RegistryDef = { name: string; kind: "keyed" | "slot"; map: string; sides: Side[] };

type ApiSet = {
    config: SetConfig;
    files: { sf: ts.SourceFile; info: FileInfo }[];
    globals: Global[];
    classes: ClassInfo[];
    enums: EnumInfo[];
    eventMaps: EventMap[];
    registryDefs: RegistryDef[];
    registryNatives: RegistryNative[];
    includes: string[];
};

function memberFromSignature(model: Model, node: ts.MethodSignature | ts.MethodDeclaration | ts.FunctionDeclaration | ts.PropertySignature | ts.PropertyDeclaration, owner: string): Member {
    const tags = tagsOf(node);
    for (const t of tags.keys()) {
        if (!KNOWN_TAGS.has(t) && !["param", "returns", "deprecated", "see", "example"].includes(t)) {
            fail(node, `${owner}: unknown tag @${t}`);
        }
    }
    const name = node.name ? node.name.getText().replace(/^["']|["']$/g, "") : "?";
    const what = `${owner}.${name}`;
    const doc = docOf(node);
    const sig = node.getText().replace(/\/\*\*[\s\S]*?\*\/\s*/g, "").replace(/\s+/g, " ").trim();
    if (ts.isPropertySignature(node) || ts.isPropertyDeclaration(node)) {
        const readonly = !!node.modifiers?.some((m) => m.kind === ts.SyntaxKind.ReadonlyKeyword);
        const m: Member = { name, kind: "property", doc, params: [], ret: { k: "void", optional: false, text: "" }, readonly, sig };
        if (tags.has("raw")) {
            m.raw = tags.get("raw");
            if (!m.raw) fail(node, `${what}: @raw needs the native's name`);
            if (!readonly) fail(node, `${what}: a @raw property must be readonly`);
            return m;
        }
        m.ret = resolveType(model, node.type, node, what);
        if (node.questionToken) m.ret = { ...m.ret, optional: true };
        m.getter = tags.get("get");
        m.setter = tags.get("set");
        if (!m.getter) fail(node, `${what}: no binding (a property needs @get, or @raw)`);
        if (!readonly && !m.setter) fail(node, `${what}: a mutable property needs @set (or mark it readonly)`);
        if (readonly && m.setter) fail(node, `${what}: a readonly property cannot have @set`);
        if (m.ret.k === "table" || m.ret.k === "function" || m.ret.k === "any") {
            fail(node, `${what}: a ${m.ret.k} property needs @raw (the native pushes it)`);
        }
        return m;
    }
    const m: Member = { name, kind: "method", doc, params: [], ret: { k: "void", optional: false, text: "" }, readonly: true, sig };
    if (tags.has("raw")) {
        m.raw = tags.get("raw");
        if (!m.raw) fail(node, `${what}: @raw needs the native's name`);
        return m;
    }
    m.native = tags.get("native");
    if (!m.native) fail(node, `${what}: no binding (a function needs @native or @raw)`);
    if (node.typeParameters) {
        for (const tp of node.typeParameters) {
            if (!tp.constraint || !keyofTarget(tp.constraint)) fail(tp, `${what}: type parameter '${tp.name.text}' must be constrained by keyof <interface>`);
        }
    }
    for (const p of node.parameters) {
        if (isThisVoid(p)) continue;
        if (p.dotDotDotToken) fail(p, `${what}: rest parameters need @raw`);
        let ty = resolveType(model, p.type, p, `${what}(${p.name.getText()})`);
        if (p.questionToken) ty = { ...ty, optional: true };
        m.params.push({ name: p.name.getText(), ty });
    }
    m.ret = resolveType(model, node.type, node, `${what} (result)`);
    if (m.ret.k === "table" || m.ret.k === "function" || m.ret.k === "any" || m.ret.k === "event") {
        fail(node, `${what}: returning a ${m.ret.k} needs @raw`);
    }
    return m;
}

// Each side is its own program (server and client both declare `script`), so a file bound for
// several sides is read in the program of the first side it appears in.
function loadSet(config: SetConfig): ApiSet {
    const set: ApiSet = { config, files: [], globals: [], classes: [], enums: [], eventMaps: [], registryDefs: [], registryNatives: [], includes: [] };
    const contextFiles = config.contextDirs.flatMap((d) =>
        fs.readdirSync(d).filter((f) => f.endsWith(".d.ts")).map((f) => path.join(d, f)));
    const sidesOf = new Map<string, Side[] | undefined>();
    for (const f of contextFiles) {
        sidesOf.set(path.resolve(f), fileInfo(ts.createSourceFile(f, fs.readFileSync(f, "utf8"), ts.ScriptTarget.ESNext, true))?.sides);
    }
    const done = new Set<string>();
    for (const side of SIDES) {
        const roots = contextFiles.filter((f) => { const s = sidesOf.get(path.resolve(f)); return !s || s.includes(side); });
        if (!roots.some((f) => sidesOf.get(path.resolve(f)))) continue;
        loadSide(config, roots, set, done);
    }
    return set;
}

function loadSide(config: SetConfig, contextFiles: string[], set: ApiSet, done: Set<string>): void {
    const program = ts.createProgram(contextFiles, {
        noEmit: true,
        strict: true,
        target: ts.ScriptTarget.ESNext,
        lib: ["lib.esnext.d.ts"],
        types: [],
        typeRoots: [path.join(TOOLS, "node_modules")],
        skipLibCheck: false,
    });
    const diags = ts.getPreEmitDiagnostics(program).filter((d) => !d.file || !d.file.fileName.includes("node_modules"));
    if (diags.length) {
        const host: ts.FormatDiagnosticsHost = { getCanonicalFileName: (f) => f, getCurrentDirectory: () => ROOT, getNewLine: () => "\n" };
        throw new GenError(`the declarations do not type-check:\n${ts.formatDiagnostics(diags, host)}`);
    }
    const model = new Model(program);
    const bindSet = new Set(config.bindDirs.map((d) => path.resolve(d)));
    const all = program.getSourceFiles().filter((sf) => config.contextDirs.some((d) => path.resolve(path.dirname(sf.fileName)) === path.resolve(d)));

    // Pass 1, every context file: enums, handle classes, event maps, registry names.
    for (const sf of all) {
        const info = fileInfo(sf);
        const sides = info?.sides ?? [];
        for (const st of sf.statements) {
            if (ts.isModuleDeclaration(st) && st.name.getText() === "defines") {
                fail(st, "enums are top-level 'declare const enum' declarations, not members of a 'defines' namespace");
            }
            if (ts.isEnumDeclaration(st)) mergeEnum(model, readEnum(st, sides));
            if (ts.isClassDeclaration(st) && st.name) {
                const traits = tagsOf(st).get("handle");
                if (traits) model.classes.set(st.name.text, { name: st.name.text, traits, members: [], sides, doc: docOf(st), decl: st });
            }
            if (ts.isInterfaceDeclaration(st) && tagsOf(st).has("events")) model.eventMaps.add(st.name.text);
        }
    }
    // Registries: the members of the type of `declare const registries`.
    for (const sf of all) {
        for (const st of sf.statements) {
            if (!ts.isVariableStatement(st)) continue;
            for (const d of st.declarationList.declarations) {
                if (d.name.getText() !== "registries" || !d.type || !ts.isTypeReferenceNode(d.type)) continue;
                const sym = model.checker.getSymbolAtLocation(d.type.typeName);
                const iface = sym?.declarations?.find(ts.isInterfaceDeclaration);
                if (!iface) fail(d, "registries must be typed by an interface");
                for (const m of iface.members) {
                    if (!ts.isPropertySignature(m) || !m.type || !ts.isTypeReferenceNode(m.type)) fail(m, "registries members are KeyedRegistry<Map> or SlotRegistry<Signature>");
                    const kindName = m.type.typeName.getText();
                    const arg = m.type.typeArguments?.[0];
                    if (!arg || !ts.isTypeReferenceNode(arg) || (kindName !== "KeyedRegistry" && kindName !== "SlotRegistry")) {
                        fail(m, "registries members are KeyedRegistry<Map> or SlotRegistry<Signature>");
                    }
                    const kind = kindName === "KeyedRegistry" ? "keyed" : "slot";
                    const regName = m.name.getText();
                    model.registryOf.set(arg.typeName.getText(), { name: regName, kind });
                    const info = fileInfo(sf);
                    if (info && config.bindDirs.some((b) => path.resolve(path.dirname(sf.fileName)) === path.resolve(b)) && !done.has(path.resolve(sf.fileName))) {
                        set.registryDefs.push({ name: regName, kind, map: arg.typeName.getText(), sides: info.sides });
                    }
                }
            }
        }
    }

    // Pass 2, bound files: everything that becomes a value.
    for (const sf of all) {
        if (!bindSet.has(path.resolve(path.dirname(sf.fileName)))) continue;
        if (done.has(path.resolve(sf.fileName))) continue;
        done.add(path.resolve(sf.fileName));
        const info = fileInfo(sf);
        if (!info) {
            for (const st of sf.statements) {
                if (ts.isVariableStatement(st) || ts.isFunctionDeclaration(st) || (ts.isClassDeclaration(st))) {
                    fail(st, "a file without @side holds types only");
                }
            }
            continue;
        }
        set.files.push({ sf, info });
        for (const inc of info.includes) if (!set.includes.includes(inc)) set.includes.push(inc);
        const rawTypes = new Set<string>();
        for (const st of sf.statements) {
            if (ts.isVariableStatement(st)) {
                for (const d of st.declarationList.declarations) {
                    const tags = tagsOf(st);
                    if (tags.has("raw") && d.type && ts.isTypeReferenceNode(d.type)) rawTypes.add(d.type.typeName.getText());
                }
            }
        }
        for (const st of sf.statements) {
            if (ts.isVariableStatement(st)) {
                const tags = tagsOf(st);
                for (const d of st.declarationList.declarations) {
                    const name = d.name.getText();
                    if (tags.has("raw")) {
                        const native = tags.get("raw");
                        if (!native) fail(d, `${name}: @raw needs the native's name`);
                        set.globals.push({ kind: "raw", name, native, sides: info.sides, doc: docOf(st), type: typeText(d.type), node: d });
                        continue;
                    }
                    if (!d.type || !ts.isTypeReferenceNode(d.type)) fail(d, `${name}: a global is a @raw value or typed by a @noSelf interface`);
                    const sym = model.checker.getSymbolAtLocation(d.type.typeName);
                    const ifaces = sym?.declarations?.filter(ts.isInterfaceDeclaration) ?? [];
                    if (!ifaces.length) fail(d, `${name}: a global is a @raw value or typed by a @noSelf interface`);
                    const noSelf = ifaces.some((i) => tagsOf(i).has("noSelf"));
                    if (!noSelf) fail(d, `${name}: its interface ${d.type.getText()} needs @noSelf (calls are dot calls)`);
                    const members: Member[] = [];
                    for (const i of ifaces) {
                        for (const m of i.members) {
                            if (ts.isMethodSignature(m) || ts.isPropertySignature(m)) members.push(memberFromSignature(model, m, name));
                            else fail(m, `${name}: only methods and properties are bindable`);
                        }
                    }
                    set.globals.push({ kind: "singleton", name, iface: d.type.getText(), members, sides: info.sides, doc: docOf(st), noSelf, node: d });
                }
            } else if (ts.isFunctionDeclaration(st)) {
                const member = memberFromSignature(model, st, "(global)");
                const first = st.parameters[0];
                if (!first || !isThisVoid(first)) fail(st, `${member.name}: global functions declare 'this: void' first`);
                set.globals.push({ kind: "function", name: member.name, member, sides: info.sides, node: st });
            } else if (ts.isClassDeclaration(st)) {
                const cls = st.name && model.classes.get(st.name.text);
                if (!cls) fail(st, "a class in a bound file must be a handle class (@handle <traits>)");
                for (const m of st.members) {
                    if (ts.isConstructorDeclaration(m)) {
                        if (!m.modifiers?.some((x) => x.kind === ts.SyntaxKind.PrivateKeyword)) fail(m, `${cls.name}: handles are made by the host; declare 'private constructor()'`);
                        continue;
                    }
                    if (m.name?.getText() === "valid") continue;
                    if (ts.isMethodDeclaration(m) || ts.isPropertyDeclaration(m)) cls.members.push(memberFromSignature(model, m, cls.name));
                    else fail(m, `${cls.name}: only methods and properties are bindable`);
                }
                set.classes.push(cls);
            } else if (ts.isInterfaceDeclaration(st)) {
                const tags = tagsOf(st);
                if (tags.has("events")) {
                    const map: EventMap = { name: st.name.text, sides: info.sides, events: [], model };
                    for (const m of st.members) {
                        if (!ts.isPropertySignature(m) || !m.type) fail(m, `${st.name.text}: events are '[Event.Name]: PayloadType'`);
                        map.events.push({ ...enumKey(model, m, st.name.text), payload: m.type.getText(), doc: docOf(m), node: m });
                    }
                    set.eventMaps.push(map);
                } else if (tags.has("registry")) {
                    const reg = model.registryOf.get(st.name.text);
                    if (!reg) fail(st, `${st.name.text}: @registry interface is not the map of any member of 'registries'`);
                    for (const m of st.members) {
                        if (!ts.isPropertySignature(m) || !m.type || !ts.isFunctionTypeNode(m.type)) fail(m, `${st.name.text}: entries are '[Id.Member]: (this: void, ...) => R'`);
                        const native = tagsOf(m).get("native");
                        const id = enumKey(model, m, st.name.text).name;
                        if (!native) fail(m, `${st.name.text}["${id}"]: no binding (@native for the entry's base implementation)`);
                        set.registryNatives.push({ ...signatureOf(model, m.type, m, `${reg.name}["${id}"]`), registry: reg.name, id, native, sides: info.sides, doc: docOf(m) });
                    }
                } else if (!st.name.text.startsWith("Lua") && !rawTypes.has(st.name.text)) {
                    for (const m of st.members) {
                        if (ts.isMethodSignature(m) && !set.globals.some((g) => g.kind === "singleton" && g.iface === st.name.text)) {
                            // Interfaces of plain data hold properties; methods belong to a bound singleton.
                            if (!tagsOf(st).has("noSelf")) fail(m, `${st.name.text}.${m.name.getText()}: a method outside a bound singleton or handle class`);
                        }
                    }
                }
            } else if (ts.isTypeAliasDeclaration(st)) {
                const reg = model.registryOf.get(st.name.text);
                if (reg && reg.kind === "slot") {
                    const native = tagsOf(st).get("native");
                    if (!native) fail(st, `${st.name.text}: slot registry signature without @native base implementation`);
                    if (!ts.isFunctionTypeNode(st.type)) fail(st, `${st.name.text}: a slot signature is a function type`);
                    set.registryNatives.push({ ...signatureOf(model, st.type, st, reg.name), registry: reg.name, id: null, native, sides: info.sides, doc: docOf(st) });
                }
            } else if (ts.isEnumDeclaration(st)) {
                // This set installs the members its own files declare; the merged enum types natives.
                const own = { ...readEnum(st, info.sides), assertInclude: info.assertInclude };
                const full = model.enums.get(own.name)!;
                const prev = set.enums.find((e) => e.name === own.name);
                if (prev) prev.members.push(...own.members);
                else set.enums.push({ ...own, cpp: full.cpp, scope: full.scope });
            }
        }
    }
}

function signatureOf(model: Model, fn: ts.FunctionTypeNode, where: ts.Node, what: string): { params: Param[]; ret: Ty; sig: string } {
    const params: Param[] = [];
    for (const p of fn.parameters) {
        if (isThisVoid(p)) continue;
        if (p.dotDotDotToken) fail(p, `${what}: rest parameters are not bindable`);
        let ty = resolveType(model, p.type, p, `${what}(${p.name.getText()})`);
        if (p.questionToken) ty = { ...ty, optional: true };
        params.push({ name: p.name.getText(), ty });
    }
    const ret = resolveType(model, fn.type, where, `${what} (result)`);
    if (ret.k === "table" || ret.k === "function" || ret.k === "any") fail(where, `${what}: a registry native returning a ${ret.k} is not bindable`);
    return { params, ret, sig: fn.getText().replace(/\s+/g, " ") };
}

// --- C++ ---------------------------------------------------------------------------------------------

function cppIdent(s: string): string {
    return s.replace(/[^A-Za-z0-9_]/g, "_");
}

function cppArgType(ty: Ty): string {
    switch (ty.k) {
        case "string": case "oneof": case "event": return "const char*";
        case "number": return ty.optional ? "D2LuaOpt<double>" : "double";
        case "integer": return ty.optional ? "D2LuaOpt<lua_Integer>" : "lua_Integer";
        case "boolean": return ty.optional ? "D2LuaOpt<bool>" : "bool";
        case "enum": return ty.optional ? `D2LuaOpt<${ty.e.cpp}>` : ty.e.cpp;
        case "handle": return `${ty.c.traits}::Object*`;
        case "function": return "D2LuaFn";
        case "table": return "D2LuaTable";
        case "any": return "D2LuaAny";
        case "void": return "void";
    }
}

function cppRetType(ty: Ty): string {
    if (ty.k === "void") return "void";
    return cppArgType(ty);
}

function cString(s: string): string {
    return JSON.stringify(s);
}

// Code that converts argument i into local aN.
function convertArg(ty: Ty, i: number, local: string, where: string): string {
    const opt = ty.optional ? "true" : "false";
    const w = cString(where);
    switch (ty.k) {
        case "string": return `const char* ${local} = D2LuaArg_String(L, ${i}, ${w}, ${opt});`;
        case "event": return `const char* ${local} = D2LuaArg_EventName(L, ${i}, ${w}, ${cString(ty.map)}, ${opt});`;
        case "oneof": {
            const list = [...ty.values.map(cString), "nullptr"].join(", ");
            return `static const char* const ${local}_values[] = { ${list} };\n    const char* ${local} = D2LuaArg_OneOf(L, ${i}, ${w}, ${local}_values, ${opt});`;
        }
        case "number": return ty.optional ? `D2LuaOpt<double> ${local} = D2LuaArg_OptNumber(L, ${i}, ${w});` : `double ${local} = D2LuaArg_Number(L, ${i}, ${w});`;
        case "integer": return ty.optional ? `D2LuaOpt<lua_Integer> ${local} = D2LuaArg_OptInteger(L, ${i}, ${w});` : `lua_Integer ${local} = D2LuaArg_Integer(L, ${i}, ${w});`;
        case "boolean": return ty.optional ? `D2LuaOpt<bool> ${local} = D2LuaArg_OptBoolean(L, ${i}, ${w});` : `bool ${local} = D2LuaArg_Boolean(L, ${i}, ${w});`;
        case "enum": {
            const values = `kEnum_${cppIdent(ty.e.name)}`;
            const call = `(${ty.e.cpp})D2LuaArg_Enum(L, ${i}, ${w}, ${cString(ty.e.name)}, ${values}, ${ty.e.members.length})`;
            if (ty.optional) {
                return `D2LuaOpt<${ty.e.cpp}> ${local} = { false, (${ty.e.cpp})0 };\n    if (!lua_isnoneornil(L, ${i})) {\n        ${local}.bHas = true;\n        ${local}.tValue = ${call};\n    }`;
            }
            return `${ty.e.cpp} ${local} = ${call};`;
        }
        case "handle": return `${ty.c.traits}::Object* ${local} = (${ty.c.traits}::Object*)D2LuaHandle_Check(L, ${i}, &kClass_${ty.c.name}, ${w}, ${opt});`;
        case "function": return `D2LuaFn ${local} = D2LuaArg_Function(L, ${i}, ${w}, ${opt});`;
        case "table": return `D2LuaTable ${local} = D2LuaArg_Table(L, ${i}, ${w}, ${opt});`;
        case "any": return `D2LuaAny ${local} = D2LuaArg_Any(L, ${i});`;
        case "void": return "";
    }
}

// Code that pushes value expr of type ty; evaluates to the number pushed in the return statement.
function pushResult(ty: Ty, expr: string): string {
    switch (ty.k) {
        case "void": return `${expr};\n    return 0;`;
        case "string": case "oneof": case "event":
            return `const char* r = ${expr};\n    if (r) {\n        lua_pushstring(L, r);\n    } else {\n        lua_pushnil(L);\n    }\n    return 1;`;
        case "number": return ty.optional
            ? `D2LuaOpt<double> r = ${expr};\n    if (r.bHas) {\n        lua_pushnumber(L, r.tValue);\n    } else {\n        lua_pushnil(L);\n    }\n    return 1;`
            : `lua_pushnumber(L, ${expr});\n    return 1;`;
        case "integer": return ty.optional
            ? `D2LuaOpt<lua_Integer> r = ${expr};\n    if (r.bHas) {\n        lua_pushnumber(L, (lua_Number)r.tValue);\n    } else {\n        lua_pushnil(L);\n    }\n    return 1;`
            : `lua_pushnumber(L, (lua_Number)(${expr}));\n    return 1;`;
        case "boolean": return ty.optional
            ? `D2LuaOpt<bool> r = ${expr};\n    if (r.bHas) {\n        lua_pushboolean(L, r.tValue);\n    } else {\n        lua_pushnil(L);\n    }\n    return 1;`
            : `lua_pushboolean(L, (${expr}) ? 1 : 0);\n    return 1;`;
        case "enum": return ty.optional
            ? `D2LuaOpt<${ty.e.cpp}> r = ${expr};\n    if (r.bHas) {\n        lua_pushinteger(L, (lua_Integer)r.tValue);\n    } else {\n        lua_pushnil(L);\n    }\n    return 1;`
            : `lua_pushinteger(L, (lua_Integer)(${expr}));\n    return 1;`;
        case "handle": return `D2LuaHandle_PushObject(L, &kClass_${ty.c.name}, (void*)(${expr}));\n    return 1;`;
        default: throw new GenError(`cannot return ${ty.k}`);
    }
}

function needsPrototype(native: string): boolean {
    return /^[A-Za-z_][A-Za-z0-9_]*$/.test(native);
}

class CppWriter {
    lines: string[] = [];
    protos = new Map<string, string>();
    line(s = "") { this.lines.push(s); }
    proto(native: string, decl: string) {
        if (!needsPrototype(native)) return;
        const prev = this.protos.get(native);
        if (prev && prev !== decl) throw new GenError(`native ${native} is bound with two signatures:\n  ${prev}\n  ${decl}`);
        this.protos.set(native, decl);
    }
}

function emitFunctionGlue(w: CppWriter, fnName: string, m: Member, where: string, ctxExpr: string, self?: ClassInfo) {
    w.line(`static int ${fnName}(lua_State* L) {`);
    w.line(`    D2LuaCtx ctx = ${ctxExpr};`);
    if (m.raw) {
        w.proto(m.raw, `int ${m.raw}(D2LuaCtx& ctx);`);
        w.line(`    return ${m.raw}(ctx);`);
        w.line("}");
        return;
    }
    const args: string[] = ["ctx"];
    const protoArgs: string[] = ["D2LuaCtx& ctx"];
    let first = 1;
    if (self) {
        w.line(`    ${self.traits}::Object* self = (${self.traits}::Object*)D2LuaHandle_Check(L, 1, &kClass_${self.name}, ${cString(where)}, false);`);
        args.push("self");
        protoArgs.push(`${self.traits}::Object* self`);
        first = 2;
    }
    m.params.forEach((p, i) => {
        const local = `a${i + 1}`;
        w.line(`    ${convertArg(p.ty, first + i, local, where)}`);
        args.push(local);
        protoArgs.push(`${cppArgType(p.ty)} ${cppIdent(p.name)}`);
    });
    const native = m.native!;
    w.proto(native, `${cppRetType(m.ret)} ${native}(${protoArgs.join(", ")});`);
    w.line(`    ${pushResult(m.ret, `${native}(${args.join(", ")})`)}`);
    w.line("}");
}

function emitGetter(w: CppWriter, fnName: string, m: Member, where: string, ctxExpr: string, self?: ClassInfo) {
    w.line(`static int ${fnName}(lua_State* L) {`);
    w.line(`    D2LuaCtx ctx = ${ctxExpr};`);
    if (m.raw) {
        w.proto(m.raw, `int ${m.raw}(D2LuaCtx& ctx);`);
        w.line(`    return ${m.raw}(ctx);`);
        w.line("}");
        return;
    }
    const args = ["ctx"];
    const protoArgs = ["D2LuaCtx& ctx"];
    if (self) {
        w.line(`    ${self.traits}::Object* self = (${self.traits}::Object*)D2LuaHandle_Check(L, 1, &kClass_${self.name}, ${cString(where)}, false);`);
        args.push("self");
        protoArgs.push(`${self.traits}::Object* self`);
    }
    w.proto(m.getter!, `${cppRetType(m.ret)} ${m.getter}(${protoArgs.join(", ")});`);
    w.line(`    ${pushResult(m.ret, `${m.getter}(${args.join(", ")})`)}`);
    w.line("}");
}

function emitSetter(w: CppWriter, fnName: string, m: Member, where: string, ctxExpr: string, self?: ClassInfo) {
    w.line(`static int ${fnName}(lua_State* L) {`);
    w.line(`    D2LuaCtx ctx = ${ctxExpr};`);
    const args = ["ctx"];
    const protoArgs = ["D2LuaCtx& ctx"];
    let index = 1;
    if (self) {
        w.line(`    ${self.traits}::Object* self = (${self.traits}::Object*)D2LuaHandle_Check(L, 1, &kClass_${self.name}, ${cString(where)}, false);`);
        args.push("self");
        protoArgs.push(`${self.traits}::Object* self`);
        index = 2;
    }
    w.line(`    ${convertArg(m.ret, index, "value", where)}`);
    args.push("value");
    protoArgs.push(`${cppArgType(m.ret)} value`);
    w.proto(m.setter!, `void ${m.setter}(${protoArgs.join(", ")});`);
    w.line(`    ${m.setter}(${args.join(", ")});`);
    w.line("    return 0;");
    w.line("}");
}

function sideMask(sides: Side[]): string {
    return sides.length ? sides.map((s) => SIDE_BIT[s]).join(" | ") : "0";
}

function emitCpp(set: ApiSet): { cpp: string; header: string; asserts?: string } {
    const name = set.config.name;
    const w = new CppWriter();
    const body = new CppWriter();
    body.protos = w.protos;

    // Enums: checks that the C++ enumerators agree, and the tables plain Lua reads.
    for (const e of set.enums) {
        if (!e.scope || e.assertInclude) continue;
        for (const m of e.members) {
            body.line(`static_assert(${e.scope}::${m.cpp} == (${e.cpp})(${m.value}), "${e.name}.${m.name} does not match ${e.scope}::${m.cpp}");`);
        }
    }
    for (const e of set.enums) {
        const id = cppIdent(e.name);
        body.line(`static const char* const kEnumNames_${id}[] = { ${[...e.members.map((m) => cString(m.name)), "nullptr"].join(", ")} };`);
        if (e.kind === "number") {
            body.line(`static const lua_Number kEnumNumbers_${id}[] = { ${e.members.map((m) => String(m.value)).join(", ")} };`);
        } else {
            body.line(`static const char* const kEnumStrings_${id}[] = { ${e.members.map((m) => cString(String(m.value))).join(", ")} };`);
        }
    }
    // The value lists natives check enum arguments against: every member of the merged enum.
    const usedEnums = new Map<string, EnumInfo>();  // by name: each side's program has its own EnumInfo
    const noteTy = (ty: Ty) => { if (ty.k === "enum") usedEnums.set(ty.e.name, ty.e); };
    for (const g of set.globals) if (g.kind === "singleton") g.members.forEach((m) => { m.params.forEach((p) => noteTy(p.ty)); if (m.setter) noteTy(m.ret); });
    for (const c of set.classes) c.members.forEach((m) => { m.params.forEach((p) => noteTy(p.ty)); if (m.setter) noteTy(m.ret); });
    for (const r of set.registryNatives) { r.params.forEach((p) => noteTy(p.ty)); }
    for (const e of usedEnums.values()) {
        body.line(`static const lua_Integer kEnum_${cppIdent(e.name)}[] = { ${e.members.map((m) => String(m.value)).join(", ")} };`);
    }
    body.line();

    // Handle classes: forward the descriptors so glue can refer to them.
    const usedClasses = new Set<ClassInfo>(set.classes);
    for (const c of set.classes) {
        body.line(`extern const D2LuaHandleClass kClass_${c.name};`);
    }
    body.line();
    for (const c of set.classes) {
        const ctx = "D2LuaCtx_FromOwner(L)";
        const methods: string[] = [];
        const getters: string[] = [];
        const setters: string[] = [];
        for (const m of c.members) {
            const where = `${c.name}.${m.name}`;
            if (m.kind === "method") {
                const fn = `M_${c.name}_${cppIdent(m.name)}`;
                emitFunctionGlue(body, fn, m, where, ctx, c);
                methods.push(`{ ${cString(m.name)}, ${fn} }`);
            } else {
                const g = `Get_${c.name}_${cppIdent(m.name)}`;
                emitGetter(body, g, m, where, ctx, c);
                getters.push(`{ ${cString(m.name)}, ${g} }`);
                if (m.setter) {
                    const s = `Set_${c.name}_${cppIdent(m.name)}`;
                    emitSetter(body, s, m, where, ctx, c);
                    setters.push(`{ ${cString(m.name)}, ${s} }`);
                }
            }
            body.line();
        }
        body.line(`static const luaL_Reg kMethods_${c.name}[] = { ${[...methods, "{ nullptr, nullptr }"].join(", ")} };`);
        body.line(`static const luaL_Reg kGetters_${c.name}[] = { ${[...getters, "{ nullptr, nullptr }"].join(", ")} };`);
        body.line(`static const luaL_Reg kSetters_${c.name}[] = { ${[...setters, "{ nullptr, nullptr }"].join(", ")} };`);
        body.line(`static void* Resolve_${c.name}(uint64_t nId) {`);
        body.line(`    return (void*)${c.traits}::Resolve(nId);`);
        body.line("}");
        body.line(`static uint64_t IdOf_${c.name}(void* pObject) {`);
        body.line(`    return ${c.traits}::IdOf((${c.traits}::Object*)pObject);`);
        body.line("}");
        body.line(`const D2LuaHandleClass kClass_${c.name} = { ${cString(c.name)}, Resolve_${c.name}, IdOf_${c.name}, kMethods_${c.name}, kGetters_${c.name}, kSetters_${c.name} };`);
        body.line();
    }

    // Singletons and global functions.
    type Install = { sides: Side[]; code: string[] };
    const installs: Install[] = [];
    for (const g of set.globals) {
        if (g.kind === "singleton") {
            const prefix = `${cppIdent(g.name)}_${g.sides.join("_")}`;
            const methods: string[] = [];
            const getters: string[] = [];
            const setters: string[] = [];
            for (const m of g.members) {
                const where = `${g.name}.${m.name}`;
                if (m.kind === "method") {
                    const fn = `G_${prefix}_${cppIdent(m.name)}`;
                    emitFunctionGlue(body, fn, m, where, "D2LuaCtx_FromUpvalue(L)");
                    methods.push(`{ ${cString(m.name)}, ${fn} }`);
                } else {
                    const gf = `Get_${prefix}_${cppIdent(m.name)}`;
                    emitGetter(body, gf, m, where, "D2LuaCtx_FromUpvalue(L)");
                    getters.push(`{ ${cString(m.name)}, ${gf} }`);
                    if (m.setter) {
                        const sf = `Set_${prefix}_${cppIdent(m.name)}`;
                        emitSetter(body, sf, m, where, "D2LuaCtx_FromUpvalue(L)");
                        setters.push(`{ ${cString(m.name)}, ${sf} }`);
                    }
                }
                body.line();
            }
            body.line(`static const luaL_Reg kMethods_${prefix}[] = { ${[...methods, "{ nullptr, nullptr }"].join(", ")} };`);
            body.line(`static const luaL_Reg kGetters_${prefix}[] = { ${[...getters, "{ nullptr, nullptr }"].join(", ")} };`);
            body.line(`static const luaL_Reg kSetters_${prefix}[] = { ${[...setters, "{ nullptr, nullptr }"].join(", ")} };`);
            body.line();
            installs.push({ sides: g.sides, code: [
                `D2LuaApi_PushSingleton(L, pMod, ${cString(g.name)}, kMethods_${prefix}, kGetters_${prefix}, kSetters_${prefix});`,
                `lua_setfield(L, nEnv, ${cString(g.name)});`,
            ] });
        } else if (g.kind === "function") {
            const fn = `F_${cppIdent(g.name)}`;
            emitFunctionGlue(body, fn, g.member, g.name, "D2LuaCtx_FromUpvalue(L)");
            body.line();
            installs.push({ sides: g.sides, code: [
                "lua_pushlightuserdata(L, pMod);",
                `lua_pushcclosure(L, ${fn}, 1);`,
                `lua_setfield(L, nEnv, ${cString(g.name)});`,
            ] });
        } else {
            w.proto(g.native, `int ${g.native}(D2LuaCtx& ctx);`);
            installs.push({ sides: g.sides, code: [
                "{",
                "    D2LuaCtx ctx = D2LuaCtx_ForMod(L, pMod);",
                `    int nPushed = ${g.native}(ctx);`,
                "    if (nPushed == 1) {",
                `        lua_setfield(L, nEnv, ${cString(g.name)});`,
                "    } else {",
                "        lua_pop(L, nPushed);",
                "    }",
                "}",
            ] });
        }
    }
    // Enums: the read-only table in each environment.
    for (const e of set.enums) {
        installs.push({ sides: e.sides, code: [`D2LuaApi_InstallEnum(L, nEnv, ${cString(e.name)});`] });
    }

    // Registry natives.
    const registers: { sides: Side[]; code: string[] }[] = [];
    for (const e of set.enums) {
        const id = cppIdent(e.name);
        const values = e.kind === "number" ? `kEnumNumbers_${id}, nullptr` : `nullptr, kEnumStrings_${id}`;
        registers.push({ sides: e.sides, code: [`D2LuaApi_DefineEnum(L, ${cString(e.name)}, kEnumNames_${id}, ${values});`] });
    }
    for (const c of set.classes) registers.push({ sides: c.sides, code: [`D2LuaHandle_Register(L, &kClass_${c.name});`] });
    for (const map of set.eventMaps) {
        const names = [...map.events.map((e) => cString(e.name)), "nullptr"].join(", ");
        registers.push({ sides: map.sides, code: [
            "{",
            `    static const char* const s_apszNames[] = { ${names} };`,
            `    D2LuaApi_DefineEvents(L, ${cString(map.name)}, ${sideMask(map.sides)}, s_apszNames);`,
            "}",
        ] });
    }
    for (const r of set.registryDefs) {
        registers.push({ sides: r.sides, code: [`D2LuaApi_DefineRegistry(L, ${cString(r.name)}, ${r.kind === "keyed" ? "D2LUA_REGISTRY_KEYED" : "D2LUA_REGISTRY_SLOT"}, ${sideMask(r.sides)});`] });
    }
    for (const r of set.registryNatives) {
        const fn = `R_${cppIdent(r.registry)}_${cppIdent(r.id ?? "slot")}`;
        const m: Member = { name: r.id ?? r.registry, kind: "method", doc: r.doc, params: r.params, ret: r.ret, native: r.native, readonly: true, sig: r.sig };
        emitFunctionGlue(body, fn, m, r.id ? `${r.registry}["${r.id}"]` : r.registry, "D2LuaCtx_FromOwner(L)");
        body.line();
        registers.push({ sides: r.sides, code: [`D2LuaApi_RegistryNative(L, ${cString(r.registry)}, ${r.id ? cString(r.id) : "nullptr"}, ${fn});`] });
    }

    // Raise helpers for events whose payload fields are all bindable values.
    const header: string[] = [];
    const raiseProtos: string[] = [];
    for (const map of set.eventMaps) {
        for (const ev of map.events) {
            const fields = payloadFields(map.model, ev.node);
            if (!fields) continue;
            const params = fields.map((f) => `${f.ty.k === "handle" ? `${f.ty.c.traits}::Object*` : cppArgType({ ...f.ty, optional: false })} ${cppIdent(f.name)}`);
            const decl = `bool D2LuaRaise_${cppIdent(ev.name)}(lua_State* L${params.map((p) => ", " + p).join("")})`;
            raiseProtos.push(`${decl};`);
            body.line(`${decl} {`);
            body.line(`    lua_createtable(L, 0, ${fields.length + 2});`);
            for (const f of fields) {
                const v = cppIdent(f.name);
                switch (f.ty.k) {
                    case "string": case "oneof": case "event": body.line(`    lua_pushstring(L, ${v});`); break;
                    case "number": body.line(`    lua_pushnumber(L, ${v});`); break;
                    case "integer": body.line(`    lua_pushnumber(L, (lua_Number)${v});`); break;
                    case "boolean": body.line(`    lua_pushboolean(L, ${v} ? 1 : 0);`); break;
                    case "enum": body.line(`    lua_pushinteger(L, (lua_Integer)${v});`); break;
                    case "handle": body.line(`    D2LuaHandle_PushObject(L, &kClass_${f.ty.c.name}, (void*)${v});`); break;
                    default: throw new GenError(`payload field ${f.name}`);
                }
                body.line(`    lua_setfield(L, -2, ${cString(f.name)});`);
            }
            body.line(`    return D2LuaApi_RaiseEvent(L, ${cString(ev.name)}) != 0;`);
            body.line("}");
            body.line();
        }
    }

    const emitBySide = (list: { sides: Side[]; code: string[] }[]) => {
        const out: string[] = [];
        for (const item of list) {
            out.push(`    if (nSide & (${sideMask(item.sides)})) {`);
            for (const c of item.code) out.push(`        ${c}`);
            out.push("    }");
        }
        return out;
    };

    const out: string[] = [];
    out.push(`// Generated by Redivivus/Lua/tools/src/gen.ts from ${set.config.bindDirs.map((d) => path.relative(ROOT, d)).join(", ")}. Do not edit;`);
    out.push("// change the declarations and run the generator (node Redivivus/Lua/tools/src/gen.ts).");
    out.push("#if D2_FEATURE_MODS");
    out.push(`#include "${path.relative(ROOT, set.config.header)}"`);
    out.push("");
    out.push("#include <stdint.h>");
    out.push("");
    for (const inc of set.includes) out.push(`#include "${inc}"`);
    if (set.includes.length) out.push("");
    out.push("// The natives this glue calls, with the signatures the declarations give them. A native that");
    out.push("// is missing or has another signature fails the link.");
    for (const p of [...w.protos.values()].sort()) out.push(p);
    out.push("");
    out.push(...body.lines);
    out.push(`static void Register_${name}(lua_State* L, int nSide) {`);
    out.push(...emitBySide(registers));
    if (!registers.length) out.push("    (void)L;\n    (void)nSide;");
    out.push("}");
    out.push("");
    out.push(`static void Install_${name}(lua_State* L, int nEnv, D2LuaMod* pMod, int nSide) {`);
    out.push(...emitBySide(installs));
    if (!installs.length) out.push("    (void)L;\n    (void)nEnv;\n    (void)pMod;\n    (void)nSide;");
    out.push("}");
    out.push("");
    out.push(`const D2LuaApiDesc g_D2LuaApi_${name} = { ${cString(name)}, Register_${name}, Install_${name} };`);
    out.push("#endif");
    out.push("");

    header.push(`// Generated by Redivivus/Lua/tools/src/gen.ts. Do not edit.`);
    header.push("#pragma once");
    header.push("");
    header.push("#if D2_FEATURE_MODS");
    header.push(`#include "Redivivus/Lua/d2_lua_api.h"`);
    for (const inc of set.includes) header.push(`#include "${inc}"`);
    header.push("");
    header.push(`extern const D2LuaApiDesc g_D2LuaApi_${name};`);
    if (raiseProtos.length) {
        header.push("");
        header.push("// Raise an event on the state's runtime: builds the payload and dispatches it to every mod.");
        header.push(...raiseProtos);
    }
    header.push("#endif");
    header.push("");
    void usedClasses;
    // Value checks against headers that only the game builds with.
    const asserted = set.enums.filter((e) => e.scope && e.assertInclude);
    let asserts: string | undefined;
    if (asserted.length) {
        const a: string[] = [
            `// Generated by Redivivus/Lua/tools/src/gen.ts. Do not edit. Every engine enum value a mod sees equals the`,
            "// engine's constant; a header that changes one fails here until the generator runs again. Not part of",
            "// the host test, whose system headers clash with the engine's.",
            "#if D2_FEATURE_MODS",
            ...[...new Set(asserted.map((e) => e.assertInclude!))].map((i) => `#include "${i}"`),
            "",
        ];
        for (const e of asserted) {
            for (const m of e.members) a.push(`static_assert(${e.scope}::${m.cpp} == (${e.cpp})(${m.value}), "${e.name}.${m.name} does not match ${e.scope}::${m.cpp}");`);
        }
        a.push("#endif", "");
        asserts = a.join("\n");
    }
    return { cpp: out.join("\n"), header: header.join("\n"), asserts };
}

function payloadFields(model: Model, node: ts.PropertySignature): { name: string; ty: Ty }[] | undefined {
    if (!node.type || !ts.isTypeReferenceNode(node.type)) return undefined;
    const sym = model.checker.getSymbolAtLocation(node.type.typeName);
    const iface = sym?.declarations?.find(ts.isInterfaceDeclaration);
    if (!iface || iface.name.text === "EventBase") return undefined;
    const fields: { name: string; ty: Ty }[] = [];
    for (const m of iface.members) {
        if (!ts.isPropertySignature(m)) return undefined;
        const ty = resolveType(model, m.type, m, `${iface.name.text}.${m.name.getText()}`);
        if (["table", "function", "any", "void"].includes(ty.k)) return undefined;
        fields.push({ name: m.name.getText(), ty });
    }
    return fields;
}

// --- docs -------------------------------------------------------------------------------------------

function emitDocs(set: ApiSet): string {
    const out: string[] = [];
    out.push("# Redivivus Lua API reference");
    out.push("");
    out.push("Generated from `Redivivus/Lua/api/*.d.ts` by `Redivivus/Lua/tools/src/gen.ts`. Do not edit.");
    out.push("The declarations are the spec: TypeScript mods use them directly (`@redivivus/api/<side>`),");
    out.push("plain Lua mods get the same as LuaLS annotations in `api/luals/`.");
    out.push("");
    out.push("Calls on globals are dot calls (`script.on_event(...)`); methods of handle classes are colon");
    out.push("calls (`unit:damage(5)`). A handle is valid while its object exists; `.valid` says so, and");
    out.push("any other use of an invalid handle raises an error.");
    out.push("");
    for (const side of SIDES) {
        const files = set.files.filter((f) => f.info.sides.includes(side));
        if (!files.length) continue;
        out.push(`## ${side}`);
        out.push("");
        for (const g of set.globals.filter((g) => g.sides.includes(side))) {
            if (g.kind === "singleton") {
                out.push(`### \`${g.name}\` (${g.iface})`);
                out.push("");
                if (g.doc) out.push(g.doc, "");
                for (const m of g.members) {
                    out.push(`- \`${m.sig}\``);
                    if (m.doc) out.push(`  ${m.doc.replace(/\n/g, "\n  ")}`);
                }
                out.push("");
            } else if (g.kind === "function") {
                out.push(`### \`${g.member.sig}\``);
                out.push("");
                if (g.member.doc) out.push(g.member.doc, "");
            } else {
                out.push(`### \`${g.name}: ${g.type}\``);
                out.push("");
                if (g.doc) out.push(g.doc, "");
            }
        }
        for (const map of set.eventMaps.filter((m) => m.sides.includes(side))) {
            out.push(`### Events (${map.name})`);
            out.push("");
            for (const e of map.events) out.push(`- \`${e.member}\` (\`"${e.name}"\`): \`${e.payload}\`${e.doc ? ` - ${e.doc}` : ""}`);
            out.push("");
        }
        for (const r of set.registryDefs.filter((r) => r.sides.includes(side))) {
            out.push(`### Registry \`registries.${r.name}\` (${r.kind === "keyed" ? `keyed, ids in ${r.map}` : `single slot, ${r.map}`})`);
            out.push("");
            for (const n of set.registryNatives.filter((n) => n.registry === r.name)) {
                out.push(`- ${n.id ? `\`${n.id}\`` : "the slot"}: \`${n.sig}\`${n.doc ? ` - ${n.doc.replace(/\n/g, " ")}` : ""}`);
            }
            out.push("");
        }
    }
    if (set.enums.length) {
        out.push("## Enums");
        out.push("");
        out.push("Declared as `declare const enum`: TypeScript inlines the values. Plain Lua has a read-only table of the");
        out.push("same name in every environment of the enum's sides (`UnitType.Player`); `pairs` walks its members.");
        out.push("");
        for (const e of set.enums) {
            const origin = e.scope ? ` (C++ \`${e.cpp}\`)` : "";
            out.push(`### \`${e.name}\`${origin}, ${e.members.length} members, ${e.sides.join(" ")}`);
            out.push("");
            if (e.doc) out.push(e.doc, "");
            if (e.members.length <= 48) {
                out.push(e.members.map((m) => `\`${m.name}\` = \`${JSON.stringify(m.value)}\``).join(", "));
            } else {
                out.push(`${e.members.slice(0, 12).map((m) => `\`${m.name}\` = \`${JSON.stringify(m.value)}\``).join(", ")}, ... (all in the declarations)`);
            }
            out.push("");
        }
    }
    out.push("## storage");
    out.push("");
    out.push("A mod's `storage` is saved with the game. It holds booleans, numbers, strings, tables without a");
    out.push("metatable, and handles; keys are strings, numbers or booleans. Shared tables and cycles are kept.");
    out.push("Functions, coroutines and other userdata are refused with the path of the value in the error.");
    out.push("");
    return out.join("\n");
}

// --- LuaLS ------------------------------------------------------------------------------------------

function luaType(ty: Ty, set: ApiSet): string {
    let t: string;
    switch (ty.k) {
        case "string": t = "string"; break;
        case "number": t = "number"; break;
        case "integer": t = "integer"; break;
        case "boolean": t = "boolean"; break;
        case "void": t = "nil"; break;
        case "any": t = "any"; break;
        case "function": t = "function"; break;
        case "table": t = "table"; break;
        case "enum": t = ty.e.name; break;
        case "handle": t = ty.c.name; break;
        case "oneof": t = ty.enumName ?? ty.values.map((v) => JSON.stringify(v)).join("|"); break;
        case "event": {
            const members = set.eventMaps.filter((m) => m.name === ty.map).flatMap((m) => m.events.map((e) => e.member));
            t = members.length ? members[0].split(".")[0] : "string";
            break;
        }
    }
    return ty.optional ? (t.includes("|") ? `(${t})?` : `${t}?`) : t;
}

function luaDoc(doc: string): string[] {
    return doc ? doc.split("\n").map((l) => `--- ${l}`) : [];
}

function emitLuaLs(set: ApiSet, side: Side): string {
    const out: string[] = ["---@meta", `-- Redivivus Lua API, ${side} side. Generated by Redivivus/Lua/tools/src/gen.ts; do not edit.`, ""];
    for (const e of set.enums.filter((e) => e.sides.includes(side))) {
        out.push(...luaDoc(e.doc), "---@enum " + e.name, `${e.name} = {`);
        for (const m of e.members) {
            out.push(...luaDoc(m.doc).map((l) => "    " + l), `    ${m.name} = ${JSON.stringify(m.value)},`);
        }
        out.push("}", "");
    }
    for (const g of set.globals.filter((g) => g.sides.includes(side))) {
        if (g.kind === "singleton") {
            out.push(...luaDoc(g.doc), `---@class ${g.iface}`);
            for (const m of g.members.filter((m) => m.kind === "property")) out.push(`---@field ${m.name} ${m.raw ? "table" : luaType(m.ret, set)} ${m.doc.split("\n")[0]}`);
            out.push(`${g.name} = {}`, "");
            for (const m of g.members.filter((m) => m.kind === "method")) {
                out.push(...luaDoc(m.doc));
                if (m.raw) {
                    out.push(`---@param ... any`, `function ${g.name}.${m.name}(...) end`, "");
                    continue;
                }
                for (const p of m.params) out.push(`---@param ${p.name}${p.ty.optional ? "?" : ""} ${luaType({ ...p.ty, optional: false }, set)}`);
                if (m.ret.k !== "void") out.push(`---@return ${luaType(m.ret, set)}`);
                out.push(`function ${g.name}.${m.name}(${m.params.map((p) => p.name).join(", ")}) end`, "");
            }
        } else if (g.kind === "function") {
            out.push(...luaDoc(g.member.doc), "---@param ... any", `function ${g.name}(...) end`, "");
        } else {
            out.push(...luaDoc(g.doc), `---@type table`, `${g.name} = {}`, "");
        }
    }
    for (const r of set.registryDefs.filter((r) => r.sides.includes(side))) {
        out.push(`--- Registry ${r.name}: ${r.kind === "keyed" ? "register(id, impl), wrap(id, fn(next, ...)), call(id, ...), has(id)" : "set(impl), wrap(fn(next, ...)), call(...)"}`);
    }
    return out.join("\n") + "\n";
}

// LuaLS settings for a plain-Lua mod: LuaJIT, and the libraries the sandbox removes switched off so
// using them is flagged. Copy beside the mod and point workspace.library at api/luals.
const LUARC = JSON.stringify({
    "runtime.version": "LuaJIT",
    "runtime.builtin": Object.fromEntries(["debug", "ffi", "io", "jit", "jit.profile", "jit.util", "os", "package", "string.buffer",
        "table.clear", "table.new", "utf8", "bit32"].map((l) => [l, "disable"])),
    "diagnostics.globals": [],
}, null, 4) + "\n";

// The language service plugin (lsplugin.ts and the checks it shares with the build), as CommonJS
// for tsserver, in the api package.
function lsPluginOutputs(): Map<string, string> {
    const out = new Map<string, string>();
    for (const name of ["lsplugin", "scriptcheck"]) {
        const source = fs.readFileSync(path.join(TOOLS, "src", `${name}.ts`), "utf8");
        const js = ts.transpileModule(source, {
            compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2020, rewriteRelativeImportExtensions: true },
            fileName: `${name}.ts`,
        }).outputText;
        const banner = `// Generated from Redivivus/Lua/tools/src/${name}.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.\n`;
        out.set(path.join(LUA, "api/ls-plugin", name === "lsplugin" ? "index.js" : "scriptcheck.js"), banner + js);
    }
    return out;
}

// --- main ---------------------------------------------------------------------------------------------

function main(): number {
    const check = process.argv.includes("--check");
    const outputs = new Map<string, string>();
    try {
        // The declarations below read these, so without --check they are written first.
        const early = luaEnvTypings(path.join(LUA, "api/lua"));
        early.set(path.join(LUA, "api/enums.d.ts"), engineEnums(path.join(LUA, "api/engine-enums.json"), path.join(ROOT, "Source/D2Headers/d2_enums.h"), ROOT));
        for (const [file, text] of early) {
            outputs.set(file, text);
            if (!check && (!fs.existsSync(file) || fs.readFileSync(file, "utf8") !== text)) {
                fs.mkdirSync(path.dirname(file), { recursive: true });
                fs.writeFileSync(file, text);
                console.log(`gen: wrote ${path.relative(ROOT, file)}`);
            }
        }
        outputs.set(path.join(LUA, "api/luals/.luarc.json"), LUARC);
        for (const config of SETS) {
            const set = loadSet(config);
            const { cpp, header, asserts } = emitCpp(set);
            if (asserts) outputs.set(config.cpp.replace(/\.gen\.cpp$/, "_asserts.gen.cpp"), asserts);
            outputs.set(config.cpp, cpp);
            outputs.set(config.header, header);
            if (config.docs) outputs.set(config.docs, emitDocs(set));
            if (config.luals) {
                for (const side of SIDES) outputs.set(path.join(config.luals, `${side}.lua`), emitLuaLs(set, side));
            }
        }
        for (const [file, text] of lsPluginOutputs()) outputs.set(file, text);
        // The kit compiles against the declarations written above.
        for (const [file, text] of kitOutputs()) outputs.set(file, text);
    } catch (e) {
        if (e instanceof GenError || e instanceof EnumError || e instanceof KitError) {
            console.error(`gen: ${e.message}`);
            return 1;
        }
        throw e;
    }
    let stale = 0;
    for (const file of staleKitFiles(outputs)) {
        console.error(`gen: ${path.relative(ROOT, file)} has no kit source any more; remove it`);
        stale++;
    }
    for (const [file, text] of outputs) {
        const old = fs.existsSync(file) ? fs.readFileSync(file, "utf8") : undefined;
        if (old === text) continue;
        if (check) {
            console.error(`gen: stale: ${path.relative(ROOT, file)} (run node Redivivus/Lua/tools/src/gen.ts)`);
            stale++;
        } else {
            fs.mkdirSync(path.dirname(file), { recursive: true });
            fs.writeFileSync(file, text);
            console.log(`gen: wrote ${path.relative(ROOT, file)}`);
        }
    }
    if (check) {
        console.log(stale ? `gen: ${stale} stale output(s)` : `gen: ${outputs.size} outputs up to date`);
    }
    return stale ? 1 : 0;
}

process.exit(main());
