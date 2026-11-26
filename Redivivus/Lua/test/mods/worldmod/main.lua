-- The world classes and the AI hook, run by the host test on its stand-in engine
-- (test/d2_lua_test_engine.cpp): one player, two zombies, then a Fallen.
local shared = require("redivivus:shared")
local server = require("redivivus:server")
local enums = require("redivivus:enums")
local log = shared.log
local M = {}

local function errorOf(fn, want)
  local ok, e = pcall(fn)
  if ok then return "no error" end
  e = tostring(e)
  return e:find(want, 1, true) and want or e
end

local function fallenScript(m, think)
  fallen = m
  log("fallen script start")
  local o = m:walkTo(server.subtile(125, 100))
  log("walk outcome", o)
  o = m:idle(5)
  log("idle outcome", o)
  o = m:walkTo(server.subtile(5000, 5000))
  log("blocked outcome", o)
  local victim = g.spawnMonster(enums.MonsterId.Zombie, server.subtile(140, 100), { radius = 2 })
  pendingKill = victim
  o = m:walkToward(victim)
  log("toward outcome", o)
  local pl
  for p in g.players() do pl = p end
  o = m:attack(pl)
  log("attack outcome", o)
  while true do
    m:idle(10)
    if explode then error("worldmod fails on purpose") end
  end
end

function M.server(game)
  g = game
  log("game:", game.difficulty, game.expansion, game.name, game.seed)
  local names = {}
  for p in game.players() do names[#names + 1] = p.name end
  log("players: " .. table.concat(names, ","))
  local lvl = game.level(enums.LevelId.BloodMoor)
  local b = lvl.bounds
  local n = 0
  for _ in lvl:rooms() do n = n + 1 end
  log("level 2:", lvl.name, lvl.isTown, lvl.generated, b.x, b.y, b.width, b.height, "rooms", n)
  log("level 1:", game.level(enums.LevelId.RogueEncampment))
  local player
  for p in game.players() do player = p end
  local c = 0
  for _ in player:nearby(25, enums.UnitType.Monster) do c = c + 1 end
  log("near the player: " .. c .. " monster(s)")
  c = 0
  for _ in game.unitsNear(server.subtile(118, 100), 13) do c = c + 1 end
  log("near (118,100): " .. c)
  local zombie = game.findUnit(enums.UnitType.Monster, 1)
  far = game.findUnit(enums.UnitType.Monster, 2)
  local pos = zombie.position
  log("zombie:", zombie.classId, pos.x, pos.y, zombie.valid, zombie.life, zombie.room and "room", "level", zombie.level.id)
  local keyed = { [zombie] = true }
  log("same handle:", game.findUnit(enums.UnitType.Monster, 1) == zombie, keyed[game.findUnit(enums.UnitType.Monster, 1)] == true)
  zombie.life = 30
  local had = zombie:addState(enums.StateId.Red, 25) and zombie:hasState(enums.StateId.Red)
  zombie:removeState(enums.StateId.Red)
  log("stat:", player.charLevel, "life", zombie.life, "state", had, zombie:hasState(enums.StateId.Red))
  log("errors: " .. table.concat({
    errorOf(function() return zombie:distanceTo(server.tile(1, 1)) end, "tile point"),
    errorOf(function() return game.spawnMonster(5, server.subtile(0, 0), { radius = 99 }) end, "outside"),
    errorOf(function() zombie.life = 0 end, "0 or less"),
    errorOf(function() zombie:setStat(enums.StatId.Gold, 5) end, "gold"),
    errorOf(function() zombie:idle(5) end, "own AI"),
  }, "; "))
  local p = server.subtile(100, 100)
  local t = server.toTile(p)
  local s = server.toSubtile(t)
  log("points:", p.kind, p.x, p.y, t.kind, t.x, t.y, s.kind, s.x, s.y)
  game.registry.ai.wrap(enums.MonsterAiId.Zombie, function(next, m, think)
    log("wrap think")
    next()
  end)
  game.registry.ai.register(enums.MonsterAiId.Fallen, fallenScript)
  log("registered:", game.registry.ai.has(enums.MonsterAiId.Fallen), "wrapped:", game.registry.ai.has(enums.MonsterAiId.Zombie),
      "none:", game.registry.ai.has(enums.MonsterAiId.Skeleton))
  game.on(enums.Event.Tick, function()
    if pendingKill and pendingKill.valid then
      pendingKill:kill()
      pendingKill = nil
    end
  end)
  game.message("hello from worldmod")
end

return M
