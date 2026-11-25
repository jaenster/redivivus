local helper = require("__alpha__/shared/helper")
log("beta control", helper.double(5), helper.loaded_by)
script.on_event("on_tick", function(e)
  beta_ticks = (beta_ticks or 0) + 1
end)
