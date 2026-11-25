// Generated from Redivivus/Lua/tools/src/lsplugin.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
"use strict";
// A TypeScript language service plugin: shows the Script misuses the mod build refuses (a Script
// function called without yield*, a yield* of something that is not an action; see scriptcheck.ts)
// in the editor, at the call. The generator compiles it to CommonJS in api/ls-plugin/ (tsserver
// loads plugins with require), and rdvmod's tsconfig names it:
//   "compilerOptions": { "plugins": [{ "name": "@redivivus/api/ls-plugin" }] }
const scriptcheck_ts_1 = require("./scriptcheck.js");
function init(modules) {
    const ts = modules.typescript;
    const checks = (0, scriptcheck_ts_1.scriptChecks)(ts);
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
            const ours = checks.misuses(program.getTypeChecker(), sf).map((m) => ({
                file: sf,
                start: m.node.getStart(),
                length: m.node.getWidth(),
                messageText: m.message,
                category: ts.DiagnosticCategory.Error,
                code: scriptcheck_ts_1.SCRIPT_DIAGNOSTIC,
                source: "rdvmod",
            }));
            return [...prior, ...ours];
        };
        return proxy;
    }
    return { create };
}
module.exports = init;
