local shared = require("redivivus:shared")
local chat = require("chat")
local log = shared.log
local M = {}
function M.server(game)
  games = games or {}
  games[game.id] = game
  local ch = game.channel(chat)
  channels = channels or {}
  channels[game.id] = ch
  ch.implement({
    say = function(peer, text, colour)
      log("server got say", game.id, peer, text, colour)
      ch.emit.said(0, peer, text)
    end,
    move = function(peer, pos, how)
      log("server got move", peer, pos.x, pos.y, tostring(how))
    end,
  })
end
function M.client(client)
  clients = clients or {}
  clients[client.gameId] = client
  local ch = client.channel(chat)
  channels = channels or {}
  channels[client.gameId] = ch
  ch.on.said(function(from, text) log("client said", from, text) end)
  ch.on.tags(function(list, flag, n) log("client tags", #list, list[1], list[3], flag, n) end)
end
return M
