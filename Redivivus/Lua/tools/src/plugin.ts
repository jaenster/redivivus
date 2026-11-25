// A TypeScriptToLua plugin for Redivivus mods.
//
// Scripts. A generator function whose return type is the API's Script<T> (declared or taken from
// context, as in `script.start(function* () { ... })`) is compiled to a plain Lua function, not
// to TSTL's generator object:
//   yield x        -> coroutine.yield(x)        (what TSTL emits anyway)
//   yield* f(...)  -> f(...)                    (a plain call: f yields on the same coroutine)
//   the function   -> function(...) ... end     (no __TS__Generator wrapper)
// Lua coroutines are stackful, so a Script function yields the coroutine of whatever script runs
// it, however deep the call. A Script function written in Lua (a plain function that yields) and
// one written in TypeScript are the same thing at run time, so either calls the other.
// Ordinary generators (not returning Script) stay on TSTL's path.
// Calling a Script function anywhere but as the operand of `yield*` is an error: it would run the
// action immediately instead of handing back a generator, unlike what TypeScript's types say.
//
// The runtime library. TSTL emits require("lualib_bundle"); a mod requires base's copy instead,
// so the runtime ships once (Redivivus/Lua/mods/base/shared/lualib_bundle.lua).
//
// Source maps. "sources" is rewritten to the TypeScript file relative to the Lua file, both
// mod-relative, which is what the host's error mapping resolves against the chunk name.

import * as path from "node:path";
import ts from "typescript";
import * as tstl from "typescript-to-lua";
import { type FunctionNode, MESSAGE_CALL, MESSAGE_DELEGATE, SCRIPT_DIAGNOSTIC, scriptChecks } from "./scriptcheck.ts";

export const LUALIB_REQUIRE = "__base__/shared/lualib_bundle";

const { isScriptFunction, misusedCall, misusedDelegate } = scriptChecks(ts);

function diagnostic(node: ts.Node, message: string): ts.Diagnostic {
    return {
        file: node.getSourceFile(),
        start: node.getStart(),
        length: node.getWidth(),
        messageText: message,
        category: ts.DiagnosticCategory.Error,
        code: SCRIPT_DIAGNOSTIC,
        source: "rdvmod",
    };
}

// Replace the first __TS__Generator(fn) found without entering function bodies with fn.
function unwrapGenerator(nodes: tstl.Node[]): boolean {
    const visit = (parent: Record<string, unknown>, key: string | number, value: unknown): boolean => {
        if (!value || typeof value !== "object") return false;
        const n = value as tstl.Node;
        if (Array.isArray(value)) {
            for (let i = 0; i < value.length; i++) if (visit(value as unknown as Record<string, unknown>, i, value[i])) return true;
            return false;
        }
        if (tstl.isCallExpression(n) && tstl.isIdentifier(n.expression) && n.expression.text === "__TS__Generator"
            && n.params.length === 1 && tstl.isFunctionExpression(n.params[0])) {
            parent[key] = n.params[0];
            return true;
        }
        if (tstl.isFunctionExpression(n)) return false;
        for (const k of ["right", "expressions", "expression", "params", "value", "fields"]) {
            if (k in (n as object) && visit(n as unknown as Record<string, unknown>, k, (n as unknown as Record<string, unknown>)[k])) return true;
        }
        return false;
    };
    const holder = { nodes } as Record<string, unknown>;
    return visit(holder, "nodes", nodes);
}

// Whether a file has a generator that stays on TSTL's path, which needs the runtime's __TS__Generator.
const ordinaryGenerators = new WeakMap<ts.SourceFile, boolean>();
function hasOrdinaryGenerator(checker: ts.TypeChecker, sf: ts.SourceFile): boolean {
    let found = ordinaryGenerators.get(sf);
    if (found === undefined) {
        found = false;
        const visit = (n: ts.Node): void => {
            if (found) return;
            if ((ts.isFunctionDeclaration(n) || ts.isFunctionExpression(n) || ts.isMethodDeclaration(n)) && n.asteriskToken && !isScriptFunction(checker, n)) {
                found = true;
                return;
            }
            ts.forEachChild(n, visit);
        };
        visit(sf);
        ordinaryGenerators.set(sf, found);
    }
    return found;
}

