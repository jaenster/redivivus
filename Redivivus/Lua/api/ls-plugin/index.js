// Generated from Redivivus/Lua/tools/src/lsplugin.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
"use strict";
// A TypeScript language service plugin: shows in the editor, at the place, what the mod build
// reports - a Script function called without yield*, a yield* of something that is not an action
// (scriptcheck.ts), a wrong lifecycle export of main (server(game), client(client)), and a
// module-level variable written in one side's export and read in the other's. The generator
// compiles it to CommonJS in api/ls-plugin/ (tsserver loads plugins with require), and rdvmod's
// tsconfig names it:
//   "compilerOptions": { "plugins": [{ "name": "@redivivus/api/ls-plugin" }] }
const scriptcheck_ts_1 = require("./scriptcheck.js");
function init(modules) {
    const ts = modules.typescript;
    const checks = (0, scriptcheck_ts_1.scriptChecks)(ts);
    const main = (0, scriptcheck_ts_1.mainChecks)(ts);
    function create(info) {
        const ls = info.languageService;
        const proxy = Object.create(null);
        for (const key of Object.keys(ls)) {
            const member = ls[key];
            proxy[key] = typeof member === "function" ? (...args) => member.apply(ls, args) : member;
        }
        proxy.getSemanticDiagnostics = (fileName) => {
            const prior = ls.getSemanticDiagnostics(fileName);
            const program = ls.getProgram();
            const sf = program?.getSourceFile(fileName);
            if (!program || !sf)
                return prior;
            const checker = program.getTypeChecker();
            const issues = [
                ...checks.misuses(checker, sf).map((m) => ({ node: m.node, message: m.message, code: scriptcheck_ts_1.SCRIPT_DIAGNOSTIC, warning: false })),
                ...main.exportIssues(checker, sf),
                ...main.crossSideIssues(checker, sf),
            ];
            const ours = issues.map((m) => ({
                file: sf,
                start: m.node.getStart(),
                length: m.node.getWidth(),
                messageText: m.message,
                category: m.warning ? ts.DiagnosticCategory.Warning : ts.DiagnosticCategory.Error,
                code: m.code,
                source: "rdvmod",
            }));
            return [...prior, ...ours];
        };
        return proxy;
    }
    return { create };
}
module.exports = init;
