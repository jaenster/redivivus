// A TypeScript language service plugin: shows in the editor, at the place, what the mod build
// reports - a Script function called without yield*, a yield* of something that is not an action
// (scriptcheck.ts), a wrong lifecycle export of main (server(game), client(client)), and a
// module-level variable written in one side's export and read in the other's. The generator
// compiles it to CommonJS in api/ls-plugin/ (tsserver loads plugins with require), and rdvmod's
// tsconfig names it:
//   "compilerOptions": { "plugins": [{ "name": "@redivivus/api/ls-plugin" }] }

import type TS from "typescript";
import { mainChecks, SCRIPT_DIAGNOSTIC, scriptChecks } from "./scriptcheck.ts";

function init(modules: { typescript: typeof TS }) {
    const ts = modules.typescript;
    const checks = scriptChecks(ts);
    const main = mainChecks(ts);
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
            const checker = program.getTypeChecker();
            const issues = [
                ...checks.misuses(checker, sf).map((m) => ({ node: m.node, message: m.message, code: SCRIPT_DIAGNOSTIC, warning: false })),
                ...main.exportIssues(checker, sf),
                ...main.crossSideIssues(checker, sf),
            ];
            const ours = issues.map((m): TS.Diagnostic => ({
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

export = init;