// After a Script function is compiled without its generator wrapper: drop the runtime import
// TSTL recorded for it, unless an ordinary generator in the file still needs it.
function dropGeneratorImport(node: ts.Node, context: tstl.TransformationContext): void {
    if (!hasOrdinaryGenerator(context.checker, node.getSourceFile())) context.usedLuaLibFeatures.delete(tstl.LuaLibFeature.Generator);
}

function functionVisitor<T extends FunctionNode>(kind: "statements" | "expression") {
    return (node: T, context: tstl.TransformationContext) => {
        if (!isScriptFunction(context.checker, node)) {
            return kind === "statements" ? context.superTransformStatements(node as ts.FunctionDeclaration) : context.superTransformExpression(node as ts.FunctionExpression);
        }
        if (kind === "statements") {
            const result = context.superTransformStatements(node as ts.FunctionDeclaration);
            unwrapGenerator(result);
            dropGeneratorImport(node, context);
            return result;
        }
        const holder: tstl.Node[] = [context.superTransformExpression(node as ts.FunctionExpression)];
        unwrapGenerator(holder);
        dropGeneratorImport(node, context);
        return holder[0] as tstl.Expression;
    };
}

export function makePlugin(modRoot: string): tstl.Plugin {
    return {
        visitors: {
            [ts.SyntaxKind.FunctionDeclaration]: functionVisitor<ts.FunctionDeclaration>("statements"),
            [ts.SyntaxKind.FunctionExpression]: functionVisitor<ts.FunctionExpression>("expression"),
            [ts.SyntaxKind.MethodDeclaration]: (node: ts.MethodDeclaration, context: tstl.TransformationContext) => {
                if (!isScriptFunction(context.checker, node)) return context.superTransformNode(node) as never;
                const result = context.superTransformNode(node);
                unwrapGenerator(result);
                dropGeneratorImport(node, context);
                return result as never;
            },
            [ts.SyntaxKind.YieldExpression]: (node: ts.YieldExpression, context: tstl.TransformationContext) => {
                if (!node.asteriskToken || !node.expression) return context.superTransformExpression(node);
                const fn = ts.findAncestor(node.parent, (n) => ts.isFunctionLike(n));
                if (!fn || !isScriptFunction(context.checker, fn)) return context.superTransformExpression(node);
                if (misusedDelegate(context.checker, node)) {
                    context.diagnostics.push(diagnostic(node, MESSAGE_DELEGATE));
                    return context.superTransformExpression(node);
                }
                return context.transformExpression(node.expression);
            },
            [ts.SyntaxKind.CallExpression]: (node: ts.CallExpression, context: tstl.TransformationContext) => {
                if (misusedCall(context.checker, node)) context.diagnostics.push(diagnostic(node, MESSAGE_CALL));
                return context.superTransformExpression(node);
            },
        },
        beforeEmit(program: ts.Program, options: tstl.CompilerOptions, emitHost: tstl.EmitHost, result: tstl.EmitFile[]) {
            void program;
            void emitHost;
            const outDir = options.outDir ?? "";
            for (let i = result.length - 1; i >= 0; i--) {
                const file = result[i];
                if (path.basename(file.outputPath) === "lualib_bundle.lua") {
                    result.splice(i, 1);
                    continue;
                }
                file.code = file.code.split('require("lualib_bundle")').join(`require("${LUALIB_REQUIRE}")`);
                const source = file.sourceFiles?.[0]?.fileName;
                if (file.sourceMap && source) {
                    const luaRel = path.relative(outDir, file.outputPath);
                    const tsRel = path.relative(modRoot, source);
                    const map = JSON.parse(file.sourceMap);
                    map.sources = [path.relative(path.dirname(luaRel), tsRel).split(path.sep).join("/")];
                    map.sourceRoot = "";
                    map.file = path.basename(file.outputPath);
                    file.sourceMap = JSON.stringify(map);
                }
            }
        },
    };
}
