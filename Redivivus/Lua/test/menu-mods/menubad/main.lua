-- A screen whose view fails, a button that opens it, and a menu export that then fails.
local M = {}
function M.menu(menu)
  local s = menu.addScreen(function() error("the view fails") end)
  menu.addButton("Bad", function() s.open() end)
  error("menubad's menu fails")
end
return M
