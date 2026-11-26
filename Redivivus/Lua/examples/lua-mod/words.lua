-- A module of lua-mod's own, required as require("words") or require("lua-mod:words").
local words = {}

function words.describe(...)
  return table.concat({ ... }, ", ")
end

return words
