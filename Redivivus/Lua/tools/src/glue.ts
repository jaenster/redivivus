// The native binding glue: C++ that the host compiles, generated from declarations. Two kinds of
// input, both read as TypeScript:
//   - the native spec (Redivivus/Lua/core/native/*.d.ts, hand-written): the functions the native
//     layer gives the core (`export declare const core: NativeCore`, installed as natives.core) and
//     handle classes (engine objects named by id, the native object API mods use directly)
//   - the core's public modules (core/redivivus/*.ts): the enums, the events maps and the
//     registries the native layer has to know (raise helpers for engine hook sites, the native
//     implementations at the bottom of registry entries, the enum values plain Lua reads)
// Each glue set is a list of files; "main" goes into the game, "test" into the host test only.
//
// Tags (JSDoc):
//   @side <data|server|client ...>   file level (first comment): the sides a file's natives and
//                                    handle classes are registered on
//   @include "<header>"              file level: a header the glue includes (handle traits)
//   @assertInclude "<header>"        file level: the header the static_asserts of its @native enums
//                                    need; those go to a unit of their own (*_asserts.gen.cpp)
//   @native <C++ function>           a native function or method
//   @get / @set <C++ function>       a handle property's getter / setter
//   @raw <C++ function>              the native takes the Lua stack itself (int f(D2LuaCtx&))
//   @handle <C++ traits type>        a class whose instances are handles (d2_lua_api.h)
//   @native <C++ enum> / @scope      on a numeric const enum: its C++ type, for the static_asserts
//   @events <sides>                  an interface mapping Event members to payload types
//   @registry                        an interface mapping registry ids to entry signatures
//   @registries <sides>              the interface listing the registries (KeyedRegistry<Map>,
//                                    SlotRegistry<Signature>)
// (@noSelf is TypeScriptToLua's: required on the native interfaces.)
//
// Natives are called by the core only. They are installed into the table the core starts with,
// never into a mod environment; @raw natives check their own arguments.

import * as path from "node:path";
import ts from "typescript";

const TOOLS = path.resolve(import.meta.dirname, "..");
const LUA = path.resolve(TOOLS, "..");
const ROOT = path.resolve(LUA, "../..");

export type SetConfig = {
    name: string;
    /** Files whose declarations become values of this set. */
    bind: string[];
    cpp: string;
    header: string;
};

const SIDES = ["data", "server", "client"] as const;
type Side = (typeof SIDES)[number];
const SIDE_BIT: Record<Side, string> = { data: "D2LUA_SIDE_DATA", server: "D2LUA_SIDE_SERVER", client: "D2LUA_SIDE_CLIENT" };

function sidesOf(text: string, where: string): Side[] {
    const sides = text.trim().split(/\s+/).filter((s) => s.length);
    for (const s of sides) if (!(SIDES as readonly string[]).includes(s)) throw new GenError(`${where}: unknown side '${s}'`);
    return sides as Side[];
}

export class GenError extends Error {}

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

