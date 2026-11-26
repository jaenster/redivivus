-- A mod in plain Lua: the same program shape as a TypeScript one. main returns its exports; the
-- engine calls server(game) for every game this process hosts and client(client) for every game
-- this client is in.
local shared = require("redivivus:shared")
local enums = require("redivivus:enums")
local words = require("words")

shared.log("lua-mod loaded: " .. words.describe("redivivus:shared", "redivivus:enums") .. " and its own module")

local M = {}

function M.server(game)
  local seen = game.storage("seen", { ticks = 0 })
  shared.log("lua-mod server(game " .. game.id .. ")")
  game.on(enums.Event.Tick, function(e)
    seen.ticks = seen.ticks + 1
  end)
  game.startScript(function()
    shared.wait(2)
    shared.log("lua-mod script waited, tick " .. game.tick)
  end)
end

function M.client(client)
  shared.log("lua-mod client(game " .. client.gameId .. ")")
end

return M
