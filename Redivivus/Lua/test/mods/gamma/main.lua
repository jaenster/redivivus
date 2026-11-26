local shared = require("redivivus:shared")
local ok, err = pcall(require, "alpha:helper")
shared.log("gamma requires alpha without the dependency:", ok, err)
return {
  server = function(game)
    games = games or {}
    games[game.id] = game
    game.on("tick", function(e)
      gamma_ticks = (gamma_ticks or 0) + 1
    end)
  end,
}
