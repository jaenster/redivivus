-- The kit's typed settings, from plain Lua.
local kit = require("__base__/shared/kit/settings")
local s = kit.setting
return kit.defineSettings({
  boost = s.int({ default = 1, min = 0, max = 5, scope = SettingScope.Startup, title = "Boost" }),
  rate = s.number({ default = 0.5, min = 0, max = 1, scope = SettingScope.Startup }),
  mode = s.choice({ choices = { "easy", "hard" }, default = "easy", scope = SettingScope.Game }),
  hud = s.bool({ default = true, scope = SettingScope.Player }),
  motto = s.text({ default = "hi", scope = SettingScope.Startup }),
})
