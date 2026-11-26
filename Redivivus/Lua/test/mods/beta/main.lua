local shared = require("redivivus:shared")
local helper = require("alpha:helper")
local alpha = require("alpha:main")
shared.log("beta main", helper.double(5), helper.loaded_by, alpha.add(2, 3))
local M = {}
function M.server(game)
  games = games or {}
  games[game.id] = game
  game.on("tick", function(e)
    beta_ticks = (beta_ticks or 0) + 1
  end)
end
return M
