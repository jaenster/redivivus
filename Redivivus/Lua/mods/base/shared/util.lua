-- Small helpers every mod may use: require("__base__/shared/util").
local util = {}

-- A shallow copy of a table.
function util.copy(t)
  local out = {}
  for k, v in pairs(t) do
    out[k] = v
  end
  return out
end

-- The number of entries in a table, array part or not.
function util.count(t)
  local n = 0
  for _ in pairs(t) do
    n = n + 1
  end
  return n
end

return util
