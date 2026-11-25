// Generated from Redivivus/Lua/kit/server/persist.ts. Do not edit.
/** The persisted state of owner, created from defaults the first time. */
export declare function persist<T extends object>(owner: Handle, defaults: T): T;
/** Whether owner has persisted state. */
export declare function hasPersisted(owner: Handle): boolean;
/** Drop the state of every owner whose object is gone. The module's script calls it each tick. */
export declare function sweep(): integer;
