// Imports across mods: only declared dependencies, only what they export.
import { greet } from "__first-mod__/shared/greet";
// @ts-expect-error first-mod does not export this
import { farewell } from "__first-mod__/shared/greet";
// @ts-expect-error lua-user is not a dependency of this mod
import { anything } from "__lua-user__/shared/whatever";
// @ts-expect-error a module first-mod does not have
import { walk } from "__first-mod__/shared/nope";

const s: string = greet("x");
// @ts-expect-error greet returns a string
const n: number = greet("x");
void s;
void n;
void farewell;
void anything;
void walk;
