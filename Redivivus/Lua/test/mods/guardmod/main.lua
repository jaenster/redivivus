-- guard inside a monster's AI script, run by the host test on its stand-in engine
-- (test/d2_lua_test_engine.cpp): one Fallen.
local shared = require("redivivus:shared")
local server = require("redivivus:server")
local enums = require("redivivus:enums")
local log = shared.log
local guard = shared.guard
local M = {}

-- The coroutines of the script and of its guarded bodies, to see them go.
threads = setmetatable({}, { __mode = "k" })

local function script(m)
  threads[coroutine.running()] = true
  -- Actions inside a guarded body are the monster's own script's.
  local o = guard(function() return true end, function()
    threads[coroutine.running()] = true
    local a = m:walkTo(server.subtile(125, 100))
    local b = m:idle(5)
    return a .. "," .. b
  end)
  log("ai guarded actions", o)
  -- The check fails while an action the body started runs: the engine finishes it, the body is dropped.
  cut = false
  o = guard(function() return not cut end, function()
    m:idle(4)
    cut = true
    cutAt = g.frame
    m:idle(4)
    log("ai never")
  end)
  log("ai cut", o, g.frame - cutAt)
  o = guard(function() return true end, function()
    return guard(function() return false end, function() log("ai never 2") end)
  end)
  log("ai nested", o)
  -- An action from a coroutine of the mod's own is still refused.
  local co = coroutine.create(function() return m:idle(1) end)
  log("ai own coroutine", select(2, coroutine.resume(co)))
  log("ai guarding forever")
  guard(function() return true end, function()
    threads[coroutine.running()] = true
    while true do m:idle(3) end
  end)
end

function M.server(game)
  g = game
  game.registry.ai.register(enums.MonsterAiId.Fallen, script)
  log("guardmod ready")
end

return M
