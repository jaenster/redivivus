// Raw table access as the core uses it: on any table, with any key. The core reads and writes
// tables a mod handed it only through these (and next), so no metamethod of the mod's runs in the
// middle of core code.
export {};

declare global {
    function rawget(table: object, key: unknown): any;
    function rawset(table: object, key: unknown, value: unknown): void;
    function rawequal(a: unknown, b: unknown): boolean;
}
