-- A shared module: each requiring mod gets its own instance.
local helper = { loads = 0 }
helper.loads = helper.loads + 1
function helper.double(x)
  return x * 2
end
helper.loaded_by = script and script.mod_name or "data stage"
return helper