const KNOWN_TAGS = new Set(["side", "include", "native", "get", "set", "raw", "handle", "events", "registry", "registries", "noSelf", "internal"]);

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
    if (!mods.some((m) => m.kind === ts.SyntaxKind.ConstKeyword)) {
        fail(e, `enum ${name}: write 'const enum' (TypeScript inlines the values; plain Lua reads the table)`);
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

export type ApiSet = {
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

// Top-level statements, and those inside `declare module "..." { }` augmentations.
function statementsOf(sf: ts.SourceFile): ts.Statement[] {
    const out: ts.Statement[] = [];
    for (const st of sf.statements) {
        if (ts.isModuleDeclaration(st) && ts.isStringLiteral(st.name) && st.body && ts.isModuleBlock(st.body)) {
            out.push(...st.body.statements);
        } else {
            out.push(st);
        }
    }
    return out;
}

function isExported(st: ts.Statement): boolean {
    return !!(ts.canHaveModifiers(st) && ts.getModifiers(st)?.some((m) => m.kind === ts.SyntaxKind.ExportKeyword || m.kind === ts.SyntaxKind.DeclareKeyword));
}

// Read one glue set. contextFiles: every declaration file of the API (the native spec and the
// core's public modules); config.bind: the ones whose declarations become this set's values.
export function loadSet(config: SetConfig, program: ts.Program, contextFiles: string[]): ApiSet {
    const set: ApiSet = { config, files: [], globals: [], classes: [], enums: [], eventMaps: [], registryDefs: [], registryNatives: [], includes: [] };
    const model = new Model(program);
    const context = contextFiles.map((f) => program.getSourceFile(f)).filter((sf): sf is ts.SourceFile => !!sf);
    const bindSet = new Set(config.bind.map((f) => path.resolve(f)));
    const registrySides = new Map<string, Side[]>();

    // Pass 1, every file: enums, handle classes, events maps, registries.
    for (const sf of context) {
        const sides = fileInfo(sf)?.sides ?? [];
        for (const st of statementsOf(sf)) {
            if (ts.isEnumDeclaration(st)) mergeEnum(model, readEnum(st, sides));
            if (ts.isClassDeclaration(st) && st.name) {
                const traits = tagsOf(st).get("handle");
                if (traits) model.classes.set(st.name.text, { name: st.name.text, traits, members: [], sides, doc: docOf(st), decl: st });
            }
            if (ts.isInterfaceDeclaration(st) && tagsOf(st).has("events")) model.eventMaps.add(st.name.text);
            if (ts.isInterfaceDeclaration(st) && tagsOf(st).has("registries")) {
                const regSides = sidesOf(tagsOf(st).get("registries")!, `${st.name.text} @registries`);
                for (const m of st.members) {
                    if (!ts.isPropertySignature(m) || !m.type || !ts.isTypeReferenceNode(m.type)) fail(m, "registries are KeyedRegistry<Map> or SlotRegistry<Signature>");
                    const kindName = m.type.typeName.getText();
                    const arg = m.type.typeArguments?.[0];
                    if (!arg || !ts.isTypeReferenceNode(arg) || (kindName !== "KeyedRegistry" && kindName !== "SlotRegistry")) {
                        fail(m, "registries are KeyedRegistry<Map> or SlotRegistry<Signature>");
                    }
                    const kind = kindName === "KeyedRegistry" ? "keyed" : "slot";
                    const regName = m.name.getText();
                    model.registryOf.set(arg.typeName.getText(), { name: regName, kind });
                    registrySides.set(regName, regSides);
                    if (bindSet.has(path.resolve(sf.fileName))) set.registryDefs.push({ name: regName, kind, map: arg.typeName.getText(), sides: regSides });
                }
            }
        }
    }

    // Pass 2, this set's files: everything that becomes a value.
    for (const sf of context) {
        if (!bindSet.has(path.resolve(sf.fileName))) continue;
        const info = fileInfo(sf);
        if (!info) throw new GenError(`${path.relative(ROOT, sf.fileName)}: a bound file needs @side in its first comment`);
        set.files.push({ sf, info });
        for (const inc of info.includes) if (!set.includes.includes(inc)) set.includes.push(inc);
        for (const st of statementsOf(sf)) {
            if (ts.isVariableStatement(st)) {
                if (!sf.isDeclarationFile) continue;  // a core module's runtime values are the core's
                for (const d of st.declarationList.declarations) {
                    const name = d.name.getText();
                    if (!d.type || !ts.isTypeReferenceNode(d.type)) fail(d, `${name}: natives are typed by a @noSelf interface`);
                    const sym = model.checker.getSymbolAtLocation(d.type.typeName);
                    const target = sym && sym.flags & ts.SymbolFlags.Alias ? model.checker.getAliasedSymbol(sym) : sym;
                    const ifaces = target?.declarations?.filter(ts.isInterfaceDeclaration) ?? [];
                    if (!ifaces.length) fail(d, `${name}: natives are typed by a @noSelf interface`);
                    if (!ifaces.some((i) => tagsOf(i).has("noSelf"))) fail(d, `${name}: its interface needs @noSelf`);
                    const members: Member[] = [];
                    for (const i of ifaces) {
                        for (const m of i.members) {
                            if (!ts.isMethodSignature(m)) fail(m, `${name}: a native table holds functions only`);
                            members.push(memberFromSignature(model, m, name));
                        }
                    }
                    set.globals.push({ kind: "singleton", name, iface: d.type.getText(), members, sides: info.sides, doc: docOf(st), noSelf: true, node: d });
                }
            } else if (ts.isClassDeclaration(st)) {
                const cls = st.name && model.classes.get(st.name.text);
                if (!cls) continue;
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
                    const map: EventMap = { name: st.name.text, sides: sidesOf(tags.get("events")!, `${st.name.text} @events`), events: [], model };
                    for (const m of st.members) {
                        if (!ts.isPropertySignature(m) || !m.type) fail(m, `${st.name.text}: events are '[Event.Name]: PayloadType'`);
                        map.events.push({ ...enumKey(model, m, st.name.text), payload: m.type.getText(), doc: docOf(m), node: m });
                    }
                    set.eventMaps.push(map);
                } else if (tags.has("registry")) {
                    const reg = model.registryOf.get(st.name.text);
                    if (!reg) fail(st, `${st.name.text}: @registry interface is not the map of any registry (@registries)`);
                    for (const m of st.members) {
                        if (!ts.isPropertySignature(m) || !m.type || !ts.isFunctionTypeNode(m.type)) fail(m, `${st.name.text}: entries are '[Id.Member]: (this: void, ...) => R'`);
                        const native = tagsOf(m).get("native");
                        const id = enumKey(model, m, st.name.text).name;
                        if (!native) fail(m, `${st.name.text}["${id}"]: no binding (@native for the entry's base implementation)`);
                        set.registryNatives.push({ ...signatureOf(model, m.type, m, `${reg.name}["${id}"]`), registry: reg.name, id, native, sides: registrySides.get(reg.name) ?? [], doc: docOf(m) });
                    }
                }
            } else if (ts.isTypeAliasDeclaration(st)) {
                const reg = model.registryOf.get(st.name.text);
                if (reg && reg.kind === "slot") {
                    const native = tagsOf(st).get("native");
                    if (!native) fail(st, `${st.name.text}: slot registry signature without @native base implementation`);
                    if (!ts.isFunctionTypeNode(st.type)) fail(st, `${st.name.text}: a slot signature is a function type`);
                    set.registryNatives.push({ ...signatureOf(model, st.type, st, reg.name), registry: reg.name, id: null, native, sides: registrySides.get(reg.name) ?? [], doc: docOf(st) });
                }
            } else if (ts.isEnumDeclaration(st)) {
                const own = { ...readEnum(st, info.sides), assertInclude: info.assertInclude };
                const full = model.enums.get(own.name)!;
                const prev = set.enums.find((e) => e.name === own.name);
                if (prev) prev.members.push(...own.members);
                else set.enums.push({ ...own, cpp: full.cpp, scope: full.scope });
            }
            void isExported;
        }
    }
    return set;
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

export function emitCpp(set: ApiSet): { cpp: string; header: string; asserts?: string } {
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
        body.line(`static void* Resolve_${c.name}(lua_State* L, uint64_t nId) {`);
        body.line("    (void)L;");
        body.line(`    return (void*)${c.traits}::Resolve(nId);`);
        body.line("}");
        body.line(`static uint64_t IdOf_${c.name}(void* pObject) {`);
        body.line(`    return ${c.traits}::IdOf((${c.traits}::Object*)pObject);`);
        body.line("}");
        body.line(`const D2LuaHandleClass kClass_${c.name} = { ${cString(c.name)}, Resolve_${c.name}, IdOf_${c.name}, kMethods_${c.name}, kGetters_${c.name}, kSetters_${c.name} };`);
        body.line();
    }

    // Native tables: each singleton becomes a table of C functions in the core's natives table.
    type Install = { sides: Side[]; code: string[] };
    const installs: Install[] = [];
    for (const g of set.globals) {
        if (g.kind !== "singleton") continue;
        const prefix = `${cppIdent(g.name)}`;
        const methods: string[] = [];
        for (const m of g.members) {
            const fn = `N_${prefix}_${cppIdent(m.name)}`;
            emitFunctionGlue(body, fn, m, `${g.name}.${m.name}`, "D2LuaCtx_FromOwner(L)");
            methods.push(`{ ${cString(m.name)}, ${fn} }`);
            body.line();
        }
        body.line(`static const luaL_Reg kNatives_${prefix}[] = { ${[...methods, "{ nullptr, nullptr }"].join(", ")} };`);
        body.line();
        installs.push({ sides: g.sides, code: [
            `D2LuaApi_PushNatives(L, kNatives_${prefix});`,
            `lua_setfield(L, nTable, ${cString(g.name)});`,
        ] });
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
    out.push(`// Generated by Redivivus/Lua/tools/src/gen.ts from ${set.config.bind.map((d) => path.relative(ROOT, d)).join(", ")}. Do not edit;`);
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
    out.push(`static void Natives_${name}(lua_State* L, int nTable, int nSide) {`);
    out.push(...emitBySide(installs));
    if (!installs.length) out.push("    (void)L;\n    (void)nTable;\n    (void)nSide;");
    out.push("}");
    out.push("");
    out.push(`const D2LuaApiDesc g_D2LuaApi_${name} = { ${cString(name)}, Register_${name}, Natives_${name} };`);
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
