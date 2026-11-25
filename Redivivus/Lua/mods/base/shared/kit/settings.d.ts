// Generated from Redivivus/Lua/kit/shared/settings.ts. Do not edit.
/** A declared setting whose value is a T. */
export interface SettingSpec<T> {
    readonly declaration: SettingDeclaration;
    /** Carries T for the types; never set. */
    readonly value?: T;
}
type Common = {
    readonly scope: SettingScope;
    readonly title?: string;
    readonly description?: string;
};
/** The builders of setting declarations. */
export declare const setting: {
    bool(this: void, o: Common & {
        readonly default: boolean;
    }): SettingSpec<boolean>;
    int(this: void, o: Common & {
        readonly default: integer;
        readonly min?: integer;
        readonly max?: integer;
    }): SettingSpec<integer>;
    number(this: void, o: Common & {
        readonly default: number;
        readonly min?: number;
        readonly max?: number;
    }): SettingSpec<number>;
    text(this: void, o: Common & {
        readonly default: string;
    }): SettingSpec<string>;
    choice<const C extends string>(this: void, o: Common & {
        readonly choices: readonly C[];
        readonly default: NoInfer<C>;
    }): SettingSpec<C>;
};
/** The read-only values of a set of declared settings. */
export type Settings<T> = {
    readonly [K in keyof T]: T[K] extends SettingSpec<infer V> ? V : never;
};
/**
 * Declare the settings (sorted by name, which is the order a settings screen lists them in) and
 * get an object that reads their values. Writing to it is an error.
 */
export declare function defineSettings<T extends Record<string, SettingSpec<unknown>>>(specs: T): Settings<T>;
export {};
