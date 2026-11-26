-- Typed settings, from plain Lua.
local shared = require("redivivus:shared")
local enums = require("redivivus:enums")
local s = shared.setting
return shared.defineSettings({
  boost = s.int({ default = 1, min = 0, max = 5, scope = enums.SettingScope.Startup, title = "Boost" }),
  rate = s.number({ default = 0.5, min = 0, max = 1, scope = enums.SettingScope.Startup }),
  mode = s.choice({ choices = { "easy", "hard" }, default = "easy", scope = enums.SettingScope.Game }),
  hud = s.bool({ default = true, scope = enums.SettingScope.Player }),
  motto = s.text({ default = "hi", scope = enums.SettingScope.Startup }),
})
