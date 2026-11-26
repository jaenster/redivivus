// The example's settings (mods/settings.json, "mephisto": { ... }).
import { SettingScope } from "redivivus:enums";
import { defineSettings, setting } from "redivivus:shared";

export const options = defineSettings({
    /** Which AI runs Mephisto: the composed port, the mechanical one, or the game's own. */
    version: setting.choice({ choices: ["composed", "switch", "stock"], default: "composed", scope: SettingScope.Game, title: "Mephisto's AI" }),
});
