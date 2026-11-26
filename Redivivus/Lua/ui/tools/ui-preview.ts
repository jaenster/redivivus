#!/usr/bin/env node
// ui-preview: builds the kit, then renders an entry to a PNG (host/cli.ts has the options).

import * as path from "node:path";
import { pathToFileURL } from "node:url";
import { buildNode, OUT_NODE } from "./build.ts";

const argv = process.argv.slice(2);
if (!argv.includes("--no-build")) buildNode();
const cli = (await import(pathToFileURL(path.join(OUT_NODE, "ui/host/cli.js")).href)) as { main(a: string[]): Promise<number> };
try {
    process.exitCode = await cli.main(argv.filter((a) => a !== "--no-build"));
} catch (e) {
    console.error(`ui-preview: ${(e as Error).message}`);
    process.exitCode = 1;
}
