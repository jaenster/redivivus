-- alpha's control entry: one program, loaded into the server and the client state alike.
local shared = require("redivivus:shared")
local enums = require("redivivus:enums")
local helper = require("helper")
local log = shared.log
log("alpha main", helper.double(21), helper == require("alpha:helper"), helper == require("./helper"))

local M = {}

-- A plain export other mods import (alpha:main).
function M.add(a, b)
  return a + b
end

-- Module-level: shared by every game this state serves; a test shows it is not per game.
side_seen = shared.side

function M.server(game)
  games = games or {}
  games[game.id] = game
  local state = game.storage("state", { counter = 0, inits = 0 })
  if game.loaded then
    log("alpha loaded, counter", state.counter)
  else
    state.inits = state.inits + 1
  end
  server_wrote = "server " .. game.id
  game.on(enums.Event.Tick, function(e)
    state.counter = state.counter + 1
  end)
end

function M.client(client)
  clients = clients or {}
  clients[client.gameId] = client
  log("alpha client sees server_wrote =", tostring(server_wrote))
end

return M
