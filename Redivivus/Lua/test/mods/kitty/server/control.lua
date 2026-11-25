local chat = require("shared.chat")
chat.implement({
  say = function(peer, text, colour)
    log("server got say", peer, text, colour)
    chat.emit.said(0, peer, text)
  end,
  move = function(peer, pos, how)
    log("server got move", peer, pos.x, pos.y, tostring(how))
  end,
})
