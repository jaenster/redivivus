-- Generated from Redivivus/Lua/kit/server/persist.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
local ____lualib = require("__base__/shared/lualib_bundle")
local __TS__ArraySplice = ____lualib.__TS__ArraySplice
local ____exports = {}
local KEY = "__kit_persist"
local index = {}
local indexed
local function entries()
    local list = storage[KEY]
    if not list then
        list = {}
        storage[KEY] = list
    end
    if list ~= indexed then
        index = {}
        for ____, e in ipairs(list) do
            index[e.owner] = e
        end
        indexed = list
    end
    return list
end
local function copy(value)
    if type(value) ~= "table" then
        return value
    end
    local out = {}
    for k, v in pairs(value) do
        out[k] = copy(v)
    end
    return out
end
--- The persisted state of owner, created from defaults the first time.
function ____exports.persist(owner, defaults)
    if type(owner) ~= "userdata" or not owner.valid then
        error("persist: the owner must be a valid handle", 2)
    end
    local list = entries()
    local found = index[owner]
    if found then
        return found.state
    end
    local entry = {
        owner = owner,
        state = copy(defaults)
    }
    list[#list + 1] = entry
    index[owner] = entry
    return entry.state
end
--- Whether owner has persisted state.
function ____exports.hasPersisted(owner)
    entries()
    return index[owner] ~= nil
end
--- Drop the state of every owner whose object is gone. The module's script calls it each tick.
function ____exports.sweep()
    local list = storage[KEY]
    if not list then
        return 0
    end
    entries()
    local removed = 0
    do
        local i = #list - 1
        while i >= 0 do
            if not list[i + 1].owner.valid then
                index[list[i + 1].owner] = nil
                __TS__ArraySplice(list, i, 1)
                removed = removed + 1
            end
            i = i - 1
        end
    end
    return removed
end
script.start(function()
    do
        while true do
            coroutine.yield()
            ____exports.sweep()
        end
    end
end)
return ____exports
