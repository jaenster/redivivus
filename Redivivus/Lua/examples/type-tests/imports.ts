// Imports across mods: only declared dependencies, only what they export.
import { greet } from "first-mod:greet";
// @ts-expect-error first-mod does not export this
import { farewell } from "first-mod:greet";
// @ts-expect-error a module first-mod does not have
import { walk } from "first-mod:nope";
// @ts-expect-error there are no API globals: everything is imported
log("x");

const s: string = greet("x");
// @ts-expect-error greet returns a string
const n: number = greet("x");
void [s, n, farewell, walk];
