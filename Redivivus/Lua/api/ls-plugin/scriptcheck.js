// Generated from Redivivus/Lua/tools/src/scriptcheck.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
"use strict";
// What makes a function a Script function, and the two ways of misusing one, shared by the
// TypeScriptToLua plugin (plugin.ts, which fails the build) and the language service plugin
// (lsplugin.ts, which shows the same errors in an editor). Written against the typescript module
// it is handed, because a language service plugin must use the editor's own copy.
Object.defineProperty(exports, "__esModule", { value: true });
exports.MESSAGE_DELEGATE = exports.MESSAGE_CALL = exports.SCRIPT_DIAGNOSTIC = void 0;
exports.scriptChecks = scriptChecks;
exports.SCRIPT_DIAGNOSTIC = 91001;
exports.MESSAGE_CALL = "A Script function runs only as `yield* action(...)` inside another Script function, or as the body handed to script.start.";
exports.MESSAGE_DELEGATE = "In a Script function, yield* takes a call of a Script function (an action).";
function scriptChecks(ts) {
    function isScriptType(type) {
        if (!type)
            return false;
        const symbol = type.aliasSymbol ?? type.getSymbol();
        if (!symbol || symbol.getName() !== "Script")
            return false;
        return (symbol.declarations ?? []).some((d) => /[\\/]api[\\/]core\.d\.ts$/.test(d.getSourceFile().fileName));
    }
    function returnsScript(checker, signature) {
        return !!signature && isScriptType(checker.getReturnTypeOfSignature(signature));
    }
    function isScriptFunction(checker, node) {
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
    function enclosingFunction(node) {
        return ts.findAncestor(node.parent, (n) => ts.isFunctionLike(n));
    }
    // A call of a Script function that is not the operand of yield* in a Script function.
    function misusedCall(checker, node) {
        if (!returnsScript(checker, checker.getResolvedSignature(node)))
            return false;
        const parent = node.parent;
        const delegated = ts.isYieldExpression(parent) && !!parent.asteriskToken;
        const fn = enclosingFunction(node);
        return !delegated || !fn || !isScriptFunction(checker, fn);
    }
    // A yield* in a Script function whose operand is not a call of a Script function.
    function misusedDelegate(checker, node) {
        if (!node.asteriskToken)
            return false;
        const fn = enclosingFunction(node);
        if (!fn || !isScriptFunction(checker, fn))
            return false;
        const operand = node.expression;
        return !operand || !ts.isCallExpression(operand) || !returnsScript(checker, checker.getResolvedSignature(operand));
    }
    // Every misuse in a file.
    function misuses(checker, sf) {
        const out = [];
        const visit = (n) => {
            if (ts.isCallExpression(n) && misusedCall(checker, n))
                out.push({ node: n, message: exports.MESSAGE_CALL });
            if (ts.isYieldExpression(n) && misusedDelegate(checker, n))
                out.push({ node: n, message: exports.MESSAGE_DELEGATE });
            ts.forEachChild(n, visit);
        };
        visit(sf);
        return out;
    }
    return { isScriptType, returnsScript, isScriptFunction, enclosingFunction, misusedCall, misusedDelegate, misuses };
}
