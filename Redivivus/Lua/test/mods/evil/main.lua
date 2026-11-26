-- The adversarial suite's mod: the host test runs its attempts as chunks in this mod.
local M = {}
function M.server(game)
  games = games or {}
  games[game.id] = game
end
return M
