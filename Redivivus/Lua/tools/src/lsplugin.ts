// A TypeScript language service plugin: shows the Script misuses the mod build refuses (a Script
// function called without yield*, a yield* of something that is not an action; see scriptcheck.ts)
// in the editor, at the call. The generator compiles it to CommonJS in api/ls-plugin/ (tsserver
// loads plugins with require), and rdvmod's tsconfig names it:
//   "compilerOptions": { "plugins": [{ "name": "@redivivus/api/ls-plugin" }] }

import type TS from "typescript";
import { SCRIPT_DIAGNOSTIC, scriptChecks } from "./scriptcheck.ts";

function init(modules: { typescript: typeof TS }) {
    const ts = modules.typescript;
    const checks = scriptChecks(ts);
    function create(info: TS.server.PluginCreateInfo): TS.LanguageService {
        const ls = info.languageService;
        const proxy: TS.LanguageService = Object.create(null);
        for (const key of Object.keys(ls) as (keyof TS.LanguageService)[]) {
            const member = ls[key];
            (proxy as unknown as Record<string, unknown>)[key] = typeof member === "function" ? (...args: unknown[]) => (member as Function).apply(ls, args) : member;
        }
        proxy.getSemanticDiagnostics = (fileName: string) => {
            const prior = ls.getSemanticDiagnostics(fileName);
            const program = ls.getProgram();
            const sf = program?.getSourceFile(fileName);
            if (!program || !sf) return prior;
            const ours = checks.misuses(program.getTypeChecker(), sf).map((m): TS.Diagnostic => ({
                file: sf,
                start: m.node.getStart(),
                length: m.node.getWidth(),
                messageText: m.message,
                category: ts.DiagnosticCategory.Error,
                code: SCRIPT_DIAGNOSTIC,
                source: "rdvmod",
            }));
            return [...prior, ...ours];
        };
        return proxy;
    }
    return { create };
}

export = init;
