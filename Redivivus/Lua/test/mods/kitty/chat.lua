-- A typed channel, declared once: requests go client to server, events server to client.
local shared = require("redivivus:shared")
local t = shared.t
return shared.defineChannel("chat", {
  requests = {
    say = { t.text(20), t.int(0, 3) },
    move = { t.record({ x = t.int(), y = t.int() }), t.optional(t.oneOf({ "walk", "run" })) },
  },
  events = {
    said = { t.int(), t.text(20) },
    tags = { t.list(t.text(8), 3), t.bool(), t.number(0, 10) },
  },
})
