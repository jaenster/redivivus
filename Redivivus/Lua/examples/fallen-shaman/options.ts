// The example's settings (mods/settings.json, "fallen-shaman": { ... }).
import { SettingScope } from "redivivus:enums";
import { defineSettings, setting } from "redivivus:shared";

export const options = defineSettings({
    /** Leave the stock AI in place (only wrap it to log what it did), to compare the two. */
    stock: setting.bool({ default: false, scope: SettingScope.Game, title: "Keep the stock AI" }),
    /** Spawn a shaman with its Fallen near a player out of town and kill one Fallen, to watch it raised. */
    demo: setting.bool({ default: false, scope: SettingScope.Game, title: "Spawn a shaman to watch" }),
    /** Log what each think decided. */
    trace: setting.bool({ default: true, scope: SettingScope.Game, title: "Log each decision" }),
});
