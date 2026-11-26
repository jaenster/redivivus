// What makes a function a Script function, and the two ways of misusing one, shared by the
// TypeScriptToLua plugin (plugin.ts, which fails the build) and the language service plugin
// (lsplugin.ts, which shows the same errors in an editor). Written against the typescript module
// it is handed, because a language service plugin must use the editor's own copy.

import type TS from "typescript";

export const SCRIPT_DIAGNOSTIC = 91001;
export const MESSAGE_CALL =
    "A Script function runs only as `yield* action(...)` inside another Script function, or as the body handed to startScript.";
export const MESSAGE_DELEGATE = "In a Script function, yield* takes a call of a Script function (an action).";
export const MESSAGE_DEFER_SPREAD = "A Script call handed over unstarted (an argument such as guard's body) cannot take a spread argument.";

export type FunctionNode = TS.FunctionDeclaration | TS.FunctionExpression | TS.MethodDeclaration;

export function scriptChecks(ts: typeof TS) {
    function isScriptType(type: TS.Type | undefined): boolean {
        if (!type) return false;
        const symbol = type.aliasSymbol ?? type.getSymbol();
        if (!symbol || symbol.getName() !== "Script") return false;
        return (symbol.declarations ?? []).some((d) => /[\\/]redivivus[\\/]shared\.(d\.)?ts$/.test(d.getSourceFile().fileName));
    }

    function returnsScript(checker: TS.TypeChecker, signature: TS.Signature | undefined): boolean {
        return !!signature && isScriptType(checker.getReturnTypeOfSignature(signature));
    }

    function isScriptFunction(checker: TS.TypeChecker, node: TS.Node): node is FunctionNode {
        if (!(ts.isFunctionDeclaration(node) || ts.isFunctionExpression(node) || ts.isMethodDeclaration(node)) || !node.asteriskToken) {
            return false;
        }
        if (node.type) {
            return isScriptType(checker.getTypeFromTypeNode(node.type));
        }
        if (ts.isFunctionExpression(node)) {
            const contextual = checker.getContextualType(node);
            const signatures = contextual?.getCallSignatures() ?? [];
            return signatures.some((s) => returnsScript(checker, s));
        }
        return false;
    }

    function enclosingFunction(node: TS.Node): TS.Node | undefined {
        return ts.findAncestor(node.parent, (n) => ts.isFunctionLike(n));
    }

    // A call of a Script function passed as an argument where a Script is expected (guard's body):
    // the build hands it over unstarted, as a function that runs it, its arguments evaluated in place.
    function isDeferredArgument(checker: TS.TypeChecker, node: TS.CallExpression): boolean {
        const parent = node.parent;
        if (!parent || !(ts.isCallExpression(parent) || ts.isNewExpression(parent)) || !parent.arguments?.includes(node)) return false;
        if (!returnsScript(checker, checker.getResolvedSignature(node))) return false;
        return isScriptType(checker.getContextualType(node));
    }

    // A call of a Script function that is not the operand of yield* in a Script function, nor an
    // argument where a Script is expected.
    function misusedCall(checker: TS.TypeChecker, node: TS.CallExpression): boolean {
        if (!returnsScript(checker, checker.getResolvedSignature(node))) return false;
        if (isDeferredArgument(checker, node)) return false;
        const parent = node.parent;
        const delegated = ts.isYieldExpression(parent) && !!parent.asteriskToken;
        const fn = enclosingFunction(node);
        return !delegated || !fn || !isScriptFunction(checker, fn);
    }

    // A yield* in a Script function whose operand is not a call of a Script function.
    function misusedDelegate(checker: TS.TypeChecker, node: TS.YieldExpression): boolean {
        if (!node.asteriskToken) return false;
        const fn = enclosingFunction(node);
        if (!fn || !isScriptFunction(checker, fn)) return false;
        const operand = node.expression;
        return !operand || !ts.isCallExpression(operand) || !returnsScript(checker, checker.getResolvedSignature(operand));
    }

    // Every misuse in a file.
    function misuses(checker: TS.TypeChecker, sf: TS.SourceFile): { node: TS.Node; message: string }[] {
        const out: { node: TS.Node; message: string }[] = [];
        const visit = (n: TS.Node): void => {
            if (ts.isCallExpression(n) && misusedCall(checker, n)) out.push({ node: n, message: MESSAGE_CALL });
            if (ts.isCallExpression(n) && isDeferredArgument(checker, n) && n.arguments.some((a) => ts.isSpreadElement(a))) {
                out.push({ node: n, message: MESSAGE_DEFER_SPREAD });
            }
            if (ts.isYieldExpression(n) && misusedDelegate(checker, n)) out.push({ node: n, message: MESSAGE_DELEGATE });
            ts.forEachChild(n, visit);
        };
        visit(sf);
        return out;
    }

    return { isScriptType, returnsScript, isScriptFunction, enclosingFunction, isDeferredArgument, misusedCall, misusedDelegate, misuses };
}

