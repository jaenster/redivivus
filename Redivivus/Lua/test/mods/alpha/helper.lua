-- A module of alpha's: each requiring mod gets its own instance.
local helper = { loads = 0 }
helper.loads = helper.loads + 1
function helper.double(x)
  return x * 2
end
helper.loaded_by = require("redivivus:shared").modName
return helper
