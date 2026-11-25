// Typed mod settings. A mod declares its settings once, in a shared module, and imports that module
// wherever it reads them (its settings.lua too, so the data stage lists them at boot):
//
//   import { defineSettings, setting } from "__base__/shared/kit/settings";
//   export const settings = defineSettings({
//       boost: setting.int({ default: 0, min: 0, max: 5, scope: SettingScope.Startup }),
//       mode: setting.choice({ choices: [Mode.Easy, Mode.Hard], default: Mode.Easy, scope: SettingScope.Game }),
//   });
//   settings.boost   // number, read-only
//
// Values come from the settings file under the mods root, checked by the host against the
// declaration; a bad or missing value is the default. A setting is read on its scope's side only.
// From Lua: local s = require("__base__/shared/kit/settings"); s.defineSettings({ boost = s.setting.int({ ... }) }).

/** A declared setting whose value is a T. */
export interface SettingSpec<T> {
    readonly declaration: SettingDeclaration;
    /** Carries T for the types; never set. */
    readonly value?: T;
}

type Common = { readonly scope: SettingScope; readonly title?: string; readonly description?: string };

function spec<T>(declaration: SettingDeclaration): SettingSpec<T> {
    return { declaration };
}

/** The builders of setting declarations. */
export const setting = {
    bool(this: void, o: Common & { readonly default: boolean }): SettingSpec<boolean> {
        return spec({ ...o, type: SettingType.Bool });
    },
    int(this: void, o: Common & { readonly default: integer; readonly min?: integer; readonly max?: integer }): SettingSpec<integer> {
        return spec({ ...o, type: SettingType.Int });
    },
    number(this: void, o: Common & { readonly default: number; readonly min?: number; readonly max?: number }): SettingSpec<number> {
        return spec({ ...o, type: SettingType.Number });
    },
    text(this: void, o: Common & { readonly default: string }): SettingSpec<string> {
        return spec({ ...o, type: SettingType.String });
    },
    choice<const C extends string>(this: void, o: Common & { readonly choices: readonly C[]; readonly default: NoInfer<C> }): SettingSpec<C> {
        return spec({ ...o, type: SettingType.Choice });
    },
};

/** The read-only values of a set of declared settings. */
export type Settings<T> = { readonly [K in keyof T]: T[K] extends SettingSpec<infer V> ? V : never };

/**
 * Declare the settings (sorted by name, which is the order a settings screen lists them in) and
 * get an object that reads their values. Writing to it is an error.
 */
export function defineSettings<T extends Record<string, SettingSpec<unknown>>>(specs: T): Settings<T> {
    const names = Object.keys(specs).sort();
    const declared = new LuaTable<string, true>();
    for (const name of names) {
        mod_settings.declare(name, specs[name].declaration);
        declared.set(name, true);
    }
    const values = setmetatable({}, {
        __index(this: void, _: object, key: string) {
            if (!declared.has(key)) error(`no setting '${tostring(key)}' in this set`, 2);
            return mod_settings.get(key);
        },
        __newindex(this: void, _: object, key: string) {
            error(`settings are read-only (setting '${tostring(key)}')`, 2);
        },
        __metatable: false,
    });
    return values as unknown as Settings<T>;
}
