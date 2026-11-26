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
// action immediately instead of handing back a generator, unlike what TypeScript's types say. The
// one other place is an argument where a Script is expected (guard's body): there the call is
// handed over unstarted, as a function that runs it, its callee and arguments evaluated in place:
//   guard(c, chase(m, t))  ->  guard(c, (function(f, a1, a2) return function() return f(a1, a2) end end)(chase, m, t))
//
// The runtime library. TSTL emits require("lualib_bundle"); a mod requires redivivus:lualib instead,
// the copy built into the game, so a mod ships none.
//
// Source maps. "sources" is rewritten to the TypeScript file relative to the Lua file, both in the
// mod's namespace, which is what the host's error mapping resolves against the chunk name.

import * as path from "node:path";
import ts from "typescript";
import * as tstl from "typescript-to-lua";
import { type FunctionNode, MESSAGE_CALL, MESSAGE_DEFER_SPREAD, MESSAGE_DELEGATE, SCRIPT_DIAGNOSTIC, scriptChecks } from "./scriptcheck.ts";

export const LUALIB_REQUIRE = "redivivus:lualib";

const { isDeferredArgument, isScriptFunction, misusedCall, misusedDelegate } = scriptChecks(ts);

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

// A Script call as a function that runs it: (function(f, a1, ...) return function() return f(a1, ...) end end)(callee, args...),
// so the callee and the arguments are evaluated where the call stands, the body only when run.
function deferCall(call: tstl.Expression, node: ts.Node): tstl.Expression | undefined {
    const inline = tstl.NodeFlags.Inline;
    const closure = (params: tstl.Identifier[], run: tstl.Expression): tstl.Expression =>
        tstl.createFunctionExpression(tstl.createBlock([tstl.createReturnStatement([
            tstl.createFunctionExpression(tstl.createBlock([tstl.createReturnStatement([run])]), [], undefined, inline),
        ])]), params, undefined, inline);
    const argNames = (n: number) => Array.from({ length: n }, (_, i) => tstl.createIdentifier(`____arg${i + 1}`));
    if (tstl.isCallExpression(call)) {
        const args = call.params;
        const f = tstl.createIdentifier("____body");
        const names = argNames(args.length);
        const run = tstl.createCallExpression(tstl.cloneIdentifier(f), names.map((a) => tstl.cloneIdentifier(a)));
        return tstl.createCallExpression(closure([f, ...names], run), [call.expression, ...args], node);
    }
    if (tstl.isMethodCallExpression(call)) {
        const args = call.params;
        const self = tstl.createIdentifier("____self");
        const names = argNames(args.length);
        const run = tstl.createMethodCallExpression(tstl.cloneIdentifier(self), tstl.cloneIdentifier(call.name), names.map((a) => tstl.cloneIdentifier(a)));
        return tstl.createCallExpression(closure([self, ...names], run), [call.prefixExpression, ...args], node);
    }
    return undefined;
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
                if (!isDeferredArgument(context.checker, node)) return context.superTransformExpression(node);
                if (node.arguments.some((a) => ts.isSpreadElement(a))) {
                    context.diagnostics.push(diagnostic(node, MESSAGE_DEFER_SPREAD));
                    return context.superTransformExpression(node);
                }
                const call = context.superTransformExpression(node);
                const deferred = deferCall(call, node);
                if (!deferred) {
                    context.diagnostics.push(diagnostic(node, "This Script call cannot be handed over unstarted; call it from a Script function and pass that."));
                    return call;
                }
                return deferred;
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
