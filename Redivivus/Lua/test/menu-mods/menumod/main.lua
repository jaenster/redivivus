-- A mod with a main menu button and a screen of its own, through the menu export.
local ui = require("redivivus:ui")
local shared = require("redivivus:shared")

local M = {}

function M.menu(menu)
  local screen
  screen = menu.addScreen(function(s)
    shared.log("menumod view built", s == screen)
    return ui.h("flow", { direction = ui.Direction.Column, gap = 4 },
      ui.h("label", { text = "Menu Mod screen" }),
      ui.h("button", { label = "Close", testId = "close", onClick = function() s.close() end }))
  end)
  menu.addButton("Menu Mod", function()
    shared.log("menumod pressed")
    screen.open()
  end)
  -- The menu has room for two buttons: MODS and this mod's first.
  menu.addButton("Third", function() end)
  shared.log("menumod menu ran", type(menu), screen.isOpen())
end

function M.server(game)
  shared.log("menumod server", game.id)
end

return M