// --- the control entry --------------------------------------------------------------------------
// main.ts (or main.tsx) exports the functions the engine calls: server(game: Game) and
// client(client: Client). A wrong one is reported at the export itself.

export const MAIN_DIAGNOSTIC = 91002;
export const CROSS_DIAGNOSTIC = 91003;

const LIFECYCLE: Record<string, { type: string; module: string; param: string }> = {
    server: { type: "Game", module: "server", param: "game" },
    client: { type: "Client", module: "client", param: "client" },
    menu: { type: "Menu", module: "client", param: "menu" },
};

function distance(a: string, b: string): number {
    const d: number[][] = [];
    for (let i = 0; i <= a.length; i++) d.push([i, ...new Array<number>(b.length).fill(0)]);
    for (let j = 1; j <= b.length; j++) d[0][j] = j;
    for (let i = 1; i <= a.length; i++) {
        for (let j = 1; j <= b.length; j++) {
            d[i][j] = Math.min(d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + (a[i - 1] === b[j - 1] ? 0 : 1));
        }
    }
    return d[a.length][b.length];
}

export type Issue = { node: TS.Node; message: string; code: number; warning?: boolean };

export function mainChecks(ts: typeof TS) {
    function isMain(sf: TS.SourceFile): boolean {
        return /(^|[\\/])main\.tsx?$/.test(sf.fileName);
    }

    // The entry type a type is (Game of redivivus:server, Client of redivivus:client), or undefined.
    function entryTypeName(type: TS.Type | undefined): string | undefined {
        const symbol = type?.aliasSymbol ?? type?.getSymbol();
        if (!symbol) return undefined;
        for (const [, l] of Object.entries(LIFECYCLE)) {
            if (symbol.getName() !== l.type) continue;
            const re = new RegExp(`[\\\\/]redivivus[\\\\/]${l.module}\\.(d\\.)?ts$`);
            if ((symbol.declarations ?? []).some((d) => re.test(d.getSourceFile().fileName))) return l.type;
        }
        return undefined;
    }

    function nameNodeOf(symbol: TS.Symbol): TS.Node | undefined {
        const decl = symbol.declarations?.[0];
        if (!decl) return undefined;
        return (decl as TS.NamedDeclaration).name ?? decl;
    }

    // Wrong lifecycle exports of a main file.
    function exportIssues(checker: TS.TypeChecker, sf: TS.SourceFile): Issue[] {
        if (!isMain(sf)) return [];
        const moduleSymbol = checker.getSymbolAtLocation(sf);
        if (!moduleSymbol) return [];
        const out: Issue[] = [];
        for (const exp of checker.getExportsOfModule(moduleSymbol)) {
            const name = exp.getName();
            const node = nameNodeOf(exp);
            if (!node || node.getSourceFile() !== sf) continue;
            const type = checker.getTypeOfSymbolAtLocation(exp, node);
            const signatures = type.getCallSignatures();
            const life = LIFECYCLE[name];
            if (life) {
                const sig = signatures[0];
                if (!sig) {
                    out.push({ node, code: MAIN_DIAGNOSTIC, message: `main's export '${name}' must be a function: export function ${name}(${life.param}: ${life.type}) { ... }` });
                    continue;
                }
                const param = sig.getParameters()[0];
                if (!param) {
                    out.push({ node, code: MAIN_DIAGNOSTIC, message: `${name}(): the ${life.param} parameter is missing; the engine calls ${name}(${life.param}: ${life.type}) and everything is registered through it` });
                    continue;
                }
                const paramType = checker.getTypeOfSymbolAtLocation(param, node);
                if (entryTypeName(paramType) !== life.type) {
                    out.push({ node, code: MAIN_DIAGNOSTIC, message: `${name} takes a ${life.type} (import type { ${life.type} } from "redivivus:${life.module}"), not ${checker.typeToString(paramType)}` });
                }
                if (sig.getParameters().length > 1) {
                    out.push({ node, code: MAIN_DIAGNOSTIC, message: `${name} takes one parameter, the ${life.param}` });
                }
                continue;
            }
            const takesEntry = signatures.some((s) => s.getParameters().some((p) => entryTypeName(checker.getTypeOfSymbolAtLocation(p, node)) !== undefined));
            const near = Object.keys(LIFECYCLE).find((l) => distance(name.toLowerCase(), l) <= 2);
            if (takesEntry || near) {
                out.push({ node, code: MAIN_DIAGNOSTIC, message: `'${name}' is not a lifecycle export: main exports server(game: Game), client(client: Client) and menu(menu: Menu)${near ? ` (did you mean '${near}'?)` : ""}` });
            }
        }
        return out;
    }

    // Module-level variables written inside one side's export and read inside the other's: the two
    // run in separate Lua states, so the read never sees the write.
    function crossSideIssues(checker: TS.TypeChecker, sf: TS.SourceFile): Issue[] {
        if (!isMain(sf)) return [];
        const bodies = new Map<string, TS.Node>();
        for (const st of sf.statements) {
            if (ts.isFunctionDeclaration(st) && st.name && LIFECYCLE[st.name.text] && st.body) bodies.set(st.name.text, st.body);
            if (ts.isVariableStatement(st)) {
                for (const d of st.declarationList.declarations) {
                    if (ts.isIdentifier(d.name) && LIFECYCLE[d.name.text] && d.initializer) bodies.set(d.name.text, d.initializer);
                }
            }
        }
        const moduleVars = new Set<TS.Symbol>();
        for (const st of sf.statements) {
            if (!ts.isVariableStatement(st)) continue;
            for (const d of st.declarationList.declarations) {
                if (ts.isIdentifier(d.name) && !LIFECYCLE[d.name.text]) {
                    const s = checker.getSymbolAtLocation(d.name);
                    if (s) moduleVars.add(s);
                }
            }
        }
        const isWrite = (id: TS.Identifier): boolean => {
            const p = id.parent;
            if (ts.isBinaryExpression(p) && p.left === id && p.operatorToken.kind >= ts.SyntaxKind.FirstAssignment && p.operatorToken.kind <= ts.SyntaxKind.LastAssignment) return true;
            if ((ts.isPrefixUnaryExpression(p) || ts.isPostfixUnaryExpression(p)) && (p.operator === ts.SyntaxKind.PlusPlusToken || p.operator === ts.SyntaxKind.MinusMinusToken)) return true;
            return false;
        };
        const uses = (body: TS.Node) => {
            const writes = new Set<TS.Symbol>();
            const reads: { symbol: TS.Symbol; node: TS.Identifier }[] = [];
            const visit = (n: TS.Node): void => {
                if (ts.isIdentifier(n)) {
                    const s = checker.getSymbolAtLocation(n);
                    if (s && moduleVars.has(s)) {
                        if (isWrite(n)) writes.add(s);
                        else reads.push({ symbol: s, node: n });
                    }
                }
                ts.forEachChild(n, visit);
            };
            visit(body);
            return { writes, reads };
        };
        const out: Issue[] = [];
        const sides = [...bodies.keys()];
        for (const writer of sides) {
            for (const reader of sides) {
                if (writer === reader) continue;
                const w = uses(bodies.get(writer)!);
                for (const r of uses(bodies.get(reader)!).reads) {
                    if (!w.writes.has(r.symbol)) continue;
                    out.push({
                        node: r.node, code: CROSS_DIAGNOSTIC, warning: true,
                        message: `'${r.node.text}' is written in ${writer}() and read in ${reader}(): the two run in separate Lua states (even in single player), so this read never sees that write. Send it over a channel.`,
                    });
                }
            }
        }
        return out;
    }

    return { isMain, exportIssues, crossSideIssues };
}
