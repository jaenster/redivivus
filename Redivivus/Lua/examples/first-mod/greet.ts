// Plain exports other mods import as "first-mod:greet", and an id this mod adds to the example
// registry, which other mods may call and wrap, typed.
import type { ExampleId } from "redivivus:enums";

declare module "redivivus:enums" {
    const enum ExampleId {
        Shout = "first-mod:shout",
    }
}
declare module "redivivus:server" {
    interface ExampleRegistry {
        [ExampleId.Shout]: (this: void, text: string) => string;
    }
}

export function greet(name: string): string {
    return `hello, ${name}`;
}

export function double(n: number): number {
    return n * 2;
}
