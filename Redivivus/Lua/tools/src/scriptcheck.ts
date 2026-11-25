// What makes a function a Script function, and the two ways of misusing one, shared by the
// TypeScriptToLua plugin (plugin.ts, which fails the build) and the language service plugin
// (lsplugin.ts, which shows the same errors in an editor). Written against the typescript module
// it is handed, because a language service plugin must use the editor's own copy.

import type TS from "typescript";

export const SCRIPT_DIAGNOSTIC = 91001;
export const MESSAGE_CALL =
    "A Script function runs only as `yield* action(...)` inside another Script function, or as the body handed to script.start.";
export const MESSAGE_DELEGATE = "In a Script function, yield* takes a call of a Script function (an action).";

export type FunctionNode = TS.FunctionDeclaration | TS.FunctionExpression | TS.MethodDeclaration;

export function scriptChecks(ts: typeof TS) {
    function isScriptType(type: TS.Type | undefined): boolean {
        if (!type) return false;
        const symbol = type.aliasSymbol ?? type.getSymbol();
        if (!symbol || symbol.getName() !== "Script") return false;
        return (symbol.declarations ?? []).some((d) => /[\\/]api[\\/]core\.d\.ts$/.test(d.getSourceFile().fileName));
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

    // A call of a Script function that is not the operand of yield* in a Script function.
    function misusedCall(checker: TS.TypeChecker, node: TS.CallExpression): boolean {
        if (!returnsScript(checker, checker.getResolvedSignature(node))) return false;
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
            if (ts.isYieldExpression(n) && misusedDelegate(checker, n)) out.push({ node: n, message: MESSAGE_DELEGATE });
            ts.forEachChild(n, visit);
        };
        visit(sf);
        return out;
    }

    return { isScriptType, returnsScript, isScriptFunction, enclosingFunction, misusedCall, misusedDelegate, misuses };
}
