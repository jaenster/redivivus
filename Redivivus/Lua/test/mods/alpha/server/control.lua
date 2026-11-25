local helper = require("shared.helper")
local util = require("__base__/shared/util")
log("alpha control", helper.double(21), util.count({ a = 1, b = 2 }), helper == require("shared/helper"))
remote.add_interface("alpha", {
  add = function(a, b) return a + b end,
  whoami = function() return script.mod_name end,
  fail = function() error("alpha remote fails") end,
  spin = function() while true do end end,
})
script.on_init(function()
  storage.counter = 0
  storage.inits = (storage.inits or 0) + 1
end)
script.on_load(function()
  log("alpha on_load, counter", storage.counter)
end)
script.on_event(Event.Tick, function(e)
  storage.counter = storage.counter + 1
end)
