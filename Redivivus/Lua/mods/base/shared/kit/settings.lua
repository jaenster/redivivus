-- Generated from Redivivus/Lua/kit/shared/settings.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
local ____lualib = require("__base__/shared/lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local __TS__ObjectKeys = ____lualib.__TS__ObjectKeys
local __TS__ArraySort = ____lualib.__TS__ArraySort
local ____exports = {}
local function spec(declaration)
    return {declaration = declaration}
end
--- The builders of setting declarations.
____exports.setting = {
    bool = function(o)
        return spec(__TS__ObjectAssign({}, o, {type = "bool"}))
    end,
    int = function(o)
        return spec(__TS__ObjectAssign({}, o, {type = "int"}))
    end,
    number = function(o)
        return spec(__TS__ObjectAssign({}, o, {type = "number"}))
    end,
    text = function(o)
        return spec(__TS__ObjectAssign({}, o, {type = "string"}))
    end,
    choice = function(o)
        return spec(__TS__ObjectAssign({}, o, {type = "choice"}))
    end
}
--- Declare the settings (sorted by name, which is the order a settings screen lists them in) and
-- get an object that reads their values. Writing to it is an error.
function ____exports.defineSettings(specs)
    local names = __TS__ArraySort(__TS__ObjectKeys(specs))
    local declared = {}
    for ____, name in ipairs(names) do
        mod_settings.declare(name, specs[name].declaration)
        declared[name] = true
    end
    local values = setmetatable(
        {},
        {
            __index = function(_, key)
                if not (declared[key] ~= nil) then
                    error(
                        ("no setting '" .. tostring(key)) .. "' in this set",
                        2
                    )
                end
                return mod_settings.get(key)
            end,
            __newindex = function(_, key)
                error(
                    ("settings are read-only (setting '" .. tostring(key)) .. "')",
                    2
                )
            end,
            __metatable = false
        }
    )
    return values
end
return ____exports
