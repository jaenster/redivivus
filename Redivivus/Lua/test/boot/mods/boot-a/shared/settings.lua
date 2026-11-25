local kit = require("__base__/shared/kit/settings")
return kit.defineSettings({
  volume = kit.setting.int({ default = 5, min = 0, max = 10, scope = SettingScope.Startup }),
  lang = kit.setting.choice({ choices = { "en", "nl" }, default = "en", scope = SettingScope.Player }),
})
