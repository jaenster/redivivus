local shared = require("redivivus:shared")
local actions = require("first-mod:actions")
return {
  server = function(game)
    game.startScript(function()
      local gave = actions.walk(3)
      shared.log("lua script: ts action gave", gave)
      shared.log("lua script: done")
    end)
  end,
}
