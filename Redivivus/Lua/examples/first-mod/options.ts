// This mod's settings, declared once; read wherever this module is imported.
import { SettingScope } from "redivivus:enums";
import { defineSettings, setting } from "redivivus:shared";

export const enum Pace {
    Slow = "slow",
    Fast = "fast",
}

export const options = defineSettings({
    greetings: setting.int({ default: 3, min: 1, max: 10, scope: SettingScope.Startup, title: "Greetings" }),
    pace: setting.choice({ choices: [Pace.Slow, Pace.Fast], default: Pace.Slow, scope: SettingScope.Game }),
});
