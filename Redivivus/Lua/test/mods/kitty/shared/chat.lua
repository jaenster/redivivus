-- A typed channel, from plain Lua: requests go client to server, events server to client.
local ch = require("__base__/shared/kit/channels")
local t = ch.t
return ch.defineChannel("chat", {
  requests = {
    say = { t.text(20), t.int(0, 3) },
    move = { t.record({ x = t.int(), y = t.int() }), t.optional(t.oneOf({ "walk", "run" })) },
  },
  events = {
    said = { t.int(), t.text(20) },
    tags = { t.list(t.text(8), 3), t.bool(), t.number(0, 10) },
  },
})
