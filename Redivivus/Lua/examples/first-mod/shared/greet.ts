// Shared by both sides, and imported by other mods as "__first-mod__/shared/greet".

declare global {
    // An id this mod adds to the example registry: other mods may call and wrap it, typed.
    const enum ExampleId {
        Shout = "first-mod:shout",
    }
    interface ExampleRegistry {
        [ExampleId.Shout]: (this: void, text: string) => string;
    }
    // This mod's remote interface, typed for callers.
    interface RemoteInterfaces {
        "first-mod": {
            add(this: void, a: number, b: number): number;
        };
    }
}

export function greet(name: string): string {
    return `hello, ${name}`;
}

export function double(n: number): number {
    return n * 2;
}
