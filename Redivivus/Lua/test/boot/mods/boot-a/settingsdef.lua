local shared = require("redivivus:shared")
local enums = require("redivivus:enums")
return shared.defineSettings({
  volume = shared.setting.int({ default = 5, min = 0, max = 10, scope = enums.SettingScope.Startup }),
  lang = shared.setting.choice({ choices = { "en", "nl" }, default = "en", scope = enums.SettingScope.Player }),
})
