-- Generated from Redivivus/Lua/kit/server/scripts.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
local ____exports = {}
--- Start a script, owned by this mod and, when given, by owner. It first runs on the next tick.
function ____exports.startScript(body, owner)
    local id = script.start(body, owner)
    return {
        id = id,
        cancel = function() return script.cancel(id) end,
        isRunning = function() return script.running(id) end
    }
end
--- Wait one tick.
function ____exports.wait()
    coroutine.yield()
end
--- Wait n ticks (at least one). The outcome is the tick the script resumes on.
function ____exports.waitTicks(n)
    return script.wait(n)
end
--- Wait until pred holds, checking it now and then once a tick, for at most timeoutTicks ticks.
-- Outcome.Done when it held, Outcome.Timeout when the time ran out first.
function ____exports.waitUntil(pred, timeoutTicks)
    if pred() then
        return 0
    end
    do
        local i = 0
        while i < timeoutTicks do
            coroutine.yield()
            if pred() then
                return 0
            end
            i = i + 1
        end
    end
    return 3
end
return ____exports
