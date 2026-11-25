// This mod's settings, declared once with the kit; read wherever this module is imported.
import { defineSettings, setting } from "__base__/shared/kit/settings";

export const enum Pace {
    Slow = "slow",
    Fast = "fast",
}

export const settings = defineSettings({
    greetings: setting.int({ default: 3, min: 1, max: 10, scope: SettingScope.Startup, title: "Greetings" }),
    pace: setting.choice({ choices: [Pace.Slow, Pace.Fast], default: Pace.Slow, scope: SettingScope.Game }),
});
